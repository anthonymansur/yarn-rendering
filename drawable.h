#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Drawable
{
protected:
    int count;
    GLuint bufIdx;
    GLuint bufPos;
    GLuint bufNor;
    GLuint bufUV;
    GLuint bufCol;

    GLenum currDrawMode;

    bool idxBound;
    bool posBound;
    bool norBound;
    bool uvBound;
    bool colBound;

public:
    Drawable();

    virtual void create() = 0;
    void destroy();

    // Getter functions for various GL data
    virtual GLenum drawMode();
    int elemCount();

    void generateIdx();
    void generatePos();
    void generateNor();
    void generateUV();
    void generateCol();

    bool bindIdx();
    bool bindPos();
    bool bindNor();
    bool bindUV();
    bool bindCol();
};