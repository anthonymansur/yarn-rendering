#include "InterleavedDrawable.h"

InterleavedDrawable::InterleavedDrawable()
	: Drawable::Drawable(), bufVBO(), vboBound()
{}

InterleavedDrawable::~InterleavedDrawable()
{
	Drawable::destroy();
	glDeleteBuffers(1, &bufVBO);
}

void InterleavedDrawable::generateVBO()
{
	vboBound = true;
	glGenBuffers(1, &bufVBO);
}

bool InterleavedDrawable::bindVBO()
{
	if (vboBound)
		glBindBuffer(GL_ARRAY_BUFFER, bufPos);
	return vboBound;

}