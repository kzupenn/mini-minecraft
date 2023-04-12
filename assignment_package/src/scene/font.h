#pragma once
#include <glm_includes.h>
#include "drawable.h"
#include "shaderprogram.h"

class Font: public Drawable{
private:
    std::string text;
    glm::vec4 color;

public:
    Font(OpenGLContext* context, std::string text_string, glm::vec4 text_color) : Drawable(context),
        text(text_string), color(text_color){
        createVBOdata();
    };
    Font(OpenGLContext* context) : Font(context, "", glm::vec4(1,1,1,1)){};
    virtual ~Font(){};
    void setText(std::string); //setters

    float width;

    virtual void createVBOdata();
    void unbindVBO();
    virtual GLenum drawMode();
};
