#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_CellWidth(width / cols)
	, m_CellHeight(height / rows)
{
	float left{};
	float bottom{};

	for (int rowNr{}; rowNr < rows; ++rowNr)
	{
		for (int colNr{}; colNr < cols; ++colNr)
		{
			m_Cells.push_back(Cell(left, bottom, m_CellWidth, m_CellHeight));
			left += m_CellWidth;
		}
		left = 0.f;
		bottom += m_CellHeight;
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	const int index{ PositionToIndex(agent->GetPosition()) };
	m_Cells[index].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent)
{
	const int newIndex{ PositionToIndex(agent->GetPosition()) };
	const int previousIndex{ PositionToIndex(agent->GetPreviousPos()) };

	if(newIndex != previousIndex)
	{
		m_Cells[previousIndex].agents.remove(agent);

		m_Cells[newIndex].agents.push_back(agent);
	}
}

void CellSpace::RegisterNeighbors(const SteeringAgent* agent, float queryRadius, std::vector<SteeringAgent*>& neighbors, int& nrOfNeighbors) const
{
	nrOfNeighbors = 0;

	const Elite::Vector2 agentPos{ agent->GetPosition() };

	const int bottomLeftCellIndex{ PositionToIndex({agentPos.x - queryRadius, agentPos.y - queryRadius}) };
	const int topRightCellIndex{ PositionToIndex({agentPos.x + queryRadius, agentPos.y + queryRadius}) };

	const int bottomLeftCellRowNr = bottomLeftCellIndex % m_NrOfCols;
	const int bottomLeftCellColNr = bottomLeftCellIndex / m_NrOfCols;

	const int topRightCellRowNr = topRightCellIndex % m_NrOfCols;
	const int topRightCellColNr = topRightCellIndex / m_NrOfCols;

	for (int rowNr{ bottomLeftCellRowNr }; rowNr <= topRightCellRowNr; ++rowNr)
	{
		for (int colNr{ bottomLeftCellColNr }; colNr <= topRightCellColNr; ++colNr)
		{
			const int cellIndex{ rowNr + (colNr * m_NrOfCols) };

			const Cell cell{ m_Cells[cellIndex] };
			
			for (SteeringAgent* otherAgent : cell.agents)
			{
				if (otherAgent != agent && (otherAgent->GetPosition() - agent->GetPosition()).Magnitude() <= queryRadius)
				{
					neighbors[nrOfNeighbors] = otherAgent;
					++nrOfNeighbors;
				}
			}

			if(agent->CanRenderBehavior())
			{
				DEBUGRENDERER2D->DrawPolygon(&Elite::Polygon(cell.GetRectPoints()), Elite::Color{1.f, 1.f, 0.f}, 0.f);
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{
	Elite::Vector2 textPos{ 0, m_CellHeight };
	for (size_t index{}; index < m_Cells.size(); ++index)
	{
		DEBUGRENDERER2D->DrawPolygon(&Elite::Polygon(m_Cells[index].GetRectPoints()), {1.f, 0.f, 0.f}, 0.f);

		const int amountOfAgentsInCell{ static_cast<int>(m_Cells[index].agents.size()) };
		DEBUGRENDERER2D->DrawString( textPos, std::to_string(amountOfAgentsInCell).c_str());
		textPos.x += m_CellWidth;
		if((index + 1) % m_NrOfCols == 0 && index != 0)
		{
			textPos.x = 0;
			textPos.y += m_CellHeight;
		}
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	for (size_t index{}; index < m_Cells.size(); ++index)
	{
		const Elite::Rect boundingBox{ m_Cells[index].boundingBox };

		if (pos.x >= boundingBox.bottomLeft.x && pos.x <= boundingBox.bottomLeft.x + m_CellWidth
			&& pos.y >= boundingBox.bottomLeft.y && pos.y <= boundingBox.bottomLeft.y + m_CellHeight)
		{
			return static_cast<int>(index);
		}
	}
	return 0;
}