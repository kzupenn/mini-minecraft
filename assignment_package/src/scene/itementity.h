#pragma once

#include "entity.h"
#include "scene/item.h"

class ItemEntity: public Entity {
public:
    int id;
    float a;
    int untouchable_ticks;
    Item item;
    ItemEntity(glm::vec3 pos, ItemType it, int it_count, int id, OpenGLContext* context) : Entity(pos, context), item(context, it, it_count), untouchable_ticks(100) {}
    virtual void createVBOdata() override;
    virtual GLenum drawMode() override;
    virtual void tick(float dT, InputBundle &input) override;
};
