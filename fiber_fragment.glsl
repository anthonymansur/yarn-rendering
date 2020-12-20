#version 410 core

uniform float u_yarn_radius; 
uniform sampler2D u_heightTexture;

in float fs_height;
in vec3 fs_normal; 
in float fs_alpha;
in vec2 fs_texCoords;

out vec4 frag_colour;

float lerp(float p0, float p1, float t);
float clamp(float x);

void main()
{
    // map 0 to 1 to 0.388 to 0.611
    vec2 texCoords = vec2(fs_texCoords[0], lerp(0.388f, 0.611f, fs_texCoords[1]));
    vec4 heightTex = texture(u_heightTexture, texCoords);
//    vec4 normalTex = texture(u_texture,  vec3(fs_texCoords, 1));
//    vec4 alphaTex = texture(u_texture,  vec3(fs_texCoords, 2));

    float height = clamp(fs_height);
    vec4 heightColor = vec4(height, height, height, 1.f);
    
    if (fs_texCoords[0] < 0)
        frag_colour = heightColor;
    else
        frag_colour = vec4(heightTex.rgb, length(heightTex.rgb) < 0.01 ? 0 : 1);
        //frag_colour = vec4(fract(fs_texCoords[0]), 0, fract(fs_texCoords[1]), 1);
}

float lerp(float p0, float p1, float t)
{
    return (1 - t) * p0 + t * p1;
}

float clamp(float x)
{
    return x > 1 ? 1 : x < 0 ? 0 : x;
}
