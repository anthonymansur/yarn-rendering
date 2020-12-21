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

enum FIBER_TYPE {
    COTTON1, 
    COTTON2,
    POLYESTER1,
    RAYON1,
    RAYON2,
    RAYON3,
    RAYON4,
    SILK1,
    SILK2
};

class Fiber {
public:
    Fiber(FIBER_TYPE type);
    ~Fiber();
    void initShaders();
    void initFrameBuffer();
    void initializeGL();

    void readFiberParameters(FIBER_TYPE);
    void setFiberParameters(RENDER_TYPE);
    void addPoint(float x, float y, float z);
    void setRenderType(RENDER_TYPE);

    void render();

    const Shader& getActiveShader();
    const std::vector<Shader*> getActiveShaders();
    RENDER_TYPE getRenderType();
    FIBER_TYPE getFiberType();
    float getFiberAlpha();

    void createGUIWindow();

    unsigned int SCR_WIDTH;
    unsigned int SCR_HEIGHT;
    
    RENDER_TYPE renderType;

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

    GLuint _frameBuffer;
    GLuint frameBuffer;
    GLuint depthrenderbuffer;
    GLuint heightTexture;
    GLuint normalTexture;
    GLuint alphaTexture;

    Shader coreShader_;
    Shader fiberShader_;
    Shader pointsShader_;

    FIBER_TYPE fiberType;

    // Fiber parameters
    int ply_num;
    int fiber_num;

    glm::vec3 bounding_min;
    glm::vec3 bounding_max;

    float z_step_size;
    int z_step_num;
    int fly_step_size;

    int yarn_clock_wise;
    int fiber_clock_wise;
    float yarn_alpha;
    float alpha;

    float yarn_radius;
    float ellipse_long;
    float ellipse_short;

    int epsilon;
    float beta;
    float r_max;

    int use_migration;
    float s_i;
    float rho_min;
    float rho_max;

    int use_flyaways;
    float flyaway_hair_density;
    glm::vec2 flyaway_hair_ze;
    glm::vec2 flyaway_hair_r0;
    glm::vec2 flyaway_hair_re;
    glm::vec2 flyaway_hair_pe;
    float flyaway_loop_density;
    glm::vec2 flyaway_loop_r1;
};
