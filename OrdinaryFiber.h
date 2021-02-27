#pragma once
#include "Fiber.h"

class OrdinaryFiber : public Fiber
{
public:
	OrdinaryFiber(Shader* shader, FIBER_TYPE type);
	~OrdinaryFiber();

	void initializeGL();
	void initShaders();
	void render();

	void setHeightTexture(GLuint);
	void setNormalTexture(GLuint);
	void setAlphaTexture(GLuint);
};