#pragma once
#include "Fiber.h"
#include "glm/glm.hpp"
#include <vector>

// https://www.heddels.com/2017/12/7-weave-patterns-to-know-twill-basketweave-satin-and-more/

// NOTE: this is where you will use simulation's output
typedef glm::vec3 Point;
typedef glm::vec3 Normal;

struct EndPoints
{
	Point hPoint; // the horizontal endpoint
	Point vPoint; // the vertical endpoint
};

// TODO: add weave type

class Pattern2
{
public:
	Pattern2(float density);

	std::vector<Strand> getUnitPattern(std::vector<Point> points, std::vector<EndPoints> endpoints, float edgeLength);
private:
	mutable int indexOffset;
	float density; // number of strands per centimeter
};