#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_pressure;

in vec2 C, L, R, T, B;
layout (binding = 0) uniform sampler2D u_divergence;
layout (binding = 1) uniform sampler2D u_pressure;

#define boundary(uv) min(max(uv, 0.0), 1.0)

//
void main()
{
    float l = texture(u_pressure, boundary(L)).x;
    float r = texture(u_pressure, boundary(R)).x;
    float t = texture(u_pressure, boundary(T)).x;
    float b = texture(u_pressure, boundary(B)).x;
    float div = texture(u_divergence, C).x;
    float pressure = (l + r + b + t - div) * 0.25;
    out_pressure = vec4(pressure, 0.0, 0.0, 1.0);

}