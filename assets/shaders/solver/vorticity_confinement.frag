#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_velocity;

in vec2 C, L, R, T, B;
uniform float u_confinement;
uniform float u_Dissipation;
uniform float u_dt;
layout (binding = 0) uniform sampler2D u_velocity;
layout (binding = 1) uniform sampler2D u_curl;

//
void main()
{
    float c = texture(u_curl, C).x;
    float l = texture(u_curl, L).x;
    float r = texture(u_curl, R).x;
    float t = texture(u_curl, T).x;
    float b = texture(u_curl, B).x;
    
    vec2 force = 0.5 * vec2(abs(t) - abs(b), abs(r) - abs(l));
    force /= length(force) + 0.0001;
    force *= u_confinement * C;
    force.y *= -1.0;
    force *= u_Dissipation;

    vec2 vel = texture(u_velocity, C).xy;
    out_velocity = vec4(vel + force * u_dt, 0.0, 1.0);
    
}
