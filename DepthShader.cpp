#include "DepthShader.h"
#include "OrdinaryFiber.h"
#include <glm/gtc/matrix_transform.hpp>

DepthShader::DepthShader(
    const char* vertexPath,
    const char* fragmentPath,
    const char* geometryPath,
    const char* tessellationControlPath,
    const char* tessellationEvalPath) : 
    FiberShader::FiberShader(vertexPath, fragmentPath, geometryPath, tessellationControlPath, tessellationEvalPath)
{
}

void DepthShader::draw(Drawable* d, int texSlot)
{
    use();

    OrdinaryFiber* od = dynamic_cast<OrdinaryFiber*>(d);

    if (od == nullptr)
        throw std::runtime_error("Incorrect Drawable passed to Depth Shader.");

    const Fiber& fiberType = od->getFiberType();

    // set uniform variables
    setFiberParameters(fiberType);

    // Set the Vertex Attrib Pointers
    if (!od->bindVAO())
        throw std::runtime_error("Error binding VAO.");

    // Render to depth map
    // -------------------
    glViewport(0, 0, od->SHADOW_WIDTH, od->SHADOW_HEIGHT);
    if (!od->bindDepthMap())
        throw std::runtime_error("Error binding Depth map.");
    glClear(GL_DEPTH_BUFFER_BIT);
    // Draw the fiber
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawElements(d->drawMode(), od->elemCount(), GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}