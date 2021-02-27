#pragma once
#include "Fiber.h"

class CoreFiber : public Fiber
{
public:
	CoreFiber(Shader* shader, FIBER_TYPE type);
	~CoreFiber();

	void initializeGL();
	void initShaders();
	void initFrameBuffer();
	void render();
	void draw(); // Render to screen instead of off-screen framebuffer

	GLuint getHeightTexture() const;
	GLuint getNormalTexture() const;
	GLuint getAlphaTexture() const;
private:
	GLuint m_interFramebuffer;
	GLuint m_frameBuffer;
};