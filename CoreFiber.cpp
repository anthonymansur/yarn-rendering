#include "CoreFiber.h"
#include <iostream>

CoreFiber::CoreFiber(const Fiber& fiber) : depthrenderbuffer(-1), fbBound(-1), interFbBound(-1), m_frameBuffer(-1), m_interFramebuffer(-1), FiberDrawable(fiber)
{}

CoreFiber::~CoreFiber()
{
	FiberDrawable::~FiberDrawable();
	glDeleteFramebuffers(1, &m_interFramebuffer);
	glDeleteFramebuffers(1, &m_frameBuffer);
	glDeleteBuffers(1, &depthrenderbuffer);
	glDeleteTextures(1, &heightTexture);
	glDeleteTextures(1, &normalTexture);
	glDeleteTextures(1, &alphaTexture);
}

void CoreFiber::create()
{
	// Create the Control Points
	// TODO: verify proper core texture mapping
	std::vector<ControlPoint> points;
	points.push_back(ControlPoint{ glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 0 });
	points.push_back(ControlPoint{ glm::vec3(0.01f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1 });
	points.push_back(ControlPoint{ glm::vec3(0.99f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 2 });
	points.push_back(ControlPoint{ glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 3 });
	for (const ControlPoint& point : points) {
		addPoint(point, true);
	}

	// Store the control points in Vertex Buffer Object
	FiberDrawable::create();

	// Clear std::vectors no longer needed
	m_points.clear();
	m_indices.clear();

	generateFrameBuffers();
	initFrameBuffers();
}

void CoreFiber::generateFrameBuffers()
{
	interFbBound = true;
	fbBound = true;
	glGenFramebuffers(1, &m_interFramebuffer);
	glGenFramebuffers(1, &m_frameBuffer);
	glGenRenderbuffers(1, &depthrenderbuffer);
}

void CoreFiber::initFrameBuffers()
{
	// Create texture for core fibers
	// ------------------------------
	// step 1: bind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

	// step 2: create a texture image to attach the color attachment too
	// generate texture 
	// TODO: use texture 2d array


	// configure MSAA framebuffer
	// --------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, m_interFramebuffer);
	// create a multisampled color attachment texture
	unsigned int textureColorBufferMultiSampled0;
	unsigned int textureColorBufferMultiSampled1;
	unsigned int textureColorBufferMultiSampled2;

	glGenTextures(1, &textureColorBufferMultiSampled0);
	glGenTextures(1, &textureColorBufferMultiSampled1);
	glGenTextures(1, &textureColorBufferMultiSampled2);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled0);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, m_fiber.SCR_WIDTH, m_fiber.CORE_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled1);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, m_fiber.SCR_WIDTH, m_fiber.CORE_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled2);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, m_fiber.SCR_WIDTH, m_fiber.CORE_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled1, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled2, 0);

	// create a (also multisampled) renderbuffer object for depth and stencil attachments
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH_COMPONENT, m_fiber.SCR_WIDTH, m_fiber.CORE_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create regular FBO
	// ------------------
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fiber.SCR_WIDTH, m_fiber.CORE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fiber.SCR_WIDTH, m_fiber.CORE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, alphaTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fiber.SCR_WIDTH, m_fiber.CORE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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

	heightTexBound = true;
	normalTexBound = true;
	alphaTexBound = true;
}

bool CoreFiber::bindFrameBuffer()
{
	if (fbBound) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	}
	return fbBound;
}

bool CoreFiber::bindInterFrameBuffer()
{
	if (interFbBound) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_interFramebuffer);
	}
	return interFbBound;
}

bool CoreFiber::bindReadFrameBuffer()
{
	if (interFbBound) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_interFramebuffer);
	}
	return interFbBound;
}
bool CoreFiber::bindDrawFrameBuffer()
{
	if (fbBound) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer);
	}
	return fbBound;
}

GLuint CoreFiber::getHeightTexture() const
{
	if (heightTexBound)
		return heightTexture;
	return -1;
}
GLuint CoreFiber::getNormalTexture() const
{
	if (normalTexBound)
		return normalTexture;
	return -1;
}
GLuint CoreFiber::getAlphaTexture() const
{
	if (alphaTexBound)
		return alphaTexture;
	return -1;
}