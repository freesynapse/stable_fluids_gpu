#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec2 a_position;
layout(location = 4) in vec2 a_uv;

out vec2 C;

//
void main()
{
    C = a_uv;
    gl_Position = vec4(a_position, 0.0, 1.0);

}


#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_color;

in vec2 C;

layout (binding = 0) uniform sampler2D u_field;
layout (binding = 1) uniform sampler1D u_colormap;
uniform vec2 u_range = vec2(0, 1);

//
void main()
{
    float s = texture(u_field, C).x;
    out_color = texture(u_colormap, (s - u_range.x) / (u_range.y - u_range.x));

}
