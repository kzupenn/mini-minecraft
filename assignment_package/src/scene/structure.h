#pragma once

#include "glm_includes.h"

enum StructureType : unsigned char{
    OAK_TREE, VILLAGE
};

struct Structure{
    //type of structure
    StructureType type;
    //position the structure is centered at
    glm::vec2 pos;
    //size of chunks the structure will take up
    glm::vec2 chunk_size;

    Structure(StructureType st, glm::vec2 p, glm::vec2 c):type(st), pos(p), chunk_size(c){

    }
};
