#pragma once
#include "drawable.h"

class InterleavedDrawable : public Drawable
{
public:
    InterleavedDrawable();
    ~InterleavedDrawable();

    void generateVBO();
    bool bindVBO();

    virtual void create() = 0;

protected:
    GLuint bufVBO;
    bool vboBound;
};