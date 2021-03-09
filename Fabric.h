#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include "Drawable.h"
#include <memory>
#include <map>
#include <string>

class Fabric;
class FabricVertex;

class iterator {
public:
    using value_type = FabricVertex*;
    using difference_type = std::ptrdiff_t;
    using reference = FabricVertex*&;
    using pointer = FabricVertex**;
    using iterator_category = std::forward_iterator_tag;
    /*using nextFunc = reference(Fabric::*)(std::size_t);
    typedef reference (Fabric::*NxFunc)(std::size_t);*/

    iterator() : fab(), idx(0) { }
    iterator(Fabric* b, std::size_t idx) : fab(b), idx(idx) {}

    reference operator*() const;// { return fab->next(idx); }
    pointer operator->() const { return &**this; }

    friend iterator& operator++(iterator& rhs) { ++rhs.idx; return rhs; }
    friend iterator operator++(iterator& lhs, int) { auto cp = lhs; ++lhs; return cp; }

    friend bool operator==(iterator lhs, iterator rhs) { return lhs.idx == rhs.idx; }
    friend bool operator!=(iterator lhs, iterator rhs) { return !(lhs == rhs); }

private:
    Fabric* fab;
    std::size_t idx;
};

//class const_iterator {
//public:
//    using value_type = FabricVertex*;
//    using difference_type = std::ptrdiff_t;
//    using reference = const FabricVertex*&;
//    using pointer = const FabricVertex**;
//    using iterator_category = std::forward_iterator_tag;
//
//    const_iterator(Fabric* b, std::size_t idx) : fab(b), idx(idx) {}
//
//    reference operator*() const { return fab->c_next(idx); }
//    pointer operator->() const { return &**this; }
//
//    friend iterator& operator++(iterator& rhs) { ++rhs.idx; return rhs; }
//    friend iterator operator++(iterator& lhs, int) { auto cp = lhs; ++lhs; return cp; }
//
//    friend bool operator==(iterator lhs, iterator rhs) { return lhs.idx == rhs.idx; }
//    friend bool operator!=(iterator lhs, iterator rhs) { return !(lhs == rhs); }
//
//private:
//    Fabric* fab;
//    std::size_t idx;
//};

class FabricVertex {
public:
    int id;
    //std::map<std::string, glm::vec3> values;
    std::vector<glm::vec3> attr;
    glm::vec3 debugColor;
    glm::vec3 localPos, tmpPos, origPos, velocity;
    bool at_rest = false;

    FabricVertex* posWeft, * negWeft, * posWarp, * negWarp, * other;

    glm::vec2 edgeLength; //Warp, Weft

    FabricVertex(glm::vec3 pos, glm::vec2 length);
    void _reset();
};

class Edge {
public:
    Edge();
    ~Edge();
    void calcProperties();

public:
    float refAngle;
    std::vector<FabricVertex*> edgeVertices; // THIS CLASS DOES NOT OWN THE VERTICES AND WILL NOT DELETE THEM
    int nullDirection; // 0 = nWeft, 1 = pWeft, 2 = nWarp, 3 = pWarp
};

class Fabric : public Drawable {
public:
    virtual ~Fabric() {};
    Fabric(std::string name, glm::vec3 pos, float mass);
    virtual void create() override = 0;
    virtual void reset(glm::vec3 pos) = 0;
    virtual void connect(Fabric* fab, int edgeA, int edgeB) = 0;
    virtual std::size_t size() const = 0;
    virtual FabricVertex*& next(std::size_t idx) = 0;
    void mult(glm::mat3 m);
    iterator begin()
    {
        return { this, 0 };
    }
    iterator end()
    {
        return { this, size() };
    }

    void applyTemporaryPositionChanges()
    {
        for (FabricVertex*& v : *this) {
            v->localPos = v->tmpPos;
        }
    }
public:
    // Identifier for this particular Fabric part
    std::string name;
    // Starting position of Fabric
    glm::vec3 position;
    // Total mass of fabric part
    float mass;
    std::vector<std::vector<FabricVertex*>> edges;
    bool debug = true;
};

class SquareFabric : public Fabric {
public:
    SquareFabric(std::string name, glm::vec3 pos, glm::vec3 pos2, int stringDensity, float mass);
    SquareFabric(std::string name, glm::vec3 pos, glm::vec2 dim, glm::vec2 density, float mass);
    ~SquareFabric();

    void create() override;
    void reset(glm::vec3 pos);
    std::size_t size() const;
    FabricVertex*& next(std::size_t idx);
    void connect(Fabric* fab, int edgeA, int edgeB);

    FabricVertex* get(glm::vec2 relpos);
private:
    std::vector<FabricVertex*> vertices;
    glm::vec2 amount;
    bool isString;
};
class PolyFabric : public Fabric {
public:
    PolyFabric(std::string name, glm::vec3 pos, std::vector<glm::vec2>& vertices, glm::vec2 denFace, float m);
    ~PolyFabric();

    void create() override;
    void reset(glm::vec3 pos);
    std::size_t size() const;
    FabricVertex*& next(std::size_t idx);
    void connect(Fabric* fab, int edgeA, int edgeB);

private:
    std::vector<FabricVertex*> vertices;
    std::vector<std::vector<FabricVertex*>> tmpArr;
    FabricVertex*** arr;
    int xVals, yVals, amount;

};

//class FabricA : public Drawable {
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
//    FabricA(glm::vec3 pos, glm::vec3 pos2, int stringDensity, float mass);
//    FabricA(glm::vec3 pos, glm::vec2 dim, glm::vec2 density, float mass);
//    FabricA(std::vector<glm::vec2> vertices, glm::vec2 denFace, float m);
//    ~FabricA();
//    void clear();
//    void reset(glm::vec3 position);
//    FabricVertex* get(glm::vec2 relpos);
//    void create() override;
//    void applyTemporaryPositionChanges();
//};