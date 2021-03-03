#include "Pattern.h"
#define pi 3.14159265358979323846f


Pattern::Pattern(const Fiber& type) : type(type), yarnRadius(type.getYarnRadius())
{}

std::vector<Strand> Pattern::getBasicWeave(uint8_t size) const
{

	float r = this->yarnRadius;
	std::vector<Strand> strands;
	int inx = 0;
	// horizontal control points
	for (uint8_t i = 0; i < size; i++)
	{
		std::vector<ControlPoint> points;

		// left endpoint
		points.push_back(
			ControlPoint{
				Point3f(-r, 2 * r * i, 0),
				Normal3f(-1, 0, 0),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
		for (uint8_t j = 0; j < size / 2; j++)
		{
			points.push_back(
				ControlPoint{
					Point3f(j * 4 * r, 2 * r * i, r),
					Normal3f(0, 0, 1),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
			points.push_back(
				ControlPoint{
					Point3f((j * 4 + 1) * r, 2 * r * i, r),
					Normal3f(1, 0, 0),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
			points.push_back(
				ControlPoint{
					Point3f((j * 4 + 2) * r, 2 * r * i, -r),
					Normal3f(0, 0, 1),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
			points.push_back(
				ControlPoint{
					Point3f((j * 4 + 3) * r, 2 * r * i, -r),
					Normal3f(-1, 0, 0),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
		}

		// right endpoint
		points.push_back(
			ControlPoint{
				Point3f(((size / 2 - 1) * 4 + 3) * r, 2 * r * i, 0),
				Normal3f(1, 0, 0),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);

		strands.push_back(Strand{ points });
	}

	// vertical control points
	for (uint8_t i = 0; i < size; i++)
	{
		std::vector<ControlPoint> points;

		// left endpoint
		points.push_back(
			ControlPoint{
				Point3f(2 * r * i + r/2.f, -r, 0 - r),
				Normal3f(0, -1, 0),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
		for (uint8_t j = 0; j < size / 2; j++)
		{
			points.push_back(
				ControlPoint{
					Point3f(2 * r * i + r / 2.f, j * 4 * r, -r - r),
					Normal3f(0, 0, 1),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
			points.push_back(
				ControlPoint{
					Point3f(2 * r * i + r / 2.f, (j * 4 + 1) * r, -r - r),
					Normal3f(0, 1, 0),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
			points.push_back(
				ControlPoint{
					Point3f(2 * r * i + r / 2.f, (j * 4 + 2) * r, r - r),
					Normal3f(0, 0, 1),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
			points.push_back(
				ControlPoint{
					Point3f(2 * r * i + r / 2.f, (j * 4 + 3) * r, r - r),
					Normal3f(0, -1, 0),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
		}

		// right endpoint
		points.push_back(
			ControlPoint{
				Point3f(2 * r * i + r / 2.f, ((size / 2 - 1) * 4 + 3) * r, 0 - r),
				Normal3f(0, 1, 0),
				inx++,
				((pi* r) / 2.f)* inx
			}
		);

		strands.push_back(Strand{ points });
	}

	return strands;
}

std::vector<Strand> Pattern::_getHorizontalStrand() const
{
	float r = this->yarnRadius;
	std::vector<Strand> strands;
	int inx = 0;
	int size = 10;

	std::vector<ControlPoint> points;

	// left endpoint
	points.push_back(
		ControlPoint{
			Point3f(-r, 0, 0),
			Normal3f(-1, 0, 0),
			inx++, 
			0
		}
	);
	for (uint8_t j = 0; j < size / 2; j++)
	{
		points.push_back(
			ControlPoint{
				Point3f(j * 4 * r, 0, r),
				Normal3f(0, 0, 1),
				inx++, 
				((pi * r) / 2.f) * inx
			}
		);
		points.push_back(
			ControlPoint{
				Point3f((j * 4 + 1) * r, 0, r),
				Normal3f(1, 0, 0),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
		points.push_back(
			ControlPoint{
				Point3f((j * 4 + 2) * r, 0, -r),
				Normal3f(0, 0, 1),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
		points.push_back(
			ControlPoint{
				Point3f((j * 4 + 3) * r, 0, -r),
				Normal3f(-1, 0, 0),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
	}

	// right endpoint
	points.push_back(
		ControlPoint{
			Point3f(((size / 2 - 1) * 4 + 3) * r, 0, 0),
			Normal3f(1, 0, 0),
			inx++,
			((pi * r) / 2.f)* inx
		}
	);

	strands.push_back(Strand{ points });

	return strands;
}
std::vector<Strand> Pattern::_getVerticalStrand(int offset) const
{
	float r = this->yarnRadius;
	std::vector<Strand> strands;
	int inx = offset;
	int size = 10;

	std::vector<ControlPoint> points;
	// left endpoint
	points.push_back(
		ControlPoint{
			Point3f(0, -r, 0),
			Normal3f(0, -1, 0),
			inx++,
			((pi * r) / 2.f)* (inx - offset)
		}
	);
	for (uint8_t j = 0; j < size / 2; j++)
	{
		points.push_back(
			ControlPoint{
				Point3f(0, j * 4 * r, -r),
				Normal3f(0, 0, 1),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
		points.push_back(
			ControlPoint{
				Point3f(0, (j * 4 + 1) * r, -r),
				Normal3f(0, 1, 0),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
		points.push_back(
			ControlPoint{
				Point3f(0, (j * 4 + 2) * r, r),
				Normal3f(0, 0, 1),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
		points.push_back(
			ControlPoint{
				Point3f(0, (j * 4 + 3) * r, r),
				Normal3f(0, -1, 0),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
	}

	// right endpoint
	points.push_back(
		ControlPoint{
			Point3f(0, ((size / 2 - 1) * 4 + 3) * r, 0),
			Normal3f(0, 1, 0),
			inx++,
				((pi * r) / 2.f)* inx
		}
	);

	strands.push_back(Strand{ points });
	return strands;
}