#include "rectangle.h"

void Rectangle::createVBOdata() {
    std::vector<glm::vec4> pos, col;
    pos = {
        glm::vec4(0,0,0,1),
        glm::vec4(0,1,0,1),
        glm::vec4(1,1,0,1),
        glm::vec4(1,0,0,1)
    };
    for(int i = 0; i < pos.size(); i++) {
        col.emplace_back(0.2,0.2,0.2,0.5);
    }
    std::vector<int> idx = {0, 1, 2,
                           2, 3, 0};
    m_count = 6;

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);
    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);
    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

GLenum Rectangle::drawMode() {
    return GL_TRIANGLES;
}
