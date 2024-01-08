#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_div;

in vec2 C, L, R, T, B;
layout (binding = 0) uniform sampler2D u_velocity;
uniform float u_half_inv_dx;

//
vec2 vel(vec2 uv)
{
    vec2 v = texture(u_velocity, uv).xy;

    if(uv.x < 0.0 || uv.x > 1.0) v.x = -v.x;
    if(uv.y < 0.0 || uv.y > 1.0) v.y = -v.y;

    return v;

}

//
void main()
{
    float l = vel(L).x;
    float r = vel(R).x;
    float t = vel(T).y;
    float b = vel(B).y;
    
    float div = u_half_inv_dx * (r - l + t - b);    //
    out_div = vec4(div, 0.0, 0.0, 1.0);

}