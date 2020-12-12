#version 410

layout(vertices = 2) out;

patch out vec4 p_1;
patch out vec4 p2;

patch out int num_of_isolines;

uniform int u_ply_num;

void main()
{
	if(gl_InvocationID == 0) {
		num_of_isolines = u_ply_num * 21;
		gl_TessLevelOuter[0] = float(num_of_isolines); // number of isolines
		gl_TessLevelOuter[1] = float(64); // curve subdivision

		p_1 = gl_in[0].gl_Position;
		p2 = gl_in[3].gl_Position;
	}

	if(gl_InvocationID == 0) {
		gl_out[gl_InvocationID].gl_Position = gl_in[1].gl_Position;
	}

	if(gl_InvocationID == 1) {
		gl_out[gl_InvocationID].gl_Position = gl_in[2].gl_Position;
	}
}
