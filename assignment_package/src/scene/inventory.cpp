#include "inventory.h"

GLenum Inventory::drawMode() {
    return GL_TRIANGLES;
}

GLenum Hotbar::drawMode() {
    return GL_TRIANGLES;
}

void Inventory::createVBOdata() {
    std::vector<glm::vec4> pos, uvs, col;
    std::vector<int> idx = {0,1,2,2,3,0};

    float w = 550*176.f/256.f;
    float h = 550*166.f/256.f;
    pos = {glm::vec4(-w, -h, 0, 1),
           glm::vec4(-w, h, 0, 1),
           glm::vec4(w, h, 0, 1),
           glm::vec4(w, -h, 0, 1)};
    uvs = {glm::vec4(0, 90.f/256.f, 0, 1),glm::vec4(0, 1, 0, 1),glm::vec4(176.f/256.f, 1, 0, 1),glm::vec4(176.f/256.f, 90.f/256.f, 0, 1)};
    col = {glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1)};
    m_count = 6;

    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec4), uvs.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

bool Inventory::addItem(Item& item) {
    //tries to first merge with existing same items
    for(int i = 0; i < hotbar.max_slots; i++){
        if(hotbar.items[i]){
            hotbar.items[i]->merge(item);
            if(item.item_count == 0){
                return true;
            }
        }
    }
    for(int i = 0; i < max_slots; i++) {
        if(items[i]){
            items[i]->merge(item);
            if(item.item_count == 0) {
                return true;
            }
        }
    }
    //if that still doesn't use up all the items, try to add the remainder to a free inventory slot
    for(int i = 0; i < hotbar.max_slots; i++) {
        if(!hotbar.items[i]) {
            hotbar.items[i] = item;
            return true;
        }
    }
    for(int i = 0; i < max_slots; i++) {
        if(!items[i]) {
            items[i] = item;
            return true;
        }
    }
    return false;
}

bool Inventory::addItem(Item& item, int slot_num) {
    if(slot_num < hotbar.max_slots) {
        if(!hotbar.items[slot_num]) { //if slot is unoccupied, add item in
            hotbar.items[slot_num] = item;
            return true;
        }
        else { //otherwise we need to merge it in
            hotbar.items[slot_num]->merge(item);
            return item.item_count == 0;
        }
    }
    else{
        slot_num -= hotbar.max_slots;
        if(!items[slot_num]) { //if slot is unoccupied, add item in
            items[slot_num] = item;
            return true;
        }
        else { //otherwise we need to merge it in
            items[slot_num]->merge(item);
            return item.item_count == 0;
        }
    }
}

int Inventory::calcArmor() {
    int ret = 0;
    for(std::optional<Item> it: armor) {
        if(it){
            switch(it->type) {
            case GOLDEN_HELMET:
                ret+=2;
                break;
            case GOLDEN_CHESTPLATE:
                ret+=5;
                break;
            case GOLDEN_LEGGINGS:
                ret+=3;
                break;
            case GOLDEN_BOOTS:
                ret+=1;
                break;
            case IRON_HELMET:
                ret+=2;
                break;
            case IRON_CHESTPLATE:
                ret+=6;
                break;
            case IRON_LEGGINGS:
                ret+=5;
                break;
            case IRON_BOOTS:
                ret+=2;
                break;
            case DIAMOND_HELMET:
                ret+=3;
                break;
            case DIAMOND_CHESTPLATE:
                ret+=8;
                break;
            case DIAMOND_LEGGINGS:
                ret+=6;
                break;
            case DIAMOND_BOOTS:
                ret+=3;
                break;
            default:
            break;
            }
        }
    }
    return ret;
}

bool Inventory::isArmor(std::optional<Item>& item, int slot_num) {
    if(!item) return true; //allow empty
    switch(slot_num) {
    case 0:
        return (item->type == GOLDEN_HELMET || item->type == IRON_HELMET || item->type == DIAMOND_HELMET);
        break;
    case 1:
        return (item->type == GOLDEN_CHESTPLATE || item->type == IRON_CHESTPLATE || item->type == DIAMOND_CHESTPLATE);
        break;
    case 2:
        return (item->type == GOLDEN_LEGGINGS || item->type == IRON_LEGGINGS || item->type == DIAMOND_LEGGINGS);
        break;
    case 3:
        return (item->type == GOLDEN_BOOTS || item->type == IRON_BOOTS || item->type == DIAMOND_BOOTS);
        break;
    default: return false;
    }
}

void Hotbar::createVBOdata() {
    std::vector<glm::vec4> pos, uvs, col;
    std::vector<int> idx;

    m_count = 6*max_slots;
    float size = 100;
    for(int i = 0; i < max_slots; i++) {
        idx.push_back(pos.size());
        idx.push_back(pos.size()+1);
        idx.push_back(pos.size()+2);
        idx.push_back(pos.size()+2);
        idx.push_back(pos.size()+3);
        idx.push_back(pos.size());
        pos.emplace_back(-size*0.5*max_slots + i*size, 0, 0, 1);
        pos.emplace_back(-size*0.5*max_slots + i*size, size, 0, 1);
        pos.emplace_back(-size*0.5*max_slots + i*size + size, size, 0, 1);
        pos.emplace_back(-size*0.5*max_slots + i*size + size, 0, 0, 1);
        if(selected == i) {
            uvs.emplace_back(165.f/256.f, 66.f/256.f, 0, 1);
            uvs.emplace_back(165.f/256.f, 90.f/256.f, 0, 1);
            uvs.emplace_back(189.f/256.f, 90.f/256.f, 0, 1);
            uvs.emplace_back(189.f/256.f, 66.f/256.f, 0, 1);
        }
        else {
            uvs.emplace_back(141.f/256.f, 66.f/256.f, 0, 1);
            uvs.emplace_back(141.f/256.f, 90.f/256.f, 0, 1);
            uvs.emplace_back(165.f/256.f, 90.f/256.f, 0, 1);
            uvs.emplace_back(165.f/256.f, 66.f/256.f, 0, 1);
        }

        for(int j = 0; j < 4; j++) {
            col.emplace_back(1,1,1,1);
        }
        generateIdx();
        // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
        // and that it will be treated as an element array buffer (since it will contain triangle indices)
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
        // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
        // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

        generatePos();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
        mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

        generateUV();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
        mp_context->glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec4), uvs.data(), GL_STATIC_DRAW);

        generateCol();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
        mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
    }
}

