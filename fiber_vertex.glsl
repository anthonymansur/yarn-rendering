#version 410 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec3 norm_in;
layout (location = 2) in float dist;

out vec4 vs_norm;
out float vs_dist;

void main()
{
    gl_Position = vec4(pos_in, 1);
    vs_norm = vec4(norm_in, 0);
    vs_dist = dist;
}
