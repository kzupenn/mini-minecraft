#include <iostream>
#include <string>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server.h"
#include "gamestate.h"
#include "scene/runnables.h"
#include <QThreadPool>

using namespace std;

Server::Server(int s) : m_terrain(nullptr), seed(s), setup(false){
    m_clients.setMaxThreadCount(10); //allow at most 10 clients for now
    ServerConnectionWorker* sw = new ServerConnectionWorker(this);
    QThreadPool::globalInstance()->start(sw);
}

void Server::handle_client(int client_fd)
{
    char buffer[BUFFER_SIZE];
    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(client_fd, buffer, BUFFER_SIZE);
        if (valread == 0)
        {
            // client has disconnected
            cout << "Client " << client_fd << " disconnected" << endl;
            close(client_fd);
            pthread_exit(NULL);
        }
        else
        {
            // broadcast message to all clients
            cout << "Client " << client_fd << " says: " << buffer << endl;
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_fds[i] != 0 && client_fds[i] != client_fd)
                {
                    const char* toSend = process_packet(buffer).c_str();
                    send(client_fds[i], toSend, strlen(buffer), 0);
                }
            }
        }
    }
}

std::string Server::process_packet(char* c) {
    return "lmao";
}

void Server::initClient(int i) {
    //send(client_fds[i], "see" , 3, 0);
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

