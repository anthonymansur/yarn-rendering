#pragma once
#include "FiberDrawable.h"

class OrdinaryFiber : public FiberDrawable
{
public:
	OrdinaryFiber(const Fiber& fiber);
	~OrdinaryFiber();

	void create();

	void generateDepthMap();

	void setHeightTexture(GLuint);
	void setNormalTexture(GLuint);
	void setAlphaTexture(GLuint);

	bool bindDepthMap();
	void bindHeightTexture();
	void bindNormalTexture();
	void bindAlphaTexture();
	void bindDepthTexture();

	const unsigned int SHADOW_WIDTH, SHADOW_HEIGHT;

private:
	GLuint depthMapFBO;
	GLuint depthMap;
	bool fbBound, depthTextureBound;
	GLuint depthTexture;
};