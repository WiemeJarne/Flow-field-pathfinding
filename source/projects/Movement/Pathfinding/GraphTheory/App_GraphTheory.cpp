//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_GraphTheory.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EEularianPath.h"

using namespace Elite;
using namespace std;
//Destructor
App_GraphTheory::~App_GraphTheory()
{
	SAFE_DELETE(m_pGraph2D)
}

//Functions
void App_GraphTheory::Start()
{
	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(80.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(0, 0));
	DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(false);
	DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(false);

	m_pGraph2D = new Graph2D<GraphNode2D, GraphConnection2D>(false);
	m_pGraph2D->AddNode(new GraphNode2D(0, { 20, 30 }));
	m_pGraph2D->AddNode(new GraphNode2D(1, { -10, -10 }));
	m_pGraph2D->AddConnection(new GraphConnection2D(0, 1));

	srand(time(nullptr));

	m_Colors.push_back(DEFAULT_NODE_COLOR);
}

void App_GraphTheory::Update(float deltaTime)
{
	m_GraphEditor.UpdateGraph(m_pGraph2D);
	m_pGraph2D->SetConnectionCostsToDistance();

	const auto eulerFinder = EulerianPath<GraphNode2D, GraphConnection2D>(m_pGraph2D);
	Eulerianity euleriantity = eulerFinder.IsEulerian();

	auto path = eulerFinder.FindPath(euleriantity);

	// graph coloring
	m_pPathGraph = eulerFinder.GetGraph();
	
	for (auto& node : m_pGraph2D->GetAllNodes())
	{
		ChangeNodeColor(node);
	}

	switch (euleriantity)
	{
	case Elite::Eulerianity::notEulerian:
		cout << "not eulerian\n";
		break;
	case Elite::Eulerianity::semiEulerian:
		cout << "semi eulerian\n";
		break;
	case Elite::Eulerianity::eulerian:
		cout << "eulerian\n";
		break;
	default:
		break;
	}

	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
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
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Graph Theory");
		ImGui::Spacing();
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
	

}

void App_GraphTheory::Render(float deltaTime) const
{
	m_GraphRenderer.RenderGraph(m_pGraph2D, true, true);
}

bool App_GraphTheory::DoesNeighborsHaveSameColor(const Elite::GraphNode2D* pNode) const
{
	const auto connectionsCurrentNode{ m_pPathGraph->GetNodeConnections(pNode->GetIndex()) };
	for (const auto connection : connectionsCurrentNode)
	{
		const auto currentNodeNeighbor{ m_pPathGraph->GetNode(connection->GetTo()) };

		if (GetColorIndex(currentNodeNeighbor) == GetColorIndex(pNode))
		{
			return true;
		}
	}

	return false;
}

void App_GraphTheory::ChangeNodeColor(Elite::GraphNode2D* pNode)
{
	std::vector<Elite::Color> m_PossibleColorsCurrentNode{ m_Colors };

	// loop over all the colors in m_Colors
	for (const auto& color : m_Colors)
	{
		pNode->SetColor(color);
		
		// if a neighbor of the current node has the same color
		// then is the current color of the node no longer a possibility
		if (DoesNeighborsHaveSameColor(pNode))
		{
			m_PossibleColorsCurrentNode[GetColorIndex(pNode)] = { -1.f, -1.f, -1.f }; // set all color channels to -1 so the color is invalid
		}
	}

	for (const auto& color : m_PossibleColorsCurrentNode)
	{
		if (!AreColorsTheSame(color, { -1.f, -1.f, -1.f })) // if the color has -1 in all channels then the color is invalid
		{
			pNode->SetColor(color);
			return; // return when a possible color is foudn
		}
	}

	// when there are no possible colors make a new color and add it to the back of m_Colors vector
	AddNewRandomColor();
	// calculate the index of the new color
	int colorIndex{ static_cast<int>(m_Colors.size()) - 1 };
	
	// set the color of the node to the new color
	pNode->SetColor(m_Colors[colorIndex]);
}

int App_GraphTheory::GetColorIndex(const Elite::GraphNode2D* pNode) const
{
	const size_t amountOfColors{ m_Colors.size() };
	// loop over all the colors in the m_Colors vector
	// until the color of the node is the same as the color in the vector
	for (size_t index{}; index < amountOfColors; ++index)
	{
		if (AreColorsTheSame(m_Colors[index], pNode->GetColor()))
		{
			return static_cast<int>(index);
		}
	}

	// when the color of the current node is not in the m_Colors vector return an invalid index
	return invalid_node_index;
}

void App_GraphTheory::AddNewRandomColor()
{
	Elite::Color randomColor{ GenerateRandomColor() };

	// check if the random generated color already exists
	// if it already exists generate another random color
	// until a new color that does not yet exist is generated
	while (DoesColorExist(randomColor)) 
	{
		randomColor = GenerateRandomColor();
	}

	// add the new color to the back of the m_Colors vector
	m_Colors.push_back(randomColor);
}

Elite::Color App_GraphTheory::GenerateRandomColor() const
{
	const float randomR{ rand() % 100 / 100.f }; // generate a random nubmer between 0.f and 1.f
	const float randomG{ rand() % 100 / 100.f }; // generate a random nubmer between 0.f and 1.f
	const float randomB{ rand() % 100 / 100.f }; // generate a random nubmer between 0.f and 1.f

	return { randomR, randomG, randomB };
}

bool App_GraphTheory::DoesColorExist(const Elite::Color color) const
{
	const size_t amountOfColors{ m_Colors.size() };
	for (size_t index{}; index < amountOfColors; ++index)
	{
		if (AreColorsTheSame(m_Colors[index], color))
		{
			return true;
		}
	}
	return false;
}

bool App_GraphTheory::AreColorsTheSame(Elite::Color color1, Elite::Color color2) const
{
	if (color1.r == color2.r
		&& color1.g == color2.g
		&& color1.b == color2.b)
	{
		return true;
	}
	return false;
}