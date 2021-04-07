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
#include "DepthShader.h"
#include "Camera.h"
#include "SphericalCamera.h"
#include "Fiber.h"
#include "CoreFiber.h"
#include "OrdinaryFiber.h"
#include "Pattern.h"
#include "Light.h"

// global variables
// ----------------
float deltaTime = 0.f; // Time between current frame and last frame
float lastFrame; // Time of last frame
float lastX;
float lastY;
bool firstMouse;
bool moveCamera;
GLFWwindow* window;
SphericalCamera scam;

// glfw callbacks
// --------------
void mouseClicked(GLFWwindow* window, int button, int action, int mods);
void mouseMoved(GLFWwindow* window, double xpos, double ypos);
void mouseScrolled(GLFWwindow* window, double xoffset, double yoffset);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
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
	glfwSetMouseButtonCallback(window, mouseClicked);
	glfwSetCursorPosCallback(window, mouseMoved);
	glfwSetScrollCallback(window, mouseScrolled);
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
    scam = SphericalCamera(
        1200, 
        1200, 
        glm::vec4(0, 0, 1, 1), 
        glm::vec4(0, 0, 0, 1), 
        glm::vec4(0, 1, 0, 0)
    ); // It's a scam!
    scam.near_clip = 0.001f;
//    scam.zoom = 1.f;


    // Initialize Shaders
    // ------------------
    FiberShader coreShader = FiberShader("fiber_vertex.glsl", "core_fragment.glsl", "core_geometry.glsl", "core_tess_control.glsl", "core_tess_eval.glsl");
    FiberShader fiberShader = FiberShader("fiber_vertex.glsl", "fiber_fragment.glsl", "fiber_geometry.glsl", "fiber_tess_control.glsl", "fiber_tess_eval.glsl");
	DepthShader depthShader = DepthShader("fiber_vertex.glsl", "depth_fragment.glsl", "depth_geometry.glsl", "fiber_tess_control.glsl", "depth_tess_eval.glsl");
	Shader lightCubeShader = Shader("lightcube_vertex.glsl", "lightcube_fragment.glsl");

	// Initialize Light
	// ----------------
	glm::vec3 lightPos(1.0f, 2.0f, 2.0f);
	Light light(lightPos);
	light.create();

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
	// TODO: change to ordinaryFiber.create(std::vector<Fabric*> strands)
	// NOTE: please see createTest() for current density examples. 

    // Set the textures of CoreFiber into OrdinaryFiber
    ordinaryFiber.setHeightTexture(coreFiber.getHeightTexture());
    ordinaryFiber.setNormalTexture(coreFiber.getNormalTexture());
    ordinaryFiber.setAlphaTexture(coreFiber.getAlphaTexture());

#ifdef CORE_RENDER
    glfwSetWindowSize(window, 800, 800);
#else
	glfwSetWindowSize(window, 2400, 2400);
#endif

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
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
        glm::mat4 view = scam.getView();
        glm::mat4 projection = scam.getProj();

		// Light
		// -----
		// get light space transform
		float near_plane = 1.0f, far_plane = 7.5f; // TODO: change values
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); // TODO: change values
		glm::mat4 lightView = glm::lookAt(lightPos,
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)); // TODO: change values
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;
		// update uniform variables
		lightCubeShader.use();
		glm::mat4 lightModel = glm::mat4(1.0f);
		lightModel = glm::translate(lightModel, lightPos);
		lightModel = glm::scale(lightModel, glm::vec3(0.2f));
		lightCubeShader.setMat4("model", lightModel);
		lightCubeShader.setMat4("view", view);
		lightCubeShader.setMat4("projection", projection);
		
        // Update uniform variables defining the camera properties
        // -------------------------------------------------------
        coreShader.use();
        coreShader.setMat4("model", model);
        coreShader.setMat4("view", view);
        coreShader.setMat4("projection", projection);
        coreShader.setVec3("camera_pos", scam.eye);
        coreShader.setVec3("view_dir", scam.look);

        fiberShader.use();
        fiberShader.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.f)));
        fiberShader.setMat4("view", view);
        fiberShader.setMat4("projection", projection);
        fiberShader.setVec3("camera_pos", scam.eye);
        fiberShader.setVec3("view_dir", scam.look);
		fiberShader.setVec3("light_pos", lightPos);
		fiberShader.setMat4("light_transform", lightSpaceMatrix);

		depthShader.use();
		depthShader.setMat4("light_transform", lightSpaceMatrix);
		depthShader.setVec3("light_pos", lightPos);

        // render Fiber
        // ------------
        coreShader.draw(&coreFiber, -1);
#ifndef CORE_RENDER
		depthShader.draw(&ordinaryFiber, -1);
        fiberShader.draw(&ordinaryFiber, -1);
#endif

		// render Light
		// ------------
		lightCubeShader.draw(&light, -1);

		// render ImGui
		// ------------
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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
    {
        moveCamera = !moveCamera;
        if (moveCamera)
			glfwSetCursorPos(window, lastX, lastY);
    }
}

void processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
}

// Control camera by mouse click
glm::vec2 left_mouse_last_drag;
glm::vec2 curr_mouse_pos;
bool dragging = false;
void mouseClicked(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &xpos, &ypos);
        dragging = true;
        left_mouse_last_drag = glm::vec2(xpos, ypos);
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        glfwGetCursorPos(window, &xpos, &ypos);
        dragging = false;
    }
}
void mouseMoved(GLFWwindow* window, double xpos, double ypos)
{
    curr_mouse_pos = glm::vec2(xpos, ypos);
    if (ImGui::GetIO().WantCaptureMouse) return;
    if (dragging)
    {
        glm::vec2 curr(xpos, ypos);
        glm::vec2 dragged = left_mouse_last_drag - curr;
        left_mouse_last_drag = curr;
        SphericalCamera* targetCam = nullptr;
        // Main panel drag
        if (left_mouse_last_drag.x <= 2400)
        {
            targetCam = &scam;
        }

        // Camera has been selected, rotate it
        if (targetCam != nullptr)
        {
            dragged /= glm::vec2(2400, 2400);
            targetCam->RotateYAboutPoint(dragged.x * 5);
            targetCam->RotateXAboutPoint(dragged.y * 5);
        }
    }
}
void mouseScrolled(GLFWwindow* window, double xoffset, double yoffset)
{
    SphericalCamera* targetCam = nullptr;
    // Main panel
    if (curr_mouse_pos.x <= 2400)
    {
        targetCam = &scam;
    }
    // Zoom
    if (targetCam != nullptr)
    {
        targetCam->ZoomToPoint(-yoffset / 2.f);
    }
}