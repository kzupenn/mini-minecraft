#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "server.h"
#include "scene/runnables.h"
#include <QThreadPool>

#define BUFFER_SIZE 5000

using namespace std;


Server::Server(int s) : m_terrain(nullptr), seed(s), setup(false), open(true){
    m_clients.setMaxThreadCount(MAX_CLIENTS);
    ServerConnectionWorker* sw = new ServerConnectionWorker(this);
    QThreadPool::globalInstance()->start(sw);
}

void Server::handle_client(int client_fd)
{
    QByteArray buffer;
    buffer.resize(BUFFER_SIZE);
    while (open)
    {
        int valread = read(client_fd, &buffer[0], BUFFER_SIZE);
        if (valread == 0)
        {
            // client has disconnected
            cout << "Client " << client_fd << " disconnected" << endl;
            close(client_fd);
            break;
        }
        else
        {
            qDebug() << "Client " << client_fd << " says: " << buffer;
            buffer.resize(valread);
            process_packet(bufferToPacket(buffer));
            buffer.resize(BUFFER_SIZE);
        }
    }
}

void Server::process_packet(Packet packet) {
    switch(packet.type) {
    case PLAYER_STATE: {
        PlayerStatePacket* thispack = dynamic_cast<PlayerStatePacket*>(&packet);
        m_players_mutex.lock();
        m_players[thispack->player_id].pos = thispack->player_pos;
        m_players[thispack->player_id].phi = thispack->player_phi;
        m_players[thispack->player_id].theta = thispack->player_theta;
        m_players_mutex.unlock();
    }
    default:
        qDebug() << "unexpected packet type found";
    }
}

void Server::send_packet(Packet packet, int exclude) { //use exclude = 0 if you dont want to exclude
    client_fds_mutex.lock();
    for (int i = 0; i < client_fds.size(); i++)
    {
        if (client_fds[i] != exclude)
        {
            QByteArray buffer = packet.packetToBuffer();
            send(client_fds[i], buffer, buffer.size(), 0);
        }
    }
    client_fds_mutex.unlock();
}

void Server::target_packet(Packet packet, int target) {
    client_fds_mutex.lock();
    for (int i = 0; i < client_fds.size(); i++)
    {
        if (client_fds[i] == target)
        {
            QByteArray buffer = packet.packetToBuffer();
            send(client_fds[i], buffer, buffer.size(), 0);
            client_fds_mutex.unlock();
            return;
        }
    }
    qDebug() << "target for packet not found: " << target;
    client_fds_mutex.unlock();
}

void Server::initClient(int i) {
    client_fds_mutex.lock();
    client_fds.push_back(i);
    m_players.push_back(PlayerState(glm::vec3(0, 80, 0), 0.f, 0.f));
    client_fds_mutex.unlock();
}

int Server::start()
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        cout << "Failed to create server socket" << endl;
        return -1;
    }

    // bind server socket to address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (::bind(server_fd, (struct sockaddr*)&address, addrlen) < 0)
    {
        cout << "Failed to bind server socket to address" << endl;
        return -1;
    }

    // listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        cout << "Failed to listen for incoming connections" << endl;
        return -1;
    }

    cout << "Server started listening on port " << PORT << endl;

    setup = true;

    // wait for incoming connections and handle each one in a separate thread
    while (true)
    {
        int client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_fd < 0)
        {
            cout << "Failed to accept incoming connection" << endl;
            continue;
        }
        else
        {
            // do initial actions
            initClient(client_fd);
            // create a new thread to handle the client
            ServerThreadWorker* stw = new ServerThreadWorker(this, client_fd);
            m_clients.start(stw);
            cout << "New client connected: " << inet_ntoa(address.sin_addr) << endl;
        }
    }

    return 0;
}

void Server::shutdown() {
    open = false;
}

