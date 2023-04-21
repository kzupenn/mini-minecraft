#pragma once
#include "QtCore/qthreadpool.h"
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "chunk.h"
#include "scene/structure.h"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include "shaderprogram.h"
#include <mutex>
#include <QSemaphore>


struct metadata{
    BlockType type;
    glm::vec3 pos;
    bool(*con)(int,int,int,Chunk*);
    metadata(BlockType t, glm::vec3 p, bool(*c)(int,int,int,Chunk*)): type(t), pos(p), con(c){

    }
    metadata(BlockType t, glm::vec3 p) : metadata(t, p, nullptr){

    }
};

// Helper functions to convert (x, z) to and from hash map key
int64_t toKey(int x, int z);
glm::ivec2 toCoords(int64_t k);


// The container class for all of the Chunks in the game.
// Ultimately, while Terrain will always store all Chunks,
// not all Chunks will be drawn at any given time as the world
// expands.
class Terrain {
private:
    // Stores every Chunk according to the location of its lower-left corner
    // in world space.
    // We combine the X and Z coordinates of the Chunk's corner into one 64-bit int
    // so that we can use them as a key for the map, as objects like std::pairs or
    // glm::ivec2s are not hashable by default, so they cannot be used as keys.
    std::unordered_map<int64_t, uPtr<Chunk>> m_chunks;
    mutable std::mutex m_chunks_mutex;

    OpenGLContext* mp_context;

    //multithreading!
    //meta data, stores chunk changes until that chunk is loaded, after which it loads those changes in
    std::mutex metaData_mutex;
    std::map<int64_t, std::vector<metadata>> metaData;

    //for user changes, to be implemented strictly after all changes to simulate user changes
    std::mutex metaChangeData_mutex;
    std::map<int64_t, std::vector<metadata>> metaChangeData;

    //generates mega structures
    std::mutex metaStructures_mutex;
    std::map<std::pair<int64_t, int>, StructureType> metaStructures; //marks the meta structure to prevent regeneration

    std::mutex metaSubStructures_mutex; //prevent weird stuff from happening when we clear processed structures
    std::map<int64_t, std::vector<Structure>> metaSubStructures; //stores the structures generated by the metaStructure

    QThreadPool terrainWorkers;
    QThreadPool VBOWorkers;
public:
    Terrain(OpenGLContext *context);
    ~Terrain();

    // We will designate every 64 x 64 area of the world's x-z plane
    // as one "terrain generation zone". Every time the player moves
    // near a portion of the world that has not yet been generated
    // (i.e. its lower-left coordinates are not in this set), a new
    // 4 x 4 collection of Chunks is created to represent that area
    // of the world.
    // The world that exists when the base code is run consists of exactly
    // one 64 x 64 area with its lower-left corner at (0, 0).
    // When milestone 1 has been implemented, the Player can move around the
    // world to add more "terrain generation zone" IDs to this set.
    // While only the 3 x 3 collection of terrain generation zones
    // surrounding the Player should be rendered, the Chunks
    // in the Terrain will never be deleted until the program is terminated.
    std::unordered_set<int64_t> m_generatedTerrain;

    // Instantiates a new Chunk and stores it in
    // our chunk map at the given coordinates.
    // Returns a pointer to the created Chunk.
    Chunk* instantiateChunkAt(int x, int z);
    //for generating surface level objects that require multiple chunks
    void instantiateStructures(std::vector<Structure> vs);

    // Do these world-space coordinates lie within
    // a Chunk that exists?
    bool hasChunkAt(int x, int z) const;
    // Assuming a Chunk exists at these coords,
    // return a mutable reference to it
    uPtr<Chunk>& getChunkAt(int x, int z);
    // Assuming a Chunk exists at these coords,
    // return a const reference to it
    const uPtr<Chunk>& getChunkAt(int x, int z) const;
    // Given a world-space coordinate (which may have negative
    // values) return the block stored at that point in space.
    BlockType getBlockAt(int x, int y, int z) const;
    BlockType getBlockAt(glm::vec3 p) const;
    // Given a world-space coordinate (which may have negative
    // values) set the block at that point in space to the
    // given type.
    void setBlockAt(int x, int y, int z, BlockType t);
    // like setblock, but checks a conditional before placing
    void setBlockAt(int x, int y, int z, BlockType t, bool(*con)(int,int,int,Chunk*));
    // setblock for changes made after terrain generation
    void changeBlockAt(int x, int y, int z, BlockType t);
    // gets all changed blocks in chunks
    std::vector<std::pair<int64_t, vec3Map>> getChunkChanges();

    // Draws every Chunk that falls within the bounding box
    // described by the min and max coords, using the provided
    // ShaderProgram
    void draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram);

    // Initializes the Chunks that store the 64 x 256 x 64 block scene you
    // see when the base code is run.
    void createSpawn();
    std::atomic_bool setSpawn;
    glm::vec3 worldSpawn;

    //for multithreading
    //creates a ground thread
    void createGroundThread(glm::vec2);
    //creates a vbo thread
    void createVBOThread(Chunk* c);

    //processes the sub structures returned by generation functions into either meta data or directly into the chunk
    void processMegaStructure(const std::vector<Structure>& s);

    //builds the structures
    void buildStructure(const Structure&);

    bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, float *out_dist,
                   glm::ivec3 *out_blockHit, Direction &out_dir) const;
};


