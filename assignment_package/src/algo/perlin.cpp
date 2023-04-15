#include "perlin.h"
#include <QDebug>
#include "noise.h"

using namespace glm;


float surflet(vec2 P, vec2 gridPoint, vec4 perlinSeed) {
    // Compute falloff function by converting linear distance to a polynomial
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.f) + 15 * pow(distX, 4.f) - 10 * pow(distX, 3.f);
    float tY = 1 - 6 * pow(distY, 5.f) + 15 * pow(distY, 4.f) - 10 * pow(distY, 3.f);
    // Get the random vector for the grid point
    vec2 gradient = 2.f * random2(gridPoint, perlinSeed) - vec2(1.f);
    // Get the vector from the grid point to P
    vec2 diff = P - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;
}

glm::vec3 vecPow(glm::vec3 &v, float f) {
    for (int i = 0; i < 3; i++) {
        v[i] = pow(v[i], f);
    }
    return v;
}

//float surflet3D(vec3 p, vec3 gridPoint, glm::vec4 perlinSeed) {
//    // Compute the distance between p and the grid point along each axis, and warp it with a
//    // quintic function so we can smooth our cells
//    vec3 t2 = abs(p - gridPoint);
//    vec3 t = vec3(1.f) - 6.f * vecPow(t2, 5.f) + 15.f * vecPow(t2, 4.f) - 10.f * vecPow(t2, 3.f);
//    // Get the random vector for the grid point (assume we wrote a function random2
//    // that returns a vec2 in the range [0, 1])
//    vec3 gradient = random3(gridPoint, perlinSeed) * 2.f - vec3(1);
//    // Get the vector from the grid point to P
//    vec3 diff = p - gridPoint;
//    // Get the value of our height field by dotting grid->P with our gradient
//    float height = dot(diff, gradient);
//    // Scale our height field (i.e. reduce it) by our polynomial falloff function
//    return height * t.x * t.y * t.z;
//}

float perlinNoise(vec2 uv, vec4 seed, int g) {
    float gridSizePerlin = 1.f/g;
    uv *= gridSizePerlin;
    float surfletSum = 0.f;
    // Iterate over the four integer corners surrounding uv
    for(int dx = 0; dx <= 1; ++dx) {
        for(int dy = 0; dy <= 1; ++dy) {
            surfletSum += surflet(uv, floor(uv) + vec2(dx, dy), seed);
        }
    }
    return 2*surfletSum;
}

//float perlinNoise3D(vec3 p, vec4 seed, int g) {
//    float gridSizePerlin = 1.f/g;
//    p *= gridSizePerlin;
//    float surfletSum = 0.f;
//    // Iterate over the eight integer corners surrounding uv
//    for(int dx = 0; dx <= 1; ++dx) {
//        for(int dy = 0; dy <= 1; ++dy) {
//            for(int dz = 0; dz <= 1; ++dz) {
//                surfletSum += surflet3D(p, floor(p) + vec3(dx, dy, dz), seed);
//            }
//        }
//    }
//    return surfletSum;
//}

float perlinNoise3D(glm::vec3 p, glm::vec4 seed, int g) {
    p/=g;

    glm::vec3 i = glm::floor(p);
    glm::vec3 f = glm::fract(p);
    glm::vec3 u = f * f * (3.0f - 2.0f * f);

    float c[2][2][2];
    for (int di = 0; di < 2; ++di) {
        for (int dj = 0; dj < 2; ++dj) {
            for (int dk = 0; dk < 2; ++dk) {
                glm::vec3 g = random3((i + glm::vec3(di, dj, dk))*g, seed);
                glm::vec3 v(di, dj, dk);
                c[di][dj][dk] = glm::dot(g, f - v);
            }
        }
    }

    float x = glm::mix(glm::mix(glm::mix(c[0][0][0], c[1][0][0], u.x),
                                glm::mix(c[0][1][0], c[1][1][0], u.x), u.y),
                       glm::mix(glm::mix(c[0][0][1], c[1][0][1], u.x),
                                glm::mix(c[0][1][1], c[1][1][1], u.x), u.y), u.z);

    return x;
}

float normPerlin(vec2 uv, vec4 seed, int g) {
    return clamp(0.5f*(1+perlinNoise(uv, seed, g)), 0.f, 1.f);
}

void distTest() {
    int buckets[10] = {0,0,0,0,0,0,0,0,0,0};
    float all[1000*1000];
    int k = 0;
    for(int i = 0; i < 100; i++) {
        for(int j = 0; j < 100; j++) {
            float f = perlinNoise3D(vec3(i,j, k), vec4(13123,1231,3123, 12313), 512);
            all[k++] = f;
            f*=10;
            buckets[(int)(floor(f))%10]++;

        }
    }
    for(int i = 0; i < 10; i++) {
        qDebug() << i << buckets[i]/1000000.f;
    }
    std::sort(std::begin(all), std::end(all));
    qDebug() << "5 percentiles";
    for(int i = 0; i < 20; i++) {
        qDebug() << i*0.05 << all[1000*1000*i/20];
    }
}

