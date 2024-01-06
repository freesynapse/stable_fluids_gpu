
#include "Config.h"

#include <synapse/External/imgui/imgui_internal.h>


// static decls
Property<float>     Config::velocityDissipation(0.997f, 0.95f, 1.0f);
Property<float>     Config::densityDissipation(0.997f, 0.95f, 1.0f);
Property<float>     Config::vorticityDissipation(0.7f, 0.0f, 1.0f);
Property<float>     Config::vorticityConfinement(0.1f, 0.0f, 5.0f);
Property<int>       Config::jacobiIterCount(40, 5, 100);

Property<float>     Config::forceRadius(0.001f, 0.001f, 0.02f);
Property<float>     Config::forceMultiplier(10.0f, 1.0f, 120.0f);
Property<float>     Config::densityRadius(0.001f, 0.001f, 0.02f);

Property<bool>      Config::isRunning(true, false, true);
Property<bool>      Config::showQuivers(false, false, true);

Property<bool>      Config::renderRGB(false, false, true);
Property<int>       Config::quiverSamplingRate(8, 1, 64);

//---------------------------------------------------------------------------------------
void Config::settingsDialog()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
    ImGui::Begin("Settings", __null, ImGuiWindowFlags_NoNavInputs);

    ImGui::SetNextItemOpen(true);
    if (ImGui::TreeNode("Solver parameters"))
    {
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##velocity", &velocityDissipation.m_val, velocityDissipation.m_min, velocityDissipation.m_max, "%.3f", 1.0f); ImGui::SameLine(); ImGui::Text("velocity dissipation"); 
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##density", &densityDissipation.m_val, densityDissipation.m_min, densityDissipation.m_max, "%.3f", 1.0f); ImGui::SameLine(); ImGui::Text("density dissipation"); 
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##vorticity", &vorticityDissipation.m_val, vorticityDissipation.m_min, vorticityDissipation.m_max, "%.3f", 1.0f); ImGui::SameLine(); ImGui::Text("vorticity dissipation"); 
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderInt("##jacobi_iter", &jacobiIterCount.m_val, jacobiIterCount.m_min, jacobiIterCount.m_max); ImGui::SameLine(); ImGui::Text("Jacobi iterations");
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderFloat("##confinement", &vorticityConfinement.m_val, vorticityConfinement.m_min, vorticityConfinement.m_max, "%.1f", 1.0f); ImGui::SameLine(); ImGui::Text("vorticity confinement"); 
        
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
        ImGui::Checkbox("show velocity field", &showQuivers.m_val);
        ImGui::TreePop();

    }

    ImGui::SetNextItemOpen(true);
    if (ImGui::TreeNode("Rendering"))
    {
        ImGui::Checkbox("RGB scalar fields", &renderRGB.m_val);
        ImGui::SetNextItemWidth(300.0f); ImGui::SliderInt("##quiver_sampling", &quiverSamplingRate.m_val, quiverSamplingRate.m_min, quiverSamplingRate.m_max); ImGui::SameLine(); ImGui::Text("quiver sampling rate");
        ImGui::TreePop();

    }

    

    ImGui::End();
    ImGui::PopStyleVar();

}

