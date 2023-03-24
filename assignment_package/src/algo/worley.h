#pragma once
#include "glm_includes.h"

std::vector<std::pair<float, glm::vec2> > worleyNoise(glm::vec2 position, int results, glm::vec4 seed, int grid_size);
