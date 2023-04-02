#pragma once

#include <pthread.h>
#include <string>
#include <QThreadPool>
#include "scene/terrain.h"
#include "scene/entity.h"

#define PORT 3078
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

class Server{
private:
    int server_fd;
    std::vector<int> client_fds;

    static std::string process_packet(char*);

    //actions to do when a client joins
    //send over world seed, entity and player list, etc
    void initClient(int);

    Terrain m_terrain;
    std::vector<Entity> m_players;
    std::vector<Entity> m_entities;

    QThreadPool m_clients;

    int seed;
public:
    Server(int);
    int start();
    void handle_client(int);
    bool setup;
};

