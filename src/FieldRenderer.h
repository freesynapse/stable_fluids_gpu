#pragma once

#include <synapse/Renderer>
#include <synapse/Debug>
#include <synapse/API>
using namespace Syn;

#include "Quad.h"
#include "FieldFBO.h"
#include "Config.h"

//
#define CM_GRAYSCALE    0
#define CM_TOFINO       1

//
inline const std::vector<std::vector<glm::vec3>> colorMaps
{
    {
        // gray
        { 0.000f, 0.000f, 0.000f },
        { 0.116f, 0.116f, 0.116f },
        { 0.208f, 0.208f, 0.208f },
        { 0.307f, 0.307f, 0.307f },
        { 0.412f, 0.412f, 0.412f },
        { 0.522f, 0.522f, 0.522f },
        { 0.636f, 0.636f, 0.636f },
        { 0.754f, 0.754f, 0.754f },
        { 0.875f, 0.875f, 0.875f },
        { 1.000f, 1.000f, 1.000f }
    },

    {
        // tofino (diverging)
        { 0.870f, 0.850f, 1.000f },
        { 0.602f, 0.662f, 0.883f },
        { 0.343f, 0.467f, 0.731f },
        { 0.186f, 0.287f, 0.478f },
        { 0.096f, 0.145f, 0.238f },
        { 0.049f, 0.084f, 0.078f },
        { 0.093f, 0.195f, 0.103f },
        { 0.169f, 0.362f, 0.187f },
        { 0.289f, 0.553f, 0.295f },
        { 0.561f, 0.743f, 0.453f },
        { 0.860f, 0.900f, 0.610f }
    },
};

//
class FieldRenderer
{
public:
    FieldRenderer() {}
    FieldRenderer(const glm::vec2 &_vp)
    {
        m_vp = _vp;
        init_static_shaders_();
        create_noise_data_();
        setup_color_map_();
    }
    ~FieldRenderer() = default;

    // set colormap
    void setColorMap(int _cm)
    {
        if (_cm != m_cm)
            m_cm = _cm;
        setup_color_map_();
    }

    // rendering functions
    //

    //
    void renderScalarField(FieldFBO *_scalar_field, const glm::vec2 *_range=nullptr)
    {
        if (_scalar_field == nullptr)
            return;

        m_scalarField = _scalar_field;
        bool normalize_range = _range != nullptr;
        Shader *shader = (normalize_range ? m_scalarFieldNorm_shader.get() : m_scalarField_shader.get());

        auto &dim = m_scalarField->getSize();
        glm::vec2 tx_size = 1.0f / glm::vec2(dim.x, dim.y);
        Quad::bind();
        shader->enable();
        m_scalarField->bindTexture(0, 0, GL_LINEAR);
        shader->setUniform1i("u_field", 0);
        m_colorMap->bind(1);
        shader->setUniform1i("u_colormap", 1);

        // normalize scalar field if requested
        if (normalize_range)
            shader->setUniform2fv("u_range", *_range);
        
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
                                  bool _field_updated=false)
    {
        if (_vector_field == nullptr)
            return;

        if (_vector_field != m_vectorField_quiver)
        {
            m_vectorField_quiver = _vector_field;
            m_samplingRate = (_sampling_rate > 0 ? _sampling_rate : 1);
            m_dim = m_vectorField_quiver->getSize();
            m_texelSize = 1.0f / glm::vec2(m_dim[0], m_dim[1]);
            init_quivers_();
        }

        if (_field_updated)
            init_quivers_();

        static auto &renderer = Renderer::get();
        
        m_vectorField_quiver_shader->enable();
        m_vectorField_quiver->bindTexture(0);
        m_vectorField_quiver_shader->setUniform1i("u_vector_field", 0);
        m_vectorField_quiver_shader->setUniform1f("u_arrow_size", m_arrowSize);

        renderer.drawArrays(m_vaoQuiver, m_arrowCount, 0, false, GL_POINTS);

    }

    //
    void renderVectorFieldLIC(FieldFBO *_vector_field, int _N, float _trace_time)
    {
        if (_vector_field == nullptr)
            return;
        
        if (_vector_field != m_vectorField_LIC)
            m_vectorField_LIC = _vector_field;
        
        static auto &renderer = Renderer::get();

        Quad::bind();
        m_vectorField_LIC_shader->enable();
        m_vectorField_LIC->bindTexture(0, 0, GL_LINEAR);
        m_vectorField_LIC_shader->setUniform1i("u_velocity", 0);
        m_noise->bind(1);
        m_vectorField_LIC_shader->setUniform1i("u_noise", 1);
        m_vectorField_LIC_shader->setUniform1i("u_N", _N);
        m_vectorField_LIC_shader->setUniform1f("u_trace_time", _trace_time);
        Quad::render();
    }

    
    // overloads for Ref<FieldFBO>
    __always_inline 
    void renderScalarField(Ref<FieldFBO> _scalar_field, glm::vec2 *_range=nullptr) 
    { renderScalarField(_scalar_field.get(), _range); }
    
    __always_inline 
    void renderVectorField(Ref<FieldFBO> _vector_field) 
    { renderVectorField(_vector_field.get()); }
    
    __always_inline
    void renderVectorFieldQuivers(Ref<FieldFBO> _vector_field, 
                                  uint32_t _sampling_rate=1,
                                  bool _field_updated=false)
    { renderVectorFieldQuivers(_vector_field.get(), _sampling_rate, _field_updated); }

    __always_inline
    void renderVectorFieldLIC(Ref<FieldFBO> _vector_field, int _N, float _trace_time)
    { renderVectorFieldLIC(_vector_field.get(), _N, _trace_time); }


private:
    void init_static_shaders_()
    {
        m_scalarField_shader = ShaderLibrary::load("scalar_field_shader", "../assets/shaders/render/scalar_field.glsl");
        m_scalarFieldNorm_shader = ShaderLibrary::load("scalar_field_norm_shader", "../assets/shaders/render/scalar_field_norm.glsl");
        m_vectorField_shader = ShaderLibrary::load("vector_field_shader", "../assets/shaders/render/vector_field.glsl");
        m_vectorField_quiver_shader = ShaderLibrary::load("vector_field_quiver_shader", "../assets/shaders/render/vector_field_quivers.glsl");
        m_vectorField_LIC_shader = ShaderLibrary::load("vector_field_LIC_shader", "../assets/shaders/render/vector_field_LIC.glsl");

    }

    // 
    void create_noise_data_()
    {
        // creates noise data for rendering streamlines (smearing of noise along velocity)
        m_noise = API::newTexture2D(m_vp.x * 0.5f, m_vp.y * 0.5f, ColorFormat::R32F);
        size_t sz = (size_t)(m_vp.x * 0.5) * (size_t)(m_vp.y * 0.5);
        std::vector<float> noise_data(sz, 0.0f);
        for (auto& n : noise_data)
            n = Random::rand_fC();
        m_noise->setData((void*)&noise_data[0], sz * sizeof(float));
        SYN_TRACE("noise created for LIC");

    }

    void setup_color_map_()
    {
        m_colorMap = API::newTexture1D(colorMaps[m_cm].size(), ColorFormat::RGB32F);
        m_colorMap->setData((void*)&colorMaps[m_cm][0], sizeof(glm::vec3) * colorMaps[m_cm].size());

    }
    void update_color_map()
    {
        m_colorMap->setData((void*)&colorMaps[m_cm][0], sizeof(glm::vec3) * colorMaps[m_cm].size());

    }

    //
    void init_quivers_()
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
    FieldFBO *m_scalarField                 = nullptr;
    FieldFBO *m_vectorField                 = nullptr;
    FieldFBO *m_vectorField_quiver          = nullptr;
    FieldFBO *m_vectorField_LIC             = nullptr;

    Ref<Shader> m_scalarField_shader        = nullptr;
    Ref<Shader> m_scalarFieldNorm_shader    = nullptr;
    Ref<Shader> m_vectorField_shader        = nullptr;
    Ref<Shader> m_vectorField_quiver_shader = nullptr;
    Ref<Shader> m_vectorField_LIC_shader    = nullptr;

    Ref<Texture2D> m_noise;                 // used for line integral convolution
    Ref<Texture1D> m_colorMap;
    int m_cm = CM_GRAYSCALE;

    // quiver render parameters
    glm::ivec2 m_dim = { 0, 0 };
    glm::vec2 m_texelSize;                  // texel size in of UV-space [0..1]
    glm::vec2 m_vp;                         // viewport size of the frambuffer, used for calculating arrow size
    float m_arrowSize;                      // size of the arrow, calculated based on the dim and the vp, and used as a uniform in the shader
    uint32_t m_samplingRate;                // min 0, downsampling of the dim
    uint32_t m_arrowCount = 0;              // calculated from dim and sampling rate
    Ref<VertexArray> m_vaoQuiver;


};

