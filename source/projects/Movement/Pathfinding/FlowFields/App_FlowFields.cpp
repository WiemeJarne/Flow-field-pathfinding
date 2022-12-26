//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_FlowFields.h"
#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"

//Destructor
App_FlowFields::~App_FlowFields()
{
	/*for (auto pAgent : m_Agents)
	{
		SAFE_DELETE(pAgent)
	}
	m_Agents.clear();*/

	SAFE_DELETE(m_pGridGraph);
	SAFE_DELETE(m_pGraphRenderer);
}

//Functions
void App_FlowFields::Start()
{
	srand(time(nullptr));

	m_pGraphRenderer = new Elite::GraphRenderer();
	m_WorldSize = COLUMNS * static_cast<float>(m_CellSize);

	//Create the agents
	Vector2 randomPos{};
	for (int index{}; index < m_AmountOfAgents; ++index)
	{
		m_Agents.push_back(new SteeringAgent());
		m_Agents[index]->SetMaxLinearSpeed(25.f);
		m_Agents[index]->SetMass(0.f);
		m_Agents[index]->SetAutoOrient(true);
		randomPos.x = static_cast<float>(rand() % static_cast<int>(m_WorldSize));
		randomPos.y = static_cast<float>(rand() % static_cast<int>(m_WorldSize));
		m_Agents[index]->SetPosition(randomPos);
	}

	//Initialization of the application. If you want access to the physics world you will need to store it yourself.
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(135.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(m_WorldSize / 1.5f, m_WorldSize / 2));
	DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(false);
	DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(false);

	//Create Graph
	MakeGridGraph();

	//set all the cost in the costField to 1
	//fill the integrationField with a high number
	//fill the vectorField
	for (int rowNr{}; rowNr < ROWS; ++rowNr)
	{
		for (int colNr{}; colNr < COLUMNS; ++colNr)
		{
			//m_CostField.push_back(1);
			m_IntegrationField.push_back(1000);
			m_VectorField.push_back(VectorDirection::none);
		}
	}

	//m_CostField[252] = 255;
}

void App_FlowFields::Update(float deltaTime)
{
	//INPUT
	HandleInput();

	//Update agents
	for (const auto& agent : m_Agents)
	{
		agent->Update(deltaTime);
		agent->TrimToWorld(m_WorldSize);
		const auto nodeIndex{ m_pGridGraph->GetNodeIdxAtWorldPos(agent->GetPosition()) };

		if (nodeIndex == invalid_node_index) continue;

		switch (m_VectorField[nodeIndex])
		{
		case VectorDirection::none:
			agent->SetLinearVelocity(Vector2(0.f, 0.f) * agent->GetMaxLinearSpeed());
			break;

		case VectorDirection::top:
			agent->SetLinearVelocity(Vector2(0.f, 1.f) * agent->GetMaxLinearSpeed());
			break;

		case VectorDirection::topRight:
			agent->SetLinearVelocity(Vector2(1.f, 1.f) * agent->GetMaxLinearSpeed());
			break;

		case VectorDirection::right:
			agent->SetLinearVelocity(Vector2(1.f, 0.f) * agent->GetMaxLinearSpeed());
			break;

		case VectorDirection::bottomRight:
			agent->SetLinearVelocity(Vector2(1.f, -1.f) * agent->GetMaxLinearSpeed());
			break;

		case VectorDirection::bottom:
			agent->SetLinearVelocity(Vector2(0.f, -1.f) * agent->GetMaxLinearSpeed());
			break;

		case VectorDirection::bottomLeft:
			agent->SetLinearVelocity(Vector2(-1.f, -1.f) * agent->GetMaxLinearSpeed());
			break;

		case VectorDirection::Left:
			agent->SetLinearVelocity(Vector2(-1.f, 0.f) * agent->GetMaxLinearSpeed());
			break;

		case VectorDirection::topLeft:
			agent->SetLinearVelocity(Vector2(-1.f, 1.f) * agent->GetMaxLinearSpeed());
			break;
		}
	}

	//UI
	UpdateImGui();

	if (m_AmountOfAgents != m_PreviousAmountOfAgents)
	{
		for (auto pAgent : m_Agents)
		{
			SAFE_DELETE(pAgent)
		}
		m_Agents.clear();

		//Create the agents
		Vector2 randomPos{};
		for (int index{}; index < m_AmountOfAgents; ++index)
		{
			m_Agents.push_back(new SteeringAgent());
			m_Agents[index]->SetMaxLinearSpeed(15.f);
			m_Agents[index]->SetMass(0.f);
			m_Agents[index]->SetAutoOrient(true);
			randomPos.x = static_cast<float>(rand() % static_cast<int>(m_WorldSize));
			randomPos.y = static_cast<float>(rand() % static_cast<int>(m_WorldSize));
			m_Agents[index]->SetPosition(randomPos);
			m_Agents[index]->SetRenderBehavior(true);
		}
	}

	m_PreviousAmountOfAgents = m_AmountOfAgents;
}

void App_FlowFields::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);
	//Render grid
	m_pGraphRenderer->RenderGraph(m_pGridGraph, m_DebugSettings.DrawNodes, m_DebugSettings.DrawNodeNumbers, m_DebugSettings.DrawConnections, m_DebugSettings.DrawConnectionCosts);

	//Render costField
	if (m_DebugSettings.DrawCostField)
	{
		Vector2 textPos{1.f, static_cast<float>(m_CellSize) };

		for (int rowNr{}; rowNr < ROWS; ++rowNr)
		{
			for (int colNr{}; colNr < COLUMNS; ++colNr)
			{
				DEBUGRENDERER2D->DrawString(textPos, std::to_string(m_CostField[rowNr * COLUMNS + colNr]).c_str());
				textPos.x += m_CellSize;
			}
			textPos.y += m_CellSize;
			textPos.x = 1.f;
		}
	}

	//Render integrationField
	if (m_DebugSettings.DrawIntegrationField)
	{
		Vector2 textPos{ static_cast<float>(m_CellSize) - 3, static_cast<float>(m_CellSize) };

		for (int rowNr{}; rowNr < ROWS; ++rowNr)
		{
			for (int colNr{}; colNr < COLUMNS; ++colNr)
			{
				DEBUGRENDERER2D->DrawString(textPos, std::to_string(m_IntegrationField[rowNr * COLUMNS + colNr]).c_str());
				textPos.x += m_CellSize;
			}
			textPos.y += m_CellSize;
			textPos.x = static_cast<float>(m_CellSize) - 3;
		}
	}

	//Render vectorField
	if (m_DebugSettings.DrawVectorField)
	{
		Vector2 textPos{ 1.f,  5.f };

		for (int rowNr{}; rowNr < ROWS; ++rowNr)
		{
			for (int colNr{}; colNr < COLUMNS; ++colNr)
			{
				DrawArrow(m_pGridGraph->GetNodeWorldPos(rowNr * COLUMNS + colNr), m_VectorField[rowNr * COLUMNS + colNr]);
				textPos.x += m_CellSize;
			}
			textPos.y += m_CellSize;
			textPos.x = 1.f;
		}
	}

	//Render destination node on top if applicable
	if (m_DestinationNodeIndex != invalid_node_index)
	{
		m_pGraphRenderer->HighlightNodes(m_pGridGraph, { m_pGridGraph->GetNode(m_DestinationNodeIndex) }, START_NODE_COLOR);
	}
}

void App_FlowFields::MakeGridGraph()
{
	m_pGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(COLUMNS, ROWS, m_CellSize, true, true, 1.f, 1.5f);
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

		ImGui::Text("LMB: increase cost");
		ImGui::Text("RMB: decrease cost");
		ImGui::Text("MMB: set destination");		

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

		ImGui::Checkbox("Grid", &m_DebugSettings.DrawNodes);
		ImGui::Checkbox("Connections", &m_DebugSettings.DrawConnections);
		ImGui::Checkbox("CostField", &m_DebugSettings.DrawCostField);
		ImGui::Checkbox("IntegrationField", &m_DebugSettings.DrawIntegrationField);
		ImGui::Checkbox("VectorField", &m_DebugSettings.DrawVectorField);
		
		ImGui::SliderInt("Agents", &m_AmountOfAgents, 0, 2000);

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

void App_FlowFields::CalculateIntegrationField()
{
	if (m_DestinationNodeIndex == invalid_node_index) return;

	auto destinationNode{ m_pGridGraph->GetNode(m_DestinationNodeIndex) };

	//set the cost of all cells to a high number in the integration field
	for (int& cost : m_IntegrationField)
	{
		cost = 1000;
	}

	//create the openList
	std::deque<int> openList{};

	//set the cost of the destination node cost to 0
	m_IntegrationField[m_DestinationNodeIndex] = 0;

	//add the destinationNode to the openList
	openList.push_back(m_DestinationNodeIndex);

	while (!openList.empty())
	{
		//Get the next node form the openList
		int nodeIndex{ openList.front() };
		openList.pop_front();

		//Get neighbors of the node
		auto connections{ m_pGridGraph->GetNodeConnections(nodeIndex) };

		//loop over the neighbors
		for (const auto& connection : connections)
		{
			const int neighborIndex{ connection->GetTo() };

			//Calculate the new cost of the neighbor node
			//int cost{ m_IntegrationField[nodeIndex] + m_CostField[neighborIndex] };
			
		//	if (cost < m_IntegrationField[neighborIndex])
			{
				//check if the neighbor is already in the openList if not add it
				bool neighborIsInOpenList{ false };
				for (const auto& index : openList)
				{
					if (index == neighborIndex)
					{
						neighborIsInOpenList = true;
						break;
					}
				}

				if (!neighborIsInOpenList)
				{
					openList.push_back(neighborIndex);
				}

				//Set the cost of the neighbor
				//m_IntegrationField[neighborIndex] = cost;
			}
		}
	}
}

void App_FlowFields::CalculateVectorField()
{
	//loop over all the nodes in the integrationField
	for (size_t index{}; index < m_IntegrationField.size(); ++index)
	{
		//loop over the neighbors of the current node an find the cheapest neighbor
		int cheapestNeighborIndex{ invalid_node_index };
		int cheapestNeighborCost{ 1000 };
		const auto& connections{ m_pGridGraph->GetNodeConnections(index) };
		for (const auto& connection : connections)
		{
			if (m_IntegrationField[connection->GetTo()] < cheapestNeighborCost)
			{
				cheapestNeighborIndex = connection->GetTo();
				cheapestNeighborCost = m_IntegrationField[cheapestNeighborIndex];
			}
		}

		//set the correct direction of the current node
		if (cheapestNeighborIndex != invalid_node_index && cheapestNeighborCost != 1000)
		{
			if (cheapestNeighborIndex == index + COLUMNS)
			{
				m_VectorField[index] = VectorDirection::top;
			}
			else if (cheapestNeighborIndex == index + COLUMNS + 1)
			{
				m_VectorField[index] = VectorDirection::topRight;
			}
			else if (cheapestNeighborIndex == index + 1)
			{
				m_VectorField[index] = VectorDirection::right;
			}
			else if (cheapestNeighborIndex == index - COLUMNS + 1)
			{
				m_VectorField[index] = VectorDirection::bottomRight;
			}
			else if (cheapestNeighborIndex == index - COLUMNS)
			{
				m_VectorField[index] = VectorDirection::bottom;
			}
			else if (cheapestNeighborIndex == index - COLUMNS - 1)
			{
				m_VectorField[index] = VectorDirection::bottomLeft;
			}
			else if (cheapestNeighborIndex == index - 1)
			{
				m_VectorField[index] = VectorDirection::Left;
			}
			else if (cheapestNeighborIndex == index + COLUMNS - 1)
			{
				m_VectorField[index] = VectorDirection::topLeft;
			}
			else
			{
				m_VectorField[index] = VectorDirection::none;
			}
		}
	}

	//set the direction of the destination node to zero
	if (m_DestinationNodeIndex != invalid_node_index)
	{
		m_VectorField[m_DestinationNodeIndex] = VectorDirection::none;
	}
}

void App_FlowFields::HandleInput()
{
	bool const LMBPressed{ INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) };
	if (LMBPressed)
	{
		MouseData mouseData{ INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft) };
		Elite::Vector2 mousePos{ DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y }) };

		//Find closest node to click pos
		int indexclosestNode{ m_pGridGraph->GetNodeIdxAtWorldPos(mousePos) };

		if (indexclosestNode == invalid_node_index) return;

	//	if (m_CostField[indexclosestNode] + 1 < 256)
		{
		//	++m_CostField[indexclosestNode];
		//	if (m_CostField[indexclosestNode] == 255)
			{
				//m_pGridGraph->RemoveConnectionsToAdjacentNodes(indexclosestNode);
			}
			CalculateIntegrationField();
			CalculateVectorField();
		}
	}

	bool const RMBPressed{ INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eRight) };
	if (RMBPressed)
	{
		MouseData mouseData{ INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eRight) };
		Elite::Vector2 mousePos{ DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y }) };

		//Find closest node to click pos
		int indexclosestNode{ m_pGridGraph->GetNodeIdxAtWorldPos(mousePos) };

		if (indexclosestNode == invalid_node_index) return;

		//if (m_CostField[indexclosestNode] - 1 > 0)
		{
		//	if (m_CostField[indexclosestNode] == 255)
			{
				m_pGridGraph->AddConnectionsToAdjacentCells(indexclosestNode);
			}

		//	--m_CostField[indexclosestNode];
			CalculateIntegrationField();
			CalculateVectorField();
		}
	}

	bool const MMBPressed{ INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle) };
	if (MMBPressed)
	{
		MouseData mouseData{ INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos{ DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y }) };

		//Find closest node to click pos
		int indexclosestNode{ m_pGridGraph->GetNodeIdxAtWorldPos(mousePos) };

		if (indexclosestNode == invalid_node_index) return;
		
		m_DestinationNodeIndex = indexclosestNode;
		CalculateIntegrationField();
		CalculateVectorField();
	}
}

void App_FlowFields::DrawArrow(Vector2 cellMiddle, VectorDirection direction) const
{
	Vector2 arrowDirection{};
	std::vector<Vector2> triangleVertices{};
	Vector2 triangleStartPoint{};
	const float triangleSideLenght{ 4 };

	switch (direction)
	{
	case VectorDirection::none:
		return;

	case VectorDirection::top:
		arrowDirection = { 0.f, 1.f };
		triangleStartPoint.x = cellMiddle.x;
		triangleStartPoint.y = cellMiddle.y;
		triangleVertices.push_back(Vector2(triangleStartPoint.x - triangleSideLenght / 2.f, triangleStartPoint.y));
		triangleVertices.push_back(Vector2(triangleStartPoint.x, triangleStartPoint.y + triangleSideLenght));
		triangleVertices.push_back(Vector2(triangleStartPoint.x + triangleSideLenght / 2.f, triangleStartPoint.y));
		break;

	case VectorDirection::topRight:
		arrowDirection = { 1.f, 1.f };
		triangleStartPoint.x = cellMiddle.x;
		triangleStartPoint.y = cellMiddle.y;
		triangleVertices.push_back(Vector2(triangleStartPoint.x - triangleSideLenght / 2.f, triangleStartPoint.y + triangleSideLenght / 2.f));
		triangleVertices.push_back(Vector2(triangleStartPoint.x + triangleSideLenght, triangleStartPoint.y + triangleSideLenght));
		triangleVertices.push_back(Vector2(triangleStartPoint.x + triangleSideLenght / 2.f, triangleStartPoint.y - triangleSideLenght / 2.f));
		break;

	case VectorDirection::right:
		arrowDirection = { 1.f, 0.f };
		triangleStartPoint.x = cellMiddle.x;
		triangleStartPoint.y = cellMiddle.y;
		triangleVertices.push_back(Vector2(triangleStartPoint.x, triangleStartPoint.y + triangleSideLenght / 2.f));
		triangleVertices.push_back(Vector2(triangleStartPoint.x + triangleSideLenght, triangleStartPoint.y));
		triangleVertices.push_back(Vector2(triangleStartPoint.x, triangleStartPoint.y - triangleSideLenght / 2.f));
		break;

	case VectorDirection::bottomRight:
		arrowDirection = { 1.f, -1.f };
		triangleStartPoint.x = cellMiddle.x;
		triangleStartPoint.y = cellMiddle.y;
		triangleVertices.push_back(Vector2(triangleStartPoint.x - triangleSideLenght / 2.f, triangleStartPoint.y - triangleSideLenght / 2.f));
		triangleVertices.push_back(Vector2(triangleStartPoint.x + triangleSideLenght, triangleStartPoint.y - triangleSideLenght));
		triangleVertices.push_back(Vector2(triangleStartPoint.x + triangleSideLenght / 2.f, triangleStartPoint.y + triangleSideLenght / 2.f));
		break;

	case VectorDirection::bottom:
		arrowDirection = { 0.f, -1.f };
		triangleStartPoint.x = cellMiddle.x;
		triangleStartPoint.y = cellMiddle.y;
		triangleVertices.push_back(Vector2(triangleStartPoint.x + triangleSideLenght / 2.f, triangleStartPoint.y));
		triangleVertices.push_back(Vector2(triangleStartPoint.x, triangleStartPoint.y - triangleSideLenght));
		triangleVertices.push_back(Vector2(triangleStartPoint.x - triangleSideLenght / 2.f, triangleStartPoint.y));
		break;

	case VectorDirection::bottomLeft:
		arrowDirection = { -1.f, -1.f };
		triangleStartPoint.x = cellMiddle.x;
		triangleStartPoint.y = cellMiddle.y;
		triangleVertices.push_back(Vector2(triangleStartPoint.x - triangleSideLenght / 2.f, triangleStartPoint.y + triangleSideLenght / 2.f));
		triangleVertices.push_back(Vector2(triangleStartPoint.x - triangleSideLenght, triangleStartPoint.y - triangleSideLenght));
		triangleVertices.push_back(Vector2(triangleStartPoint.x + triangleSideLenght / 2.f, triangleStartPoint.y - triangleSideLenght / 2.f));
		break;

	case VectorDirection::Left:
		arrowDirection = { -1.f, 0.f };
		triangleStartPoint.x = cellMiddle.x;
		triangleStartPoint.y = cellMiddle.y;
		triangleVertices.push_back(Vector2(triangleStartPoint.x, triangleStartPoint.y - triangleSideLenght / 2.f));
		triangleVertices.push_back(Vector2(triangleStartPoint.x - triangleSideLenght, triangleStartPoint.y));
		triangleVertices.push_back(Vector2(triangleStartPoint.x, triangleStartPoint.y + triangleSideLenght / 2.f));
		break;

	case VectorDirection::topLeft:
		arrowDirection = { -1.f, 1.f };
		triangleStartPoint.x = cellMiddle.x;
		triangleStartPoint.y = cellMiddle.y;
		triangleVertices.push_back(Vector2(triangleStartPoint.x - triangleSideLenght / 2.f, triangleStartPoint.y - triangleSideLenght / 2.f));
		triangleVertices.push_back(Vector2(triangleStartPoint.x - triangleSideLenght, triangleStartPoint.y + triangleSideLenght));
		triangleVertices.push_back(Vector2(triangleStartPoint.x + triangleSideLenght / 2.f, triangleStartPoint.y + triangleSideLenght / 2.f));
		break;
	}

	
		DEBUGRENDERER2D->DrawPolygon(&Elite::Polygon(triangleVertices), Elite::Color(1.f, 1.f, 1.f));
	
}