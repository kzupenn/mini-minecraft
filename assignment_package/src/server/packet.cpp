#include "packet.h"

Packet bufferToPacket(QByteArray buffer) {
    QDataStream in(&buffer, QIODevice::ReadWrite);
    PacketType pt;
    in >> pt;
    qDebug() << "packet parser has packet of type: " << pt;
    switch(pt) {
    case PLAYER_STATE:
        int pid;
        float f1, f2, f3, f4, f5;
        in >> pid >> f1 >> f2 >> f3 >> f4 >> f5;
        return PlayerStatePacket(pid, glm::vec3(f1, f2, f3), f4, f5);
        break;
    default:
        qDebug() << "weird packet received:" << pt;
        break;
    }
}


