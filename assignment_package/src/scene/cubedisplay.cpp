#include "cubedisplay.h"

CubeDisplay::CubeDisplay(OpenGLContext* context) :
    Drawable(context)
{}

CubeDisplay::~CubeDisplay() {};

void CubeDisplay::createVBOdata() {
    std::vector<glm::vec4> pos, col;
    std::vector<GLuint> idx;

    //Front face
    pos.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    //LR
    pos.emplace_back(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
    //LL
    pos.emplace_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    //UL
    pos.emplace_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));

    //Right face
    //UR
    pos.emplace_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    //LR
    pos.emplace_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    //LL
    pos.emplace_back(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
    //UL
    pos.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    //Left face
    //UR
    pos.emplace_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
    //LR
    pos.emplace_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    //LL
    pos.emplace_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    //UL
    pos.emplace_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

    //Back face
    //UR
    pos.emplace_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    //LR
    pos.emplace_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    //LL
    pos.emplace_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    //UL
    pos.emplace_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

    //Top face
    //UR
    pos.emplace_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    //LR
    pos.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    //LL
    pos.emplace_back(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
    //UL
    pos.emplace_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

    //Bottom face
    //UR
    pos.emplace_back(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
    //LR
    pos.emplace_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    //LL
    pos.emplace_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    //UL
    pos.emplace_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

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
        col.emplace_back(glm::vec4(1, 1, 1, 1));
    }

    m_count = idx.size();

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);

    generateIdx();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ARRAY_BUFFER, idx.size() * sizeof(glm::vec4), idx.data(), GL_STATIC_DRAW);


}

GLenum CubeDisplay::drawMode() { return GL_LINES; }
