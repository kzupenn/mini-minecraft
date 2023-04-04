#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <QThreadPool>
#include "client.h"
#include "scene/runnables.h"
#include <QDebug>

#define PORT 3078
#define BUFFER_SIZE 5000


using namespace std;

void Client::start()
{
    QByteArray buffer;
    qDebug() << "client listening";
    buffer.resize(BUFFER_SIZE);
    while (open)
    {
        int bytes_received = recv(client_fd, &buffer[0], buffer.size(), 0);
        if (bytes_received < 0)
        {
            qDebug() << "Failed to receive message";
            break;
        }
        else if (bytes_received == 0)
        {
            qDebug() << "Connection closed by server";
            break;
        }
        else
        {
            buffer.resize(bytes_received);
            qDebug() << "Server packet: " << buffer;
            packet_parser(bufferToPacket(buffer));
        }
    }
}

Client::Client(std::string address, void (*pp)(Packet)) : packet_parser(pp)
{
    // create client socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        qDebug() << "Failed to create client socket";
        return;
    }

    // connect to server
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, &address[0], &server_address.sin_addr) <= 0)
    {
        qDebug() << "Invalid server address";
        return;
    }
    if (connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
        qDebug() << "Failed to connect to server";
        return;
    }

    // create thread to receive messages
    open = true;
    ClientWorker* cw = new ClientWorker(this);
    QThreadPool::globalInstance()->start(cw);
}

bool Client::sendPacket(Packet* packet) {
    QByteArray buffer;
    qDebug() << "piqDebug" << packet->type;
    switch(packet->type) {
    case PLAYER_STATE:{
        buffer = (dynamic_cast<PlayerStatePacket*>(packet))->packetToBuffer();
        qDebug() << "???" << buffer.size() << dynamic_cast<PlayerStatePacket*>(packet)->player_id;
        break;
    }
    default:
        break;
    }
    int bytes_sent = send(client_fd, buffer.data(), buffer.size(), 0);
    //qDebug() << bytes_sent;
    return (bytes_sent >= 0);
}

//closes the client thread
void Client::close() {
    open = false;
}

