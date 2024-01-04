#pragma once

#include "FieldFBO.h"
#include "FieldRenderer.h"
#include "Config.h"

//
#define DENSITY_FIELD       0
#define PRESSURE_FIELD      1
#define CURL_FIELD          2

#define FIELD_COUNT         3

//
class FluidSimulator
{
public:
    FluidSimulator() {}
    FluidSimulator(const glm::vec2 &_vp, int _downsampling=1);
    ~FluidSimulator() = default;

    // interaction
    void handleInput();
    void onKeyPress(int _key);
    const char *displayFieldName();
    float avgStepTimeMs() { return m_solverTimeAvgMs; }

    // solver
    void step(float _dt);

    // rendering through FieldRenderer.h
    void render(float _dt);


private:
    __always_inline
    void change_current_field_(int _offset)
    { m_activeScalarField = (m_activeScalarField + _offset + FIELD_COUNT) % FIELD_COUNT; }
    
    __always_inline
    void next_field_() { change_current_field_( 1); }

    __always_inline
    void prev_field_() { change_current_field_(-1); }

    __always_inline
    glm::vec2 mpos_vp_to_screen_(const glm::vec2 &_mpos)
    {
        glm::vec2 mpos_vp = _mpos - glm::vec2(0.0f, m_vpLim.y);
        glm::vec2 mpos_norm = lmap(mpos_vp, { 0.0f, 0.0f }, m_vp, { 0.0f, 1.0f }, { 1.0f, 0.0f });
        mpos_norm.x = clamp(mpos_norm.x, 0.0f, 1.0f);
        mpos_norm.y = clamp(mpos_norm.y, 0.0f, 1.0f);
        return mpos_norm;
    }

    __always_inline
    void add_time_to_avg(std::list<float> &_list, float _time_ms)
    {
        static int n = 100;
        if (_list.size() >= n)
            _list.pop_front();
        _list.push_back(_time_ms);

        m_solverTimeAvgMs = std::accumulate(_list.begin(), _list.end(), 0.0f) / (float)_list.size();
    }

private:
    // solver functions -- in FluidSimulatorSolver.cpp
    void advect(Ref<FieldFBO> &_quantity, float _dissipation);
    void computeDivergence();
    void solvePressure();
    void subtractPressureGradient();
    void computeCurl();
    void applyVorticityConfinement();

    // helper functions
    void applyForce(const glm::vec2 &_pos,          // pos is normalized to [0..1]
                    const glm::vec2 &_direction,
                    float _force);
    void addDensity(const glm::vec2 &_pos);         // pos is normalized to [0..1]
    void clearField(Ref<FieldFBO> &_field, float _value);


private:

    // dimensions etc
    glm::vec2 m_vp;
    glm::vec4 m_vpLim;
    float m_aspectRatioX;
    glm::ivec2 m_shape;
    int m_sampling;
    glm::vec2 m_txSize;
    bool m_initialized = false;

    // solver parameters
    float m_dt = 0.0f;

    //
    std::list<float> m_solverTimes;
    float m_solverTimeAvgMs = 0.0f;

    // fields and shaders
    Ref<FieldFBO> m_velocity;
    Ref<FieldFBO> m_density;
    Ref<FieldFBO> m_divergence;
    Ref<FieldFBO> m_pressure;
    Ref<FieldFBO> m_curl;
    Ref<FieldFBO> m_tmpField;
    Ref<FieldFBO> m_tmpField2;

    // solver shaders
    Ref<Shader> m_advectionShader;
    Ref<Shader> m_divergenceShader;
    Ref<Shader> m_pressureShader;
    Ref<Shader> m_projectionShader;
    Ref<Shader> m_curlShader;
    Ref<Shader> m_vorticityShader;
    
    // interaction shaders
    Ref<Shader> m_splatShader;
    Ref<Shader> m_applyForceShader;
    Ref<Shader> m_clearShader;

    // visualization
    // bool m_showQuivers;
    FieldRenderer m_fieldRenderer;
    int m_activeScalarField = DENSITY_FIELD;
    // bool m_isSimRunning = false;

};
