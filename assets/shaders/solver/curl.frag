#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_curl;

in vec2 C, L, R, T, B;
layout (binding = 0) uniform sampler2D u_velocity;

//
void main()
{
    float l = texture(u_velocity, L).y;
    float r = texture(u_velocity, R).y;
    float t = texture(u_velocity, T).x;
    float b = texture(u_velocity, B).x;
    
    float curl = (r - l) - (t - b);
    out_curl = vec4(curl, 0.0, 0.0, 1.0);

}