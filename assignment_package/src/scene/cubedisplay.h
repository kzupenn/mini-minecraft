#pragma once
#include <glm_includes.h>
#include "drawable.h"
#include "shaderprogram.h"

class CubeDisplay : public Drawable {
public:
    CubeDisplay(OpenGLContext* context);
    virtual ~CubeDisplay();
    virtual GLenum drawMode() override;
    virtual void createVBOdata() override;
};


