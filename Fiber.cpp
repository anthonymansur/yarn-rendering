#include "Fiber.h"

#define CORE_RENDER

namespace
{
	const GLuint POS_VAO_ID = 0;
	const GLuint STRIDE = 3;
}

#ifdef CORE_RENDER
unsigned int Fiber::SCR_WIDTH = 1200;
unsigned int Fiber::SCR_HEIGHT = 300;
#else
unsigned int Fiber::SCR_WIDTH = 3000;
unsigned int Fiber::SCR_HEIGHT = 600;
#endif


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

	// Create texture for core fibers
	// ------------------------------
	// texture dimension
	int width = 800;
	int height = 600;

	// the texture we are going to render to 
	glGenTextures(1, &renderedTexture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, renderedTexture);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, 3, 0, GL_RGBA, GL_UNSIGNED_SHORT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// The depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	// configure frame buffer
	glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_ARRAY, renderedTexture, 0, 0);
	glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_ARRAY, renderedTexture, 0, 1);
	glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_ARRAY, renderedTexture, 0, 2);

	// Set the list of draw buffers.
	GLenum DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Framebuffer has not been properly configured.");

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0, 0, width, height);

	// Start rendering the fibers
	// --------------------------
	glBindVertexArray(vao_id_);

	// render core fiber to framebuffer
	coreShader_.use();
	setFiberParameters(CORE);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, ebo_.size(), GL_UNSIGNED_INT, 0);

	// render full fiber to screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	fiberShader_.use();
	setFiberParameters(FIBER);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, ebo_.size(), GL_UNSIGNED_INT, 0);
#endif
#ifdef FIBER_RENDER
	glBindVertexArray(vao_id_);
	fiberShader_.use();
	setFiberParameters(FIBER);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, ebo_.size(), GL_UNSIGNED_INT, 0);
#endif
#ifdef CORE_RENDER
	glBindVertexArray(vao_id_);
	coreShader_.use();
	setFiberParameters(CORE);
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





