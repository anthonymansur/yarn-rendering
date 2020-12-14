#version 410 core

//uniform float u_yarn_radius; 
//in float height;
//in vec2 normal; 
//in float u;
//in float v;

out vec4 frag_colour;

float clamp(float x)
{
    return x > 1 ? 1 : x < 0 ? 0 : x;
}

void main()
{
    frag_colour = vec4(1, 1, 1, 1);
    //frag_colour = vec4(u, 0, v, 1); // fix the u coordinate
    //frag_colour = vec4(clamp(1.f - height), clamp(1.f - height), clamp(1.f - height), 1f);

   // float heightColor = 0.5 + (1 - height) / 2.f;
    //frag_colour = vec4(0.5 + (1 - height) / 2.f, 0.5 + (1 - height) / 2.f, 0.5 + (1 - height) / 2.f, 1f); // height map
    //frag_colour = vec4(0.5, 0.5 + normal[0] / 2.f, 0.5 + (1 - normal[1]) / 2.f, 1f); // 2D surface normal map
    //frag_colour = vec4(1, 0.8, 0.58, (0.5 - abs(heightColor * 0.5)) + 0.5); // alpha channel

    /* deprecated */
    //frag_colour = vec4(clamp(1.f - height), clamp(1.f - height), clamp(1.f - height), 1f);
}
