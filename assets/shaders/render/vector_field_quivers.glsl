#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec2 a_position;

out vec2 v_rot;
out float v_output_dot;

uniform float u_arrow_size;
layout (binding = 0) uniform sampler2D u_vector_field;

#define EPSILON 0.01

//
void main()
{
    vec2 uv = (a_position + 1.0) * 0.5;
    vec2 data = texture(u_vector_field, uv).xy;

    v_output_dot = (length(data) < EPSILON ? 1.0 : 0.0);

    // TODO : check if these two can be trivially compressed into one
    // -- don't think so, since atan (=atan2) calculates the angle between the positive
    // x-axis and the vector, and cos and sin of this angle gives the unit circle vector
    // coordinates
    float orientation = atan(data.y, data.x);
    v_rot = vec2(cos(orientation), sin(orientation));
    
    gl_Position = vec4(a_position, 0.0, 1.0);
    gl_PointSize = u_arrow_size;

}


#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 out_color;

in vec2 v_rot;
in float v_output_dot;

uniform vec4 u_arrow_color = vec4(1.0);
uniform float u_linewidth = 0.08;
uniform float u_antialias = 0.01;

// Fill function for arrows
vec4 filled(float distance,     // Signed distance to line
            float linewidth,    // Stroke line width
            float antialias,    // Stroke antialiased area
            vec4 fill)          // Fill color
{
    float t = linewidth / 2.0 - antialias;
    float signed_distance = distance;
    float border_distance = abs(signed_distance) - t;
    float alpha = border_distance / antialias;
    alpha = exp(-alpha * alpha);
    if( border_distance < 0.0 )
        return fill;
    else if( signed_distance < 0.0 )
        return fill;
    else
        return vec4(fill.rgb, alpha * fill.a);
}

//
float line_distance(vec2 p, vec2 p1, vec2 p2)
{
    vec2 center = (p1 + p2) * 0.5;
    float len = length(p2 - p1);
    vec2 dir = (p2 - p1) / len;
    vec2 rel_p = p - center;
    return dot(rel_p, vec2(dir.y, -dir.x));
}

//
float segment_distance(vec2 p, vec2 p1, vec2 p2)
{
    vec2 center = (p1 + p2) * 0.5;
    float len = length(p2 - p1);
    vec2 dir = (p2 - p1) / len;
    vec2 rel_p = p - center;
    float dist1 = abs(dot(rel_p, vec2(dir.y, -dir.x)));
    float dist2 = abs(dot(rel_p, dir)) - 0.5*len;
    return max(dist1, dist2);
}

//
float arrow_triangle(vec2 texcoord, float body, float head, float height, 
                    float linewidth, float antialias)
{
    float w = linewidth/2.0 + antialias;
    vec2 start = -vec2(body/2.0, 0.0);
    vec2 end = vec2(body/2.0, 0.0);
    
    // Head : 3 lines
    float d1 = line_distance(texcoord,
    end, end - head*vec2(+1.0,-height));
    float d2 = line_distance(texcoord,
    end - head*vec2(+1.0,+height), end);
    float d3 = texcoord.x - end.x + head;
    
    // Body : 1 segment
    float d4 = segment_distance(texcoord,
    start, end - vec2(linewidth,0.0));
    float d = min(max(max(d1, d2), -d3), d4);
    return d;
}

//
void main()
{
    vec2 p = gl_PointCoord.xy - vec2(0.5);

    if (v_output_dot == 0.0)
    {
        p = vec2(v_rot.x * p.x - v_rot.y * p.y,
                v_rot.y * p.x + v_rot.x * p.y);

        float d = arrow_triangle(p, 0.6, 0.1, 0.6, u_linewidth, u_antialias);

        out_color = filled(d, u_linewidth, u_antialias, u_arrow_color);
    }
    else
    {
        vec4 color = vec4(1.0, 1.0, 1.0, 0.0);
        if (length(p) < 0.03)
            color.a = 1.0;

        out_color = color;
    }

}            
