#pragma once
#include "Fiber.h"
#include "glm/glm.hpp"
#include <vector>

// https://www.heddels.com/2017/12/7-weave-patterns-to-know-twill-basketweave-satin-and-more/

// NOTE: this is where you will use simulation's output
typedef glm::vec3 Point;

class Pattern
{
public:
	Pattern(const Fiber& type, glm::vec3 posOffset = glm::vec3(0.f));
	
	std::vector<Strand> getBasicWeave(uint8_t size) const;
	void updatePosition(glm::vec3 posOffset);

	template <size_t rows, size_t cols>
	std::vector<Strand> generatePattern(Point(&points)[rows][cols], float edgeLength) const;

	// for debugging purposes
	std::vector<Strand> _getHorizontalStrand() const;
	std::vector<Strand> _getVerticalStrand(int offset = 0) const;
private:
	const Fiber& type;
	float yarnRadius;
	mutable int indexOffset;
	glm::vec3 posOffset;
};