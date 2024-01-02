#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_force;

in vec2 C;

uniform float u_ar;
uniform vec2 u_point;
uniform float u_radius;
layout (binding = 0) uniform sampler2D u_velocity;

//
void main()
{
    vec2 p = C - u_point.xy;
    p.x *= u_ar;
    float splat = exp(-dot(p, p) / u_radius);
    vec2 base_vel = texture(u_velocity, C).xy;
    out_force = vec4(base_vel.x+2.0*splat, base_vel.y, 0.0, 1.0);

}