#type FRAGMENT_SHADER
#version 450 core

layout(location = 0) out vec4 color;

in vec2 C;

vec2 f(vec2 C)
{
    vec2 c = C - 0.5;
    vec2 r = vec2(c.y, -c.x);
    // vec2 r = vec2(0.05, 1.0);
    return r;
    
}

void main()
{
    color = vec4(f(C), 0.0, 1.0);

}
