#pragma once
#include "FiberShader.h"

// TODO: make fiberShader an abstract class where DepthShader can inherit from

// TODO: depth map isn't working. Try to render the depth map onto the screen.

class DepthShader : public FiberShader
{
public:
    DepthShader(
        const char* vertexPath,
        const char* fragmentPath,
        const char* geometryPath = nullptr,
        const char* tessellationControlPath = nullptr,
        const char* tessellationEvalPath = nullptr);

    void draw(Drawable* d, int texSlot = 0);
};