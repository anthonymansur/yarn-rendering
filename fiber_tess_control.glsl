#version 410

layout(vertices = 2) out;

in vec4 vs_norm[];
in float vs_dist[];

patch out vec4 p_1; // left endpoint
patch out vec4 p2;  // right endpoint
patch out vec4 tcs_norm[4];
patch out float tcs_dist[4];

patch out int num_of_isolines; // the number of fibers

uniform int u_ply_num; // the number of plies
uniform float u_yarn_radius;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

mat4 MVP = projection * view * model;

float lerp(float p0, float p1, float t)
{
	return p0 * (1 - t) + p1 * t;
}


void main()
{
	if(gl_InvocationID == 0) {
		p_1 = gl_in[0].gl_Position;
		p2 = gl_in[3].gl_Position;
		tcs_norm[0] = vs_norm[0];
		tcs_norm[3] = vs_norm[3];
		tcs_dist[0] = vs_dist[0];
		tcs_dist[3] = vs_dist[3];
		float subdivision;

		// Level of Detail
		// ---------------
		// the further away, the greater the w value
		vec4 c_p_1 = MVP * p_1;
		vec4 c_p2 = MVP * p2;
		float width = 1 / c_p_1.w;
		float fiber_width = 0.001 * width;
		float ply_width = u_yarn_radius * width;

		float LOD = 0.0005; // TODO: make uniform

		float lambda = LOD <= fiber_width ? 1 :
					   ply_width <= LOD ? 0 :
					   (fiber_width / LOD) * ((ply_width - LOD)/(ply_width - fiber_width));
		float scaleFactor = lambda * lambda;


		// check if within viewing frustum
		vec4 ndc_p_1 = c_p_1 / c_p_1.w;
		vec4 ndc_p2 = c_p2 / c_p2.w;
		if (ndc_p2.x < -1 || ndc_p2.y < -1|| ndc_p_1.x > 1 || ndc_p_1.y > 1)
		{
			// discard yarn that's not within viewing frustum
			num_of_isolines = 0;
			subdivision = 0;
		}
		else
		{
			// Decrease the number of isolines and subdivision depending on scale factor
			num_of_isolines = int(lerp(3.f, u_ply_num * (64 / u_ply_num), scaleFactor));
			subdivision = int(lerp(2, 64, sqrt(lambda))); // TODO: make less constant?
		}

		gl_TessLevelOuter[0] = float(num_of_isolines); // number of isolines
		gl_TessLevelOuter[1] = float(subdivision); // curve subdivision
	}

	if(gl_InvocationID == 0) {
		gl_out[gl_InvocationID].gl_Position = gl_in[1].gl_Position; // left end of curve
		tcs_norm[1] = vs_norm[1];
		tcs_dist[1] = vs_dist[1];
	}

	if(gl_InvocationID == 1) {
		gl_out[gl_InvocationID].gl_Position = gl_in[2].gl_Position; // right end of curve
		tcs_norm[2] = vs_norm[2];
		tcs_dist[2] = vs_dist[2];
	}
}
