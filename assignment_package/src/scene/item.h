#pragma once
#include <glm_includes.h>
#include "drawable.h"

enum ItemType: unsigned char {
    BOW
};

class Item: public Drawable {
private:
    int max_count;

public:
    Item(OpenGLContext* context) : Drawable(context), item_count(0), max_count(64){};
    Item(OpenGLContext* context, ItemType t, int init_count, int m_count) : Drawable(context), max_count(m_count), type(t), item_count(init_count){};
    virtual ~Item(){};
    virtual void createVBOdata();
    virtual GLenum drawMode();

    ItemType type;
    int item_count;
    void merge(Item&); //merges a number of items from the input item to its count
};
