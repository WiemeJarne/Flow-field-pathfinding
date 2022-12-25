#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, const NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};
			
			//Get the start and endTriangle
			const Triangle* pStartTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos) };
			const Triangle* pEndTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos) };
		
			//We have valid start/end triangles and they are not the same
			if (pStartTriangle == nullptr || pEndTriangle == nullptr) return finalPath;
			if (pStartTriangle == pEndTriangle)
			{
				finalPath.push_back(endPos);
				return finalPath;
			}

			//=> Start looking for a path
			//Copy the graph
			auto graph{ pNavGraph->Clone() };
			
			const auto& lines{ pNavGraph->GetNavMeshPolygon()->GetLines() };

			//Create extra node for the Start Node (Agent's position)
			const auto& startNode{ new NavGraphNode(graph->GetNextFreeNodeIndex(), -1, startPos) };

			graph->AddNode(startNode );

			for(int lineIndex : pStartTriangle->metaData.IndexLines)
			{
				const int nodeIndex{ pNavGraph->GetNodeIdxFromLineIdx(lineIndex) };

				if (nodeIndex == invalid_node_index) continue; //check if there is a node on the line
				
				const auto& node{ graph->GetNode(nodeIndex) }; //get the node

				//create a connection from the startNode node
				GraphConnection2D* pNewConnection{ new GraphConnection2D(startNode->GetIndex(), nodeIndex, Distance(startPos, node->GetPosition())) };
				graph->AddConnection(pNewConnection);
			}

			//Create extra node for the endNode
			const auto& endNode{ new NavGraphNode(graph->GetNextFreeNodeIndex(), -1, endPos) };

			graph->AddNode(endNode);

			for (int lineIndex : pEndTriangle->metaData.IndexLines)
			{
				const int nodeIndex{ pNavGraph->GetNodeIdxFromLineIdx(lineIndex) };

				if (nodeIndex == invalid_node_index) continue; //check if there is a node on the line

				const auto& node{ graph->GetNode(nodeIndex) }; //get the node

				//create a connection from the node to the end node
				GraphConnection2D* pNewConnection{ new GraphConnection2D(nodeIndex, endNode->GetIndex(), Distance(endPos, node->GetPosition())) };
				graph->AddConnection(pNewConnection);
			}

			//Run A star on new graph
			auto pathfinder = AStar<NavGraphNode, GraphConnection2D>(graph.get(), HeuristicFunctions::Chebyshev);
			const auto path{ pathfinder.FindPath(startNode, endNode) };

			for (const auto& node : path)
			{
				finalPath.push_back(node->GetPosition());
			}

			//OPTIONAL BUT ADVICED: Debug Visualisation
			debugNodePositions = finalPath;

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			auto portals{ SSFA::FindPortals(path, pNavGraph->GetNavMeshPolygon()) };
			debugPortals = portals;
			finalPath = SSFA::OptimizePortals(portals);

			return finalPath;
		}
	};
}