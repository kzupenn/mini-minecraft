#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "client.h"

#define PORT 3078
#define BUFFER_SIZE 1024

using namespace std;

void * Client::receive_messages(void *arg)
{
    int client_fd = *(int*)arg;
    char buffer[BUFFER_SIZE];
    while (true)
    {
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0)
        {
            cout << "Failed to receive message" << endl;
            break;
        }
        else if (bytes_received == 0)
        {
            cout << "Connection closed by server" << endl;
            break;
        }
        else
        {
            buffer[bytes_received] = '\0';
            cout << buffer << endl;
        }
    }
    close(client_fd);
    pthread_exit(NULL);
}

int Client::start()
{
    int client_fd;
    struct sockaddr_in server_address;

    // create client socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "Failed to create client socket" << endl;
        return -1;
    }

    // connect to server
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        cout << "Invalid server address" << endl;
        return -1;
    }
    if (connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
        cout << "Failed to connect to server" << endl;
        return -1;
    }

    // create thread to receive messages
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, (void*)&client_fd))
    {
        cout << "Failed to create thread for receiving messages" << endl;
        return -1;
    }

    // read and send messages from user input
    char buffer[BUFFER_SIZE];
    while (true)
    {
        cin.getline(buffer, BUFFER_SIZE);
        int bytes_sent = send(client_fd, buffer, strlen(buffer), 0);
        if (bytes_sent < 0)
        {
            cout << "Failed to send message" << endl;
            break;
        }
    }

    close(client_fd);
    pthread_join(receive_thread, NULL);

    return 0;
}
