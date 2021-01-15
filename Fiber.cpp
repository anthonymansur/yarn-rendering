#include "Fiber.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

namespace
{
	const GLuint POS_VAO_ID = 0;
	const GLuint NORM_VAO_ID = 1;
	const GLuint STRIDE = 6;
}

unsigned int loadTexture(const char* path);
static std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems);
static std::vector<std::string> split(const std::string& s, char delim);

Fiber::Fiber(FIBER_TYPE type, RENDER_TYPE rType) : corepoints_{}, fiberpoints_{}, coreebo_{}, fiberebo_{},  renderType(rType), fiberType(type)
{
	readFiberParameters(type);
	if (renderType == CORE)
		renderCore = true;
}

void Fiber::readFiberParameters(FIBER_TYPE type)
{
	string filename;
	switch (type)
	{
	case COTTON1:
		filename = "cotton1.txt";
		break;
	case COTTON2:
		filename = "cotton2.txt";
		break;
	case POLYESTER1:
		filename = "polyester1.txt";
		break;
	case RAYON1:
		filename = "rayon1.txt";
		break;
	case RAYON2:
		filename = "rayon2.txt";
		break;
	case RAYON3:
		filename = "rayon3.txt";
		break;
	case RAYON4:
		filename = "rayon4.txt";
		break;
	case SILK1:
		filename = "silk1.txt";
		break;
	case SILK2:
		filename = "silk2.txt";
		break;
	default:
		throw std::runtime_error("Fiber type has not been implemented yet.");
	}

	ifstream myfile(filename);
	if (myfile.is_open())
	{
		string line;
		while (getline(myfile, line))
		{
			std::vector<std::string> splits = split(line, ' ');
			if (splits.size() < 2)    continue;
			std::string p_name = splits[0];
			if (p_name == "ply_num:")
				ply_num = stoi(splits[1]);
			else if (p_name == "fiber_num:")
				fiber_num = stoi(splits[1]);
			else if (p_name == "aabb_min:")
			{
				string subline = splits[1].substr(1, splits[1].size() - 2);
				std::vector<std::string> subsplits = split(subline, ',');
				bounding_min = glm::vec3(stof(subsplits[0]), stof(subsplits[1]), stof(subsplits[2]));
			}
			else if (p_name == "aabb_max:")
			{
				string subline = splits[1].substr(1, splits[1].size() - 2);
				std::vector<std::string> subsplits = split(subline, ',');
				bounding_max = glm::vec3(stof(subsplits[0]), stof(subsplits[1]), stof(subsplits[2]));
			}
			else if (p_name == "z_step_size:")
				z_step_size = stof(splits[1]);
			else if (p_name == "z_step_num:")
				z_step_num = stoi(splits[1]);
			else if (p_name == "fly_step_size:")
				fly_step_size = stoi(splits[1]);
			else if (p_name == "yarn_clock_wise:")
				yarn_clock_wise = stoi(splits[1]);
			else if (p_name == "fiber_clock_wise:")
				fiber_clock_wise = stoi(splits[1]);
			else if (p_name == "yarn_alpha:")
				yarn_alpha = stof(splits[1]);
			else if (p_name == "alpha:")
				alpha = stof(splits[1]);
			else if (p_name == "yarn_radius:")
				yarn_radius = stof(splits[1]);
			else if (p_name == "ellipse_long:")
				ellipse_long = stof(splits[1]);
			else if (p_name == "ellipse_short:")
				ellipse_short = stof(splits[1]);
			else if (p_name == "epsilon:")
				epsilon = stoi(splits[1]);
			else if (p_name == "beta:")
				beta = stof(splits[1]);
			else if (p_name == "R_max:")
				r_max = stof(splits[1]);
			else if (p_name == "use_migration:")
				use_migration = stoi(splits[1]);
			else if (p_name == "s_i:")
				s_i = stof(splits[1]);
			else if (p_name == "rho_min:")
				rho_min = stof(splits[1]);
			else if (p_name == "rho_max:")
				rho_max = stof(splits[1]);
			else if (p_name == "use_flyaways:")
				use_flyaways = stoi(splits[1]);
			else if (p_name == "flyaway_hair_density:")
				flyaway_hair_density = stof(splits[1]);
			else if (p_name == "flyaway_hair_ze:")
				flyaway_hair_ze = glm::vec2(stof(splits[1]), stof(splits[2]));
			else if (p_name == "flyaway_hair_r0:")
				flyaway_hair_r0 = glm::vec2(stof(splits[1]), stof(splits[2]));
			else if (p_name == "flyaway_hair_re:")
				flyaway_hair_re = glm::vec2(stof(splits[1]), stof(splits[2]));
			else if (p_name == "flyaway_hair_pe:")
				flyaway_hair_pe = glm::vec2(stof(splits[1]), stof(splits[2]));
			else if (p_name == "flyaway_loop_density:")
				flyaway_loop_density = stof(splits[1]);
			else if (p_name == "flyaway_loop_r1:")
				flyaway_loop_r1 = glm::vec2(stof(splits[1]), stof(splits[2]));
		}
	}
	else
	{
		throw std::runtime_error("Unable to open fiber parameters file.");
	}

	float fiberWidth = (alpha / 2.f);
	float fiberHeight = ellipse_short * (2 / 3.f);

	SCR_WIDTH = 2400;
	SCR_HEIGHT = SCR_WIDTH * (fiberHeight / fiberWidth) * 4.f;
}

void Fiber::setWindow(GLFWwindow* window)
{
	this->window = window;
	glfwSetWindowSize(window, SCR_WIDTH, SCR_HEIGHT);
}

void Fiber::initShaders()
{
	coreShader_ = Shader("fiber_vertex.glsl", "core_fragment.glsl", "core_geometry.glsl", "core_tess_control.glsl", "core_tess_eval.glsl");
	fiberShader_ = Shader("fiber_vertex.glsl", "fiber_fragment.glsl", "fiber_geometry.glsl", "fiber_tess_control.glsl", "fiber_tess_eval.glsl");
	pointsShader_ = Shader("fiber_vertex.glsl", "fiber_fragment.glsl");
}

void Fiber::initFrameBuffer()
{
	// TODO: initialize
	// Create texture for core fibers
	// ------------------------------
	// step 1: bind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	// step 2: create a texture image to attach the color attachment too
	// generate texture 
	// TODO: use texture 2d array


	// configure MSAA framebuffer
	// --------------------------
	glGenFramebuffers(1, &_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
	// create a multisampled color attachment texture
	unsigned int textureColorBufferMultiSampled0;
	unsigned int textureColorBufferMultiSampled1;
	unsigned int textureColorBufferMultiSampled2;

	glGenTextures(1, &textureColorBufferMultiSampled0);
	glGenTextures(1, &textureColorBufferMultiSampled1);
	glGenTextures(1, &textureColorBufferMultiSampled2);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled0);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled1);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled2);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled1, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled2, 0);

	// create a (also multisampled) renderbuffer object for depth and stencil attachments
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create regular FBO
	// ------------------
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, alphaTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, heightTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, alphaTexture, 0);


	// generate depth buffer for depth testing
	//glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);


	// attach it to currently bound framebuffer oject
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	// check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
		throw std::runtime_error("Framebuffer has not been properly configured.");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Fiber::initializeGL()
{
	glGenVertexArrays(1, &corevao_id_);
	glGenVertexArrays(1, &fibervao_id_);
	glGenBuffers(1, &corevbo_id_);
	glGenBuffers(1, &fibervbo_id_);
	glGenBuffers(1, &coreebo_id_);
	glGenBuffers(1, &fiberebo_id_);
	glGenFramebuffers(1, &frameBuffer);
	glGenRenderbuffers(1, &depthrenderbuffer);
	glGenTextures(1, &heightTexture);
	glGenTextures(1, &normalTexture);
	glGenTextures(1, &alphaTexture);

	
	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
		{
			glBindVertexArray(corevao_id_);
			loadPoints(true);
		}
		else
		{
			glBindVertexArray(fibervao_id_);
			loadPoints(false);
		}
		glVertexAttribPointer(POS_VAO_ID, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)0);
		glEnableVertexAttribArray(POS_VAO_ID);
		glVertexAttribPointer(NORM_VAO_ID, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(NORM_VAO_ID);
		glBindVertexArray(0);
	}


	// Determine max vertices in a patch
	GLint maxPatchVertices = 0;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxPatchVertices);
	std::cout << "Max supported patch vertices " << maxPatchVertices << std::endl;
}

Fiber::~Fiber() {
	std::cout << "deleting" << std::endl;
	glDeleteVertexArrays(1, &corevao_id_);
	glDeleteVertexArrays(1, &fibervao_id_);
	glDeleteBuffers(1, &corevbo_id_);
	glDeleteBuffers(1, &fibervbo_id_);
	glDeleteBuffers(1, &coreebo_id_);
	glDeleteBuffers(1, &fiberebo_id_);
	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteBuffers(1, &depthrenderbuffer);
	glDeleteTextures(1, &heightTexture);
	glDeleteTextures(1, &normalTexture);
	glDeleteTextures(1, &alphaTexture);
}

void Fiber::render()
{
	if (renderCore)
		renderType = CORE;
	else
		renderType = COMPLETE;

	if (renderType == COMPLETE)
	{
		// Start rendering the fibers
	// --------------------------
		glBindVertexArray(corevao_id_);
		glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
		glClearColor(0.f, 0.f, 0.f, 1.f); // temporary
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, buffers);

		// draw core fiber
		coreShader_.use();
		setFiberParameters(CORE);
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawElements(GL_PATCHES, coreebo_.size(), GL_UNSIGNED_INT, 0);

		// blit multisampled buffer(s) to normal colorbuffer of intermediate FBO
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _frameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);

		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_LINEAR);

		for (int i = 0; i < 3; i++) {
			glReadBuffer(buffers[i]);
			glDrawBuffer(buffers[i]);

			glBlitFramebuffer(0, 0,
				SCR_WIDTH,
				SCR_HEIGHT,
				0, 0,
				SCR_WIDTH,
				SCR_HEIGHT,
				GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}

		// render yarn to screen
		glBindVertexArray(fibervao_id_);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwSetWindowSize(window, 3000, 1200);
		glViewport(0, 0, 3000, 1200);


		// draw yarn
		fiberShader_.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightTexture);
		glGenerateMipmap(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture);
		glGenerateMipmap(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, alphaTexture);
		glGenerateMipmap(GL_TEXTURE_2D);

		setFiberParameters(FIBER);
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawElements(GL_PATCHES, fiberebo_.size(), GL_UNSIGNED_INT, 0);
	}
	else if (renderType == FIBER)
	{
		glBindVertexArray(fibervao_id_);
		fiberShader_.use();
		setFiberParameters(FIBER);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawElements(GL_PATCHES, fiberebo_.size(), GL_UNSIGNED_INT, 0);
	}
	else if (renderType == CORE)
	{
		glBindVertexArray(corevao_id_);
		coreShader_.use();
		setFiberParameters(CORE);
		glClearColor(0.f, 0.f, 0.f, 1.f); // temporary
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glDrawElements(GL_PATCHES, coreebo_.size(), GL_UNSIGNED_INT, 0);
	}
	else
	{
		std::runtime_error("Render type is not properly initialized.");
	}
}

const Shader& Fiber::getActiveShader()
{
	if (renderType == FIBER)
		return fiberShader_;
	if (renderType == CORE)
		return coreShader_;
	if (renderType == COMPLETE)
	{
		throw std::logic_error("Incorrect use of function. See Fiber.cpp");
	}
}

const std::vector<Shader*> Fiber::getActiveShaders()
{
	std::vector<Shader*> shaders;
	if (renderType == FIBER)
		shaders.push_back(&fiberShader_);
	if (renderType == CORE)
		shaders.push_back(&coreShader_);
	if (renderType == COMPLETE)
	{
		shaders.push_back(&coreShader_);
		shaders.push_back(&fiberShader_);
	}
	return shaders;
}

void Fiber::setRenderType(RENDER_TYPE renderType)
{
	this->renderType = renderType;
}

RENDER_TYPE Fiber::getRenderType()
{
	return renderType;
}

FIBER_TYPE Fiber::getFiberType()
{
	return fiberType;
}

// Passes to vertex shader in the form of [a, b, c, d], [b, c, d, e], [c, d, e, f] ...
void Fiber::addPoint(ControlPoint cp, bool isCore) {
	glm::vec3 pos = cp.pos;
	glm::vec3 norm = cp.norm;

	std::vector<float> &points_ = isCore ? corepoints_ : fiberpoints_;
	std::vector<GLuint> &ebo_ = isCore ? coreebo_ : fiberebo_;

	points_.push_back(pos.x);
	points_.push_back(pos.y);
	points_.push_back(pos.z);
	points_.push_back(norm.x);
	points_.push_back(norm.y);
	points_.push_back(norm.z);

	if (points_.size() <= 4 * STRIDE) {
		// first patch
		ebo_.push_back(points_.size() / STRIDE - 1);
	}
	else {
		ebo_.push_back(points_.size() / STRIDE - 4);
		ebo_.push_back(points_.size() / STRIDE - 3);
		ebo_.push_back(points_.size() / STRIDE - 2);
		ebo_.push_back(points_.size() / STRIDE - 1);
	}

	loadPoints(isCore);
}

float Fiber::getFiberAlpha()
{
	return alpha;
}

/* PRIVATE */

void Fiber::loadPoints(bool isCore) {
	std::vector<float>& points_ = isCore ? corepoints_ : fiberpoints_;
	GLuint vao_id_ = isCore ? corevao_id_ : fibervao_id_;
	GLuint vbo_id_ = isCore ? corevbo_id_ : fibervbo_id_;
	GLuint ebo_id_ = isCore ? coreebo_id_ : fiberebo_id_;
	std::vector<GLuint> &ebo_ = isCore ? coreebo_ : fiberebo_;
	glBindVertexArray(vao_id_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id_);
	glBufferData(GL_ARRAY_BUFFER, points_.size() * sizeof(float),
		points_.size() > 0 ? &points_.front() : nullptr, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ebo_.size() * sizeof(GLuint),
		ebo_.size() > 0 ? &ebo_.front() : nullptr, GL_STATIC_DRAW);
}

void Fiber::setFiberParameters(RENDER_TYPE type)
{
	Shader& shader = type == CORE ? coreShader_ : fiberShader_;

	if (renderType == COMPLETE && type == FIBER)
	{
		shader.setInt("u_heightTexture", 0);
		shader.setInt("u_normalTexture", 1);
		shader.setInt("u_alphaTexture", 2);
	}

	if (fiberType == COTTON1 || fiberType == COTTON2)
	{
		shader.setVec3("objectColor", 217 / 255.f, 109 / 255.f, 2 / 255.f);
	}
	else if (fiberType == SILK1 || fiberType == SILK2)
	{
		shader.setVec3("objectColor", 178 / 255.f, 168 / 255.f, 200 / 255.f);
	}
	else if (fiberType == POLYESTER1)
	{
		shader.setVec3("objectColor", 171 / 255.f, 201 / 255.f, 228 / 255.f);
	}
	else if (fiberType == RAYON1 || fiberType == RAYON2 || fiberType == RAYON3 || fiberType == RAYON4)
	{
		shader.setVec3("objectColor", 98 / 255.f, 142 / 255.f, 56 / 255.f);
	}

	// TODO: replace w/ file implementation
	shader.setInt("u_ply_num", ply_num);
	shader.setInt("u_fiber_num", fiber_num);

	shader.setVec3("u_bounding_min", bounding_min[0], bounding_min[1], bounding_min[2]);
	shader.setVec3("u_bounding_max", bounding_max[0], bounding_max[1], bounding_max[2]);

	shader.setFloat("u_z_step_size", z_step_size);
	shader.setInt("u_z_step_num", z_step_num);
	shader.setInt("u_fly_step_size", fly_step_size);

	shader.setInt("u_yarn_clock_wise", yarn_clock_wise);
	shader.setInt("u_fiber_clock_wise", fiber_clock_wise);
	shader.setFloat("u_yarn_alpha", yarn_alpha);
	shader.setFloat("u_alpha", alpha);

	shader.setFloat("u_yarn_radius", yarn_radius);
	shader.setFloat("u_ellipse_long", ellipse_long);
	shader.setFloat("u_ellipse_short", ellipse_short);

	shader.setInt("u_epsilon", epsilon);
	shader.setFloat("u_beta", beta);
	shader.setFloat("u_r_max", r_max);

	shader.setInt("u_use_migration", use_migration);
	shader.setFloat("u_s_i", s_i);
	shader.setFloat("u_rho_min", rho_min);
	shader.setFloat("u_rho_max", rho_max);

	shader.setInt("u_use_flyaways", use_flyaways);
	shader.setFloat("u_flyaway_hair_density", flyaway_hair_density);
	shader.setVec2("u_flyaway_hair_ze", flyaway_hair_ze[0], flyaway_hair_ze[1]);
	shader.setVec2("u_flyaway_hair_r0", flyaway_hair_r0[0], flyaway_hair_r0[1]);
	shader.setVec2("u_flyaway_hair_re", flyaway_hair_re[0], flyaway_hair_re[1]);
	shader.setVec2("u_flyaway_hair_pe", flyaway_hair_pe[0], flyaway_hair_pe[1]);
	shader.setFloat("u_flyaway_loop_density", flyaway_loop_density);
	shader.setVec2("u_flyaway_loop_r1", flyaway_loop_r1[0], flyaway_loop_r1[1]);
}

void Fiber::createGUIWindow()
{
	ImGui::Begin("Fiber Editor");
	ImGui::Text("--- Fiber and Render Types ---");
	const char* fiberTypes[] = { "Cotton 1", "Cotton 2", "Polyester 1", "Rayon 1", "Rayon 2", 
		"Rayon 3", "Rayon 4", "Silk 1", "Silk 2"};
	ImGui::Combo("Type", (int*) &fiberType, fiberTypes, IM_ARRAYSIZE(fiberTypes));
	ImGui::Checkbox("Render Core", &renderCore);

	float sMin, sMax;
	ImGui::Text("");
	ImGui::Text("--- Fiber-level Parameters ---");
	ImGui::Text("Distribution & twisting");
	sMin = 0.000f, sMax = 0.5f;
	ImGui::SliderScalar("epsilon", ImGuiDataType_::ImGuiDataType_Float, &epsilon, &sMin, &sMax, "%.3lf");
	sMin = 0.1f, 0.7;
	ImGui::SliderScalar("beta", ImGuiDataType_::ImGuiDataType_Float, &beta, &sMin, &sMax, "%.3lf");
	sMin = 0.30f, sMax = 0.79f;
	ImGui::SliderScalar("alpha", ImGuiDataType_::ImGuiDataType_Float, &alpha, &sMin, &sMax, "%.3lf");

	ImGui::Text("Migration");
	sMin = 0.6f, sMax = 0.9f;
	ImGui::SliderScalar("rho_min", ImGuiDataType_::ImGuiDataType_Float, &rho_min, &sMin, &sMax, "%.3lf");
	sMin = 1.0f, sMax = 2.f;
	ImGui::SliderScalar("s_i", ImGuiDataType_::ImGuiDataType_Float, &s_i, &sMin, &sMax, "%.3lf");

	ImGui::Text("");
	ImGui::Text("--- Ply-level parameters ---");
	ImGui::Text("Cross section");
	sMin = 0.02f, sMax = 0.06f;
	ImGui::SliderScalar("ellipse_long", ImGuiDataType_::ImGuiDataType_Float, &ellipse_long, &sMin, &sMax, "%.3lf");
	sMin = 0.01f, sMax = 0.04f;
	if ((ellipse_long - 0.005) > ellipse_short)
		ImGui::SliderScalar("ellipse_short", ImGuiDataType_::ImGuiDataType_Float, &ellipse_short, &sMin, &sMax, "%.3lf");
	
	ImGui::Text("Twisting");
	sMin = 0.02f, sMax = 0.06f;
	ImGui::SliderScalar("yarn_radius", ImGuiDataType_::ImGuiDataType_Float, &yarn_radius, &sMin, &sMax, "%.3lf");
	sMin = 0.31f, sMax = 0.8f;
	ImGui::SliderScalar("yarn_alpha", ImGuiDataType_::ImGuiDataType_Float, &yarn_alpha, &sMin, &sMax, "%.3lf");

	ImGui::Text("");
	ImGui::Text("--- Flyaway fiber distribution ---");
	sMin = 10.f, sMax = 64.f;
	ImGui::SliderScalar("loop distribution", ImGuiDataType_::ImGuiDataType_Float, &flyaway_loop_density, &sMin, &sMax, "%.3lf");
	sMin = 0.02f, sMax = 0.05f;
	ImGui::SliderScalar("normal", ImGuiDataType_::ImGuiDataType_Float, &flyaway_loop_r1[0], &sMin, &sMax, "%.3lf");
	sMin = 0.003f, sMax = 0.02f;
	ImGui::SliderScalar("standard deviation", ImGuiDataType_::ImGuiDataType_Float, &flyaway_loop_r1[1], &sMin, &sMax, "%.3lf");

	ImGui::End();
}

// Helper Functions
// ----------------
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


static std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}