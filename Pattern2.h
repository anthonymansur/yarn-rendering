#pragma once
#include "Fiber.h"
#include "Fabric.h"
#include <glm/glm.hpp>
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

/**
 * TODO checklist:
 * 0. Finish basic
 * 1. Finish the getUnitPattern with linear interpolation (12 points), and then render basic case and verify
 *    it outputs the same. Then change the orientation around and see how it looks
 * 2. Create the 2D bezier curve case with 16 points
 */

class Pattern2
{
public:
	Pattern2(const Fiber& type);
	std::vector<Strand> getUnitPattern(std::vector<Point> points, float edgeLength);
	std::vector<Strand> getUnitPattern(std::vector<Point> points, std::vector<EndPoints> endpoints, float edgeLength);
	std::vector<Strand> fabricTraversal(FabricVertex* node);
private:
	mutable int indexOffset;
	const Fiber& type;
	float density;
};