#version 410 core
#define pi 3.14159265358979323846f
#define e 2.718281746F
#define EPSILON 0.0001F

layout (isolines) in; // patch type: isolines

// input to and output of shader
// ----------------------
patch in vec4 p_1; // left endpoint
patch in vec4 p2; // right endpoint
patch in vec4 tcs_norm[4];
patch in float tcs_dist[4];
patch in int num_of_isolines;

out float isCore;
out vec4 _pos;
out vec3 prevPosition;
out vec3 nextPosition;
out vec3 geo_normal;
out vec2 geo_texCoords;

uniform vec3 view_dir;
uniform float u_time;

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

float lerp(float p0, float p1, float t);
vec4 lerp(vec4 p0, vec4 p1, float t);
vec4 slerp(vec4 p0, vec4 p1, float t);

float rand(vec2 co, float seed); // pseudo-random number gen given two floats 
float fiberDistribution(float R, float rho_max); // rejection sampling
float sampleR(float v, float rho_max, float seed); // get the radius of fiber (<= r_max) given the ply center
float normalDistribution(float x, float mu, float sigma);
float sampleLoop(float v, float mu, float sigma, float seed);

bool vectorEquality(vec3 a, vec3 b);
bool vectorEquality(vec4 a, vec4 b);

void main()
{

	// init variables 
	// --------------
	vec4 p0 = gl_in[0].gl_Position; // left endpoint of yarn
	vec4 p1 = gl_in[1].gl_Position; // right endpoint of yarn
	float u = gl_TessCoord.x; // location at the curve; from 0 to 1
	/* NOTE: v represents the i-th fiber are are working with. In general, we will have 3 plies, and 
	each ply will have v / 3 fibers. Thus, the i-th fiber corresponds to the mod(v/3)-th ply. 
	*/
	float v = round(num_of_isolines * gl_TessCoord.y); // the i-th fiber we are working with
	// needed for adjacency information for triangle strip in geometry shader 
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

	// iterate three times for adjacency information
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
		vec4 tangent = normalize(computeBezierDerivative(u, p_1, p0, p1, p2));
		vec4 normal = normalize(computeBezierCurve(u, tcs_norm[0], tcs_norm[1], tcs_norm[2], tcs_norm[3]));

		//tangent = vec4(1, 0, 0, 0); // DEBUG
	    //normal = vec4(0, 1, 0, 0); // DEBUG

		vec4 bitangent = normalize(vec4(cross(tangent.xyz, normal.xyz), 0));

		// TODO: convert parametrization to cubic
		float position = lerp(tcs_dist[1], tcs_dist[2], u);
		float theta = (2 * pi * position / u_yarn_alpha); // WARNING: currently linear

		// Calculate the fiber center given the yarn center
		// ------------------------------------------
		// calculate ply displacement
		float ply_radius = u_yarn_radius / 2.f; 
		float ply_theta = (2*pi*(mod(v, u_ply_num))) / (u_ply_num * 1.0); // initial polar angle of i-th ply

		vec4 ply_displacement = 
			ply_radius * (cos(ply_theta + theta) * normal + sin(ply_theta + theta) * bitangent); 

		// calculate fiber displacement
		vec4 derivNormal = computeBezierDerivative(u, tcs_norm[0], tcs_norm[1], tcs_norm[2], tcs_norm[3]);
		vec4 derivTangent = computeBezierSecondDerivative(u, p_1, p0, p1, p2);
		vec4 derivBitangent = normalize(vec4(cross(derivTangent.xyz, normal.xyz), 0) + 
							  vec4(cross(tangent.xyz, derivNormal.xyz), 0));
		vec4 ply_tangent = normalize(-sin(ply_theta + theta) * normal + cos(ply_theta + theta) * derivNormal + cos(ply_theta + theta) * bitangent + sin(ply_theta + theta) * derivBitangent);
		vec4 ply_normal = normalize(ply_displacement);

		// TODO: see why this is needed for it to look visually appealing
		//ply_normal = vec4(0, 1, 0, 0);
		//ply_tangent = vec4(1, 0, 0, 0);

		vec4 ply_bitangent = vec4(cross(ply_tangent.xyz, ply_normal.xyz), 0);

		float rho_max = u_rho_max;

		if (u_use_flyaways == 1)
		{
			/* Loop fiber */
			// TODO: verify it's addition and not equals
			if (v < u_flyaway_loop_density)
				rho_max += sampleLoop(v, u_flyaway_loop_r1[0], u_flyaway_loop_r1[1], u_time * 10.f) / 2.f; // TODO: verify, and see why you have to divide.
			
			/* Hair fiber */
			// TODO: implement
		}

		// TODO: compute in cpu and pass as texture
		float z_i = sampleR(v, rho_max, u_time * 10.f); // distance between fiber curve and the ply center;
		float y_i = sampleR(2 * v, rho_max, u_time * 10.f);
		float fiber_radius = sqrt(pow(z_i, 2.f) + pow(y_i, 2.f)); // TODO: add fiber migration
		float fiber_theta = atan(y_i, z_i) + 2 * pi * rand(vec2(v * 2.f, v * 3.f), u_time * 10.f);  // theta_i
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

		//fiber_center = yarn_center + ply_displacement; // DEBUG

		if (i == 0) 
			prevPosition = fiber_center.xyz;
		if (i == 1)
		{
			gl_Position = fiber_center;
			//geo_normal = normalize(fiber_center - yarn_center).xyz;
			geo_normal = normal.xyz; // DEBUG
			//geo_normal = u < 0.5 ? glm::vec3(-1, 0, 0) : glm::vec3(0, 1, 0); // DEBUG
			//geo_normal = normalize(computeBezierSecondDerivative(u, p_1, p0, p1, p2)).xyz;
			geo_texCoords[0] = (1 / (2 * pi)) * ((theta * pow(u_yarn_alpha, 2.f) * u_alpha)/abs(u_yarn_alpha - u_alpha) +
							   acos(dot(view_dir, normal.xyz)) / 2.f); // u coord
			geo_texCoords[1] = 0; // will be set by geometry shader
		}
		if (i == 2) 
			nextPosition = fiber_center.xyz;
		_pos = fiber_center; // TODO: is this being used anywhere?
	}
}

// Defintion of helper functions
// -----------------------------
vec4 computeBezierCurve(float u, vec4 p_1, vec4 p0, vec4 p1, vec4 p2)
{
	float b0 = (-1.f * u) + (2.f * u * u) + (-1.f * u * u * u);
	float b1 = (2.f) + (-5.f * u * u) + (3.f * u * u * u);
	float b2 = (u) + (4.f * u * u) + (-3.f * u * u * u);
	float b3 = (-1.f * u * u) + (u * u * u);
	return 0.5f * (b0*p_1 + b1*p0 + b2*p1 + b3*p2);
}

vec4 computeBezierDerivative(float u, vec4 p_1, vec4 p0, vec4 p1, vec4 p2)
{
	float b0 = -1.f + 4.f * u - 3.f * u * u;
	float b1 = -10 * u + 9 * u * u;
	float b2 = 1 + 8.f * u - 9.f * u * u;
	float b3 = -2.f * u + 3 * u * u;
	return 0.5f * (b0*p_1 + b1*p0 + b2*p1 + b3*p2);
}
vec4 computeBezierSecondDerivative(float u, vec4 p_1, vec4 p0, vec4 p1, vec4 p2)
{
	// TODO: implement
	float b0 = 4.f - 6.f * u;
	float b1 = -10 + 18 * u;
	float b2 = 8.f - 18.f * u;
	float b3 = -2.f + 6 * u;
	return 0.5f * (b0*p_1 + b1*p0 + b2*p1 + b3*p2);
}

float rand(vec2 co, float seed){
	vec2 new = seed * co; // TODO: fix seed
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
float sampleR(float v, float rho_max, float seed) {
	int i = 0;
	while (true) {
		float radius = sqrt(rand(vec2(v + i, v + i), seed)) * rho_max;
		float pdf = rand(vec2(v + i, v + i), seed);
		if (pdf < fiberDistribution(radius, rho_max))
			return radius;
		i++;
	}
}

// loop
float normalDistribution(float x, float mu, float sigma) {
	return 1 / (sigma * sqrt(2 * pi)) + pow(e, -pow((x - mu),2) / (2*pow(sigma, 2)));
}

float sampleLoop(float v, float mu, float sigma, float seed) {
	int i = 0;
	while (true) {
		float x = rand(vec2(v, v + i), seed);
		float distribution = normalDistribution(x, mu, sigma);
		float pdf = rand(vec2(v, v + i), seed);
		if (pdf < distribution)
			return pdf;
		i++;
	}
}

float lerp(float p0, float p1, float t)
{
	return p0 * (1 - t) + p1 * t;
}

vec4 lerp(vec4 p0, vec4 p1, float t)
{
	return p0 * (1 - t) + p1 * t;
}

vec4 slerp(vec4 p0, vec4 p1, float t)
{
	float angle = acos(dot(p0, p1) / (length(p0) * length(p1)));
	return ((sin(1 - t) * angle) / sin(angle)) * p0 + (sin(t * angle) / sin(angle)) * p1;
}

bool vectorEquality(vec3 a, vec3 b)
{
	return (a.x - b.x) < EPSILON && 
		   (a.y - b.y) < EPSILON &&
		   (a.z - b.z) < EPSILON;
}
bool vectorEquality(vec4 a, vec4 b)
{
	return (a.x - b.x) < EPSILON && 
		   (a.y - b.y) < EPSILON &&
		   (a.z - b.z) < EPSILON &&
		   (a.w - b.w) < EPSILON;
}