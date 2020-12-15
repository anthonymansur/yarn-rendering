#version 410 core
#define pi 3.14159265358979323846f
#define e 2.718281746F

layout (isolines) in; // patch type: isolines

// input to and output of shader
// ----------------------
patch in vec4 p_1; // left endpoint
patch in vec4 p2; // right endpoint
patch in int num_of_isolines;

out vec3 prevPosition;
out vec3 nextPosition;

// Fiber pre-defined parameters
// ----------------------------
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

// helper functions (see below for definitions)
// ----------------
vec4 computeBezierCurve(float t, vec4 p_1, vec4 p0, vec4 p1, vec4 p2);
vec4 computeBezierDerivative(float t, vec4 p_1, vec4 p0, vec4 p1, vec4 p2);
vec4 computeBezierSecondDerivative(float t, vec4 p_1, vec4 p0, vec4 p1, vec4 p2);

float rand(vec2 co); // pseudo-random number gen given two floats 
float fiberDistribution(float R); // rejection sampling
float sampleR(float v); // get the radius of fiber (<= r_max) given the ply center
float normalDistribution(float x, float mu, float sigma);
float sampleLoop(float v, float mu, float sigma);

void main()
{
	
	vec4 p0 = gl_in[0].gl_Position; // left endpoint of yarn
	vec4 p1 = gl_in[1].gl_Position; // right endpoint of yarn
	float u = gl_TessCoord.x; // location at the curve; from 0 to 1
	float v = round(num_of_isolines * gl_TessCoord.y); // the i-th fiber we are working with

	/* NOTE: v represents the i-th fiber are are working with. */
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
		vec4 tangent = vec4(normalize(computeBezierDerivative(u, p_1, p0, p1, p2).xyz), 0);
		vec4 normal = vec4(normalize(computeBezierSecondDerivative(u, p_1, p0, p1, p2).xyz), 0);
		vec4 bitangent = vec4(cross(tangent.xyz, normal.xyz), 0);

		// TODO: convert parametrization to cubic
		float position = (1.f - u) * p0[0] + u * p1[0];
		float theta = (position / u_yarn_alpha) * 2.f * pi; // WARNING: currently linear

		// Calculate the fiber center given the yarn center
		// ------------------------------------------
		// calculate fiber displacement

		// TODO: compute in cpu and pass as texture
		float z_i = sampleR(v); // distance between fiber curve and the ply center;
		float y_i = sampleR(2 * v);
		float fiber_radius = sqrt(pow(z_i, 2.f) + pow(y_i, 2.f)); // TODO: add fiber migration
		float fiber_theta = atan(y_i, z_i) + 2 * pi * rand(vec2(v * 2.f, v * 3.f));  // theta_i
		float en = u_ellipse_long;
		float eb = u_ellipse_short;

		theta = (position / u_alpha) * 2.f * pi; // update theta with ply pitch

		if (u_use_migration == 1)
		{
			float fiber_r_min = u_rho_min;
			float fiber_r_max = u_rho_max;
			float fiber_s = u_s_i;
			fiber_radius *= 0.5f * (fiber_r_max + fiber_r_min + 
							(fiber_r_max - fiber_r_min) * cos(fiber_theta + fiber_s * theta)); 
		}

		vec4 fiber_displacement = fiber_radius * (cos(fiber_theta + theta) * vec4(0, 1.f, 0, 0) * en + 
												  sin(fiber_theta + theta) * vec4(0, 0, 1.f, 0) * eb);

		// fiber center
		vec4 fiber_center = yarn_center + fiber_displacement;

		if (i == 0)
			prevPosition = fiber_center.xyz;
		if (i == 1)
			gl_Position = fiber_center;
		if (i == 2)
			nextPosition = fiber_center.xyz;
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
float fiberDistribution(float R) {
	float eTerm = (e - pow(e, R / u_rho_max)) / (e - 1);
	float pR = (1 - 2 * u_epsilon) * pow(eTerm, u_beta) + u_epsilon;
	return pR;
}

// Used to get the radius of each fiber w.r.t. its ply center
float sampleR(float v) {
	int i = 0;
	while (true) {
		float radius = sqrt(rand(vec2(v + i, v + i))) * u_r_max;
		float pdf = rand(vec2(v + i, v + i));
		if (pdf < fiberDistribution(radius))
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