#include "item.h"

GLenum Item::drawMode() {
    return GL_TRIANGLES;
}

void Item::createVBOdata() {
    m_count = 6;
}

void Item::merge(Item& x) {
    if(x.type != this->type) return; //refuse to merge different items

    int toAdd = std::min(x.item_count, max_count-item_count);
    item_count += toAdd;
    x.item_count -= toAdd;
}
