#include "terrain.h"
#include "algo/worley.h"
#include "scene/biome.h"
#include "scene/structure.h"
#include <stdexcept>
#include <iostream>
#include <QDebug>
#include "runnables.h"
#include <thread>
#include <queue>
#include "algo/noise.h"
#include "algo/seed.h"


#define TEST_RADIUS 256

#define ocean_level 0.4
#define OCEAN_LEVEL 64
#define BEDROCK_LEVEL 32
#define beach_level 0.1

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), mp_context(context), m_generatedTerrain()
{
    //QThreadPool::globalInstance()->setMaxThreadCount(100);
}

Terrain::~Terrain() {

}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}


// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::ivec2 chunkOrigin = glm::ivec2(16*static_cast<int>(glm::floor(x / 16.f)),
                                            16*static_cast<int>(glm::floor(z / 16.f)));
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}


BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    m_chunks_mutex.lock();
    bool b= m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
    //qDebug() << x << z << b;
    m_chunks_mutex.unlock();
    return b;
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    m_chunks_mutex.lock();
    uPtr<Chunk>& c = m_chunks[toKey(16 * xFloor, 16 * zFloor)];
    m_chunks_mutex.unlock();
    return c;
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::ivec2 chunkOrigin = glm::ivec2(16*static_cast<int>(glm::floor(x / 16.f)),
                                            16*static_cast<int>(glm::floor(z / 16.f)));
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        int xFloor = static_cast<int>(glm::floor(x / 16.f));
        int zFloor = static_cast<int>(glm::floor(z / 16.f));
        int64_t key = toKey(16 * xFloor, 16 * zFloor);
        metaData_mutex.lock();
        if(metaData.find(key) == metaData.end()){
            metaData[key] = std::vector<metadata>();
        }
        metaData[key].emplace_back(t, glm::vec3(x-16*xFloor, y, z-16*zFloor));
        metaData_mutex.unlock();
//        throw std::out_of_range("Coordinates " + std::to_string(x) +
//                                " " + std::to_string(y) + " " +
//                                std::to_string(z) + " have no Chunk!");
    }
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t, bool(*con)(int,int,int,Chunk*)) {
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        int xFloor = 16*static_cast<int>(glm::floor(x / 16.f));
        int zFloor = 16*static_cast<int>(glm::floor(z / 16.f));
        if(con(x-xFloor, y, z-zFloor, c.get())){
            c->setBlockAt(static_cast<unsigned int>(x - xFloor),
                          static_cast<unsigned int>(y),
                          static_cast<unsigned int>(z - zFloor),
                          t);
        }
    }
    else {
        int xFloor = static_cast<int>(glm::floor(x / 16.f));
        int zFloor = static_cast<int>(glm::floor(z / 16.f));
        int64_t key = toKey(16 * xFloor, 16 * zFloor);
        metaData_mutex.lock();
        if(metaData.find(key) == metaData.end()){
            metaData[key] = std::vector<metadata>();
        }
        metaData[key].emplace_back(t, glm::vec3(x-16*xFloor, y, z-16*zFloor), con);
        metaData_mutex.unlock();
//        throw std::out_of_range("Coordinates " + std::to_string(x) +
//                                " " + std::to_string(y) + " " +
//                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    //semaphore blocking to limit thread count
    x = floor(x/16.f)*16;
    z = floor(z/16.f)*16;

    int64_t key = toKey(x, z);

    uPtr<Chunk> chunk = mkU<Chunk>(mp_context);
    Chunk *cPtr = chunk.get();

    //biome info to generate with blocktype later
    BiomeType biomeMap[16][16];

    //terrain initialization
    for(int xx = x; xx < x+16; xx++) {
        for(int zz = z; zz < z+16; zz++) {
            float bedrock = generateBedrock(glm::vec2(xx,zz));
            float beachhead = beach_level*generateBeach(glm::vec2(xx,zz));
            std::pair<float, BiomeType> groundInfo = generateGround(glm::vec2(xx,zz));

            //deep ocean
            if(bedrock < ocean_level/2) {
                //use center of chunk as the biome of the chunk
                if(xx == x+8 && zz == z+8) {
                    cPtr->biome = OCEAN;
                }
                //height
                cPtr->heightMap[xx-x][zz-z] = OCEAN_LEVEL;
                biomeMap[xx-x][zz-z] = OCEAN;
            }
            //shallow ocean
            else if(bedrock < ocean_level) {
                //use center of chunk as the biome of the chunk
                if(xx == x+8 && zz == z+8) {
                    cPtr->biome = OCEAN;
                }
                //height
                cPtr->heightMap[xx-x][zz-z] = OCEAN_LEVEL;
                biomeMap[xx-x][zz-z] = OCEAN;
            }
            //beach
            else if(bedrock < ocean_level+beachhead) {
                //use center of chunk as the biome of the chunk
                if(xx == x+8 && zz == z+8) {
                    cPtr->biome = BEACH;
                }
                //height
                //float erosion = generateErosion(vec2(xx,zz));
                //shoreline
                float height = glm::clamp((int)(OCEAN_LEVEL + pow((bedrock-ocean_level)/beachhead,2)*(groundInfo.first+(bedrock-ocean_level)*BEDROCK_LEVEL)),
                                     0, 256);
                cPtr->heightMap[xx-x][zz-z] = height;
                if(height <= OCEAN_LEVEL+5 && groundInfo.second != RIVER){
                    biomeMap[xx-x][zz-z] = BEACH;
                }
                else {
                    biomeMap[xx-x][zz-z] = groundInfo.second;
                }
            }
            //land
            else {
                //use center of chunk as the biome of the chunk
                if(xx == x+8 && zz == z+8) {
                    cPtr->biome = groundInfo.second;
                }
                //height
                float height = glm::clamp((int)(OCEAN_LEVEL + groundInfo.first +(bedrock-ocean_level)*BEDROCK_LEVEL), 0, 256);
                cPtr->heightMap[xx-x][zz-z] = height;
                biomeMap[xx-x][zz-z] = groundInfo.second;
            }
        }
    }

    //using height and biome map, generate chunk
    //TO DO: add ocean floor and river bed, do swamp somehow
    for(int xx = 0; xx < 16; xx++) {
        for(int zz = 0; zz < 16; zz++) {
            switch(biomeMap[xx][zz]) {
            case TUNDRA: {
                int maxy = cPtr->heightMap[xx][zz]-1;
                int y = maxy;
                for(; y > maxy-3; y--) cPtr->setBlockAt(xx, y, zz, SNOW);
                for(; y > maxy-6; y--) cPtr->setBlockAt(xx, y, zz, DIRT);
                for(; y >= 0; y--) cPtr->setBlockAt(xx, y, zz, STONE);
                break;
            }
            case PLAINS: {
                int maxy = cPtr->heightMap[xx][zz]-1;
                int y = maxy;
                if(y > generateSnowLayer(glm::vec2(xx+x, zz+z))) cPtr->setBlockAt(xx, y, zz, SNOW);
                else if(y>generateRockLayer(glm::vec2(xx+x, zz+z))) cPtr->setBlockAt(xx, y, zz, STONE);
                else cPtr->setBlockAt(xx, y, zz, GRASS);
                y--;
                if(y < 100) for(; y > maxy-4; y--) cPtr->setBlockAt(xx, y, zz, DIRT);
                for(; y >= 0; y--) cPtr->setBlockAt(xx, y, zz, STONE);
                break;
            }
            case DESERT:{
                int maxy = cPtr->heightMap[xx][zz]-1;
                int y = maxy;
                for(; y > maxy-3; y--) cPtr->setBlockAt(xx, y, zz, SAND);
                for(; y > maxy-8; y--) cPtr->setBlockAt(xx, y, zz, SANDSTONE);
                for(; y >= 0; y--) cPtr->setBlockAt(xx, y, zz, STONE);
                break;
            }
            case TAIGA: {
                int maxy = cPtr->heightMap[xx][zz]-1;
                int y = maxy;
                if(y > generateSnowLayer(glm::vec2(xx+x, zz+z))) cPtr->setBlockAt(xx, y, zz, SNOW);
                else if(y>generateRockLayer(glm::vec2(xx+x, zz+z))) cPtr->setBlockAt(xx, y, zz, STONE);
                else cPtr->setBlockAt(xx, y, zz, GRASS);
                y--;
                if(y < 100) for(; y > maxy-4; y--) cPtr->setBlockAt(xx, y, zz, DIRT);
                for(; y >= 0; y--) cPtr->setBlockAt(xx, y, zz, STONE);
                break;
            }
            case FOREST: {
                int maxy = cPtr->heightMap[xx][zz]-1;
                int y = maxy;
                if(y > generateSnowLayer(glm::vec2(xx+x, zz+z))) cPtr->setBlockAt(xx, y, zz, SNOW);
                else if(y>generateRockLayer(glm::vec2(xx+x, zz+z))) cPtr->setBlockAt(xx, y, zz, STONE);
                else cPtr->setBlockAt(xx, y, zz, GRASS);
                y--;
                for(; y > maxy-4; y--) cPtr->setBlockAt(xx, y, zz, DIRT);
                for(; y >= 0; y--) cPtr->setBlockAt(xx, y, zz, STONE);
                break;
            }
            case SAVANNA: {
                int maxy = cPtr->heightMap[xx][zz]-1;
                int y = maxy;
                if(y > generateSnowLayer(glm::vec2(xx+x, zz+z))) cPtr->setBlockAt(xx, y, zz, SNOW);
                else if(y>generateRockLayer(glm::vec2(xx+x, zz+z))) cPtr->setBlockAt(xx, y, zz, STONE);
                else cPtr->setBlockAt(xx, y, zz, GRASS);
                y--;
                if(y < 100) for(; y > maxy-4; y--) cPtr->setBlockAt(xx, y, zz, DIRT);
                for(; y >= 0; y--) cPtr->setBlockAt(xx, y, zz, STONE);
                break;
            }
            case RAINFOREST: {
                int maxy = cPtr->heightMap[xx][zz]-1;
                int y = maxy;
                if(y > generateSnowLayer(glm::vec2(xx+x, zz+z))) cPtr->setBlockAt(xx, y, zz, SNOW);
                else if(y>generateRockLayer(glm::vec2(xx+x, zz+z))) cPtr->setBlockAt(xx, y, zz, STONE);
                else cPtr->setBlockAt(xx, y, zz, GRASS);
                y--;
                if(y < 100) for(; y > maxy-4; y--) cPtr->setBlockAt(xx, y, zz, DIRT);
                for(; y >= 0; y--) cPtr->setBlockAt(xx, y, zz, STONE);
                break;
            }
            case BEACH:{
                int maxy = cPtr->heightMap[xx][zz]-1;
                int y = maxy;
                for(; y > maxy-6; y--) cPtr->setBlockAt(xx, y, zz, SAND);
                for(; y > maxy-12; y--) cPtr->setBlockAt(xx, y, zz, DIRT);
                for(; y >= 0; y--) cPtr->setBlockAt(xx, y, zz, STONE);
                break;
            }
            case OCEAN:{
                int maxy = cPtr->heightMap[xx][zz]-1;
                int y = maxy;
                for(; y >= 0; y--) cPtr->setBlockAt(xx, y, zz, WATER);
                break;
            }
            case RIVER: {
                int maxy = cPtr->heightMap[xx][zz]-1;
                int y = maxy;
                for(; y >= 0; y--) cPtr->setBlockAt(xx, y, zz, WATER);
                break;
            }
            default:{
                for(int y = 0; y < cPtr->heightMap[xx][zz]; y++)
                    cPtr->setBlockAt(xx, y, zz, GRASS);
                break;
            }
            }
        }
    }

    //inserts chunk into m_chunks
    //meta data will no longer be added and changes will instead be directly made to the chunk
    m_chunks_mutex.lock();
    m_chunks[toKey(x, z)] = move(chunk);
    m_chunks_mutex.unlock();

    std::vector<Structure> chunkStructures = getStructureZones(cPtr, x, z);

    //checks if metasubstructures has structures for this chunk and adds them to the list if necessary
    metaSubStructures_mutex.lock();
    if(metaSubStructures.find(toKey(x, z))!=metaSubStructures.end()){
        chunkStructures.insert(chunkStructures.end(), metaSubStructures[toKey(x, z)].begin(), metaSubStructures[toKey(x, z)].end());
        metaSubStructures.erase(toKey(x, z)); //remove from memory to save space once added to the chunk structure list
    }

    metaSubStructures_mutex.unlock();
    //checks and generates metaStructures
    for(std::pair<std::pair<int64_t, int>, StructureType> metaS: getMetaStructures(glm::vec2(x, z))){
        metaStructures_mutex.lock();
        if(metaStructures.find(metaS.first) == metaStructures.end()){
            metaStructures.insert(metaS);
            //unlock the map so other threads can use it after marking this one as generating
            metaStructures_mutex.unlock();
            StructureWorker* sw = new StructureWorker(this, metaS.second, toCoords(metaS.first.first).x, metaS.first.second, toCoords(metaS.first.first).y);
            QThreadPool::globalInstance()->start(sw);
        }
        else{
            metaStructures_mutex.unlock();
        }
    }

    //generates structures
    for(const Structure &s: chunkStructures){
        buildStructure(s);
    }

    //checks for meta data
    metaData_mutex.lock();
    if(metaData.find(key) != metaData.end()) {
        for(metadata md: metaData[key]){
            if(md.con == nullptr || md.con(md.pos.x, md.pos.y, md.pos.z, cPtr))
                cPtr->setBlockAt(md.pos.x, md.pos.y, md.pos.z, md.type);
        }
    }
    metaData.erase(key);
    metaData_mutex.unlock();


    createVBOThread(cPtr);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }


    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            if(hasChunkAt(x, z)){
                uPtr<Chunk> &chunk = getChunkAt(x, z);
                //only renders chunks with generated terrain data
                if(chunk->dataGen){
                    //since only main thread can add
                    if(!chunk->dataBound){
                        chunk->bindVBOdata();
                    }
                    shaderProgram->setModelMatrix(glm::translate(glm::mat4(1.f), glm::vec3(x, 0, z)));
                    shaderProgram->drawInterleaved(*chunk.get());
                }
            }
            else {
                //qDebug() << "missing chunk at " << x << z;
            }
        }
    }
    //check if we should clear unloaded chunk vbos
    for(int x = minX - 32; x < maxX + 32; x+= 16) {
        if(hasChunkAt(x, minZ - 32)) {
            uPtr<Chunk> &chunk = getChunkAt(x, minZ - 32);
            if(chunk->dataGen && chunk->dataBound) {
                chunk->unbindVBOdata();
            }
        }
        if(hasChunkAt(x, maxZ+16)) { //not that we don't actually hit maxZ in the drawloop
            uPtr<Chunk> &chunk = getChunkAt(x, maxZ+16);
            if(chunk->dataGen && chunk->dataBound) {
                chunk->unbindVBOdata();
            }
        }
    }
    //dont need to recheck corners, lower bounds by 1 chunk
    for(int z = minZ - 16; z < maxZ+16; z+= 16) {
        if(hasChunkAt(minX-32, z)) {
            uPtr<Chunk> &chunk = getChunkAt(minX-32, z);
            if(chunk->dataGen && chunk->dataBound) {
                chunk->unbindVBOdata();
            }
        }
        if(hasChunkAt(maxX+16, z)) { //not that we don't actually hit maxZ in the drawloop
            uPtr<Chunk> &chunk = getChunkAt(maxX+16, z);
            if(chunk->dataGen && chunk->dataBound) {
                chunk->unbindVBOdata();
            }
        }
    }
}

void Terrain::createInitScene()
{
    // mark one semaphor immediately to signal start
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    // We'll do this part somewhat synchronously since doing it async will result in 500+ threads and crash

    for(int dx = -256; dx <= 256; dx+=64) {
        for(int dy = -256; dy <= 256; dy+=64) {
            m_generatedTerrain.insert(toKey(dx, dy));
            for(int ddx = dx; ddx < dx + 64; ddx+=16) {
                for(int ddy = dy; ddy < dy + 64; ddy+=16) {
                    createGroundThread(glm::vec2(ddx, ddy));
                }
            }
        }
    }
}

void Terrain::createGroundThread(glm::vec2 p) {
    if(hasChunkAt(p.x, p.y)) return;

    BlockTypeWorker* btw = new BlockTypeWorker(this, p.x, p.y);
    QThreadPool::globalInstance()->start(btw);
}

void Terrain::createVBOThread(Chunk* c) {
    VBOWorker* vw = new VBOWorker(c);
    QThreadPool::globalInstance()->start(vw);
}

void Terrain::processMegaStructure(const std::vector<Structure>& s) {
    for(const Structure &st: s) {
        if(hasChunkAt(st.pos.x, st.pos.y)) {
            buildStructure(st);
        }
        else{
            int x = 16*static_cast<int>(glm::floor(st.pos.x / 16.f));
            int z = 16*static_cast<int>(glm::floor(st.pos.y / 16.f));
            metaSubStructures_mutex.lock();
            if(metaSubStructures.find(toKey(x, z)) == metaSubStructures.end()) {
                metaSubStructures[toKey(x, z)] = std::vector<Structure>();
            }
            metaSubStructures[toKey(x, z)].emplace_back(st);
            metaSubStructures_mutex.unlock();
        }
    }
}

void Terrain::buildStructure(const Structure& s) {
    int xx = s.pos.x;
    int zz = s.pos.y;

    Chunk* c = getChunkAt(xx, zz).get();
    glm::ivec2 chunkOrigin = glm::ivec2(16*static_cast<int>(glm::floor(xx / 16.f)),
                                        16*static_cast<int>(glm::floor(zz / 16.f)));
    int x = chunkOrigin.x;
    int z = chunkOrigin.y;

    switch(s.type){
    case OAK_TREE: {
        //how tall the tree is off the ground
        //TO DO: replace GRASS with LEAVES block once implemented
        //TO DO: replace DIRT with WOOD block once implemented
        int ymax = 6+3.f*noise1D(glm::vec2(xx, zz), SEED.getSeed(8654.512,8568.53,3163.562));
        //find base of tree
        int ymin = c->heightMap[xx-x][zz-z];
        for(int dy = 0; dy < 4; dy++) {
            int yat = ymin+ymax-dy;
            switch(dy) {
                case 0:
                    setBlockAt(xx, yat, zz, OAK_LEAVES, isEmpty);
                    setBlockAt(xx-1, yat, zz, OAK_LEAVES, isEmpty);
                    setBlockAt(xx+1, yat, zz, OAK_LEAVES, isEmpty);
                    setBlockAt(xx, yat, zz-1, OAK_LEAVES, isEmpty);
                    setBlockAt(xx, yat, zz+1, OAK_LEAVES, isEmpty);
                    break;
                case 1:
                    setBlockAt(xx, yat, zz, OAK_LEAVES, isEmpty);
                    setBlockAt(xx-1, yat, zz, OAK_LEAVES, isEmpty);
                    setBlockAt(xx+1, yat, zz, OAK_LEAVES, isEmpty);
                    setBlockAt(xx, yat, zz-1, OAK_LEAVES, isEmpty);
                    setBlockAt(xx, yat, zz+1, OAK_LEAVES, isEmpty);
                    if(noise1D(glm::vec3(xx+1, yat, zz+1), SEED.getSeed(7785.015,5766.378,649.792,6102.897)) > 0.5) {
                        setBlockAt(xx+1, yat, zz+1, OAK_LEAVES);
                    }
                    if(noise1D(glm::vec3(xx+1, yat, zz-1), SEED.getSeed(1420.159,7503.537,1373.417,2979.007)) > 0.5) {
                        setBlockAt(xx+1, yat, zz-1, OAK_LEAVES, isEmpty);
                    }
                    if(noise1D(glm::vec3(xx-1, yat, zz+1), SEED.getSeed(464.713,1450.085,4383.409,6818.919)) > 0.5) {
                        setBlockAt(xx-1, yat, zz+1, OAK_LEAVES, isEmpty);
                    }
                    if(noise1D(glm::vec3(xx-1, yat, zz-1), SEED.getSeed(8513.165,8543.726,1277.831,9162.371)) > 0.5) {
                        setBlockAt(xx-1, yat, zz-1, OAK_LEAVES, isEmpty);
                    }
                    break;
                default: //2, 3
                    for(int dx = xx-2; dx <= xx+2; dx++) {
                        for(int dz = zz-1; dz <= zz+1; dz++) {
                            if(dx != xx || dz != zz) {
                                setBlockAt(dx, yat, dz, OAK_LEAVES, isEmpty);
                            }
                        }
                    }
                    setBlockAt(xx-1, yat, zz+2, OAK_LEAVES, isEmpty);
                    setBlockAt(xx, yat, zz+2, OAK_LEAVES, isEmpty);
                    setBlockAt(xx+1, yat, zz+2, OAK_LEAVES, isEmpty);
                    setBlockAt(xx-1, yat, zz-2, OAK_LEAVES, isEmpty);
                    setBlockAt(xx, yat, zz-2, OAK_LEAVES);
                    setBlockAt(xx+1, yat, zz-2, OAK_LEAVES, isEmpty);
                    if(noise1D(glm::vec3(xx+2, yat, zz+2), SEED.getSeed(7798.159,7306.237,4491.404,966.212)) > 0.5) {
                        setBlockAt(xx+2, yat, zz+2, OAK_LEAVES, isEmpty);
                    }
                    if(noise1D(glm::vec3(xx+2, yat, zz-2), SEED.getSeed(3953.665,7624.82,5599.103,4681.367)) > 0.5) {
                        setBlockAt(xx+2, yat, zz-2, OAK_LEAVES, isEmpty);
                    }
                    if(noise1D(glm::vec3(xx-2, yat, zz+2), SEED.getSeed(431.931,9230.515,2698.152,3252.572)) > 0.5) {
                        setBlockAt(xx-2, yat, zz+2, OAK_LEAVES, isEmpty);
                    }
                    if(noise1D(glm::vec3(xx-2, yat, zz-2), SEED.getSeed(2799.543,9511.908,2472.754,4812.237)) > 0.5) {
                        setBlockAt(xx-2, yat, zz-2, OAK_LEAVES, isEmpty);
                    }
                    break;
            }
        }
        for(int y = ymin; y < ymin+ymax; y++){
            setBlockAt(xx, y, zz, OAK_LOG);
        }
        break;
    }
    case FANCY_OAK_TREE:{
        int ymin = c->heightMap[xx-x][zz-z];
        break;
    }
        //creates a spruce tree
        //TO DO: replace block types with appropriate leaves and wood
    case SPRUCE_TREE:{
        int ymin = c->heightMap[xx-x][zz-z];
        int ymax = 5+7*noise1D(glm::vec2(xx,zz), SEED.getSeed(9606.874,301.036,378.273));
        float leaves = 1;
        setBlockAt(xx, ymax, zz, DIRT);
        for(int y = ymax+ymin-1; y > ymax; y--) {
            float transition = noise1D(glm::vec3(xx, y, zz), SEED.getSeed(7656.579,4083.936,4656.875,8280.13));
            if(leaves == 0){
                leaves++;
            }
            else if(leaves == 1) { //radius 1
                setBlockAt(xx-1, y, zz, GRASS);
                setBlockAt(xx+1, y, zz, GRASS);
                setBlockAt(xx, y, zz-1, GRASS);
                setBlockAt(xx, y, zz+1, GRASS);
                if(transition < 0.3) leaves--;
                else leaves++;
            }
            else if(leaves == 2) { //radius 2
                for(int xxx = xx-2; xxx <= xx+2; xxx++) {
                    for(int zzz = zz-2; zzz <= zz+2; zzz++) {
                        if(abs(xxx-xx)+abs(zzz-zz) != 4)
                            setBlockAt(xxx, y, zzz, GRASS);
                    }
                }
                if(transition<0.7) leaves--;
                else leaves++;
            }
            else{ //radius 3
                for(int xxx = xx-3; xxx <= xx+3; xxx++) {
                    for(int zzz = zz-3; zzz <= zz+3; zzz++) {
                        if(abs(xxx-xx)+abs(zzz-zz) != 6)
                            setBlockAt(xxx, y, zzz, GRASS);
                    }
                }
                leaves--;
            }
            setBlockAt(xx, y, zz, DIRT);
        }
        setBlockAt(xx, ymax+ymin, zz, GRASS);
        break;
    }
    case VILLAGE_CENTER:
        for(int i = -1; i <= 1; i++) {
            for(int j = -1; j <= 1; j++) {
                setBlockAt(xx+i, 1000, zz+j, STONE);
            }
        }
        for(int i = -5; i <= 5; i++) {
            for(int j = -5; j <= 5; j++) {
                float f = noise1D(glm::vec2(xx+i, zz+j), SEED.getSeed(57091, 850135, 323));
                if(f < 0.33)
                    setBlockAt(xx+i, 1000-1, zz+j, PATH);
                else if(f < 0.66)
                    setBlockAt(xx+i, 1000-1, zz+j, STONE);
                else
                    setBlockAt(xx+i, 1000-1, zz+j, GRASS);
            }
        }
        break;
    case VILLAGE_ROAD:{
        glm::vec2 perp = glm::vec2(dirToVec(s.orient).z, dirToVec(s.orient).x);
        if(c->getBlockAt(xx-x, c->heightMap[xx-x][zz-z]-1, zz-z) == WATER) {
            for(int i = -1; i <= 1; i++) {
                setBlockAt(xx+i*perp.x, 1000-1, zz+i*perp.y, OAK_PLANKS);
            }
        }
        else{
            for(int i = -1; i <= 1; i++) {
                setBlockAt(xx+i*perp.x, 1000-1, zz+i*perp.y, PATH);
            }
        }
        break;
    }
    case VILLAGE_HOUSE_1: {
        int floorh = c->heightMap[xx-x][zz-z];
        glm::vec2 perp = glm::vec2(-dirToVec(s.orient).z, dirToVec(s.orient).x);
        glm::vec2 back = glm::vec2(-perp.y, perp.x);
        glm::vec2 pp;
        //clear the area, base
        for(int i = -1; i <= 5; i++) {
            for(int j = -3; j <= 3; j++) {
                for(int y = 0; y < 8; y++) {
                    pp = glm::vec2(xx, zz) + perp*(float)j + back*(float)i;
                    setBlockAt(pp.x, floorh+y, pp.y, EMPTY);
                }
            }
        }
        for(int i = -1; i <= 5; i++) {
            for(int j = -3; j <= 3; j++) {
                pp = glm::vec2(xx, zz) + perp*(float)j + back*(float)i;
                setBlockAt(pp.x, floorh-1, pp.y, DIRT, isTransparent);
            }
        }
        for(int i = 0; i <= 4; i++) {
            for(int j = -2; j <= 2; j++) {
                pp = glm::vec2(xx, zz) + perp*(float)j + back*(float)i;
                setBlockAt(pp.x, floorh-2, pp.y, DIRT, isTransparent);
            }
        }
        for(int i = 1; i <= 3; i++) {
            for(int j = -1; j <= 1; j++) {
                pp = glm::vec2(xx, zz) + perp*(float)j + back*(float)i;
                setBlockAt(pp.x, floorh-3, pp.y, DIRT, isTransparent);
            }
        }
        //floor
        for(int i = -1; i <= 1; i++) {
            for(int j = 1; j <= 3; j++) {
                pp = glm::vec2(xx, zz) + perp*(float)i + back*(float)j;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
        }
        //pillars
        for(int i = -2; i <= 2; i+= 4){
            for(int j = 0; j <= 4; j+= 4){
                for(int y = 0; y < 4; y++) {
                    pp = glm::vec2(xx, zz) + perp*(float)i + back*(float)j;
                    setBlockAt(pp.x, floorh+y, pp.y, OAK_LOG);
                }
            }
        }
        //walls
        for(int i = -2; i <= 2; i+= 4) {
            for(int j = 1; j<=3; j++) {
                for(int y = 0; y < 4; y++){
                    pp = glm::vec2(xx, zz) + perp*(float)i + back*(float)j;
                    setBlockAt(pp.x, floorh+y, pp.y, COBBLESTONE);
                }
            }
        }
        for(int i = -1; i <= 1; i++) {
            for(int j = 0; j <= 4; j+= 4) {
                for(int y = 0; y < 4; y++){
                    pp = glm::vec2(xx, zz) + perp*(float)i + back*(float)j;
                    setBlockAt(pp.x, floorh+y, pp.y, COBBLESTONE);
                }
            }
        }
        //carve out windows+door
        pp = glm::vec2(xx, zz) - perp*2.f + back*2.f;
        setBlockAt(pp.x, floorh+2, pp.y, GLASS);
        pp = glm::vec2(xx, zz) + perp*2.f + back*2.f;
        setBlockAt(pp.x, floorh+2, pp.y, GLASS);
        pp = glm::vec2(xx, zz) + back*4.f;
        setBlockAt(pp.x, floorh+2, pp.y, GLASS);

        setBlockAt(xx, floorh+1, zz, EMPTY);
        setBlockAt(xx, floorh+2, zz, EMPTY);

        //roof
        for(int i = -3; i <= 3; i++) {
            setBlockAt(xx+i+2.f*back.x, floorh+4, zz+3+2.f*back.y, OAK_PLANKS);
            setBlockAt(xx+i+2.f*back.x, floorh+4, zz-3+2.f*back.y, OAK_PLANKS);
            setBlockAt(xx+3+2.f*back.x, floorh+4, zz+i+2.f*back.y, OAK_PLANKS);
            setBlockAt(xx-3+2.f*back.x, floorh+4, zz+i+2.f*back.y, OAK_PLANKS);
        }
        for(int i = -2; i <= 2; i++) {
            for(int j = 0; j < 2; j++){
                setBlockAt(xx+i+2.f*back.x, floorh+4+j, zz+2+2.f*back.y, OAK_PLANKS);
                setBlockAt(xx+i+2.f*back.x, floorh+4+j, zz-2+2.f*back.y, OAK_PLANKS);
                setBlockAt(xx+2+2.f*back.x, floorh+4+j, zz+i+2.f*back.y, OAK_PLANKS);
                setBlockAt(xx-2+2.f*back.x, floorh+4+j, zz+i+2.f*back.y, OAK_PLANKS);
            }
        }
        for(int i = -1; i <= 1; i++) {
            for(int j = 0; j < 2; j++){
                setBlockAt(xx+i+2.f*back.x, floorh+5+j, zz+1+2.f*back.y, OAK_PLANKS);
                setBlockAt(xx+i+2.f*back.x, floorh+5+j, zz-1+2.f*back.y, OAK_PLANKS);
                setBlockAt(xx+1+2.f*back.x, floorh+5+j, zz+i+2.f*back.y, OAK_PLANKS);
                setBlockAt(xx-1+2.f*back.x, floorh+5+j, zz+i+2.f*back.y, OAK_PLANKS);
            }
        }
        setBlockAt(xx+2.f*back.x, floorh+7, zz+2.f*back.y, OAK_PLANKS);
        break;
    }
    case VILLAGE_LIBRARY: {
        int floorh = c->heightMap[xx-x][zz-z];
        glm::vec2 perp = glm::vec2(-dirToVec(s.orient).z, dirToVec(s.orient).x);
        glm::vec2 back = glm::vec2(-perp.y, perp.x);
        glm::vec2 pp;
        //clearing and setting ground
        for(int i = -8; i <= 8; i++) {
            for(int j = -1; j <= 9; j++) {
                for(int y = 0; y < 10; y++){
                     pp = glm::vec2(xx, zz) + perp*(float)i+back*(float)j;
                    setBlockAt(pp.x, floorh+y, pp.y, EMPTY);
                }
            }
        }
        for(int y = 0; y <= 2; y++) {
            for(int i = -8+y; i <= 8-y; i++) {
                for(int j = -1+y; j <= 9-y; j++) {
                    pp = glm::vec2(xx, zz) + perp*(float)i+back*(float)j;
                    setBlockAt(pp.x, floorh-y-1, pp.y, DIRT, isTransparent);
                }
            }
        }
        //layer 1
        for(int i = -1; i <= 1; i++) {
            pp = glm::vec2(xx, zz) + perp*(float)i;
            setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        }
        for(int i = -2; i <= 2; i++) {
            pp = glm::vec2(xx, zz) + perp*(float)i+back;
            setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        }
        for(int i = 2; i <= 8; i++) {
            for(int j = -7; j <= 7; j++) {
                pp = glm::vec2(xx, zz) + perp*(float)j+back*(float)i;
                setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
            }
        }
        //layer 2
        floorh++;
        pp = glm::vec2(xx, zz) + perp;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        pp = glm::vec2(xx, zz) - perp;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        pp = glm::vec2(xx, zz) + 2.f*perp + back;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        pp = glm::vec2(xx, zz) - 2.f*perp + back;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        pp = glm::vec2(xx, zz) - perp*2.f + 4.f*back;
        setBlockAt(pp.x, floorh, pp.y, BOOKSHELF);
        pp = glm::vec2(xx, zz) + perp*2.f + 4.f*back;
        setBlockAt(pp.x, floorh, pp.y, BOOKSHELF);
        for(int i = 3; i <= 7; i++){
            pp = glm::vec2(xx, zz) - perp*(float)i + 2.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            pp = glm::vec2(xx, zz) + perp*(float)i + 2.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        }
        for(int i = 2; i <= 8; i++) {
            pp = glm::vec2(xx, zz) - perp*7.f + back*(float)i;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            pp = glm::vec2(xx, zz) + perp*7.f + back*(float)i;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        }
        for(int i = -7; i <= 7; i++){
            pp = glm::vec2(xx, zz) - perp*(float)i + 8.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        }
        for(int i = 3; i <= 5; i++){
            pp = glm::vec2(xx, zz) - perp*(float)i + 4.f*back;
            setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
            pp = glm::vec2(xx, zz) + perp*(float)i + 4.f*back;
            setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        }
        for(int i = 1; i <= 3; i++){
            pp = glm::vec2(xx, zz) - perp*(float)i + 7.f*back;
            setBlockAt(pp.x, floorh, pp.y, BOOKSHELF);
            pp = glm::vec2(xx, zz) + perp*(float)i + 7.f*back;
            setBlockAt(pp.x, floorh, pp.y, BOOKSHELF);
        }
        for(int i = 4; i <= 6; i++){
            pp = glm::vec2(xx, zz) - perp*(float)i + 7.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            pp = glm::vec2(xx, zz) + perp*(float)i + 7.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        }
        pp = glm::vec2(xx, zz) - perp*6.f + 6.f*back;
        setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        pp = glm::vec2(xx, zz) + perp*6.f + 6.f*back;
        setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        //layer 3
        floorh++;
        pp = glm::vec2(xx, zz) + perp;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        pp = glm::vec2(xx, zz) - perp;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        pp = glm::vec2(xx, zz) + 2.f*perp + back;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        pp = glm::vec2(xx, zz) - 2.f*perp + back;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        for(int i = -1; i <= 1; i+= 2) {
            pp = glm::vec2(xx, zz) + 3.f*perp*(float)i + 2.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_LOG);
            pp = glm::vec2(xx, zz) + 4.f*perp*(float)i + 2.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            pp = glm::vec2(xx, zz) + 5.f*perp*(float)i + 2.f*back;
            setBlockAt(pp.x, floorh, pp.y, GLASS);
            pp = glm::vec2(xx, zz) + 6.f*perp*(float)i + 2.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        }
        for(int i = -1; i <= 1; i+= 2) {
            pp = glm::vec2(xx, zz) + 7.f*perp*(float)i + 3.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            pp = glm::vec2(xx, zz) + 7.f*perp*(float)i + 4.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_LOG);
            pp = glm::vec2(xx, zz) + 7.f*perp*(float)i + 5.f*back;
            setBlockAt(pp.x, floorh, pp.y, GLASS);
            pp = glm::vec2(xx, zz) + 7.f*perp*(float)i + 6.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_LOG);
            pp = glm::vec2(xx, zz) + 7.f*perp*(float)i + 7.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        }
        for(int i = -1; i <= 1; i+= 2) {
            pp = glm::vec2(xx, zz) + 1.f*perp*(float)i + 8.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_LOG);
            pp = glm::vec2(xx, zz) + 2.f*perp*(float)i + 8.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            pp = glm::vec2(xx, zz) + 3.f*perp*(float)i + 8.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_LOG);
            pp = glm::vec2(xx, zz) + 4.f*perp*(float)i + 8.f*back;
            setBlockAt(pp.x, floorh, pp.y, GLASS);
            pp = glm::vec2(xx, zz) + 5.f*perp*(float)i + 8.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_LOG);
            pp = glm::vec2(xx, zz) + 6.f*perp*(float)i + 8.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            pp = glm::vec2(xx, zz) + 2.f*perp*(float)i + 7.f*back;
            setBlockAt(pp.x, floorh, pp.y, BOOKSHELF);
        }
        pp = glm::vec2(xx, zz) + 8.f*back;
        setBlockAt(pp.x, floorh, pp.y, GLASS);
        for(int i = -1; i <= 1; i+= 2) {
            pp = glm::vec2(xx, zz) + perp*3.f*(float)i + 4.f*back;
            setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
            pp = glm::vec2(xx, zz) + perp*4.f*(float)i + 4.f*back;
            setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        }
        //layer 4
        floorh++;
        setBlockAt(xx, floorh, zz, COBBLESTONE);
        pp = glm::vec2(xx, zz) + perp;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        pp = glm::vec2(xx, zz) - perp;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        pp = glm::vec2(xx, zz) + 2.f*perp + back;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        pp = glm::vec2(xx, zz) - 2.f*perp + back;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        for(int i = -1; i <= 1; i++) {
            pp = glm::vec2(xx, zz) + perp*(float)i + back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        }
        for(int i = -6; i <= 6; i++) {
            pp = glm::vec2(xx, zz) + perp*(float)i + 2.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            pp = glm::vec2(xx, zz) + perp*(float)i + 8.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        }
        for(int i = -1; i <= 1; i+= 2) {
            for(int j = 3; j <= 7; j++) {
                pp = glm::vec2(xx, zz) + 7.f*perp*(float)i + back*(float)j;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
        }
        for(int i = -2; i <= 2; i++) {
            for(int j = 3; j <= 4; j++) {
                pp = glm::vec2(xx, zz) + perp*(float)i + back*(float)j;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
        }
        pp = glm::vec2(xx, zz) - 3.f*perp + 4.f*back;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        pp = glm::vec2(xx, zz) + 3.f*perp + 4.f*back;
        setBlockAt(pp.x, floorh, pp.y, COBBLESTONE);
        //layer 5
        floorh++;
        for(int i = 3; i <= 6; i++){
            for(int j = -1; j <= 1; j+= 2){
                pp = glm::vec2(xx, zz) + perp*(float)(i*j) + 2.f*back;
                setBlockAt(pp.x, floorh, pp.y, OAK_LOG);
            }
        }
        for(int i = -6; i <= 6; i++) {
            pp = glm::vec2(xx, zz) + perp*(float)i + 8.f*back;
            setBlockAt(pp.x, floorh, pp.y, OAK_LOG);
        }
        for(int i = -1; i <= 1; i+= 2) {
            for(int j = 3; j <= 7; j++) {
                pp = glm::vec2(xx, zz) + 7.f*perp*(float)i + back*(float)j;
                setBlockAt(pp.x, floorh, pp.y, OAK_LOG);
            }
        }
        pp = glm::vec2(xx, zz) - 2.f*perp + 2.f*back;
        setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        pp = glm::vec2(xx, zz) + 2.f*back;
        setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        pp = glm::vec2(xx, zz) + 3.f*back;
        setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        pp = glm::vec2(xx, zz) + 2.f*perp + 2.f*back;
        setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        //layer 6
        floorh++;
        for(int i = 2; i <= 8; i++) {
            for(int j = -1; j <= 1; j+= 2) {
                pp = glm::vec2(xx, zz) + perp*(float)(i*j) + 2.f*back;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
        }
        for(int i = -8; i <= 8; i++) {
            for(int j = 8; j <= 9; j++) {
                pp = glm::vec2(xx, zz) + perp*(float)i + back*(float)j;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
        }
        for(int i = -1; i <= 1; i+= 2) {
            for(int j = 3; j <= 7; j++) {
                pp = glm::vec2(xx, zz) + 7.f*perp*(float)i + back*(float)j;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
        }
        for(int i = -1; i <= 1; i+= 2) {
            for(int j = 4; j <= 8; j++) {
                pp = glm::vec2(xx, zz) + perp*(float)(i*j) + back;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
        }
        pp = glm::vec2(xx, zz) + 2.f*back;
        setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        //layer 7
        floorh++;
        for(int i = -8; i <= 8; i++) {
            for(int j = 7; j <= 8; j++) {
                pp = glm::vec2(xx, zz) + perp*(float)i + back*(float)j;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
            pp = glm::vec2(xx, zz) + perp*(float)i + back*2.f;
            setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        }
        for(int i = -1; i <= 1; i+= 2) {
            for(int j = 3; j <= 8; j++){
                pp = glm::vec2(xx, zz) + perp*(float)(i*j) + back*3.f;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
        }
        for(int i = -1; i <= 1; i+= 2) {
            for(int j = 4; j <= 6; j++){
                pp = glm::vec2(xx, zz) + 7.f*perp*(float)i + back*(float)j;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
        }
        pp = glm::vec2(xx, zz) - perp*3.f + back;
        setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        pp = glm::vec2(xx, zz) + perp*3.f + back;
        setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
        //layer 8
        floorh++;
        for(int i = -8; i <= 8; i++) {
            for(int j = 6; j <= 7; j++){
                pp = glm::vec2(xx, zz) + perp*(float)i + back*(float)j;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
        }
        for(int i = 2; i <= 8; i++) {
            for(int j = -1; j <= 1; j+= 2){
                for(int k = 3; k <= 4; k++) {
                    pp = glm::vec2(xx, zz) + perp*(float)(i*j) + back*(float)k;
                    setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
                }
            }
        }
        for(int i = 4; i <= 6; i++) {
            for(int j = -1; j <= 1; j+= 2) {
                pp = glm::vec2(xx, zz) + 7.f*perp*(float)j + back*(float)i;
                setBlockAt(pp.x, floorh, pp.y, OAK_LOG);
            }
        }
        for(int i = -2; i <= 2; i++) {
            pp = glm::vec2(xx, zz) + perp*(float)i + back*2.f;
            setBlockAt(pp.x, floorh, pp.y, OAK_LOG);
        }
        //layer 9
        floorh++;
        for(int i = 4; i <= 6; i++) {
            for(int j = -8; j <= 8; j++) {
                pp = glm::vec2(xx, zz) + perp*(float)j + back*(float)i;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
        }
        for(int i = -2; i <= 2; i++) {
            for(int j = 1; j <= 3; j++) {
                pp = glm::vec2(xx, zz) + perp*(float)i + back*(float)j;
                setBlockAt(pp.x, floorh-1, pp.y, OAK_PLANKS);
            }
        }
        pp = glm::vec2(xx, zz) + back;
        setBlockAt(pp.x, floorh, pp.y, EMPTY);
        pp = glm::vec2(xx, zz) + back*3.f;
        setBlockAt(pp.x, floorh, pp.y, EMPTY);
        pp = glm::vec2(xx, zz) + back*4.f;
        setBlockAt(pp.x, floorh, pp.y, EMPTY);
        //layer 10
        //floorh++;
        for(int i = -1; i <= 1; i++) {
            for(int j = 1; j <= 5; j++) {
                pp = glm::vec2(xx, zz) + perp*(float)i + back*(float)j;
                setBlockAt(pp.x, floorh, pp.y, OAK_PLANKS);
            }
        }
        //pillars
        floorh = c->heightMap[xx-x][zz-z];
        for(int i = 0; i < 6; i++) {
            pp = glm::vec2(xx, zz) + perp*7.f + back*2.f;
            setBlockAt(pp.x, floorh+i, pp.y, OAK_LOG);
            pp = glm::vec2(xx, zz) - perp*7.f + back*2.f;
            setBlockAt(pp.x, floorh+i, pp.y, OAK_LOG);
            pp = glm::vec2(xx, zz) + perp*7.f + back*8.f;
            setBlockAt(pp.x, floorh+i, pp.y, OAK_LOG);
            pp = glm::vec2(xx, zz) - perp*7.f + back*8.f;
            setBlockAt(pp.x, floorh+i, pp.y, OAK_LOG);
        }
        for(int i = 0; i < 7; i++) {
            pp = glm::vec2(xx, zz) + perp*3.f + back*2.f;
            setBlockAt(pp.x, floorh+i, pp.y, OAK_LOG);
            pp = glm::vec2(xx, zz) - perp*3.f + back*2.f;
            setBlockAt(pp.x, floorh+i, pp.y, OAK_LOG);
        }
        break;
    }
    default:
        break;
    }
}

bool Terrain::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection,
                        float *out_dist, glm::ivec3 *out_blockHit) const

{
    float maxLen = glm::length(rayDirection); // Farthest we search
        glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
        rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

        float curr_t = 0.f;
        int count = 0;
        while(curr_t < maxLen) {
            float min_t = glm::sqrt(3.f);
            float interfaceAxis = -1; // Track axis for which t is smallest
            for(int i = 0; i < 3; ++i) { // Iterate over the three axes
                if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                    float offset = glm::max(0.f, glm::sign(rayDirection[i]));
                    // If the player is *exactly* on an interface then
                    // they'll never move if they're looking in a negative direction
                    if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                        offset = -1.f;
                    }
                    int nextIntercept = currCell[i] + offset;
                    float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                    axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                    if(axis_t < min_t) {
                        min_t = axis_t;
                        interfaceAxis = i;
                    }
                }
            }
            if(interfaceAxis == -1) {
                throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
            }
            curr_t += min_t; // min_t is declared in slide 7 algorithm
            rayOrigin += rayDirection * min_t;
            glm::ivec3 offset = glm::ivec3(0,0,0);
            // Sets it to 0 if sign is +, -1 if sign is -
            offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
            currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
            // If currCell contains something other than EMPTY, return
            // curr_t
            BlockType cellType = getBlockAt(currCell.x, currCell.y, currCell.z);
            if(cellType != EMPTY) {
                *out_blockHit = currCell;
                if (count == 0) {
                    *out_dist = 0;
                } else {
                    *out_dist = glm::min(maxLen, curr_t);
                }
                return true;
            }
            count++;
        }
        *out_dist = glm::min(maxLen, curr_t);
        return false;
}
