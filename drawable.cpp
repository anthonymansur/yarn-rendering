#include "Drawable.h"

Drawable::Drawable()
    : bufIdx(), bufPos(), bufNor(), bufUV(),
    idxBound(false), posBound(false), norBound(false), uvBound(false),
    count(-1), currDrawMode(GL_TRIANGLES)
{}


void Drawable::destroy()
{
    glDeleteBuffers(1, &bufIdx);
    glDeleteBuffers(1, &bufPos);
    glDeleteBuffers(1, &bufNor);
    glDeleteBuffers(1, &bufUV);
}

GLenum Drawable::drawMode()
{
    return currDrawMode; //GL_LINES;
}

int Drawable::elemCount()
{
    return count;
}

void Drawable::generateIdx()
{
    idxBound = true;
    glGenBuffers(1, &bufIdx);
}

void Drawable::generatePos()
{
    posBound = true;
    glGenBuffers(1, &bufPos);
}

void Drawable::generateNor()
{
    norBound = true;
    glGenBuffers(1, &bufNor);
}

void Drawable::generateUV()
{
    uvBound = true;
    glGenBuffers(1, &bufUV);
}

void Drawable::generateCol()
{
    colBound = true;
    glGenBuffers(1, &bufCol);
}

bool Drawable::bindIdx()
{
    if (idxBound) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    }
    return idxBound;
}

bool Drawable::bindPos()
{
    if (posBound) {
        glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    }
    return posBound;
}

bool Drawable::bindNor()
{
    if (norBound) {
        glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    }
    return norBound;
}

bool Drawable::bindUV()
{
    if (uvBound) {
        glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    }
    return uvBound;
}

bool Drawable::bindCol()
{
    if (colBound) {
        glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    }
    return colBound;
}