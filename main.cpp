#ifdef SIMULATION
//GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//Dear ImGUI
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
//#include "imgui/implot/implot.h"
//C++ Libs
#include <stdio.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <execution>
#include <memory>
//Own Code
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "SphericalCamera.h"
#include "Fabric.h"

#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void onGLError(int error, const char* description)
{
	std::cout << "OGLE: " << description << std::endl;
	fprintf(stderr, "OpenGL Error: %s\n", description);
}

void copyShaderContents(std::string vname, std::string fname, const char** vert, const char** frag)
{
	std::ifstream file1, file2;
	file1.open(vname);
	file2.open(fname);
	std::stringstream strStream1, strStream2;
	strStream1 << file1.rdbuf();
	strStream2 << file2.rdbuf();
	*vert = strStream1.str().c_str();
	*frag = strStream2.str().c_str();
	file1.close();
	file2.close();
}

void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam)
{
	std::cout << "OGLE " << source << " " << type << " " << message << std::endl;
}

GLuint loadTexture(std::string filename)
{
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);

	return texture;
}

void simulate(Collidable& c, Fabric& f, double d);
void mouseClicked(GLFWwindow* window, int button, int action, int mods);
void mouseMoved(GLFWwindow* window, double xpos, double ypos);
void mouseScrolled(GLFWwindow* window, double xoffset, double yoffset);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Sim settings
float k = 0.f, kd = 0.5f; // Spring constant, Damping constant
float shearCoeff = 0.5f; // Shear constant
float muK = 0.f, muS = 0.f; // Friction coefficients
bool paused = false;
// Camera
SphericalCamera scam, debugCam;

// Colliding objects
Cube cube(glm::vec3(0, 0, 0), glm::vec3(1.f, 1, 1), .785f, glm::vec3(1, 0, 1));
Cube floorC(glm::vec3(0, -.005f, 0), glm::vec3(10, .005f, 10), 0, glm::vec3(1, 0, 0));
TruncatedPyramid pyr(glm::vec3(0, 0, 0), glm::vec2(1, 1), 3, 2);

// Fabric
bool reset_fabric = false;
Fabric* fabric = nullptr;// = //new SquareFabric(glm::vec3(0, 2, 0), glm::vec2(1, 1), glm::vec2(5, 5), .5f);
//new Fabric(glm::vec3(-.5f, 2, 0), glm::vec3(.5f, 2, 0), 1, .5f);//
Fabric* fab2 = nullptr;

// Debug fabric forces
Axis3D axis(glm::vec3(0, 0, 0), 2);
glm::vec3 fabric_point;
glm::vec3 Fg_d;
glm::vec3 Fn_d;
glm::vec3 Ff_d;
glm::vec3 Ft_d;
glm::vec3 Fnt_d;

float fs, fd;
float totalEnergy, currKE, currUg, currUs;


int main()
{
	//printf("%f|%f|%f\n%f|%f|%f\n%f|%f|%f\n", m[0][0], m[1][0], m[2][0], m[0][1], m[1][1], m[2][1], m[0][2], m[1][2], m[2][2]);

	std::vector<glm::vec2> poly; poly.push_back(glm::vec2(0, .5)); poly.push_back(glm::vec2(2, -.5)); poly.push_back(glm::vec2(1.5, 1));
	//fabric = new PolyFabric("A", glm::vec3(-.5f, 2.1f, -.5f), poly, glm::vec2(15, 15), .5f);
	fabric = new SquareFabric("square", glm::vec3(-.5f, 2.1f, -.5f), glm::vec2(2, 2), glm::vec2(20, 20), .5f);
	fab2 = new SquareFabric("square2", glm::vec3(2.f, 2.1f, 2.f), glm::vec2(2, 2), glm::vec2(20, 20), .5f);


	// Initialize OpenGL
	if (!glfwInit())
	{
		// Initialization failed
		return 1;
	}
	glfwSetErrorCallback(onGLError);

	// Set required OpenGL version (Todo)

	// Set up window and GL context
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	GLFWwindow* window = glfwCreateWindow(1200, 800, "Cloth Modelling Test", NULL, NULL);
	if (!window)
	{
		// Window creation failed
		glfwTerminate();
		return 2;
	}

	// Use context
	glfwMakeContextCurrent(window);
	gladLoadGL();

	// OpenGL Settings
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(debugMessage, NULL);

	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

	GLuint vao;
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glPointSize(5);
	glClearColor(0.9, 0.9, 0.9, 1);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Set up Dear ImGUI settings
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImPlot::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsLight();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	ImVec4 debugClearCol(1.f, 1.f, 1.f, 1.00f);

	// Create GLFW input callbacks
	glfwSetMouseButtonCallback(window, mouseClicked);
	glfwSetCursorPosCallback(window, mouseMoved);
	glfwSetScrollCallback(window, mouseScrolled);
	glfwSetKeyCallback(window, keyCallback);

	// Load files and create basic shader
	const char* sv1, * sf1, * sv2, * sf2, * sv_vec, * sf_vec;
	copyShaderContents("glsl/vert.glsl", "glsl/frag.glsl", &sv1, &sf1);
	copyShaderContents("glsl/bland.vert.glsl", "glsl/bland.frag.glsl", &sv2, &sf2);
	copyShaderContents("glsl/vec.vert.glsl", "glsl/vec.frag.glsl", &sv_vec, &sf_vec);
	//Shader lambert_prog(sv1, sf1);
	//Shader vec_prog(sv_vec, sf_vec);
	Shader lambert_prog("glsl/vert.glsl", "glsl/frag.glsl");
	Shader vec_prog("glsl/vec.vert.glsl", "glsl/vec.frag.glsl");
	Shader debugColor_prog("glsl/color.debug.vert.glsl", "glsl/color.debug.frag.glsl");

	// Texture creation
	GLuint texture = loadTexture("test_tex_atlas.jpg");
	GLuint fabTex = loadTexture("test_tex.jpg");

	//Camera cam(800, 800, glm::vec3(2, 10, 10), glm::vec3(0,0,0), glm::vec3(0, 1, 0));
	debugCam = SphericalCamera(400, 400, glm::vec4(0, 0, 4, 1), glm::vec4(0, 0, 0, 1), glm::vec4(0, 1, 0, 0));
	scam = SphericalCamera(800, 800, glm::vec4(0, 0, 10, 1), glm::vec4(0, 0, 0, 1), glm::vec4(0, 1, 0, 0));

	// Create constant drawables
	cube.create();
	floorC.create();
	pyr.create();
	axis.create();

	// Fill in constants
	//totalEnergy = 9.81f * fabric->mass * fabric->position.y;

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	int steppedBeforeRender = 0, toStepBeforeRender = 500;

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		//TODO remove
		bool connect = false;


		// Should simulation be stepped once even if paused?
		bool step = false;

		/*
		 * Dear ImGUI Debugging windows
		 */
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Simulation Settings");
			connect = ImGui::Button("Connect");
			ImGui::Checkbox("Pause", &paused);      // Edit bools storing our window open/close state
			ImGui::SliderFloat("Spring Coeff", &k, 0.0f, 500.0f);
			ImGui::SliderFloat("Damping Coeff", &kd, 0.0f, 2.0f);
			ImGui::SliderFloat("Shearing", &shearCoeff, 0.0f, 20.0f);
			ImGui::SliderFloat("mu S", &muS, 0.0f, 2.0f);
			ImGui::SliderFloat("mu K", &muK, 0.0f, 2.0f);
			ImGui::NewLine();
			ImGui::InputFloat("X", &fabric->position.x, 0.01f, .5f, "%.3f");
			ImGui::InputFloat("Y", &fabric->position.y, 0.01f, .5f, "%.3f");
			ImGui::InputFloat("Z", &fabric->position.z, 0.01f, .5f, "%.3f");
			step = ImGui::Button("Step");
			ImGui::Text("Fs: %f, Fd: %f", fs, fd);
			ImGui::Text("Ei_pot: %f, Ei: %f, Ecurr: %f", 9.81f * fabric->mass * fabric->position.y, totalEnergy, currKE + currUg + currUs);
			/*if (ImPlot::BeginPlot("Energy")) {
				ImPlot::PlotLine
				ImPlot::EndPlot();
			}*/
			ImGui::End();
		}


		// Create Fabric if it needs to be reset
		if (reset_fabric)
		{
			reset_fabric = false;
			fabric->reset(glm::vec3(-.4f, 2.1f, -.8f));
			//fabric->get(glm::vec2(0, 0))->attr[0] = glm::vec3(-.5f, 0, /*-.1f*/0);
			//fabric->get(glm::vec2(1, 0))->attr[0] = glm::vec3(.5f, 0, /*.1f*/0);
		}


		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		double delta = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
		delta = 1.f / 100.f;
		begin = end;

		if (!paused) {
			/*for (int i = 0; i < 600; i++) {
				simulate(pyr, *fabric, delta * .01);
			}*/
			simulate(pyr, *fabric, delta * .01);
			steppedBeforeRender++;
		}
		if (step) {
			for (int i = 0; i < 100000; i++) {
				simulate(pyr, *fabric, delta * .01);
			}
			std::cout << "Stepped!" << std::endl;
		}

		if (connect && fab2 != nullptr) {
			fabric->connect(fab2, 3, 2);
		}
		//TODO connect fabrics in middle of sim

		/* Rendering */
		// Simulate a couple times first without skipping input
		if (steppedBeforeRender < toStepBeforeRender) {
			steppedBeforeRender = 0;

			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			glViewport(0, 0, 800, height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Use normal camera for current drawing shader
			scam.ref = glm::vec4(fabric->position, 1);
			scam.RecomputeAttributes();
			glm::mat4 viewProj = scam.getViewProj();
			lambert_prog.setViewProjMatrix(viewProj);
			lambert_prog.setModelMatrix(glm::mat4(1));
			lambert_prog.setCameraPosition(scam.eye);
			// Use debug camera for vector shader
			glm::mat4 debugViewProj = debugCam.getViewProj();
			vec_prog.setViewProjMatrix(debugViewProj);
			vec_prog.setModelMatrix(glm::mat4(1));
			vec_prog.setCameraPosition(debugCam.eye);
			// Debug color shader
			debugColor_prog.setViewProjMatrix(viewProj);
			debugColor_prog.setModelMatrix(glm::mat4(1));
			debugColor_prog.setCameraPosition(scam.eye);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			lambert_prog.draw(&floorC);
			//lambert_prog.draw(cube);//, texture);
			lambert_prog.draw(&pyr);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fabTex);
			fabric->destroy();
			fabric->create();
			debugColor_prog.draw(fabric);//, fabTex);
			if (fab2 != nullptr) {
				fab2->destroy();
				fab2->create();
				debugColor_prog.draw(fab2);
			}

			//// Debug forces in different panel
			glViewport(800, 0, 400, 400);

			/*debugColor_prog.draw(fabric);
			if (fab2 != nullptr) {
				debugColor_prog.draw(fab2);
			}*/

			fabric_point = glm::vec3(0.5f, 0.5f, 0.5f);
			Vector gravV(fabric_point, fabric_point + Fg_d);
			Vector normV(fabric_point, fabric_point + Fn_d);
			Vector fricV(fabric_point, fabric_point + Ff_d);
			Vector netfV(fabric_point, fabric_point + Ft_d);
			Vector nettV(fabric_point, fabric_point + Fnt_d);
			gravV.create();
			normV.create();
			fricV.create();
			netfV.create();
			nettV.create();
			vec_prog.draw(&axis);
			vec_prog.draw(&normV);
			vec_prog.draw(&gravV);
			vec_prog.draw(&fricV);
			vec_prog.draw(&nettV);
			gravV.destroy();
			normV.destroy();
			fricV.destroy();
			netfV.destroy();
			nettV.destroy();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}
	// Finish
	cube.destroy();
	floorC.destroy();
	axis.destroy();
	pyr.destroy();
	fabric->destroy();
	delete fabric;
	glDeleteVertexArrays(1, &vao);
	//ImGui Cleanup
	//ImPlot::DestroyContext();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

float totalTime = 0;

void simulate(Collidable& collidable, Fabric& fab, double delta)
{
	totalTime += delta;
	float dm = fab.mass / fab.size();

	//Reset current energy total
	currKE = 0;
	currUg = 0;
	currUs = 0;

	bool te0 = false;
	if (totalEnergy == 0) te0 = true;

	std::for_each(std::execution::par_unseq, fab.begin(), fab.end(), [&](auto curr)
		//for (auto curr : fab)
		{
			// Query vertex currently considered
			if (curr == nullptr) return;

			// Add current energies
			if (te0) {
				float speed = glm::length(curr->velocity);
				totalEnergy += .5f * dm * speed * speed;
				totalEnergy += dm * 9.81 * (fab.position.y + curr->localPos.y);
				if (curr->posWarp != nullptr) {
					float dst_diff = glm::distance(curr->localPos, curr->posWarp->localPos) - curr->edgeLength.x;
					totalEnergy += .5f * k * dst_diff * dst_diff;
				}
				if (curr->posWeft != nullptr) {
					float dst_diff = glm::distance(curr->localPos, curr->posWeft->localPos) - curr->edgeLength.y;
					totalEnergy += .5f * k * dst_diff * dst_diff;
				}
			}
			float speed = glm::length(curr->velocity);
			currKE += .5f * dm * speed * speed;
			currUg += dm * 9.81 * (fab.position.y + curr->localPos.y);
			if (curr->posWarp != nullptr) {
				float dst_diff = glm::distance(curr->localPos, curr->posWarp->localPos) - curr->edgeLength.x;
				currUs += .5f * k * dst_diff * dst_diff;
			}
			if (curr->posWeft != nullptr) {
				float dst_diff = glm::distance(curr->localPos, curr->posWeft->localPos) - curr->edgeLength.y;
				currUs += .5f * k * dst_diff * dst_diff;
			}
			// TODO include stitches in total energy calculation
			// TODO stop simulation when energy converges


			/*
				* Calculate forces
				*
				* Net force composed of:
					(1) Gravity,
					(2) Normal force of colliding surface,
					(3) Friction forces with colliding surface (What about self-folded cloth?),
					(4) Tension forces (For now)
				*/
			glm::vec3 totalForce(0, 0, 0);

			// Resolve potential collision at vertex
			// TODO: Have it wrapped in an object. 
			//		 and do proper collision checking. Maybe use "curves", or a math function that maps out the surface?
			glm::vec3 currPos = fab.position + curr->localPos;
			glm::vec3 normalUnitDir(0, 0, 0);
			if (collidable.isInBounds(currPos))
			{
				normalUnitDir = collidable.nearestNormal(currPos);
			}

			if (currPos.y <= -1) {
				normalUnitDir = glm::vec3(0, 1, 0);
			}

			// (1) Gravity
			// F = mg, g = 9.81m/s2, dm (infenitesimal mass) = mass of fabric / numVertices
			glm::vec3 F1(0, -9.81 * dm, 0);

			// (4) Tension forces
			// Can model the yarn in each direction as a spring, maybe?
			// F = kx, where k is some constant and x the displacement from rest. 
			glm::vec3 F4_NWa(0, 0, 0), F4_PWa(0, 0, 0), F4_NWe(0, 0, 0), F4_PWe(0, 0, 0), F4_5th(0, 0, 0);
			//float k = 0.f; // How do I determine this...

			// Tension force calculating function
			auto tension = [=](FabricVertex* neighbor, float defaultEdgeLength, bool save = false) {
				if (neighbor == nullptr)
					return glm::vec3(0, 0, 0);
				// Calculate distance between two vertices (intersections)
				float dst = glm::distance(curr->localPos, neighbor->localPos);
				// Calculate how much the "spring" got stretched from its resting position
				float dst_diff = dst - defaultEdgeLength; // TODO: right now its the same but ned to check if its actually x or y
				// Calculate direction of force
				glm::vec3 dir = glm::normalize(neighbor->localPos - curr->localPos);
				// Calculate velocities in direction of tension of both mass points
				float vDiff = glm::abs(glm::dot(/*neighbor->attr[0] - */curr->velocity, dir));
				// Calculate Hooke's Spring + Damping force


				//US[j + fab.amount.x * i] = .5f * k * dst_diff * dst_diff;


				return dir * (k * dst_diff /*- kd * vDiff*/);
			};
			// TODO: implement the other direction everywhere properly
			F4_NWa = tension(curr->negWarp, curr->edgeLength.x);
			F4_PWa = tension(curr->posWarp, curr->edgeLength.x);
			F4_NWe = tension(curr->negWeft, curr->edgeLength.y);
			F4_PWe = tension(curr->posWeft, curr->edgeLength.y);
			F4_5th = tension(curr->other, 0.01); // TODO save actual stitch length

			//^^ did the edges get calculated properly? I dont want the edges to get "pulled" in because the tension force on them is not equalized

			// (2) Normal force of colliding surface
			// Requires resolving potential collision with object on a vertex-to-vertex basis.
			// Need gradient of surface at point of collision.
			// Normal to gradient is Fn direction, magnitude determined by dot product with F1. (Need to break F1 into components)
			// ADDED: Not only dotted with gravity, but all forces so far, so normal force also compensates tension.
			// Unit normal vector dot force * -unitnormalvec = Fn
			glm::vec3 Fnormalless = F1 + F4_NWa + F4_PWa + F4_NWe + F4_PWe + F4_5th;
			float normalMag = std::abs(glm::dot(Fnormalless, normalUnitDir)); // Is the use of abs correct here?
			glm::vec3 F2 = normalMag * normalUnitDir;

			// Calculate Net Force without friction
			glm::vec3 netF = Fnormalless + F2;

			netF += -curr->velocity * kd;

			// (3) Friction forces with surface
			// Literally the direction of gradient of surface at point of collision. Magnitude = muk * |F2|
			// Not sure how to interpret gradient when flat?
			// How to get direction...
			// 
			glm::vec3 F3(0, 0, 0);
			float netMag = glm::length(netF);
			// Normal force needs to exist (otherwise implies no collision) and net force can't be 0 (division by 0)
			if (F2 != glm::vec3(0, 0, 0) && netMag != 0) {
				glm::vec3 unitA = netF / netMag;
				// Friction will be in opposite direction of unitA,
				// If netMag > muS Fn, there is movement and friction force is muK Fn, otherwise friction force is -netMag
				if (netMag > muS * normalMag)
				{
					F3 = muK * normalMag * -unitA;
				}
				else
				{
					F3 = netMag * -unitA;
				}
			}
			// ^^ should only exist when colliding.

			// Add friction forces to current net force
			netF += F3;


			/*
			Angle attempts:
			*/
			// If dot is negative, angle > pi/2, if positive, angle < pi/2.
				// All I want is dot to be negative to be in [90, -90] range
			// TODO include stitch direction
			if (curr->negWeft != nullptr && curr->posWeft != nullptr)
			{
				glm::vec3 nWeV = curr->negWeft->localPos - curr->localPos;
				glm::vec3 pWeV = curr->posWeft->localPos - curr->localPos;
				glm::vec3 weftCross = glm::cross(nWeV, pWeV);
				float weftCross_len = glm::length(weftCross);
				float weftDot = glm::dot(nWeV, pWeV);
				// Truth of this condition implies division by 0, and the absence of shear forces
				float nWeV_len = glm::length(nWeV);
				float pWeV_len = glm::length(pWeV);
				glm::vec3 nWeV_unit = glm::normalize(nWeV);
				glm::vec3 pWeV_unit = glm::normalize(pWeV);
				glm::vec3 cWeV = nWeV_unit + pWeV_unit;
				float cWeV_len = glm::length(cWeV);
				bool cond = nWeV_len == 0 || pWeV_len == 0 || cWeV_len == 0;
				if (!cond)
				{
					float cos = weftDot / (nWeV_len * pWeV_len);
					float sin = weftCross_len / (nWeV_len * pWeV_len);
					netF += glm::normalize(cWeV) * glm::abs(sin) * shearCoeff;
				}
			}
			if (curr->negWarp != nullptr && curr->posWarp != nullptr)
			{
				glm::vec3 nWaV = curr->negWarp->localPos - curr->localPos;
				glm::vec3 pWaV = curr->posWarp->localPos - curr->localPos;
				glm::vec3 warpCross = glm::cross(nWaV, pWaV);
				float warpCross_len = glm::length(warpCross);
				float warpDot = glm::dot(nWaV, pWaV);
				// Truth of this condition implies division by 0, and the absence of shear forces
				float nWaV_len = glm::length(nWaV);
				float pWaV_len = glm::length(pWaV);
				glm::vec3 nWaV_unit = glm::normalize(nWaV);
				glm::vec3 pWaV_unit = glm::normalize(pWaV);
				glm::vec3 cWaV = nWaV_unit + pWaV_unit;
				float cWaV_len = glm::length(cWaV);
				bool cond = nWaV_len == 0 || pWaV_len == 0 || cWaV_len == 0;
				if (!cond)
				{
					float cos = warpDot / (nWaV_len * pWaV_len);
					float sin = warpCross_len / (nWaV_len * pWaV_len);
					netF += glm::normalize(cWaV) * glm::abs(sin) * shearCoeff;
				}
			}


			if (normalUnitDir != glm::vec3(0, 0, 0)) {
				// If object is colliding, cancel velocity in colinear direction to normal.
				curr->velocity += glm::abs(glm::dot(curr->velocity, normalUnitDir)) * normalUnitDir;
				curr->attr[1] += glm::abs(glm::dot(curr->attr[1], normalUnitDir)) * normalUnitDir;
				//if (!curr->at_rest) {
				//	curr->at_rest = true;
				//	// Momentum:
				//	float dMomentum = glm::length(curr->attr[1]) * dm;
				//	glm::vec3 Fcollision = normalUnitDir * dMomentum /* /(float)delta*/;

				//	// Add the force from impulse to model collision
				//	netF += Fcollision;
				//}
			}

			// F = ma -> a = F/m
			glm::vec3 acceleration = netF / dm;

			//if(totalTime < 2)
			//	std::cout << acceleration.y << " - " << delta << std::endl;



			float df = (float)delta;
			// TEMP: Assign 0 velocity if not defined yet


			/*VERLET ORIGINAL*/
			//if (curr->attr.size() == 0) {
			//	// First pass: Standard integration, saving old coords
			//	curr->attr.push_back(curr->localPos);
			//	glm::vec3 x_delta = acceleration * (df * df / 2.f + ((float)totalTime - df) * df);
			//	curr->tmpPos = curr->localPos + x_delta;
			//}
			//else {
			//	curr->tmpPos = 2.f * curr->localPos - curr->attr[0] + acceleration * df * df;
			//	curr->attr[0] = curr->localPos;
			//}

			/*VERLET VELOCITY*/
			if (curr->attr.size() == 0) {
				curr->attr.push_back(glm::vec3(0, 0, 0)); //Velocity
				curr->attr.push_back(glm::vec3(0, 0, 0)); //Old acc
			}
			/*
			curr->tmpPos = curr->localPos + curr->attr.at(0) * df + .5f * curr->attr[1] * df * df;
			curr->attr[0] = curr->attr[0] + .5f * (acceleration + curr->attr[1]) * df;
			curr->attr[1] = acceleration;*/
			//float df = (float)delta;
			curr->tmpPos = curr->localPos + curr->velocity * df + .5f * curr->attr[1] * df * df;
			curr->velocity = curr->velocity + .5f * (acceleration + curr->attr[1]) * df;
			curr->attr[1] = acceleration;

			/*STANDARD INTEGRATION*/
			// x(t) = x0 + v0t + .5at^2
			// x(t+delta) = x0 + v0t + v0delta + .5a(t^2 + 2tdelta + delta^2) = x(t) + delta^2/2 * a + deltata
			//glm::vec3 x_delta = acceleration * (float)delta * ((float)delta / 2.f + (float)(totalTime - delta));
				//(float)(delta * delta) * acceleration / 2.f + delta * (totalTime - delta) * acceleration;
			//glm::vec3 x_delta = acceleration * (df * df / 2.f + ((float)totalTime - df) * df);
			//curr->tmpPos = curr->localPos + x_delta;

		//}
		});
	// Finish iteration
	fab.applyTemporaryPositionChanges();
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
		if (left_mouse_last_drag.x <= 800)
		{
			targetCam = &scam;
		}
		// Debug panel drag
		if (left_mouse_last_drag.x > 800 && left_mouse_last_drag.y > 400)
		{
			targetCam = &debugCam;
		}
		// Camera has been selected, rotate it
		if (targetCam != nullptr)
		{
			dragged /= glm::vec2(800, 800);
			targetCam->RotateYAboutPoint(dragged.x * 5);
			targetCam->RotateXAboutPoint(dragged.y * 5);
		}
	}
}
void mouseScrolled(GLFWwindow* window, double xoffset, double yoffset)
{
	SphericalCamera* targetCam = nullptr;
	// Main panel
	if (curr_mouse_pos.x <= 800)
	{
		targetCam = &scam;
	}
	// Debug panel
	if (curr_mouse_pos.x > 800 && curr_mouse_pos.y > 400)
	{
		targetCam = &debugCam;
	}
	// Zoom
	if (targetCam != nullptr)
	{
		targetCam->ZoomToPoint(-yoffset);
	}
}

bool wireframe = false;
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_R && action == GLFW_RELEASE)
	{
		reset_fabric = true;
		totalTime = 0;
	}
	if (key == GLFW_KEY_P && action == GLFW_RELEASE)
	{
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
		wireframe = !wireframe;
	}
}
#else
//#define CORE_RENDER
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
Camera camera;
SphericalCamera scam;

// glfw callbacks
// --------------
void mouseClicked(GLFWwindow* window, int button, int action, int mods);
void mouseMoved(GLFWwindow* window, double xpos, double ypos);
void mouseScrolled(GLFWwindow* window, double xoffset, double yoffset);

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
    camera = Camera(glm::vec3(0.0f, 0.f, 2.f));
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
        if (moveCamera)
			glfwSetCursorPos(window, lastX, lastY);
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
#endif