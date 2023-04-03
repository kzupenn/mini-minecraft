#pragma once
#include "glm_includes.h"
#include "scene/chunk.h"
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

using namespace glm;

enum PacketType: unsigned char {
    PLAYER_STATE, PLAYER_INFO, GAME_INIT
};

struct Packet{
    PacketType type;
    virtual QByteArray packetToBuffer() {
        return NULL;
    };
    Packet(PacketType t) : type(t) {}
};

//World Init packet
//sends over the initialization information for the player world
//Server: send world seed info to client for consistency
struct WorldInitPacket : Packet {
    int seed;
    vec2 spawn;
};

//Chunk changes packet
//send over the changes made to a chunk
struct ChunkChangePacket: Packet {
    int64_t chunkPos;
    std::vector<std::pair<vec2, BlockType>> changes;
};

//PlayerState packet
//sends over the position and attitude state of the player
//Server: send player state updates and player creation to client
//Client: send over movement input updates to server
struct PlayerStatePacket : Packet{
    int player_id; //id by server assigned client_fd
    vec3 player_pos;
    float player_phi, player_theta;

    PlayerStatePacket(int i, glm::vec3 p, int t, int ph) : Packet(PLAYER_STATE), player_id(i), player_pos(p), player_theta(t), player_phi(ph) {}
    PlayerStatePacket(glm::vec3 p, int t, int ph) : PlayerStatePacket(-1, p, t, ph) {}
    QByteArray packetToBuffer(Packet) {
        QByteArray buffer;
        QDataStream out(&buffer,QIODevice::ReadWrite);
        out << PLAYER_STATE << player_id
            << player_pos.x << player_pos.y << player_pos.z
            << player_theta << player_phi;
        return buffer;
    }
};

Packet bufferToPacket(QByteArray buffer);


