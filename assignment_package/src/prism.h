#pragma once
#include <glm_includes.h>
#include "drawable.h"
#include "shaderprogram.h"
#include "texture.h"

class Prism : public Drawable {
private:
    int w, h, d;
    bool hit;
    glm::ivec2 p1, p2;
public:
    Prism(OpenGLContext* context, glm::ivec3 dim,
          glm::ivec2 p1, glm::ivec2 p2);
    virtual ~Prism();
    virtual GLenum drawMode() override;
    virtual void createVBOdata() override;
    void setHit(bool hit);
    void draw(ShaderProgram* m_prog);
};


