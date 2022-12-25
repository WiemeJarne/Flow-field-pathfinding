#pragma once
#include "framework/EliteAI/EliteNavigation/ENavigation.h"

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		std::vector<T_NodeType*> path{};
		std::vector<NodeRecord> openList{};
		std::vector<NodeRecord> closedList{};
		NodeRecord currentNodeRecord
		{
			pStartNode,
			nullptr,
			0.f,
			GetHeuristicCost(pStartNode, pGoalNode)
		};
		
		openList.push_back(currentNodeRecord);

		while (!openList.empty())
		{
			currentNodeRecord = *std::min_element(openList.begin(), openList.end());

			if (currentNodeRecord.pNode == pGoalNode) break;

			const auto connectionsCurrentNode{ m_pGraph->GetNodeConnections(currentNodeRecord.pNode) };

			for (const auto& connection : connectionsCurrentNode)
			{
				const float costSoFar{ currentNodeRecord.costSoFar + connection->GetCost() };

				//check if the connection is pointing to a node that was previously visited
				bool isConnectionInClosedList{};
				for (const auto& nodeRecord : closedList)
				{
					if (nodeRecord.pNode->GetIndex() == connection->GetTo())
					{
						//check if the new connection is cheaper the the already existing one
						if (costSoFar < nodeRecord.costSoFar)
						{
							isConnectionInClosedList = true;
							//if the new connection is cheaper remove the old one
							closedList.erase(std::remove(closedList.begin(), closedList.end(), nodeRecord));
							break;
						}
					}
				}

				if (isConnectionInClosedList) continue;

				//if the connection is not in the closedList check if it is in the openList
				bool isConnectionInOpenList{};

				for (const auto& nodeRecord : openList)
				{
					if (nodeRecord.pNode->GetIndex() == connection->GetTo())
					{
						//check if the new connection is cheaper the the already existing one
						if (costSoFar < nodeRecord.costSoFar)
						{
							isConnectionInOpenList = true;
							//if the new connection is cheaper remove the old one
							openList.erase(std::remove(openList.begin(), openList.end(), nodeRecord));
							break;
						}
					}
				}

				if (isConnectionInOpenList) continue;

				openList.push_back
				(
					NodeRecord
					{
						m_pGraph->GetNode(connection->GetTo()),
						connection,
						costSoFar,
						costSoFar + GetHeuristicCost(m_pGraph->GetNode(connection->GetTo()), pGoalNode)
					}
				);
				
			}
			//remove the connection from the openList and add it to the closedList
			openList.erase(std::remove(openList.begin(), openList.end(), currentNodeRecord));
			closedList.push_back(currentNodeRecord);
		}

		//backtracking
		while (currentNodeRecord.pNode != pStartNode)
		{
			path.push_back(currentNodeRecord.pNode);
			
			for (const auto& nodeRecord : closedList)
			{
				if (currentNodeRecord.pConnection->GetFrom() == nodeRecord.pNode->GetIndex())
				{
					currentNodeRecord = nodeRecord;
					break;
				}
			}
		}

		path.push_back(pStartNode);

		std::reverse(path.begin(), path.end());

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}