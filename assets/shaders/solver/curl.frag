#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_curl;

in vec2 L, R, T, B;
layout (binding = 0) uniform sampler2D u_velocity;
uniform float u_half_inv_dx;

//
vec2 v(vec2 uv)
{
    vec2 vel = texture(u_velocity, uv).xy;

    if(uv.x < 0.0 || uv.x > 1.0) vel.x = -vel.x;
    if(uv.y < 0.0 || uv.y > 1.0) vel.y = -vel.y;

    return vel;

}

//
void main()
{
    // float l = vel(L).y;
    // float r = vel(R).y;
    // float t = vel(T).x;
    // float b = vel(B).x;
    
    // float curl = 0.5 * (r - l - t + b);
    float curl = u_half_inv_dx * (v(R).y - v(L).y - v(T).x + v(B).x);
    out_curl = vec4(curl, 0.0, 0.0, 1.0);

}
