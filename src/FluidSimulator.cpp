
#include "FluidSimulator.h"

#include <synapse/Input>


//
FluidSimulator::FluidSimulator(const glm::ivec2 &_vp, int _downsampling)
{
    // dimensions etc
    m_vp = _vp;
    m_aspectRatioX = (float)m_vp.x / (float)m_vp.y;
    
    m_sampling = (_downsampling >= 1 ? _downsampling : 1);

    m_shape = m_vp / m_sampling;
    m_txSize = 1.0f / glm::vec2(m_shape.x, m_shape.y);

    // solver parameters
    m_densityDissipation = 0.997f;
    m_velocityDissipation = 0.997f;
    m_jacobiIterCount = 40;
    m_confinement = 0.2f;

    // create fields
    m_velocity      = VectorField(m_shape, "velocity_field");
    m_density       = ScalarField(m_shape, "density_field");
    m_divergence    = ScalarField(m_shape, "divergence_field");
    m_pressure      = ScalarField(m_shape, "pressure_field");
    m_curl          = ScalarField(m_shape, "curl_field");
    m_tmpField      = VectorField(m_shape, "tmp_field");
    m_tmpField2     = ScalarField(m_shape, "tmp_field2");

    // solver shaders
    m_advectionShader = ShaderLibrary::load(
                                "advection_shader", 
                                FileName("../assets/shaders/stencil.vert"),
                                FileName("../assets/shaders/solver/advect.frag"));

    m_divergenceShader = ShaderLibrary::load(
                                "divergence_shader",
                                FileName("../assets/shaders/stencil.vert"),
                                FileName("../assets/shaders/solver/divergence.frag"));

    m_pressureShader = ShaderLibrary::load(
                                "pressure_shader",
                                FileName("../assets/shaders/stencil.vert"),
                                FileName("../assets/shaders/solver/pressure.frag"));

    m_projectionShader = ShaderLibrary::load(
                                "projection_shader",
                                FileName("../assets/shaders/stencil.vert"),
                                FileName("../assets/shaders/solver/projection.frag"));

    m_curlShader = ShaderLibrary::load(
                                "curl_shader",
                                FileName("../assets/shaders/stencil.vert"),
                                FileName("../assets/shaders/solver/curl.frag"));
    
    m_vorticityShader = ShaderLibrary::load(
                                "vorticity_confinement_shader",
                                FileName("../assets/shaders/stencil.vert"),
                                FileName("../assets/shaders/solver/vorticity_confinement.frag"));
    

    // interaction shaders
    m_splatShader = ShaderLibrary::load(
                                "splat_shader",
                                FileName("../assets/shaders/stencil.vert"),
                                FileName("../assets/shaders/interaction/gaussian_splat.frag"));

    m_applyForceShader = ShaderLibrary::load(
                                "apply_force_shader",
                                FileName("../assets/shaders/stencil.vert"),
                                FileName("../assets/shaders/interaction/apply_force.frag"));

    m_clearShader = ShaderLibrary::load(
                                "clear_shader",
                                FileName("../assets/shaders/stencil.vert"),
                                FileName("../assets/shaders/interaction/clear.frag"));

    // -- DEBUG -- 
    // initialize velocity and density fields
    __debug__initVelocityShader = ShaderLibrary::load("__debug_init_velocity_shader",
                                                      FileName("../assets/shaders/stencil.vert"),
                                                      FileName("../assets/shaders/debug/init_velocity.frag"));

    // initial conditions
    // Quad::bind();
    // m_velocity->bind();
    // __debug__initVelocityShader->enable();
    // __debug__initVelocityShader->setUniform2fv("u_tx_size", m_txSize);
    // Quad::render();

    // glm::vec2 point = { 0.3f, 0.3f };
    // float radius = 0.005f;
    // Quad::bind();
    // m_tmpField->bind();
    // m_splatShader->enable();
    // m_splatShader->setUniform2fv("u_tx_size", m_txSize);
    // m_splatShader->setUniform1f("u_ar", m_aspectRatioX);
    // m_splatShader->setUniform2fv("u_point", point);
    // m_splatShader->setUniform1f("u_radius", radius);
    // m_density->bindTexture(0);
    // m_splatShader->setUniform1i("u_field", 0);
    // Quad::render();
    // std::swap(m_tmpField, m_density);

    // visualization
    m_showQuivers = false;
    m_activeScalarField = DENSITY_FIELD;
    change_current_field_(0);

    //
    m_initialized = true;
}

//---------------------------------------------------------------------------------------
void FluidSimulator::keyPress(int _key)
{
    switch (_key)
    {
        case SYN_KEY_TAB:   m_showQuivers = !m_showQuivers; break;
        case SYN_KEY_SPACE: m_isSimRunning = !m_isSimRunning; break;
        case SYN_KEY_RIGHT: next_field_(); break;
        case SYN_KEY_LEFT:  prev_field_(); break;
        case SYN_KEY_R:
        {
            Quad::bind();
            clearField(m_density);
            clearField(m_velocity);
            clearField(m_pressure);

        }
    }
}

//---------------------------------------------------------------------------------------
void FluidSimulator::mouseClick(int _key)
{
    static auto &renderer = Renderer::get();

    // get mouse position and normalize to [0..1] (uv)
    glm::vec4 vp_lim = renderer.getViewportLimitsF();
    glm::vec2 mpos_vp = InputManager::get_mouse_position() - glm::vec2(0.0f, vp_lim.y);
    glm::vec2 vp = renderer.getViewportF();
    glm::vec2 mpos_norm = lmap(mpos_vp, { 0.0f, 0.0f }, vp, { 0.0f, 1.0f }, { 1.0f, 0.0f });
    mpos_norm.x = clamp(mpos_norm.x, 0.0f, 1.0f);
    mpos_norm.y = clamp(mpos_norm.y, 0.0f, 1.0f);

    switch (_key)
    {
        // add density
        case SYN_MOUSE_BUTTON_1:
        {
            float radius = 0.005f;

            Quad::bind();
            m_tmpField->bind();
            m_splatShader->enable();
            m_splatShader->setUniform2fv("u_tx_size", m_txSize);
            m_splatShader->setUniform1f("u_ar", m_aspectRatioX);
            m_splatShader->setUniform2fv("u_point", mpos_norm);
            m_splatShader->setUniform1f("u_radius", radius);
            m_density->bindTexture(0);
            m_splatShader->setUniform1i("u_density", 0);
            Quad::render();
            std::swap(m_tmpField, m_density);
            
        // }

        // apply force
        // case SYN_MOUSE_BUTTON_2:
        // {
            // float radius = 0.001f;

            Quad::bind();
            m_tmpField->bind();
            m_applyForceShader->enable();
            m_applyForceShader->setUniform2fv("u_tx_size", m_txSize);
            m_applyForceShader->setUniform1f("u_ar", m_aspectRatioX);
            m_applyForceShader->setUniform2fv("u_point", mpos_norm);
            m_applyForceShader->setUniform1f("u_radius", radius);
            m_velocity->bindTexture(0);
            m_applyForceShader->setUniform1i("u_velocity", 0);
            Quad::render();
            std::swap(m_tmpField, m_velocity);
        }

    }
}

//---------------------------------------------------------------------------------------
void FluidSimulator::step(float _dt)
{
    if (!m_initialized || !m_isSimRunning)
        return;

    m_dt = _dt;

    Quad::bind();

    // advect the velocity by itself
    advect(m_velocity, m_velocityDissipation);

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
    advect(m_density, m_densityDissipation);

}

//---------------------------------------------------------------------------------------
void FluidSimulator::render(float _dt)
{
    if (!m_initialized)
        return;

    switch (m_activeScalarField)
    {
        case DENSITY_FIELD:     m_fieldRenderer.renderScalarField(m_density, false);    break;
        case DIVERGENCE_FIELD:  m_fieldRenderer.renderScalarField(m_divergence, false); break;
        case PRESSURE_FIELD:    m_fieldRenderer.renderScalarField(m_pressure, false);   break;
        case CURL_FIELD:        m_fieldRenderer.renderScalarField(m_curl, true);       break;
    }

    // m_fieldRenderer.renderScalarField(m_divergence, true);

    if (m_showQuivers)
        m_fieldRenderer.renderVectorFieldQuivers(m_velocity, 16, true, m_vp);

}

//---------------------------------------------------------------------------------------
const char *FluidSimulator::displayFieldName()
{ 
    switch (m_activeScalarField)
    {
        case DENSITY_FIELD:     return "DENSITY_FIELD";     break;
        case DIVERGENCE_FIELD:  return "DIVERGENCE_FIELD";  break;
        case PRESSURE_FIELD:    return "PRESSURE_FIELD";    break;
        case CURL_FIELD:        return "CURL_FIELD";        break;
        default:                return "UNKNOWN FIELD";     break;
    }        
}


