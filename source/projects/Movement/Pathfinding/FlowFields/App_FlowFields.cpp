//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_FlowFields.h"
#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"
#include "projects/Shared/NavigationColliderElement.h"

//Destructor
App_FlowFields::~App_FlowFields()
{
	for (auto& pAgent : m_vAgents)
	{
		SAFE_DELETE(pAgent)
	}
	m_vAgents.clear();

	for (auto& pWall : m_lWalls)
	{
		SAFE_DELETE(pWall);
	}
	m_lWalls.clear();

	SAFE_DELETE(m_pGridGraph);
	SAFE_DELETE(m_pGraphRenderer);
}

//Functions
void App_FlowFields::Start()
{
	m_pGraphRenderer = new Elite::GraphRenderer();

	m_WorldWidth = m_AmountOfColumns * static_cast<float>(m_CellSize);
	m_WorldHeight = m_AmountOfRows * static_cast<float>(m_CellSize);

	DetermineWorldPoints();

	//Create the agents
	Vector2 randomPos{};
	for (int index{}; index < m_AmountOfAgents; ++index)
	{
		m_vAgents.push_back(new SteeringAgent());
		m_vAgents[index]->SetMaxLinearSpeed(25.f);
		m_vAgents[index]->SetMass(0.f);
		m_vAgents[index]->SetAutoOrient(true);
		randomPos.x = static_cast<float>(rand() % static_cast<int>(m_WorldWidth));
		randomPos.y = static_cast<float>(rand() % static_cast<int>(m_WorldHeight));
	}

	//Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(135.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(m_WorldWidth / 1.5f, m_WorldHeight / 2));
	DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(false);
	DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(false);

	//Create Graph
	MakeGridGraph();

	//set all the cost in the costField to 1
	//fill the integrationField with a high number
	//fill the vectorField
	for (int rowNr{}; rowNr < m_AmountOfRows; ++rowNr)
	{
		for (int colNr{}; colNr < m_AmountOfColumns; ++colNr)
		{
			m_vCostField.push_back(1);
			m_vIntegrationField.push_back(1000);
			m_vVectorField.push_back(VectorDirection::none);
		}
	}
}

void App_FlowFields::Update(float deltaTime)
{
	//Input
	HandleInput();

	//Update agents
	for (const auto& agent : m_vAgents)
	{
		agent->Update(deltaTime);
		agent->TrimToWorld({0.f, 0.f}, {m_WorldWidth, m_WorldHeight});

		//find the index of the node where the agent is in
		const auto nodeIndex{ m_pGridGraph->GetNodeIdxAtWorldPos(agent->GetPosition()) };

		//if the nodeIndex is invalid continue to the next agent
		if (nodeIndex == invalid_node_index) continue;

		//determine and set the velocity of the agent
		switch (m_vVectorField[nodeIndex])
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

	//check if the amount of agents was changed
	if (m_AmountOfAgents != m_PreviousAmountOfAgents)
	{
		//delete all the agents
		for (auto pAgent : m_vAgents)
		{
			SAFE_DELETE(pAgent)
		}
		m_vAgents.clear();

		//Create new agents
		Vector2 randomPos{};
		for (int index{}; index < m_AmountOfAgents; ++index)
		{
			m_vAgents.push_back(new SteeringAgent());
			m_vAgents[index]->SetMaxLinearSpeed(15.f);
			m_vAgents[index]->SetMass(0.f);
			m_vAgents[index]->SetAutoOrient(true);
			randomPos.x = static_cast<float>(rand() % static_cast<int>(m_WorldWidth));
			randomPos.y = static_cast<float>(rand() % static_cast<int>(m_WorldHeight));
			m_vAgents[index]->SetPosition(randomPos);
		}

		m_PreviousAmountOfAgents = m_AmountOfAgents;
	}

	//check if the amount of columns or rows or the cellSize has changed
	if (m_AmountOfColumns != m_PreviousAmountOfColumns || m_AmountOfRows != m_PreviousAmountOfRows || m_CellSize != m_PreviousCellSize)
	{
		//delete the existing grid and make a new grid
		SAFE_DELETE(m_pGridGraph);
		MakeGridGraph();

		m_PreviousAmountOfColumns = m_AmountOfColumns;
		m_PreviousAmountOfRows = m_AmountOfRows;
		m_PreviousCellSize = m_CellSize;

		//clear the m_vDestinationNodesIndices vector to avoid a crash
		m_lDestinationNodesIndices.clear();

		//reset all the fields
		ResetFields();

		//recalculate the IntegrationField and VectorField
		CalculateIntegrationField();
		CalculateVectorField();

		//change the world datamembers
		m_WorldWidth = static_cast<float>(m_CellSize) * m_AmountOfColumns;
		m_WorldHeight = static_cast<float>(m_CellSize) * m_AmountOfRows;
		DetermineWorldPoints();

		//delete all the walls
		for (auto& wall : m_lWalls)
		{
			SAFE_DELETE(wall);
		}
		m_lWalls.clear();
	}
}

void App_FlowFields::Render(float deltaTime) const
{
	//Render grid
	m_pGraphRenderer->RenderGraph(m_pGridGraph, m_DebugSettings.DrawNodes, m_DebugSettings.DrawNodeNumbers, m_DebugSettings.DrawConnections, m_DebugSettings.DrawConnectionCosts);

	//Render costField
	if (m_DebugSettings.DrawCostField)
	{
		Vector2 textPos{1.f, static_cast<float>(m_CellSize) };

		for (int rowNr{}; rowNr < m_AmountOfRows; ++rowNr)
		{
			for (int colNr{}; colNr < m_AmountOfColumns; ++colNr)
			{
				DEBUGRENDERER2D->DrawString(textPos, std::to_string(m_vCostField[rowNr * m_AmountOfColumns + colNr]).c_str());
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

		for (int rowNr{}; rowNr < m_AmountOfRows; ++rowNr)
		{
			for (int colNr{}; colNr < m_AmountOfColumns; ++colNr)
			{
				DEBUGRENDERER2D->DrawString(textPos, std::to_string(m_vIntegrationField[rowNr * m_AmountOfColumns + colNr]).c_str());
				textPos.x += m_CellSize;
			}
			textPos.y += m_CellSize;
			textPos.x = static_cast<float>(m_CellSize) - 3;
		}
	}

	//Render vectorField
	if (m_DebugSettings.DrawVectorField)
	{
		for (size_t index{}; index < m_vVectorField.size(); ++index)
		{
			DrawArrow(m_pGridGraph->GetNodeWorldPos(index), m_vVectorField[index]);
		}
	}

	//Render destination nodes on top if applicable
	for (const auto& destinationNodeIndex : m_lDestinationNodesIndices)
	{
		m_pGraphRenderer->HighlightNodes(m_pGridGraph, { m_pGridGraph->GetNode(destinationNodeIndex) }, START_NODE_COLOR);
	}

	//Render world bounds
	DEBUGRENDERER2D->DrawPolygon(&Elite::Polygon(m_WorldPoints), Color(1.f, 1.f, 1.f));
}

void App_FlowFields::MakeGridGraph()
{
	m_pGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(m_AmountOfColumns, m_AmountOfRows, m_CellSize, true, true, 1.f, 1.5f);
}

void App_FlowFields::UpdateImGui()
{
#ifdef  PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth{ 170 };
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

		if (m_DebugSettings.DrawCostField)
		{
			ImGui::Text("LMB: increase cost");
			ImGui::Text("RMB: decrease cost");
		}
		else
		{
			ImGui::Text("LMB: add wall");
			ImGui::Text("RMB: remove wall");
		}

		ImGui::Text("MMB: set destination/\nremove destination");		

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Text("%.3f ms/frame", 1000.f, ImGui::GetIO().Framerate);
		ImGui::Text("%.01f FPS", ImGui::GetIO().Framerate);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Flow Field");
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Checkbox("Grid", &m_DebugSettings.DrawNodes);
		ImGui::Checkbox("Connections", &m_DebugSettings.DrawConnections);
		ImGui::Checkbox("CostField", &m_DebugSettings.DrawCostField);
		ImGui::Checkbox("IntegrationField", &m_DebugSettings.DrawIntegrationField);
		ImGui::Checkbox("VectorField", &m_DebugSettings.DrawVectorField);
		
		ImGui::SliderInt("Agents", &m_AmountOfAgents, 0, 2000);
		ImGui::SliderInt("Columns", &m_AmountOfColumns, 15, 50);
		ImGui::SliderInt("Rows", &m_AmountOfRows, 15, 50);
		ImGui::SliderInt("CellSize", &m_CellSize, 1, 25);

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

void App_FlowFields::CalculateIntegrationField()
{
	//set the cost of all cells to a high number in the integration field
	for (int& cost : m_vIntegrationField)
	{
		cost = 1000;
	}

	//if there are no destination nodes don't calculate the integrationField
	if (m_lDestinationNodesIndices.empty()) return;

	//create the openList
	std::deque<int> openList{};

	//set the cost of the destination nodes to 0 and add them to the openList
	for (const auto& destinationNodeIndex : m_lDestinationNodesIndices)
	{
		m_vIntegrationField[destinationNodeIndex] = 0;
		openList.push_back(destinationNodeIndex);
	}

	while (!openList.empty())
	{
		//Get the next node from the openList and remove it from the openList
		const int nodeIndex{ openList.front() };
		openList.pop_front();

		//Get the connections of the current node
		const auto& connections{ m_pGridGraph->GetNodeConnections(nodeIndex) };

		//loop over the connection
		for (const auto& connection : connections)
		{
			//get the index of the neighbor
			const int neighborIndex{ connection->GetTo() };

			//Calculate the new cost of the neighbor node
			int cost{ m_vIntegrationField[nodeIndex] + m_vCostField[neighborIndex] };
			
			//check if the new cost is cheaper then the current cost of the neighbor
			if (cost < m_vIntegrationField[neighborIndex])
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
				m_vIntegrationField[neighborIndex] = cost;
			}
		}
	}
}

void App_FlowFields::CalculateVectorField()
{
	//loop over all the nodes in the integrationField
	for (size_t index{}; index < m_vIntegrationField.size(); ++index)
	{
		//if there are no destination nodes set the vectors of all cells to none
		if (m_lDestinationNodesIndices.empty())
		{
			for (auto& direction : m_vVectorField)
			{
				direction = VectorDirection::none;
			}
		}

		//loop over the neighbors of the current node an find the cheapest neighbor
		int cheapestNeighborIndex{ invalid_node_index };
		int cheapestNeighborCost{ 1000 };
		const auto& connections{ m_pGridGraph->GetNodeConnections(index) };
		for (const auto& connection : connections)
		{
			if (m_vIntegrationField[connection->GetTo()] < cheapestNeighborCost)
			{
				cheapestNeighborIndex = connection->GetTo();
				cheapestNeighborCost = m_vIntegrationField[cheapestNeighborIndex];
			}
		}

		//set the correct direction of the current node in the vectorField
		if (cheapestNeighborIndex != invalid_node_index && cheapestNeighborCost != 1000)
		{
			if (cheapestNeighborIndex == index + m_AmountOfColumns)
			{
				m_vVectorField[index] = VectorDirection::top;
			}
			else if (cheapestNeighborIndex == index + m_AmountOfColumns + 1)
			{
				m_vVectorField[index] = VectorDirection::topRight;
			}
			else if (cheapestNeighborIndex == index + 1)
			{
				m_vVectorField[index] = VectorDirection::right;
			}
			else if (cheapestNeighborIndex == index - m_AmountOfColumns + 1)
			{
				m_vVectorField[index] = VectorDirection::bottomRight;
			}
			else if (cheapestNeighborIndex == index - m_AmountOfColumns)
			{
				m_vVectorField[index] = VectorDirection::bottom;
			}
			else if (cheapestNeighborIndex == index - m_AmountOfColumns - 1)
			{
				m_vVectorField[index] = VectorDirection::bottomLeft;
			}
			else if (cheapestNeighborIndex == index - 1)
			{
				m_vVectorField[index] = VectorDirection::Left;
			}
			else if (cheapestNeighborIndex == index + m_AmountOfColumns - 1)
			{
				m_vVectorField[index] = VectorDirection::topLeft;
			}
			else
			{
				m_vVectorField[index] = VectorDirection::none;
			}
		}
	}

	//set the direction of the destination nodes and all the walls to zero
	for (const auto& destinationNodeIndex : m_lDestinationNodesIndices)
	{
		m_vVectorField[destinationNodeIndex] = VectorDirection::none;
	}

	for (size_t index{}; index < m_vCostField.size(); ++index)
	{
		if (m_vCostField[index] == 255)
		{
			m_vVectorField[index] = VectorDirection::none;
		}
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

		if (m_vCostField[indexclosestNode] + 1 < 256 && m_DebugSettings.DrawCostField == true)
		{
			++m_vCostField[indexclosestNode];
			
			CalculateIntegrationField();
			CalculateVectorField();
		}
		else //if the costField is not drawn then add a wall when clicking on a cell
		{
			m_vCostField[indexclosestNode] = 255;
		}

		if (m_vCostField[indexclosestNode] == 255)
		{
			m_pGridGraph->RemoveConnectionsToAdjacentNodes(indexclosestNode);
			AddWall(m_pGridGraph->GetNodeWorldPos(indexclosestNode));

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

		if (m_vCostField[indexclosestNode] - 1 > 0 && m_DebugSettings.DrawCostField)
		{
			--m_vCostField[indexclosestNode];

			CalculateIntegrationField();
			CalculateVectorField();
		}
		else //when the costField isn't drawn remove the wall if click on a wall
		{
			const Vector2 nodePos{ m_pGridGraph->GetNodeWorldPos(indexclosestNode) };

			//loop over the walls and check if the player clicked on a wall if so delete the wall and set the cost to 1
			for (auto& wall : m_lWalls)
			{
				if (wall->GetPosition() == nodePos)
				{
					SAFE_DELETE(wall);
					m_lWalls.remove(wall);
					m_vCostField[indexclosestNode] = 1;
					break;
				}
			}
		}

		if (m_vCostField[indexclosestNode] <= 254)
		{
			//add the connections to the neighbors
			m_pGridGraph->AddConnectionsToAdjacentCells(indexclosestNode);

			//loop over all the cells to delete their connections if the cell is a wall
			const int amountOfNodex{ static_cast<int>(m_pGridGraph->GetAllNodes().size()) };
			for (int index{}; index < amountOfNodex; ++index)
			{
				if (m_vCostField[index] == 255)
				{
					m_pGridGraph->RemoveConnectionsToAdjacentNodes(index);
					continue;
				}
			}

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
		
		//check if this index already is in the m_vDestinationNodesIndices if so remove it if not add it
		bool nodeIsAlreadyDestination{};
		for (const auto& destinationNodeIndex : m_lDestinationNodesIndices)
		{
			if (destinationNodeIndex == indexclosestNode)
			{
				m_lDestinationNodesIndices.remove(destinationNodeIndex);
				nodeIsAlreadyDestination = true;
				break;
			}
		}

		if (!nodeIsAlreadyDestination)
		{
			m_lDestinationNodesIndices.push_back(indexclosestNode);
		}

		CalculateIntegrationField();
		CalculateVectorField();
	}
}

void App_FlowFields::AddWall(Vector2 pos)
{
	bool wallAlreadyExists{ false };
	for (const auto& wall : m_lWalls)
	{
		if (wall->GetPosition() == pos)
		{
			wallAlreadyExists = true;
		}
	}

	if (!wallAlreadyExists)
	{
		m_lWalls.push_back(new NavigationColliderElement(pos, static_cast<float>(m_CellSize), static_cast<float>(m_CellSize)));
	}
}

void App_FlowFields::DrawArrow(Vector2 cellMiddle, VectorDirection direction) const
{
	std::vector<Vector2> triangleVertices{};
	Vector2 triangleStartPoint{};
	const float triangleSideLenght{ m_CellSize / 4.f };

	//calculate the correct point for the arrow
	switch (direction)
	{
	case VectorDirection::none:
		return;

	case VectorDirection::top:
		triangleVertices.push_back(Vector2(cellMiddle.x - triangleSideLenght / 2.f, cellMiddle.y));
		triangleVertices.push_back(Vector2(cellMiddle.x, cellMiddle.y + triangleSideLenght));
		triangleVertices.push_back(Vector2(cellMiddle.x + triangleSideLenght / 2.f, cellMiddle.y));
		break;

	case VectorDirection::topRight:
		triangleVertices.push_back(Vector2(cellMiddle.x - triangleSideLenght / 2.f, cellMiddle.y + triangleSideLenght / 2.f));
		triangleVertices.push_back(Vector2(cellMiddle.x + triangleSideLenght, cellMiddle.y + triangleSideLenght));
		triangleVertices.push_back(Vector2(cellMiddle.x + triangleSideLenght / 2.f, cellMiddle.y - triangleSideLenght / 2.f));
		break;

	case VectorDirection::right:
		triangleVertices.push_back(Vector2(cellMiddle.x, cellMiddle.y + triangleSideLenght / 2.f));
		triangleVertices.push_back(Vector2(cellMiddle.x + triangleSideLenght, cellMiddle.y));
		triangleVertices.push_back(Vector2(cellMiddle.x, cellMiddle.y - triangleSideLenght / 2.f));
		break;

	case VectorDirection::bottomRight:
		triangleVertices.push_back(Vector2(cellMiddle.x - triangleSideLenght / 2.f, cellMiddle.y - triangleSideLenght / 2.f));
		triangleVertices.push_back(Vector2(cellMiddle.x + triangleSideLenght, cellMiddle.y - triangleSideLenght));
		triangleVertices.push_back(Vector2(cellMiddle.x + triangleSideLenght / 2.f, cellMiddle.y + triangleSideLenght / 2.f));
		break;

	case VectorDirection::bottom:
		triangleVertices.push_back(Vector2(cellMiddle.x + triangleSideLenght / 2.f, cellMiddle.y));
		triangleVertices.push_back(Vector2(cellMiddle.x, cellMiddle.y - triangleSideLenght));
		triangleVertices.push_back(Vector2(cellMiddle.x - triangleSideLenght / 2.f, cellMiddle.y));
		break;

	case VectorDirection::bottomLeft:
		triangleVertices.push_back(Vector2(cellMiddle.x - triangleSideLenght / 2.f, cellMiddle.y + triangleSideLenght / 2.f));
		triangleVertices.push_back(Vector2(cellMiddle.x - triangleSideLenght, cellMiddle.y - triangleSideLenght));
		triangleVertices.push_back(Vector2(cellMiddle.x + triangleSideLenght / 2.f, cellMiddle.y - triangleSideLenght / 2.f));
		break;

	case VectorDirection::Left:
		triangleVertices.push_back(Vector2(cellMiddle.x, cellMiddle.y - triangleSideLenght / 2.f));
		triangleVertices.push_back(Vector2(cellMiddle.x - triangleSideLenght, cellMiddle.y));
		triangleVertices.push_back(Vector2(cellMiddle.x, cellMiddle.y + triangleSideLenght / 2.f));
		break;

	case VectorDirection::topLeft:
		triangleVertices.push_back(Vector2(cellMiddle.x - triangleSideLenght / 2.f, cellMiddle.y - triangleSideLenght / 2.f));
		triangleVertices.push_back(Vector2(cellMiddle.x - triangleSideLenght, cellMiddle.y + triangleSideLenght));
		triangleVertices.push_back(Vector2(cellMiddle.x + triangleSideLenght / 2.f, cellMiddle.y + triangleSideLenght / 2.f));
		break;
	}
	
	DEBUGRENDERER2D->DrawPolygon(&Elite::Polygon(triangleVertices), Elite::Color(1.f, 1.f, 1.f));
}

void App_FlowFields::DetermineWorldPoints()
{
	m_WorldPoints.clear();
	m_WorldPoints.push_back({ 0.f, 0.f });
	m_WorldPoints.push_back({ 0.f, m_WorldHeight });
	m_WorldPoints.push_back({ m_WorldWidth, m_WorldHeight });
	m_WorldPoints.push_back({ m_WorldWidth, 0.f });
}

void App_FlowFields::ResetFields()
{
	m_vCostField.resize(m_AmountOfColumns * m_AmountOfRows);
	for (int& cost : m_vCostField)
	{
		cost = 1;
	}

	m_vIntegrationField.resize(m_AmountOfColumns * m_AmountOfRows);
	for (int& cost : m_vIntegrationField)
	{
		cost = 1000;
	}

	m_vVectorField.resize(m_AmountOfColumns * m_AmountOfRows);
}