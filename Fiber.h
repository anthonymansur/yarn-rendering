#pragma once

#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"

enum RENDER_TYPE {
    CORE,
    FIBER,
    COMPLETE
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
    const std::vector<Shader*> getActiveShaders();
    RENDER_TYPE getRenderType();
    void initFrameBuffer();

    static unsigned int SCR_WIDTH;
    static unsigned int SCR_HEIGHT;
    static unsigned int CORE_WIDTH;
    static unsigned int CORE_HEIGHT;

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

    GLuint frameBuffer;
    GLuint renderedTexture;
    GLuint depthrenderbuffer;

    Shader coreShader_;
    Shader fiberShader_;
    Shader pointsShader_;
};
