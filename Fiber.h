#pragma once
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Shader.h"

typedef glm::vec3 Point3f, Normal3f;

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

class Fiber
{
public:
    Fiber(FIBER_TYPE type);
    virtual ~Fiber();
    
    virtual void initializeGL();

    void setFiberParameters();

    void readFiberParameters(FIBER_TYPE);

    void addPoint(ControlPoint, bool);
    void loadPoints();
    void addStrands(const std::vector<Strand>& strands);

    virtual void render() = 0;

    FIBER_TYPE getFiberType() const;
    float getFiberAlpha() const;
    float getYarnRadius() const;

    void createGUIWindow();

protected:
    // OpenGL variables
    std::vector<float> m_points;
    std::vector<GLuint> m_indices;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;
    GLuint depthrenderbuffer;
    GLuint heightTexture;
    GLuint normalTexture;
    GLuint alphaTexture;

    Shader* m_shader;

    unsigned int SCR_WIDTH;
    unsigned int SCR_HEIGHT;
    unsigned int CORE_HEIGHT;


    // Fiber-specific parameters
    FIBER_TYPE fiberType;

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