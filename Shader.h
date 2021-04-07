#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Drawable.h"
#include "InterleavedDrawable.h"

// NOTE: need to merge Alex's shader with this shader. Perhaps use some inheritance

class Shader
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath,
        const char* fragmentPath,
        const char* geometryPath = nullptr,
        const char* tessellationControlPath = nullptr,
        const char* tessellationEvalPath = nullptr);
    // activate the shader
    // ------------------------------------------------------------------------
    void use() const;
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const;
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const;
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const;
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w);
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    virtual void draw(Drawable* d, int texSlot = 0);
    void draw(InterleavedDrawable* d, int texSlot = 0);

    void setTime(int t);

    void setModelMatrix(const glm::mat4& model);

    void setViewProjMatrix(const glm::mat4& vp);

    void setCameraPosition(const glm::vec3 cam);

    int unifSampler2D;
    int unifTime;

    int unifModel;
    int unifModelInvTr;
    int unifViewProj;
    int unifCam;

    int attrPos;
    int attrNor;
    int attrUV;
    int attrCol;

protected:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, std::string type);
};