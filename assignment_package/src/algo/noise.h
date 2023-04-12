#pragma once

#include "glm_includes.h"

//given a coordinate and a seed, produce a float in the range [0-1]
float noise1D(glm::vec2, glm::vec3);
float noise1D(glm::vec3, glm::vec4);

//given a coordinate and a seed, produce a random pair of [0-1]
glm::vec2 random2(glm::vec2, glm::vec4);
glm::vec3 random3(glm::vec3, glm::vec4);
