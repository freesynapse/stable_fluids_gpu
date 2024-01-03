#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_density;

in vec2 C;

uniform float u_ar;
uniform vec2 u_point;
uniform float u_radius;
layout (binding = 0) uniform sampler2D u_density;

//
void main()
{
    vec2 p = C - u_point.xy;
    p.x *= u_ar;
    float splat = exp(-dot(p, p) / u_radius);
    float base = texture(u_density, C).x;
    out_density = vec4(base + 0.1 * splat, 0.0, 0.0, 1.0);

}