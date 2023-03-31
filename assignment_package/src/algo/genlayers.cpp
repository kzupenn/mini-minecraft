#include "genlayers.h"

using namespace glm;

int64_t convolute(int64_t x, int64_t a){
    return (x * (x * int64_t(6364136223846793005) + int64_t(1442695040888963407))) + a;
}
