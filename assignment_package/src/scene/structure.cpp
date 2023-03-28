#include "structure.h"
#include "algo/perlin.h"
#include "algo/worley.h"
#include "algo/noise.h"
#include <queue>

using namespace glm;

#define mp(a, b) std::make_pair(a, b)

//TODO: MOVE SEED TO SOMEWHERE ELSE
int seed2 = 2294021;
float getSeed2(float f) {
    return seed2*f;
}

std::vector<Structure> getStructureZones(Chunk* c) {
    std::vector<Structure> ret;
    ivec2 cp = ivec2(c->pos.x, c->pos.z);
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
std::vector<std::pair<vec2, StructureType>> getMetaStructures(vec2 pp) {
    std::vector<std::pair<vec2, StructureType>> ret;
    vec2 villageCenter = vec2(0);//floor(pp*0.0011f)*900.f + 900.f*random2(floor(pp*0.0011f)*900.f, glm::vec4(125,45356,23,532));
    ret.push_back(std::make_pair(villageCenter, VILLAGE_CENTER));
    return ret;
}
std::vector<Structure> generateVillage(vec2 pp) {
    qDebug() << "generating village";
    std::vector<Structure> ret;
    //generates village center
    vec2 villageCenter = vec2(0,0);//floor(pp*0.0011f)*900.f + 900.f*random2(floor(pp*0.0011f)*900.f, glm::vec4(125,45356,23,532));
    ret.emplace_back(VILLAGE_CENTER, villageCenter);
    //generates main road
    std::pair<vec2, int> seg1, seg2;
    vec2 delta1, delta2;
    vec2 tilt1, tilt2;
    vec2 perp;
    //decides on direction of main road
    if(noise1D(villageCenter, vec3(123,434,65))<0.5) {
        delta1 = vec2(1, 0);
        delta2 = vec2(-1, 0);
        perp = vec2(0, 1);
    }
    else {
        delta1 = vec2(0, 1);
        delta2 = vec2(0, -1);
        perp = vec2(1, 0);
    }
    //main road gen data
    seg1 = std::make_pair(villageCenter, 0);
    seg2 = std::make_pair(villageCenter, 0);
    tilt1 = tilt2 = vec2(1);
    int l1, l2;
    l1 = l2 = 0;
    //building data, will be created along main roads so space them out
    std::vector<vec2> buildingPos;
    int buildSpacer = 12;
    float buildFromRoadSpacer = 7.f;
    Direction faceUp, faceDown;
    if(delta1.x == 1) { //(1, 0) and (-1, 0)
        faceUp = ZNEG;
        faceDown = ZPOS;
    }
    else {
        faceUp = XNEG;
        faceDown = XPOS;
    }
    //TO DO: add sub roads branching off of the main road here and perform a generation with those as well. maybe generalize the main road gen code?
    for(int i = 0; i < 200; i++) {
        if(l1 > -1) {
            //place in the path tiles
            // TO DO: to simulate wear and tear, maybe omit some of them for grass/dirt instead
            ret.emplace_back(VILLAGE_ROAD, seg1.first);
            ret.emplace_back(VILLAGE_ROAD, seg1.first+perp);
            ret.emplace_back(VILLAGE_ROAD, seg1.first-perp);
            ret.emplace_back(VILLAGE_ROAD, seg1.first+2.f*perp);
            ret.emplace_back(VILLAGE_ROAD, seg1.first-2.f*perp);
            //check if we want to add a building above/below the road
            vec2 buildUp = seg1.first+buildFromRoadSpacer*perp;
            vec2 buildDown = seg1.first-buildFromRoadSpacer*perp;
            if(noise1D(buildUp, vec3(123,434,65)) < 0.25) {
                bool canbuild = true;
                for(vec2 otherB: buildingPos) {
                    if(abs(otherB.x-buildUp.x) < buildSpacer && abs(otherB.y-buildUp.y) < buildSpacer) {
                        canbuild = false;
                        break;
                    }
                }
                if(canbuild) {
                    // TO DO: add house variance here
                    ret.emplace_back(VILLAGE_HOUSE_1, buildUp, faceUp);
                    buildingPos.push_back(buildUp);
                }
            }
            if(noise1D(buildDown, vec3(123,434,65)) < 0.25) {
                bool canbuild = true;
                for(vec2 otherB: buildingPos) {
                    if(abs(otherB.x-buildDown.x) < buildSpacer && abs(otherB.y-buildDown.y) < buildSpacer) {
                        canbuild = false;
                        break;
                    }
                }
                if(canbuild) {
                    // TO DO: add house variance here
                    ret.emplace_back(VILLAGE_HOUSE_1, buildDown, faceDown);
                    buildingPos.push_back(buildDown);
                }
            }
            //use a basic L-system to warp, continue, or cut off the main road
            if(seg1.second >= 10) {
                float tilt = noise1D(seg1.first, vec3(123,434,65));
                if(tilt<0.5){
                    //tilt up
                    if(tilt*2<tilt1.x/(tilt1.x+tilt1.y)) {
                        seg1.first+=delta1+perp;
                        tilt1.x+=pow(l1*0.02, 2);
                    }
                    //tilt down
                    else {
                        seg1.first += delta1-perp;
                        tilt1.y+=pow(l1*0.02, 2);
                    }
                    seg1.second = 0;
                }
                else if(tilt<0.6 && l1 > 100) {
                    l1 = -1;
                }
                else{
                    seg1.first+=delta1;
                    seg1.second-=5;
                }
            }
            else {
                seg1.first+=delta1;
                seg1.second++;
            }
            l1++;
        }
        if(l2 > -1) {
            ret.emplace_back(VILLAGE_ROAD, seg2.first);
            ret.emplace_back(VILLAGE_ROAD, seg2.first+perp);
            ret.emplace_back(VILLAGE_ROAD, seg2.first-perp);
            ret.emplace_back(VILLAGE_ROAD, seg2.first+2.f*perp);
            ret.emplace_back(VILLAGE_ROAD, seg2.first-2.f*perp);
            //check if we want to add a building above/below the road
            vec2 buildUp = seg2.first+buildFromRoadSpacer*perp;
            vec2 buildDown = seg2.first-buildFromRoadSpacer*perp;
            if(noise1D(buildUp, vec3(123,434,65)) < 0.25) {
                bool canbuild = true;
                for(vec2 otherB: buildingPos) {
                    if(abs(otherB.x-buildUp.x) < buildSpacer && abs(otherB.y-buildUp.y) < buildSpacer) {
                        canbuild = false;
                        break;
                    }
                }
                if(canbuild) {
                    // TO DO: add house variance here
                    ret.emplace_back(VILLAGE_HOUSE_1, buildUp, faceUp);
                    buildingPos.push_back(buildUp);
                }
            }
            if(noise1D(buildDown, vec3(123,434,65)) < 0.25) {
                bool canbuild = true;
                for(vec2 otherB: buildingPos) {
                    if(abs(otherB.x-buildDown.x) < buildSpacer && abs(otherB.y-buildDown.y) < buildSpacer) {
                        canbuild = false;
                        break;
                    }
                }
                if(canbuild) {
                    // TO DO: add house variance here
                    ret.emplace_back(VILLAGE_HOUSE_1, buildDown, faceDown);
                    buildingPos.push_back(buildDown);
                }
            }

            if(seg2.second >= 10) {
                float tilt = noise1D(seg2.first, vec3(123,434,65));
                if(tilt<0.5){
                    //tilt up
                    if(tilt*2<tilt2.x/(tilt2.x+tilt2.y)) {
                        seg2.first+=delta2+perp;
                        tilt2.x+=pow(l2*0.02, 2);
                    }
                    //tilt down
                    else {
                        seg2.first += delta2-perp;
                        tilt2.y+=pow(l2*0.02, 2);
                    }
                    seg2.second = 0;
                }
                else if(tilt<0.6 && l2 > 100) {
                    l1 = -1;
                }
                else{
                    seg2.first+=delta2;
                    seg2.second-=5;
                }
            }
            else {
                seg2.first+=delta2;
                seg2.second++;
            }
            l2++;
        }
    }
    return ret;
}
