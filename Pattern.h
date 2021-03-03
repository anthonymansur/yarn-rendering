#pragma once
#include "Fiber.h"

// https://www.heddels.com/2017/12/7-weave-patterns-to-know-twill-basketweave-satin-and-more/

// NOTE: this is where you will use simulation's output
class Pattern
{
public:
	Pattern(const Fiber& type);
	
	std::vector<Strand> getBasicWeave(uint8_t size) const;

	// for debugging purposes
	std::vector<Strand> _getHorizontalStrand() const;
	std::vector<Strand> _getVerticalStrand(int offset = 0) const;
private:
	const Fiber& type;
	float yarnRadius;
};