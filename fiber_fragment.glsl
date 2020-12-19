#version 410 core

uniform float u_yarn_radius; 

in float fs_height;
in vec3 fs_normal; 
in float fs_alpha;
in vec2 fs_texCoords;

out vec4 frag_colour;

//layout(location = 0) out vec3 heightColor;
//layout(location = 1) out vec3 normalColor;
//layout(location = 2) out vec4 alphaColor;

float clamp(float x)
{
    return x > 1 ? 1 : x < 0 ? 0 : x;
}

void main()
{
    float height = clamp(fs_height);
    vec4 heightColor = vec4(height, height, height, 1.f);
    vec4 normalColor = vec4(fs_normal.r, fs_normal.g, fs_normal.b, 1.f);
    vec4 alphaColor = vec4(.953f, .745f, .561f, fs_alpha);  
    
    if (fs_texCoords[0] < 0)
        frag_colour = vec4(.961f, .961f, .961f, fs_alpha);
    else
        frag_colour = vec4(fract(fs_texCoords[0]), 0, fract(fs_texCoords[1]), 1);
}
