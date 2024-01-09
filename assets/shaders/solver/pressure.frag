#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_pressure;

in vec2 C, L, R, T, B;

layout (binding = 0) uniform sampler2D u_divergence;
layout (binding = 1) uniform sampler2D u_pressure;
uniform float u_alpha;
uniform float u_beta;

#define p(tx) texture(u_pressure, tx).x

//
void main()
{
    float div = texture(u_divergence, C).x;
    float pressure = (p(L) + p(R) + p(T) + p(B) - u_alpha * div) / u_beta;
    out_pressure = vec4(pressure, 0.0, 0.0, 1.0);

}