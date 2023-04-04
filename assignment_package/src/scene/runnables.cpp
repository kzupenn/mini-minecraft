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

StructureWorker::StructureWorker(Terrain* tt, StructureType ss, int xx, int yy, int zz):
t(tt), s(ss), x(xx), y(yy), z(zz){}
StructureWorker::~StructureWorker(){};

void StructureWorker:: run() {
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
    s->start();
}

ServerThreadWorker::ServerThreadWorker(Server* ss, int tt): s(ss), t(tt){}
ServerThreadWorker::~ServerThreadWorker(){};

void ServerThreadWorker:: run() {
    s->handle_client(t);
}

ClientWorker::ClientWorker(MyGL* ss): s(ss){}
ClientWorker::~ClientWorker(){};

void ClientWorker:: run() {
    s->run_client();
}

