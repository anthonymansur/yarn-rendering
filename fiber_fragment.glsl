#version 410 core

uniform vec3 view_dir;
uniform vec3 objectColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D u_heightTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_alphaTexture;

in float fs_height;
in vec3 fs_normal; 
in float fs_alpha;
in vec2 fs_texCoords;

out vec4 out_color;

float lerp(float p0, float p1, float t);
float clamp(float x);

void main()
{
    // out_color = vec4(0, 0, 0, 1); return; // DEBUG
    bool isCore = fs_texCoords[0] >= 0;
    mat4 MVP = projection * view * model;
    MVP = mat4(1.f); // not ready

    // Textures 
    // --------
    // map 0 to 1 to 0.388 to 0.611
    vec2 texCoords = vec2(fs_texCoords[0], lerp(0.388f, 0.611f, fs_texCoords[1]));
    vec4 heightTex = texture(u_heightTexture, texCoords);
    vec4 normalTex = texture(u_normalTexture, texCoords);
    vec4 alphaTex = texture(u_alphaTexture, texCoords);

    //isCore = false; // debug

    // Height map
    // ----------
    float height = isCore ? heightTex[0] : clamp(fs_height);

    // Normal map
    // ----------
    vec3 normal = isCore ? normalTex.rgb : fs_normal;
    normal = vec3(MVP * vec4((normal - 0.5f) * 2.f, 1.f));

    // Alpha channel
    // -------------
    float alpha = isCore ? alphaTex[0] : clamp(fs_alpha);
    alpha = 1.f; // TODO: not ready

    // Lambertian lighting
    // -------------------
    float ambient = 0.3 + height / 4.f;
    float diff = max(dot(normal, isCore ? view_dir : -view_dir), 0.f); // TODO: fix
    vec3 diffuse = diff * vec3(1.f);

    vec3 color = (ambient + diffuse) * objectColor.rgb;
    out_color = vec4(color, length(color) < .1 ? 0 : alpha);  
    //out_color = vec4(fract(fs_texCoords[0]), 0, fract(fs_texCoords[1]), 1);

    //out_color = vec4(color, 1);  // debug
}

float lerp(float p0, float p1, float t)
{
    return (1 - t) * p0 + t * p1;
}

float clamp(float x)
{
    return x > 1 ? 1 : x < 0 ? 0 : x;
}
