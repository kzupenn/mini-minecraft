#pragma once

#include "glm_includes.h"
#include <vector>

enum BiomeType : unsigned char
{
    TUNDRA, PLAINS, DESERT, TAIGA, SAVANNA, FOREST, SWAMP, RAINFOREST,
    RIVER, BEACH, OCEAN, TEST_BIOME
};

float generateBedrock(glm::vec2);

float generateErosion(glm::vec2);

float generateBeach(glm::vec2);

float generateRiver(glm::vec2);

std::pair<float, BiomeType> generateGround(glm::vec2);

void erosionDist();
void biomeDist();
