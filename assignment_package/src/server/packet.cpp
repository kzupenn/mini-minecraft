#include "packet.h"

Packet* bufferToPacket(QByteArray buffer) {
    QDataStream in(&buffer, QIODevice::ReadWrite);
    PacketType pt;
    in >> pt;
    switch(pt) {
    case PLAYER_STATE:{
        int pid;
        float f1, f2, f3, f4, f5;
        ItemType c;
        in >> pid >> f1 >> f2 >> f3 >> f4 >> f5 >> c;
        return new PlayerStatePacket(pid, glm::vec3(f1, f2, f3), f4, f5, c);
        break;
    }
    case WORLD_INIT:{
        int s;
        float f1, f2, f3;
        in >> s >> f1 >> f2 >> f3;
        return new WorldInitPacket(s, glm::vec3(f1, f2, f3));
        break;
    }
    
    case CHUNK_CHANGE: {
        int64_t cP;
        int num;
        in >> cP >> num;
        std::vector<std::pair<glm::vec3, BlockType>> changes;
        for(int i = 0; i < num; i++) {
            unsigned char a, b;
            BlockType c;
            in >> a >> b >> c;
            changes.push_back(std::make_pair(glm::vec3(a/16, b, a%16), c));
        }
        return new ChunkChangePacket(cP, changes);
        break;
    }
    case BLOCK_CHANGE:{
        int64_t cP;
        unsigned char yP;
        BlockType bt;
        in >> cP >> yP >> bt;
        return new BlockChangePacket(cP, yP, bt);
        break;
    }

    default:
        qDebug() << "weird packet received:" << pt;
        break;
    }
}


