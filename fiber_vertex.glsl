#version 410 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 pos_in;

void main()
{
    gl_Position = vec4(pos_in, 1.0);
}
