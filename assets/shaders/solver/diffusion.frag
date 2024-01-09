#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_velocity;

in vec2 C, L, R, T, B;

layout (binding = 0) uniform sampler2D u_velocity;
layout (binding = 1) uniform sampler2D u_old_velocity;
uniform float u_alpha;
uniform float u_beta;

//
vec2 v(vec2 tx)
{
    vec2 vel = texture(u_velocity, tx).xy;

    if(tx.x < 0.0 || tx.x > 1.0) vel.x = -vel.x;
    if(tx.y < 0.0 || tx.y > 1.0) vel.y = -vel.y;

    return vel;
}

//
void main()
{
    vec2 v0 = texture(u_old_velocity, C).xy;
    vec2 v1 = (v(L) + v(T) + v(R) + v(B) + u_alpha * v0) / (u_beta);
    out_velocity = vec4(v1, 0.0, 1.0);

}