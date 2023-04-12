#pragma once
#include <glm_includes.h>
#include "drawable.h"

class Rectangle: public Drawable {
public:
    Rectangle(OpenGLContext* context) : Drawable(context){};
    virtual ~Rectangle(){};
    virtual void createVBOdata();
    virtual GLenum drawMode();
};
