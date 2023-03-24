#pragma once

#include "glm_includes.h"

using namespace glm;

class GenLayer{
protected:
    int64_t base_seed;
    int64_t world_gen_seed;
public:
    GenLayer(int64_t seed);
    void setWorldGenSeed(int64_t seed);
    void
};
