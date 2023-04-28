#pragma once

#include <string>
#include <QThreadPool>
#include "scene/terrain.h"
#include "scene/entity.h"
#include "scene/player.h"
#include "server/packet.h"

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

struct PlayerState {
    float phi, theta;
    glm::vec3 velo;
    glm::vec3 pos;
    QString name;
    //in hand item
    ItemType hand;
    //hp, armor
    int armor, health, toughness;
    //creative mode
    bool creative;
    //fall damage calculations
    bool isFalling;
    float fallHeight;
    int regen;

    PlayerState(glm::vec3 p, glm::vec3 v, float t, float ph, QString n)
        : phi(ph), theta(t), velo(v) , pos(p), name(n), armor(0), health(20), creative(true), isFalling(false), fallHeight(0), regen(600){};
    PlayerState(){};
    int calcArmor(std::vector<ItemType> armor) {
        int ret = 0;
        for(ItemType it: armor) {
            if(it){
                switch(it) {
                case GOLDEN_HELMET:
                    ret+=2;
                    break;
                case GOLDEN_CHESTPLATE:
                    ret+=5;
                    break;
                case GOLDEN_LEGGINGS:
                    ret+=3;
                    break;
                case GOLDEN_BOOTS:
                    ret+=1;
                    break;
                case IRON_HELMET:
                    ret+=2;
                    break;
                case IRON_CHESTPLATE:
                    ret+=6;
                    break;
                case IRON_LEGGINGS:
                    ret+=5;
                    break;
                case IRON_BOOTS:
                    ret+=2;
                    break;
                case DIAMOND_HELMET:
                    ret+=3;
                    break;
                case DIAMOND_CHESTPLATE:
                    ret+=8;
                    break;
                case DIAMOND_LEGGINGS:
                    ret+=6;
                    break;
                case DIAMOND_BOOTS:
                    ret+=3;
                    break;
                default:
                break;
                }
            }
        }
        return ret;
    }
    int calcTough(std::vector<ItemType> armor) {
        int ret = 0;
        for(ItemType it: armor) {
            if(it){
                switch(it) {
                case DIAMOND_HELMET:
                    ret+=2;
                    break;
                case DIAMOND_CHESTPLATE:
                    ret+=2;
                    break;
                case DIAMOND_LEGGINGS:
                    ret+=2;
                    break;
                case DIAMOND_BOOTS:
                    ret+=2;
                    break;
                default:
                break;
                }
            }
        }
        return ret;
    }
};

class Server{
private:
    int server_fd;
    std::mutex client_fds_mutex;
    std::vector<int> client_fds;

    void process_packet(Packet*, int);
    void broadcast_packet(Packet*, int);
    void target_packet(Packet*, int);

    //actions to do when a client joins
    //send over world seed, entity and player list, etc
    void initClient(int);
    //generates terrain around a player
    void generateTerrain(int x, int z);

    Terrain& m_terrain;
    std::mutex m_players_mutex;
    std::map<int, PlayerState> m_players;
    std::mutex m_entities_mutex;
    std::vector<Entity> m_entities;

    QThreadPool m_clients;

    int seed;
    int time;
public:
    Server(int, int, Terrain&);
    int port;

    //starts the server
    int start();

    //client handler
    void handle_client(int);

    //determines if we can accept clients yet
    bool setup, open;

    //shuts down the server, doesn't quite work yet
    void shutdown();

    //server tick
    void tick();
};

