#pragma once
#include "Shader.h"
#include "Fiber.h"

class FiberShader : public Shader
{
public:
    FiberShader(
        const char* vertexPath,
        const char* fragmentPath,
        const char* geometryPath = nullptr,
        const char* tessellationControlPath = nullptr,
        const char* tessellationEvalPath = nullptr);

    void draw(Drawable* d, int texSlot);

    void setFiberParameters(const Fiber& fiberType);
};