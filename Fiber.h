#pragma once

#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"

enum RENDER_TYPE {
    CORE,
    FIBER
};

class Fiber {
public:
    Fiber();
    ~Fiber();
    void addPoint(float x, float y, float z);
    void render();
    void setFiberParameters(RENDER_TYPE);
    void initShaders();
    const Shader& getActiveShader();

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

    Shader coreShader_;
    Shader fiberShader_;
    Shader pointsShader_;
};
