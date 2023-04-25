#pragma once

#include "entity.h"
#include "scene/item.h"

class ItemEntity: public Entity {
public:
    int id;
    float a;
    int untouchable_ticks;
    Item item;
    ItemEntity(glm::vec3 pos, Item it, OpenGLContext* context) : Entity(pos), item(it), untouchable_ticks(100) {}
    virtual void createVBOdata();
    virtual GLenum drawMode();
    virtual void tick(float dT, InputBundle &input) override;
};
