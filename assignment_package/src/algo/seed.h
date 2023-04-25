#pragma once
#include "glm_includes.h"
#include <mutex>

class Seed {
private:
    float seed;
public:
    Seed(float s);
    void setSeed(float s) ;
    float getSeed(float f) const; //returns seed with obfuscation factor f
    glm::vec3 getSeed(float f1, float f2, float f3) const;
    glm::vec4 getSeed(float f1, float f2, float f3, float f4) const;
};

static Seed SEED(69);
