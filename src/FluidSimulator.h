#pragma once

#include "FieldFBO.h"
#include "FieldRenderer.h"

//
#define DENSITY_FIELD       0
#define DIVERGENCE_FIELD    1
#define PRESSURE_FIELD      2
#define CURL_FIELD          3

#define FIELD_COUNT         4

//
class FluidSimulator
{
public:
    FluidSimulator() {}
    FluidSimulator(const glm::ivec2 &_vp, int _downsampling=1);
    ~FluidSimulator() = default;

    // interaction
    void keyPress(int _key);
    void mouseClick(int _key);
    const char *displayFieldName();

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

private:
    // solver functions -- in FluidSimulatorSolver.cpp
    void advect(Ref<FieldFBO> &_quantity, float _dissipation);
    void computeDivergence();
    void solvePressure();
    void subtractPressureGradient();
    void computeCurl();
    void applyVorticityConfinement();

    // helper functions
    void clearField(Ref<FieldFBO> &_field);


private:

    // dimensions etc
    glm::ivec2 m_vp;
    float m_aspectRatioX;
    glm::ivec2 m_shape;
    int m_sampling;
    glm::vec2 m_txSize;
    bool m_initialized = false;

    // solver parameters
    float m_velocityDissipation = 0.0f;
    float m_densityDissipation = 0.0f;
    uint32_t m_jacobiIterCount = 0;
    float m_confinement = 0.0f;
    float m_dt = 0.0f;

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
    bool m_showQuivers;
    FieldRenderer m_fieldRenderer;
    int m_activeScalarField = DENSITY_FIELD;
    bool m_isSimRunning = false;
    bool m_singleStep = false;

    // -- DEBUG --
    Ref<Shader> __debug__initVelocityShader;
    
};
