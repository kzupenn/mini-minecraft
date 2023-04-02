#pragma once
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>

#define BUFFER_SIZE 1024

class Client
{
private:
    void receive_messages(void*arg);

    int client_fd;
    struct sockaddr_in server_address;
    bool open;
    pthread_t receive_thread;

public:
    Client(char*);
    void start();
    bool sendPacket(char buffer[BUFFER_SIZE]);
    void close();
};

