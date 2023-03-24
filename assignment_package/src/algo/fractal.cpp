#include "fractal.h"
#include "perlin.h"
#include <QDebug>

using namespace glm;


float fBm(vec2 x, int numoctaves, vec3 seed, int grid_size)
{
    float G = 0.5;
    float f = 1.0;
    float a = 0.5;//1.0;
    float t = 0.0;
    for( int i=0; i<numoctaves; i++ )
    {
        t += a*(normPerlin(f*x, seed, grid_size));
        f *= 2.0;
        a *= G;
    }
    return t;
}

float hybridMultifractal(vec2 p, int numoctaves, vec3 seed, int grid_size) {
    float G = 0.5;
    float f = 1.0;
    float a = 0.5;//1.0; using 0.5 to scale from 0,2 to 0,1

    float t = a*normPerlin(f*p, seed, grid_size);
    float weight = t;
    f *= 2.0;
    a *= G;
    for( int i=1; i<numoctaves; i++ )
    {
        if(weight>1.0) weight = 1.0;

        float signal = normPerlin(f*p, seed, grid_size);
        t += weight*a*signal;
        f *= 2.0;
        a *= G;

        weight *= signal;
    }
    return clamp(t, 0.f, 1.f);
}

float hybridMultifractalM(vec2 p, int numoctaves, vec3 seed, int grid_size) {
    float G = 0.5;
    float f = 1.0;
    float a = 0.5;//1.0; using 0.5 to scale from 0,2 to 0,1

    float t = a*(1-abs(perlinNoise(f*p, seed, grid_size)));
    float weight = t;
    f *= 2.0;
    a *= G;
    for( int i=1; i<numoctaves; i++ )
    {
        if(weight>1.0) weight = 1.0;

        float signal = 1-abs(perlinNoise(f*p, seed, grid_size));
        t += weight*a*signal;
        f *= 2.0;
        a *= G;

        weight *= signal;
    }
    return clamp(t, 0.f, 1.f);
}


float warpPattern(vec2 p, vec2 &q, vec2 &r, int numoctaves, float distortion, vec3 seed, int grid_size)
{
    q = vec2(fBm( p + vec2(0.0,0.0), numoctaves, seed, grid_size),
             fBm( p + distortion*vec2(5.2,1.3), numoctaves, seed, grid_size));

    r = vec2(fBm( p + q*4.f + distortion*vec2(1.7,9.2), numoctaves, seed, grid_size ),
             fBm( p + 4.f*q + distortion*vec2(8.3,2.8), numoctaves, seed, grid_size ));

    return hybridMultifractal( p + 4.f*distortion*r, numoctaves, seed, grid_size );
}
