#version 430 core

layout (triangles) in;
layout (triangle_strip) out;
layout (max_vertices = 3) out;

out vec3 TF_VPOS;
out float TF_VID;

in VS_OUT
{
    vec4 position;
    int vid;
} gs_in[];

void main(void)
{
    vec3 vertex0 = gs_in[0].position.xyz; 
    vec3 vertex1 = gs_in[1].position.xyz;    
    vec3 vertex2 = gs_in[2].position.xyz;

    TF_VPOS = vertex0;
    TF_VID = gs_in[0].vid;
    EmitVertex();
    
    TF_VPOS = vertex1;
    TF_VID = gs_in[1].vid;
    EmitVertex();
    
    TF_VPOS = vertex2;
    TF_VID = gs_in[2].vid;
    EmitVertex();

    EndPrimitive();
}