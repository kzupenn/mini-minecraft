#pragma once
#include "glm_includes.h"

//worley noise, uses a 5x5 for smoother results than a 3x3
std::vector<std::pair<float, glm::vec2> > worleyNoise(glm::vec2 position, int results, glm::vec4 seed, int grid_size);

