#include "structure.h"
#include "algo/perlin.h"
#include "algo/worley.h"
#include "algo/noise.h"
#include "terrain.h"

using namespace glm;

//TODO: MOVE SEED TO SOMEWHERE ELSE
int seed2 = 2294021;
float getSeed2(float f) {
    return seed2*f;
}

std::vector<Structure> getStructureZones(Chunk* c) {
    std::vector<Structure> ret;
    vec2 cp = vec2(c->pos.x, c->pos.z);
    //find trees, trees should force a 3x3 generation zone
    switch(c->biome){
    case PLAINS:
        for(int i = 0; i < 16; i+=4) {
            for(int j = 0; j < 16; j+=4) {
                vec2 pp = cp+vec2(i,j)+4.f*random2(cp, vec4(getSeed2(1),getSeed2(1.2),getSeed2(1.4),getSeed2(1.1)));
                ret.push_back(Structure(OAK_TREE, pp, vec2(3, 3)));
            }
        }
        break;
    default:
        break;
    }
}
