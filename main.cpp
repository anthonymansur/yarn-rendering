#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Shader.h"
#include "Camera.h"
#include "Fiber.h"
#include "CoreFiber.h"
#include "OrdinaryFiber.h"
#include "Pattern.h"

#include "mygl.h"

// glfw callbacks
// --------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

MyGL mygl;

FIBER_TYPE fiberType = COTTON1;
void addControlPoints(CoreFiber coreFiber, OrdinaryFiber fiber);
std::vector<ControlPoint> pointsToAdd;
float timeValue;

// NOTE: need to converge on these global OpenGL settings
int main()
{
    // Initialize OpenGL global settings
    // ---------------------------------
    mygl.initializeGL();
    GLFWwindow* m_window = mygl.getWindow();
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(m_window, mouse_callback);
    glfwSetScrollCallback(m_window, scroll_callback);
    glfwSetKeyCallback(m_window, key_callback);

    // Setup Dear ImGui context
    // ------------------------
    mygl.initializeImGuiContext();

    // Fiber
    // -----
    timeValue = glfwGetTime();

    CoreFiber coreFiber = CoreFiber(&mygl, fiberType);
    coreFiber.initializeGL();
    coreFiber.initFrameBuffer();

    coreFiber.render(); // render off-screen

    OrdinaryFiber fiber = OrdinaryFiber(&mygl, fiberType);
    fiber.initializeGL();
    fiber.setHeightTexture(coreFiber.getHeightTexture());
    fiber.setNormalTexture(coreFiber.getNormalTexture());
    fiber.setAlphaTexture(coreFiber.getAlphaTexture());

    addControlPoints(coreFiber, fiber);

    Camera& camera = mygl.getCamera();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(mygl.getWindow()))
    {
        // ImGUI
        // -----
        // NOTE: change UI contents depending if simulation or rendering is running
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        fiber.createGUIWindow();

        mygl.updateTime();

        // input
        // ----- 
        mygl.processInput();

        // render
        // ------

        // update the mvp matrices
        // TODO: change model so that world coordinate system is standardized.
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0, 0));
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)2400 / (float)2400, 0.01f, 10.0f);

        Shader* fiberShader = mygl.getFiberShader();

        fiberShader->use();
        fiberShader->setMat4("model", model);
        fiberShader->setMat4("view", view);
        fiberShader->setMat4("projection", projection);
        fiberShader->setVec3("camera_pos", camera.Position);
        fiberShader->setVec3("view_dir", camera.Front);

        fiber.render();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (fiber.getFiberType() != fiberType)
        {
            // TODO: implement
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        mygl.swapAndPoll();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void addControlPoints(CoreFiber coreFiber, OrdinaryFiber fiber)
{
    // NOTE: control points must be added in the following order: [a b c d] [b c d e] [c d e f] 
    // to draw the curves from b-c-d-e, where a and f are the end points.
    // TODO: verify CORE vs COMPLETE inconsistency

    pointsToAdd.push_back(ControlPoint{ glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0 });
    pointsToAdd.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1 });
    pointsToAdd.push_back(ControlPoint{ glm::vec3((coreFiber.getFiberAlpha() - 0.01f) / 2.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2 });
    pointsToAdd.push_back(ControlPoint{ glm::vec3(coreFiber.getFiberAlpha() / 2.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3 });
    for (const ControlPoint& point : pointsToAdd) {
        coreFiber.addPoint(point, true);
    }
    coreFiber.loadPoints();
    pointsToAdd.clear();

    enum RENDER{
        TEST0, 
        TEST1,
        TEST2,
        TEST3,
        TEST4,
        TEST5,
        TEST6,
        TEST7,
        LIVE
    };

    RENDER render = TEST0;
    std::vector<Strand> strands;

    // TODO: implement hair fibers

    switch (render)
    {
        case TEST0:
            // Basic yarn along the X-axis
            // STATUS: passed
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0, 0 });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1, 0.01f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.99f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2, 0.99f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3, 1.f });
            strands.push_back(Strand{ pointsToAdd });

            fiber.addStrands(strands);
            fiber.loadPoints();
            break;
        case TEST1:
            // Additional control point. Should render basic yarn from previous test.
            // STATUS: passed
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0, 0 });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1, 0.01f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.25f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2, 0.25f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.5f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3, 0.5f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.75f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 4, 0.75f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.99f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 5, 0.99f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 6, 1.f });
            strands.push_back(Strand{ pointsToAdd });
            fiber.addStrands(strands);
            fiber.loadPoints();
            break;
        case TEST2:
            // Additional strand on top of basic yarn.
            // STATUS: passed
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0, 0 });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1, 0.01f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.99f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2, 0.99f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3, 1.f });
            strands.push_back(Strand{ pointsToAdd });

            pointsToAdd.clear();
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.0f, 0.2f, 0.f), glm::vec3(0.f, 1.f, 0.f), 4, 0 });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.01f, 0.2f, 0.f), glm::vec3(0.f, 1.f, 0.f), 5, 0.01f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.99f, 0.2f, 0.f), glm::vec3(0.f, 1.f, 0.f), 6, 0.99f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(1.f, 0.2f, 0.f), glm::vec3(0.f, 1.f, 0.f), 7, 1.f });
            strands.push_back(Strand{ pointsToAdd });

            fiber.addStrands(strands);
            fiber.loadPoints();
            break;
        case TEST3:
            // Basic yarn along the Y-axis
            // STATUS: passed
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), 0, 0.f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.f, 0.01f, 0.f), glm::vec3(1.f, 0.f, 0.f), 1, 0.01f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.f, 0.99f, 0.f), glm::vec3(1.f, 0.f, 0.f), 3, 0.99f });
            pointsToAdd.push_back(ControlPoint{ glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, 0.f, 0.f), 4, 1.f });
            strands.push_back(Strand{ pointsToAdd });
            fiber.addStrands(strands);
            fiber.loadPoints();
            break;
        case TEST4:
        {
            // Basic weave pattern - horizontal
            // TODO: make fiber_center = yarn_center in fiber_tess_eval.glsl
            // TODO: remove texture effect from fiber_fragment.glsl
            // TODO: make line height equal to yarn_radius * 2 in fiber_gemoetry.glsl
            // STATUS: passed
            Pattern pattern = Pattern(&fiber);
            strands = pattern._getHorizontalStrand();
            fiber.addStrands(strands);
            fiber.loadPoints();
        }
            break;
        case TEST5:
        {
            // Basic weave pattern - vertical
            // TODO: make fiber_center = yarn_center in fiber_tess_eval.glsl
            // TODO: remove texture effect from fiber_fragment.glsl
            // TODO: make line height equal to yarn_radius * 2 in fiber_gemoetry.glsl
            // STATUS: passed
            Pattern pattern = Pattern(&fiber);
            strands = pattern._getVerticalStrand();
            fiber.addStrands(strands);
            fiber.loadPoints();
        }
        break;
        case TEST6:
        {
            // Basic weave pattern
            // TODO: make fiber_center = yarn_center in fiber_tess_eval.glsl
            // TODO: remove texture effect from fiber_fragment.glsl
            // TODO: make line height equal to yarn_radius * 2 in fiber_gemoetry.glsl
            // STATUS: passed
            Pattern pattern = Pattern(&fiber);
            strands = pattern.getBasicWeave(10);
            fiber.addStrands(strands);
            fiber.loadPoints();
        }
            break;
        case TEST7:
        {
            Pattern pattern = Pattern(&fiber);
            strands = pattern.getBasicWeave(50);
            fiber.addStrands(strands);
            fiber.loadPoints();
        }
        break;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    mygl.framebuffer_size_callback(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    mygl.mouse_callback(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    mygl.scroll_callback(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    mygl.key_callback(key, scancode, action, mods);
}