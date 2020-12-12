#version 410 core
#define pi 3.14159265358979323846f
#define e 2.718281746F

layout (isolines) in;

patch in vec4 p_1;
patch in vec4 p2;

patch in int num_of_isolines;
uniform int u_ply_num;
uniform int u_fiber_num;

uniform float u_yarn_radius;
uniform float u_yarn_alpha;

// cross-sectional fiber distribution
uniform float u_beta;
uniform float u_epsilon;
uniform float u_r_max;

uniform float u_alpha; // fiber (pitch) displacement 

// migration fiber params
uniform int u_use_migration;
uniform int u_s_i;
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

out float isCore;
out vec3 prevPosition;
out vec3 nextPosition;

out vec2 textureParams;

// random number gen
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

// TODO: may need to refactor the way fibers are generated as there is a possibility you are
// messing up moving between yarn, ply, and fiber space.
void main()
{
	
	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	float u = gl_TessCoord.x; 
	float v = round(num_of_isolines * gl_TessCoord.y); // the i-th ply we are working with

	vec4 yarn_center;

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
		// yarn center
		float b0 = (-1.f * u) + (2.f * u * u) + (-1.f * u * u * u);
		float b1 = (2.f) + (-5.f * u * u) + (3.f * u * u * u);
		float b2 = (u) + (4.f * u * u) + (-3.f * u * u * u);
		float b3 = (-1.f * u * u) + (u * u * u);

		yarn_center = 0.5f * (b0*p_1 + b1*p0 + b2*p1 + b3*p2);

		// ply center
		float ply_radius = u_yarn_radius / 2.f; 
		float ply_alpha = 2 * u_yarn_alpha; // modified
		float ply_theta = (2*pi*(mod(v, u_ply_num))) / (u_ply_num * 1.0);

		float ply_x = yarn_center[0];
		float ply_y = yarn_center[1] + ply_radius * sin(2 * pi * ply_x / ply_alpha + ply_theta);
		float ply_z = yarn_center[2] + ply_radius * cos(2 * pi * ply_x / ply_alpha + ply_theta);
		float ply_w = yarn_center[3]; // zoom factor - removed due to geometry shader

		vec4 ply_center = vec4(ply_x, ply_y, ply_z, ply_w);

		// fiber cross-sectional location
		float fiber_radius = sampleR(v); // temp 
		float fiber_theta = 2 * pi * rand(vec2(1, v));
		float z_i = fiber_radius;
		float y_i = fiber_radius;

		// fiber curve
		float theta = pi * ply_center[0]; // may need to be dependant on u, not the ply_center.
		float theta_i = atan(y_i, z_i) + fiber_theta;
		float r_i = length(vec2(y_i, z_i)); 
		float rho_max = u_rho_max;
		bool isHair = false;

		if (u_use_flyaways == 1 && false)
		{
			// TODO: change to probability

			/* Loop fibers */
			int numOfLoopsPerPly = int(round(u_flyaway_loop_density * ply_radius * 2.f));
			int numOfLoops = u_ply_num * numOfLoopsPerPly;
			float loopMultiplier = pow(numOfLoops, 2) / (u_fiber_num * 1.f); // guarantee numOfLoops will be received

			/* Hair fibers */
			int numOfHairsPerPly = int(round(u_flyaway_hair_density * ply_radius * 2.f));
			int numOfHairs = u_ply_num * numOfHairsPerPly;
			float hairMultiplier = pow(numOfHairs, 2) / (u_fiber_num * 1.f);

			if (mod(ceil(v * loopMultiplier), numOfLoops) == 1)
			{
				// this is a loop fiber
				// TODO: make sure this is the correct implementation. You are setting a loop fiber to have the
				// *potential* to be a loop fiber, but not necessarily making it a loop.
				rho_max += sampleLoop(v, u_flyaway_loop_r1[0], u_flyaway_loop_r1[1]);
				r_i = u_rho_min * r_i + ((rho_max - u_rho_min) * r_i / 2.f) * (cos(u_s_i * fiber_theta + theta_i) + 1);
			} else if (mod(ceil(v * hairMultiplier), numOfHairs) == 2)
			{
				// this is a hair fiber
				// TODO: implement
			} 
		}

		if (u_use_migration == 1)
		{
			// TODO: this isn't implemented. We are currently always assuming migration will be used.
		}

		if (v < 3)
			r_i = 0;
		isCore = v < 3 ? 1.f : 0.f;

		float fiber_x = (u_alpha * theta) / (2 * pi);
		float fiber_y = ply_center[1] + r_i * sin(theta + theta_i) * ply_radius;
		float fiber_z = ply_center[2] + r_i * cos(theta + theta_i) * ply_radius;
		float fiber_w = ply_center[3];

		vec4 fiber_curve = vec4(fiber_x, fiber_y, fiber_z, fiber_w);

		if (i == 0)
			prevPosition = fiber_curve.xyz;
		if (i == 1)
			gl_Position = fiber_curve;
			textureParams = vec2(2 * pi * u, ply_alpha); // TODO: figure out why the thetas are different.
		if (i == 2)
			nextPosition = fiber_curve.xyz;
	}
}
