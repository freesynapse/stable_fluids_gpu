
#include "FluidSimulator.h"

#include <synapse/Input>


//
FluidSimulator::FluidSimulator(const glm::vec2 &_vp, int _downsampling)
{
    // dimensions etc
    m_vp = _vp;
    m_vpLim = Renderer::get().getViewportLimitsF();

    m_aspectRatioX = m_vp.x / m_vp.y;
    
    m_sampling = (_downsampling >= 1 ? _downsampling : 1);

    m_shape = glm::ivec2(m_vp.x, m_vp.y) / m_sampling;
    m_txSize = 1.0f / glm::vec2(m_shape.x, m_shape.y);

    // create fields
    m_velocity      = VectorField(m_shape, "velocity_field");
    m_density       = ScalarField(m_shape, "density_field");
    m_divergence    = ScalarField(m_shape, "divergence_field");
    m_pressure      = ScalarField(m_shape, "pressure_field");
    m_curl          = ScalarField(m_shape, "curl_field");
    m_tmpField      = VectorField(m_shape, "tmp_field");
    m_tmpField2     = ScalarField(m_shape, "tmp_field2");

    // solver shaders
    m_advectionShader   = ShaderLibrary::load("advection_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/solver/advect.frag"));
    m_diffusionShader   = ShaderLibrary::load("diffusion_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/solver/diffusion.frag"));
    m_divergenceShader  = ShaderLibrary::load("divergence_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/solver/divergence.frag"));
    m_pressureShader    = ShaderLibrary::load("pressure_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/solver/pressure.frag"));
    m_projectionShader  = ShaderLibrary::load("projection_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/solver/projection.frag"));
    m_curlShader        = ShaderLibrary::load("curl_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/solver/curl.frag"));
    m_vorticityShader   = ShaderLibrary::load("vorticity_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/solver/vorticity.frag"));

    // interaction shaders
    m_splatShader = ShaderLibrary::load("splat_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/interaction/gaussian_splat.frag"));
    m_applyForceShader = ShaderLibrary::load("apply_force_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/interaction/apply_force.frag"));
    m_clearShader = ShaderLibrary::load("clear_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/interaction/clear.frag"));

    // visualization
    m_fieldRenderer = FieldRenderer(m_vp);
    m_activeScalarField = DENSITY_FIELD;
    change_current_field_(0);

    //
    m_initialized = true;
}

//---------------------------------------------------------------------------------------
void FluidSimulator::onKeyPress(int _key)
{
    switch (_key)
    {
        case SYN_KEY_TAB:       Config::showQuivers = !Config::showQuivers(); break;
        case SYN_KEY_PARAGRAPH: Config::showLIC = !Config::showLIC(); break;
        case SYN_KEY_P:         Config::isRunning = !Config::isRunning(); break;
        case SYN_KEY_RIGHT:     next_field_(); break;
        case SYN_KEY_LEFT:      prev_field_(); break;
        case SYN_KEY_R:
        {
            Quad::bind();
            clearField(m_density, 0.0f);
            clearField(m_velocity, 0.0f);
            clearField(m_pressure, 0.0f);
            break;
        }

    }
}

//---------------------------------------------------------------------------------------
void FluidSimulator::onMouse()
{
    static glm::vec2 prev_mpos;
    glm::vec2 mpos = InputManager::get_mouse_position();
    
    // direction vector of force application
    if (mpos != prev_mpos && (InputManager::is_button_pressed(SYN_MOUSE_BUTTON_1) ||
                              InputManager::is_button_pressed(SYN_MOUSE_BUTTON_2)))
    {
        m_mousePosNorm = mpos_vp_to_screen_(mpos);
        glm::vec2 delta_mpos = m_mousePosNorm - mpos_vp_to_screen_(prev_mpos);
        m_forceDirection = glm::normalize(delta_mpos);
        m_force = min(Config::forceMultiplier() * glm::length(delta_mpos), Config::forceMultiplier.m_max);
        m_applyForces = true;

    }

    else if (!InputManager::is_button_pressed(SYN_MOUSE_BUTTON_1) && 
             !InputManager::is_button_pressed(SYN_MOUSE_BUTTON_2))
        m_applyForces = false;

    prev_mpos = mpos;

}

//---------------------------------------------------------------------------------------
void FluidSimulator::step(float _dt)
{
    onMouse();

    Timer t;

    if (!m_initialized || !Config::isRunning())
        return;

    m_dt = _dt;
    m_dx = Config::domainWidth() / m_shape.x;

    Quad::bind();

    // advect the velocity by itself
    advect(m_velocity, 1.0f);
    // advect(m_velocity, Config::velocityDissipation());

    // diffuse velocity (viscocity-dependant)
    diffuseVelocity();

    //
    addForces();

    // solve the pressure Poisson equation, using a Jacobi solver
    computePressure();
    
    // subtract the gradient of the pressure from the velocity, enforcing the
    // divergence-free fluid.
    subtractPressureGradient();

    // vorticity confinement
    computeCurl();
    applyVorticityConfinement();

    // finally, advect the density by the velocity field
    advect(m_density, Config::densityDissipation());

    //
    add_time_to_avg_(m_solverTimes, t.getDeltaTimeMs());

}

//---------------------------------------------------------------------------------------
void FluidSimulator::render(float _dt)
{
    if (!m_initialized)
        return;

    setScalarField();

    switch (m_activeScalarField)
    {
        case DENSITY_FIELD:  m_fieldRenderer.renderScalarField(m_density); break;
        case PRESSURE_FIELD: m_fieldRenderer.renderScalarField(m_pressure, &m_normRange); break;
        case CURL_FIELD:     m_fieldRenderer.renderScalarField(m_curl, &m_normRange); break;
    }

    if (Config::showQuivers())
        m_fieldRenderer.renderVectorFieldQuivers(m_velocity, 
                                                 Config::quiverSamplingRate(), 
                                                 true);

    if (Config::showLIC())
        m_fieldRenderer.renderVectorFieldLIC(m_velocity,
                                             Config::LIC_steps(),
                                             Config::LIC_traceTime());

}

//---------------------------------------------------------------------------------------
void FluidSimulator::setScalarField()
{
    std::pair<glm::vec4, glm::vec4> range;

    switch (m_activeScalarField)
    {
        case DENSITY_FIELD:     range = { glm::vec4(0.0f), glm::vec4(1.0f) };
                                Config::scalarFieldID = "DENSITY"; break;

        case PRESSURE_FIELD:    range = m_pressure->range();
                                Config::scalarFieldID = "PRESSURE"; break;

        case CURL_FIELD:        range = m_curl->range();
                                // if (Config::renderRGB())
                                //     m_fieldRenderer.setColorMap(CM_TOFINO);
                                Config::scalarFieldID = "CURL"; break;
    }

    if (Config::renderRGB())
        m_fieldRenderer.setColorMap(CM_TOFINO);
    Config::scalarFieldRange = { range.first[0], range.second[0] };
    float max = std::max(fabs(range.first[0]), fabs(range.second[0]));
    m_normRange = { -max, max };

}
