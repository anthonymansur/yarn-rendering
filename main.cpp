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

void addCoreControlPoints(CoreFiber &coreFiber);
void addFiberControlPoints(OrdinaryFiber &fiber);

int main()
{
    // Initialize OpenGL global settings
    // ---------------------------------
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

    // Initialize Shaders
    // ------------------
    Shader coreShader = Shader("fiber_vertex.glsl", "core_fragment.glsl", "core_geometry.glsl", "core_tess_control.glsl", "core_tess_eval.glsl");
    Shader fiberShader = Shader("fiber_vertex.glsl", "fiber_fragment.glsl", "fiber_geometry.glsl", "fiber_tess_control.glsl", "fiber_tess_eval.glsl");

    // Fiber
    // -----
    FIBER_TYPE fiberType = COTTON1;
    std::vector<ControlPoint> points;
    float timeValue = glfwGetTime();

    CoreFiber coreFiber = CoreFiber(&coreShader, fiberType);
    coreFiber.initializeGL();
    coreFiber.initFrameBuffer();

    addCoreControlPoints(coreFiber);

    glfwSetWindowSize(window, coreFiber.SCR_WIDTH, coreFiber.CORE_HEIGHT);
    glViewport(0, 0, coreFiber.SCR_WIDTH, coreFiber.CORE_HEIGHT);
    coreFiber.render(); // render off-screen

    OrdinaryFiber fiber = OrdinaryFiber(&fiberShader, fiberType);
    fiber.initializeGL();
    fiber.setHeightTexture(coreFiber.getHeightTexture());
    fiber.setNormalTexture(coreFiber.getNormalTexture());
    fiber.setAlphaTexture(coreFiber.getAlphaTexture());
    
    addFiberControlPoints(fiber);

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
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0, 0));
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)2400 / (float)2400, 0.01f, 10.0f);

        // Update uniform variables defining the camera properties
        // -------------------------------------------------------
        fiberShader.use();
        fiberShader.setMat4("model", model);
        fiberShader.setMat4("view", view);
        fiberShader.setMat4("projection", projection);
        fiberShader.setVec3("camera_pos", camera.Position);
        fiberShader.setVec3("view_dir", camera.Front);

        // resize GL
        // ---------
        glfwSetWindowSize(window, 3000, 3000);
        glViewport(0, 0, 3000, 3000);

        // render Fiber
        // ------------
        fiber.render();
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

void addCoreControlPoints(CoreFiber &coreFiber)
{
    // TODO: verify proper core texture mapping
    std::vector<ControlPoint> points;
    points.push_back(ControlPoint{ glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0 });
    points.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1 });
    points.push_back(ControlPoint{ glm::vec3(0.99f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2 });
    points.push_back(ControlPoint{ glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3 });
    //points.push_back(ControlPoint{ glm::vec3((coreFiber.getFiberAlpha() - 0.01f) / 2.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2 });
    //points.push_back(ControlPoint{ glm::vec3(coreFiber.getFiberAlpha() / 2.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3 });
    for (const ControlPoint& point : points) {
        coreFiber.addPoint(point, true);
    }
    coreFiber.loadPoints();
}

void addFiberControlPoints(OrdinaryFiber &fiber)
{
    enum RENDER {
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
    std::vector<ControlPoint> points;

    switch (render)
    {
    case TEST0:
        // Basic yarn along the X-axis
        // STATUS: passed
        points.push_back(ControlPoint{ glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0, 0 });
        points.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1, 0.01f });
        points.push_back(ControlPoint{ glm::vec3(0.99f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2, 0.99f });
        points.push_back(ControlPoint{ glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3, 1.f });
        strands.push_back(Strand{ points });

        fiber.addStrands(strands);
        fiber.loadPoints();
        break;
    case TEST1:
        // Additional control point. Should render basic yarn from previous test.
        // STATUS: passed
        points.push_back(ControlPoint{ glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0, 0 });
        points.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1, 0.01f });
        points.push_back(ControlPoint{ glm::vec3(0.25f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2, 0.25f });
        points.push_back(ControlPoint{ glm::vec3(0.5f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3, 0.5f });
        points.push_back(ControlPoint{ glm::vec3(0.75f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 4, 0.75f });
        points.push_back(ControlPoint{ glm::vec3(0.99f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 5, 0.99f });
        points.push_back(ControlPoint{ glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 6, 1.f });
        strands.push_back(Strand{ points });
        fiber.addStrands(strands);
        fiber.loadPoints();
        break;
    case TEST2:
        // Additional strand on top of basic yarn.
        // STATUS: passed
        points.push_back(ControlPoint{ glm::vec3(0.0f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0, 0 });
        points.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1, 0.01f });
        points.push_back(ControlPoint{ glm::vec3(0.99f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2, 0.99f });
        points.push_back(ControlPoint{ glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3, 1.f });
        strands.push_back(Strand{ points });

        points.clear();
        points.push_back(ControlPoint{ glm::vec3(0.0f, 0.2f, 0.f), glm::vec3(0.f, 1.f, 0.f), 4, 0 });
        points.push_back(ControlPoint{ glm::vec3(0.01f, 0.2f, 0.f), glm::vec3(0.f, 1.f, 0.f), 5, 0.01f });
        points.push_back(ControlPoint{ glm::vec3(0.99f, 0.2f, 0.f), glm::vec3(0.f, 1.f, 0.f), 6, 0.99f });
        points.push_back(ControlPoint{ glm::vec3(1.f, 0.2f, 0.f), glm::vec3(0.f, 1.f, 0.f), 7, 1.f });
        strands.push_back(Strand{ points });

        fiber.addStrands(strands);
        fiber.loadPoints();
        break;
    case TEST3:
        // Basic yarn along the Y-axis
        // STATUS: passed
        points.push_back(ControlPoint{ glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), 0, 0.f });
        points.push_back(ControlPoint{ glm::vec3(0.f, 0.01f, 0.f), glm::vec3(1.f, 0.f, 0.f), 1, 0.01f });
        points.push_back(ControlPoint{ glm::vec3(0.f, 0.99f, 0.f), glm::vec3(1.f, 0.f, 0.f), 3, 0.99f });
        points.push_back(ControlPoint{ glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, 0.f, 0.f), 4, 1.f });
        strands.push_back(Strand{ points });
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