// https://blog.tammearu.eu/posts/gllines/ 
#version 410 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

uniform float u_yarn_radius; 

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
    float zoomFactor = .25;

    vec3 start = gl_in[0].gl_Position.xyz;
    vec3 end = gl_in[1].gl_Position.xyz;

    float width = 0.001;
    vec3 lhs = width * cross(normalize(end - start), vec3(0.0, 0.0, -1.0)); // second argument is plane normal, in this case lines are on XY plane

    // determine position, height, normal for each vertex
    gl_Position = vec4(start+lhs, zoomFactor);
    height = ((start.z / 2.f) + (u_yarn_radius / 4.f)) / (u_yarn_radius / 2.f); // 0=(u_yarn_radius/2.f), 1=(u_yarn_radius/2.f);
    normal = vec2(((start.y / 2.f) + (u_yarn_radius / 4.f)) / (u_yarn_radius / 2.f), height);
    EmitVertex();

    gl_Position = vec4(start-lhs, zoomFactor);
    height = ((start.z / 2.f) + (u_yarn_radius / 4.f)) / (u_yarn_radius / 2.f); // 0=(u_yarn_radius/2.f), 1=(u_yarn_radius/2.f);
    normal = vec2(((start.y / 2.f) + (u_yarn_radius / 4.f)) / (u_yarn_radius / 2.f), height);
    EmitVertex();

    gl_Position = vec4(end+lhs, zoomFactor);
    height = ((end.z / 2.f) + (u_yarn_radius / 4.f)) / (u_yarn_radius / 2.f); // 0=(u_yarn_radius/2.f), 1=(u_yarn_radius/2.f);
    normal = vec2(((end.y / 2.f) + (u_yarn_radius / 4.f)) / (u_yarn_radius / 2.f), height);
    EmitVertex();

    gl_Position = vec4(end-lhs, zoomFactor);
    height = ((end.z / 2.f) + (u_yarn_radius / 4.f)) / (u_yarn_radius / 2.f); // 0=(u_yarn_radius/2.f), 1=(u_yarn_radius/2.f);
    normal = vec2(((end.y / 2.f) + (u_yarn_radius / 4.f)) / (u_yarn_radius / 2.f), height);
    EmitVertex();

    EndPrimitive();
}