#include "Pattern2.h"
#define pi 3.14159265358979323846f

Pattern2::Pattern2(float density) : density(density)
{}

template <class T>
T lerp(T& start, T& end, float k)
{
	return (1 - k) * start + k * end;
}

std::vector<Strand> Pattern2::getUnitPattern(std::vector<Point> points, std::vector<EndPoints> endpoints, float edgeLength)
{
	// points 
	Point &p1 = points.at(0), h1 = endpoints.at(0).hPoint, v1 = endpoints.at(0).vPoint;
	Point &p2 = points.at(1), h2 = endpoints.at(1).hPoint, v2 = endpoints.at(1).vPoint;
	Point &p3 = points.at(2), h3 = endpoints.at(2).hPoint, v3 = endpoints.at(2).vPoint;
	Point &p4 = points.at(3), h4 = endpoints.at(3).hPoint, v4 = endpoints.at(3).vPoint;

	// norms
	Normal h_n1 = glm::normalize(p1 - h1), v_n1 = glm::normalize(p1 - v1);
	Normal h_n2 = glm::normalize(h2 - p2), v_n2 = glm::normalize(p2 - v2);
	Normal h_n3 = glm::normalize(h3 - p3), v_n3 = glm::normalize(v3 - p3);
	Normal h_n4 = glm::normalize(p4 - h4), v_n4 = glm::normalize(v4 - p4);

	int numOfStrands = edgeLength * density; // Make sure this gives an integer

	float d1 = glm::length(p2 - p1); // distance of edge 1
	float d2 = glm::length(p3 - p2); // distance of edge 2
	float d3 = glm::length(p4 - p3); // distance of edge 3
	float d4 = glm::length(p1 - p4); // distance of edge 4

	std::vector<Strand> strands;
	int inx = indexOffset;
	float u = 1 / density; // distance between each strand
	float r = u / 4.f;

	for (int i = 0; i < numOfStrands; i++)
	{
		// for each horizontal strand
		Point startPoint = lerp(p1, p4, i*u);
		Point endPoint = lerp(p2, p3, i*u);

		Normal h_startNorm = lerp(h_n1, h_n4, i*u), v_startNorm = lerp(v_n1, v_n4, i*u);
		Normal h_endNorm = lerp(h_n2, h_n3, i*u), v_endNorm = lerp(v_n2, v_n3, i*u);

		std::vector<ControlPoint> points;

		// left control point
		// TODO: insert here 

		for (float j = 0; j < numOfStrands / 2; j++)
		{
			Point leftPoint = lerp(startPoint, endPoint, 2 * j * u);
			Point rightPoint = lerp(startPoint, endPoint, (2 * j + 1) * u);

			Normal h_leftNorm = lerp(h_startNorm, h_endNorm, 2 * j * u);
			Normal v_leftNorm = lerp(v_startNorm, v_endNorm, 2 * j * u);
			Normal h_rightNorm = lerp(h_startNorm, h_endNorm, (2 * j + 1) * u);
			Normal v_rightNorm = lerp(v_startNorm, v_endNorm, (2 * j + 1) * u);

			// center control points
			Point point = lerp(leftPoint, rightPoint, 0);
			Normal h_normal = lerp(h_leftNorm, h_rightNorm, 0);
			Normal v_normal = lerp(h_leftNorm, h_rightNorm, 0);
			Normal z_normal = glm::cross(h_normal, v_normal);
			points.push_back(
				ControlPoint{
					point + z_normal * r,
					z_normal,
					inx++,
					((pi * r) / 2.f) * inx
				}
			);

			// TODO: add point 2

			// TODO: add point 3

			// TODO: add point 4

		}

		// right control point
		// TODO: insert here 

		// TODO: generate control points for strand 

		strands.push_back(Strand{ points });
	}

	for (int i = 0; i < numOfStrands; i++)
	{
		// for each vertical strand
		Point startPoint = lerp(p1, p2, i * u);
		Point endPoint = lerp(p4, p3, i * u);

		std::vector<ControlPoint> points;

		// TODO: generate control points for strand 

		strands.push_back(Strand{ points });
	}
	return strands;
}
