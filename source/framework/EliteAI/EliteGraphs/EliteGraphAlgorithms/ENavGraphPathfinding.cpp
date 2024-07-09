#include "stdafx.h"
#include "ENavGraphPathfinding.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EAStar.h"
#include "framework/EliteAI/EliteGraphs/EliteNavGraph/ENavGraph.h"

using namespace Elite;

std::vector<Vector2> NavMeshPathfinding::FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
{
	// Create the path to return
	std::vector<Vector2> finalPath;

	// Get the startTriangle and endTriangle from the start and end positions
	const Triangle* pStartTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos);
	const Triangle* pEndTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos);

	// Check if the startTriangle and endTriangle exist
	if (!pStartTriangle || !pEndTriangle) return finalPath;

	// If the start and end triangles are the same, return a direct path
	if (pStartTriangle == pEndTriangle)
	{
		finalPath.emplace_back(endPos);
		return finalPath;
	}

	// Clone the graph
	auto clonedGraph = pNavGraph->Clone();

	// Create the start NavGraphNode and add to the cloned graph
	NavGraphNode* pStartNode = new NavGraphNode(-1, startPos);
	clonedGraph->AddNode(pStartNode);

	// Loop over all the edges of the startTriangle and add connections
	for (int lineIdx : pStartTriangle->metaData.IndexLines)
	{
		const int nodeIdx{ pNavGraph->GetNodeIdFromLineIndex(lineIdx) };
		if (nodeIdx != invalid_node_id)
		{
			// Add a connection from the start node to the node on this edge
			const auto* node = clonedGraph->GetNode(nodeIdx);
			if (node)
			{
				clonedGraph->AddConnection(new GraphConnection(pStartNode->GetId(), nodeIdx, Elite::Distance(startPos, node->GetPosition())));
			}
		}
	}

	// Create the end NavGraphNode and add to the cloned graph
	NavGraphNode* pEndNode = new NavGraphNode(-1, endPos);
	clonedGraph->AddNode(pEndNode);

	// Loop over all the edges of the endTriangle and add connections
	for (int lineIdx : pEndTriangle->metaData.IndexLines)
	{
		const int nodeIdx = pNavGraph->GetNodeIdFromLineIndex(lineIdx);
		if (nodeIdx != invalid_node_id)
		{
			// Add a connection from the node on this edge to the end node
			const auto* node = clonedGraph->GetNode(nodeIdx);
			clonedGraph->AddConnection(new GraphConnection(nodeIdx, pEndNode->GetId(), Elite::Distance(endPos, node->GetPosition())));
		}
	}

	AStar pathfinder(clonedGraph.get(), HeuristicFunctions::Chebyshev);
	const auto path{ pathfinder.FindPath(pStartNode, pEndNode) };

	for (const auto& node : path)
	{
		finalPath.push_back(node->GetPosition());
	}

	//Debug Visualisation
	debugNodePositions = finalPath;

	//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
	const auto portals = SSFA::FindPortals(path, pNavGraph->GetNavMeshPolygon());
	finalPath = SSFA::OptimizePortals(portals);

	// Debug Visualisation
	debugPortals = portals;

	return finalPath;
}
