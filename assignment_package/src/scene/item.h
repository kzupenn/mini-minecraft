#pragma once
#include <glm_includes.h>
#include "drawable.h"

class Item: public Drawable {
private:
    int item_count;

public:
    Item(OpenGLContext* context) : Drawable(context){};
    virtual ~Item(){};
    virtual void createVBOdata();
    virtual GLenum drawMode();
};
