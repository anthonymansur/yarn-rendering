#pragma once

#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"

enum render_type {
    CORE,
    FIBER
};

class Fiber {
public:
    Shader coreShader_;
    Shader fiberShader_;

    Fiber();
    ~Fiber();
    void addPoint(float x, float y);
    void render();
    void setFiberParameters(render_type);
    void initShaders();

private:
    void loadPoints();

    std::vector<float> points_;
    std::vector<GLuint> ebo_;
    GLuint vao_id_;
    GLuint vbo_id_;
    GLuint ebo_id_;

    GLuint normalMap;
    GLuint heightMap;
    GLuint alphaChannel;

    GLuint normalFrameBuffer;
    GLuint heightFrameBuffer;
    GLuint alphaFrameBuffer;
};
