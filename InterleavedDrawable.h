#pragma once
#include "drawable.h"

class InterleavedDrawable : public Drawable
{
public:
    InterleavedDrawable();
    ~InterleavedDrawable();

    virtual void create() = 0;

    inline void generateVAO()
    {
        vaoBound = true;
        glGenVertexArrays(1, &m_vao);
    }

    inline void generateVBO()
    {
        vboBound = true;
        glGenBuffers(1, &bufVBO);
    }

    inline bool bindVAO()
    {
        if (vaoBound)
            glBindVertexArray(m_vao);
        return vaoBound;
    }

    inline bool bindVBO()
    {
        if (vboBound)
            glBindBuffer(GL_ARRAY_BUFFER, bufVBO);
        return vboBound;
    }

protected:
    GLuint bufVBO, m_vao;
    bool vboBound, vaoBound;
};