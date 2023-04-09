#include "noise.h"
#include <QDebug>

using namespace glm;

float noise1D( vec2 p, vec3 seed) {
    //return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
    int n=(int)(p.x*seed.x+p.y*seed.y) + seed.z; // add your seed on this line.
    n = (n << 13)^n;
    int nn=(n*(n*n*41333 +53307781)+1376312589)&0x7fffffff;
    return ((1.0-((double)nn/1073741824.0))+1)/2.0;
}

float noise1D( vec3 p, vec4 seed) {
    return fract(sin(abs(dot(p+vec3(seed[1], seed[2], seed[3]), vec3(seed))) *
                 seed[3]));
}

vec2 random2( vec2 p, vec4 seed) {
    return fract(sin(abs(vec2(dot(p+vec2(seed[2], seed[3]), vec2(seed)),
                 dot(p+vec2(seed), vec2(seed[2], seed[3]))))));
}

vec3 random3( vec3 p, vec4 seed) {
    return fract(sin(abs(vec3(dot(p+vec3(0, seed[2], seed[3]), vec3(seed)),
                 dot(p, vec3(seed[1], seed[3], seed[2])),
                 dot(p+vec3(seed[3], seed[2], seed[1]), vec3(seed))
            ))));
}
