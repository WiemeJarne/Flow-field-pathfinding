#ifndef INFLUENCE_MAP_APPLICATION_H
#define INFLUENCE_MAP_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"

#include "framework\EliteAI\EliteGraphs\EGraphNodeTypes.h"
#include "framework\EliteAI\EliteGraphs\EGraphConnectionTypes.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EGraph2D.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"


//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_GraphTheory final : public IApp
{
public:
	//Constructor & Destructor
	App_GraphTheory() = default;
	virtual ~App_GraphTheory() final;

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	Elite::Graph2D<Elite::GraphNode2D, Elite::GraphConnection2D>* m_pGraph2D;

	Elite::GraphRenderer m_GraphRenderer{};
	Elite::GraphEditor m_GraphEditor{};

	std::vector<Elite::Color> m_Colors{};

	Elite::IGraph<Elite::GraphNode2D, Elite::GraphConnection2D>* m_pPathGraph{};

	void AddNewRandomColor();
	Elite::Color GenerateRandomColor() const;
	bool DoesColorExist(const Elite::Color color) const;
	int GetColorIndex(const Elite::GraphNode2D* pNode) const;
	bool DoesNeighborsHaveSameColor(const Elite::GraphNode2D* pNode) const;
	void ChangeNodeColor(Elite::GraphNode2D* pNode);
	bool AreColorsTheSame(Elite::Color color1, Elite::Color color2) const;

	//C++ make the class non-copyable
	App_GraphTheory(const App_GraphTheory&) = delete;
	App_GraphTheory& operator=(const App_GraphTheory&) = delete;
};
#endif