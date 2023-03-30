#pragma once
#include <QThreadPool>
#include <QThread>
#include <QRunnable>
#include "glm_includes.h"
#include "terrain.h"

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

