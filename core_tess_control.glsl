#version 410

layout(vertices = 2) out;

in vec4 vs_norm[];

uniform int u_fiber_num;

patch out vec4 p_1;
patch out vec4 p2;
patch out vec4 tcs_norm[4];

patch out int num_of_isolines;

void main()
{
	if(gl_InvocationID == 0) {
		num_of_isolines = 64;
		gl_TessLevelOuter[0] = float(num_of_isolines); // number of isolines
		gl_TessLevelOuter[1] = float(64); // curve subdivision

		p_1 = gl_in[0].gl_Position;
		p2 = gl_in[3].gl_Position;
		tcs_norm[0] = vs_norm[0];
		tcs_norm[3] = vs_norm[3];
	}

	if(gl_InvocationID == 0) {
		gl_out[gl_InvocationID].gl_Position = gl_in[1].gl_Position;
		tcs_norm[1] = vs_norm[1];
	}

	if(gl_InvocationID == 1) {
		gl_out[gl_InvocationID].gl_Position = gl_in[2].gl_Position;
		tcs_norm[2] = vs_norm[2];
	}
}
