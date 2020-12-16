// https://blog.tammearu.eu/posts/gllines/ 
#version 410 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

uniform float u_yarn_radius; 
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camera_pos;

in float[] isCore;
in vec3[] prevPosition;
in vec3[] nextPosition;

out float height;

out vec2 normal; // 2D surface normal

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
    mat4 MVP = projection * view * model;
    float zoomFactor = 1;
    float yarn_radius = u_yarn_radius / 2.f;

    vec3 start = gl_in[0].gl_Position.xyz;
    vec3 end = gl_in[1].gl_Position.xyz;

    vec3 dir = normalize(0.5f * (vec4(start, 1) + vec4(end, 1)) - vec4(camera_pos, 1)).xyz;

    // you have the right idea but it's not working properly. think harder
    float u = abs(dot(normalize(dir), vec3(0, 0, 1)));

    vec3 _avg1 = end - start;
    vec3 avg1 = vec3(_avg1.x, u * (abs(_avg1.y) + abs(_avg1.z)), (1 - u) * (abs(_avg1.y) + abs(_avg1.z)));

    float width = isCore[0] > 0.5 ? 0.03 : 0.002;
    vec3 lhs = cross(normalize(avg1), dir); // second argument is plane normal, in this case lines are on XY plane
    vec3 prev = prevPosition[0];
    vec3 next = nextPosition[1];

    vec3 _avg2 = start - prev;
    vec3 _avg3 = start - end;
    vec3 _avg4 = end - next;

    vec3 avg2 = vec3(_avg2.x, u * (abs(_avg2.y) + abs(_avg2.z)), (1 - u) * (abs(_avg2.y) + abs(_avg2.z)));
    vec3 avg3 = vec3(_avg3.x, u * (abs(_avg3.y) + abs(_avg3.z)), (1 - u) * (abs(_avg3.y) + abs(_avg3.z)));
    vec3 avg4 = vec3(_avg4.x, u * (abs(_avg4.y) + abs(_avg4.z)), (1 - u) * (abs(_avg4.y) + abs(_avg4.z)));

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


    // determine position, height, normal for each vertex
    gl_Position = MVP * vec4(start+startLhs, zoomFactor);
    height = ((start.z / 2.f) + (yarn_radius / 4.f)) / (yarn_radius / 2.f); // 0=(yarn_radius/2.f), 1=(yarn_radius/2.f);
    EmitVertex();

    gl_Position = MVP * vec4(start-startLhs, zoomFactor);
    height = ((start.z / 2.f) + (yarn_radius / 4.f)) / (yarn_radius / 2.f); 
    EmitVertex();

    gl_Position = MVP * vec4(end+endLhs, zoomFactor);
    height = ((end.z / 2.f) + (yarn_radius / 4.f)) / (yarn_radius / 2.f); 
    EmitVertex();

    gl_Position = MVP * vec4(end-endLhs, zoomFactor);
    height = ((end.z / 2.f) + (yarn_radius / 4.f)) / (yarn_radius / 2.f); 
    EmitVertex();

    EndPrimitive();
}