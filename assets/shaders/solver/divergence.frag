#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_div;

in vec2 C, L, R, T, B;
layout (binding = 0) uniform sampler2D u_velocity;

//
vec2 vel(vec2 uv)
{
    vec2 mult = vec2(1.0, 1.0);
    if (uv.x < 0.0) { uv.x = 0.0; mult.x = -1.0; }
    if (uv.x > 1.0) { uv.x = 1.0; mult.x = -1.0; }
    if (uv.y < 0.0) { uv.y = 0.0; mult.y = -1.0; }
    if (uv.y > 1.0) { uv.y = 1.0; mult.y = -1.0; }
    return mult * texture(u_velocity, uv).xy;
}

//
void main()
{
    float l = vel(L).x;
    float r = vel(R).x;
    float t = vel(T).y;
    float b = vel(B).y;
    float div = 0.5 * (r - l + t - b);
    out_div = vec4(div, 0.0, 0.0, 1.0);

}