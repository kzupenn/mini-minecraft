#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include "drawable.h"
#include "biome.h"


//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, SAND, SNOW};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

glm::vec3 dirToVec(Direction);

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

//biomes here
//enum BiomeType : unsigned char
//{
//    //snowy
//    SNOWY_PLAINS, ICE_SPIKES, SNOWY_TAIGA, SNOWY_BEACH, GROVE,
//    SNOWY_SLOPES, JAGGED_PEAKS, FROZEN_PEAKS,
//    //cold
//    WINDSWEPT_HILLS, WINDSWEPT_GRAVELLY_HILLS, WINDSWEPT_FOREST, TAIGA, OLD_GROWTH_PINE_TAIGA, OLD_GROWTH_SPRUCE_TAIGA,
//    STONY_SHORE,
//    //temperate
//    PLAINS, SUNFLOWER_PLAINS, FOREST, FLOWER_FOREST, BIRCH_FOREST, OLD_GROWTH_BIRCH_FOREST, DARK_FOREST, SWAMP, MANGROVE_SWAMP, JUNGLE, SPARSE_JUNGLE, BAMBOO_JUNGLE, BEACH, MUSHROOM_FIELDS, MEADOW, STONY_PEAKS, CHERRY_GROVE,
//    //warm
//    DESERT, SAVANNA, SAVANNA_PLATEAU, WINDSWEPT_SAVANNA, BADLANDS, WOODED_BADLANDS, ERODED_BADLANDS
//};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable
class Chunk : public Drawable{
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;


    //for vbo
    std::vector<glm::vec4> VBOpos;
    std::vector<glm::vec4> VBOnor;
    std::vector<glm::vec4> VBOcol;
    std::vector<int> idx;

    std::mutex setBlock_mutex;
    std::mutex createVBO_mutex;
public:
    Chunk(OpenGLContext*);
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);

    void setPos(int x, int z);

    virtual void createVBOdata();
    //locks for multithreading stages
    bool dataBound, dataGen, surfaceGen;

    //for generating structures, don't want to redraw vbo for every single one
    std::atomic_bool blocksChanged;

    void bindVBOdata();
    virtual GLenum drawMode();

    //for debugging
    glm::vec3 debugColor;

    //for terrain gen
    glm::vec4 pos; //position of chunk
    BiomeType biome; //biome of chunk, taking 8,8
    int heightMap[16][16]; //height map of surface level ground, to generate surface structs
};
