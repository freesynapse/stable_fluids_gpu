
#include "FluidSimulator.h"

//
void FluidSimulator::advect(Ref<FieldFBO> &_quantity, float _dissipation)
{
    m_tmpField->bind();
    m_advectionShader->enable();
    m_advectionShader->setUniform2fv("u_tx_size", m_txSize);
    m_velocity->bindTexture(0);
    m_advectionShader->setUniform1i("u_velocity", 0);
    _quantity->bindTexture(1, 0, GL_LINEAR);
    m_advectionShader->setUniform1i("u_quantity", 1);
    m_advectionShader->setUniform1f("u_dissipation", _dissipation);
    m_advectionShader->setUniform1f("u_dt", m_dt);
    Quad::render();
    std::swap(m_tmpField, _quantity);

}

//---------------------------------------------------------------------------------------
void FluidSimulator::computeDivergence()
{
    m_divergence->bind();
    m_divergenceShader->enable();
    m_divergenceShader->setUniform2fv("u_tx_size", m_txSize);
    m_velocity->bindTexture(0);
    m_divergenceShader->setUniform1i("u_velocity", 0);
    Quad::render();    
}

//---------------------------------------------------------------------------------------
void FluidSimulator::solvePressure()
{
    m_tmpField->bind();
    m_pressureShader->enable();
    m_pressureShader->setUniform2fv("u_tx_size", m_txSize);
    m_divergence->bindTexture(0);
    m_pressureShader->setUniform1i("u_divergence", 0);
    m_pressure->bindTexture(1);
    m_pressureShader->setUniform1i("u_pressure", 1);
    Quad::render();
    
    // m_tmpField now holds the pressure
    for (uint32_t i = 0; i < Config::jacobiIterCount(); i++)
    {
        m_tmpField2->bind();
        m_divergence->bindTexture(0);
        m_pressureShader->setUniform1i("u_divergence", 0);
        m_tmpField->bindTexture(1);
        m_pressureShader->setUniform1i("u_pressure", 1);
        Quad::render();
        std::swap(m_tmpField, m_tmpField2);
    }
    std::swap(m_tmpField, m_pressure);
    
}

//---------------------------------------------------------------------------------------
void FluidSimulator::subtractPressureGradient()
{
    m_tmpField->bind();
    m_projectionShader->enable();
    m_projectionShader->setUniform2fv("u_tx_size", m_txSize);
    m_pressure->bindTexture(0);
    m_projectionShader->setUniform1i("u_pressure", 0);
    m_velocity->bindTexture(1);
    m_projectionShader->setUniform1i("u_velocity", 1);
    Quad::render();
    std::swap(m_tmpField, m_velocity);

}

//---------------------------------------------------------------------------------------
void FluidSimulator::computeCurl()
{
    m_curl->bind();
    m_curlShader->enable();
    m_curlShader->setUniform2fv("u_tx_size", m_txSize);
    m_velocity->bindTexture(0);
    m_curlShader->setUniform1i("u_velocity", 0);
    Quad::render();

}

//---------------------------------------------------------------------------------------
void FluidSimulator::applyVorticityConfinement()
{
    m_tmpField->bind();
    m_vorticityShader->enable();
    m_vorticityShader->setUniform2fv("u_tx_size", m_txSize);
    m_vorticityShader->setUniform1f("u_confinement", Config::vorticityConfinement());
    m_vorticityShader->setUniform1f("u_Dissipation", Config::vorticityDissipation());
    m_vorticityShader->setUniform1f("u_dt", m_dt);
    m_velocity->bindTexture(0);
    m_vorticityShader->setUniform1i("u_velocity", 0);
    m_curl->bindTexture(1);
    m_vorticityShader->setUniform1i("u_curl", 1);
    Quad::render();
    std::swap(m_tmpField, m_velocity);

}

//---------------------------------------------------------------------------------------
void FluidSimulator::applyForce(const glm::vec2 &_pos, 
                                const glm::vec2 &_direction, 
                                float _force)
{
    m_tmpField->bind();
    m_applyForceShader->enable();
    m_applyForceShader->setUniform2fv("u_tx_size", m_txSize);
    m_applyForceShader->setUniform1f("u_ar", m_aspectRatioX);
    m_applyForceShader->setUniform2fv("u_point", _pos);
    m_applyForceShader->setUniform2fv("u_direction", _direction);
    m_applyForceShader->setUniform1f("u_force", _force);
    m_applyForceShader->setUniform1f("u_radius", Config::forceRadius());
    m_velocity->bindTexture(0);
    m_applyForceShader->setUniform1i("u_velocity", 0);
    Quad::render();
    std::swap(m_tmpField, m_velocity);

}

//---------------------------------------------------------------------------------------
void FluidSimulator::addDensity(const glm::vec2 &_pos)
{
    m_tmpField->bind();
    m_splatShader->enable();
    m_splatShader->setUniform2fv("u_tx_size", m_txSize);
    m_splatShader->setUniform1f("u_ar", m_aspectRatioX);
    m_splatShader->setUniform2fv("u_point", _pos);
    m_splatShader->setUniform1f("u_radius", Config::densityRadius());
    m_density->bindTexture(0);
    m_splatShader->setUniform1i("u_density", 0);
    Quad::render();
    std::swap(m_tmpField, m_density);
}

//---------------------------------------------------------------------------------------
void FluidSimulator::clearField(Ref<FieldFBO> &_field, float _value)
{
    m_tmpField->bind();
    m_clearShader->enable();
    m_clearShader->setUniform2fv("u_tx_size", m_txSize);
    m_clearShader->setUniform1f("u_value", _value);
    _field->bindTexture(0);
    m_clearShader->setUniform1i("u_field", 0);
    Quad::render();
    std::swap(m_tmpField, _field);

}

