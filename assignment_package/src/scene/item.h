#pragma once
#include <glm_includes.h>
#include "drawable.h"

//ordered by row in texture map
enum ItemType: unsigned char {
    DIAMOND_HOE,
    DIAMOND_LEGGINGS, GOLD_NUGGET,
    IRON_NUGGET,
    IRON_BOOTS, STONE_SWORD,
    DIAMOND_SWORD, IRON_CHESTPLATE, STRING,
    IRON_HELMET,
    IRON_INGOT,
    GOLD_INGOT, IRON_LEGGINGS,
    GOLDEN_APPLE,
    GOLDEN_BOOTS, IRON_SWORD,
    GOLDEN_CHESTPLATE,
    GOLDEN_HELMET,
    ENDER_PEARL, SNOWBALL,
    GOLDEN_LEGGINGS,
    GOLDEN_SWORD,
    APPLE, ARROW,
    BOW, BOW_TAUNT_1, BOW_TAUNT_2, BOW_TAUNT_3,
    FISHING_ROD,
    DIAMOND, DIAMOND_BOOTS, DIAMOND_CHESTPLATE, DIAMOND_HELMET, STICK
};

class Item: public Drawable {
private:
    int max_count;

public:
    Item(OpenGLContext* context) : Drawable(context), item_count(0), max_count(64){};
    Item(OpenGLContext* context, ItemType t, int init_count);
    virtual ~Item(){};
    virtual void createVBOdata();
    virtual GLenum drawMode();

    ItemType type;
    int item_count;
    void merge(Item&); //merges a number of items from the input item to its count
};
