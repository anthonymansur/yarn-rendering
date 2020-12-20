#include "Fiber.h"
#include <iostream>

#define COMPLETE_RENDER

namespace
{
	const GLuint POS_VAO_ID = 0;
	const GLuint STRIDE = 3;
}

float fiberWidth = (0.366114f / 2.f);
float fiberHeight = 0.0200419f * (2 / 3.f);

#ifdef CORE_RENDER
unsigned int Fiber::SCR_WIDTH = 2400;
unsigned int Fiber::SCR_HEIGHT = SCR_WIDTH * (fiberHeight / fiberWidth) * 4.f;
#else
unsigned int Fiber::SCR_WIDTH = 3000;
unsigned int Fiber::SCR_HEIGHT = 600;
#endif

unsigned int Fiber::CORE_WIDTH = 2400;
unsigned int Fiber::CORE_HEIGHT = SCR_WIDTH * (fiberHeight / fiberWidth) * 4.f;

Fiber::Fiber() : 
	points_{}, 
	ebo_{}
{
	glGenVertexArrays(1, &vao_id_);
	glGenBuffers(1, &vbo_id_);
	glGenBuffers(1, &ebo_id_);
	glGenFramebuffers(1, &frameBuffer);
	glGenRenderbuffers(1, &depthrenderbuffer);

	glBindVertexArray(vao_id_);

	loadPoints();

	glVertexAttribPointer(POS_VAO_ID, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)0);
	glEnableVertexAttribArray(POS_VAO_ID);

	// Determine max vertices in a patch
	GLint maxPatchVertices = 0;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxPatchVertices);
	std::cout << "Max supported patch vertices " << maxPatchVertices << std::endl;
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
	glGenTextures(1, &renderedTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CORE_WIDTH, CORE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// TODO: add mipmap

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

	// generate depth buffer for depth testing
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, CORE_WIDTH, CORE_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

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

Fiber::~Fiber() {
	glDeleteVertexArrays(1, &vao_id_);
	glDeleteBuffers(1, &vbo_id_);
	glDeleteBuffers(1, &ebo_id_);
	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteBuffers(1, &depthrenderbuffer);
}

void Fiber::render()
{
#ifdef COMPLETE_RENDER
	// Start rendering the fibers
	// --------------------------
	glBindVertexArray(vao_id_);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glClearColor(1.f, 1.f, 1.f, 1.f); // temporary
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, CORE_WIDTH, CORE_HEIGHT);

	// draw core fiber
	coreShader_.use();
	setFiberParameters(CORE);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, ebo_.size(), GL_UNSIGNED_INT, 0);

	// render yarn to screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	// draw yarn
	fiberShader_.use();
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	setFiberParameters(FIBER);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, ebo_.size(), GL_UNSIGNED_INT, 0);
#endif
#ifdef FIBER_RENDER
	glBindVertexArray(vao_id_);
	fiberShader_.use();
	setFiberParameters(FIBER);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, ebo_.size(), GL_UNSIGNED_INT, 0);
#endif
#ifdef CORE_RENDER
	glBindVertexArray(vao_id_);
	coreShader_.use();
	setFiberParameters(CORE);
	glClearColor(0.f, 0.f, 0.f, 1.f); // temporary
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, CORE_WIDTH, CORE_HEIGHT);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, ebo_.size(), GL_UNSIGNED_INT, 0);
#endif
}

const Shader& Fiber::getActiveShader()
{
#ifdef FIBER_RENDER
	return fiberShader_;
#endif
#ifdef CORE_RENDER
	return coreShader_;
#endif
#ifdef COMPLETE_RENDER
	throw std::logic_error("Incorrect use of function. See Fiber.cpp");
#endif
}

const std::vector<Shader*> Fiber::getActiveShaders()
{
	std::vector<Shader*> shaders;
#ifdef FIBER_RENDER
	shaders.push_back(&fiberShader_);
#endif
#ifdef CORE_RENDER
	shaders.push_back(&coreShader_);
#endif
#ifdef COMPLETE_RENDER
	shaders.push_back(&coreShader_);
	shaders.push_back(&fiberShader_);
#endif
	return shaders;
}

RENDER_TYPE Fiber::getRenderType()
{
#ifdef FIBER_RENDER
	return FIBER;
#endif 
#ifdef CORE_RENDER
	return CORE;
#endif
#ifdef COMPLETE_RENDER
	return COMPLETE;
#endif
}

// Passes to vertex shader in the form of [a, b, c, d], [b, c, d, e], [c, d, e, f] ...
void Fiber::addPoint(float x, float y, float z) {
	points_.push_back(x);
	points_.push_back(y);
	points_.push_back(z);

	if (points_.size() <= 4 * 3) {
		// first patch
		ebo_.push_back(points_.size() / 3 - 1);
	}
	else {
		ebo_.push_back(points_.size() / 3 - 4);
		ebo_.push_back(points_.size() / 3 - 3);
		ebo_.push_back(points_.size() / 3 - 2);
		ebo_.push_back(points_.size() / 3 - 1);
	}

	loadPoints();
}

/* PRIVATE */

void Fiber::loadPoints() {
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

#ifdef COMPLETE_RENDER
	if (type == FIBER)
	{
		shader.setInt("u_texture", 0);
	}
#endif

	// TODO: replace w/ file implementation
	shader.setInt("u_ply_num", 3);
	shader.setInt("u_fiber_num", 75);

	shader.setVec3("u_bounding_min", -0.0538006, -0.0538006, -0.488648);
	shader.setVec3("u_bounding_max", 0.0538006, 0.0538006, 0.488648);

	shader.setFloat("u_z_step_size", 0.01);
	shader.setInt("u_z_step_num", 98);
	shader.setInt("u_fly_step_size", 0);

	shader.setInt("u_yarn_clock_wise", 0);
	shader.setInt("u_fiber_clock_wise", 1);
	shader.setFloat("u_yarn_alpha", 0.452737);
	shader.setFloat("u_alpha", 0.366114);

	shader.setFloat("u_yarn_radius", 0.0378238);
	shader.setFloat("u_ellipse_long", 0.0257183);
	shader.setFloat("u_ellipse_short", 0.0200419);

	shader.setInt("u_epsilon", 0);
	shader.setFloat("u_beta", 0.200965);
	shader.setFloat("u_r_max", 1.f);

	shader.setInt("u_use_migration", 1);
	shader.setFloat("u_s_i", 1.1f);
	shader.setFloat("u_rho_min", 0.85);
	shader.setFloat("u_rho_max", 1.f);

	shader.setInt("u_use_flyaways", 1);
	shader.setFloat("u_flyaway_hair_density", 30.3559);
	shader.setVec2("u_flyaway_hair_ze", 0.002419, 0.0631994);
	shader.setVec2("u_flyaway_hair_r0", 0.0203949, 0.0055814);
	shader.setVec2("u_flyaway_hair_re", 0.0166551, 0.00949242);
	shader.setVec2("u_flyaway_hair_pe", 0.390188, 0.342302);
	shader.setFloat("u_flyaway_loop_density", 19.1003);
	shader.setVec2("u_flyaway_loop_r1", 0.0245694, 0.00522926);
}





