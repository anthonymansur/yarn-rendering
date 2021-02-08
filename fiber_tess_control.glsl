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

void main()
{
	if(gl_InvocationID == 0) {
		num_of_isolines = u_ply_num * (64 / u_ply_num);
		gl_TessLevelOuter[0] = float(num_of_isolines); // number of isolines
		gl_TessLevelOuter[1] = float(64); // curve subdivision

		p_1 = gl_in[0].gl_Position;
		p2 = gl_in[3].gl_Position;
		tcs_norm[0] = vs_norm[0];
		tcs_norm[3] = vs_norm[3];
		tcs_dist[0] = vs_dist[0];
		tcs_dist[3] = vs_dist[3];
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
