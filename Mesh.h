#pragma once
#include "Drawable.h"
#include <memory>
#include <vector>
#include <SOIL/SOIL.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <map>


//class FabricVertex {
//public:
//    int id;
//    //std::map<std::string, glm::vec3> values;
//    std::vector<glm::vec3> attr;
//    glm::vec3 localPos, tmpPos, origPos, velocity;
//    bool at_rest = false;
//    
//    FabricVertex *posWeft, *negWeft, *posWarp, *negWarp;
//
//    glm::vec2 edgeLength; //Warp, Weft
//
//    FabricVertex(glm::vec3 pos, glm::vec2 length);
//    void _reset();
//};
//
//class Fabric : public Drawable {
//public:
//    virtual ~Fabric() {};
//    virtual void create() override = 0;
//    virtual void reset(glm::vec3 pos) = 0;
//    virtual void applyTemporaryPositionChanges() = 0;
//    virtual void begin
//};
//
//class Fabric : public Drawable {
//    // Fabric vertices have an inherent ordering to them, as determined by their vertex links
//    // We'll still store all of them though, for convenience or something idk
//
//public:
//    glm::vec3 position; // Center
//    std::vector<std::unique_ptr<FabricVertex>> vertices;
//    FabricVertex*** vertices;
//    glm::vec2 amount;
//    float mass;
//    bool isString;
//
//    Fabric(glm::vec3 pos, glm::vec3 pos2, int stringDensity, float mass);
//    Fabric(glm::vec3 pos, glm::vec2 dim, glm::vec2 density, float mass);
//    Fabric(std::vector<glm::vec2> vertices, glm::vec2 denFace, float m);
//    ~Fabric();
//    void clear();
//    void reset(glm::vec3 position);
//    FabricVertex* get(glm::vec2 relpos);
//    void create() override;
//    void applyTemporaryPositionChanges();
//};

class Collidable {
public:
    // Returns the normal vector of the mesh at the point of collision, if it exists. If not, return 0-vector.
    virtual glm::vec3 nearestNormal(glm::vec3 point) = 0;
    virtual bool isInBounds(glm::vec3 point) = 0;
};

class Cube : public Drawable, public Collidable {
public:
    Cube(glm::vec3 orig, glm::vec3 halfDims, float angle, glm::vec3 rotAxis);
    void create() override;

    glm::vec3 nearestNormal(glm::vec3 point) override;
    bool isInBounds(glm::vec3 point) override;
private:
    glm::vec3 origin, halfLengths;
    glm::mat4 rot, invrot;
};

class TruncatedPyramid : public Drawable, public Collidable {
public:
    TruncatedPyramid(glm::vec3 orig, glm::vec2 baseHalfDims, float pyrH, float truncH);
    void create() override;

    glm::vec3 nearestNormal(glm::vec3 point) override;
    bool isInBounds(glm::vec3 point) override;
private:
    glm::vec3 o, top, A, B, C, D;
    glm::vec2 bhd;
    float ph, th;
    glm::vec4 planeFront, planeRight, planeBack, planeLeft;
};

//class Slanted

class Vector : public Drawable {
public:
    glm::vec3 origin;
    glm::vec3 pointer;
    Vector(glm::vec3 o, glm::vec3 p);
    ~Vector();
    void create() override;
};

class Vectors : public Drawable {
private:
    std::vector<glm::vec4> pointers, colors;
    std::vector<GLuint> idx;
    glm::vec4 startColor, endColor;
public:
    void addVector(glm::vec3 origin, glm::vec3 pointer);
    void addVector(glm::vec3 origin, glm::vec3 pointer, glm::vec4 sc, glm::vec4 ec);
    void clear();
    void change(int index, glm::vec3 origin, glm::vec3 pointer);
    Vectors(glm::vec4 originColor, glm::vec4 pointerColor);
    ~Vectors();
    void create() override;
};

class Axis3D : public Vectors {
public:
    Axis3D(glm::vec3 origin, float scale);
};