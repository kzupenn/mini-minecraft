#pragma once
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include "server/packet.h"

class Client
{
private:
    void receive_messages(void*arg);

    int client_fd;
    struct sockaddr_in server_address;
    bool open;
    pthread_t receive_thread;

    void (*packet_parser) (Packet);

public:
    Client(std::string, void (*packet_parser) (Packet));
    void start();
    bool sendPacket(Packet);
    void close();
};

