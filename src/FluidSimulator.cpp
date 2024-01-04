
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
    m_divergenceShader  = ShaderLibrary::load("divergence_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/solver/divergence.frag"));
    m_pressureShader    = ShaderLibrary::load("pressure_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/solver/pressure.frag"));
    m_projectionShader  = ShaderLibrary::load("projection_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/solver/projection.frag"));
    m_curlShader        = ShaderLibrary::load("curl_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/solver/curl.frag"));
    m_vorticityShader   = ShaderLibrary::load("vorticity_confinement_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/solver/vorticity_confinement.frag"));

    // interaction shaders
    m_splatShader = ShaderLibrary::load("splat_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/interaction/gaussian_splat.frag"));
    m_applyForceShader = ShaderLibrary::load("apply_force_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/interaction/apply_force.frag"));
    m_clearShader = ShaderLibrary::load("clear_shader", FileName("../assets/shaders/stencil.vert"), FileName("../assets/shaders/interaction/clear.frag"));

    // visualization
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
        case SYN_KEY_TAB:   Config::showQuivers = !Config::showQuivers(); break;
        case SYN_KEY_SPACE: Config::isRunning = !Config::isRunning(); break;
        case SYN_KEY_RIGHT: next_field_(); break;
        case SYN_KEY_LEFT:  prev_field_(); break;
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
void FluidSimulator::handleInput()
{
    static glm::vec2 mpos, prev_mpos;
    mpos = InputManager::get_mouse_position();
    glm::vec2 mpos_norm = mpos_vp_to_screen_(mpos);

    // direction vector of force application
    if (mpos != prev_mpos)
    {
        // glm::vec2 delta_mpos = mpos - prev_mpos;
        glm::vec2 delta_mpos = mpos_norm - mpos_vp_to_screen_(prev_mpos);
        glm::vec2 dir = glm::normalize(delta_mpos);
        float force = min(Config::forceMultiplier() * glm::length(delta_mpos), 100.0f);
     
        if (InputManager::is_button_pressed(SYN_MOUSE_BUTTON_1))
        {
            Quad::bind();
            applyForce(mpos_norm, dir, force);
            addDensity(mpos_norm);

        }
        else if (InputManager::is_button_pressed(SYN_MOUSE_BUTTON_2))
        {
            Quad::bind();
            applyForce(mpos_norm, dir, force);

        }

    }

    if (InputManager::is_key_pressed(SYN_KEY_ENTER))
    {
        Quad::bind();
        addDensity(mpos_norm);

    }

    prev_mpos = mpos;

}

//---------------------------------------------------------------------------------------
void FluidSimulator::step(float _dt)
{
    if (!m_initialized || !Config::isRunning())
        return;

    m_dt = _dt;

    Quad::bind();

    // advect the velocity by itself
    advect(m_velocity, Config::velocityDissipation());

    // compute the divergence for enforcing a divergence-free velocity field (below)
    computeDivergence();
    
    // solve the pressure Poisson equation, using a Jacobi solver
    solvePressure();
    
    // subtract the gradient of the pressure from the velocity, enforcing the
    // divergence-free fluid.
    subtractPressureGradient();

    // vorticity confinement
    computeCurl();
    // auto r = m_curl->range();
    // printf("curl : [%f .. %f]\n", r.first[0], r.second[0]);
    applyVorticityConfinement();

    // finally, advect the density by the velocity field
    advect(m_density, Config::densityDissipation());

}

//---------------------------------------------------------------------------------------
void FluidSimulator::render(float _dt)
{
    if (!m_initialized)
        return;

    switch (m_activeScalarField)
    {
        case DENSITY_FIELD:     m_fieldRenderer.renderScalarField(m_density, false); break;
        case PRESSURE_FIELD:    m_fieldRenderer.renderScalarField(m_pressure, false); break;
        case CURL_FIELD:        m_fieldRenderer.renderScalarField(m_curl, true); break;
    }

    if (Config::showQuivers())
        m_fieldRenderer.renderVectorFieldQuivers(m_velocity, 
                                                 Config::quiverSamplingRate(), 
                                                 true, 
                                                 m_vp);

}

//---------------------------------------------------------------------------------------
const char *FluidSimulator::displayFieldName()
{ 
    switch (m_activeScalarField)
    {
        case DENSITY_FIELD:     return "DENSITY_FIELD";     break;
        case PRESSURE_FIELD:    return "PRESSURE_FIELD";    break;
        case CURL_FIELD:        return "CURL_FIELD";        break;
        default:                return "UNKNOWN FIELD";     break;
    }        
}


