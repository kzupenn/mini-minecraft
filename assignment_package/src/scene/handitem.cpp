#include "handitem.h"

HandItem::HandItem(OpenGLContext* context, glm::vec4 p) :
    Drawable(context), p(p)
{}

HandItem::~HandItem() {};

void HandItem::createVBOdata() {
    std::vector<glm::vec4> pos, uvs, nor, inter;
    std::vector<GLuint> idx;
    float w = 4;
    float dd = 4;
    float h = 12;
    //top
//    pos.emplace_back(glm::vec4(dd / 2, 0, w / 2, 1));
//    pos.emplace_back(glm::vec4(-dd / 2, 0, w / 2, 1));
//    pos.emplace_back(glm::vec4(-dd / 2, 0, -w / 2, 1));
//    pos.emplace_back(glm::vec4(dd/ 2, 0, -w / 2, 1));

    //bott
    pos.emplace_back(glm::vec4(dd / 2, -h, w / 2, 1));
    pos.emplace_back(glm::vec4(-dd / 2, -h, w / 2, 1));
    pos.emplace_back(glm::vec4(-dd / 2, -h, -w / 2, 1));
    pos.emplace_back(glm::vec4(dd / 2, -h, -w / 2, 1));

    //front
//    pos.emplace_back(glm::vec4(dd / 2, 0, -w / 2, 1));
//    pos.emplace_back(glm::vec4(dd / 2, 0, w / 2, 1));
//    pos.emplace_back(glm::vec4(dd / 2, -h, w / 2, 1));
//    pos.emplace_back(glm::vec4(dd / 2, -h, -w / 2, 1));

    //left
//    pos.emplace_back(glm::vec4(dd / 2, 0, w / 2, 1));
//    pos.emplace_back(glm::vec4(-dd / 2, 0, w / 2, 1));
//    pos.emplace_back(glm::vec4(-dd / 2, -h, w / 2, 1));
//    pos.emplace_back(glm::vec4(dd / 2, -h, w / 2, 1));

    //back
//    pos.emplace_back(glm::vec4(-dd / 2, 0, w / 2, 1));
//    pos.emplace_back(glm::vec4(-dd / 2, 0, -w / 2, 1));
//    pos.emplace_back(glm::vec4(-dd / 2, -h, -w / 2, 1));
//    pos.emplace_back(glm::vec4(-dd / 2, -h, w / 2, 1));

    //right
//    pos.emplace_back(glm::vec4(-dd / 2, 0, -w / 2, 1));
//    pos.emplace_back(glm::vec4(dd / 2, 0, -w / 2, 1));
//    pos.emplace_back(glm::vec4(dd / 2, -h, -w / 2, 1));
//    pos.emplace_back(glm::vec4(-dd / 2, -h, -w / 2, 1));

    //top
//    for (int i = 0; i < 4; i++) {
//        nor.emplace_back(0, 1, 0, 0);
//    }
//    //bott
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(0, -1, 0, 0);
    }
//    //+x=front
//    for (int i = 0; i < 4; i++) {
//        nor.emplace_back(1, 0, 0, 0);
//    }
//    //+z=left
//    for (int i = 0; i < 4; i++) {
//        nor.emplace_back(0, 0, 1, 0);
//    }
//    //-x=back
//    for (int i = 0; i < 4; i++) {
//        nor.emplace_back(-1, 0, 0, 0);
//    }
//    //-z=right
//    for (int i = 0; i < 4; i++) {
//        nor.emplace_back(0, 0, -1, 0);
//    }

    float d = 16.f/1024.f;
    //top
    uvs.emplace_back(p + glm::vec4(d, 0, 0, 0));
    uvs.emplace_back(p + glm::vec4(d, d, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, 0, 0, 0));

//    //bott
//    uvs.emplace_back(p + glm::vec4(d, d, 0, 0));
//    uvs.emplace_back(p + glm::vec4(d, 0, 0, 0));
//    uvs.emplace_back(p + glm::vec4(0, 0, 0, 0));
//    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));

//    //front
//    uvs.emplace_back(p + glm::vec4(d, d, 0, 0));
//    uvs.emplace_back(p + glm::vec4(d, d, 0, 0));
//    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));
//    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));

//    //right
//    uvs.emplace_back(p + glm::vec4(d, 0, 0, 0));
//    uvs.emplace_back(p + glm::vec4(d, 0, 0, 0));
//    uvs.emplace_back(p + glm::vec4(d, d, 0, 0));
//    uvs.emplace_back(p + glm::vec4(d, d, 0, 0));

//    //left
//    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));
//    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));
//    uvs.emplace_back(p + glm::vec4(0, 0, 0, 0));
//    uvs.emplace_back(p + glm::vec4(0, 0, 0, 0));

//    //back
//    uvs.emplace_back(p + glm::vec4(0, 0, 0, 0));
//    uvs.emplace_back(p + glm::vec4(0, 0, 0, 0));
//    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));
//    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));

    for(int i = 0; i < 1; i++){
        idx.push_back(i*4);
        idx.push_back(i*4+1);
        idx.push_back(i*4+2);
        idx.push_back(i*4);
        idx.push_back(i*4+2);
        idx.push_back(i*4+3);
    }


    for (int i = 0; i < pos.size(); i++) {
        inter.push_back(pos[i]);
        inter.push_back(nor[i]);
        inter.push_back(uvs[i]);
    }

    m_count = idx.size();

    generateInter();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInter);
    mp_context->glBufferData(GL_ARRAY_BUFFER, inter.size() * sizeof(glm::vec4), inter.data(), GL_STATIC_DRAW);

    generateIdx();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ARRAY_BUFFER, idx.size() * sizeof(glm::vec4), idx.data(), GL_STATIC_DRAW);
}

GLenum HandItem::drawMode() { return GL_TRIANGLES; }

