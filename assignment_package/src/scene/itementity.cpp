#include "itementity.h"

void ItemEntity::createVBOdata() {

}
GLenum ItemEntity::drawMode() {
    return GL_TRIANGLES;
}
void ItemEntity::tick(float dT, InputBundle &input) {
    if(untouchable_ticks>0) untouchable_ticks--;
}
