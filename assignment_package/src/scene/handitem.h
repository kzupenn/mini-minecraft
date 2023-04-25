#pragma once
#include <glm_includes.h>
#include "drawable.h"
#include "shaderprogram.h"
#include "texture.h"

class HandItem : public Drawable {
public:
    glm::vec4 p;

    HandItem(OpenGLContext* context, glm::vec4 p);
    virtual ~HandItem();
    virtual GLenum drawMode() override;
    virtual void createVBOdata() override;
};
