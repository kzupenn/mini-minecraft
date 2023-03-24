#include "worley.h"
#include "noise.h"
#include <vector>

using namespace glm;

struct less_than_key
{
    inline bool operator() (const std::pair<float, vec2>& struct1, const std::pair<float, vec2>& struct2)
    {
        return (struct1.first > struct2.first);
    }
};

//returns worley noise as a list of top x influences on a point
std::vector<std::pair<float, vec2> > worleyNoise(glm::vec2 pos, int maxout, vec4 seed, int gridSize) {
    float gridSizeWorley = 1.f/gridSize;
    pos/= gridSizeWorley;
    int wx = floor(pos[0]);
    int wy = floor(pos[1]);

    std::vector<std::pair<float, vec2>> ret;

    for(int i = wx - 1; i <= wx + 1; i++){
        for(int j = wy-1; j < wy + 2; j++) {
            //calculate centroid, using top left corner as reference to grid
            vec2 centerpt = vec2(i,j) + random2(vec2(i,j), seed);
            ret.push_back(std::make_pair(1.f/pow(glm::length(centerpt-pos), 3), (float)gridSizeWorley*centerpt));
        }
    }
    std::sort(ret.begin(), ret.end(), less_than_key());
    //resizes to fit maxout parameter
    ret.resize(std::max(1, std::min(maxout, 9)));
    //normalizes the weights
    float weightSum = 0;
    for(const std::pair<float, vec2> &p: ret){
        weightSum += p.first;
    }
    for(std::pair<float, vec2> &p: ret){
        p.first/=weightSum;
    }
    return ret;
}


