#include "inventory.h"

GLenum Inventory::drawMode() {
    return GL_TRIANGLES;
}

void Inventory::createVBOdata() {
    std::vector<glm::vec4> pos, uvs;
    std::vector<int> idx = {0,1,2,2,3,0};

    pos = {glm::vec4(-200, -200, 0, 1), glm::vec4(-200, -200, 0, 1), glm::vec4(-200, -200, 0, 1), glm::vec4(-200, -200, 0, 1)};
    uvs = {glm::vec4(0, 0, 0, 1),glm::vec4(0, 1, 0, 1),glm::vec4(1, 1, 0, 1),glm::vec4(1, 0, 0, 1)};

    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec4), uvs.data(), GL_STATIC_DRAW);
}
