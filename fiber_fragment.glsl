#version 410 core

uniform float u_yarn_radius; 

out vec4 frag_colour;
in vec4 pos;
in float height;

float clamp(float x)
{
    return x > 1 ? 1 : x < 0 ? 0 : x;
}

void main()
{
    //frag_colour = vec4(1.0);
//    float z = _pos.z;
//    float height = clamp(z / (u_yarn_radius * 2.f) + 0.5f); 
//    frag_colour = vec4(height, height, height, 1);
    frag_colour = vec4(0.5 + (1 - height) / 2.f, 0.5 + (1 - height) / 2.f, 0.5 + (1 - height) / 2.f, 1f); // height map
}
