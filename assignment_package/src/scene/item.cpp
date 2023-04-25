#include "item.h"
#include "chunk.h"

const std::map<ItemType, glm::vec2> itemUV = {
    {DIAMOND_HOE, glm::vec2(528.f/1024.f, 1008.f/1024.f)},

    {DIAMOND_LEGGINGS, glm::vec2(528.f/1024.f, 976.f/1024.f)},
    {GOLD_NUGGET, glm::vec2(624.f/1024.f, 976.f/1024.f)},

    {IRON_NUGGET, glm::vec2(624.f/1024.f, 960.f/1024.f)},

    {IRON_BOOTS, glm::vec2(560.f/1024.f, 944.f/1024.f)},
    {STONE_SWORD, glm::vec2(640.f/1024.f, 944.f/1024.f)},

    {DIAMOND_SWORD, glm::vec2(528.f/1024.f, 928.f/1024.f)},
    {IRON_CHESTPLATE, glm::vec2(560.f/1024.f, 928.f/1024.f)},
    {STRING, glm::vec2(640.f/1024.f, 928.f/1024.f)},

    {IRON_HELMET, glm::vec2(560.f/1024.f, 896.f/1024.f)},

    {IRON_INGOT, glm::vec2(560.f/1024.f, 848.f/1024.f)},

    {GOLD_INGOT, glm::vec2(544.f/1024.f, 832.f/1024.f)},
    {IRON_LEGGINGS, glm::vec2(560.f/1024.f, 832.f/1024.f)},

    {GOLDEN_APPLE, glm::vec2(544.f/1024.f, 800.f/1024.f)},

    {GOLDEN_BOOTS, glm::vec2(544.f/1024.f, 768.f/1024.f)},
    {IRON_SWORD, glm::vec2(560.f/1024.f, 768.f/1024.f)},

    {GOLDEN_CHESTPLATE, glm::vec2(544.f/1024.f, 736.f/1024.f)},

    {GOLDEN_HELMET, glm::vec2(544.f/1024.f, 720.f/1024.f)},

    {ENDER_PEARL, glm::vec2(528.f/1024.f, 704.f/1024.f)},
    {SNOWBALL, glm::vec2(624.f/1024.f, 704.f/1024.f)},

    {GOLDEN_LEGGINGS, glm::vec2(544.f/1024.f, 672.f/1024.f)},

    {GOLDEN_SWORD, glm::vec2(544.f/1024.f, 608.f/1024.f)},

    {APPLE, glm::vec2(528.f/1024.f, 1008.f/1024.f)},
    {ARROW, glm::vec2(528.f/1024.f, 1008.f/1024.f)},

    {BOW, glm::vec2(528.f/1024.f, 1008.f/1024.f)},
    {BOW_TAUNT_1, glm::vec2(528.f/1024.f, 1008.f/1024.f)},
    {BOW_TAUNT_2, glm::vec2(528.f/1024.f, 1008.f/1024.f)},
    {BOW_TAUNT_3, glm::vec2(528.f/1024.f, 1008.f/1024.f)},

    {FISHING_ROD, glm::vec2(528.f/1024.f, 1008.f/1024.f)},

    {DIAMOND, glm::vec2(528.f/1024.f, 1008.f/1024.f)},
    {DIAMOND_BOOTS, glm::vec2(528.f/1024.f, 1008.f/1024.f)},
    {DIAMOND_CHESTPLATE, glm::vec2(528.f/1024.f, 1008.f/1024.f)},
    {DIAMOND_HELMET, glm::vec2(528.f/1024.f, 1008.f/1024.f)},
    {STICK, glm::vec2(528.f/1024.f, 1008.f/1024.f)}
};

const std::map<ItemType, int> itemMaxStack = {
    {DIAMOND_HOE, 1},
    {DIAMOND_LEGGINGS, 1},
    {GOLD_NUGGET, 64},
    {IRON_NUGGET, 64},
    {IRON_BOOTS, 1},
    {STONE_SWORD, 1},
    {DIAMOND_SWORD, 1},
    {IRON_CHESTPLATE, 1},
    {STRING, 64},
    {IRON_HELMET, 1},
    {IRON_INGOT, 64},
    {GOLD_INGOT, 64},
    {IRON_LEGGINGS, 1},
    {GOLDEN_APPLE, 64},
    {GOLDEN_BOOTS, 1},
    {IRON_SWORD, 1},
    {GOLDEN_CHESTPLATE, 1},
    {GOLDEN_HELMET, 1},
    {ENDER_PEARL, 8},
    {SNOWBALL, 8},
    {GOLDEN_LEGGINGS, 1},
    {GOLDEN_SWORD, 1},
    {APPLE, 64},
    {ARROW, 64},
    {BOW, 1},
    {BOW_TAUNT_1, 1},
    {BOW_TAUNT_2, 1},
    {BOW_TAUNT_3, 1},
    {FISHING_ROD, 1},
    {DIAMOND, 64},
    {DIAMOND_BOOTS, 1}
};



Item:: Item(OpenGLContext* context, ItemType t, int init_count, bool drawImmediately) :
    Drawable(context),
    count_text(context, std::to_string(init_count), glm::vec4(1), drawImmediately), type(t), item_count(init_count)
{
    if(drawImmediately) createVBOdata();
    if(itemMaxStack.find(t)!=itemMaxStack.end()) max_count = itemMaxStack.at(t);
    else max_count = 64;
};

GLenum Item::drawMode() {
    return GL_TRIANGLES;
}

void Item::createVBOdata() {
    if(type < blockItemLimit){
        m_count = 6;
        std::vector<glm::vec4> VBOpos, col, uvs;
        std::vector<int> idx = {0, 1, 2, 2, 3, 0};

        col = {glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1),glm::vec4(1,1,1,1)};
        glm::vec4 blc = glm::vec4(656.f/1024.f, 944.f/1024.f, 0, 1);
        if(itemUV.find(type)!=itemUV.end()) {
            blc = glm::vec4(itemUV.at(type), 0, 1);
        }
        VBOpos = {glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 1), glm::vec4(0, 1, 0, 1)};
        uvs = {blc, blc+glm::vec4(16.f/1024.f, 0, 0, 0), blc+glm::vec4(16.f/1024.f, 16.f/1024.f,0,0), blc+glm::vec4(0, 16.f/1024.f, 0, 0)};

        generateIdx();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufIdx);
        mp_context->glBufferData(GL_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

        generatePos();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
        mp_context->glBufferData(GL_ARRAY_BUFFER, VBOpos.size() * sizeof(glm::vec4), VBOpos.data(), GL_STATIC_DRAW);

        generateUV();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
        mp_context->glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec4), uvs.data(), GL_STATIC_DRAW);

        generateCol();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
        mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
    }
    else {
        std::vector<glm::vec4> VBOpos, nor, uvs, col;
        std::vector<GLuint> idx;
        //top
        VBOpos.emplace_back(glm::vec4(1, 1, 1, 1));
        VBOpos.emplace_back(glm::vec4(0, 1, 1, 1));
        VBOpos.emplace_back(glm::vec4(0, 1, 0, 1));
        VBOpos.emplace_back(glm::vec4(1, 1, 0, 1));

        //bott
        VBOpos.emplace_back(glm::vec4(1, 0, 1, 1));
        VBOpos.emplace_back(glm::vec4(0, 0, 1, 1));
        VBOpos.emplace_back(glm::vec4(0, 0, 0, 1));
        VBOpos.emplace_back(glm::vec4(1, 0, 0, 1));

        //front
        VBOpos.emplace_back(glm::vec4(1, 1, 0, 1));
        VBOpos.emplace_back(glm::vec4(1, 1, 1, 1));
        VBOpos.emplace_back(glm::vec4(1, 0, 1, 1));
        VBOpos.emplace_back(glm::vec4(1, 0, 0, 1));

        //left
        VBOpos.emplace_back(glm::vec4(1, 1, 1, 1));
        VBOpos.emplace_back(glm::vec4(0, 1, 1, 1));
        VBOpos.emplace_back(glm::vec4(0, 0, 1, 1));
        VBOpos.emplace_back(glm::vec4(1, 0, 1, 1));

        //back
        VBOpos.emplace_back(glm::vec4(0, 1, 1, 1));
        VBOpos.emplace_back(glm::vec4(0, 1, 0, 1));
        VBOpos.emplace_back(glm::vec4(0, 0, 0, 1));
        VBOpos.emplace_back(glm::vec4(0, 0, 1, 1));

        //right
        VBOpos.emplace_back(glm::vec4(0, 1, 0, 1));
        VBOpos.emplace_back(glm::vec4(1, 1, 0, 1));
        VBOpos.emplace_back(glm::vec4(1, 0, 0, 1));
        VBOpos.emplace_back(glm::vec4(0, 0, 0, 1));

        //top
        for (int i = 0; i < 4; i++) {
            nor.emplace_back(0, 1, 0, 0);
        }
        //bott
        for (int i = 0; i < 4; i++) {
            nor.emplace_back(0, -1, 0, 0);
        }
        //+x=front
        for (int i = 0; i < 4; i++) {
            nor.emplace_back(1, 0, 0, 0);
        }
        //+z=left
        for (int i = 0; i < 4; i++) {
            nor.emplace_back(0, 0, 1, 0);
        }
        //-x=back
        for (int i = 0; i < 4; i++) {
            nor.emplace_back(-1, 0, 0, 0);
        }
        //-z=right
        for (int i = 0; i < 4; i++) {
            nor.emplace_back(0, 0, -1, 0);
        }

        BlockType bt = item2block.at(type);
        //top
        for(glm::vec4 foo: getBlockUV(bt, 2)) {
            uvs.emplace_back(foo);
        }
        //bott
        for(glm::vec4 foo: getBlockUV(bt, 3)) {
            uvs.emplace_back(foo);
        }

        //front
        for(glm::vec4 foo: getBlockUV(bt, 4)) {
            uvs.emplace_back(foo);
        }

        //left
        for(glm::vec4 foo: getBlockUV(bt, 0)) {
            uvs.emplace_back(foo);
        }

        //back
        for(glm::vec4 foo: getBlockUV(bt, 5)) {
            uvs.emplace_back(foo);
        }

        //right
        for(glm::vec4 foo: getBlockUV(bt, 1)) {
            uvs.emplace_back(foo);
        }

        for(int i = 0; i < 6; i++){
            idx.push_back(i*4);
            idx.push_back(i*4+1);
            idx.push_back(i*4+2);
            idx.push_back(i*4);
            idx.push_back(i*4+2);
            idx.push_back(i*4+3);
        }

        m_count = idx.size();

        for(int i = 0; i < VBOpos.size(); i++) {
           col.emplace_back(1);
        }

        generateIdx();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufIdx);
        mp_context->glBufferData(GL_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

        generatePos();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
        mp_context->glBufferData(GL_ARRAY_BUFFER, VBOpos.size() * sizeof(glm::vec4), VBOpos.data(), GL_STATIC_DRAW);

        generateUV();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
        mp_context->glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec4), uvs.data(), GL_STATIC_DRAW);

        generateCol();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
        mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);

        generateNor();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
        mp_context->glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);
    }
}

void Item::merge(Item& x) {
    if(x.type != this->type) return; //refuse to merge different items

    int toAdd = std::min(x.item_count, max_count-item_count);
    item_count += toAdd;
    count_text.setText(std::to_string(item_count));
    x.item_count -= toAdd;
    x.count_text.setText(std::to_string(x.item_count));
}

void Item::draw(ShaderProgram* m_prog, Texture& block, Texture& text, float block_size, float text_size, glm::vec3 block_pos, glm::vec3 text_pos) {
    block.bind(0);
    if(type < blockItemLimit)
        m_prog->setModelMatrix(glm::translate(glm::mat4(1), block_pos)*
                           glm::scale(glm::mat4(1), glm::vec3(block_size, block_size, 1)));
    else
//        m_prog->setModelMatrix(glm::translate(glm::mat4(), block_pos) *
//                               glm::translate(glm::mat4(), glm::vec3(-0.5, -0.5, -0.5))*
//                               glm::rotate(glm::mat4(), glm::radians(45.f), glm::vec3(0, 1, 0))*
//                               glm::rotate(glm::mat4(), glm::radians(45.f), glm::vec3(1, 0, 0))*
//                               glm::translate(glm::mat4(), glm::vec3(0.5, 0.5, 0.5))*
//                               glm::scale(glm::mat4(1), glm::vec3(0.7*block_size, 0.7*block_size, 0.7*block_size)));
        m_prog->setModelMatrix(glm::translate(glm::mat4(), block_pos) *
                                       glm::rotate(glm::mat4(), glm::radians(0.f), glm::vec3(0, 1, 0))*
                                       //glm::rotate(glm::mat4(), glm::radians(45.f), glm::vec3(1, 0, 0))*
                               glm::scale(glm::mat4(1), glm::vec3(block_size, block_size, block_size)));

    m_prog->draw(*this);
    if(item_count > 1) {
        text.bind(0);
        m_prog->setModelMatrix(glm::translate(glm::mat4(1), text_pos-glm::vec3(text_size*count_text.width, 0, 0))*
                               glm::scale(glm::mat4(1), glm::vec3(text_size, text_size, 1)));
        m_prog->draw(count_text);
    }
}
