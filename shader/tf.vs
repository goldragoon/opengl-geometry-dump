#version 410 core

// Per-vertex inputs
layout (location = 0) in vec4 position;

out VS_OUT
{
    vec4 position;
    int vid;
} vs_out;

void main(void)
{
    vs_out.position = position;
    vs_out.vid = gl_VertexID;
}
