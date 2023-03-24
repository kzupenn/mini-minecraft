#pragma once

#include <pthread.h>
#include <string>

#define PORT 3078
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

class Server{
private:
    static std::string process_packet(char*);

    static void* handle_client(void* arg);
public:
    int start();
};

