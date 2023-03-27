#pragma once

#include "glm_includes.h"

using namespace glm;

//performs fractal brownian noise using perlin basis
float fBm(vec2 position, int numoctaves, vec3 seed, int grid_size);

//produces a warped pattern
float warpPattern(vec2 position, vec2 &q, vec2 &r, int numoctaves, float distortion, vec3 seed, int grid_size);

float hybridMultifractal(vec2 p, int numoctaves, vec3 seed, int grid_size);

float hybridMultifractalM(vec2 p, int numoctaves, vec3 seed, int grid_size);
