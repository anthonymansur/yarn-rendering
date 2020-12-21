#version 410 core
#define pi 3.14159265358979323846f
#define e 2.718281746F
#define EPSILON 0.0001F

layout (isolines) in; // patch type: isolines

// input to and output of shader
// ----------------------
patch in vec4 p_1; // left endpoint
patch in vec4 p2; // right endpoint
patch in int num_of_isolines;

out float isCore;
out vec4 _pos;
out vec3 prevPosition;
out vec3 nextPosition;
out vec3 geo_normal;
out vec2 geo_texCoords;

uniform vec3 view_dir;

// Fiber pre-defined parameters
// ----------------------------
uniform int u_ply_num;
uniform int u_fiber_num;

uniform float u_yarn_radius;
uniform float u_yarn_alpha;

uniform float u_ellipse_long;
uniform float u_ellipse_short;

// cross-sectional fiber distribution
uniform float u_beta;
uniform float u_epsilon;
uniform float u_r_max;

uniform float u_alpha; // fiber (pitch) displacement 

// migration fiber params
uniform int u_use_migration;
uniform float u_s_i;
uniform float u_rho_min;
uniform float u_rho_max;

// flyaways
uniform int u_use_flyaways;
uniform vec2 u_flyaway_loop_r1;
uniform float u_flyaway_loop_density;

uniform float u_flyaway_hair_density;
uniform float u_flyaway_hair_ze;
uniform float u_flyaway_hair_r0;
uniform float u_flyaway_hair_re;
uniform float u_flyaway_hair_pe;

// helper functions (see below for definitions)
// ----------------
vec4 computeBezierCurve(float t, vec4 p_1, vec4 p0, vec4 p1, vec4 p2);
vec4 computeBezierDerivative(float t, vec4 p_1, vec4 p0, vec4 p1, vec4 p2);
vec4 computeBezierSecondDerivative(float t, vec4 p_1, vec4 p0, vec4 p1, vec4 p2);

float rand(vec2 co); // pseudo-random number gen given two floats 
float fiberDistribution(float R, float rho_max); // rejection sampling
float sampleR(float v, float rho_max); // get the radius of fiber (<= r_max) given the ply center
float normalDistribution(float x, float mu, float sigma);
float sampleLoop(float v, float mu, float sigma);

void main()
{
	vec4 p0 = gl_in[0].gl_Position; // left endpoint of yarn
	vec4 p1 = gl_in[1].gl_Position; // right endpoint of yarn
	float u = gl_TessCoord.x; // location at the curve; from 0 to 1
	float v = round(num_of_isolines * gl_TessCoord.y); // the i-th fiber we are working with

	/* NOTE: v represents the i-th fiber are are working with. In general, we will have 3 plies, and 
	   each ply will have v / 3 fibers. Thus, the i-th fiber corresponds to the mod(v/3)-th ply. 
	*/
	// needed for adjacency information
		float prev = u - 1/64.f;
	float curr = u;
	float next = u + 1/64.f;

	if (u < 1/64.f)
	{
		prev = u;
	}
	if (u > 62.5 / 64.f)
	{
		next = u;
	}

	for (int i = 0; i < 3; i++)
	{
		if (i == 0)
			u = prev;
		if (i == 1)
			u = curr;
		if (i == 2)
			u = next;

		// Calculate yarn center and its orientation
		// -----------------------------------------
		vec4 yarn_center = computeBezierCurve(u, p_1, p0, p1, p2);

		// TODO: these values are incorrect for some reason; not working properly
		// NOTE: the reason why these values don't work is because the derivative and second derivative
		//		 equal zero. https://tutorial.math.lamar.edu/classes/calciii/TangentNormalVectors.aspx
		//       When given the control points, you may want to calculate the normals in CPU first.
		vec4 tangent = computeBezierDerivative(u, p_1, p0, p1, p2);
		vec4 normal = computeBezierSecondDerivative(u, p_1, p0, p1, p2);

		// short-term solution
		normal = vec4(0, 1, 0, 0);
		tangent = vec4(1, 0, 0, 0);

		vec4 bitangent = vec4(cross(tangent.xyz, normal.xyz), 0);

		// TODO: convert parametrization to cubic
		float position = (1.f - u) * p0[0] + u * p1[0];
		float theta = (2 * pi * position / u_yarn_alpha); // WARNING: currently linear

		// Calculate the fiber center given the yarn center
		// ------------------------------------------
		// calculate ply displacement
		float ply_radius = u_yarn_radius / 2.f; 
		float ply_theta = (2*pi*(mod(v, u_ply_num))) / (u_ply_num * 1.0); // initial polar angle of i-th ply

		vec4 ply_displacement = 
			ply_radius * (cos(ply_theta + theta) * normal + sin(ply_theta + theta) * bitangent); 

		// calculate fiber displacement
		vec4 ply_tangent = -sin(ply_theta + theta) * normal + cos(ply_theta + theta) * bitangent;
		vec4 ply_normal = normalize(ply_displacement);

		// TODO: see why this is needed for it to look visually appealing
		ply_normal = vec4(0, 1, 0, 0);
		ply_tangent = vec4(1, 0, 0, 0);

		vec4 ply_bitangent = vec4(cross(ply_tangent.xyz, ply_normal.xyz), 0);

		float rho_max = u_rho_max;

		if (u_use_flyaways == 1)
		{
			/* Loop fiber */
			// TODO: verify it's addition and not equals
			if (v < u_flyaway_loop_density)
				rho_max = sampleLoop(v, u_flyaway_loop_r1[0], u_flyaway_loop_r1[1]);
			
			/* Hair fiber */
			// TODO: implement
		}

		// TODO: compute in cpu and pass as texture
		float z_i = sampleR(v, rho_max); // distance between fiber curve and the ply center;
		float y_i = sampleR(2 * v, rho_max);
		float fiber_radius = sqrt(pow(z_i, 2.f) + pow(y_i, 2.f)); // TODO: add fiber migration
		float fiber_theta = atan(y_i, z_i) + 2 * pi * rand(vec2(v * 2.f, v * 3.f));  // theta_i
		float en = u_ellipse_long;
		float eb = u_ellipse_short;

		theta = (2 * pi * position / u_alpha); // update theta with ply pitch

		if (u_use_migration == 1)
		{
			float fiber_r_min = u_rho_min;
			float fiber_r_max = rho_max;
			float fiber_s = u_s_i;
			fiber_radius *= 0.5f * (fiber_r_max + fiber_r_min + 
							(fiber_r_max - fiber_r_min) * cos(fiber_theta + fiber_s * theta)); 
		}

		// core fibers
		if (v < u_ply_num)
			fiber_radius = 0;
		isCore = v < u_ply_num ? 1.f : 0.f;

		vec4 fiber_displacement = fiber_radius * (cos(fiber_theta + theta) * ply_normal * en + 
												  sin(fiber_theta + theta) * ply_bitangent * eb);

		// fiber center
		vec4 fiber_center = yarn_center + ply_displacement + fiber_displacement;

		if (i == 0)
			prevPosition = fiber_center.xyz;
		if (i == 1)
		{
			gl_Position = fiber_center;
			geo_normal = normalize(fiber_center - yarn_center).xyz;
			geo_texCoords[0] = (1 / (2 * pi)) * ((theta * pow(u_yarn_alpha, 2.f) * u_alpha)/abs(u_yarn_alpha - u_alpha) +
							   acos(dot(view_dir, normal.xyz)) / 2.f); // u coord
			geo_texCoords[1] = 0; // will be set by geometry shader
		}
		if (i == 2)
			nextPosition = fiber_center.xyz;
		_pos = fiber_center;
	}
}

// Defintion of helper functions
// -----------------------------
vec4 computeBezierCurve(float t, vec4 p_1, vec4 p0, vec4 p1, vec4 p2)
{
		vec4 b0 = pow(1 - t, 3.f) * p_1;
		vec4 b1 = 3 * pow(1 - t, 2.f) * t * p0;
		vec4 b2 = 3 * (1 - t) * pow(t, 2.f) * p1;
		vec4 b3 = pow(t, 3.f) * p2;

		return b0 + b1 + b2 + b3;
}

vec4 computeBezierDerivative(float t, vec4 p_1, vec4 p0, vec4 p1, vec4 p2)
{
	vec4 b0 = 3 * pow(1 - t, 2.f) * (p0 - p_1);
	vec4 b1 = 6 * (1 - t) * t * (p1 - p0);
	vec4 b2 = 3 * pow(t, 2.f) * (p2 - p1);

	return b0 + b1 + b2;
}
vec4 computeBezierSecondDerivative(float t, vec4 p_1, vec4 p0, vec4 p1, vec4 p2)
{
	vec4 b0 = 6 * (1 - t) * (p1 - 2 * p0 + p_1);
	vec4 b1 = 6 * t * (p2 - 2 * p1 + p0);

	return b0 + b1;
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// TODO: see if it's possible to only run this once at the start when u = 0;
// TODO: make sure random functions are random per yarn, not per fiber in yarn. i.e., function of its position

// rejection sampling 
float fiberDistribution(float R, float rho_max) {
	float eTerm = (e - pow(e, R / rho_max)) / (e - 1);
	float pR = (1 - 2 * u_epsilon) * pow(eTerm, u_beta) + u_epsilon;
	return pR;
}

// Used to get the radius of each fiber w.r.t. its ply center
float sampleR(float v, float rho_max) {
	int i = 0;
	while (true) {
		float radius = sqrt(rand(vec2(v + i, v + i))) * rho_max;
		float pdf = rand(vec2(v + i, v + i));
		if (pdf < fiberDistribution(radius, rho_max))
			return radius;
		i++;
	}
}

// loop
float normalDistribution(float x, float mu, float sigma) {
	return 1 / (sigma * sqrt(2 * pi)) + pow(e, -pow((x - mu),2) / (2*pow(sigma, 2)));
}

float sampleLoop(float v, float mu, float sigma) {
	int i = 0;
	while (true) {
		float x = rand(vec2(v, v + i));
		float distribution = normalDistribution(x, mu, sigma);
		float pdf = rand(vec2(v, v + i));
		if (pdf < distribution)
			return pdf;
		i++;
	}
}
