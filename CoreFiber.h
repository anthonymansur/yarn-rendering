#pragma once
#include "FiberDrawable.h"

class CoreFiber : public FiberDrawable
{
public:
	CoreFiber(const Fiber& fiber);
	~CoreFiber();

	void create();

	void generateTextures();
	void generateFrameBuffers();

	void initFrameBuffers();

	bool bindInterFrameBuffer();
	bool bindFrameBuffer();

	bool bindReadFrameBuffer();
	bool bindDrawFrameBuffer();

	GLuint getHeightTexture() const;
	GLuint getNormalTexture() const;
	GLuint getAlphaTexture() const;

private:
	GLuint m_interFramebuffer;
	GLuint m_frameBuffer;
	GLuint depthrenderbuffer;

	bool fbBound;
	bool interFbBound;
};