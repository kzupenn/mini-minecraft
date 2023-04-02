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


using namespace std;

void Client::start()
{
    char buffer[BUFFER_SIZE];
    while (open)
    {
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
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
            buffer[bytes_received] = '\0';
            qDebug() << "Server packet: " << buffer;
        }
    }
}

Client::Client(std::string address)
{
    // create client socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "Failed to create client socket" << endl;
        return;
    }

    // connect to server
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, &address[0], &server_address.sin_addr) <= 0)
    {
        cout << "Invalid server address" << endl;
        return;
    }
    if (connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
        cout << "Failed to connect to server" << endl;
        return;
    }

    // create thread to receive messages
    ClientWorker* cw = new ClientWorker(this);
    QThreadPool::globalInstance()->start(cw);
}

bool Client::sendPacket(char* buffer) {
    int bytes_sent = send(client_fd, buffer, strlen(buffer), 0);
    return (bytes_sent >= 0);
}

//closes the client thread
void Client::close() {
    open = false;
}

