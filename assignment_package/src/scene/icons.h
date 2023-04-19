#pragma once
#include <glm_includes.h>
#include "drawable.h"
#include "shaderprogram.h"
#include "texture.h"

class Heart: public Drawable {
public:
    Heart(OpenGLContext* context) : Drawable(context){}
    virtual ~Heart(){}
    void createVBOdata() override;
    GLenum drawMode() override;
};

class FullHeart: public Drawable {
public:
    FullHeart(OpenGLContext* context) : Drawable(context){}
    virtual ~FullHeart(){}
    void createVBOdata() override;
    GLenum drawMode() override;
};

class HalfHeart: public Drawable {
public:
    HalfHeart(OpenGLContext* context) : Drawable(context){}
    virtual ~HalfHeart(){}
    void createVBOdata() override;
    GLenum drawMode() override;
};

class Armor: public Drawable {
public:
    Armor(OpenGLContext* context) : Drawable(context){}
    virtual ~Armor(){}
    void createVBOdata() override;
    GLenum drawMode() override;
};

class FullArmor: public Drawable {
public:
    FullArmor(OpenGLContext* context) : Drawable(context){}
    virtual ~FullArmor(){}
    void createVBOdata() override;
    GLenum drawMode() override;
};

class HalfArmor: public Drawable {
public:
    HalfArmor(OpenGLContext* context) : Drawable(context){}
    virtual ~HalfArmor(){}
    void createVBOdata() override;
    GLenum drawMode() override;
};
