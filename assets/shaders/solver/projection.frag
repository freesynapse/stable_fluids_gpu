#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_velocity;

in vec2 C, L, R, T, B;
layout (binding = 0) uniform sampler2D u_pressure;
layout (binding = 1) uniform sampler2D u_velocity;

#define boundary(uv) min(max(uv, 0.0), 1.0)

//
void main()
{
    float l = texture(u_pressure, boundary(L)).x;
    float r = texture(u_pressure, boundary(R)).x;
    float t = texture(u_pressure, boundary(T)).x;
    float b = texture(u_pressure, boundary(B)).x;
    
    vec2 velocity = texture(u_velocity, C).xy;
    velocity.xy -= vec2(r - l, t - b);
    
    out_velocity = vec4(velocity, 0.0, 1.0);

}