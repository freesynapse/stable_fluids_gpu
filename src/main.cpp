
#include <synapse/Synapse>
#include <synapse/SynapseMain.hpp>
using namespace Syn;

#include "FluidSimulator.h"


//
class layer : public Layer
{
public:
    layer() : Layer("layer") {}
    virtual ~layer() {}

    virtual void onAttach() override;
    virtual void onUpdate(float _dt) override;
    void onResize(Event *_e);
    virtual void onImGuiRender() override;
    
    void onKeyDownEvent(Event *_e);
    void onMouseButtonEvent(Event *_e);

private:
    Ref<Framebuffer> m_renderBuffer = nullptr;
    Ref<Font> m_font = nullptr;
    glm::ivec2 m_vp = { 0, 0 };

    FluidSimulator m_fluid;

    // flags
    bool m_wireframeMode = false;
    bool m_toggleCulling = false;

};

//
class syn_app_instance : public Application
{
public:
    syn_app_instance() { this->pushLayer(new layer); }
};
Application* CreateSynapseApplication() { return new syn_app_instance(); }

//---------------------------------------------------------------------------------------
void layer::onAttach()
{
    // register event callbacks
    EventHandler::register_callback(EventType::INPUT_KEY, SYN_EVENT_MEMBER_FNC(layer::onKeyDownEvent));
    EventHandler::register_callback(EventType::INPUT_MOUSE_BUTTON, SYN_EVENT_MEMBER_FNC(layer::onMouseButtonEvent));
    EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(layer::onResize));
    EventHandler::push_event(new WindowToggleFullscreenEvent());

    m_renderBuffer = API::newFramebuffer(ColorFormat::RGBA16F, glm::ivec2(0), 1, true, true, "render_buffer");

    // load font
    m_font = MakeRef<Font>("../assets/ttf/JetBrainsMono-Medium.ttf", 14.0f);
    m_font->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    // general settings
	Renderer::get().setClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	Renderer::get().disableImGuiUpdateReport();
    // Application::get().setMaxFPS(30.0f);
}

//---------------------------------------------------------------------------------------
void layer::onUpdate(float _dt)
{
    SYN_PROFILE_FUNCTION();
	
    static auto& renderer = Renderer::get();
    
    // -- BEGINNING OF SIMULATION -- //

    m_fluid.handleInput();
    m_fluid.step(_dt);
        
    // -- END OF SIMULATION -- //

    m_renderBuffer->bind();
    renderer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_wireframeMode)
        renderer.enableWireFrame();


    // -- BEGINNING OF SCENE -- //

    m_fluid.render(_dt);

    // -- END OF SCENE -- //


    if (m_wireframeMode)
        renderer.disableWireFrame();

	
    // Text rendering 
    // TODO: all text rendering should go into an overlay layer.
    static float fontHeight = m_font->getFontHeight() + 1.0f;
    int i = 0;
    m_font->beginRenderBlock();
	m_font->addString(2.0f, fontHeight * ++i, "fps=%.0f  VSYNC=%s", TimeStep::getFPS(), Application::get().getWindow().isVSYNCenabled() ? "ON" : "OFF");
    m_font->addString(2.0f, fontHeight * ++i, "field=%s  running=%s  step=%.3f ms", Config::scalarFieldID.c_str(), Config::isRunning() ? "TRUE" : "FALSE", m_fluid.avgStepTimeMs());
    m_font->endRenderBlock();

    //
    m_renderBuffer->bindDefaultFramebuffer();
}
 
//---------------------------------------------------------------------------------------
void layer::onResize(Event *_e)
{
    ViewportResizeEvent *e = dynamic_cast<ViewportResizeEvent*>(_e);
    m_vp = e->getViewport();

    m_fluid = FluidSimulator(m_vp, 4);

}
//---------------------------------------------------------------------------------------
void layer::onKeyDownEvent(Event *_e)
{
    KeyDownEvent *e = dynamic_cast<KeyDownEvent*>(_e);
    static bool vsync = true;

    if (e->getAction() == SYN_KEY_PRESSED)
    {
        switch (e->getKey())
        {
            case SYN_KEY_Z:         vsync = !vsync; Application::get().getWindow().setVSYNC(vsync); break;
            case SYN_KEY_V:         m_renderBuffer->saveAsPNG(); break;
            case SYN_KEY_ESCAPE:    EventHandler::push_event(new WindowCloseEvent()); break;
            case SYN_KEY_F4:        m_wireframeMode = !m_wireframeMode; break;
            case SYN_KEY_F5:        m_toggleCulling = !m_toggleCulling; Renderer::setCulling(m_toggleCulling); break;

            default: break;
        }
    
        m_fluid.onKeyPress(e->getKey());
    }
    
    
}

//---------------------------------------------------------------------------------------
void layer::onMouseButtonEvent(Event *_e)
{
    MouseButtonEvent *e = dynamic_cast<MouseButtonEvent *>(_e);

    switch (e->getButton())
    {
        case SYN_MOUSE_BUTTON_1:    break;
        case SYN_MOUSE_BUTTON_2:    break;
        default: break;
    }

    // if (e->getAction() == SYN_MOUSE_PRESSED)
    //     m_fluid.onMouseClick(e->getButton());

}

//---------------------------------------------------------------------------------------
void layer::onImGuiRender()
{
    static bool p_open = true;

    static bool opt_fullscreen_persistant = true;
    static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
    bool opt_fullscreen = opt_fullscreen_persistant;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
    	ImGuiViewport* viewport = ImGui::GetMainViewport();
    	ImGui::SetNextWindowPos(viewport->Pos);
    	ImGui::SetNextWindowSize(viewport->Size);
    	ImGui::SetNextWindowViewport(viewport->ID);
    	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (opt_flags & ImGuiDockNodeFlags_PassthruCentralNode)
	    window_flags |= ImGuiWindowFlags_NoBackground;

    window_flags |= ImGuiWindowFlags_NoTitleBar;

    ImGui::GetCurrentContext()->NavWindowingToggleLayer = false;

    //-----------------------------------------------------------------------------------
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("synapse-core", &p_open, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
	    ImGui::PopStyleVar(2);

    // Dockspace
    ImGuiIO& io = ImGui::GetIO();
    ImGuiID dockspace_id = 0;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        dockspace_id = ImGui::GetID("dockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
    }
	
    //-----------------------------------------------------------------------------------
    // UI for solver
    Config::settingsDialog();

    //-----------------------------------------------------------------------------------
    // viewport
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("synapse-core::renderer");
    static ImVec2 oldSize = { 0, 0 };
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    if (viewportSize.x != oldSize.x || viewportSize.y != oldSize.y)
    {
        // dispatch a viewport resize event -- registered classes will receive this.
        EventHandler::push_event(new ViewportResizeEvent(glm::vec2(viewportSize.x, viewportSize.y)));
        SYN_CORE_TRACE("viewport [ ", viewportSize.x, ", ", viewportSize.y, " ]");
        oldSize = viewportSize;
    }

    // direct ImGui to the framebuffer texture
    ImGui::Image((void*)m_renderBuffer->getColorAttachmentIDn(0), viewportSize, { 0, 1 }, { 1, 0 });

    ImGui::End();
    ImGui::PopStyleVar();

    // end root
    ImGui::End();

}

