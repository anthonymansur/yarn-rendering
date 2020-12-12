#version 410 core

uniform float u_yarn_radius; 
in float height;
in vec2 normal; 

layout(location = 0) out vec3 heightColor;
layout(location = 1) out vec3 normalColor;
layout(location = 2) out vec4 alphaColor;

float clamp(float x)
{
    return x > 1 ? 1 : x < 0 ? 0 : x;
}

void main()
{
   float heightCentered = 0.5 + (1 - height) / 2.f;

   heightColor = vec3(clamp(1.f - height), clamp(1.f - height), clamp(1.f - height));
   normalColor = vec3(0.5, 0.5 + normal[0] / 2.f, 0.5 + (1 - normal[1]) / 2.f);
   alphaColor = vec4(1, 0.8, 0.58, (0.5 - abs(heightCentered * 0.5)) + 0.5);
}
