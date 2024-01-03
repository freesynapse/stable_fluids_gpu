#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_color;

in vec2 C;

uniform float u_value;
layout (binding = 0) uniform sampler2D u_field;

//
void main()
{
    // out_color = vec4(u_value * texture(u_field, C).xyz, 1.0);
    out_color = vec4(vec3(u_value), 1.0);

}