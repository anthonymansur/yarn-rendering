#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
#include "Pattern.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void addControlPoints();

FIBER_TYPE fiberType = COTTON1;
RENDER_TYPE renderType = COMPLETE; // DEBUG
Fiber fiber = Fiber(fiberType, renderType);
Camera camera(glm::vec3(0.0f, 0.f, 2.f));
float lastX = fiber.SCR_WIDTH / 2.0f;
float lastY = fiber.SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

bool moveCamera = true;

std::vector<ControlPoint> pointsToAdd;

float timeValue;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // for antialiasing

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(fiber.SCR_WIDTH, fiber.SCR_HEIGHT, "Fiber-Level Cloth Rendering", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE); // for antialiasing
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode

    // Setup Dear ImGui context
    // ------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
    io.Fonts->AddFontDefault();
    io.FontGlobalScale = 1.25f;

    // Fiber
    // -----
    timeValue = glfwGetTime();
    fiber.setWindow(window);
    fiber.initializeGL();
    fiber.initShaders();
    fiber.initFrameBuffer();
    addControlPoints();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // ImGUI
        // -----
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        fiber.createGUIWindow();

        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------

        // update the mvp matrices
        glm::mat4 model;
        if (fiber.getRenderType() == CORE)
        {
            model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.366114f / 4.f, 0, 0));
        }
        if (fiber.getRenderType() == FIBER || fiber.getRenderType() == COMPLETE)
        {
            model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0, 0));
        }
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)fiber.SCR_WIDTH / (float)fiber.SCR_HEIGHT, 0.01f, 10.0f);

        if (fiber.getRenderType() != COMPLETE)
        {
            const Shader& shader = fiber.getActiveShader();
            shader.use();

            shader.setMat4("model", model);
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);
            shader.setVec3("camera_pos", camera.Position);
            shader.setVec3("view_dir", camera.Front);
            shader.setFloat("u_time", timeValue);
        }
        else
        {
            const std::vector<Shader*> shaders = fiber.getActiveShaders();

            for (int i = 0; i < 2; i++)
            {
                shaders.at(i)->use();
                shaders.at(i)->setMat4("model", model);
                shaders.at(i)->setMat4("view", view);
                shaders.at(i)->setMat4("projection", projection);
                shaders.at(i)->setVec3("camera_pos", camera.Position);
                shaders.at(i)->setVec3("view_dir", camera.Front);
            }
        }

        fiber.render();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (fiber.getFiberType() != fiberType || fiber.getRenderType() != renderType)
        {
            fiberType = fiber.getFiberType();
            renderType = fiber.getRenderType();
            pointsToAdd.clear();
            fiber = Fiber(fiberType, renderType);
            timeValue = glfwGetTime();
            fiber.setWindow(window);
            fiber.initializeGL();
            fiber.initShaders();
            fiber.initFrameBuffer();
            addControlPoints();
            framebuffer_size_callback(window, fiber.SCR_WIDTH, fiber.SCR_HEIGHT);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!moveCamera)
        return;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
    {
        moveCamera = !moveCamera;
        if (!moveCamera)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
        {
            glfwSetCursorPos(window, lastX, lastY);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void addControlPoints()
{
    // NOTE: control points must be added in the following order: [a b c d] [b c d e] [c d e f] 
    // to draw the curves from b-c-d-e, where a and f are the end points.
    if (fiber.getRenderType() == COMPLETE)
    {
        std::cout << "LOGIC ERROR" << std::endl;
        pointsToAdd.push_back(ControlPoint{ glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0 });
        pointsToAdd.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1 });
        pointsToAdd.push_back(ControlPoint{ glm::vec3(0.99f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2 });
        pointsToAdd.push_back(ControlPoint{ glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3 });
        for (const ControlPoint& point : pointsToAdd) {
            fiber.addPoint(point, true);
        }
        fiber.loadPoints(true);
        pointsToAdd.clear();
    }
    else if (fiber.getRenderType() == CORE)
    {
        pointsToAdd.push_back(ControlPoint{ glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0 });
        pointsToAdd.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1 });
        pointsToAdd.push_back(ControlPoint{ glm::vec3((fiber.getFiberAlpha() - 0.01f) / 2.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2 });
        pointsToAdd.push_back(ControlPoint{ glm::vec3(fiber.getFiberAlpha() / 2.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3 });
        for (const ControlPoint& point : pointsToAdd) {
            fiber.addPoint(point, true);
        }
        fiber.loadPoints(true);
        pointsToAdd.clear();
    }

    // TODO: check algorithm to render multiple strands of yarn before testing pattern weave

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

    //TODO: Fix height problem between horizontal and vertical yarns
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
            fiber.loadPoints(false);
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
            fiber.loadPoints(false);
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
            fiber.loadPoints(false);
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
            fiber.loadPoints(false);
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
            fiber.loadPoints(false);
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
            fiber.loadPoints(false);
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
            fiber.loadPoints(false);
        }
            break;
        case TEST7:
        {
            Pattern pattern = Pattern(&fiber);
            strands = pattern._getHorizontalStrand();
            std::vector<Strand> strands2 = pattern._getVerticalStrand(strands.at(0).points.size());
            for (const Strand& strand : strands2)
                strands.push_back(strand);
            fiber.addStrands(strands);
            fiber.loadPoints(false);
        }
        break;
    }
}