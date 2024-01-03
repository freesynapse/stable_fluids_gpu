#pragma once

#include <synapse/Renderer>
#include <synapse/Debug>
#include <synapse/API>
using namespace Syn;

#include "Quad.h"
#include "FieldFBO.h"


//
class FieldRenderer
{
public:
    FieldRenderer() { initStaticShaders(); }
    ~FieldRenderer() = default;

    //
    void renderScalarField(FieldFBO *_scalar_field, bool _normalize_range=false)
    {
        if (_scalar_field == nullptr)
            return;

        m_scalarField = _scalar_field;
        // printf("rendering field '%s'\n", m_scalarField->getName().c_str());
        Shader *shader = _normalize_range ? m_scalarFieldNorm_shader.get() : m_scalarField_shader.get();

        auto &dim = m_scalarField->getSize();
        glm::vec2 tx_size = 1.0f / glm::vec2(dim.x, dim.y);
        Quad::bind();
        shader->enable();
        m_scalarField->bindTexture(0);
        shader->setUniform1i("u_field", 0);
        shader->setUniform2fv("u_tx_size", tx_size);

        // normalize scalar field if requested
        if (_normalize_range)
        {
            auto range = m_scalarField->range();
            glm::vec2 vrange = { range.first[0], range.second[0] };
            shader->setUniform2fv("u_range", vrange);
        }
        
        Quad::render();

    }

    //
    void renderVectorField(FieldFBO *_vector_field)
    {
        if (_vector_field == nullptr)
            return;

        m_vectorField = _vector_field;
        
        auto &dim = m_vectorField->getSize();
        glm::vec2 tx_size = 1.0f / glm::vec2(dim.x, dim.y);
        Quad::bind();
        m_vectorField_shader->enable();
        m_vectorField->bindTexture(0);
        m_vectorField_shader->setUniform1i("u_field", 0);
        m_vectorField_shader->setUniform2fv("u_tx_size", tx_size);
        Quad::render();

    }

    //
    void renderVectorFieldQuivers(FieldFBO *_vector_field, 
                                  uint32_t _sampling_rate=1,
                                  bool _field_updated=false,
                                  const glm::vec2 &_vp=Renderer::getViewportF())
    {
        if (_vector_field == nullptr)
            return;

        if (_vector_field != m_vectorField_quiver)
        {
            m_vectorField_quiver = _vector_field;
            m_samplingRate = (_sampling_rate > 0 ? _sampling_rate : 1);
            m_dim = m_vectorField_quiver->getSize();
            m_texelSize = 1.0f / glm::vec2(m_dim[0], m_dim[1]);
            m_vp = _vp;
            initQuivers();
        }

        if (_field_updated)
            initQuivers();

        static auto &renderer = Renderer::get();
        
        m_vectorField_quiver_shader->enable();
        m_vectorField_quiver->bindTexture(0);
        m_vectorField_quiver_shader->setUniform1i("u_vector_field", 0);
        m_vectorField_quiver_shader->setUniform1f("u_arrow_size", m_arrowSize);

        renderer.drawArrays(m_vaoQuiver, m_arrowCount, 0, false, GL_POINTS);

    }

    // overlaods
    
    __always_inline 
    void renderScalarField(Ref<FieldFBO> _scalar_field, bool _normalize_range=false) 
    { renderScalarField(_scalar_field.get(), _normalize_range); }
    
    __always_inline 
    void renderVectorField(Ref<FieldFBO> _vector_field) 
    { renderVectorField(_vector_field.get()); }
    
    __always_inline
    void renderVectorFieldQuivers(Ref<FieldFBO> _vector_field, 
                                  uint32_t _sampling_rate=1,
                                  bool _field_updated=false,
                                  const glm::vec2 &_vp=Renderer::getViewportF())
    { renderVectorFieldQuivers(_vector_field.get(), _sampling_rate, _field_updated, _vp); }



private:
    void initStaticShaders()
    {
        std::string scalarSrc = R"(
            #type VERTEX_SHADER
            #version 450 core
            layout(location = 0) in vec2 a_position;
            layout(location = 4) in vec2 a_uv;
            uniform vec2 u_tx_size;
            out vec2 C;

            void main()
            {
                C = a_uv;
                gl_Position = vec4(a_position, 0.0, 1.0);

            }

            #type FRAGMENT_SHADER
            #version 450 core
            layout (location = 0) out vec4 out_color;
            in vec2 C;
            layout (binding = 0) uniform sampler2D u_field;

            void main()
            {
                out_color = vec4(vec3(texture(u_field, C).x), 1.0);
            }
        )";

        std::string scalarNormalizedSrc = R"(
            #type VERTEX_SHADER
            #version 450 core
            layout(location = 0) in vec2 a_position;
            layout(location = 4) in vec2 a_uv;
            uniform vec2 u_tx_size;
            out vec2 C;

            void main()
            {
                C = a_uv;
                gl_Position = vec4(a_position, 0.0, 1.0);

            }

            #type FRAGMENT_SHADER
            #version 450 core
            layout (location = 0) out vec4 out_color;
            in vec2 C;
            layout (binding = 0) uniform sampler2D u_field;
            uniform vec2 u_range;
            float max_range_inv = 1.0 / u_range.y;
            
            void main()
            {
                float val = texture(u_field, C).x;
                val = (val - u_range.x) * max_range_inv;
                out_color = vec4(vec3(val), 1.0);
            }
        )";

        std::string vectorSrc = R"(
            #type VERTEX_SHADER
            #version 450 core
            layout(location = 0) in vec2 a_position;
            layout(location = 4) in vec2 a_uv;
            uniform vec2 u_tx_size;
            out vec2 C;

            void main()
            {
                C = a_uv;
                gl_Position = vec4(a_position, 0.0, 1.0);
            }
            
            #type FRAGMENT_SHADER
            #version 450 core
            layout (location = 0) out vec4 out_color;
            in vec2 C;
            layout (binding = 0) uniform sampler2D u_field;
            void main()
            {
                out_color = vec4(texture(u_field, C).xy, 0.0, 1.0);
            }
        )";

        std::string quiverSrc = R"(
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
        )";

        m_scalarField_shader = ShaderLibrary::loadFromSrc("scalar_field_shader", scalarSrc);
        m_scalarFieldNorm_shader = ShaderLibrary::loadFromSrc("scalar_field_norm_shader", scalarNormalizedSrc);
        m_vectorField_shader = ShaderLibrary::loadFromSrc("vector_field_shader", vectorSrc);
        m_vectorField_quiver_shader = ShaderLibrary::loadFromSrc("vector_field_quiver_shader", quiverSrc);

    }

    //
    void initQuivers()
    {
        // Timer t(__func__, true);   
        int new_x = m_dim.x / m_samplingRate;
        int new_y = m_dim.y / m_samplingRate;
        
        m_arrowCount = new_x * new_y;
        glm::vec2 vs[m_arrowCount];

        // set vertices as points on a grid, at grid square centers
        glm::vec2 tx_sz_ndc = 2.0f / glm::vec2(new_x, new_y);
        glm::vec2 center_offset = tx_sz_ndc * 0.5f;
        glm::vec2 coord = { -1.0f, -1.0f };
        //
        int idx = 0;
        for (int y = 0; y < new_y; y++)
        {
            coord.x = -1.0f;
            for (int x = 0; x < new_x; x++)
            {
                vs[idx++] = coord + center_offset;
                coord.x += tx_sz_ndc.x;

            }
            coord.y += tx_sz_ndc.y;

        }
        
        Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
        vbo->setData((void*)&(vs[0]), sizeof(glm::vec2) * m_arrowCount);
        vbo->setBufferLayout({
            { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" },
        });

        m_vaoQuiver = API::newVertexArray(vbo);

        //
        // m_shader = ShaderLibrary::load("arrows2d_shader", "../assets/shaders/arrows2d.glsl");

        // calculate arrow size based on the vp and the lowest dim
        float min_dim = (float)(m_dim.x <= m_dim.y ? m_dim.x : m_dim.y);
        float min_dim_vp = (m_dim.x <= m_dim.y ? m_vp.x : m_vp.y);
        m_arrowSize = m_samplingRate * (min_dim_vp / min_dim);

    }

private:
    FieldFBO *m_scalarField = nullptr;
    FieldFBO *m_vectorField = nullptr;
    FieldFBO *m_vectorField_quiver = nullptr;

    Ref<Shader> m_scalarField_shader = nullptr;
    Ref<Shader> m_scalarFieldNorm_shader = nullptr;
    Ref<Shader> m_vectorField_shader = nullptr;
    Ref<Shader> m_vectorField_quiver_shader = nullptr;

    // quiver render parameters
    glm::ivec2 m_dim = { 0, 0 };
    glm::vec2 m_texelSize;          // texel size in of UV-space [0..1]
    glm::vec2 m_vp;                 // viewport size of the frambuffer, used for calculating arrow size
    float m_arrowSize;              // size of the arrow, calculated based on the dim and the vp, and used as a uniform in the shader
    uint32_t m_samplingRate;        // min 0, downsampling of the dim
    uint32_t m_arrowCount = 0;      // calculated from dim and sampling rate
    Ref<VertexArray> m_vaoQuiver;


};

