#include "prism.h"

Prism::Prism(OpenGLContext* context, glm::ivec3 dim,
             glm::ivec2 p1, glm::ivec2 p2) :
    Drawable(context),
    w(dim.x),
    h(dim.y),
    d(dim.z),
    p1(p1),
    p2(p2)
{}

Prism::~Prism() {}

GLenum Prism::drawMode() {
    return GL_TRIANGLES;
}

void Prism::setHit(bool h) {
    hit = h;
}

void Prism::createVBOdata() {
    std::vector<glm::vec4> pos, nor, uvs, inter;
    std::vector<GLuint> idx;
    //top
    pos.emplace_back(glm::vec4(d / 2, 0, w / 2, 1));
    pos.emplace_back(glm::vec4(-d / 2, 0, w / 2, 1));
    pos.emplace_back(glm::vec4(-d / 2, 0, -w / 2, 1));
    pos.emplace_back(glm::vec4(d / 2, 0, -w / 2, 1));

    //bott
    pos.emplace_back(glm::vec4(d / 2, -h, w / 2, 1));
    pos.emplace_back(glm::vec4(-d / 2, -h, w / 2, 1));
    pos.emplace_back(glm::vec4(-d / 2, -h, -w / 2, 1));
    pos.emplace_back(glm::vec4(d / 2, -h, -w / 2, 1));

    //front
    pos.emplace_back(glm::vec4(d / 2, 0, -w / 2, 1));
    pos.emplace_back(glm::vec4(d / 2, 0, w / 2, 1));
    pos.emplace_back(glm::vec4(d / 2, -h, w / 2, 1));
    pos.emplace_back(glm::vec4(d / 2, -h, -w / 2, 1));

    //left
    pos.emplace_back(glm::vec4(d / 2, 0, w / 2, 1));
    pos.emplace_back(glm::vec4(-d / 2, 0, w / 2, 1));
    pos.emplace_back(glm::vec4(-d / 2, -h, w / 2, 1));
    pos.emplace_back(glm::vec4(d / 2, -h, w / 2, 1));

    //back
    pos.emplace_back(glm::vec4(-d / 2, 0, w / 2, 1));
    pos.emplace_back(glm::vec4(-d / 2, 0, -w / 2, 1));
    pos.emplace_back(glm::vec4(-d / 2, -h, -w / 2, 1));
    pos.emplace_back(glm::vec4(-d / 2, -h, w / 2, 1));

    //right
    pos.emplace_back(glm::vec4(-d / 2, 0, -w / 2, 1));
    pos.emplace_back(glm::vec4(d / 2, 0, -w / 2, 1));
    pos.emplace_back(glm::vec4(d / 2, -h, -w / 2, 1));
    pos.emplace_back(glm::vec4(-d / 2, -h, -w / 2, 1));

    //top
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(0, 1, 0, 0);
    }
    //bott
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(0, -1, 0, 0);
    }
    //+x=front
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(1, 0, 0, 0);
    }
    //+z=left
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(0, 0, 1, 0);
    }
    //-x=back
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(-1, 0, 0, 0);
    }
    //-z=right
    for (int i = 0; i < 4; i++) {
        nor.emplace_back(0, 0, -1, 0);
    }

    glm::vec4 b = glm::vec4(p1.x, p1.y, 0, 1);

    //top
    uvs.emplace_back(b + glm::vec4(d + w, d - 1, 0, 0));
    uvs.emplace_back(b + glm::vec4(d + w, 0, 0, 0));
    uvs.emplace_back(b + glm::vec4(d, 0, 0, 0));
    uvs.emplace_back(b + glm::vec4(d, d - 1, 0, 0));

    //bott
    uvs.emplace_back(b + glm::vec4(d + 2 * w, d - 1, 0, 0));
    uvs.emplace_back(b + glm::vec4(d + 2 * w, 0, 0, 0));
    uvs.emplace_back(b + glm::vec4(d + w, 0, 0, 0));
    uvs.emplace_back(b + glm::vec4(d + w, d - 1, 0, 0));

    //front
    uvs.emplace_back(b + glm::vec4(d, d, 0, 0));
    uvs.emplace_back(b + glm::vec4(d + w, d, 0, 0));
    uvs.emplace_back(b + glm::vec4(d + w, d + h, 0, 0));
    uvs.emplace_back(b + glm::vec4(d, d + h, 0, 0));

    //left
    uvs.emplace_back(b + glm::vec4(d + w, d, 0, 0));
    uvs.emplace_back(b + glm::vec4(w + 2 * d, d, 0, 0));
    uvs.emplace_back(b + glm::vec4(w + 2 * d, d + h, 0, 0));
    uvs.emplace_back(b + glm::vec4(d + w, d + h, 0, 0));

    //back
    uvs.emplace_back(b + glm::vec4(w + 2 * d, d, 0, 0));
    uvs.emplace_back(b + glm::vec4(2 * (w + d), d, 0, 0));
    uvs.emplace_back(b + glm::vec4(2 * (w + d), d + h, 0, 0));
    uvs.emplace_back(b + glm::vec4(w + 2 * d, d + h, 0, 0));

    //right
    uvs.emplace_back(b + glm::vec4(0, d, 0, 0));
    uvs.emplace_back(b + glm::vec4(d, d, 0, 0));
    uvs.emplace_back(b + glm::vec4(d, d + h, 0, 0));
    uvs.emplace_back(b + glm::vec4(0, d + h, 0, 0));

    if (hit) for (auto &u : uvs) u.w = 1;

    for(int i = 0; i < 6; i++){
        idx.push_back(i*4);
        idx.push_back(i*4+1);
        idx.push_back(i*4+2);
        idx.push_back(i*4);
        idx.push_back(i*4+2);
        idx.push_back(i*4+3);
    }

    float tx = 64.f;
    for (int i = 0; i < pos.size(); i++) {
       inter.push_back(pos[i]);
       inter.push_back(nor[i]);
       uvs[i].y = tx - uvs[i].y;
       inter.push_back(uvs[i] / tx);
    }

    m_count = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()* sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generateInter();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInter);
    mp_context->glBufferData(GL_ARRAY_BUFFER, inter.size() * sizeof(glm::vec4), inter.data(), GL_STATIC_DRAW);
}
