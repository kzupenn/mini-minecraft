#include "biome.h"
#include "algo/fractal.h"
#include "algo/perlin.h"
#include "algo/worley.h"
#include "terrain.h"
#include <map>

using namespace glm;

#define OCEAN 0.55

//TODO: MOVE SEED TO SOMEWHERE ELSE
int seed = 12294021;
float getSeed(float f) {
    return seed*f;
}

vec4 heightSeed = vec4(getSeed(4), getSeed(3), getSeed(5), getSeed(2));

const bool TESTING = false;

//terrain generation
//divets
float divet(glm::vec2 pos) {
    return -20*max(0.f, fBm(pos, 4, vec4(getSeed(1.43), getSeed(43.3), getSeed(431), getSeed(223)), 50)-0.57f);
}
//flat
float superflat(glm::vec2 p){
    return 0;
}

//flat, but with natural divets
float flat(glm::vec2 pos) {
    //add divets in terrain
    return divet(pos);
}

//shallow hills
float hills(glm::vec2 pos) {
    std::vector<std::pair<float, glm::vec2>> hillWeights = worleyNoise(pos, 9, glm::vec4(getSeed(2), getSeed(3), getSeed(4), getSeed(5)), 32);
    float ret = 0;
    for(std::pair<float, glm::vec2> p: hillWeights) {
        ret+=pow(glm::tanh(1.5*p.first-1.5)*0.5+0.5, 4);
    }

    //return true height
    return 6*ret-6;
}

//valley?

float terraces(glm::vec2 pos) {
    return 50*fBm(pos, 8, heightSeed, 64);
}

//desert
float dunes(glm::vec2 pos) {
    return 40*hybridMultifractal(pos, 8, heightSeed, 64);
}

//mountains
float mountains(glm::vec2 pos) {
    return 128*pow(hybridMultifractalM(pos, 8, heightSeed, 128), 2);
}

//UNUSED
//tall cone peaks, has issues with jagged terrain
float cone_mountains(glm::vec2 pos) {
    std::vector<std::pair<float, glm::vec2>> hillWeights = worleyNoise(pos, 9, glm::vec4(getSeed(2), getSeed(3), getSeed(4), getSeed(5)), 32);
    float ret = 0;
    for(std::pair<float, glm::vec2> p: hillWeights) {
        ret+=1/(1+p.first);
    }
    return 20*ret;
}

//perlin percentiles from 0-100%
const float p5P[21] = {0,
                       0.163355,
                       0.228847,
                       0.279731,
                       0.322539,
                       0.358845,
                       0.390918,
                       0.420682,
                       0.447783,
                       0.47443,
                       0.5,
                       0.525273,
                       0.552094,
                       0.579494,
                       0.608912,
                       0.640787,
                       0.677271,
                       0.719848,
                       0.771009,
                       0.837976,
                      1};


std::map<BiomeType, std::vector<std::pair<vec2, float(*)(vec2)>>> biomeErosion = {
    {TUNDRA, std::vector<std::pair<vec2, float(*)(vec2)>>{
        std::make_pair(vec2(p5P[0/5], p5P[20/5]), flat),
        std::make_pair(vec2(p5P[20/5], p5P[50/5]), hills),
        std::make_pair(vec2(p5P[50/5], p5P[70/5]), terraces),
        std::make_pair(vec2(p5P[70/5], p5P[100/5]), mountains)
    }},
    {PLAINS, std::vector<std::pair<vec2, float(*)(vec2)>>{
         std::make_pair(vec2(p5P[0/5], p5P[25/5]), flat),
         std::make_pair(vec2(p5P[25/5], p5P[70/5]), hills),
         std::make_pair(vec2(p5P[70/5], p5P[90/5]), terraces),
         std::make_pair(vec2(p5P[90/5], p5P[100/5]), mountains)
    }},
    {DESERT, std::vector<std::pair<vec2, float(*)(vec2)>>{
         std::make_pair(vec2(p5P[0/5], p5P[20/5]), flat),
         std::make_pair(vec2(p5P[20/5], p5P[30/5]), hills),
         std::make_pair(vec2(p5P[30/5], p5P[80/5]), dunes),
         std::make_pair(vec2(p5P[80/5], p5P[100/5]), terraces)
    }},
    {TAIGA, std::vector<std::pair<vec2, float(*)(vec2)>>{
        std::make_pair(vec2(p5P[0/5], p5P[10/5]), flat),
        std::make_pair(vec2(p5P[10/5], p5P[30/5]), hills),
        std::make_pair(vec2(p5P[30/5], p5P[80/5]), terraces),
        std::make_pair(vec2(p5P[80/5], p5P[100/5]), mountains)
    }},
    {SAVANNA, std::vector<std::pair<vec2, float(*)(vec2)>>{
        std::make_pair(vec2(p5P[0/5], p5P[25/5]), flat),
        std::make_pair(vec2(p5P[25/5], p5P[50/5]), hills),
        std::make_pair(vec2(p5P[50/5], p5P[70/5]), dunes),
        std::make_pair(vec2(p5P[70/5], p5P[80/5]), terraces),
        std::make_pair(vec2(p5P[80/5], p5P[100/5]), mountains)
    }},
    {FOREST, std::vector<std::pair<vec2, float(*)(vec2)>>{
         std::make_pair(vec2(p5P[0/5], p5P[15/5]), flat),
         std::make_pair(vec2(p5P[15/5], p5P[50/5]), hills),
         std::make_pair(vec2(p5P[50/5], p5P[90/5]), terraces),
         std::make_pair(vec2(p5P[90/5], p5P[100/5]), mountains)
    }},
    {SWAMP, std::vector<std::pair<vec2, float(*)(vec2)>>{
        std::make_pair(vec2(0, 1), flat)
    }},
    {RAINFOREST, std::vector<std::pair<vec2, float(*)(vec2)>>{
         std::make_pair(vec2(p5P[0/5], p5P[10/5]), flat),
         std::make_pair(vec2(p5P[10/5], p5P[30/5]), hills),
         std::make_pair(vec2(p5P[30/5], p5P[90/5]), terraces),
         std::make_pair(vec2(p5P[90/5], p5P[100/5]), mountains)
    }},
    {TEST_BIOME, std::vector<std::pair<vec2, float(*)(vec2)>>{
         std::make_pair(vec2(p5P[0/5], p5P[100/5]), superflat)
    }}
};

std::pair<BiomeType, std::vector<vec2>> biomeSpace[] = {
    std::make_pair(TUNDRA, std::vector<vec2>{vec2(0.42, 0.38), vec2(0.52, 0.42)}),
    std::make_pair(PLAINS, std::vector<vec2>{vec2(0.42, 0.54), vec2(0.52, 0.52)}),
    std::make_pair(DESERT, std::vector<vec2>{vec2(0.42, 0.67)}),
    std::make_pair(TAIGA, std::vector<vec2>{vec2(0.52, 0.46), vec2(0.57, 0.44)}),
    std::make_pair(SAVANNA, std::vector<vec2>{vec2(0.52, 0.69)}),
    std::make_pair(FOREST, std::vector<vec2>{vec2(0.57, 0.56), vec2(0.57, 0.68)}),
    std::make_pair(SWAMP, std::vector<vec2>{vec2(0.69, 0.58)}),
    std::make_pair(RAINFOREST, std::vector<vec2>{vec2(0.69, 0.68)})
};

//pretty laggy
float getBiomeHeight(float e, vec2 pp, std::vector<std::pair<vec2, float(*)(vec2)>> v){
    float ret = 0;
    float mag = 0;
    for(std::pair<vec2, float(*)(vec2)> p: v) {
        float d = 0.5*(p.first.x + p.first.y);
        float w = 0.5*(p.first.y - p.first.x);
        if(d == e){
            return p.second(pp);
        }
        float ww = w/abs(d-e);
        float www = pow(ww, 3);
        mag+=www;
        ret += p.second(pp)*www;
    }
    return ret/mag;
}

float generateErosion(vec2 pp) {
    return normPerlin(pp, vec4(getSeed(7.8), getSeed(5.6), getSeed(2.5), getSeed(4.6)), 1024);
//    setMultiFractalNoise(normPerlin);
//    return hybridMultifractal(pp);
}

std::pair<float, BiomeType> generateGround (vec2 pp) {
    //generate rainfall and temp
    vec2 q = vec2(fBm( pp + vec2(0.0,0.0), 8,  vec4(getSeed(3), getSeed(2), getSeed(3), getSeed(546)), 1024),
             fBm( pp + 500.f*vec2(-5,5), 8,  vec4(getSeed(3), getSeed(2), getSeed(3), getSeed(546)), 1024 ));

    vec2 r = vec2(fBm( pp + 4.f*q + 500.f*vec2(5,-5), 8,  vec4(getSeed(3), getSeed(2), getSeed(3), getSeed(523)), 1024  ),
             fBm( pp + 4.f*q + 500.f*vec2(-5,5), 8,  vec4(getSeed(3), getSeed(2), getSeed(3), getSeed(434)), 1024  ));

    float rain = length(q)/sqrt(2);
    float temp = length(r)/sqrt(2);

    float crain = rain * glm::sqrt(1.f - 0.5f*temp*temp);
    float ctemp = 1-(temp * glm::sqrt(1.f - 0.5f*rain*rain));

    //generate erosion
    float erosion = generateErosion(pp);

    float mag = 0;
    float adjmag = 0;
    float output = 0;

    //TO DO: remove this later
    if(TESTING) {
        return std::make_pair(getBiomeHeight(erosion, pp, biomeErosion[TEST_BIOME]), TEST_BIOME);
    }

    //calculate total distance magnitude
    for(std::pair<BiomeType, std::vector<vec2>> p: biomeSpace) {
        BiomeType b = p.first;
        for(vec2 rt: p.second) {
            float d = length(rt-vec2(crain,ctemp));
            if(d==0){
                return std::make_pair(getBiomeHeight(erosion, pp, biomeErosion[b]), b);
            }
            mag += 1/pow(d, 6);
        }
    }

    BiomeType bigb;
    float bigw = -1;
    for(std::pair<BiomeType, std::vector<vec2>> p: biomeSpace) {
        BiomeType b = p.first;
        float td = 0;
        for(vec2 rt: p.second) {
            td += 1/pow(length(rt-vec2(crain,ctemp)), 8);
        }
        //dont compute influences with < 10% to reduce unnecessary computation time
        if(td > mag*0.1) {
            output += td*getBiomeHeight(erosion, pp, biomeErosion[b]);
            adjmag += td;
        }
        if(td > bigw){
            bigw = td;
            bigb = b;
        }
    }

    //make rivers here
    float height = output/adjmag;
    float rivercoef = generateRiver(pp);
    float riverdepression = pow(clamp((float)(50*(abs(rivercoef-0.5)-0.007)), 0.f, 1.f), 3);
    //river depression needs to be 0 at 0.5+-, and grow to 1 at like 0.8
    if(rivercoef < 0.5+0.007 && rivercoef >0.5-0.007) {
        bigb = RIVER;
        //TO DO: replace height with a river bed calculation
        height = 0;
    }
    else {
        height*= riverdepression;
    }

    return std::make_pair(height, bigb);
}

float generateBedrock(vec2 pp){
    vec2 q, r;
    return warpPattern(pp, q, r, 12, 500, vec4(getSeed(2), getSeed(1), getSeed(3), getSeed(2)), 2048);
}

void erosionDist() {
    float foo[100000];
    for(int i = 0; i < 100000; i++) {
        float f = generateErosion(vec2(std::rand()%10000,std::rand()%10000));
        foo[i] = f;
    }
    std::sort(std::begin(foo), std::end(foo));
    for(int i = 0; i < 20; i++) {
        qDebug() << foo[i*100000/20] << ',';
    }
}

float generateBeach(vec2 pp) {
    return normPerlin(pp, heightSeed, 128);
}

float generateRiver(vec2 pp) {
    return fBm(pp, 8, heightSeed*4.f, 1024);
}


