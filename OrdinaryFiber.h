#pragma once
#include "FiberDrawable.h"

class OrdinaryFiber : public FiberDrawable
{
public:
	OrdinaryFiber(const Fiber& fiber);
	~OrdinaryFiber();

	void create();

	void setHeightTexture(GLuint);
	void setNormalTexture(GLuint);
	void setAlphaTexture(GLuint);

	void bindHeightTexture();
	void bindNormalTexture();
	void bindAlphaTexture();

};