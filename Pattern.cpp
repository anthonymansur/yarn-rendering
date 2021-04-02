#include "Pattern.h"
#include <tuple>
#define pi 3.14159265358979323846f


Pattern::Pattern(const Fiber& type, glm::vec3 posOffset) : type(type), yarnRadius(type.getYarnRadius()), posOffset(posOffset), indexOffset(0)
{}

std::vector<Strand> Pattern::getBasicWeave(uint8_t size) const
{

	float r = this->yarnRadius;
	std::vector<Strand> strands;
	int inx = indexOffset;
	// horizontal control points
	for (uint8_t i = 0; i < size; i++)
	{
		std::vector<ControlPoint> points;

		for (int8_t j = -1; j <= size / 2; j++)
		{
			float height = 2 * r * i;
			float positionIndex = j * 4;

			if (j == -1)
			{
				// left endpoint
				points.push_back(
					ControlPoint{
						posOffset + Point3f(-r, height, 0),
						Normal3f(-1, 0, 0),
						inx++,
						((pi * r) / 2.f) * inx
					}
				);
				continue;
			}

			points.push_back(
				ControlPoint{
					posOffset + Point3f(positionIndex++ * r, height, r),
					Normal3f(0, 0, 1),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);

			if (j == size / 2)
			{
				// right endpoint
				points.push_back(
					ControlPoint{
						posOffset + Point3f((j * 4) * r, height, 0),
						Normal3f(1, 0, 0),
						inx++,
						((pi * r) / 2.f) * inx
					}
				);
				break;
			}

			points.push_back(
				ControlPoint{
					posOffset + Point3f(positionIndex++ * r, height, 0),
					Normal3f(1, 0, 0),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
			points.push_back(
				ControlPoint{
					posOffset + Point3f(positionIndex++ * r, height, -r),
					Normal3f(0, 0, 1),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
			points.push_back(
				ControlPoint{
					posOffset + Point3f(positionIndex++ * r, height, 0),
					Normal3f(-1, 0, 0),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
		}

		strands.push_back(Strand{ points });
	}

	// vertical control points
	for (uint8_t i = 0; i < size; i++)
	{
		std::vector<ControlPoint> points;

		for (int8_t j = -1; j < size / 2 + 1; j++)
		{
			float width = 2 * r * i;
			float positionIndex = j * 4;

			if (j == -1)
			{
				// left endpoint
				points.push_back(
					ControlPoint{
						posOffset + Point3f(width, -r, -r),
						Normal3f(0, -1, 0),
						inx++,
						((pi * r) / 2.f) * inx
					}
				);
				continue;
			}

			points.push_back(
				ControlPoint{
					posOffset + Point3f(width, positionIndex++ * r, -r + r),
					Normal3f(0, 0, 1),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);

			if (j == size / 2)
			{
				// right endpoint
				points.push_back(
					ControlPoint{
						posOffset + Point3f(width, (j * 4) * r, -r),
						Normal3f(0, 1, 0),
						inx++,
						((pi * r) / 2.f) * inx
					}
				);
				break;
			}

			points.push_back(
				ControlPoint{
					posOffset + Point3f(width, positionIndex++ * r, -r),
					Normal3f(0, 1, 0),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
			points.push_back(
				ControlPoint{
					posOffset + Point3f(width, positionIndex++ * r, -r - r),
					Normal3f(0, 0, 1),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
			points.push_back(
				ControlPoint{
					posOffset + Point3f(width, positionIndex++ * r, -r),
					Normal3f(0, -1, 0),
					inx++,
					((pi * r) / 2.f)* inx
				}
			);
		}

		strands.push_back(Strand{ points });
	}

	indexOffset = inx + 1;

	return strands;
}

template <class T>
T lerp(T &start, T &end, float k)
{
	return (1 - k) * start + k * end;
}

bool isNull(Point& p)
{
	return p[0] == -1 || p[1] == -1 || p[2] == -1;
}

float fract(float x)
{
	return x - floor(x);
}

template <size_t rows, size_t cols>
Point f(float u, float v, Point(&points)[rows][cols])
{
	// Perform bilinear interpolation
	Point bottomLeft = points[floor(u)][floor(v)];
	Point bottomRight = points[ceil(u)][floor(v)];
	Point topLeft = points[floor(u)][ceil(v)];
	Point topRight = points[ceil(u)][ceil(v)];

	Point bottomPoint = lerp(bottomLeft, bottomRight, fract(u));
	Point topPoint = lerp(topLeft, topRight, fract(u));
	return lerp(bottomPoint, topPoint, fract(v));
}

template <size_t rows, size_t cols>
std::vector<Strand> Pattern::generatePattern(Point(&points)[rows][cols], float edgeLength) const
{
	// Define the density factor
	float d = yarnRadius / edgeLength;

	// assign the endpoints of each strand
	std::vector<float> weftStartPoints;
	std::vector<float> weftEndPoints;
	for (unsigned long i = 0; i < (cols * edgeLength) / d; i++)
	{
		float u = i * d;

		bool startFound = false, endFound = false;
		float v = 0;
		
		// find the starting position
		while (!startFound)
		{
			if (isNull(points[floor(u)][floor(v)]))
				v += d;
			else
			{
				// located the first cell with vertex data to perform bilinear interpolation 
				weftStartPoints.push_back(v);
				startFound = true;
			}
		}
		// find the end position
		while (!endFound)
		{
			if (!isNull(points[ceil(u)][ceil(v + d)]))
				v += d;
			else
			{
				// located the last cell with vertex data to perform bilinear interpolation 
				weftEndPoints.push_back(v);
				endFound = true;
			}
		}
	}
	std::vector<float> warpStartPoints;
	std::vector<float> warpEndPoints;
	for (unsigned long i = 0; i < (rows * edgeLength) / d; i++)
	{
		float v = i * d;

		bool startFound = false, endFound = false;
		float u = 0;

		// find the starting position
		while (!startFound)
		{
			if (isNull(points[floor(u)][floor(v)]))
				u += d;
			else
			{
				// located the first cell with vertex data to perform bilinear interpolation
				warpStartPoints.push_back(u);
			}
		}

		while (!endFound)
		{
			if (!isNull(points[ceil(u + d)][ceil(v)]))
				u += d;
			else
			{
				// located the last cell with vertex data to perform bilinear interpolation
				warpEndPoints.push_back(u);
			}
		}
	}

	// iterate through each strand and fill in the center points
	std::vector<std::vector<Point>> weftStrands;
	for (unsigned long i = 0; i < (cols * edgeLength) / d; i = i++)
	{
		float startPos = weftStartPoints.at(i);
		float endPos = weftEndPoints.at(i);

		std::vector<Point> strand;

		float u = i * d;
		float v = startPos;
		while (v <= endPos)
		{
			strand.push_back(f(u, v, points));
			v += d;
		}
		
		weftStrands.push_back(strand);
	}

	std::vector<std::vector<Point>> warpStrands;
	for (unsigned long i = 0; i < (rows * edgeLength) / d; i = i++)
	{
		float startPos = warpStartPoints.at(i);
		float endPos = warpEndPoints.at(i);

		std::vector<Point> strand;

		float u = startPos;
		float v = i * d;
		while (u <= endPos)
		{
			strand.push_back(f(u, v, points));
			u += d;
		}

		warpStrands.push_back(strand);
	}

	// given the center points of each strand of yarn, construct the control points
	// 1. Convert the center points of each strand into a spline 
	// 2. Edit the weave pattern generation to have a position offset of this spline
}

std::vector<Strand> Pattern::_getHorizontalStrand() const
{
	float r = this->yarnRadius;
	std::vector<Strand> strands;
	int inx = indexOffset;
	int size = 10;

	std::vector<ControlPoint> points;

	// left endpoint
	points.push_back(
		ControlPoint{
			posOffset + Point3f(-r, 0, 0),
			Normal3f(-1, 0, 0),
			inx++, 
			0
		}
	);
	for (uint8_t j = 0; j < size / 2; j++)
	{
		points.push_back(
			ControlPoint{
				posOffset + Point3f(j * 4 * r, 0, r),
				Normal3f(0, 0, 1),
				inx++, 
				((pi * r) / 2.f) * inx
			}
		);
		points.push_back(
			ControlPoint{
				posOffset + Point3f((j * 4 + 1) * r, 0, r),
				Normal3f(1, 0, 0),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
		points.push_back(
			ControlPoint{
				posOffset + Point3f((j * 4 + 2) * r, 0, -r),
				Normal3f(0, 0, 1),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
		points.push_back(
			ControlPoint{
				posOffset + Point3f((j * 4 + 3) * r, 0, -r),
				Normal3f(-1, 0, 0),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
	}

	// right endpoint
	points.push_back(
		ControlPoint{
			posOffset + Point3f(((size / 2 - 1) * 4 + 3) * r, 0, 0),
			Normal3f(1, 0, 0),
			inx++,
			((pi * r) / 2.f)* inx
		}
	);

	strands.push_back(Strand{ points });

	indexOffset = inx + 1;

	return strands;
}
std::vector<Strand> Pattern::_getVerticalStrand(int offset) const
{
	float r = this->yarnRadius;
	std::vector<Strand> strands;
	int inx = indexOffset + offset;
	int size = 10;

	std::vector<ControlPoint> points;
	// left endpoint
	points.push_back(
		ControlPoint{
			posOffset + Point3f(0, -r, 0),
			Normal3f(0, -1, 0),
			inx++,
			((pi * r) / 2.f)* (inx - offset)
		}
	);
	for (uint8_t j = 0; j < size / 2; j++)
	{
		points.push_back(
			ControlPoint{
				posOffset + Point3f(0, j * 4 * r, -r),
				Normal3f(0, 0, 1),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
		points.push_back(
			ControlPoint{
				posOffset + Point3f(0, (j * 4 + 1) * r, -r),
				Normal3f(0, 1, 0),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
		points.push_back(
			ControlPoint{
				posOffset + Point3f(0, (j * 4 + 2) * r, r),
				Normal3f(0, 0, 1),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
		points.push_back(
			ControlPoint{
				posOffset + Point3f(0, (j * 4 + 3) * r, r),
				Normal3f(0, -1, 0),
				inx++,
				((pi * r) / 2.f)* inx
			}
		);
	}

	// right endpoint
	points.push_back(
		ControlPoint{
			posOffset + Point3f(0, ((size / 2 - 1) * 4 + 3) * r, 0),
			Normal3f(0, 1, 0),
			inx++,
				((pi * r) / 2.f)* inx
		}
	);

	strands.push_back(Strand{ points });
	indexOffset = inx + 1;
	return strands;
}

void Pattern::updatePosition(glm::vec3 posOffset)
{
	this->posOffset = posOffset;
}