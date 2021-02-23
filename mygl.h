#pragma once
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Shader.h"

// TODO: Compile time errors from static camera and methods being passed to GLFW methods

class MyGL
{
public:
	MyGL();
	~MyGL();

	int initializeGL();
	void initializeImGuiContext();
	void resizeGl(int width, int height);
	void paintSimulation();
	void paintRender();

	void processInput();
	static void updateTime();
	void swapAndPoll();

	GLFWwindow* getWindow() const;
	Shader* getCoreShader();
	Shader* getFiberShader();

	static Camera& getCamera();
	 
private:
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	static Camera m_camera;
	Shader m_coreShader;
	Shader m_fiberShader;

	GLFWwindow* m_window;

	static float lastX;
	static float lastY;

	static float deltaTime; // Time between current frame and last frame
	static float lastFrame; // Time of last frame
	static bool firstMouse; 
	static bool moveCamera; 
};