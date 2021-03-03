#include "FiberDrawable.h"

FiberDrawable::FiberDrawable(const Fiber& fiber) : m_fiber(fiber)
{
	// TODO: implement the rest of the initialization list
}

FiberDrawable::~FiberDrawable()
{
	// TODO: implement
}

// Passes to vertex shader in the form of [a, b, c, d], [b, c, d, e], [c, d, e, f] ...
void FiberDrawable::addPoint(ControlPoint cp, bool isCore) {
	glm::vec3 pos = cp.pos;
	glm::vec3 norm = cp.norm;
	float distance = cp.distanceFromStart;
	int inx = cp.inx;

	if (std::find(m_indices.begin(), m_indices.end(), inx) == m_indices.end())
	{
		m_points.push_back(pos.x);
		m_points.push_back(pos.y);
		m_points.push_back(pos.z);
		m_points.push_back(norm.x);
		m_points.push_back(norm.y);
		m_points.push_back(norm.z);
		m_points.push_back(distance);
	}

	m_indices.push_back(inx);
}

// Should only be called once
void FiberDrawable::addStrands(const std::vector<Strand>& strands)
{
	for (const Strand& strand : strands)
	{
		// add patches of the bezier curve
		for (unsigned int i = 0; i < strand.points.size(); i++)
		{
			if ((i + 3) < strand.points.size())
			{
				for (int j = i; j < i + 4; j++)
					addPoint(strand.points.at(j), false);
			}
		}
	}
}

void FiberDrawable::create()
{
	count = m_indices.size(); // TODO: verify

	generateVBO();
	glBindBuffer(GL_ARRAY_BUFFER, bufVBO);
	glBufferData(GL_ARRAY_BUFFER, m_points.size() * sizeof(float),
		m_points.size() > 0 ? &m_points.front() : nullptr, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint),
		m_indices.size() > 0 ? &m_indices.front() : nullptr, GL_STATIC_DRAW);
}

const Fiber& FiberDrawable::getFiberType() const
{
	return m_fiber;
}

GLenum FiberDrawable::drawMode()
{
	return GL_PATCHES;
}