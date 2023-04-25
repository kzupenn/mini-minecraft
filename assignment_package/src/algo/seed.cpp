#include "seed.h"

Seed::Seed(float S) : seed(S) {

}

void Seed::setSeed(float f)  {
    seed = f;
}

float Seed::getSeed(float f) const{
    return seed*f;
}

glm::vec3 Seed::getSeed(float f1, float f2, float f3) const {
    return glm::vec3(getSeed(f1), getSeed(f2), getSeed(f3));
}

glm::vec4 Seed::getSeed(float f1, float f2, float f3, float f4) const {
    return glm::vec4(getSeed(f1), getSeed(f2), getSeed(f3), getSeed(f4));
}
