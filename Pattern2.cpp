#include "Pattern2.h"
#define pi 3.14159265358979323846f

Pattern2::Pattern2(float density) : density(density), indexOffset(0)
{}

template <class T>
T lerp(T& start, T& end, float k)
{
	return (1 - k) * start + k * end;
}

std::vector<Strand> Pattern2::getUnitPattern(std::vector<Point> points, float edgeLength)
{
	// points 
	Point& p1 = points.at(0);
	Point& p2 = points.at(1);
	Point& p3 = points.at(2);
	Point& p4 = points.at(3);

	// normals
	Normal n1 = glm::normalize(p2 - p1);
	Normal n2 = glm::normalize(p3 - p2);
	Normal n3 = glm::normalize(p3 - p4);
	Normal n4 = glm::normalize(p4 - p1);

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
		Point startPoint = lerp(p1, p4, i * u); // Warning: i * u doesn't go to 1.f
		Point endPoint = lerp(p2, p3, i * u);

		Normal horizontalNorm = glm::normalize(endPoint - startPoint);

		std::vector<ControlPoint> points;

		for (float j = -1; j <= numOfStrands / 2; j++)
		{
			if (j == -1)
			{
				points.push_back(
					ControlPoint{
						startPoint - 0.0001f,
						-horizontalNorm,
						inx++,
						((pi * r) / 2.f) * inx
					}
				);
				continue;
			}

			Point leftPoint = lerp(startPoint, endPoint, 2 * j * u);
			Point rightPoint = lerp(startPoint, endPoint, (2 * j + 0.5) * u);

			Normal vNormLeft = lerp(n4, n2, 2 * j * u);
			Normal vNormRight = lerp(n4, n2, (2 * j + 0.5) * u);

			// center control points
			Point point = lerp(leftPoint, rightPoint, 0);
			Normal verticalNorm = lerp(vNormLeft, vNormRight, 0);
			Normal z_normal = glm::cross(horizontalNorm, verticalNorm);
			points.push_back(
				ControlPoint{
					point + z_normal * r,
					z_normal,
					inx++,
					((pi * r) / 2.f) * inx
				}
			);

			if (j == numOfStrands / 2)
			{
				points.push_back(
					ControlPoint{
						endPoint + 0.0001f,
						horizontalNorm,
						inx++,
						((pi * r) / 2.f) * inx
					}
				);
				break;
			}

			point = lerp(leftPoint, rightPoint, .3334);
			verticalNorm = lerp(vNormLeft, vNormRight, .3334);
			z_normal = glm::cross(horizontalNorm, verticalNorm);
			points.push_back(
				ControlPoint{
					point + z_normal * 0.f,
					horizontalNorm,
					inx++,
					((pi * r) / 2.f) * inx
				}
			);

			point = lerp(leftPoint, rightPoint, .6667);
			verticalNorm = lerp(vNormLeft, vNormRight, .6667);
			z_normal = glm::cross(horizontalNorm, verticalNorm);
			points.push_back(
				ControlPoint{
					point + z_normal * -r,
					z_normal,
					inx++,
					((pi * r) / 2.f) * inx
				}
			);

			point = lerp(leftPoint, rightPoint, 1);
			verticalNorm = lerp(vNormLeft, vNormRight, 1.0);
			z_normal = glm::cross(horizontalNorm, verticalNorm);
			points.push_back(
				ControlPoint{
					point + z_normal * 0.f,
					-horizontalNorm,
					inx++,
					((pi * r) / 2.f) * inx
				}
			);

		}

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

// TODO: modify algorithm to include a 2D bezier surface as opposed lerping through each point.
std::vector<Strand> Pattern2::getUnitPattern(std::vector<Point> points, std::vector<EndPoints> endpoints, float edgeLength)
{
	// points 
	Point& p1 = points.at(0), h1 = endpoints.at(0).hPoint, v1 = endpoints.at(0).vPoint;
	Point& p2 = points.at(1), h2 = endpoints.at(1).hPoint, v2 = endpoints.at(1).vPoint;
	Point& p3 = points.at(2), h3 = endpoints.at(2).hPoint, v3 = endpoints.at(2).vPoint;
	Point& p4 = points.at(3), h4 = endpoints.at(3).hPoint, v4 = endpoints.at(3).vPoint;

	// normals
	Normal h_n1 = glm::normalize(p2 - h1), v_n1 = glm::normalize(p4 - v1);
	Normal h_n2 = glm::normalize(h2 - p1), v_n2 = glm::normalize(p3 - v2);
	Normal h_n3 = glm::normalize(h3 - p4), v_n3 = glm::normalize(v3 - p2);
	Normal h_n4 = glm::normalize(p3 - h4), v_n4 = glm::normalize(v4 - p1);

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
		Point startPoint = lerp(p1, p4, i * u);
		Point endPoint = lerp(p2, p3, i * u);

		/*Point startEndPoint = lerp(h1, h4, i * u);
		Point endEndPoint = lerp(h2, h3, i * u);*/

		Normal h_startNorm = lerp(h_n1, h_n4, i * u), v_startNorm = lerp(v_n1, v_n4, i * u);
		Normal h_endNorm = lerp(h_n2, h_n3, i * u), v_endNorm = lerp(v_n2, v_n3, i * u);

		std::vector<ControlPoint> points;

		// left control point
		// TODO: insert here 

		for (float j = -1; j <= numOfStrands / 2; j++)
		{
			if (j == -1)
			{
				// TODO: insert left control point
				continue;
			}

			Point leftPoint = lerp(startPoint, endPoint, 2 * j * u);
			Point rightPoint = lerp(startPoint, endPoint, (2 * j + 0.5) * u);

			Normal h_leftNorm = lerp(h_startNorm, h_endNorm, 2 * j * u);
			Normal v_leftNorm = lerp(v_startNorm, v_endNorm, 2 * j * u);
			Normal h_rightNorm = lerp(h_startNorm, h_endNorm, (2 * j + 1) * u);
			Normal v_rightNorm = lerp(v_startNorm, v_endNorm, (2 * j + 1) * u);

			// center control points
			Point point = lerp(leftPoint, rightPoint, 0);
			Normal h_normal = lerp(h_leftNorm, h_rightNorm, 0);
			Normal v_normal = lerp(v_leftNorm, v_rightNorm, 0);
			Normal z_normal = glm::cross(h_normal, v_normal);
			points.push_back(
				ControlPoint{
					point + z_normal * r,
					z_normal,
					inx++,
					((pi * r) / 2.f) * inx
				}
			);

			if (j == numOfStrands / 2)
			{
				// TODO: insert right control point
				break;
			}

			point = lerp(leftPoint, rightPoint, .3334);
			h_normal = lerp(h_leftNorm, h_rightNorm, .3334);
			v_normal = lerp(h_leftNorm, h_rightNorm, .3334);
			z_normal = glm::cross(h_normal, v_normal);
			points.push_back(
				ControlPoint{
					point + z_normal * 0.f,
					h_normal,
					inx++,
					((pi * r) / 2.f) * inx
				}
			);

			point = lerp(leftPoint, rightPoint, .6667);
			h_normal = lerp(h_leftNorm, h_rightNorm, .6667);
			v_normal = lerp(h_leftNorm, h_rightNorm, .6667);
			z_normal = glm::cross(h_normal, v_normal);
			points.push_back(
				ControlPoint{
					point + z_normal * -r,
					z_normal,
					inx++,
					((pi * r) / 2.f) * inx
				}
			);

			point = lerp(leftPoint, rightPoint, 1);
			h_normal = lerp(h_leftNorm, h_rightNorm, 1);
			v_normal = lerp(h_leftNorm, h_rightNorm, 1);
			z_normal = glm::cross(h_normal, v_normal);
			points.push_back(
				ControlPoint{
					point + z_normal * 0.f,
					-h_normal,
					inx++,
					((pi * r) / 2.f) * inx
				}
			);

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

