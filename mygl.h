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
	void updateTime();
	void swapAndPoll();

	GLFWwindow* getWindow() const;

	Shader* getCoreShader();
	Shader* getFiberShader();

	Camera& getCamera();

	void framebuffer_size_callback(int width, int height);
	void mouse_callback(double xpos, double ypos);
	void scroll_callback(double xoffset, double yoffset);
	void key_callback(int key, int scancode, int action, int mods);
	 
private:
	Camera m_camera;

	Shader m_coreShader;
	Shader m_fiberShader;

	GLFWwindow* m_window;
	float deltaTime = 0.f; // Time between current frame and last frame
	float lastFrame; // Time of last frame
	float lastX;
	float lastY;
	bool firstMouse;
	bool moveCamera;
};
