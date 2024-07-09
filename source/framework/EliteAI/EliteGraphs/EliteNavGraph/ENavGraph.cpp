#include "stdafx.h"
#include "ENavGraph.h"
#include "../EliteGraph/EGraphNode.h"
#include "../EliteGraph/EGraphConnection.h"
#include "../EliteGraphNodeFactory/EGraphNodeFactory.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"


using namespace Elite;

NavGraph::NavGraph(const std::vector<Polygon>& colliderShapes, float widthWorld, float heightWorld, float playerRadius = 1.0f) :
	Graph(false, new GraphNodeFactoryTemplate<NavGraphNode>()),
	m_pNavMeshPolygon(nullptr)
{
	float const halfWidth = widthWorld / 2.0f;
	float const halfHeight = heightWorld / 2.0f;
	std::list<Vector2> baseBox
	{ { -halfWidth, halfHeight },{ -halfWidth, -halfHeight },{ halfWidth, -halfHeight },{ halfWidth, halfHeight } };

	m_pNavMeshPolygon = new Polygon(baseBox); // Create copy on heap

	//Store all children
	for (auto p : colliderShapes)
	{
		p.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(p);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

NavGraph::NavGraph(const NavGraph& other): Graph(other)
{
}

NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon;
	m_pNavMeshPolygon = nullptr;
}

std::shared_ptr<NavGraph> NavGraph::Clone()
{
	return std::shared_ptr<NavGraph>(new NavGraph(*this));
}

int NavGraph::GetNodeIdFromLineIndex(int lineIdx) const
{

	for (auto& pNode : m_pNodes)
	{
		if (reinterpret_cast<NavGraphNode*>(pNode)->GetLineIndex() == lineIdx)
		{
			return pNode->GetId();
		}
	}

	return invalid_node_id;
}

Elite::Polygon* NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create a node on the center of each edge
	auto lines = m_pNavMeshPolygon->GetLines();
	for (const auto* line : lines)
	{
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(line->index).size() == 2)
		{
			this->AddNode(new NavGraphNode(line->index, (line->p1 + line->p2) * 0.5f));
		}
	}


	//2  Now that every node is created, connect the nodes that share the same triangle (for each triangle, ... )

	const auto& triangles = m_pNavMeshPolygon->GetTriangles();
	for (const auto* triangle : triangles)
	{
		std::vector<int> tempValidNodes{};

		//2.1 Loop over the line indexes
		for (int lineIdx : triangle->metaData.IndexLines)
		{
			const int nodeId = this->GetNodeIdFromLineIndex(lineIdx);

			//2.2 Check if a valid NavGraphNode for that lineIdx exists
			if (GetNodeIdFromLineIndex(lineIdx) != invalid_node_id)
			{
				tempValidNodes.emplace_back(nodeId);
			}
		}

			//2.3 If we have 2 valid nodes > we create 1 connection between them
			//	  If we have 3 valid nodes > we create 3 connections between them
		if (tempValidNodes.size() == 2)
		{
			AddConnection(new GraphConnection(tempValidNodes[0], tempValidNodes[1]));

		}
		else if (tempValidNodes.size() == 3)
		{
			AddConnection(new GraphConnection(tempValidNodes[0], tempValidNodes[1]));
			AddConnection(new GraphConnection(tempValidNodes[1], tempValidNodes[2]));
			AddConnection(new GraphConnection(tempValidNodes[2], tempValidNodes[0]));
		}

	}

	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistances();
}