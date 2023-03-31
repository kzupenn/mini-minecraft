#include "structure.h"
#include "algo/perlin.h"
#include "algo/worley.h"
#include "algo/noise.h"
#include "scene/terrain.h"
#include <queue>

using namespace glm;

#define mp(a, b) std::make_pair(a, b)

//TODO: MOVE SEED TO SOMEWHERE ELSE
int seed2 = 2294021;
float getSeed2(float f) {
    return seed2*f;
}

std::vector<Structure> getStructureZones(Chunk* c, int x, int z) {
    std::vector<Structure> ret;
    ivec2 cp = ivec2(x, z);
    //find trees, trees should force a 3x3 chunk generation zone
    switch(c->biome){
    case PLAINS:
//        for(int i = 0; i < 16; i+=4) {
//            for(int j = 0; j < 16; j+=4) {
//                vec2 pp = cp+vec2(i,j)+4.f*random2(cp, vec4(getSeed2(1),getSeed2(1.2),getSeed2(1.4),getSeed2(1.1)));
//                ret.push_back(Structure(OAK_TREE, pp, vec2(3, 3)));
//            }
//        }
        break;
    case OCEAN:
        break;
    case BEACH:
        break;
    default:
//        for(int i = 0; i < 16; i+=16) {
//            for(int j = 0; j < 16; j+=16) {
//                ivec2 pp = cp+ivec2(i,j)+ivec2(clamp(16.f*random2(cp, vec4(getSeed2(1),getSeed2(1.2),getSeed2(1.4),getSeed2(1.1))), 0.f, 15.f));
//                // TO DO: modify tree spawn conditions
//                if(c->heightMap[pp.x-cp.x][pp.y-cp.y] < 64+64 && c->getBlockAt(pp.x-cp.x, c->heightMap[i][j]-1, pp.y-cp.y) == GRASS){
//                    ret.push_back(Structure(SPRUCE_TREE, pp));
//                }
//            }
//        }
        break;
    }
    return ret;
}

//returns a quick computed list of meta structures relevant to a point
std::vector<std::pair<std::pair<int64_t, int>, StructureType>> getMetaStructures(vec2 pp) {
    std::vector<std::pair<std::pair<int64_t, int>, StructureType>> ret;
    vec2 villageCenter = vec2(0);//floor(pp*0.0011f)*900.f + 900.f*random2(floor(pp*0.0011f)*900.f, glm::vec4(125,45356,23,532));
    //using y = 1000 to indicate surface level generation
    ret.push_back(std::make_pair(std::make_pair(toKey(villageCenter.x, villageCenter.y), 1000), VILLAGE_CENTER));
    return ret;
}

//procedurally generates a village
std::vector<Structure> generateVillage(vec2 pp) {
    //TO DO: replace this fixed noise seed with a world seed based seed
    vec3 seeds7 = vec3(5893, 12319, 4394);
    qDebug() << "generating village";
    std::vector<Structure> ret;

    //generates village center
    ivec2 villageCenter = ivec2(0,0);//floor(pp*0.0011f)*900.f + 900.f*random2(floor(pp*0.0011f)*900.f, glm::vec4(125,45356,23,532));
    ret.emplace_back(VILLAGE_CENTER, villageCenter);
    ret.emplace_back(OAK_TREE, villageCenter);

    //stores the 'hitbox' of structures for spawn condition detection
    std::vector<std::pair<vec2, vec2>> hitbox;

    int libraries = 1;
    ret.emplace_back(VILLAGE_HOUSE_1, villageCenter+ivec2(-50, 50), XPOS);
    ret.emplace_back(VILLAGE_LIBRARY, villageCenter+ivec2(50, 50), XPOS);

    return ret;
}
