#include "noise.h"
#include <QDebug>

using namespace glm;

float noise1D( vec2 p, vec3 seed) {
    return fract(sin(dot(p, vec2(seed[0], seed[1]))) * seed[2]);
}

float noise1D( vec3 p, vec4 seed) {
    return fract(sin(abs(dot(p+vec3(seed[1], seed[2], seed[3]), vec3(seed))) *
                 seed[3]));
}

vec2 random2( vec2 p, vec4 seed) {
    return fract(sin(abs(vec2(dot(p, vec2(seed)),
                 dot(p, vec2(seed[2], seed[3]))))));
}

vec3 random3( vec3 p, vec4 seed) {
//    return fract(sin(abs(vec3(dot(p+vec3(0, seed[2], seed[3]), vec3(seed)),
//                 dot(p, vec3(seed[1], seed[3], seed[2])),
//                 dot(p+vec3(seed[3], seed[2], seed[1]), vec3(seed))
//            ))));
    return fract(sin(vec3(dot(p,vec3(12.71, 31.17, 56.35)),
                              dot(p,vec3(26.95, 18.33, 45.16)),
                              dot(p, vec3(42.06, 63.12, 95.28))
                        )) * 458.5453f);

}
