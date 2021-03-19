#include "Shader.h"

Shader::Shader()
{}

Shader::Shader(const char* vertexPath,
    const char* fragmentPath,
    const char* geometryPath,
    const char* tessellationControlPath,
    const char* tessellationEvalPath)
    : ID(0), unifSampler2D(-1), unifTime(-1), unifModel(-1),
    unifModelInvTr(-1), unifViewProj(-1), unifCam(-1), attrPos(-1),
    attrNor(-1), attrUV(-1), attrCol(-1)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::string tessellationControlCode;
    std::string tessellationEvalCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;
    std::ifstream tcShaderFile;
    std::ifstream teShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    tcShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    teShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // if geometry shader path is present, also load a geometry shader
        if (geometryPath != nullptr)
        {
            gShaderFile.open(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
        // if tessellation control shader path is present, also load a tessellation controller 
        // shader
        if (tessellationControlPath != nullptr)
        {
            tcShaderFile.open(tessellationControlPath);
            std::stringstream tcShaderStream;
            tcShaderStream << tcShaderFile.rdbuf();
            tcShaderFile.close();
            tessellationControlCode = tcShaderStream.str();
        }
        // if tessellation control shader path is present, also load a tessellation evaluation 
        // shader
        if (tessellationEvalPath != nullptr)
        {
            teShaderFile.open(tessellationEvalPath);
            std::stringstream teShaderStream;
            teShaderStream << teShaderFile.rdbuf();
            teShaderFile.close();
            tessellationEvalCode = teShaderStream.str();
        }
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // if geometry shader is given, compile geometry shader
    unsigned int geometry;
    if (geometryPath != nullptr)
    {
        const char* gShaderCode = geometryCode.c_str();
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "GEOMETRY");
    }
    unsigned int tessControl;
    if (tessellationControlPath != nullptr)
    {
        const char* tcShaderCode = tessellationControlCode.c_str();
        tessControl = glCreateShader(GL_TESS_CONTROL_SHADER);
        glShaderSource(tessControl, 1, &tcShaderCode, NULL);
        glCompileShader(tessControl);
        checkCompileErrors(tessControl, "TESSELLATION CONTROL");
    }
    unsigned int tessEval;
    if (tessellationEvalPath != nullptr)
    {
        const char* teShaderCode = tessellationEvalCode.c_str();
        tessEval = glCreateShader(GL_TESS_EVALUATION_SHADER);
        glShaderSource(tessEval, 1, &teShaderCode, NULL);
        glCompileShader(tessEval);
        checkCompileErrors(tessEval, "TESSELLATION EVALUATION");
    }
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometryPath != nullptr)
        glAttachShader(ID, geometry);
    if (tessellationControlPath != nullptr)
        glAttachShader(ID, tessControl);
    if (tessellationEvalPath != nullptr)
        glAttachShader(ID, tessEval);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometryPath != nullptr)
        glDeleteShader(geometry);

    attrPos = glGetAttribLocation(ID, "vs_Pos");
    attrNor = glGetAttribLocation(ID, "vs_Nor");
    attrUV = glGetAttribLocation(ID, "vs_UV");
    attrCol = glGetAttribLocation(ID, "vs_Col");

    unifSampler2D = glGetUniformLocation(ID, "u_Texture");
    unifTime = glGetUniformLocation(ID, "u_Time");

    unifModel = glGetUniformLocation(ID, "u_Model");
    unifModelInvTr = glGetUniformLocation(ID, "u_ModelInvTr");
    unifViewProj = glGetUniformLocation(ID, "u_ViewProj");
    unifCam = glGetUniformLocation(ID, "u_Cam");
}


void Shader::use() const
{
    glUseProgram(ID);
}


void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, float x, float y, float z, float w)
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::draw(Drawable* d, int texSlot)
{
    use();

    if (unifSampler2D != -1)
    {
        glUniform1i(unifSampler2D, texSlot);
    }

    if (attrPos != -1 && d->bindPos()) {
        glEnableVertexAttribArray(attrPos);
        glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 0, NULL);
    }

    if (attrNor != -1 && d->bindNor()) {
        glEnableVertexAttribArray(attrNor);
        glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 0, NULL);
    }

    if (attrUV != -1 && d->bindUV()) {
        glEnableVertexAttribArray(attrUV);
        glVertexAttribPointer(attrUV, 2, GL_FLOAT, false, 0, NULL);
    }

    if (attrCol != -1 && d->bindCol()) {
        glEnableVertexAttribArray(attrCol);
        glVertexAttribPointer(attrCol, 3, GL_FLOAT, false, 0, NULL);
    }

    d->bindIdx();
    glDrawElements(d->drawMode(), d->elemCount(), GL_UNSIGNED_INT, 0);

    if (attrPos != -1) glDisableVertexAttribArray(attrPos);
    if (attrNor != -1) glDisableVertexAttribArray(attrNor);
    if (attrUV != -1) glDisableVertexAttribArray(attrUV);
    if (attrCol != -1) glDisableVertexAttribArray(attrCol);
}

void Shader::draw(InterleavedDrawable* d, int texSlot)
{
    use();

    if (unifSampler2D != -1)
    {
        glUniform1i(unifSampler2D, texSlot);
    }

    if (d->bindVAO())
    {
        glDrawArrays(d->drawMode(), 0, d->elemCount());
    }
    else
    {
        throw std::runtime_error("Could not bind to VAO.");
    }
}

void Shader::setTime(int t)
{
    use();
    if (unifTime != -1)
    {
        glUniform1f(unifTime, t);
    }
}

void Shader::setModelMatrix(const glm::mat4& model)
{
    use();

    if (unifModel != -1) {
        glUniformMatrix4fv(unifModel,
            1,
            GL_FALSE,
            &model[0][0]);
    }
    if (unifModelInvTr != -1) {
        glm::mat3 modelinvtr = glm::inverse(glm::transpose(glm::mat3(model)));
        glUniformMatrix3fv(unifModelInvTr,
            1,
            GL_FALSE,
            &modelinvtr[0][0]);
    }
}

void Shader::setViewProjMatrix(const glm::mat4& vp)
{
    use();

    if (unifViewProj != -1) {
        glUniformMatrix4fv(unifViewProj,
            1,
            GL_FALSE,
            &vp[0][0]);
    }
}

void Shader::setCameraPosition(const glm::vec3 cam)
{
    use();
    glUniform3fv(unifCam, 1, &cam[0]);
}


void Shader::checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}