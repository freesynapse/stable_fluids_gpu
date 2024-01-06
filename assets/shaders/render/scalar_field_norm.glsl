#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec2 a_position;
layout(location = 4) in vec2 a_uv;

uniform vec2 u_tx_size;

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
uniform vec2 u_range;
uniform vec2 u_inv_field_range_lim;
uniform int u_rgb = 0;

vec2 range_lim_inv = 1.0 / u_range;

//
void main()
{
    float val = texture(u_field, C).x;
    vec3 color = vec3(0.0);
    if (u_rgb == 0)
    {
        val = (val - u_range.x) * range_lim_inv.y;
        color = vec3(val);
    }
    else
    {
        if (val < 0.0)
        {
            val *= range_lim_inv.x;
            color = vec3(0.0, 0.0, val);
        }
        else
        {
            val *= range_lim_inv.y;
            color = vec3(val, 0.0, 0.0);
        }

    }

    out_color = vec4(color, 1.0);

}
