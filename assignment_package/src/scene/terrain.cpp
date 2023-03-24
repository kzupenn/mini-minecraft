#include "terrain.h"
#include "algo/worley.h"
#include "cube.h"
#include "algo/perlin.h"
#include "algo/noise.h"
#include "algo/fractal.h"
#include "scene/biome.h"
#include "scene/structure.h"
#include <stdexcept>
#include <iostream>
#include <QDebug>
#include <thread>

#define TEST_RADIUS 256

#define ocean_level 0.54
#define beach_level 0.03

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), mp_context(context)
{
    for(int i = 0; i < 50; i++) {

    }
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
BlockType Terrain::getBlockAt(int x, int y, int z)
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
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

BlockType Terrain::getBlockAt(glm::vec3 p)  {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) {
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
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}


Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context);
    Chunk *cPtr = chunk.get();

    //terrain initialization
    //qDebug() << "Generating" << x << z;
    for(int xx = x; xx < x+16; xx++) {
        for(int zz = z; zz < z+16; zz++) {
            float bedrock = 2*generateBedrock(vec2(xx,zz));
            std::pair<float, BiomeType> groundInfo = generateGround(vec2(xx,zz));
            //ground
            if(bedrock < ocean_level) {
                //use center of chunk as the biome of the chunk
                if(xx == x+8 && zz == z+8) {
                    cPtr->biome = groundInfo.second;
                }
                //height
                float height = 64 + groundInfo.first +(ocean_level-bedrock)*50;
                cPtr->heightMap[xx-x][zz-z] = height;
                for(int y = 0; y < height; y++) {
                    switch(groundInfo.second) {
                        case TUNDRA:
                            cPtr->setBlockAt(xx-x, y, zz-z, STONE);
                            break;
                        case PLAINS:
                            cPtr->setBlockAt(xx-x, y, zz-z, GRASS);
                            break;
                        case DESERT:
                            cPtr->setBlockAt(xx-x, y, zz-z, SAND);
                            break;
                        case TAIGA:
                            cPtr->setBlockAt(xx-x, y, zz-z, GRASS);
                            break;
                        case FOREST:
                            cPtr->setBlockAt(xx-x, y, zz-z, STONE);
                            break;
                        default:
                            cPtr->setBlockAt(xx-x, y, zz-z, GRASS);
                            break;
                    }

                }
            }
            //beach
            else if(bedrock < ocean_level + beach_level){
                //use center of chunk as the biome of the chunk
                if(xx == x+8 && zz == z+8) {
                    cPtr->biome = BEACH;
                }
                //height
                //float erosion = generateErosion(vec2(xx,zz));
                //shoreline
                if(groundInfo.first > 20) {
                    float height = 64 + groundInfo.first +(ocean_level+beach_level-bedrock)*50;
                    cPtr->heightMap[xx-x][zz-z] = height;
                    for(int y = 0; y < height; y++) {
                        switch(groundInfo.second) {
                            case TUNDRA:
                                cPtr->setBlockAt(xx-x, y, zz-z, STONE);
                                break;
                            case PLAINS:
                                cPtr->setBlockAt(xx-x, y, zz-z, GRASS);
                                break;
                            case DESERT:
                                cPtr->setBlockAt(xx-x, y, zz-z, SAND);
                                break;
                            case TAIGA:
                                cPtr->setBlockAt(xx-x, y, zz-z, GRASS);
                                break;
                            case FOREST:
                                cPtr->setBlockAt(xx-x, y, zz-z, STONE);
                                break;
                            default:
                                cPtr->setBlockAt(xx-x, y, zz-z, GRASS);
                                break;
                        }
                    }
                }
                //override shoreline
                else {
                    float height = 64 + pow((ocean_level+beach_level-bedrock)*30,5)*groundInfo.first;
                    cPtr->heightMap[xx-x][zz-z] = height;
                    if(height <= 64+5){
                        for(int y = 0; y < height; y++) {
                            cPtr->setBlockAt(xx-x, y, zz-z, SAND);
                        }
                    }
                    else {
                        for(int y = 0; y < height; y++) {
                            switch(groundInfo.second) {
                                case TUNDRA:
                                    cPtr->setBlockAt(xx-x, y, zz-z, STONE);
                                    break;
                                case PLAINS:
                                    cPtr->setBlockAt(xx-x, y, zz-z, GRASS);
                                    break;
                                case DESERT:
                                    cPtr->setBlockAt(xx-x, y, zz-z, SAND);
                                    break;
                                case TAIGA:
                                    cPtr->setBlockAt(xx-x, y, zz-z, GRASS);
                                    break;
                                case FOREST:
                                    cPtr->setBlockAt(xx-x, y, zz-z, STONE);
                                    break;
                                default:
                                    cPtr->setBlockAt(xx-x, y, zz-z, GRASS);
                                    break;
                            }
                        }
                    }
                }
            }
            else if(bedrock < ocean_level + 0.05){
                //use center of chunk as the biome of the chunk
                if(xx == x+8 && zz == z+8) {
                    cPtr->biome = OCEAN;
                }
                //height
                cPtr->heightMap[xx-x][zz-z] = 64;
                for(int y = 0; y < 64; y++) {
                    cPtr->setBlockAt(xx-x, y, zz-z, WATER);
                }
            }
            else {
                //use center of chunk as the biome of the chunk
                if(xx == x+8 && zz == z+8) {
                    cPtr->biome = OCEAN;
                }
                //height
                cPtr->heightMap[xx-x][zz-z] = 64;
                for(int y = 0; y < 64; y++) {
                    cPtr->setBlockAt(xx-x, y, zz-z, WATER);
                }
            }
        }
    }

    cPtr->setPos(x, z);
    cPtr->createVBOdata();

    m_chunks_mutex.lock();
    m_chunks[toKey(x, z)] = move(chunk);
    m_chunks_mutex.unlock();
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        m_chunks_mutex.lock();
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        m_chunks_mutex.unlock();
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
         m_chunks_mutex.lock();
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        m_chunks_mutex.unlock();
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
         m_chunks_mutex.lock();
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        m_chunks_mutex.unlock();
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
         m_chunks_mutex.lock();
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        m_chunks_mutex.unlock();
        cPtr->linkNeighbor(chunkWest, XNEG);
    }

    cPtr->dataGen = true;
    return cPtr;
}

Chunk* Terrain::instantiateStructures(int x, int z) {
    Chunk* c = getChunkAt(x, z).get();
    return c;
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
                    qDebug() << chunk->biome;
                    shaderProgram->draw(*chunk.get());
                }
            }
            else {
                //qDebug() << "missing chunk at " << x << z;
            }
        }
    }
}



void Terrain::CreateTestScene()
{
    return;
    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = 0; x < TEST_RADIUS; x += 16) {
        for(int z = 0; z < TEST_RADIUS; z += 16) {
            instantiateChunkAt(x, z);
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(0, 0));
}

void Terrain::createGroundThread(glm::vec2 p) {
    if(hasChunkAt(p.x, p.y)) return;
    qDebug() << "instantiating " << p.x << p.y;
    groundGen_mutex.lock();
    groundGenThreads.push_back(std::thread(&Terrain::instantiateChunkAt, this, p.x, p.y));
    groundGen_mutex.unlock();
}

void Terrain::createStructThread(glm::vec2 p) {
    structGen_mutex.lock();
    structGenThreads.push_back(std::thread(&Terrain::instantiateStructures, this, p.x, p.y));
    structGen_mutex.unlock();
}

