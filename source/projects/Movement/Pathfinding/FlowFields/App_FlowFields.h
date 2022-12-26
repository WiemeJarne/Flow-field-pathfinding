#pragma once
#include "framework/EliteInterfaces/EIApp.h"
#include "framework/EliteAI/EliteGraphs/EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"

using namespace Elite;

class SteeringAgent;

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
		bool DrawCostField{ false };
		bool DrawIntegrationField{ false };
		bool DrawVectorField{ false };
	};

	enum class VectorDirection
	{
		none,
		top,
		topRight,
		right,
		bottomRight,
		bottom,
		bottomLeft,
		Left,
		topLeft
	};

	//Grid datamembers
	static const int COLUMNS = 50;
	static const int ROWS = 50;
	unsigned int m_CellSize = 5;
	Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>* m_pGridGraph;
	std::vector<int> m_CostField;
	std::vector<int> m_IntegrationField;
	std::vector<VectorDirection> m_VectorField;

	int m_DestinationNodeIndex{ invalid_node_index };

	int m_AmountOfAgents = 2000;
	int m_PreviousAmountOfAgents;
	std::vector<SteeringAgent*> m_Agents;
	float m_WorldSize = 0.f;

	//Visualisation
	Elite::GraphRenderer* m_pGraphRenderer{ nullptr };

	//Debug rendering information
	DebugSettings m_DebugSettings{};

	//Functions
	void MakeGridGraph();
	void UpdateImGui();
	void CalculateIntegrationField();
	void CalculateVectorField();
	void HandleInput();
	void DrawArrow(Vector2 cellPos, VectorDirection direction) const;

	App_FlowFields(const App_FlowFields&) = delete;
	App_FlowFields& operator=(const App_FlowFields&) = delete;
};

