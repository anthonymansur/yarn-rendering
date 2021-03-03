#pragma once
#include <vector>

#include "InterleavedDrawable.h"
#include "Fiber.h"

class FiberDrawable : public InterleavedDrawable
{
public:
    FiberDrawable(const Fiber& fiber);
    ~FiberDrawable();

    void create();
    void addPoint(ControlPoint, bool);
    void addStrands(const std::vector<Strand>& strands);

    const Fiber& getFiberType() const;

    GLenum drawMode();

protected:
    GLuint heightTexture;
    GLuint normalTexture;
    GLuint alphaTexture;

    std::vector<float> m_points;
    std::vector<int> m_indices;

    const Fiber& m_fiber;

    bool heightTexBound;
    bool normalTexBound;
    bool alphaTexBound;
};