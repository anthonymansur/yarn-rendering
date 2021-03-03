#include "OrdinaryFiber.h"
#include <stdexcept>

OrdinaryFiber::OrdinaryFiber(const Fiber& fiber) : FiberDrawable::FiberDrawable(fiber)
{}

OrdinaryFiber::~OrdinaryFiber()
{
	FiberDrawable::~FiberDrawable();
}

void OrdinaryFiber::setHeightTexture(GLuint i)
{
	heightTexBound = true;
	heightTexture = i;
}

void OrdinaryFiber::setNormalTexture(GLuint i)
{
	normalTexBound = true;
	normalTexture = i;
}

void OrdinaryFiber::setAlphaTexture(GLuint i)
{
	alphaTexBound = true;
	alphaTexture = i;
}

void OrdinaryFiber::bindHeightTexture()
{
	if (heightTexBound)
		glBindTexture(GL_TEXTURE_2D, heightTexture);
	else
		throw std::runtime_error("No height texture to bind to.");
}
void OrdinaryFiber::bindNormalTexture()
{
	if (normalTexBound)
		glBindTexture(GL_TEXTURE_2D, normalTexture);
	else
		throw std::runtime_error("No normal texture to bind to.");
}
void OrdinaryFiber::bindAlphaTexture()
{
	if (alphaTexBound)
		glBindTexture(GL_TEXTURE_2D, alphaTexture);
	else
		throw std::runtime_error("No alpha texture to bind to.");
}

