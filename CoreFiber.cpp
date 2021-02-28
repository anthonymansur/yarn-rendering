#include "CoreFiber.h"

CoreFiber::CoreFiber(Shader *shader, FIBER_TYPE type) : Fiber::Fiber(type)
{
	m_shader = shader;
}

CoreFiber::~CoreFiber()
{
	Fiber::~Fiber();
	glDeleteFramebuffers(1, &m_interFramebuffer);
	glDeleteFramebuffers(1, &m_frameBuffer);
	glDeleteBuffers(1, &depthrenderbuffer);
	glDeleteTextures(1, &heightTexture);
	glDeleteTextures(1, &normalTexture);
	glDeleteTextures(1, &alphaTexture);
}

void CoreFiber::initializeGL()
{
	Fiber::initializeGL();
	/* Main impl. does not gen inter frame buf here */
	glGenFramebuffers(1, &m_interFramebuffer);
	glGenFramebuffers(1, &m_frameBuffer);
	glGenRenderbuffers(1, &depthrenderbuffer);
	glGenTextures(1, &heightTexture);
	glGenTextures(1, &normalTexture);
	glGenTextures(1, &alphaTexture);
}

void CoreFiber::initFrameBuffer()
{
	// TODO: initialize
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
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, SCR_WIDTH, CORE_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled1);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, SCR_WIDTH, CORE_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled2);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, SCR_WIDTH, CORE_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled1, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled2, 0);

	// create a (also multisampled) renderbuffer object for depth and stencil attachments
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH_COMPONENT, SCR_WIDTH, CORE_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create regular FBO
	// ------------------
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, CORE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, CORE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, alphaTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, CORE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
}

void CoreFiber::render()
{
	glBindVertexArray(m_vao);
	glBindFramebuffer(GL_FRAMEBUFFER, m_interFramebuffer);
	glClearColor(0.f, 0.f, 0.f, 1.f); // temporary
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SCR_WIDTH, CORE_HEIGHT);
	GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, buffers);

	// draw core fiber
	m_shader->use();
	setFiberParameters();
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, m_indices.size(), GL_UNSIGNED_INT, 0);

	// blit multisampled buffer(s) to normal colorbuffer of intermediate FBO
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_interFramebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer);

	glBlitFramebuffer(0, 0, SCR_WIDTH, CORE_HEIGHT, 0, 0, SCR_WIDTH, CORE_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_LINEAR);

	for (int i = 0; i < 3; i++) {
		glReadBuffer(buffers[i]);
		glDrawBuffer(buffers[i]);

		glBlitFramebuffer(0, 0,
			SCR_WIDTH,
			CORE_HEIGHT,
			0, 0,
			SCR_WIDTH,
			CORE_HEIGHT,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
}

void CoreFiber::draw()
{
	glBindVertexArray(m_vao);
	m_shader->use();
	setFiberParameters();
	glClearColor(0.f, 0.f, 0.f, 1.f); // temporary
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SCR_WIDTH, CORE_HEIGHT);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawElements(GL_PATCHES, m_indices.size(), GL_UNSIGNED_INT, 0);
}

GLuint CoreFiber::getHeightTexture() const
{
	return heightTexture;
}
GLuint CoreFiber::getNormalTexture() const
{
	return normalTexture;
}
GLuint CoreFiber::getAlphaTexture() const
{
	return alphaTexture;
}