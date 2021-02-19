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
out float disable;

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
uniform vec2 u_flyaway_hair_ze;
uniform vec2 u_flyaway_hair_r0;
uniform vec2 u_flyaway_hair_re;
uniform vec2 u_flyaway_hair_pe;

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

	bool prevIsHair = false;
	bool currIsHair = false;
	bool nextIsHair = false;

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
		// Calculate yarn center using input control points
		vec4 yarn_center = computeBezierCurve(u, p_1, p0, p1, p2);

		// Calculate yarn orientation 
		vec4 tangent = normalize(vec4(computeBezierDerivative(u, p_1, p0, p1, p2).xyz, 0));
		vec4 normal = normalize(vec4(computeBezierCurve(u, tcs_norm[0], tcs_norm[1], tcs_norm[2], tcs_norm[3]).xyz, 0));
		vec4 bitangent = normalize(vec4(cross(tangent.xyz, normal.xyz), 0));

		// Calculate polar angle that parameterizes fiber helix
		float position = lerp(tcs_dist[1], tcs_dist[2], u); // WARNING: currently linear
		float theta = (2 * pi * position / u_yarn_alpha); 

		// Calculate the ply center and orientation given the yarn center
		// --------------------------------------------------------------
		// Calculate ply displacement
		float ply_radius = u_yarn_radius / 2.f; 
		float ply_theta = (2*pi*(mod(v, u_ply_num))) / (u_ply_num * 1.0); // initial polar angle of i-th ply
		vec4 ply_displacement = 
			ply_radius * (cos(ply_theta + theta) * normal + sin(ply_theta + theta) * bitangent); 

		// Calculate ply orientation
		// derivatives w.r.t. theta
		vec3 deriv_ply_displacement = ply_radius * (-sin(ply_theta + theta) * normal + cos(ply_theta + theta) * bitangent).xyz;		
		float uToTheta = ((theta * u_yarn_alpha) / (2 * pi) - tcs_dist[1]) / (tcs_dist[2] - tcs_dist[1]);
		vec3 deriv_yarn_center = computeBezierDerivative(uToTheta, p_1, p0, p1, p2).xyz * (u_yarn_alpha / (2 * pi * (tcs_dist[2] - tcs_dist[1])));

		// basis vectors
		vec4 ply_tangent = normalize(vec4(deriv_ply_displacement + deriv_yarn_center, 0));
		vec4 ply_normal = normalize(vec4(ply_displacement.xyz, 0));
		vec4 ply_bitangent = normalize(vec4(cross(ply_tangent.xyz, ply_normal.xyz), 0));

		// Calculate the position of fiber
		// -------------------------------
		float rho_max = u_rho_max; // distance from ply center
		float alpha = u_alpha;

		// loop variables
		bool isLoop = false;

		// hair variables 
		bool isHair = false;
		float minRadius = 0;
		float spanRadius = 0;
		float hairAlpha = 0;
		float theta_min;
		float theta_span;

		if (u_use_flyaways == 1)
		{
			/* Loop fiber */
			// TODO: verify it's addition and not equals
			if (v < u_flyaway_loop_density * (tcs_dist[2] - tcs_dist[1]) + u_ply_num && v > u_ply_num)
			{
				rho_max += sampleLoop(v, u_flyaway_loop_r1[0], u_flyaway_loop_r1[1], u_time * 10.f) / 2.f; // TODO: verify, and see why you have to divide.
				isLoop = true;
			}
			
			/* Hair fiber */
			// TODO: fix the whole process
			if (v > (u_flyaway_loop_density * (tcs_dist[2] - tcs_dist[1]) + u_ply_num) && v < ((u_flyaway_loop_density + u_flyaway_hair_density) * (tcs_dist[2] - tcs_dist[1]) + u_ply_num))
			{
				theta_min = lerp(tcs_dist[1], tcs_dist[2], rand(vec2(v *3.f, v*2.f), u_time)); 
				float z_min = lerp(0, 2 * pi, rand(vec2(v *4.f, v*5.f), u_time));
				theta_span = sampleLoop(v, u_flyaway_hair_pe.x, u_flyaway_hair_pe.y, u_time); // modified
				float z_span = sampleLoop(v, u_flyaway_hair_ze.x, u_flyaway_hair_ze.y, u_time);
				minRadius = sampleLoop(v, u_flyaway_hair_r0.x, u_flyaway_hair_r0.y, u_time);
				spanRadius = sampleLoop(v, u_flyaway_hair_re.x, u_flyaway_hair_re.y, u_time);

				hairAlpha = (2 * pi * z_span) / theta_span;
				isHair = true;
			}
		}

		// TODO: compute in cpu and pass as texture
		float fiber_radius = 0;
		float fiber_theta = 2 * pi * rand(vec2(v * 2.f, v * 3.f), u_time * 10.f);  // theta_i
		theta = (2 * pi * position / alpha); // update theta with ply pitch
		float z_i = sampleR(v, rho_max, u_time * 10.f); 
		float y_i = sampleR(2 * v, rho_max, u_time * 10.f);
		fiber_radius = sqrt(z_i * z_i + y_i * y_i); // distance between fiber curve and the ply center;


		if (u_use_migration == 1)
		{
			// edit the distance between the ply and the fiber as you move along yarn
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

		if (isHair)
		{
			float prevPos = lerp(tcs_dist[1], tcs_dist[2], prev);
			float currPos = position;
			float nextPos = lerp(tcs_dist[1], tcs_dist[2], next);

			if (i == 0 && prevPos >= theta_min && prevPos < (theta_min + theta_span))
				prevIsHair = true;
			if (i == 1 && currPos >= theta_min && currPos < (theta_min + theta_span))
				currIsHair = true;
			if (i == 2 && nextPos >= theta_min && nextPos < (theta_min + theta_span))
				nextIsHair = true;
		}

		// TODO: implement 
		if (isHair && currIsHair)
		{
			float fiber_r_min = minRadius;
			float fiber_r_max = spanRadius + minRadius;
			float t = (position - theta_min) / theta_span;

			fiber_radius = lerp(fiber_r_min, fiber_r_max, t) * 2.f; // modified
			theta = 2 * pi * position / hairAlpha;
		} 

		// fiber displacement
		float en = u_ellipse_long;
		float eb = u_ellipse_short;
		vec4 fiber_displacement = fiber_radius * (cos(fiber_theta + theta) * ply_normal * en + 
												  sin(fiber_theta + theta) * ply_bitangent * eb);

		// fiber center
		vec4 fiber_center = yarn_center + ply_displacement + fiber_displacement;

		//fiber_center = yarn_center; // DEBUG

		if (i == 0) 
			prevPosition = fiber_center.xyz;
		if (i == 1)
		{
			gl_Position = fiber_center;
			disable = 0.f;
			geo_normal = normalize(fiber_center - yarn_center).xyz;
			//geo_normal = isHair ? position >= theta_min && position < (theta_min + theta_span) ? vec3(1, 1, 0) : vec3(1, 0, 0) : isLoop ? vec3(0, 1, 0) : vec3(0, 0, 1); // DEBUG
			geo_texCoords[0] = (1 / (2 * pi)) * ((theta * pow(u_yarn_alpha, 2.f) * u_alpha)/abs(u_yarn_alpha - u_alpha) + acos(dot(view_dir, normal.xyz)) / 2.f); // u coord
			geo_texCoords[1] = 0; // will be set by geometry shader
		}
		if (i == 2)
		{
			nextPosition = fiber_center.xyz;
			// handle hair cases
			if (!prevIsHair && !currIsHair && nextIsHair)
			{
				// case 1
				gl_Position = vec4(prevPosition, 1);
				disable = 1.f;
			}
			if (!prevIsHair && currIsHair && nextIsHair)
			{
				// case 2 
				gl_Position = vec4(nextPosition, 1);
				disable = 1.f;
			}
			if (prevIsHair && currIsHair && !nextIsHair)
			{
				// case 3
				gl_Position = vec4(prevPosition, 1);
				disable = 1.f;
			}
			if (prevIsHair && !currIsHair && !nextIsHair)
			{
				// case 4
				gl_Position = vec4(prevPosition, 1);
				disable = 1.f;
			}
		}
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