#pragma once
#include <glm_includes.h>
#include "drawable.h"
#include "shaderprogram.h"

class Font: public Drawable{
private:
    std::string text;
    glm::vec4 color;

    float size;
public:
    Font(OpenGLContext* context, std::string text_string, glm::vec4 text_color, float text_size) : Drawable(context),
        text(text_string), color(text_color), size(text_size) {};
    Font(OpenGLContext* context) : Font(context, "", glm::vec4(1,1,1,1), 50){};
    virtual ~Font(){};
    void setText(std::string); //setters
    void setSize(float);

    float width;

    virtual void createVBOdata();
    void unbindVBO();
    virtual GLenum drawMode();
};
