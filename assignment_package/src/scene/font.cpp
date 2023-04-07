#include "font.h"

const std::map<char, std::pair<glm::vec2, float>> charUV = {
    {' ', std::make_pair(glm::vec2(0, 13.f/16.f), 3.f/8.f)},
    {'!', std::make_pair(glm::vec2(1.f/16.f, 13.f/16.f), 1.f/8.f)},
    {'"', std::make_pair(glm::vec2(2.f/16.f, 13.f/16.f), 3.f/8.f)},
    {'#', std::make_pair(glm::vec2(3.f/16.f, 13.f/16.f), 5.f/8.f)},
    {'$', std::make_pair(glm::vec2(4.f/16.f, 13.f/16.f), 5.f/8.f)},
    {'%', std::make_pair(glm::vec2(5.f/16.f, 13.f/16.f), 5.f/8.f)},
    {'&', std::make_pair(glm::vec2(6.f/16.f, 13.f/16.f), 5.f/8.f)},
    {'\'', std::make_pair(glm::vec2(7.f/16.f, 13.f/16.f), 1.f/8.f)},
    {'(', std::make_pair(glm::vec2(8.f/16.f, 13.f/16.f), 3.f/8.f)},
    {')', std::make_pair(glm::vec2(9.f/16.f, 13.f/16.f), 3.f/8.f)},
    {'*', std::make_pair(glm::vec2(10.f/16.f, 13.f/16.f), 3.f/8.f)},
    {'+', std::make_pair(glm::vec2(11.f/16.f, 13.f/16.f), 5.f/8.f)},
    {',', std::make_pair(glm::vec2(12.f/16.f, 13.f/16.f), 1.f/8.f)},
    {'-', std::make_pair(glm::vec2(13.f/16.f, 13.f/16.f), 5.f/8.f)},
    {'.', std::make_pair(glm::vec2(14.f/16.f, 13.f/16.f), 1.f/8.f)},
    {'/', std::make_pair(glm::vec2(15.f/16.f, 13.f/16.f), 5.f/8.f)},

    {'0', std::make_pair(glm::vec2(0, 12.f/16.f), 5.f/8.f)},
    {'1', std::make_pair(glm::vec2(1.f/16.f, 12.f/16.f), 5.f/8.f)},
    {'2', std::make_pair(glm::vec2(2.f/16.f, 12.f/16.f), 5.f/8.f)},
    {'3', std::make_pair(glm::vec2(3.f/16.f, 12.f/16.f), 5.f/8.f)},
    {'4', std::make_pair(glm::vec2(4.f/16.f, 12.f/16.f), 5.f/8.f)},
    {'5', std::make_pair(glm::vec2(5.f/16.f, 12.f/16.f), 5.f/8.f)},
    {'6', std::make_pair(glm::vec2(6.f/16.f, 12.f/16.f), 5.f/8.f)},
    {'7', std::make_pair(glm::vec2(7.f/16.f, 12.f/16.f), 5.f/8.f)},
    {'8', std::make_pair(glm::vec2(8.f/16.f, 12.f/16.f), 5.f/8.f)},
    {'9', std::make_pair(glm::vec2(9.f/16.f, 12.f/16.f), 5.f/8.f)},
    {':', std::make_pair(glm::vec2(10.f/16.f, 12.f/16.f), 1.f/8.f)},
    {';', std::make_pair(glm::vec2(11.f/16.f, 12.f/16.f), 1.f/8.f)},
    {'<', std::make_pair(glm::vec2(12.f/16.f, 12.f/16.f), 4.f/8.f)},
    {'=', std::make_pair(glm::vec2(13.f/16.f, 12.f/16.f), 5.f/8.f)},
    {'>', std::make_pair(glm::vec2(14.f/16.f, 12.f/16.f), 4.f/8.f)},
    {'?', std::make_pair(glm::vec2(15.f/16.f, 12.f/16.f), 5.f/8.f)},

    {'@', std::make_pair(glm::vec2(0, 11.f/16.f), 5.f/8.f)},
    {'A', std::make_pair(glm::vec2(1.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'B', std::make_pair(glm::vec2(2.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'C', std::make_pair(glm::vec2(3.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'D', std::make_pair(glm::vec2(4.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'E', std::make_pair(glm::vec2(5.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'F', std::make_pair(glm::vec2(6.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'G', std::make_pair(glm::vec2(7.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'H', std::make_pair(glm::vec2(8.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'I', std::make_pair(glm::vec2(9.f/16.f, 11.f/16.f), 3.f/8.f)},
    {'J', std::make_pair(glm::vec2(10.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'K', std::make_pair(glm::vec2(11.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'L', std::make_pair(glm::vec2(12.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'M', std::make_pair(glm::vec2(13.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'N', std::make_pair(glm::vec2(14.f/16.f, 11.f/16.f), 5.f/8.f)},
    {'O', std::make_pair(glm::vec2(15.f/16.f, 11.f/16.f), 5.f/8.f)},

    {'P', std::make_pair(glm::vec2(0, 10.f/16.f), 5.f/8.f)},
    {'Q', std::make_pair(glm::vec2(1.f/16.f, 10.f/16.f), 5.f/8.f)},
    {'R', std::make_pair(glm::vec2(2.f/16.f, 10.f/16.f), 5.f/8.f)},
    {'S', std::make_pair(glm::vec2(3.f/16.f, 10.f/16.f), 5.f/8.f)},
    {'T', std::make_pair(glm::vec2(4.f/16.f, 10.f/16.f), 5.f/8.f)},
    {'U', std::make_pair(glm::vec2(5.f/16.f, 10.f/16.f), 5.f/8.f)},
    {'V', std::make_pair(glm::vec2(6.f/16.f, 10.f/16.f), 5.f/8.f)},
    {'W', std::make_pair(glm::vec2(7.f/16.f, 10.f/16.f), 5.f/8.f)},
    {'X', std::make_pair(glm::vec2(8.f/16.f, 10.f/16.f), 5.f/8.f)},
    {'Y', std::make_pair(glm::vec2(9.f/16.f, 10.f/16.f), 5.f/8.f)},
    {'Z', std::make_pair(glm::vec2(10.f/16.f, 10.f/16.f), 5.f/8.f)},
    {'[', std::make_pair(glm::vec2(11.f/16.f, 10.f/16.f), 3.f/8.f)},
    {'\\', std::make_pair(glm::vec2(12.f/16.f, 10.f/16.f), 5.f/8.f)},
    {']', std::make_pair(glm::vec2(13.f/16.f, 10.f/16.f), 3.f/8.f)},
    {'^', std::make_pair(glm::vec2(14.f/16.f, 10.f/16.f), 5.f/8.f)},
    {'_', std::make_pair(glm::vec2(15.f/16.f, 10.f/16.f), 5.f/8.f)},

    {'`', std::make_pair(glm::vec2(0, 9.f/16.f), 2.f/8.f)},
    {'a', std::make_pair(glm::vec2(1.f/16.f, 9.f/16.f), 5.f/8.f)},
    {'b', std::make_pair(glm::vec2(2.f/16.f, 9.f/16.f), 5.f/8.f)},
    {'c', std::make_pair(glm::vec2(3.f/16.f, 9.f/16.f), 5.f/8.f)},
    {'d', std::make_pair(glm::vec2(4.f/16.f, 9.f/16.f), 5.f/8.f)},
    {'e', std::make_pair(glm::vec2(5.f/16.f, 9.f/16.f), 5.f/8.f)},
    {'f', std::make_pair(glm::vec2(6.f/16.f, 9.f/16.f), 5.f/8.f)},
    {'g', std::make_pair(glm::vec2(7.f/16.f, 9.f/16.f), 5.f/8.f)},
    {'h', std::make_pair(glm::vec2(8.f/16.f, 9.f/16.f), 5.f/8.f)},
    {'i', std::make_pair(glm::vec2(9.f/16.f, 9.f/16.f), 1.f/8.f)},
    {'j', std::make_pair(glm::vec2(10.f/16.f, 9.f/16.f), 5.f/8.f)},
    {'k', std::make_pair(glm::vec2(11.f/16.f, 9.f/16.f), 5.f/8.f)},
    {'l', std::make_pair(glm::vec2(12.f/16.f, 9.f/16.f), 2.f/8.f)},
    {'m', std::make_pair(glm::vec2(13.f/16.f, 9.f/16.f), 5.f/8.f)},
    {'n', std::make_pair(glm::vec2(14.f/16.f, 9.f/16.f), 5.f/8.f)},
    {'o', std::make_pair(glm::vec2(15.f/16.f, 9.f/16.f), 5.f/8.f)},

    {'p', std::make_pair(glm::vec2(0, 8.f/16.f), 5.f/8.f)},
    {'q', std::make_pair(glm::vec2(1.f/16.f, 8.f/16.f), 5.f/8.f)},
    {'r', std::make_pair(glm::vec2(2.f/16.f, 8.f/16.f), 5.f/8.f)},
    {'s', std::make_pair(glm::vec2(3.f/16.f, 8.f/16.f), 5.f/8.f)},
    {'t', std::make_pair(glm::vec2(4.f/16.f, 8.f/16.f), 3.f/8.f)},
    {'u', std::make_pair(glm::vec2(5.f/16.f, 8.f/16.f), 5.f/8.f)},
    {'v', std::make_pair(glm::vec2(6.f/16.f, 8.f/16.f), 5.f/8.f)},
    {'w', std::make_pair(glm::vec2(7.f/16.f, 8.f/16.f), 5.f/8.f)},
    {'x', std::make_pair(glm::vec2(8.f/16.f, 8.f/16.f), 5.f/8.f)},
    {'y', std::make_pair(glm::vec2(9.f/16.f, 8.f/16.f), 5.f/8.f)},
    {'z', std::make_pair(glm::vec2(10.f/16.f, 8.f/16.f), 5.f/8.f)},
    {'{', std::make_pair(glm::vec2(11.f/16.f, 8.f/16.f), 3.f/8.f)},
    {'|', std::make_pair(glm::vec2(12.f/16.f, 8.f/16.f), 1.f/8.f)},
    {'}', std::make_pair(glm::vec2(13.f/16.f, 8.f/16.f), 3.f/8.f)},
    {'~', std::make_pair(glm::vec2(14.f/16.f, 8.f/16.f), 5.f/8.f)}
};

void Font::setText(std::string s) {
    text = s;
}

void Font::setPos(glm::vec2 v) {
    pos = v;
}

void Font::setSize(float f) {
    size = f;
}

void Font::createVBOdata() {
    std::vector<glm::vec4> VBOpos, uvs, col;
    std::vector<int> idx;

    float width = 0;

    for(char& c: text) {
        idx.push_back(VBOpos.size());
        idx.push_back(VBOpos.size()+1);
        idx.push_back(VBOpos.size()+2);
        idx.push_back(VBOpos.size()+2);
        idx.push_back(VBOpos.size()+3);
        idx.push_back(VBOpos.size());

        std::pair<glm::vec2, float> pp;
        if(charUV.find(c) == charUV.end()) {
            pp = charUV.at(' ');
        }
        else{
            pp = charUV.at(c);
        }

        float w = pp.second*size;
        float h = size;

        VBOpos.emplace_back(width, 0, 0, 1);
        VBOpos.emplace_back(width+w, 0, 0, 1);
        VBOpos.emplace_back(width+w, h, 0, 1);
        VBOpos.emplace_back(width, h, 0, 1);

        uvs.emplace_back(pp.first.x, pp.first.y, 0, 1);
        uvs.emplace_back(pp.first.x+pp.second/16.f, pp.first.y, 0, 1);
        uvs.emplace_back(pp.first.x+pp.second/16.f, pp.first.y+1.f/16.f, 0, 1);
        uvs.emplace_back(pp.first.x, pp.first.y+1.f/16.f, 0, 1);

        for(int j = 0; j < 4; j++) col.emplace_back(color);

        width += w + size/16.f;
    }

    m_count = idx.size();

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

GLenum Font::drawMode(){
    return GL_TRIANGLES;
}
