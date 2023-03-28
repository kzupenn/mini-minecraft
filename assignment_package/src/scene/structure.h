#pragma once

#include "glm_includes.h"
#include "chunk.h"

enum StructureType : unsigned char{
    OAK_TREE, FANCY_OAK_TREE,
    SPRUCE_TREE, MEGA_SPRUCE_TREE, PINE_TREE, MEGA_PINE_TREE,
    VILLAGE_CENTER, VILLAGE_ROAD, VILLAGE_HOUSE_1, VILLAGE_HOUSE_2
};

//struct for storing structure information
struct Structure{
    //type of structure
    StructureType type;
    //position the structure is centered at
    glm::ivec2 pos;
    //orientation for asymmetric structures
    Direction orient;

    Structure(StructureType st, glm::vec2 p):type(st), pos(p){

    }
    Structure(StructureType st, glm::vec2 p, Direction d):type(st), pos(p), orient(d){

    }
};


std::vector<Structure> getStructureZones(Chunk* c);

std::vector<std::pair<glm::vec2, StructureType>> getMetaStructures(glm::vec2 p);

std::vector<Structure> generateVillage(glm::vec2 p);

