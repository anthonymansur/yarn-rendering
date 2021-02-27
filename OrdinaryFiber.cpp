#include "OrdinaryFiber.h"

OrdinaryFiber::OrdinaryFiber(Shader* shader, FIBER_TYPE type) : Fiber::Fiber(type)
{
	m_shader = shader;
}

OrdinaryFiber::~OrdinaryFiber()
{
	Fiber::~Fiber();
}

void OrdinaryFiber::initializeGL()
{
	Fiber::initializeGL();
}

void OrdinaryFiber::render()
{
	// render yarn to screen
	glBindVertexArray(m_vao);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw yarn
	m_shader->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightTexture);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, alphaTexture);
	glGenerateMipmap(GL_TEXTURE_2D);

	setFiberParameters();
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, m_indices.size(), GL_UNSIGNED_INT, 0);
}

void OrdinaryFiber::setHeightTexture(GLuint i)
{
	heightTexture = i;
}

void OrdinaryFiber::setNormalTexture(GLuint i)
{
	normalTexture = i;
}

void OrdinaryFiber::setAlphaTexture(GLuint i)
{
	alphaTexture = i;
}