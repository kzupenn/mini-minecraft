#include "noise.h"

using namespace glm;

float noise1D( vec2 p, vec3 seed) {
    return fract(sin(dot(p, vec2(seed))) *
                 seed[2]);
}

float noise1D( vec3 p, vec4 seed) {
    return fract(sin(dot(p, vec3(seed))) *
                 seed[3]);
}

vec2 random2( vec2 p, vec4 seed) {
    return fract(sin(vec2(dot(p, vec2(seed)),
                 dot(p, vec2(seed[2], seed[3])))));
}

