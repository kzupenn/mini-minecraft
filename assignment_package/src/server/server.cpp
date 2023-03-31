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

using namespace std;

int server_fd;
int client_fds[MAX_CLIENTS];
pthread_t threads[MAX_CLIENTS];

void* Server::handle_client(void* arg)
{
    int client_fd = *(int*)arg;
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

    // set socket options
    // int opt = 1;
    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    // {
    //     cout << "Failed to set socket options" << endl;
    //     return -1;
    // }

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
            // find an available slot in the client_fds array
            int i;
            for (i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_fds[i] == 0)
                {
                    client_fds[i] = client_fd;
                    break;
                }
            }

            // check if the array is full
            if (i == MAX_CLIENTS)
            {
                cout << "Maximum number of clients reached" << endl;
                close(client_fd);
            }
            else
            {
                // create a new thread to handle the client
                pthread_create(&threads[i], NULL, this->handle_client, (void*)&client_fd);
                pthread_detach(threads[i]);
                cout << "New client connected: " << inet_ntoa(address.sin_addr) << endl;
            }
        }
    }

    return 0;
}

