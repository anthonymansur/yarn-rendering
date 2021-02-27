#include "mygl.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

// initializing static member variables
// ------------------------------------


MyGL::MyGL() :
	m_camera(Camera(glm::vec3(0.0f, 0.f, 2.f))),
	m_window(nullptr)
{
}

MyGL::~MyGL()
{

}

int MyGL::initializeGL()
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
	m_window = glfwCreateWindow(800, 600, "Fiber-Level Cloth Rendering", NULL, NULL);
	if (m_window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* INSERT CALLBACKS HERE */

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Initialize Shaders
	// ------------------
	m_coreShader = Shader("fiber_vertex.glsl", "core_fragment.glsl", "core_geometry.glsl", "core_tess_control.glsl", "core_tess_eval.glsl");
	m_fiberShader = Shader("fiber_vertex.glsl", "fiber_fragment.glsl", "fiber_geometry.glsl", "fiber_tess_control.glsl", "fiber_tess_eval.glsl");

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE); // for antialiasing
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode
}

void MyGL::initializeImGuiContext()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 410");
	io.Fonts->AddFontDefault();
	io.FontGlobalScale = 1.25f;
}

void MyGL::resizeGl(int w, int h)
{
	glfwSetWindowSize(m_window, w, h);
}

void MyGL::paintSimulation()
{
	// TODO: implement
}

void MyGL::paintRender()
{
	// TODO: implement
}

Shader* MyGL::getCoreShader()
{
	return &m_coreShader;
}

Shader* MyGL::getFiberShader()
{
	return &m_fiberShader;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void MyGL::processInput()
{
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, 1);
	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
		m_camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
		m_camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
		m_camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
		m_camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS)
		m_camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS)
		m_camera.ProcessKeyboard(UP, deltaTime);
}

// per-frame time logic
// --------------------
void MyGL::updateTime()
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}
// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
// -------------------------------------------------------------------------------
void MyGL::swapAndPoll()
{
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

GLFWwindow* MyGL::getWindow() const
{
	return m_window;
}

Camera& MyGL::getCamera()
{
	return m_camera;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------

void MyGL::framebuffer_size_callback(int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


void MyGL::mouse_callback(double xpos, double ypos)
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

	m_camera.ProcessMouseMovement(xoffset, yoffset);
}


void MyGL::scroll_callback(double xoffset, double yoffset)
{
	m_camera.ProcessMouseScroll(yoffset);
}

void MyGL::key_callback(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
	{
		moveCamera = !moveCamera;
		if (!moveCamera)
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		else
		{
			glfwSetCursorPos(m_window, lastX, lastY);
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}
}