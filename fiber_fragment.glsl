#version 410 core

uniform float u_yarn_radius; 

out vec4 frag_colour;

float clamp(float x)
{
    return x > 1 ? 1 : x < 0 ? 0 : x;
}

void main()
{
    float z = gl_FragCoord.z;
    float height = clamp(z / (u_yarn_radius * 2.f) + 0.5f); 
    if (z > 0.5)
        height = 1;
    else
        height = 0;
    frag_colour = vec4(height, height, height, 1);
    //frag_colour = vec4(1.0f);
}
