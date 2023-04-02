#pragma once
#include <QThreadPool>
#include <QThread>
#include <QRunnable>
#include "glm_includes.h"
#include "terrain.h"
#include "server/server.h"
#include "server/client.h"

//runnables
class BlockTypeWorker: public QRunnable {
private:
    Terrain* t;
    int x, z;
public:
    BlockTypeWorker(Terrain* tt, int xx, int zz);
    ~BlockTypeWorker();

    void run();
};

class VBOWorker: public QRunnable {
private:
    Chunk* c;
public:
    VBOWorker(Chunk* cc);
    ~VBOWorker();

    void run();
};

class StructureWorker: public QRunnable {
private:
    Terrain* t;
    StructureType s;
    int x, y, z;
public:
    StructureWorker(Terrain* t, StructureType s, int x, int y, int z);
    ~StructureWorker();

    void run();
};

class ServerConnectionWorker: public QRunnable {
private:
    Server* s;
public:
    ServerConnectionWorker(Server*);
    ~ServerConnectionWorker();
    void run();
};

class ServerThreadWorker: public QRunnable {
private:
    Server* s;
    int t;
public:
    ServerThreadWorker(Server*, int);
    ~ServerThreadWorker();
    void run();
};

class ClientWorker: public QRunnable {
private:
    Client* s;
public:
    ClientWorker(Client*);
    ~ClientWorker();
    void run();
};
