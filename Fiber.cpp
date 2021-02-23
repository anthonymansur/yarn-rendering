#include <iostream>
#include <fstream>
#include <string>

#include "Fiber.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

namespace
{
	const GLuint POS_VAO_ID = 0;
	const GLuint NORM_VAO_ID = 1;
	const GLuint DIST_VAO_ID = 2;
	const GLuint STRIDE = 7;
}

// Helper Functions
// ----------------
unsigned int loadTexture(const char* path);
static std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems);
static std::vector<std::string> split(const std::string& s, char delim);

Fiber::Fiber(MyGL *mygl, FIBER_TYPE type) : mygl(mygl), fiberType(type), shader(nullptr) // TODO: change
{
	readFiberParameters(type);
}

void Fiber::readFiberParameters(FIBER_TYPE type)
{
	std::string filename;
	switch (type)
	{
	case COTTON1:
		filename = "cotton1.txt";
		break;
	case COTTON2:
		filename = "cotton2.txt";
		break;
	case POLYESTER1:
		filename = "polyester1.txt";
		break;
	case RAYON1:
		filename = "rayon1.txt";
		break;
	case RAYON2:
		filename = "rayon2.txt";
		break;
	case RAYON3:
		filename = "rayon3.txt";
		break;
	case RAYON4:
		filename = "rayon4.txt";
		break;
	case SILK1:
		filename = "silk1.txt";
		break;
	case SILK2:
		filename = "silk2.txt";
		break;
	default:
		throw std::runtime_error("Fiber type has not been implemented yet.");
	}

	std::ifstream myfile(filename);
	if (myfile.is_open())
	{
		std::string line;
		while (getline(myfile, line))
		{
			std::vector<std::string> splits = split(line, ' ');
			if (splits.size() < 2)    continue;
			std::string p_name = splits[0];
			if (p_name == "ply_num:")
				ply_num = stoi(splits[1]);
			else if (p_name == "fiber_num:")
				fiber_num = stoi(splits[1]);
			else if (p_name == "aabb_min:")
			{
				std::string subline = splits[1].substr(1, splits[1].size() - 2);
				std::vector<std::string> subsplits = split(subline, ',');
				bounding_min = glm::vec3(stof(subsplits[0]), stof(subsplits[1]), stof(subsplits[2]));
			}
			else if (p_name == "aabb_max:")
			{
				std::string subline = splits[1].substr(1, splits[1].size() - 2);
				std::vector<std::string> subsplits = split(subline, ',');
				bounding_max = glm::vec3(stof(subsplits[0]), stof(subsplits[1]), stof(subsplits[2]));
			}
			else if (p_name == "z_step_size:")
				z_step_size = stof(splits[1]);
			else if (p_name == "z_step_num:")
				z_step_num = stoi(splits[1]);
			else if (p_name == "fly_step_size:")
				fly_step_size = stoi(splits[1]);
			else if (p_name == "yarn_clock_wise:")
				yarn_clock_wise = stoi(splits[1]);
			else if (p_name == "fiber_clock_wise:")
				fiber_clock_wise = stoi(splits[1]);
			else if (p_name == "yarn_alpha:")
				yarn_alpha = stof(splits[1]);
			else if (p_name == "alpha:")
				alpha = stof(splits[1]);
			else if (p_name == "yarn_radius:")
				yarn_radius = stof(splits[1]);
			else if (p_name == "ellipse_long:")
				ellipse_long = stof(splits[1]);
			else if (p_name == "ellipse_short:")
				ellipse_short = stof(splits[1]);
			else if (p_name == "epsilon:")
				epsilon = stoi(splits[1]);
			else if (p_name == "beta:")
				beta = stof(splits[1]);
			else if (p_name == "R_max:")
				r_max = stof(splits[1]);
			else if (p_name == "use_migration:")
				use_migration = stoi(splits[1]);
			else if (p_name == "s_i:")
				s_i = stof(splits[1]);
			else if (p_name == "rho_min:")
				rho_min = stof(splits[1]);
			else if (p_name == "rho_max:")
				rho_max = stof(splits[1]);
			else if (p_name == "use_flyaways:")
				use_flyaways = stoi(splits[1]);
			else if (p_name == "flyaway_hair_density:")
				flyaway_hair_density = stof(splits[1]);
			else if (p_name == "flyaway_hair_ze:")
				flyaway_hair_ze = glm::vec2(stof(splits[1]), stof(splits[2]));
			else if (p_name == "flyaway_hair_r0:")
				flyaway_hair_r0 = glm::vec2(stof(splits[1]), stof(splits[2]));
			else if (p_name == "flyaway_hair_re:")
				flyaway_hair_re = glm::vec2(stof(splits[1]), stof(splits[2]));
			else if (p_name == "flyaway_hair_pe:")
				flyaway_hair_pe = glm::vec2(stof(splits[1]), stof(splits[2]));
			else if (p_name == "flyaway_loop_density:")
				flyaway_loop_density = stof(splits[1]);
			else if (p_name == "flyaway_loop_r1:")
				flyaway_loop_r1 = glm::vec2(stof(splits[1]), stof(splits[2]));
		}
	}
	else
	{
		throw std::runtime_error("Unable to open fiber parameters file.");
	}

	float fiberWidth = (alpha / 2.f);
	float fiberHeight = ellipse_short * (2 / 3.f);

	SCR_WIDTH = 2400;
	SCR_HEIGHT = 2400;// SCR_WIDTH* (fiberHeight / fiberWidth) * 4.f;
	CORE_HEIGHT = SCR_WIDTH * (fiberHeight / fiberWidth) * 4.f;
}

void Fiber::initializeGL()
{
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ebo);
 
	glVertexAttribPointer(POS_VAO_ID, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)0);
	glEnableVertexAttribArray(POS_VAO_ID);
	glVertexAttribPointer(NORM_VAO_ID, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(NORM_VAO_ID);
	glVertexAttribPointer(DIST_VAO_ID, 1, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(DIST_VAO_ID);
	glBindVertexArray(0);

	glBindVertexArray(m_vao);
	loadPoints();

	// Determine max vertices in a patch
	GLint maxPatchVertices = 0;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxPatchVertices);
	std::cout << "Max supported patch vertices " << maxPatchVertices << std::endl;
}

Fiber::~Fiber() 
{
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ebo);
}

FIBER_TYPE Fiber::getFiberType() const
{
	return fiberType;
}

float Fiber::getFiberAlpha() const
{
	return alpha;
}

float Fiber::getYarnRadius() const
{
	return yarn_radius;
}

// Passes to vertex shader in the form of [a, b, c, d], [b, c, d, e], [c, d, e, f] ...
void Fiber::addPoint(ControlPoint cp, bool isCore) {
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

void Fiber::loadPoints() {
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_points.size() * sizeof(float),
		m_points.size() > 0 ? &m_points.front() : nullptr, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint),
		m_indices.size() > 0 ? &m_indices.front() : nullptr, GL_STATIC_DRAW);
}

// Should only be called once
void Fiber::addStrands(const std::vector<Strand>& strands)
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

void Fiber::setFiberParameters()
{
	shader->setInt("u_heightTexture", 0);
	shader->setInt("u_normalTexture", 1);
	shader->setInt("u_alphaTexture", 2);

	if (fiberType == COTTON1 || fiberType == COTTON2)
	{
		shader->setVec3("objectColor", 217 / 255.f, 109 / 255.f, 2 / 255.f);
	}
	else if (fiberType == SILK1 || fiberType == SILK2)
	{
		shader->setVec3("objectColor", 178 / 255.f, 168 / 255.f, 200 / 255.f);
	}
	else if (fiberType == POLYESTER1)
	{
		shader->setVec3("objectColor", 171 / 255.f, 201 / 255.f, 228 / 255.f);
	}
	else if (fiberType == RAYON1 || fiberType == RAYON2 || fiberType == RAYON3 || fiberType == RAYON4)
	{
		shader->setVec3("objectColor", 98 / 255.f, 142 / 255.f, 56 / 255.f);
	}

	// TODO: replace w/ file implementation
	shader->setInt("u_ply_num", ply_num);
	shader->setInt("u_fiber_num", fiber_num);

	shader->setVec3("u_bounding_min", bounding_min[0], bounding_min[1], bounding_min[2]);
	shader->setVec3("u_bounding_max", bounding_max[0], bounding_max[1], bounding_max[2]);

	shader->setFloat("u_z_step_size", z_step_size);
	shader->setInt("u_z_step_num", z_step_num);
	shader->setInt("u_fly_step_size", fly_step_size);

	shader->setInt("u_yarn_clock_wise", yarn_clock_wise);
	shader->setInt("u_fiber_clock_wise", fiber_clock_wise);
	shader->setFloat("u_yarn_alpha", yarn_alpha);
	shader->setFloat("u_alpha", alpha);
	
	shader->setFloat("u_yarn_radius", yarn_radius);
	shader->setFloat("u_ellipse_long", ellipse_long);
	shader->setFloat("u_ellipse_short", ellipse_short);
	
	shader->setInt("u_epsilon", epsilon);
	shader->setFloat("u_beta", beta);
	shader->setFloat("u_r_max", r_max);
	
	shader->setInt("u_use_migration", use_migration);
	shader->setFloat("u_s_i", s_i);
	shader->setFloat("u_rho_min", rho_min);
	shader->setFloat("u_rho_max", rho_max);
	
	shader->setInt("u_use_flyaways", use_flyaways);
	shader->setFloat("u_flyaway_hair_density", flyaway_hair_density);
	shader->setVec2("u_flyaway_hair_ze", flyaway_hair_ze[0], flyaway_hair_ze[1]);
	shader->setVec2("u_flyaway_hair_r0", flyaway_hair_r0[0], flyaway_hair_r0[1]);
	shader->setVec2("u_flyaway_hair_re", flyaway_hair_re[0], flyaway_hair_re[1]);
	shader->setVec2("u_flyaway_hair_pe", flyaway_hair_pe[0], flyaway_hair_pe[1]);
	shader->setFloat("u_flyaway_loop_density", flyaway_loop_density);
	shader->setVec2("u_flyaway_loop_r1", flyaway_loop_r1[0], flyaway_loop_r1[1]);
}

void Fiber::createGUIWindow()
{
	ImGui::Begin("Fiber Editor");
	ImGui::Text("--- Fiber and Render Types ---");
	const char* fiberTypes[] = { "Cotton 1", "Cotton 2", "Polyester 1", "Rayon 1", "Rayon 2",
		"Rayon 3", "Rayon 4", "Silk 1", "Silk 2" };
	ImGui::Combo("Type", (int*)&fiberType, fiberTypes, IM_ARRAYSIZE(fiberTypes));
	//ImGui::Checkbox("Render Core", &renderCore); TODO: FIX

	float sMin, sMax;
	ImGui::Text("");
	ImGui::Text("--- Fiber-level Parameters ---");
	ImGui::Text("Distribution & twisting");
	sMin = 0.000f, sMax = 0.5f;
	ImGui::SliderScalar("epsilon", ImGuiDataType_::ImGuiDataType_Float, &epsilon, &sMin, &sMax, "%.3lf");
	sMin = 0.1f, 0.7;
	ImGui::SliderScalar("beta", ImGuiDataType_::ImGuiDataType_Float, &beta, &sMin, &sMax, "%.3lf");
	sMin = 0.30f, sMax = 0.79f;
	ImGui::SliderScalar("alpha", ImGuiDataType_::ImGuiDataType_Float, &alpha, &sMin, &sMax, "%.3lf");

	ImGui::Text("Migration");
	sMin = 0.6f, sMax = 0.9f;
	ImGui::SliderScalar("rho_min", ImGuiDataType_::ImGuiDataType_Float, &rho_min, &sMin, &sMax, "%.3lf");
	sMin = 1.0f, sMax = 2.f;
	ImGui::SliderScalar("s_i", ImGuiDataType_::ImGuiDataType_Float, &s_i, &sMin, &sMax, "%.3lf");

	ImGui::Text("");
	ImGui::Text("--- Ply-level parameters ---");
	ImGui::Text("Cross section");
	sMin = 0.02f, sMax = 0.06f;
	ImGui::SliderScalar("ellipse_long", ImGuiDataType_::ImGuiDataType_Float, &ellipse_long, &sMin, &sMax, "%.3lf");
	sMin = 0.01f, sMax = 0.04f;
	if ((ellipse_long - 0.005) > ellipse_short)
		ImGui::SliderScalar("ellipse_short", ImGuiDataType_::ImGuiDataType_Float, &ellipse_short, &sMin, &sMax, "%.3lf");

	ImGui::Text("Twisting");
	sMin = 0.02f, sMax = 0.06f;
	ImGui::SliderScalar("yarn_radius", ImGuiDataType_::ImGuiDataType_Float, &yarn_radius, &sMin, &sMax, "%.3lf");
	sMin = 0.31f, sMax = 0.8f;
	ImGui::SliderScalar("yarn_alpha", ImGuiDataType_::ImGuiDataType_Float, &yarn_alpha, &sMin, &sMax, "%.3lf");

	ImGui::Text("");
	ImGui::Text("--- Flyaway fiber distribution ---");
	sMin = 10.f, sMax = 64.f;
	ImGui::SliderScalar("loop distribution", ImGuiDataType_::ImGuiDataType_Float, &flyaway_loop_density, &sMin, &sMax, "%.3lf");
	sMin = 0.02f, sMax = 0.05f;
	ImGui::SliderScalar("normal", ImGuiDataType_::ImGuiDataType_Float, &flyaway_loop_r1[0], &sMin, &sMax, "%.3lf");
	sMin = 0.003f, sMax = 0.02f;
	ImGui::SliderScalar("standard deviation", ImGuiDataType_::ImGuiDataType_Float, &flyaway_loop_r1[1], &sMin, &sMax, "%.3lf");

	ImGui::End();
}

std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


static std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}