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
#include "FiberShader.h"
#include "Camera.h"
#include "Fiber.h"
#include "CoreFiber.h"
#include "OrdinaryFiber.h"
#include "Pattern.h"

// global variables
// ----------------
float deltaTime = 0.f; // Time between current frame and last frame
float lastFrame; // Time of last frame
float lastX;
float lastY;
bool firstMouse;
bool moveCamera;
GLFWwindow* window;
Camera camera;

// glfw callbacks
// --------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void processInput();

void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam)
{
    std::cout << "OGLE " << source << " " << type << " " << message << std::endl;
}

void onGLError(int error, const char* description)
{
    std::cout << "OGLE: " << description << std::endl;
    fprintf(stderr, "OpenGL Error: %s\n", description);
}

int main()
{
    // Initialize OpenGL global settings
    // ---------------------------------
    glfwInit();
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // for antialiasing

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(2400, 2400, "Fiber-Level Cloth Rendering", NULL, NULL);
    if (!window || window == NULL)
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

    glfwSetErrorCallback(onGLError);


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

    glfwMakeContextCurrent(window);

    // Debug Output
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    /*glDebugMessageCallback(debugMessage, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);*/

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

    // Initialize Camera
    // -----------------
    camera = Camera(glm::vec3(0.0f, 0.f, 2.f));

    // Fiber
    // -----
    // Set Fiber-specific variables
    FIBER_TYPE fiberType = COTTON1;
    Fiber fiber = Fiber(fiberType);
    float timeValue = glfwGetTime();

    // Create Core Fiber
    CoreFiber coreFiber = CoreFiber(fiber);
    coreFiber.create();

    // Create Ordinary Fiber
    OrdinaryFiber ordinaryFiber = OrdinaryFiber(fiber);
    ordinaryFiber.create();

    // Set the textures of CoreFiber into OrdinaryFiber
    ordinaryFiber.setHeightTexture(coreFiber.getHeightTexture());
    ordinaryFiber.setNormalTexture(coreFiber.getNormalTexture());
    ordinaryFiber.setAlphaTexture(coreFiber.getAlphaTexture());

    glfwSetWindowSize(window, 2400, 2400);

    // Initialize Shaders
    // ------------------
    FiberShader coreShader = FiberShader(fiber, "fiber_vertex.glsl", "core_fragment.glsl", "core_geometry.glsl", "core_tess_control.glsl", "core_tess_eval.glsl");
    FiberShader fiberShader = FiberShader(fiber, "fiber_vertex.glsl", "fiber_fragment.glsl", "fiber_geometry.glsl", "fiber_tess_control.glsl", "fiber_tess_eval.glsl");

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // ImGUI
        // -----
        // NOTE: change UI contents depending if simulation or rendering is running
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
        processInput();

        // render
        // ------

        // update the mvp matrices
        // TODO: change model so that world coordinate system is standardized.
        //glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0, 0));
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.366114f / 4.f, 0, 0));
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)2400 / (float)2400, 0.01f, 10.0f);

        // Update uniform variables defining the camera properties
        // -------------------------------------------------------
        coreShader.use();
        coreShader.setMat4("model", model);
        coreShader.setMat4("view", view);
        coreShader.setMat4("projection", projection);
        coreShader.setVec3("camera_pos", camera.Position);
        coreShader.setVec3("view_dir", camera.Front);
        fiberShader.use();
        fiberShader.setMat4("model", model);
        fiberShader.setMat4("view", view);
        fiberShader.setMat4("projection", projection);
        fiberShader.setVec3("camera_pos", camera.Position);
        fiberShader.setVec3("view_dir", camera.Front);

        // resize GL
        // ---------
        //glfwSetWindowSize(window, 3000, 3000);
        //glViewport(0, 0, 3000, 3000);

        // render Fiber
        // ------------
        coreShader.draw(&coreFiber, -1);
        fiberShader.draw(&ordinaryFiber, -1);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (fiber.getFiberType() != fiberType)
        {
            // TODO: implement
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

void processInput()
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