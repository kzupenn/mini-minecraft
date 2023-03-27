#pragma once

#include "glm_includes.h"
#include "chunk.h"

enum StructureType : unsigned char{
    OAK_TREE, FANCY_OAK_TREE,
    SPRUCE_TREE, MEGA_SPRUCE_TREE, PINE_TREE, MEGA_PINE_TREE,
    VILLAGE
};

struct Structure{
    //type of structure
    StructureType type;
    //position the structure is centered at
    glm::ivec2 pos;
    //size of chunks the structure will take up
    glm::vec2 chunk_size;

    Structure(StructureType st, glm::vec2 p, glm::vec2 c):type(st), pos(p), chunk_size(c){

    }
};

std::vector<Structure> getStructureZones(Chunk* c);

