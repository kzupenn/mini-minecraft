#pragma once
#include "glm_includes.h"


float perlinNoise(glm::vec2 position, glm::vec3 noise_seed, int grid_size);
float normPerlin(glm::vec2, glm::vec3 noise_seed, int grid_size);

void distTest();
