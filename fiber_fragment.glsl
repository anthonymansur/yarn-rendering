#version 410 core

uniform float u_yarn_radius; 
uniform sampler2D u_texture;

in float fs_height;
in vec3 fs_normal; 
in float fs_alpha;
in vec2 fs_texCoords;

out vec4 frag_colour;

float clamp(float x)
{
    return x > 1 ? 1 : x < 0 ? 0 : x;
}

void main()
{
    vec4 heightTex = texture(u_texture, fs_texCoords);
//    vec4 normalTex = texture(u_texture,  vec3(fs_texCoords, 1));
//    vec4 alphaTex = texture(u_texture,  vec3(fs_texCoords, 2));

    float height = clamp(fs_height);
    vec4 heightColor = vec4(height, height, height, 1.f);
    
    if (fs_texCoords[0] < 0)
        frag_colour = heightColor;
    else
        frag_colour = vec4(heightTex.rgb, length(heightTex.rgb) < 0.1 ? 0 : 1);
        //frag_colour = vec4(fract(fs_texCoords[0]), 0, fract(fs_texCoords[1]), 1);
}
