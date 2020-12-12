#version 410 core
#define pi 3.14159265358979323846f
#define e 2.718281746F

layout (isolines) in;

patch in vec4 p_1;
patch in vec4 p2;

patch in int num_of_isolines;

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

// bounding box
uniform vec3 u_bounding_max;

// random number gen
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// TODO: see if it's possible to only run this once at the start when u = 0;

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
		radius = rand(vec2(v - i, v - i)) >= 0.5 ? radius : -radius;
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


void main()
{
	
	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	float u = gl_TessCoord.x; // 
	float v = round(num_of_isolines * gl_TessCoord.y); // the i-th fiber we are working with

	// ply center
	float b0 = (-1.f * u) + (2.f * u * u) + (-1.f * u * u * u);
	float b1 = (2.f) + (-5.f * u * u) + (3.f * u * u * u);
	float b2 = (u) + (4.f * u * u) + (-3.f * u * u * u);
	float b3 = (-1.f * u * u) + (u * u * u);
	vec4 ply_center = 0.5f * (b0*p_1 + b1*p0 + b2*p1 + b3*p2);

	// fiber cross-sectional location
	float fiber_radius = sampleR(v); // temp 
	float fiber_theta = 2 * pi * rand(vec2(1, v));
	float z_i = ply_center[2] + fiber_radius;
	float y_i = ply_center[1] + fiber_radius;

	// fiber curve
	float theta = pi * ply_center[0]; // update to vary the amount of twisting per two control points
	float theta_i = atan(y_i, z_i) + fiber_theta;
	float r_i = length(vec2(y_i, z_i)); 

	if (u_use_migration == 1)
	{
		r_i = u_rho_min * r_i + ((u_rho_max - u_rho_min) * r_i / 2.f) * (cos(1 * theta + fiber_theta) + 1);
	}

	float fiber_x = (u_alpha * theta) / (2 * pi);
	float fiber_y = ply_center[1] + r_i * sin(theta + theta_i) * (u_yarn_radius / 2.f);
	float fiber_z = ply_center[2] + r_i * cos(theta + theta_i) * (u_yarn_radius / 2.f);
	float fiber_w = ply_center[3] * 0.5f; // zoom factor

	vec4 fiber_curve = vec4(fiber_x, fiber_y, fiber_z, fiber_w);
	// fiber_curve = vec4(ply_center[0], mod(v, 2) > 0 ? (u_yarn_radius / 2.f) : -(u_yarn_radius / 2.f), ply_center[2], ply_center[3]);
	gl_Position = fiber_curve;
}
