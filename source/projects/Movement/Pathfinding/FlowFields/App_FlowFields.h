#pragma once
#include "framework/EliteInterfaces/EIApp.h"
#include "framework/EliteAI/EliteGraphs/EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"

using namespace Elite;

class App_FlowFields : public IApp
{
public:
	//Constructor & Destructor
	App_FlowFields() = default;
	virtual ~App_FlowFields() final;

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	struct DebugSettings
	{
		bool DrawNodes{ true };
		bool DrawNodeNumbers{ false };
		bool DrawConnections{ false };
		bool DrawConnectionCosts{ false };
	};

	//Grid datamembers
	static const int COLUMNS = 20;
	static const int ROWS = 10;
	unsigned int m_SizeCell = 15;
	Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>* m_pGridGraph;

	//Visualisation
	Elite::GraphRenderer* m_pGraphRenderer{ nullptr };

	//Debug rendering information
	DebugSettings m_DebugSettings{};

	//Functions
	void MakeGridGraph();
	void UpdateImGui();

	App_FlowFields(const App_FlowFields&) = delete;
	App_FlowFields& operator=(const App_FlowFields&) = delete;
};

