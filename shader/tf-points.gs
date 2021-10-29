#version 430 core

layout (points) in;
layout (points) out;
layout (max_vertices = 1) out;

in VS_OUT
{
    vec4 position;
    int vid;
} gs_in[];

out GS_OUT
{
    vec3 TF_VPOS;
};

void main(void)
{
    vec3 vertex0 = gs_in[0].position.xyz; 

    TF_VPOS = vertex0;
    EmitVertex();
    
    EndPrimitive();
}