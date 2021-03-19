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

//uniform sampler2D u_shadowMap;

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

float ShadowCalculation(vec4 fragPosLightSpace);

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
    vec3 color = objectColor.rgb;
    vec3 lightColor = vec3(1.f);
    // ambient
    vec3 ambient = 0.4f * color;
    // diffuse
    vec3 lightDir = normalize(light_pos - fs_fragPos.xyz);
    float diff = max(dot(lightDir, normal), 0.f);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(view_pos - fs_fragPos.xyz);
    float spec = 0.f;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.f), 64.0);
    vec3 specular = spec * lightColor;
    // calculate shadow
    float shadow = ShadowCalculation(fs_depthPos);
    vec3 lighting = (ambient + (1.f - shadow) * (diffuse + specular)) * color;

    out_color = vec4(lighting, length(lighting) < .1 ? 0 : fs_disable > 0.5 ? 0 : alpha);  

    //out_color = vec4(fract(fs_texCoords[0]), 0, fract(fs_texCoords[1]), 1);

    //out_color = vec4(fs_normal, 1);  // debug
}

float lerp(float p0, float p1, float t)
{
    return (1 - t) * p0 + t * p1;
}

float clamp(float x)
{
    return x > 1 ? 1 : x < 0 ? 0 : x;
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    return 0.f;
//    // perform perspective divide
//    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//    // transform to [0,1] range
//    projCoords = projCoords * 0.5 + 0.5;
//    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
//    float closestDepth = texture(u_shadowMap, projCoords.xy).r; 
//    // get depth of current fragment from light's perspective
//    float currentDepth = projCoords.z;
//    // check whether current frag pos is in shadow
//    float bias = 0.005;
//    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
//
//    return shadow;
}