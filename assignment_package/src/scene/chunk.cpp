#include "chunk.h"
#include <QDebug>

void printVec(glm::vec4 a) {
    qDebug() << a[0] << a[1] << a[2] << a[3];
}

bool isTransparent(BlockType b) {
    if(b == EMPTY){
        return true;
    }
    return false;
}

glm::vec3 dirToVec(Direction d) {
    switch(d) {
    case XPOS:
    return glm::vec3(1,0,0);
    case XNEG:
    return glm::vec3(-1,0,0);
    case YPOS:
    return glm::vec3(0,1,0);
    case YNEG:
    return glm::vec3(0,-1,0);
    case ZPOS:
    return glm::vec3(0,0,1);
    case ZNEG:
    return glm::vec3(0,0,-1);
    default:
    return glm::vec3(0,0,0);
    }
}

Direction vecToDir(glm::vec3 v) {
    if(v.x == -1) {
        return XNEG;
    }
    else if(v.x == 1) {
        return XPOS;
    }
    else if(v.y == -1) {
        return YNEG;
    }
    else if(v.y == 1) {
        return YPOS;
    }
    else if(v.z == -1) {
        return ZNEG;
    }
    else {
        return ZPOS;
    }
}

Chunk::Chunk(OpenGLContext* mp_context) : Drawable(mp_context), m_blocks(), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}},
    dataBound(false), dataGen(false), surfaceGen(false)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    if(y > 500 && y < 1500) y+= heightMap[x][z]-1000;
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    if(y > 500 && y < 1500) y+= heightMap[x][z]-1000;
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    try{
        setBlock_mutex.lock();
        //if y is in this range, means that we want to take a delta of height map instead
        if(y > 500 && y < 1500) y += heightMap[x][z]-1000;
        m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
        setBlock_mutex.unlock();

        blocksChanged = true;
    }
    catch(...){
        qDebug() << x << y << z;
        throw std::invalid_argument( "chunk set block received faulty values" );
    }
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

const int delta[] = {1, 0, 0,
               -1, 0, 0,
               0, 1, 0,
               0, -1, 0,
               0, 0, 1,
               0, 0, -1};
const int facedeltas[] = {
    0, 0, 0,
    0, 1, 0,
    0, 1, 1,
    0, 0, 1,

    0, 0, 0,
    1, 0, 0,
    1, 0, 1,
    0, 0, 1,

    0, 0, 0,
    1, 0, 0,
    1, 1, 0,
    0, 1, 0
};
void Chunk::createVBOdata() {
//    int faces = 0;
    createVBO_mutex.lock();
    VBOpos.clear();
    VBOnor.clear();
    idx.clear();
    VBOcol.clear();

    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 256; j++) {
            for(int k = 0; k < 16; k++) {
                if(getBlockAt(i, j, k)!=EMPTY) {
                    //check in all 6 directions
                    for(int l = 0; l < 6*3; l+=3) {
                        //bound checking and neighbor
                        bool drawFace = false;
                        if(i+delta[l] < 0){
                            drawFace = (!m_neighbors[XNEG] || isTransparent(m_neighbors[XNEG]->getBlockAt(15, j, k)));
                        }
                        else if(i+delta[l] > 15){
                            drawFace = (!m_neighbors[XPOS] || isTransparent(m_neighbors[XPOS]->getBlockAt(0, j, k)));
                        }
                        else if(j+delta[l+1] < 0 || j+delta[l+1] > 255){
                            drawFace = true;
                        }
                        else if(k+delta[l+2] < 0){
                            drawFace = (!m_neighbors[ZNEG] || isTransparent(m_neighbors[ZNEG]->getBlockAt(i, j, 15)));
                        }
                        else if(k+delta[l+2] > 15){
                            drawFace = (!m_neighbors[ZPOS] || isTransparent(m_neighbors[ZPOS]->getBlockAt(i, j, 0)));
                        }
                        else if(isTransparent(getBlockAt(i+delta[l], j+delta[l+1], k+delta[l+2]))){
                            drawFace = true;
                        }
                        if(drawFace){
                            //set indices
                            idx.push_back(VBOpos.size());
                            idx.push_back(VBOpos.size()+1);
                            idx.push_back(VBOpos.size()+2);
                            idx.push_back(VBOpos.size()+2);
                            idx.push_back(VBOpos.size()+3);
                            idx.push_back(VBOpos.size());
                            //set surface positions
                            glm::vec4 faceref = pos + glm::vec4(i+fmax(0, delta[l]), j+fmax(0, delta[l+1]), k+fmax(0, delta[l+2]), 1);
                            VBOpos.push_back(faceref + glm::vec4(facedeltas[(l/6)*12], facedeltas[(l/6)*12+1], facedeltas[(l/6)*12+2], 0));
                            VBOpos.push_back(faceref + glm::vec4(facedeltas[(l/6)*12+3], facedeltas[(l/6)*12+4], facedeltas[(l/6)*12+5], 0));
                            VBOpos.push_back(faceref + glm::vec4(facedeltas[(l/6)*12+6], facedeltas[(l/6)*12+7], facedeltas[(l/6)*12+8], 0));
                            VBOpos.push_back(faceref + glm::vec4(facedeltas[(l/6)*12+9], facedeltas[(l/6)*12+10], facedeltas[(l/6)*12+11], 0));
                            //set surface normals
                            VBOnor.push_back(glm::vec4(delta[l], delta[l+1], delta[l+2], 1));
                            VBOnor.push_back(glm::vec4(delta[l], delta[l+1], delta[l+2], 1));
                            VBOnor.push_back(glm::vec4(delta[l], delta[l+1], delta[l+2], 1));
                            VBOnor.push_back(glm::vec4(delta[l], delta[l+1], delta[l+2], 1));
                            //colors, TODO
                            glm::vec4 this_color;
                            switch(getBlockAt(i, j, k)){
                            case GRASS:
                                this_color = glm::vec4(0, 1, 0, 1);
                                break;
                            case DIRT:
                                this_color = glm::vec4(181.f, 155.f, 90.f, 255.f)/255.f;
                                break;
                            case STONE:
                                this_color = glm::vec4(0.5, 0.5, 0.5, 1);
                                break;
                            case WATER:
                                this_color = glm::vec4(0, 0, 1, 0.5);
                                break;
                            case SAND:
                                this_color = glm::vec4(1,1,0,1);
                                break;
                            case SNOW:
                                this_color = glm::vec4(1,1,1,1);
                                break;
                            case COBBLESTONE:
                                this_color = glm::vec4(0.5, 0.5, 0.5, 1);
                                break;
                            case OAK_PLANKS:
                                this_color = glm::vec4(221.f, 195.f, 130.f, 255.f)/255.f;
                                break;
                            case OAK_LOG:
                                this_color = glm::vec4(151.f, 125.f, 60.f, 255.f)/255.f;
                                break;
                            case OAK_LEAVES:
                                this_color = glm::vec4(0, 0.5, 0, 1);
                                break;
                            default:
                                this_color = glm::vec4(0);
                                break;
                            }
                            for(int foo = 0; foo < 4; foo++) {
                                VBOcol.push_back(this_color);
                                //VBOcol.push_back(glm::vec4(j/256.f, j/256.f, j/256.f, 1));
                            }

//                            VBOcol.push_back(glm::vec4(abs(delta[l]), abs(delta[l+1]), abs(delta[l+2]), 1));
//                            VBOcol.push_back(glm::vec4(abs(delta[l]), abs(delta[l+1]), abs(delta[l+2]), 1));
//                            VBOcol.push_back(glm::vec4(abs(delta[l]), abs(delta[l+1]), abs(delta[l+2]), 1));
//                            VBOcol.push_back(glm::vec4(abs(delta[l]), abs(delta[l+1]), abs(delta[l+2]), 1));
//                            VBOcol.push_back(glm::vec4(debugColor, 1));
//                            VBOcol.push_back(glm::vec4(debugColor, 1));
//                            VBOcol.push_back(glm::vec4(debugColor, 1));
//                            VBOcol.push_back(glm::vec4(debugColor, 1));
                        }
                    }
                }
            }
        }
    }

    createVBO_mutex.unlock();

    //tells the main thread to bind to vbo
    dataGen = true;
    dataBound = false;
}

void Chunk::bindVBOdata() {
    createVBO_mutex.lock();
    m_count = idx.size();

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, VBOpos.size() * sizeof(glm::vec4), VBOpos.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, VBOnor.size() * sizeof(glm::vec4), VBOnor.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, VBOcol.size() * sizeof(glm::vec4), VBOcol.data(), GL_STATIC_DRAW);

    dataBound = true;
    createVBO_mutex.unlock();
}

void Chunk::unbindVBOdata() {
    createVBO_mutex.lock();
    m_count = 0;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), nullptr, GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, VBOpos.size() * sizeof(glm::vec4), nullptr, GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, VBOnor.size() * sizeof(glm::vec4), nullptr, GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, VBOcol.size() * sizeof(glm::vec4), nullptr, GL_STATIC_DRAW);

    dataBound = false;
    createVBO_mutex.unlock();
}

void Chunk::setPos(int x, int z) {
    pos = glm::vec4(x, 0, z, 0);
}

//void Chunk::setBiome(BiomeType input) {
//    biome = input;
//}

GLenum Chunk::drawMode(){
    return GL_TRIANGLES;
}
