//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_FlowFields.h"

//Destructor
App_FlowFields::~App_FlowFields()
{
	SAFE_DELETE(m_pGridGraph);
	SAFE_DELETE(m_pGraphRenderer);
}

//Functions
void App_FlowFields::Start()
{
	m_pGraphRenderer = new Elite::GraphRenderer();

	//Initialization of the application. If you want access to the physics world you will need to store it yourself.
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(80.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(0.f, 0.f));
	DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(false);
	DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(false);

	//Create Graph
	MakeGridGraph();
}

void App_FlowFields::Update(float deltaTime)
{
	//UI
	UpdateImGui();
}

void App_FlowFields::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);
	//Render grid
	m_pGraphRenderer->RenderGraph(m_pGridGraph, m_DebugSettings.DrawNodes, m_DebugSettings.DrawNodeNumbers, m_DebugSettings.DrawConnections, m_DebugSettings.DrawConnectionCosts);
}

void App_FlowFields::MakeGridGraph()
{
	m_pGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(COLUMNS, ROWS, m_SizeCell, false, true, 1.f, 1.5f);
}

void App_FlowFields::UpdateImGui()
{
#ifdef  PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth{ 150 };
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive{ true };
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width) - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(menuWidth), static_cast<float>(height) - 90));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.f, ImGui::GetIO().Framerate);
		ImGui::Text("%.01f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Flow Fields");
		ImGui::Spacing();
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}