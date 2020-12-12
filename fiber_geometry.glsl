// https://blog.tammearu.eu/posts/gllines/ 
#version 410 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;
#define pi 3.14159265358979323846f

uniform float u_yarn_radius; 
uniform float u_alpha;

in float[] isCore;
in vec3[] prevPosition;
in vec3[] nextPosition;
in vec2[] textureParams;

out float height;
out vec2 normal; // 2D surface normal

out float u;
out float v;

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

vec3 lerp(vec3 a, vec3 b, float i)
{
    if (i > 1.f || i < 0.f)
        return vec3(0,0,0);
    return vec3(a.x * (1-i) + b.x * i, a.y * (1-i) + b.y * i, a.z * (1-i) + b.z * i);
}

void main()
{
    float zoomFactor = 0.25;
    float yarn_radius = u_yarn_radius / 2.f;

    vec3 start = gl_in[0].gl_Position.xyz;
    vec3 end = gl_in[1].gl_Position.xyz;

    vec3 avg1 = vec3((end - start).x, abs((end - start).y) + abs((end - start).z), 0.f);

    //float width = isCore[0] > 0.5 ? 0.03 : 0.002; // core fiber width is a fn of maximum fiber displacement
    float width = isCore[0] > 0.5 ? 0.00 : 0.002;
    vec3 lhs = cross(normalize(avg1), vec3(0.0, 0.0, -1.0)); // second argument is plane normal, in this case lines are on XY plane
    vec3 prev = prevPosition[0];
    vec3 next = nextPosition[1];

    vec3 avg2 = vec3((start-prev).x, abs((start-prev).y) + abs((start-prev).z), 0.f);
    vec3 avg3 = vec3((start-end).x, abs((start-end).y) + abs((start-end).z), 0.f);
    vec3 avg4 = vec3((end-next).x, abs((end-next).y) + abs((end-next).z), 0.f);

    bool colStart = length(avg2) < EPSILON;
    bool colEnd = length(avg4) < EPSILON;

    vec3 a = normalize(avg2);
    vec3 b = normalize(avg3);
    vec3 c = (a+b)*0.5f;
    vec3 startLhs = normalize(c) * sign(dot(c, lhs));
    a = normalize(avg1);
    b = normalize(avg4);
    c = (a+b)*0.5f;
    vec3 endLhs = normalize(c) * sign(dot(c, lhs));

    if(colStart)
        startLhs = lhs;
    if(colEnd)
        endLhs = lhs;

    startLhs *= width / 2.f;
    endLhs *= width / 2.f;

    float theta = textureParams[0][0];
    float ply_alpha = textureParams[0][1];
    vec3 view = vec3(0, 0, 1.f);
    vec3 norm = vec3(0, start.y, start.z);
    float psi = acos(dot(view, norm));

    // determine position, height, normal for each vertex
    gl_Position = vec4(start+startLhs, zoomFactor);
    height = ((start.z / 2.f) + (yarn_radius / 4.f)) / (yarn_radius / 2.f); // 0=(yarn_radius/2.f), 1=(yarn_radius/2.f);
    u = (1/(2*pi)) * (((theta * pow(ply_alpha,2) * u_alpha) / (ply_alpha - u_alpha)) + (psi / 2.f)); 
    v = 1;
    EmitVertex();

    gl_Position = vec4(start-startLhs, zoomFactor);
    height = ((start.z / 2.f) + (yarn_radius / 4.f)) / (yarn_radius / 2.f); 
    u = (1/(2*pi)) * (((theta * pow(ply_alpha,2) * u_alpha) / (ply_alpha - u_alpha)) + (psi / 2.f)); 
    v = 0;
    EmitVertex();

    theta = textureParams[1][0];
    ply_alpha = textureParams[1][1];
    norm = vec3(0, end.y, end.z);
    psi = acos(dot(view, norm));

    gl_Position = vec4(end+endLhs, zoomFactor);
    height = ((end.z / 2.f) + (yarn_radius / 4.f)) / (yarn_radius / 2.f); 
    u = (1/(2*pi)) * (((theta * pow(ply_alpha,2) * u_alpha) / (ply_alpha - u_alpha)) + (psi / 2.f)); 
    v = 1;
    EmitVertex();

    gl_Position = vec4(end-endLhs, zoomFactor);
    height = ((end.z / 2.f) + (yarn_radius / 4.f)) / (yarn_radius / 2.f); 
    u = (1/(2*pi)) * (((theta * pow(ply_alpha,2) * u_alpha) / (ply_alpha - u_alpha)) + (psi / 2.f)); 
    v = 0;
    EmitVertex();

    EndPrimitive();
}