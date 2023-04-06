#pragma once
#include <glm_includes.h>
#include "drawable.h"

class Crosshair: public Drawable {
public:
    Crosshair(OpenGLContext* context) : Drawable(context){};
    virtual ~Crosshair(){};
    virtual void createVBOdata();
    virtual GLenum drawMode();
};
