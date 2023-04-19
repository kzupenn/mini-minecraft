#include "icons.h"

void Heart::createVBOdata() {
    std::vector<glm::vec4> VBOpos, uvs, col;
    std::vector<int> idx = {0, 1, 2, 2, 3, 0};
    m_count = 6;
    VBOpos = {glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 1), glm::vec4(0, 1, 0, 1)};
    glm::vec4 blc = glm::vec4(48.f/768.f, 742.f/768.f,0,1);
    uvs = {blc, blc+glm::vec4(26.f/768.f, 0, 0, 0), blc+glm::vec4(26.f/768.f, 26.f/768.f,0,0), blc+glm::vec4(0, 26.f/768.f, 0, 0)};
    col = {glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1)};

    generateIdx();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, VBOpos.size() * sizeof(glm::vec4), VBOpos.data(), GL_STATIC_DRAW);

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec4), uvs.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}
GLenum Heart::drawMode() {
    return GL_TRIANGLES;
}
void FullHeart::createVBOdata() {
    std::vector<glm::vec4> VBOpos, uvs, col;
    std::vector<int> idx = {0, 1, 2, 2, 3, 0};
    m_count = 6;
    VBOpos = {glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 1), glm::vec4(0, 1, 0, 1)};
    glm::vec4 blc = glm::vec4(156.f/768.f, 742.f/768.f,0,1);
    uvs = {blc, blc+glm::vec4(26.f/768.f, 0, 0, 0), blc+glm::vec4(26.f/768.f, 26.f/768.f,0,0), blc+glm::vec4(0, 26.f/768.f, 0, 0)};
    col = {glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1)};

    generateIdx();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, VBOpos.size() * sizeof(glm::vec4), VBOpos.data(), GL_STATIC_DRAW);

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec4), uvs.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}
GLenum FullHeart::drawMode() {
    return GL_TRIANGLES;
}
void HalfHeart::createVBOdata() {
    std::vector<glm::vec4> VBOpos, uvs, col;
    std::vector<int> idx = {0, 1, 2, 2, 3, 0};
    m_count = 6;
    VBOpos = {glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 1), glm::vec4(0, 1, 0, 1)};
    glm::vec4 blc = glm::vec4(182.f/768.f, 742.f/768.f,0,1);
    uvs = {blc, blc+glm::vec4(26.f/768.f, 0, 0, 0), blc+glm::vec4(26.f/768.f, 26.f/768.f,0,0), blc+glm::vec4(0, 26.f/768.f, 0, 0)};
    col = {glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1)};

    generateIdx();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, VBOpos.size() * sizeof(glm::vec4), VBOpos.data(), GL_STATIC_DRAW);

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec4), uvs.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}
GLenum HalfHeart::drawMode() {
    return GL_TRIANGLES;
}

void Armor::createVBOdata() {
    std::vector<glm::vec4> VBOpos, uvs, col;
    std::vector<int> idx = {0, 1, 2, 2, 3, 0};
    m_count = 6;
    VBOpos = {glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 1), glm::vec4(0, 1, 0, 1)};
    glm::vec4 blc = glm::vec4(48.f/768.f, 715.f/768.f,0,1);
    uvs = {blc, blc+glm::vec4(26.f/768.f, 0, 0, 0), blc+glm::vec4(26.f/768.f, 26.f/768.f,0,0), blc+glm::vec4(0, 26.f/768.f, 0, 0)};
    col = {glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1)};

    generateIdx();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, VBOpos.size() * sizeof(glm::vec4), VBOpos.data(), GL_STATIC_DRAW);

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec4), uvs.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}
GLenum Armor::drawMode() {
    return GL_TRIANGLES;
}
void FullArmor::createVBOdata() {
    std::vector<glm::vec4> VBOpos, uvs, col;
    std::vector<int> idx = {0, 1, 2, 2, 3, 0};
    m_count = 6;
    VBOpos = {glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 1), glm::vec4(0, 1, 0, 1)};
    glm::vec4 blc = glm::vec4(100.f/768.f, 715.f/768.f,0,1);
    uvs = {blc, blc+glm::vec4(26.f/768.f, 0, 0, 0), blc+glm::vec4(26.f/768.f, 26.f/768.f,0,0), blc+glm::vec4(0, 26.f/768.f, 0, 0)};
    col = {glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1)};

    generateIdx();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, VBOpos.size() * sizeof(glm::vec4), VBOpos.data(), GL_STATIC_DRAW);

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec4), uvs.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}
GLenum FullArmor::drawMode() {
    return GL_TRIANGLES;
}
void HalfArmor::createVBOdata() {
    std::vector<glm::vec4> VBOpos, uvs, col;
    std::vector<int> idx = {0, 1, 2, 2, 3, 0};
    m_count = 6;
    VBOpos = {glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 1), glm::vec4(0, 1, 0, 1)};
    glm::vec4 blc = glm::vec4(74.f/768.f, 715.f/768.f,0,1);
    uvs = {blc, blc+glm::vec4(26.f/768.f, 0, 0, 0), blc+glm::vec4(26.f/768.f, 26.f/768.f,0,0), blc+glm::vec4(0, 26.f/768.f, 0, 0)};
    col = {glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1)};

    generateIdx();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, VBOpos.size() * sizeof(glm::vec4), VBOpos.data(), GL_STATIC_DRAW);

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec4), uvs.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}
GLenum HalfArmor::drawMode() {
    return GL_TRIANGLES;
}
