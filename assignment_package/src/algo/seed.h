#pragma once
#include "glm_includes.h"

class Seed {
private:
    int seed;
public:
    Seed(int s);
    void setSeed(int s) const;
    float getSeed(float f) const; //returns seed with obfuscation factor f
    glm::vec3 getSeed(float f1, float f2, float f3) const;
    glm::vec4 getSeed(float f1, float f2, float f3, float f4) const;
};

const static Seed SEED(420);
