#include "biome.h"
#include "algo/fractal.h"
#include "algo/noise.h"
#include "algo/perlin.h"
#include "algo/worley.h"
#include "terrain.h"
#include "algo/seed.h"
#include <map>

using namespace glm;

const bool TESTING = true;

//terrain generation
//makes sporatic divets to make smooth terrain appear with erosion
float divet(glm::vec2 pos) {
    return -20*max(0.f, fBm(pos, 4, vec4(SEED.getSeed(7061.801,6269.454,594.208,8654.859)), 50)-0.57f);
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
    std::vector<std::pair<float, glm::vec2>> hillWeights = worleyNoise(pos, 9, glm::vec4(SEED.getSeed(8199.337,7556.696,704.539,8315.043)), 32);
    float ret = 0;
    for(std::pair<float, glm::vec2> p: hillWeights) {
        ret+=pow(glm::tanh(1.5*p.first-1.5)*0.5+0.5, 4);
    }

    //return true height
    return 6*ret-6;
}

//valley?

float terraces(glm::vec2 pos) {
    return 50*fBm(pos, 8, SEED.getSeed(9197.192,4666.678,4885.874,2264.0), 64);
}

//desert
float dunes(glm::vec2 pos) {
    return 40*hybridMultifractal(pos, 8, SEED.getSeed(6394.685,9164.769,6711.066,8576.487), 64);
}

//mountains
float mountains(glm::vec2 pos) {
    return clamp((float)(200*pow(hybridMultifractalM(pos, 8, SEED.getSeed(3592.123,9767.551,8272.77,2613.673), 256), 2) + 30*fBm(pos, 8, SEED.getSeed(9448.479,7845.191,1589.323,2670.333), 128)), 0.f, 255.f);
}

//UNUSED
//tall cone peaks, has issues with jagged terrain
float cone_mountains(glm::vec2 pos) {
    std::vector<std::pair<float, glm::vec2>> hillWeights = worleyNoise(pos, 9, SEED.getSeed(9304.527, 4757.075, 8550.053, 3560.767), 32);
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
    std::make_pair(TUNDRA, std::vector<vec2>{vec2(0.42, 0.35), vec2(0.52, 0.3)}),
    std::make_pair(PLAINS, std::vector<vec2>{vec2(0.42, 0.5), vec2(0.52, 0.4)}),
    std::make_pair(DESERT, std::vector<vec2>{vec2(0.42, 0.7)}),
    std::make_pair(TAIGA, std::vector<vec2>{vec2(0.52, 0.5), vec2(0.57, 0.4)}),
    std::make_pair(SAVANNA, std::vector<vec2>{vec2(0.52, 0.6)}),
    std::make_pair(FOREST, std::vector<vec2>{vec2(0.57, 0.5), vec2(0.57, 0.68)}),
    std::make_pair(SWAMP, std::vector<vec2>{vec2(0.69, 0.58)}),
    std::make_pair(RAINFOREST, std::vector<vec2>{vec2(0.69, 0.68)})
};

//pretty laggy
float getBiomeHeight(float e, vec2 pp, std::vector<std::pair<vec2, float(*)(vec2)>> &v){
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
    return normPerlin(pp, SEED.getSeed(9441.693,326.184,2558.757,8946.046), 1024);
//    setMultiFractalNoise(normPerlin);
//    return hybridMultifractal(pp);
}

std::pair<float, BiomeType> generateGround (vec2 pp) {
    float rain = fBm(pp, 12, SEED.getSeed(3885.207,977.211,3437.496,3504.515), 4096);
    float temp = fBm(pp, 12, SEED.getSeed(5716.522,6415.354,3175.466,3309.938), 4096);

    float crain = rain * std::sqrt(1 - 0.5*temp*temp);
    float ctemp = 1-(temp * std::sqrt(1 - 0.5*rain*rain));
    //qDebug() << "R/T" << crain << ctemp;
    //generate erosion
    float erosion = generateErosion(pp);

    float mag = 0;
    float adjmag = 0;
    float output = 0;

    //TO DO: remove this later
    if(TESTING) {
        std::pair<float, BiomeType> testret = std::make_pair(getBiomeHeight(erosion, pp, biomeErosion[TUNDRA]), TUNDRA);
        float height = output/adjmag;
        float rivercoef = generateRiver(pp);
        float riverdepression = pow(clamp((float)(50*(abs(rivercoef-0.5)-river_width)), 0.f, 1.f), 3);
        //river depression needs to be 0 at 0.5+-, and grow to 1 at like 0.8
        if(rivercoef < 0.5+river_width && rivercoef >0.5-river_width) {
            testret.second = RIVER;
            //TO DO: replace height with a river bed calculation
            testret.first = 0;
        }
        else {
            testret.first*= riverdepression;
        }
        return testret;
    }

    //calculate total distance magnitude
    for(std::pair<BiomeType, std::vector<vec2>> p: biomeSpace) {
        BiomeType b = p.first;
        for(vec2 rt: p.second) {
            float d = length(rt-vec2(crain,ctemp));
            if(d==0){
                mag += 1000000000;
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
    float riverdepression = pow(clamp((float)(50*(abs(rivercoef-0.5)-river_width)), 0.f, 1.f), 3);
    //river depression needs to be 0 at 0.5+-, and grow to 1 at like 0.8
    if(rivercoef < 0.5+river_width && rivercoef >0.5-river_width) {
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
    return clamp((float)(2*abs(warpPattern(pp, q, r, 12, 500, SEED.getSeed(2388.099,6949.378,3298.059,7308.408), 2048)-0.5)),
                 0.f, 1.f);
}

float generateBeach(vec2 pp) {
    return normPerlin(pp, SEED.getSeed(6391.105,5259.751,5585.325,5970.867), 128);
}

float generateRiver(vec2 pp) {
    float cmax = -1;
    float r_width = 5.f;

    for(int i = pp.x-r_width; i <= pp.x+r_width; i++) {
        for(int j = pp.y-r_width; j <= pp.y+r_width; j++) {
            float d = glm::sqrt(abs(pp.x-i)*abs(pp.x-i)+abs(pp.y-j)*abs(pp.y-j));
            if(d <= r_width) {
                float c = fBm(glm::vec2(i,j), 8, SEED.getSeed(8702.024,9507.16,44.434,1153.193)*4.f, 512);
                float m = 1.f/(1+ d*0.33);
                c = 0.5 + (c-0.5)*1;
                if(abs(c-0.5f) < abs(cmax-0.5f)) cmax = c;
            }
        }
    }
    return cmax;
}

//
float generateSnowLayer(vec2 pp) {
    return 128+20*perlinNoise(pp, SEED.getSeed(748.852,4942.126,34.161,5958.106), 32);
}
float generateRockLayer(vec2 pp) {
    return 100+10*perlinNoise(pp, SEED.getSeed(8008.714,9810.119,9169.679,9032.367), 64);
}
float generateCaves(vec3 pp) {
    return fBm(pp, 4, SEED.getSeed(60.714,45.119,99.679,20.367), 1024);
    //return perlinNoise3D(pp, SEED.getSeed(6230.714,4545.119,9169.679,2300.367), 128);
}

//noise distribution tests

void bedrockTest() {
    float foo[100000];
    for(int i = 0; i < 100000; i++) {
        float f = generateBedrock(vec2(std::rand()%10000,std::rand()%10000));
        foo[i] = f;
    }
    std::sort(std::begin(foo), std::end(foo));
    for(int i = 0; i < 20; i++) {
        qDebug() << i*5 << foo[i*100000/20];
    }
}
void biomeTest() {
    int b[9] = {0,0,0,0,0,0,0,0,0};
    for(int i = 0; i < 100000; i++) {
        vec2 pp = vec2(std::rand()%100000,std::rand()%100000);
        float rain = fBm(pp, 12, SEED.getSeed(3885.207,977.211,3437.496,3504.515), 2048);
        float temp = fBm(pp, 12, SEED.getSeed(5716.522,6415.354,3175.466,3309.938), 2048);

        float crain = rain * std::sqrt(1 - 0.5*temp*temp);
        float ctemp = 1-(temp * std::sqrt(1 - 0.5*rain*rain));

        float maxmag = -1;
        int bigb = -1;
        for(std::pair<BiomeType, std::vector<vec2>> p: biomeSpace) {
            BiomeType b = p.first;
            for(vec2 rt: p.second) {
                float d = length(rt-vec2(crain,ctemp));
                float thismag = 1/pow(d, 6);
                if(thismag > maxmag) {
                    maxmag = thismag;
                    bigb = b;
                }
            }
        }
        b[bigb]++;
    }
    qDebug() << "TUNDRA" << b[0];
    qDebug() << "PLAINS" << b[1];
    qDebug() << "DESERT" << b[2];
    qDebug() << "TAIGA" << b[3];
    qDebug() << "SAVANNA" << b[4];
    qDebug() << "FOREST" << b[5];
    qDebug() << "SWAMP" << b[6];
    qDebug() << "RAINFOREST" << b[7];
    qDebug() << "RIVER" << b[8];

}
void biomeDist() {
    //biomeTest();
    bedrockTest();
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


