#include "handitem.h"

HandItem::HandItem(OpenGLContext* context, glm::vec4 p) :
    Drawable(context), p(p)
{}

HandItem::~HandItem() {};

void HandItem::createVBOdata() {
    std::vector<glm::vec4> pos, uvs, nor, inter;
    std::vector<GLuint> idx;

    //Front face
    pos.emplace_back(glm::vec4(1.f, 0.125f, 1.f, 1.f));
    //LR
    pos.emplace_back(glm::vec4(1.f, 0.0f, 1.f, 1.f));
    //LL
    pos.emplace_back(glm::vec4(0.0f, 0.0f, 1.f, 1.f));
    //UL
    pos.emplace_back(glm::vec4(0.0f, 0.125f, 1.f, 1.f));

    //Right face
    //UR
    pos.emplace_back(glm::vec4(1.f, 0.125f, 0.0f, 1.f));
    //LR
    pos.emplace_back(glm::vec4(1.f, 0.0f, 0.0f, 1.f));
    //LL
    pos.emplace_back(glm::vec4(1.f, 0.0f, 1.f, 1.f));
    //UL
    pos.emplace_back(glm::vec4(1.f, 0.125f, 1.f, 1.f));

    //Left face
    //UR
    pos.emplace_back(glm::vec4(0.0f, 0.125f, 1.f, 1.f));
    //LR
    pos.emplace_back(glm::vec4(0.0f, 0.0f, 1.f, 1.f));
    //LL
    pos.emplace_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.f));
    //UL
    pos.emplace_back(glm::vec4(0.0f, 0.125f, 0.0f, 1.f));

    //Back face
    //UR
    pos.emplace_back(glm::vec4(0.0f, 0.125f, 0.0f, 1.f));
    //LR
    pos.emplace_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.f));
    //LL
    pos.emplace_back(glm::vec4(1.f, 0.0f, 0.0f, 1.f));
    //UL
    pos.emplace_back(glm::vec4(1.f, 0.125f, 0.0f, 1.f));

    //Top face
    //UR
    pos.emplace_back(glm::vec4(1.f, 0.125f, 0.0f, 1.f));
    //LR
    pos.emplace_back(glm::vec4(1.f, 0.125f, 1.f, 1.f));
    //LL
    pos.emplace_back(glm::vec4(0.0f, 0.125f, 1.f, 1.f));
    //UL
    pos.emplace_back(glm::vec4(0.0f, 0.125f, 0.0f, 1.f));

    //Bottom face
    //UR
    pos.emplace_back(glm::vec4(1.f, 0.0f, 1.f, 1.f));
    //LR
    pos.emplace_back(glm::vec4(1.f, 0.0f, 0.0f, 1.f));
    //LL
    pos.emplace_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.f));
    //UL
    pos.emplace_back(glm::vec4(0.0f, 0.0f, 1.f, 1.f));

    //front
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(0, 0, 1, 0);
    }
    //right
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(1, 0, 0, 0);
    }
    //left
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(-1, 0, 0, 0);
    }
    //back
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(0, 0, -1, 0);
    }
    //top
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(0, 1, 0, 0);
    }
    //bottom
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(0, -1, 0, 0);
    }

    float d = 16.f/1024.f;
    //front
    uvs.emplace_back(p + glm::vec4(d, d, 0, 0));
    uvs.emplace_back(p + glm::vec4(d, d, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));

    //right
    uvs.emplace_back(p + glm::vec4(d, 0, 0, 0));
    uvs.emplace_back(p + glm::vec4(d, 0, 0, 0));
    uvs.emplace_back(p + glm::vec4(d, d, 0, 0));
    uvs.emplace_back(p + glm::vec4(d, d, 0, 0));

    //left
    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, 0, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, 0, 0, 0));

    //back
    uvs.emplace_back(p + glm::vec4(0, 0, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, 0, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));

    //top
    uvs.emplace_back(p + glm::vec4(d, 0, 0, 0));
    uvs.emplace_back(p + glm::vec4(d, d, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, 0, 0, 0));

    //bott
    uvs.emplace_back(p + glm::vec4(d, d, 0, 0));
    uvs.emplace_back(p + glm::vec4(d, 0, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, 0, 0, 0));
    uvs.emplace_back(p + glm::vec4(0, d, 0, 0));


    for(int i = 0; i < 6; i++){
        idx.emplace_back(i*4);
        idx.emplace_back(i*4+1);
        idx.emplace_back(i*4+1);
        idx.emplace_back(i*4+2);
        idx.emplace_back(i*4+2);
        idx.emplace_back(i*4+3);
        idx.emplace_back(i*4+3);
        idx.emplace_back(i*4);
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

