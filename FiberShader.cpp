#include "FiberShader.h"
#include "CoreFiber.h"
#include "OrdinaryFiber.h"

FiberShader::FiberShader(
    const char* vertexPath,
    const char* fragmentPath,
    const char* geometryPath,
    const char* tessellationControlPath,
    const char* tessellationEvalPath)
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
}

void FiberShader::draw(Drawable *d, int texSlot)
{
    use();

    CoreFiber *cd = dynamic_cast<CoreFiber*>(d);
    OrdinaryFiber *od = dynamic_cast<OrdinaryFiber*>(d);

    if (cd != nullptr)
    {
        const Fiber& fiberType = cd->getFiberType();
        // set uniform variables
        setFiberParameters(fiberType);

        // http://wangchuan.github.io/coding/2016/05/26/multisampling-fbo.html explains this concept
        // Set the Vertex Attrib Pointers
        cd->bindVAO();

        // Configure Intermediate FrameBuffer for multisampling
        cd->bindInterFrameBuffer();
        glClearColor(0.f, 0.f, 0.f, 1.f); // temporary
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, fiberType.SCR_WIDTH, fiberType.CORE_HEIGHT);
        GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, buffers);

        // Draw Core Fiber render to Intermediate FrameBuffer
        glPatchParameteri(GL_PATCH_VERTICES, 4);
        glDrawElements(d->drawMode(), cd->elemCount(), GL_UNSIGNED_INT, 0);

        // Blit multisampled buffer(s) from Intermediate FrameBuffer to Off-Rendered FrameBuffer
        cd->bindReadFrameBuffer();
        cd->bindDrawFrameBuffer();
        glBlitFramebuffer(0, 0, fiberType.SCR_WIDTH, fiberType.CORE_HEIGHT, 0, 0, fiberType.SCR_WIDTH, fiberType.CORE_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_LINEAR);

        for (int i = 0; i < 3; i++) {
            glReadBuffer(buffers[i]);
            glDrawBuffer(buffers[i]);

            glBlitFramebuffer(0, 0,
                fiberType.SCR_WIDTH,
                fiberType.CORE_HEIGHT,
                0, 0,
                fiberType.SCR_WIDTH,
                fiberType.CORE_HEIGHT,
                GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }
        glDisableVertexAttribArray(POS_VAO_ID);
        glDisableVertexAttribArray(NORM_VAO_ID);
        glDisableVertexAttribArray(DIST_VAO_ID);
    }
    else if (od != nullptr)
    {
        const Fiber& fiberType = od->getFiberType();

        // set uniform variables
        setFiberParameters(fiberType);

        // Set the Vertex Attrib Pointers
        if (!od->bindVAO())
            std::runtime_error("Error binding VAO.");

        // Configure the Framebuffer and viewport
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, fiberType.SCR_WIDTH, fiberType.SCR_HEIGHT);

        //// Activate the textures
        //glActiveTexture(GL_TEXTURE0);
        //od->bindHeightTexture();
        //glGenerateMipmap(GL_TEXTURE_2D);

        //glActiveTexture(GL_TEXTURE1);
        //od->bindNormalTexture();
        //glGenerateMipmap(GL_TEXTURE_2D);

        //glActiveTexture(GL_TEXTURE2);
        //od->bindAlphaTexture();
        //glGenerateMipmap(GL_TEXTURE_2D);

        // Draw the fiber
        glPatchParameteri(GL_PATCH_VERTICES, 4);
        glDrawElements(d->drawMode(), od->elemCount(), GL_UNSIGNED_INT, 0);

        //glDisableVertexAttribArray(POS_VAO_ID);
        //glDisableVertexAttribArray(NORM_VAO_ID);
        //glDisableVertexAttribArray(DIST_VAO_ID);
    }
    else
    {
        throw std::runtime_error("Incorrect Drawable passed to Fiber Shader.");
    }
}

void FiberShader::setFiberParameters(const Fiber& m_fiberType)
{
    setInt("u_heightTexture", 0);
    setInt("u_normalTexture", 1);
    setInt("u_alphaTexture", 2);

    if (m_fiberType.fiberType == COTTON1 || m_fiberType.fiberType == COTTON2)
    {
        setVec3("objectColor", 217 / 255.f, 109 / 255.f, 2 / 255.f);
    }
    else if (m_fiberType.fiberType == SILK1 || m_fiberType.fiberType == SILK2)
    {
        setVec3("objectColor", 178 / 255.f, 168 / 255.f, 200 / 255.f);
    }
    else if (m_fiberType.fiberType == POLYESTER1)
    {
        setVec3("objectColor", 171 / 255.f, 201 / 255.f, 228 / 255.f);
    }
    else if (m_fiberType.fiberType == RAYON1 || m_fiberType.fiberType == RAYON2 || m_fiberType.fiberType == RAYON3 || m_fiberType.fiberType == RAYON4)
    {
        setVec3("objectColor", 98 / 255.f, 142 / 255.f, 56 / 255.f);
    }

    // TODO: replace w/ file implementation
    setInt("u_ply_num", m_fiberType.ply_num);
    setInt("u_m_fiberType_num", m_fiberType.fiber_num);

    setVec3("u_bounding_min", m_fiberType.bounding_min[0], m_fiberType.bounding_min[1], m_fiberType.bounding_min[2]);
    setVec3("u_bounding_max", m_fiberType.bounding_max[0], m_fiberType.bounding_max[1], m_fiberType.bounding_max[2]);

    setFloat("u_z_step_size", m_fiberType.z_step_size);
    setInt("u_z_step_num", m_fiberType.z_step_num);
    setInt("u_fly_step_size", m_fiberType.fly_step_size);

    setInt("u_yarn_clock_wise", m_fiberType.yarn_clock_wise);
    setInt("u_m_fiberType_clock_wise", m_fiberType.fiber_clock_wise);
    setFloat("u_yarn_alpha", m_fiberType.yarn_alpha);
    setFloat("u_alpha", m_fiberType.alpha);

    setFloat("u_yarn_radius", m_fiberType.yarn_radius);
    setFloat("u_ellipse_long", m_fiberType.ellipse_long);
    setFloat("u_ellipse_short", m_fiberType.ellipse_short);

    setInt("u_epsilon", m_fiberType.epsilon);
    setFloat("u_beta", m_fiberType.beta);
    setFloat("u_r_max", m_fiberType.r_max);

    setInt("u_use_migration", m_fiberType.use_migration);
    setFloat("u_s_i", m_fiberType.s_i);
    setFloat("u_rho_min", m_fiberType.rho_min);
    setFloat("u_rho_max", m_fiberType.rho_max);

    setInt("u_use_flyaways", m_fiberType.use_flyaways);
    setFloat("u_flyaway_hair_density", m_fiberType.flyaway_hair_density);
    setVec2("u_flyaway_hair_ze", m_fiberType.flyaway_hair_ze[0], m_fiberType.flyaway_hair_ze[1]);
    setVec2("u_flyaway_hair_r0", m_fiberType.flyaway_hair_r0[0], m_fiberType.flyaway_hair_r0[1]);
    setVec2("u_flyaway_hair_re", m_fiberType.flyaway_hair_re[0], m_fiberType.flyaway_hair_re[1]);
    setVec2("u_flyaway_hair_pe", m_fiberType.flyaway_hair_pe[0], m_fiberType.flyaway_hair_pe[1]);
    setFloat("u_flyaway_loop_density", m_fiberType.flyaway_loop_density);
    setVec2("u_flyaway_loop_r1", m_fiberType.flyaway_loop_r1[0], m_fiberType.flyaway_loop_r1[1]);
}