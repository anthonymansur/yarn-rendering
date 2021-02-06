#pragma once
#include "Fiber.h"

// https://www.heddels.com/2017/12/7-weave-patterns-to-know-twill-basketweave-satin-and-more/

class Pattern
{
public:
	Pattern(Fiber* type);
	
	std::vector<Strand> getBasicWeave(uint8_t size) const;
private:
	Fiber* type;
	float yarnRadius;
};