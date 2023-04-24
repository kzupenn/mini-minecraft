#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "algo/seed.h"
#include "server.h"
#include "scene/runnables.h"
#include <QThreadPool>

using namespace std;


Server::Server(int s) : m_terrain(nullptr), seed(s), setup(false), open(true){
    m_clients.setMaxThreadCount(MAX_CLIENTS);
    ServerConnectionWorker* sw = new ServerConnectionWorker(this);
    QThreadPool::globalInstance()->start(sw);
}

void Server::generateTerrain(int x, int z) {
    //does rendering stuff
    int minx = glm::floor(x/64.f)*64;
    int miny = glm::floor(z/64.f)*64;
    for(int dx = minx-192; dx <= minx+192; dx+=64) {
        for(int dy = miny-192; dy <= miny+192; dy+=64) {
            if(m_terrain.m_generatedTerrain.find(toKey(dx, dy)) == m_terrain.m_generatedTerrain.end()){
                m_terrain.m_generatedTerrain.insert(toKey(dx, dy));
                for(int ddx = dx; ddx < dx + 64; ddx+=16) {
                    for(int ddy = dy; ddy < dy + 64; ddy+=16) {
                        //qDebug() << "creating ground for " << ddx << ddy;
                        m_terrain.createGroundThread(glm::vec2(ddx, ddy));
                    }
                }
            }
        }
    }
}

void Server::handle_client(int client_fd)
{
    QByteArray buffer;
    buffer.resize(BUFFER_SIZE);
    while (open)
    {
        int valread = read(client_fd, buffer.data(), BUFFER_SIZE);
        if (valread == 0)
        {
            // client has disconnected
            cout << "Client " << client_fd << " disconnected" << endl;
            close(client_fd);
            broadcast_packet(mkU<PlayerJoinPacket>(false, client_fd, m_players[client_fd].name).get(), client_fd);
            m_players_mutex.lock();
            m_players.erase(client_fd);
            m_players_mutex.unlock();
            client_fds_mutex.lock();
            client_fds.erase(std::remove(client_fds.begin(), client_fds.end(), client_fd), client_fds.end());
            client_fds_mutex.unlock();
            break;
        }
        else
        {
            //qDebug() << "Client " << client_fd << " says: " << buffer;
            buffer.resize(valread);
            Packet* pp = bufferToPacket(buffer);
            process_packet(pp, client_fd);
            delete(pp);
            buffer.resize(BUFFER_SIZE);
        }
    }
}

void Server::process_packet(Packet* packet, int sender) {
    switch(packet->type) {
    case PLAYER_JOIN: {
        PlayerJoinPacket* thispack = dynamic_cast<PlayerJoinPacket*>(packet);
        m_players_mutex.lock();
        m_players[sender].name = thispack->name;
        m_players_mutex.unlock();
        broadcast_packet(mkU<PlayerJoinPacket>(true, sender, thispack->name).get(), sender);
        break;
    }
    case PLAYER_STATE: {
        PlayerStatePacket* thispack = dynamic_cast<PlayerStatePacket*>(packet);
        m_players_mutex.lock();
        m_players[sender].pos = thispack->player_pos;
        m_players[sender].velo = thispack->player_velo;
        m_players[sender].phi = thispack->player_phi;
        m_players[sender].theta = thispack->player_theta;
        m_players[sender].armor = m_players[sender].calcArmor({thispack->player_helmet, thispack->player_chest, thispack->player_leg, thispack->player_boots});
        m_players[sender].creative = thispack->player_creative;
        if(!thispack->player_creative) {
            //take fall damage
            if(m_players[sender].isFalling && thispack->player_velo.y > -0.01) {
                m_players[sender].isFalling = false;
                int damage = glm::max(0, (int)(m_players[sender].fallHeight-thispack->player_pos.y)-3);
                if(damage>0){
                    m_players[sender].health = glm::max(0, m_players[sender].health-damage);
                    //use a hit packet to simulate fall damage
                    target_packet(mkU<HitPacket>(false, damage, glm::vec3(0, 0.2, 0)).get(), sender);
                    //if player dies, broadcast that they died
                    if(m_players[sender].health == 0) {
                        broadcast_packet(mkU<DeathPacket>(sender, sender).get(), 0);
                    }
                }
            }
            //start falling
            else if(!m_players[sender].isFalling && thispack->player_velo.y <= -0.01) {
                m_players[sender].isFalling = true;
                m_players[sender].fallHeight = thispack->player_pos.y;
            }
        }
        else {
            m_players[sender].isFalling = false;
        }
        m_players_mutex.unlock();

        generateTerrain(thispack->player_pos.x, thispack->player_pos.z);
        broadcast_packet(mkU<PlayerStatePacket>(sender, thispack->player_pos, thispack->player_velo, thispack->player_theta, thispack->player_phi,
                                                thispack->player_hand, thispack->player_helmet, thispack->player_chest, thispack->player_leg, thispack->player_boots, thispack->player_creative).get(), sender);
        break;
    }
    case ITEM_ENTITY_STATE: {
        ItemEntityStatePacket* thispack = dynamic_cast<ItemEntityStatePacket*>(packet);
        m_terrain.item_entities_mutex.lock();
        int foo = m_terrain.item_entity_id++;
        m_terrain.item_entities.insert(std::make_pair(foo, ItemEntity(thispack->pos, Item(thispack->type, thispack->count), nullptr)));
        broadcast_packet(mkU<ItemEntityStatePacket>(foo, thispack->type, thispack->count, thispack->pos).get(), 0);
        m_terrain.item_entities_mutex.unlock();
        break;
    }
    case CHAT: {
        ChatPacket* thispack = dynamic_cast<ChatPacket*>(packet);
        broadcast_packet(mkU<ChatPacket>(sender, thispack->message).get(), sender);
        break;
    }
    case HIT: {
        HitPacket* thispack = dynamic_cast<HitPacket*>(packet);
        m_players_mutex.lock();
        //TO DO: perform grid marching hit detections here
        //TO DO: if another player is hit by the grid march, send a hit packet to the target
        m_players_mutex.unlock();
        break;
    }
    case BLOCK_CHANGE: {
        BlockChangePacket* thispack = dynamic_cast<BlockChangePacket*>(packet);
        glm::vec2 xz = toCoords(thispack->chunkPos);
        this->m_terrain.changeBlockAt(xz.x, thispack->yPos, xz.y, thispack->newBlock);
        broadcast_packet(mkU<BlockChangePacket>(thispack->chunkPos, thispack->yPos, thispack->newBlock).get(), 0);
        break;
    }
    default:
        qDebug() << "unexpected packet type found" << packet->type;
        break;
    }
}

void Server::broadcast_packet(Packet* packet, int exclude) { //use exclude = 0 if you dont want to exclude
    client_fds_mutex.lock();
    for (int i = 0; i < client_fds.size(); i++)
    {
        if (client_fds[i] != exclude)
        {
            QByteArray buffer = packet->packetToBuffer();
            send(client_fds[i], buffer, buffer.size(), 0);
        }
    }
    client_fds_mutex.unlock();
}

void Server::target_packet(Packet* packet, int target) {
    client_fds_mutex.lock();
    QByteArray buffer = packet->packetToBuffer();
    send(target, buffer, buffer.size(), 0);
    client_fds_mutex.unlock();
}

void Server::initClient(int i) {
    std::vector<std::pair<int, QString>> pps;
    client_fds_mutex.lock();
    client_fds.push_back(i);
    m_players_mutex.lock();
    for (map<int, PlayerState>::iterator it = m_players.begin(); it != m_players.end(); it++)
    {
        pps.push_back(std::make_pair(it->first, it->second.name));
    }
    m_players[i] = PlayerState(glm::vec3(0, 80, 0), glm::vec3(), 0.f, 0.f, QString("Player"));
    m_players_mutex.unlock();
    client_fds_mutex.unlock();
    target_packet(mkU<WorldInitPacket>(seed, m_terrain.worldSpawn, pps).get(), i);
    std::vector<std::pair<int64_t, vec3Map>> chunkChangesToSend = m_terrain.getChunkChanges();
    for(std::pair<int64_t, vec3Map> p: chunkChangesToSend) {
        std::vector<std::pair<vec3, BlockType>> ch;
        for(auto& it: p.second){
            ch.push_back(std::make_pair(glm::vec3(it.first), it.second));
        }
        if(ch.size() > 0) {
            target_packet(mkU<ChunkChangePacket>(p.first, ch).get(), i);
        }
    }
}

int Server::start()
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        cout << "Failed to create server socket" << endl;
        return -1;
    }

    // bind server socket to address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (::bind(server_fd, (struct sockaddr*)&address, addrlen) < 0)
    {
        cout << "Failed to bind server socket to address" << endl;
        return -1;
    }

    // listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        cout << "Failed to listen for incoming connections" << endl;
        return -1;
    }

    cout << "Server started listening on port " << PORT << endl;

    //initialize spawn chunks, and select a spawn point
    m_terrain.createSpawn();

    setup = true;

    // wait for incoming connections and handle each one in a separate thread
    while (true)
    {
        int client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_fd < 0)
        {
            cout << "Failed to accept incoming connection" << endl;
            continue;
        }
        else
        {
            // do initial actions
            initClient(client_fd);
            // create a new thread to handle the client
            ServerThreadWorker* stw = new ServerThreadWorker(this, client_fd);
            m_clients.start(stw);
            cout << "New client connected: " << inet_ntoa(address.sin_addr) << endl;
        }
    }

    return 0;
}

void Server::shutdown() {
    open = false;
}

void Server::tick() {
//    std::vector<int> itemsToRemove;
//    for(auto& iter: m_terrain.item_entities) {
//        iter.second.tick();
//        if(iter.second.untouchable_ticks == 0) {
//            for(auto& piter: m_players) {
//                if(glm::distance(piter.second.pos, iter.second.pos) < 1) {

//                }
//            }
//        }
//    }
}

