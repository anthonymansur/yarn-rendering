#include "InterleavedDrawable.h"

InterleavedDrawable::InterleavedDrawable()
	: Drawable::Drawable(), bufVBO(), vboBound()
{}

InterleavedDrawable::~InterleavedDrawable()
{
	Drawable::destroy();
	glDeleteBuffers(1, &bufVBO);
}