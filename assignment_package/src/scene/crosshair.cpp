#include "crosshair.h"

void Crosshair::createVBOdata() {
    std::vector<glm::vec4> pos, col;
    float l = 20;
    float w = 2;
    pos = {
        glm::vec4(-w, l, 0, 1),
        glm::vec4(w, l, 0, 1),
        glm::vec4(w, -l, 0, 1),
        glm::vec4(-w, -l, 0, 1),
        glm::vec4(-l, w, 0, 1),
        glm::vec4(l, w, 0, 1),
        glm::vec4(l, -w, 0, 1),
        glm::vec4(-l, -w, 0, 1)
    };
    for(int i = 0; i < pos.size(); i++) {
        col.emplace_back(1,1,1,1);
    }
    std::vector<int> idx = {0, 1, 2,
                           2, 3, 0,
                           4, 5, 6,
                           6, 7, 4};
    m_count = 12;

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);
    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);
    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

GLenum Crosshair::drawMode() {
    return GL_TRIANGLES;
}
