#include "Pattern.h"

Pattern::Pattern(Fiber* type) : type(type), yarnRadius(type->getYarnRadius())
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
				inx++
			}
		);
		for (uint8_t j = 0; j < size; j++)
		{
			points.push_back(
				ControlPoint{
					Point3f(j * 2 * r, 2 * r * i, r),
					Normal3f(0, 0, 1),
					inx++
				}
			);
			points.push_back(
				ControlPoint{
					Point3f((j * 4 + 2) * r, 2 * r * i, -r),
					Normal3f(0, 0, 1),
					inx++
				}
			);
		}

		// right endpoint
		points.push_back(
			ControlPoint{
				Point3f(((size - 1) * 4 + 3) * r, 2 * r * i, 0),
				Normal3f(1, 0, 0),
				inx++
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
				Point3f(2 * r * i, -r, 0),
				Normal3f(0, -1, 0),
				inx++
			}
		);
		for (uint8_t j = 0; j < size; j++)
		{
			points.push_back(
				ControlPoint{
					Point3f(2 * r * i, j * 4 * r, -r),
					Normal3f(-1, 0, 0),
					inx++
				}
			);
			points.push_back(
				ControlPoint{
					Point3f(2 * r * i, (j * 4 + 2) * r, r),
					Normal3f(-1, 0, 0),
					inx++
				}
			);
		}

		// right endpoint
		points.push_back(
			ControlPoint{
				Point3f(2 * r * i, ((size - 1) * 4 + 3) * r, 0),
				Normal3f(0, 1, 0),
				inx++
			}
		);

		strands.push_back(Strand{ points });
	}

	return strands;
}