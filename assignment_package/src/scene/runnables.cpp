#include "runnables.h"

BlockTypeWorker::BlockTypeWorker(Terrain* tt, int xx, int zz):t(tt), x(xx), z(zz){};
BlockTypeWorker::~BlockTypeWorker(){

};
void BlockTypeWorker::run() {
    //QThread::currentThread()->setPriority(QThread::LowestPriority);
    t->instantiateChunkAt(x, z);
}

VBOWorker::VBOWorker(Chunk* cc):c(cc){};
VBOWorker::~VBOWorker(){

};
void VBOWorker::run(){
    QThread::currentThread()->setPriority(QThread::HighestPriority);
    c->createVBOdata();
}

StructureWorker::StructureWorker(Terrain* tt, StructureType ss, int xx, int yy, int zz):
t(tt), s(ss), x(xx), y(yy), z(zz){}
StructureWorker::~StructureWorker(){};

void StructureWorker:: run() {
    //QThread::currentThread()->setPriority(QThread::LowestPriority);
    switch(s) {
    case VILLAGE_CENTER:{
        t->processMegaStructure(generateVillage(glm::vec2(x, z)));
    }
    default:
        break;
    }
}

ServerConnectionWorker::ServerConnectionWorker(Server* ss): s(ss){}
ServerConnectionWorker::~ServerConnectionWorker(){};

void ServerConnectionWorker:: run() {
    QThread::currentThread()->setPriority(QThread::NormalPriority);
    s->start();
}

ServerThreadWorker::ServerThreadWorker(Server* ss, int tt): s(ss), t(tt){}
ServerThreadWorker::~ServerThreadWorker(){};

void ServerThreadWorker:: run() {
    QThread::currentThread()->setPriority(QThread::NormalPriority);
    s->handle_client(t);
}

ClientWorker::ClientWorker(MyGL* ss): s(ss){}
ClientWorker::~ClientWorker(){};

void ClientWorker:: run() {
    QThread::currentThread()->setPriority(QThread::NormalPriority);
    s->run_client();
}

