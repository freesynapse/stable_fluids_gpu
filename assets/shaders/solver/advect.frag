#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_quantity;

in vec2 C;
layout (binding = 0) uniform sampler2D u_velocity;
layout (binding = 1) uniform sampler2D u_quantity;
uniform float u_dissipation;
uniform float u_dt;

//
void main()
{
    vec2 coord = C - u_dt * texture(u_velocity, C).xy;
    out_quantity = u_dissipation * texture(u_quantity, coord);
    out_quantity.a = 1.0;

}