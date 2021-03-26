// https://blog.tammearu.eu/posts/gllines/ 
#version 410 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

// fiber parameters
uniform float u_yarn_radius; 
uniform float u_ellipse_long;
uniform float u_r_max;

// other uniforms
uniform vec3 camera_pos;
uniform mat4 light_transform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in float[] isCore;
in float[] disable;
in vec3[] prevPosition;
in vec3[] nextPosition;
in vec3[] geo_normal;
in float[] geo_height;
in vec2[] geo_texCoords;
in float[] tes_scaleFactor;

out vec4 fs_fragPos;
out vec4 fs_depthPos;
out float fs_height; // height map
out vec3 fs_normal; // 2D surface normal
out float fs_alpha; // alpha channel 
out vec2 fs_texCoords;
out float fs_disable;

#define EPSILON 0.0001f

/*
    
    Given the center of the fiber, generate camera-facing triangle strips.

    Each triangle will be made up of the following components (32 total)
    - 4 * vec4 representing position
    - 4 * vec4 representing normal

    // NOTE: vec3 may work in here

    Given that an geometry shader can only output up to 256 vertices and 1024 components, and given that one strip has 32 components, we get
    the following limitation: only 32 triangle strips per isoline segment

    DEPRECATED: http://www.songho.ca/opengl/gl_cylinder.html 
    DEPRECATED: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-10-transparency/
*/
float lerp(float p0, float p1, float t);
vec3 lerp(vec3 p0, vec3 p1, float t);
vec3 slerp(vec3 p0, vec3 p1, float t); // DISABLED DUE TO FADING BUG

// thickens the isolines
void main()
{
    mat4 MVP = projection * view * model;
    float zoomFactor = .125f;
    float ply_diameter = u_ellipse_long;
    float lineHeight = isCore[0] > 0.5f ? lerp(1.5f * ply_diameter, ply_diameter, tes_scaleFactor[0]) : 0.001;

    //lineHeight = 0.001; // debug

    // four control points
    vec3 prev = prevPosition[0];
    vec3 start = gl_in[0].gl_Position.xyz;
    vec3 end = gl_in[1].gl_Position.xyz;
    vec3 next = nextPosition[1];

    // have strips face the camera
    vec3 direction = camera_pos - 0.5f * (end - start);
    direction = normalize(vec3(0, direction.y, direction.z));

    // the three different height vectors that can be generated given the four control points
    vec3 leftHeightDir = cross(normalize(start - prev), direction);
    vec3 heightDir = cross(normalize(end - start), direction);
    vec3 rightHeightDir = cross(normalize(next - end), direction);

    // average the height vectors
    vec3 startHeightDir = lerp(leftHeightDir, heightDir, 0.5f);
    vec3 endHeightDir = lerp(heightDir, rightHeightDir, 0.5f);

    // check if at endpoints
    if (length(start - prev) < EPSILON)
    {
        startHeightDir = heightDir;
    }
    if (length(end - next) < EPSILON)
    {
        endHeightDir = heightDir;
    }

    // keep height direction on the same side of the line segment and add line height 
    startHeightDir = 0.5f * lineHeight * normalize(startHeightDir) * sign(dot(startHeightDir, heightDir));
    endHeightDir = 0.5f * lineHeight * normalize(endHeightDir) * sign(dot(endHeightDir, heightDir));

    // determine position, height, normal, and uv coordinates for each vertex
    //float maxDistance = (ply_diameter / 2.f) + u_ellipse_short * u_r_max * (2/3.f);
    float maxTransparency = 0.7f; 

    //MVP = light_transform; // DEBUG

    gl_Position = MVP * vec4(start+startHeightDir, 1);
    fs_fragPos = model * vec4(start+startHeightDir, 1);
    fs_depthPos = light_transform * vec4(start+startHeightDir, 1);
    //fs_height = start.z / (2 * maxDistance) + 0.5;
    fs_height = geo_height[0];
    fs_normal = geo_normal[0] / 2.f + 0.5f;
    fs_disable = disable[0];
    //fs_alpha = 1 - (abs(start.z)/maxDistance) * (1 - maxTransparency);
    fs_alpha = -1;
    fs_texCoords[0] = isCore[0] > 0.5f ? geo_texCoords[0][0] : -1;
    fs_texCoords[1] = isCore[0] > 0.5f ? 0 : -1;
    EmitVertex();

    gl_Position = MVP *  vec4(start-startHeightDir, 1);
    fs_fragPos = model * vec4(start-startHeightDir, 1);
    fs_depthPos = light_transform * vec4(start-startHeightDir, 1);
    //fs_height = start.z / (2 * maxDistance) + 0.5;
    fs_height = geo_height[0];
    fs_normal = geo_normal[0] / 2.f + 0.5f;
    fs_disable = disable[0];
    //fs_alpha = 1 - (abs(start.z)/maxDistance) * (1 - maxTransparency);
    fs_alpha = 1;
    fs_texCoords[0] = isCore[0] > 0.5f ? geo_texCoords[0][0] : -1;
    fs_texCoords[1] = isCore[0] > 0.5f ? 1 : -1;
    EmitVertex();

    gl_Position = MVP * vec4(end+endHeightDir, 1);
    fs_fragPos = model * vec4(end+endHeightDir, 1);
    fs_depthPos = light_transform * vec4(end+endHeightDir, 1);
    //fs_height = end.z / (2 * maxDistance) + 0.5;
    fs_height = geo_height[1];
    fs_normal = geo_normal[1] / 2.f + 0.5f;
    fs_disable = disable[1];
    //fs_alpha = 1 - (abs(end.z)/maxDistance) * (1 - maxTransparency);
    fs_alpha = 1;
    fs_texCoords[0] = isCore[0] > 0.5f ? geo_texCoords[1][0] : -1;
    fs_texCoords[1] = isCore[1] > 0.5f ? 0 : -1;
    EmitVertex();

    gl_Position = MVP * vec4(end-endHeightDir, 1);
    fs_fragPos = model * vec4(end-endHeightDir, 1);
    fs_depthPos = light_transform * vec4(end-endHeightDir, 1);
    //fs_height = end.z / (2 * maxDistance) + 0.5;
    fs_height = geo_height[1];
    fs_normal = geo_normal[1] / 2.f + 0.5f;
    fs_disable = disable[1];
    //fs_alpha = 1 - (abs(end.z)/maxDistance) * (1 - maxTransparency);
    fs_alpha = 1;
    fs_texCoords[0] = isCore[0] > 0.5f ? geo_texCoords[1][0] : -1;
    fs_texCoords[1] = isCore[1] > 0.5f ? 1 : -1;
    EmitVertex();

    EndPrimitive();
}

float lerp(float p0, float p1, float t)
{
    return (1 - t) * p0 + t * p1;
}

vec3 lerp(vec3 p0, vec3 p1, float t)
{
    return (1 - t) * p0 + t * p1;
}

vec3 slerp(vec3 p0, vec3 p1, float t)
{
    float theta = acos(dot(normalize(p0), normalize(p1)));
    return (sin((1 - t)* theta) / sin(theta)) * p0 + (sin(t* theta) / sin(theta)) * p1;
}