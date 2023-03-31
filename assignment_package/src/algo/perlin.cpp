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

float normPerlin(vec2 uv, vec4 seed, int g) {
    return clamp(0.5f*(1+perlinNoise(uv, seed, g)), 0.f, 1.f);
}

void distTest() {
    int buckets[10] = {0,0,0,0,0,0,0,0,0,0};
    float all[1000*1000];
    int k = 0;
    for(int i = 0; i < 1000; i++) {
        for(int j = 0; j < 1000; j++) {
            float f = normPerlin(vec2(i,j), vec4(1203123,123123,123123, 12313), 16);
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

