#include "runnables.h"

BlockTypeWorker::BlockTypeWorker(Terrain* tt, int xx, int zz):t(tt), x(xx), z(zz){};
BlockTypeWorker::~BlockTypeWorker(){

};
void BlockTypeWorker::run() {
    t->instantiateChunkAt(x, z);
}

VBOWorker::VBOWorker(Chunk* cc):c(cc){};
VBOWorker::~VBOWorker(){

};
void VBOWorker::run(){
    c->createVBOdata();
}
