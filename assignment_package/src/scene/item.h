#pragma once
#include <glm_includes.h>
#include "drawable.h"
#include "shaderprogram.h"
#include "texture.h"
#include "font.h"
#include "chunk.h"
#include <set>

//ordered by row in texture map
enum ItemType: unsigned char {
    DIAMOND_HOE, AIR,
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
    DIAMOND, DIAMOND_BOOTS, DIAMOND_CHESTPLATE, DIAMOND_HELMET, STICK,

    EMPTY_, GRASS_BLOCK_, DIRT_, STONE_, WATER_, SAND_, SNOW_, COBBLESTONE_,
    OAK_PLANKS_, SPRUCE_PLANKS_, JUNGLE_PLANKS_, BIRCH_PLANKS_, ACACIA_PLANKS_,
    OAK_LOG_, SPRUCE_LOG_, BIRCH_LOG_, JUNGLE_LOG_, ACACIA_LOG_,
    OAK_LEAVES_, BOOKSHELF_, GLASS_, PATH_, SANDSTONE_,
    LAVA_, BEDROCK_, CACTUS_, ICE_,
    GRASS_
};

const std::map<ItemType, BlockType> item2block= {
    {EMPTY_, EMPTY},
    {GRASS_BLOCK_, GRASS_BLOCK},
    {DIRT_, DIRT},
    {STONE_, STONE},
     {WATER_, WATER},
     {SAND_, SAND},
      {SNOW_, SNOW},
       {COBBLESTONE_, COBBLESTONE},
    {OAK_PLANKS_, OAK_PLANKS},
    {SPRUCE_PLANKS_, SPRUCE_PLANKS},
    {JUNGLE_PLANKS_, JUNGLE_PLANKS},
    {BIRCH_PLANKS_, BIRCH_PLANKS},
    {ACACIA_PLANKS_, ACACIA_PLANKS},
    {OAK_LOG_, OAK_LOG},
    {SPRUCE_LOG_, SPRUCE_LOG},
    {BIRCH_LOG_, BIRCH_LOG},
    {JUNGLE_LOG_, JUNGLE_LOG},
    {ACACIA_LOG_, ACACIA_LOG},
    {OAK_LEAVES_, OAK_LEAVES},
    {BOOKSHELF_, BOOKSHELF},
    {GLASS_, GLASS},
    {PATH_, PATH},
    {SANDSTONE_,  SANDSTONE},
    {LAVA_, LAVA},
    {BEDROCK_, BEDROCK},
    {CACTUS_, CACTUS},
    {ICE_, ICE},
    {GRASS_, GRASS}
};

const unsigned char blockItemLimit = 36; //anything less than this is an item, equal or more is a block

class Item: public Drawable { 
public:
    Item(OpenGLContext* context) : Drawable(context), count_text(context),
        item_count(0), max_count(64){
        createVBOdata();
    };
    Item(OpenGLContext* context, ItemType t, int init_count, bool drawImmediately);
    //used for non-drawn items (server-side)
    Item(ItemType t, int init_count) : Drawable(nullptr), count_text(nullptr),
        type(t), item_count(init_count){}
    virtual ~Item(){};
    virtual void createVBOdata();
    virtual GLenum drawMode();

    Font count_text;

    void draw(ShaderProgram*, Texture&, Texture&, float, float, glm::vec3, glm::vec3);

    ItemType type;
    int item_count;
    int max_count;
    void merge(Item&); //merges a number of items from the input item to its count
};
