#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Fiber.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

Camera camera(glm::vec3(0.0f, 0.0f, 0.5f));
float lastX = Fiber::SCR_WIDTH / 2.0f;
float lastY = Fiber::SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

std::vector<glm::vec3> pointsToAdd;

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
    GLFWwindow* window = glfwCreateWindow(Fiber::SCR_WIDTH, Fiber::SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

    // Fiber
    // -----
    Fiber fiber = Fiber();
    fiber.initShaders();
    fiber.initFrameBuffer();
    std::cout << "Finished fiber initialization" << std::endl;
    // add yarn control points
    if (fiber.getRenderType() == CORE)
    {
        pointsToAdd.push_back(glm::vec3(0.0f, 0.f, 0.f));
        pointsToAdd.push_back(glm::vec3(0.01f, 0.f, 0.f));
        pointsToAdd.push_back(glm::vec3(0.356f / 2.f, 0.f, 0.f));
        pointsToAdd.push_back(glm::vec3(0.366114f / 2.f, 0.f, 0.f));
    }
    if (fiber.getRenderType() == FIBER || fiber.getRenderType() == COMPLETE)
    {
        pointsToAdd.push_back(glm::vec3(0.0f, 0.f, 0.f));
        pointsToAdd.push_back(glm::vec3(0.01f, 0.f, 0.f));
        pointsToAdd.push_back(glm::vec3(0.99f, 0.f, 0.f));
        pointsToAdd.push_back(glm::vec3(1.f, 0.f, 0.f));
    }

    std::cout << "Finished adding control points" << std::endl;

    for (glm::vec3 point : pointsToAdd) {
        fiber.addPoint(point[0], point[1], point[2]);
    }

    std::cout << "Rendering" << std::endl;
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
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
                               (float) Fiber::SCR_WIDTH / (float) Fiber::SCR_HEIGHT, 0.01f, 10.0f);

        if (fiber.getRenderType() != COMPLETE)
        {
            const Shader& shader = fiber.getActiveShader();
            shader.use();

            shader.setMat4("model", model);
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);
            shader.setVec3("camera_pos", camera.Position);
            shader.setVec3("view_dir", camera.Front);
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
        glfwSetWindowShouldClose(window, true);

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