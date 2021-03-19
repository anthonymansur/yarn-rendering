#pragma once
#include "interleaveddrawable.h"
#include <glm/glm.hpp>

class Light : public InterleavedDrawable
{
public:
	Light(glm::vec3 pos);
	~Light();

	void create();
	GLenum drawMode();

private:
	glm::vec3 lightPos;
};