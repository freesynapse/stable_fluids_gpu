#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_color;

in vec2 C;

uniform float u_value;
layout (binding = 0) uniform sampler2D u_field;

//
void main()
{
    out_color = u_value * texture(u_field, C);

}