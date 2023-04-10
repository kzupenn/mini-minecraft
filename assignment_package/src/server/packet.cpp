#include "packet.h"

Packet* bufferToPacket(QByteArray buffer) {
    QDataStream in(&buffer, QIODevice::ReadWrite);
    PacketType pt;
    in >> pt;
    switch(pt) {
    case PLAYER_STATE:{
        int pid;
        float f1, f2, f3, f4, f5;
        in >> pid >> f1 >> f2 >> f3 >> f4 >> f5;
        return new PlayerStatePacket(pid, glm::vec3(f1, f2, f3), f4, f5);
        break;
    }
    case WORLD_INIT:{
        int s;
        float f1, f2, f3;
        in >> s >> f1 >> f2 >> f3;
        return new WorldInitPacket(s, glm::vec3(f1, f2, f3));
        break;
    }
    default:
        qDebug() << "weird packet received:" << pt;
        break;
    }
}


