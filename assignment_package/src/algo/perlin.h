#pragma once
#include "glm_includes.h"

//returns perlin noise from [-1, 1] as outlined in the noise lecture
float perlinNoise(glm::vec2 position, glm::vec3 noise_seed, int grid_size);

//scales perlin noise from the original range to [0, 1]
float normPerlin(glm::vec2, glm::vec3 noise_seed, int grid_size);

//distribution test for noise
void distTest();
