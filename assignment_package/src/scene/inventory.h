#pragma once
#include <glm_includes.h>
#include "drawable.h"
#include "scene/item.h"

class Inventory: public Drawable {
private:
    int max_slots;
    std::vector<Item> items;
public:
    Inventory(OpenGLContext* context) : Drawable(context){};
    virtual ~Inventory(){};
    virtual void createVBOdata();
    virtual GLenum drawMode();
};

class Hotbar: public Drawable {
public:
    Hotbar(OpenGLContext* context) : Drawable(context){};
    virtual ~Hotbar();
    virtual void createVBOdata();
    virtual GLenum drawMode();
};
