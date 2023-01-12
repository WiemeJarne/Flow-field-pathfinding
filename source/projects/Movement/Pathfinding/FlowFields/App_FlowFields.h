#pragma once
#include "framework/EliteInterfaces/EIApp.h"
#include "framework/EliteAI/EliteGraphs/EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"

using namespace Elite;

class SteeringAgent;
class NavigationColliderElement;

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
	std::list<NavigationColliderElement*> m_lWalls;

	//World datamembers
	std::vector<Vector2> m_WorldPoints;
	float m_WorldWidth = 0.f;
	float m_WorldHeight = 0.f;

	//Grid datamembers
	int m_AmountOfColumns = 15;
	int m_PreviousAmountOfColumns;
	int m_AmountOfRows = 15;
	int m_PreviousAmountOfRows;
	int m_CellSize = 10;
	int m_PreviousCellSize;
	Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>* m_pGridGraph;
	std::vector<int> m_vCostField;
	std::vector<int> m_vIntegrationField;

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

	std::vector<VectorDirection> m_vVectorField;

	bool m_AddWalls{ false };

	std::list<int> m_lDestinationNodesIndices{};

	int m_AmountOfAgents = 100;
	int m_PreviousAmountOfAgents;
	std::vector<SteeringAgent*> m_vAgents;
	
	//Visualisation
	Elite::GraphRenderer* m_pGraphRenderer{ nullptr };

	//Debug rendering information
	struct DebugSettings
	{
		bool DrawNodes{ false };
		bool DrawNodeNumbers{ false };
		bool DrawConnections{ false };
		bool DrawConnectionCosts{ false };
		bool DrawCostField{ false };
		bool DrawIntegrationField{ false };
		bool DrawVectorField{ false };
	};

	DebugSettings m_DebugSettings{};

	//Functions
	void MakeGridGraph();
	void UpdateImGui();
	void CalculateIntegrationField();
	void CalculateVectorField();
	void HandleInput();
	void DrawArrow(Vector2 cellPos, VectorDirection direction) const;
	void AddWall(Vector2 pos);
	void DetermineWorldPoints();
	void ResetFields();

	App_FlowFields(const App_FlowFields&) = delete;
	App_FlowFields& operator=(const App_FlowFields&) = delete;
};