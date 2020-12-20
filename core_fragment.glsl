#version 410 core

uniform float u_yarn_radius; 

in float fs_height;
in vec3 fs_normal; 
in float fs_alpha;

layout(location = 0) out vec4 heightColor;
//layout(location = 1) out vec4 normalColor;
//layout(location = 2) out vec4 alphaColor;

float clamp(float x)
{
    return x > 1 ? 1 : x < 0 ? 0 : x;
}

void main()
{
    float height = 1 - clamp(fs_height);
    heightColor = vec4(height, height, height, 1.f);
//    normalColor = vec4(fs_normal.r, fs_normal.g, fs_normal.b, 1.f);
//    alphaColor = vec4(fs_alpha, fs_alpha, fs_alpha, 1.f);  
}
