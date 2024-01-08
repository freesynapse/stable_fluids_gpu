
#include "Config.h"

#include <synapse/External/imgui/imgui_internal.h>
#include <synapse/SynapseCore/Utils/Timer/TimeStep.hpp>


// static decls
Property<float>     Config::domainWidth(1.0f, 1.0f, 5.0f);
Property<float>     Config::mu(0.001f, 0.0f, 1.0f);
Property<float>     Config::rho(100.0f, 10.0f, 10000.0f);
// Property<float>     Config::velocityDissipation(0.997f, 0.95f, 1.0f);
Property<float>     Config::densityDissipation(0.997f, 0.95f, 1.0f);
// Property<float>     Config::vorticityDissipation(0.9f, 0.0f, 1.0f);
Property<float>     Config::vorticityConfinement(0.02f, 0.0f, 0.2f);
Property<int>       Config::jacobiIterCount(40, 5, 100);

Property<float>     Config::forceRadius(0.001f, 0.001f, 0.02f);
Property<float>     Config::forceMultiplier(25.0f, 1.0f, 120.0f);
Property<float>     Config::densityRadius(0.001f, 0.001f, 0.02f);

Property<bool>      Config::isRunning(true, false, true);
Property<bool>      Config::showQuivers(false, false, true);
Property<bool>      Config::showLIC(false, false, true);

std::string         Config::scalarFieldID;
glm::vec2           Config::scalarFieldRange;
Property<bool>      Config::renderRGB(true, false, true);
Property<int>       Config::quiverSamplingRate(8, 1, 64);
Property<int>       Config::LIC_steps(50, 10, 100);
Property<float>     Config::LIC_traceTime(0.25f, 0.05f, 1.0f);

//---------------------------------------------------------------------------------------
void Config::settingsDialog()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
    ImGui::Begin("Settings", __null, ImGuiWindowFlags_NoNavInputs);

    ImGui::SetNextItemOpen(true);
    if (ImGui::TreeNode("Solver parameters"))
    {
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##domain", &domainWidth.m_val, domainWidth.m_min, domainWidth.m_max, "%.1f", 1.0f); ImGui::SameLine(); ImGui::Text("domain width"); 
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##mu", &mu.m_val, mu.m_min, mu.m_max, "%.3f", 1.0f); ImGui::SameLine(); ImGui::Text("mu"); 
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##rho", &rho.m_val, rho.m_min, rho.m_max, "%.1f", 1.0f); ImGui::SameLine(); ImGui::Text("rho"); 
        // ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##velocity", &velocityDissipation.m_val, velocityDissipation.m_min, velocityDissipation.m_max, "%.3f", 1.0f); ImGui::SameLine(); ImGui::Text("velocity dissipation"); 
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##density", &densityDissipation.m_val, densityDissipation.m_min, densityDissipation.m_max, "%.3f", 1.0f); ImGui::SameLine(); ImGui::Text("density dissipation"); 
        // ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##vorticity", &vorticityDissipation.m_val, vorticityDissipation.m_min, vorticityDissipation.m_max, "%.3f", 1.0f); ImGui::SameLine(); ImGui::Text("vorticity dissipation"); 
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderInt("##jacobi_iter", &jacobiIterCount.m_val, jacobiIterCount.m_min, jacobiIterCount.m_max); ImGui::SameLine(); ImGui::Text("Jacobi iterations");
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##confinement", &vorticityConfinement.m_val, vorticityConfinement.m_min, vorticityConfinement.m_max, "%.2f", 1.0f); ImGui::SameLine(); ImGui::Text("vorticity confinement"); 
        
        ImGui::TreePop();

    }

    ImGui::SetNextItemOpen(true);
    if (ImGui::TreeNode("Interaction behaviour"))
    {
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##force_r", &forceRadius.m_val, forceRadius.m_min, forceRadius.m_max, "%.3f", 1.0f); ImGui::SameLine(); ImGui::Text("force radius"); 
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##force_mult", &forceMultiplier.m_val, forceMultiplier.m_min, forceMultiplier.m_max, "%.3f", 1.0f); ImGui::SameLine(); ImGui::Text("force multiplier"); 
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##dens_r", &densityRadius.m_val, densityRadius.m_min, densityRadius.m_max, "%.3f", 1.0f); ImGui::SameLine(); ImGui::Text("density radius"); 

        ImGui::TreePop();

    }

    ImGui::SetNextItemOpen(true);
    if (ImGui::TreeNode("Simulation"))
    {
        ImGui::Checkbox("running simulation", &isRunning.m_val);
        ImGui::Checkbox("velocity field quivers", &showQuivers.m_val);
        ImGui::Checkbox("velocity field LIC", &showLIC.m_val);
        ImGui::Text("Scalar field");
        ImGui::SetNextItemWidth(208.0f); ImGui::InputText("##scalar_id", (char*)scalarFieldID.c_str(), 20, ImGuiInputTextFlags_ReadOnly);
        char min_buf[20], max_buf[20];
        snprintf(min_buf, 20, "%.3f", scalarFieldRange[0]);
        snprintf(max_buf, 20, "%.3f", scalarFieldRange[1]);
        ImGui::SetNextItemWidth(100.0f); ImGui::InputText("##scalar_min", min_buf, 20, ImGuiInputTextFlags_ReadOnly); ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f); ImGui::InputText("##scalar_max", max_buf, 20, ImGuiInputTextFlags_ReadOnly); 
        ImGui::TreePop();

    }

    ImGui::SetNextItemOpen(true);
    if (ImGui::TreeNode("Rendering"))
    {
        ImGui::Checkbox("RGB pressure and curl", &renderRGB.m_val);
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderInt("##quiver_sampling", &quiverSamplingRate.m_val, quiverSamplingRate.m_min, quiverSamplingRate.m_max); ImGui::SameLine(); ImGui::Text("quiver sampling rate");
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderInt("##LIC_steps", &LIC_steps.m_val, LIC_steps.m_min, LIC_steps.m_max); ImGui::SameLine(); ImGui::Text("LIC steps");
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##LIC_time", &LIC_traceTime.m_val, LIC_traceTime.m_min, LIC_traceTime.m_max, "%.2f", 1.0f); ImGui::SameLine(); ImGui::Text("LIC trace time"); 
        char fps_buf[10];
        snprintf(fps_buf, 10, "%.0f", Syn::TimeStep::getFPS());
        ImGui::SetNextItemWidth(50.0f); ImGui::InputText("##fps", fps_buf, ImGuiInputTextFlags_ReadOnly); ImGui::SameLine(); ImGui::Text("FPS");
        ImGui::TreePop();

    }

    ImGui::End();
    ImGui::PopStyleVar();

}

