#pragma once
#include <glm_includes.h>
#include "drawable.h"
#include "scene/item.h"

class Hotbar: public Drawable {
public:
    int max_slots;
    int selected;
    std::vector<std::optional<Item>> items;

    Hotbar(OpenGLContext* context, int max_count) : Drawable(context), max_slots(max_count), selected(0){
        items.resize(max_slots, std::nullopt);
    };
    virtual ~Hotbar(){};
    virtual void createVBOdata();
    virtual GLenum drawMode();
};

class Inventory: public Drawable {
private:
    int max_slots;
    OpenGLContext* cnt;
public:
    Inventory(OpenGLContext* context, int max_count, bool has_hotbar) : Drawable(context), max_slots(max_count), cnt(context), hotbar(context, 0), showInventory(false){
        if(has_hotbar) hotbar = Hotbar(context, 9);
        items.resize(max_slots, std::nullopt);
        armor.resize(4, std::nullopt);
    };

    virtual ~Inventory(){};
    virtual void createVBOdata();
    virtual GLenum drawMode();
    std::vector<std::optional<Item>> items;
    std::vector<std::optional<Item>> armor;

    bool addItem(Item&); //adds item to first free slot
    bool addItem(Item&, int); //adds item to slot number
    int calcArmor(); //returns total armor value
    bool isArmor(std::optional<Item>&, int); //checks if the

    Hotbar hotbar;
    bool showInventory;
};



