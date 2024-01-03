#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_velocity;

in vec2 C;

uniform float u_ar;
uniform vec2 u_point;
uniform vec2 u_direction;
uniform float u_force;
uniform float u_radius;
layout (binding = 0) uniform sampler2D u_velocity;

//
void main()
{
    vec2 p = C - u_point.xy;
    p.x *= u_ar;
    float splat = exp(-dot(p, p) / u_radius);
    vec2 base_vel = texture(u_velocity, C).xy;
    vec2 force_vel = base_vel + u_force * splat * u_direction;
    out_velocity = vec4(force_vel, 0.0, 1.0);

}