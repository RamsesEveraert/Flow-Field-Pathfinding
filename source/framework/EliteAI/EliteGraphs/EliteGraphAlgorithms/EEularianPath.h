#pragma once
#include <stack>
#include "../EliteGraph/EGraph.h"
#include "../EliteGraph/EGraphConnection.h"
#include "../EliteGraph/EGraphNode.h"
namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	class EulerianPath
	{
	public:

		EulerianPath(Graph* pGraph);

		Eulerianity IsEulerian() const;
		std::vector<GraphNode*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(const std::vector<GraphNode*>& pNodes, std::vector<bool>& visited, int startIndex) const;
		bool IsConnected() const;

		Graph* m_pGraph;
	};

	inline EulerianPath::EulerianPath(Graph* pGraph)
		: m_pGraph(pGraph)
	{
	}

	inline Eulerianity EulerianPath::IsEulerian() const
	{

		// If the graph is not connected, there can be no Eulerian Trail

		std::cout << "Is the graph connected? \n " << (IsConnected() ? "Yes" : "No") << std::endl;

		if (!IsConnected())
		{
			return Eulerianity::notEulerian;
		}

		// Count nodes with odd degree 

		auto nodes = m_pGraph->GetAllNodes();
		int oddCount{};

		for (const auto& node : nodes)
		{
			auto connections = m_pGraph->GetConnectionsFromNode(node);
			int amountConnections{ static_cast<int>(connections.size()) };

			if (amountConnections % 2 == 1)
			{
				++oddCount;
			}
		}

		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (oddCount > 2 || oddCount == 1)
		{
			return Eulerianity::notEulerian;
		}

		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (unless there are only 2 nodes)
		// An Euler trail can be made, but only starting and ending in these 2 nodes
		if (oddCount == 2)
		{
			return Eulerianity::semiEulerian;
		}

		// A connected graph with no odd nodes is Eulerian
		return Eulerianity::eulerian;

	}

	inline std::vector<GraphNode*> EulerianPath::FindPath(Eulerianity& eulerianity) const
	{
		// 1. Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy{ m_pGraph->Clone() };

		std::stack<GraphNode*> nodeStack{};
		std::vector<GraphNode*> path{};

		// 2. Check if there can be an Euler path
		auto nodes{ graphCopy->GetAllNodes() };
		GraphNode* firstNode{};
		GraphNode* currentNode{};
		std::vector<GraphNode*> oddDegreeNodes{};
		int oddCount{};

		// Add nodes with odd degree to the vector
		for (auto& node : nodes)
		{
			if (!node) continue; // Skip nullptr nodes

			if (graphCopy->GetConnectionsFromNode(node->GetId()).size() % 2 == 1)
			{
				oddDegreeNodes.push_back(node);
			}
		}

		//  2.1 Find a valid starting index for the algorithm
		if (eulerianity == Eulerianity::eulerian && oddDegreeNodes.empty())
		{
			currentNode = nodes[0];
			firstNode = nodes[0];
		}
		else if (eulerianity == Eulerianity::semiEulerian)
		{
			currentNode = oddDegreeNodes[0];
			firstNode = oddDegreeNodes[0];
		}
		else
		{
			// 2.2 If this graph is not eulerian, return the empty path
			return {};
		}

		// 4. Start algorithm loop
		while (graphCopy->GetConnectionsFromNode(currentNode->GetId()).size() > 0 || !nodeStack.empty())
		{
			auto connections{ graphCopy->GetConnectionsFromNode(currentNode->GetId()) };
			if (!connections.empty())
			{
				// Push current node on the stack
				if(currentNode)
				nodeStack.push(currentNode);

				// Choose a neighbor
				int neighborId = connections[0]->GetToNodeId();
				GraphNode* neighbor = graphCopy->GetNode(neighborId);

				if (!neighbor) continue; // Skip nullptr neighbors

				// Remove the edge connecting the current node and the chosen neighbor
				graphCopy->RemoveConnection(currentNode->GetId(), neighbor->GetId());

				// neighbor is current Node
				currentNode = neighbor;
			}
			else
			{
				// Add current node to the path
				path.push_back(m_pGraph->GetNode(currentNode->GetId()));

				// Set current node to the node on the top of the stack and pop it
				currentNode = nodeStack.top();
				nodeStack.pop();
			}
		}
		path.push_back(m_pGraph->GetNode(firstNode->GetId()));


		// The obtained path will be in reverse order, so reverse it before returning
		std::reverse(path.begin(), path.end());
		return path;
	}

	void EulerianPath::VisitAllNodesDFS(const std::vector<GraphNode*>& pNodes, std::vector<bool>& visited, int startIndex) const
	{
		// Check if startIndex is within the bounds of the visited vector
		if (startIndex < 0 || startIndex >= static_cast<int>(visited.size()))
		{
			std::cerr << "Invalid startIndex in VisitAllNodesDFS: " << startIndex << std::endl;
			return;
		}

		// Mark the node as visited
		visited[startIndex] = true;
		std::cout << "Visiting node: " << startIndex << std::endl;

		// Get all connections from the current node
		auto connections = m_pGraph->GetConnectionsFromNode(pNodes[startIndex]);

		// Recursively visit any valid connected nodes that were not visited before
		for (const auto& connection : connections)
		{
			int nextNodeIndex;
			if (connection->GetFromNodeId() == pNodes[startIndex]->GetId())
			{
				nextNodeIndex = connection->GetToNodeId();
			}
			else
			{
				nextNodeIndex = connection->GetFromNodeId();
			}

			// Check if nextNodeIndex is within the bounds of the visited vector
			if (nextNodeIndex >= 0 && nextNodeIndex < static_cast<int>(visited.size()) && !visited[nextNodeIndex])
			{
				VisitAllNodesDFS(pNodes, visited, nextNodeIndex);
			}
		}
	}




	bool EulerianPath::IsConnected() const
	{
		auto nodes = m_pGraph->GetAllNodes();
		std::vector<bool> visitedNodes(nodes.size(), false);

		// Find a valid starting node that has connections
		int connectedIndex = invalid_node_id;
		for (const auto& n : nodes)
		{
			auto connections = m_pGraph->GetConnectionsFromNode(n);
			if (!connections.empty())
			{
				connectedIndex = n->GetId();
				break;
			}
		}

		// If no valid node could be found, return false
		if (connectedIndex == invalid_node_id)
		{
			return false;
		}

		// Start a depth-first-search traversal from the node that has at least one connection
		VisitAllNodesDFS(nodes, visitedNodes, connectedIndex);

		// If a node was never visited, this graph is not connected
		for (bool isVisited : visitedNodes)
		{
			if (!isVisited)
			{
				return false;
			}
		}

		return true;
	}


}