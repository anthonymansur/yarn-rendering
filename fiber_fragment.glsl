#version 410 core

uniform vec3 view_dir;
uniform vec3 objectColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_pos;
uniform vec3 view_pos;

uniform sampler2D u_heightTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_alphaTexture;

uniform sampler2D u_shadowMap;

uniform float u_yarn_radius;

in vec4 fs_depthPos;
in vec4 fs_fragPos;
in float fs_height;
in vec3 fs_normal; 
in float fs_alpha;
in vec2 fs_texCoords;
in float fs_disable;

out vec4 out_color;

float lerp(float p0, float p1, float t);
float clamp(float x);
vec3 clamp(vec3 v);

float ShadowCalculation(vec4 fragPosLightSpace, float ambient);

void main()
{
    // out_color = vec4(0, 0, 0, 1); return; // DEBUG
    bool isCore = fs_texCoords[0] >= 0;
    mat4 MVP = projection * view * model;
    MVP = mat4(1.f); // not ready

    // Textures 
    // --------
    // map 0 to 1 to 0.388 to 0.611
    vec2 texCoords = vec2(fs_texCoords[0], lerp(0.05f, 0.95f, fs_texCoords[1]));
    //vec2 texCoords = vec2(fs_texCoords[0], fs_texCoords[1]);
    vec4 heightTex = texture(u_heightTexture, texCoords);
    vec4 normalTex = texture(u_normalTexture, texCoords);
    vec4 alphaTex = texture(u_alphaTexture, texCoords);

    //isCore = false; // debug

    bool disable = false;

    // Height map
    // ----------
    float height = isCore ? heightTex[0] : clamp(fs_height / u_yarn_radius);

    // Normal map
    // ----------
    vec3 normal = isCore ? vec3(vec4(normalTex.rgb, 1.f)) : fs_normal;
    if (normal.x < 0.01f && normal.y < 0.01f && normal.z < 0.01f)
        disable = true; // TODO: doesn't work

    // Alpha channel
    // -------------
    float alpha = isCore ? alphaTex[0] : clamp(fs_alpha);
    alpha = 1.f; // TODO: not ready

    // ambient occlusion
    float ambient = lerp(0.1f, 0.4f, height);
    //ambient = 0.3f;

    // Lambertian lighting
    // -------------------
    vec3 color = objectColor.rgb;
    vec3 lightColor = vec3(1.f);
    // diffuse
    vec3 lightDir = normalize(light_pos - fs_fragPos.xyz);
    float diff = max(dot(lightDir, normal), ambient);
    vec3 diffuse = (diff - 0.3f) * lightColor;
    // specular
    vec3 viewDir = normalize(view_pos - fs_fragPos.xyz);
    float spec = 0.f;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.f), 64.0);
    vec3 specular = spec * lightColor;
    specular = vec3(0.f); // disable spec
    // calculate shadow
    float shadow = ShadowCalculation(fs_depthPos, 0.3f);
    //shadow = 0.f;
    vec3 lighting = clamp((1.f - shadow) * (ambient + diffuse + specular) * color);

    out_color = length(height) > 0.05f ? vec4(lighting, 1) : vec4(0, 0, 0, 0);
    //out_color = vec4(1 - height, 1 - height, 1 - height, 1);
}

float lerp(float p0, float p1, float t)
{
    return (1 - t) * p0 + t * p1;
}

float clamp(float x)
{
    return x > 1 ? 1 : x < 0 ? 0 : x;
}

vec3 clamp(vec3 v)
{
    return vec3(clamp(v.x), clamp(v.y), clamp(v.z));
}

float ShadowCalculation(vec4 fragPosLightSpace, float ambient)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(u_shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow

    float shadow = 0.0;
    float bias = u_yarn_radius;
    vec2 texelSize = 1.0 / textureSize(u_shadowMap, 0);

    // percentage-closer filtering (PCF)
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    return shadow / 9.0;
}