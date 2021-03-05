#pragma once
#include <vector>

#include "InterleavedDrawable.h"
#include "Fiber.h"

namespace
{
    const GLuint POS_VAO_ID = 0;
    const GLuint NORM_VAO_ID = 1;
    const GLuint DIST_VAO_ID = 2;
    const GLuint STRIDE = 7;
}

class FiberDrawable : public InterleavedDrawable
{
public:
    FiberDrawable(const Fiber& fiber);
    ~FiberDrawable();

    virtual void create();
    void addPoint(ControlPoint, bool);
    void addStrands(const std::vector<Strand>& strands);

    inline const Fiber& getFiberType() const
    {
        return m_fiber;
    }

    inline GLenum drawMode()
    {
        return GL_PATCHES;
    }

protected:
    GLuint heightTexture;
    GLuint normalTexture;
    GLuint alphaTexture;

    // NOTE: may not need to store these in the class. Will be kept in the buffer.
    std::vector<float> m_points;
    std::vector<int> m_indices;

    const Fiber& m_fiber;

    bool heightTexBound;
    bool normalTexBound;
    bool alphaTexBound;
};