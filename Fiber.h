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

using Point3f = glm::vec3;
using Normal3f = glm::vec3;

struct ControlPoint
{
    Point3f pos;
    Normal3f norm;
    int inx;
    float distanceFromStart = -1;
};

struct Strand
{
    std::vector<ControlPoint> points;
};

class Fiber {
public:
    Fiber(FIBER_TYPE type, RENDER_TYPE rType);
    ~Fiber();
    void initShaders();
    void initFrameBuffer();
    void initializeGL();

    void setWindow(GLFWwindow* window);
    void setFiberParameters(RENDER_TYPE);
    void setRenderType(RENDER_TYPE);

    void readFiberParameters(FIBER_TYPE);

    void addPoint(ControlPoint, bool);
    void loadPoints(bool);

    void addStrands(const std::vector<Strand>& strands);

    void render();

    const Shader& getActiveShader() const;
    const std::vector<Shader*> getActiveShaders();
    RENDER_TYPE getRenderType() const;
    FIBER_TYPE getFiberType() const;
    float getFiberAlpha() const;
    float getYarnRadius() const;

    void createGUIWindow();

    unsigned int SCR_WIDTH;
    unsigned int SCR_HEIGHT;

private:

    GLFWwindow* window;

    std::vector<float> corepoints_;
    std::vector<float> fiberpoints_;
    std::vector<GLuint> coreebo_;
    std::vector<GLuint> fiberebo_;
    GLuint corevao_id_;
    GLuint fibervao_id_;
    GLuint corevbo_id_;
    GLuint fibervbo_id_;
    GLuint coreebo_id_;
    GLuint fiberebo_id_;

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

    RENDER_TYPE renderType;
    FIBER_TYPE fiberType;

    bool renderCore = false;

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
