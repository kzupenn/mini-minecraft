#pragma once
#include "glm_includes.h"
#include "scene/chunk.h"
#include "scene/item.h"
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

using namespace glm;

enum PacketType: unsigned char {
    BAD_PACKET,
    PLAYER_STATE, WORLD_INIT, PLAYER_JOIN, CHAT, PLAYER_DEATH,
    CHUNK_CHANGE, BLOCK_CHANGE,
    ITEM_ENTITY_STATE, DELETE_ITEM_ENTITY, ENTITY_STATE, DELETE_ENTITY,
    HIT
};

struct Packet{
    PacketType type;
    virtual QByteArray packetToBuffer() { return QByteArray();}
    Packet(PacketType t) : type(t) {}
    Packet(){}
    virtual ~Packet(){};
};

//World Init packet
//sends over the initialization information for the player world
//Server: send world seed info to client for consistency
struct WorldInitPacket : Packet {
    int seed;
    vec3 spawn;
    int pid;
    std::vector<std::pair<int, QString>> players;
    WorldInitPacket(int s, int ppid, glm::vec3 p, std::vector<std::pair<int, QString>> pp) : Packet(WORLD_INIT), seed(s), pid(ppid), spawn(p), players(pp) {}
    ~WorldInitPacket(){}
    QByteArray packetToBuffer() override {
        QByteArray buffer;
        QDataStream out(&buffer,QIODevice::ReadWrite);
        int a = players.size(); //overloaded <<
        out << WORLD_INIT << seed << pid << spawn.x << spawn.y << spawn.z;
        out << a;
        for(std::pair<int, QString> pp: players) {
            out << pp.first << pp.second;
        }
        return buffer;
    }
};

//Chunk changes packet
//send over the changes made to a chunk so far
struct ChunkChangePacket: Packet {
    int64_t chunkPos;
    std::vector<std::pair<vec3, BlockType>> changes;
    ChunkChangePacket(int64_t cp, std::vector<std::pair<vec3, BlockType>> ch):Packet(CHUNK_CHANGE), chunkPos(cp), changes(ch) {};
    ~ChunkChangePacket(){}
    QByteArray packetToBuffer() override {
        QByteArray buffer;
        QDataStream out(&buffer,QIODevice::ReadWrite);
        int foo = changes.size();
        out << CHUNK_CHANGE << chunkPos << foo;
        for(std::pair<vec3, BlockType> &p: changes) {
            unsigned char xz = 16*p.first.x + p.first.z;
            unsigned char y = p.first.y;
            out << xz << y << p.second;
        }
        return buffer;
    }
};

//Block changes packet
//send over a singular change
struct BlockChangePacket: Packet {
    int64_t chunkPos;
    unsigned char yPos;
    BlockType newBlock;
    BlockChangePacket(int64_t cp, unsigned char yp, BlockType bt) : Packet(BLOCK_CHANGE), chunkPos(cp), yPos(yp), newBlock(bt){}
    ~BlockChangePacket(){};
    QByteArray packetToBuffer() override {
        QByteArray buffer;
        QDataStream out(&buffer,QIODevice::ReadWrite);
        out << BLOCK_CHANGE << chunkPos << yPos << newBlock;
        return buffer;
    }
};

//PlayerState packet
//sends over the position and attitude state of the player
//Server: send player state updates and player creation to client
//Client: send over movement input updates to server
struct PlayerStatePacket : public Packet{
    int player_id; //id by server assigned client_fd
    //position
    vec3 player_pos;
    //velocity
    vec3 player_velo;
    //head facing
    float player_phi, player_theta;
    //holding
    ItemType player_hand;
    //armor
    ItemType player_helmet;
    ItemType player_chest;
    ItemType player_leg;
    ItemType player_boots;
    //gamemode
    bool player_creative;

    PlayerStatePacket(int i, glm::vec3 p, glm::vec3 v, int t, int ph, ItemType it, ItemType ita, ItemType itb, ItemType itc, ItemType itd, bool cr) : Packet(PLAYER_STATE), player_id(i), player_pos(p), player_velo(v), player_theta(t), player_phi(ph), player_hand(it), player_helmet(ita), player_chest(itb), player_leg(itc), player_boots(itd), player_creative(cr) {}
    PlayerStatePacket(glm::vec3 p, glm::vec3 v, int t, int ph, ItemType it, ItemType ita, ItemType itb, ItemType itc, ItemType itd, bool cr) : PlayerStatePacket(0, p, v, t, ph, it, ita, itb, itc, itd, cr) {}
    ~PlayerStatePacket(){}
    QByteArray packetToBuffer() override {
        QByteArray buffer;
        QDataStream out(&buffer,QIODevice::ReadWrite);
        out << PLAYER_STATE << player_id
            << player_pos.x << player_pos.y << player_pos.z
            << player_velo.x << player_velo.y << player_velo.z
            << player_theta << player_phi << player_hand
            << player_helmet << player_chest << player_leg << player_boots
            << player_creative;
        return buffer;
    }
};

//Player joined/left game packet
struct PlayerJoinPacket : public Packet {
    bool join;
    int player_id;
    QString name;
    PlayerJoinPacket(bool j, int i, QString n) : Packet(PLAYER_JOIN), join(j), player_id(i), name(n) {}
    ~PlayerJoinPacket() {}
    QByteArray packetToBuffer() override {
        QByteArray buffer;
        QDataStream out(&buffer,QIODevice::ReadWrite);
        out << PLAYER_JOIN << join << player_id << name;
        return buffer;
    }
};

//Chat packet
struct ChatPacket : public Packet {
    int player_id;
    QString message;
    ChatPacket(int i, QString n) : Packet(CHAT), player_id(i), message(n) {}
    ~ChatPacket() {}
    QByteArray packetToBuffer() override {
        QByteArray buffer;
        QDataStream out(&buffer,QIODevice::ReadWrite);
        out << CHAT << player_id << message;
        return buffer;
    }
};

//Item Entity State
struct ItemEntityStatePacket : public Packet {
    int entity_id;
    ItemType type;
    int count;
    glm::vec3 pos;
    ItemEntityStatePacket(int id, ItemType it, int c, glm::vec3 p) :
        Packet(ITEM_ENTITY_STATE), entity_id(id), type(it), count(c), pos(p){}
    ~ItemEntityStatePacket(){}
    QByteArray packetToBuffer() override {
        QByteArray buffer;
        QDataStream out(&buffer, QIODevice::ReadWrite);
        out << ITEM_ENTITY_STATE << entity_id << type << count << pos.x << pos.y << pos.z;
        return buffer;
    }
};
//delete item entity
struct ItemEntityDeletePacket : public Packet {
    int entity_id;
    ItemEntityDeletePacket(int id) :
        Packet(DELETE_ITEM_ENTITY), entity_id(id){}
    ~ItemEntityDeletePacket(){}
    QByteArray packetToBuffer() override {
        QByteArray buffer;
        QDataStream out(&buffer, QIODevice::ReadWrite);
        out << DELETE_ITEM_ENTITY << entity_id;
        return buffer;
    }
};
//hit
struct HitPacket : public Packet {
    glm::vec3 direction;
    int damage;
    HitPacket(int dd, glm::vec3 d): damage(dd), direction(d), Packet(HIT){}
    ~HitPacket(){}
    QByteArray packetToBuffer() override {
        QByteArray buffer;
        QDataStream out(&buffer, QIODevice::ReadWrite);
        out << HIT << damage <<  direction.x << direction.y << direction.z;
        return buffer;
    }
};

//player dies
struct DeathPacket : public Packet {
    int victim, killer;
    DeathPacket(int a, int b): victim(a), killer(b), Packet(PLAYER_DEATH){}
    ~DeathPacket(){}
    QByteArray packetToBuffer() override {
        QByteArray buffer;
        QDataStream out(&buffer, QIODevice::ReadWrite);
        out << PLAYER_DEATH << victim << killer;
        return buffer;
    }
};



Packet* bufferToPacket(QByteArray buffer);


