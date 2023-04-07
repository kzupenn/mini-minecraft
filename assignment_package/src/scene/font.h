#pragma once
#include <glm_includes.h>
#include "drawable.h"
#include "shaderprogram.h"

class Font: public Drawable{
private:
    std::string text;
    glm::vec2 pos;
    glm::vec4 color;

    float size;
public:
    Font(OpenGLContext* context, std::string text_string, glm::vec2 position, glm::vec4 text_color, float text_size) : Drawable(context),
        text(text_string), pos(position), color(text_color), size(text_size) {};
    Font(OpenGLContext* context) : Font(context, "", glm::vec2(0,0), glm::vec4(1,1,1,1), 50){};
    virtual ~Font(){};
    void setText(std::string); //setters
    void setPos(glm::vec2);
    void setSize(float);

    virtual void createVBOdata();
    virtual GLenum drawMode();
};
