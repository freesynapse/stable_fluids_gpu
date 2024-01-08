#type VERTEX_SHADER
#version 450 core

layout (location = 0) in vec2 a_position;
layout (location = 4) in vec2 a_uv;

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

layout (binding = 0) uniform sampler2D u_velocity;
layout (binding = 1) uniform sampler2D u_noise;
uniform int u_N;
uniform float u_trace_time;

//
vec2 v(vec2 x)
{    
    vec2 vel = texture(u_velocity, x).xy;

    if(x.x < 0.0 || x.x > 1.0) vel.x = -vel.x;
    if(x.y < 0.0 || x.y > 1.0) vel.y = -vel.y;

    return vel;
}

//
void main()
{
    float dt = u_trace_time / u_N;
    float avg_noise = texture(u_noise, C).x;

    // Euler integrate forward
    vec2 x = C;
    for (int i = 0; i < u_N; i++)
    {
        x = x + dt * v(x);
        avg_noise += texture(u_noise, x).x;
    }

    // Euler integrate backward
    x = C;
    for (int i = 0; i < u_N; i++)
    {
        x = x - dt * v(x);
        avg_noise += texture(u_noise, x).x;
    }

    // normalize kernel
    avg_noise /= (2.0 * u_N + 1.0);

    // contrast
    const float contrast = 6.0;
    avg_noise = ((avg_noise - 0.5) * contrast) + 0.5;

    //
    out_color = vec4(vec3(clamp(avg_noise, 0.0, 1.0)), 1.0);

}
