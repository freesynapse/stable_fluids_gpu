#type VERTEX_SHADER
#version 450 core

layout (location = 0) in vec2 a_position;
layout (location = 4) in vec2 a_uv;

uniform vec2 u_tx_size;

out vec2 C, L, R, T, B;

//
void main()
{
    C = a_uv;
    L = a_uv - vec2(u_tx_size.x, 0.0);
    R = a_uv + vec2(u_tx_size.x, 0.0);
    B = a_uv - vec2(0.0, u_tx_size.y);
    T = a_uv + vec2(0.0, u_tx_size.y);

    gl_Position = vec4(a_position, 0.0, 1.0);

}
