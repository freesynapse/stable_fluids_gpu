#pragma once

#include <synapse/Renderer>
#include <synapse/Debug>
#include <synapse/API>
using namespace Syn;

#include "Quad.h"
#include "FieldFBO.h"
#include "Config.h"


//
class FieldRenderer
{
public:
    FieldRenderer() { initStaticShaders(); }
    ~FieldRenderer() = default;

    //
    void renderScalarField(FieldFBO *_scalar_field, bool _normalize_range=false, int _field_id=-1)
    {
        if (_scalar_field == nullptr)
            return;

        m_scalarField = _scalar_field;
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

            shader->setUniform1i("u_rgb", Config::renderRGB() ? 1 : 0);

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
    void renderScalarField(Ref<FieldFBO> _scalar_field, bool _normalize_range=false, int _field_id=-1) 
    { renderScalarField(_scalar_field.get(), _normalize_range, _field_id); }
    
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
        m_scalarField_shader = ShaderLibrary::load("scalar_field_shader", "../assets/shaders/render/scalar_field.glsl");
        m_scalarFieldNorm_shader = ShaderLibrary::load("scalar_field_norm_shader", "../assets/shaders/render/scalar_field_norm.glsl");
        m_vectorField_shader = ShaderLibrary::load("vector_field_shader", "../assets/shaders/render/vector_field.glsl");
        m_vectorField_quiver_shader = ShaderLibrary::load("vector_field_quiver_shader", "../assets/shaders/render/vector_field_quivers.glsl");

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

