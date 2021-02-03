#include "Pattern.h"

Pattern::Pattern(Fiber* type) : type(type), yarnRadius(type->getYarnRadius())
{}

std::vector<Strand> Pattern::getBasicWeave(uint8_t size) const
{

	float r = this->yarnRadius;
	std::vector<Strand> strands;
	// horizontal control points
	for (uint8_t i = 0; i < size; i++)
	{
		std::vector<ControlPoint> points;

		// left endpoint
		points.push_back(
			ControlPoint{
				Point3f(-r, 0, 0),
				Normal3f(-1, 0, 0),
				0
			}
		);
		for (uint8_t j = 0; j < size; j++)
		{
			points.push_back(
				ControlPoint{
					Point3f(j * 4  * r, 0, r),
					Normal3f(0, 0, 1),
					2 * j + 1
				}
			);
			points.push_back(
				ControlPoint{
					Point3f((j * 4 + 2) * r, 0, -r),
					Normal3f(0, 0, 1),
					2 * j + 2
				}
			);
		}

		// right endpoint
		points.push_back(
			ControlPoint{
				Point3f(((size - 1) * 4 + 3) * r, 0, 0),
				Normal3f(1, 0, 0),
				2 * (size - 1) + 3
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
				Point3f(0, -r, 0),
				Normal3f(0, -1, 0),
				0
			}
		);
		for (uint8_t j = 0; j < size; j++)
		{
			points.push_back(
				ControlPoint{
					Point3f(0, j * 4 * r, -r),
					Normal3f(-1, 0, 0),
					2 * j + 1
				}
			);
			points.push_back(
				ControlPoint{
					Point3f(0, (j * 4 + 2) * r, r),
					Normal3f(-1, 0, 0),
					2 * j + 2
				}
			);
		}

		// right endpoint
		points.push_back(
			ControlPoint{
				Point3f(0, ((size - 1) * 4 + 3) * r, 0),
				Normal3f(0, 1, 0),
				2 * (size - 1) + 3
			}
		);

		strands.push_back(Strand{ points });
	}

	return strands;
}