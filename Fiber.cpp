#include "Fiber.h"

#define CORE_RENDER

namespace
{
	const GLuint POS_VAO_ID = 0;
	const GLuint STRIDE = 3;
}

Fiber::Fiber() : 
	points_{}, 
	ebo_{}
{
	glGenVertexArrays(1, &vao_id_);
	glGenBuffers(1, &vbo_id_);
	glGenBuffers(1, &ebo_id_);

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
}

void Fiber::render()
{
	glBindVertexArray(vao_id_);
	pointsShader_.use();
	glPointSize(7);
	glDrawArrays(GL_POINTS, 0, points_.size() / STRIDE);
#ifdef FIBER_RENDER
	fiberShader_.use();
	setFiberParameters(FIBER);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, ebo_.size(), GL_UNSIGNED_INT, 0);
#endif
#ifdef CORE_RENDER
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





