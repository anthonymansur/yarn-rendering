// https://blog.tammearu.eu/posts/gllines/ 
#version 410 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

// fiber parameters
uniform float u_yarn_radius; 
uniform float u_ellipse_short;
uniform float u_r_max;

// other uniforms
uniform mat4 model;

in vec3[] prevPosition;
in vec3[] nextPosition;
in vec3[] geo_normal;

out float fs_height; // height map
out vec3 fs_normal; // 2D surface normal
out float fs_alpha; // alpha channel 

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
vec3 lerp(vec3 p0, vec3 p1, float t);
vec3 slerp(vec3 p0, vec3 p1, float t); // DISABLED DUE TO FADING BUG

// thickens the isolines
void main()
{
    float zoomFactor = .090f;
    float yarn_radius = u_yarn_radius / 2.f;
    float lineHeight = 0.001;

    // four control points
    vec3 prev = prevPosition[0];
    vec3 start = gl_in[0].gl_Position.xyz;
    vec3 end = gl_in[1].gl_Position.xyz;
    vec3 next = nextPosition[1];

    // have strips face the camera
   vec3 direction = vec3(0, 0, -1) - 0.5f * (end - start); // hardcoded
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

    // determine position, height, normal for each vertex
    float maxDistance = (u_yarn_radius / 2.f) + u_ellipse_short * u_r_max * (2/3.f);
    float maxTransparency = 0.7f;

    gl_Position = vec4((model * vec4(start+startHeightDir, 1)).xyz, zoomFactor);
    fs_height = start.z / (2 * maxDistance) + 0.5;
    fs_normal = geo_normal[0] / 2.f + 0.5f;
    fs_alpha = 1 - (abs(start.z)/maxDistance) * (1 - maxTransparency);
    EmitVertex();

    gl_Position = vec4((model * vec4(start-startHeightDir, 1)).xyz, zoomFactor);
    fs_height = start.z / (2 * maxDistance) + 0.5;
    fs_normal = geo_normal[0] / 2.f + 0.5f;
    fs_alpha = 1 - (abs(start.z)/maxDistance) * (1 - maxTransparency);
    EmitVertex();

    gl_Position = vec4((model * vec4(end+endHeightDir, 1)).xyz, zoomFactor);
    fs_height = end.z / (2 * maxDistance) + 0.5;
    fs_normal = geo_normal[1] / 2.f + 0.5f;
    fs_alpha = 1 - (abs(end.z)/maxDistance) * (1 - maxTransparency);
    EmitVertex();

    gl_Position = vec4((model * vec4(end-endHeightDir, 1)).xyz, zoomFactor);
    fs_height = end.z / (2 * maxDistance) + 0.5;
    fs_normal = geo_normal[1] / 2.f + 0.5f;
    fs_alpha = 1 - (abs(end.z)/maxDistance) * (1 - maxTransparency);
    EmitVertex();

    EndPrimitive();
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