#include "stdafx.h"
#include "EAStar.h"

using namespace Elite;
AStar::AStar(Graph* pGraph, Heuristic hFunction)
	: m_pGraph(pGraph)
	, m_HeuristicFunction(hFunction)
{
}

std::vector<GraphNode*> AStar::FindPath(GraphNode* pStartNode, GraphNode* pGoalNode)
{
    std::vector<GraphNode*> path{};
    std::list<NodeRecord> openList{};
    std::list<NodeRecord> closedList{};

    // 1.  Create a startRecord and add it to the openList to start the while loop
    NodeRecord startRecord{
        pStartNode,
        nullptr,
        0.f,
        GetHeuristicCost(pStartNode, pGoalNode)
    };
    openList.push_back(startRecord);
    while (!openList.empty())
    {
        // A. Get NodeRecord with lowest cost from openList
        auto currentIt = std::min_element(openList.begin(), openList.end());
        NodeRecord currentRecord = *currentIt;

        // B. Check if that record refers to the end node
        if (currentRecord.pNode == pGoalNode)
        {
            // 3. Reconstruct path from last connection to start node with backtracking
            while (currentRecord.pNode != pStartNode)
            {
                path.push_back(currentRecord.pNode);
                // If there is no connection, we reached the start node
                if (currentRecord.pConnection == nullptr) break;
                // Find the parent node
                auto parentNodeId = currentRecord.pConnection->GetFromNodeId();
                auto parentIt = std::find_if(closedList.begin(), closedList.end(),
                    [parentNodeId](const NodeRecord& record) {
                        return record.pNode->GetId() == parentNodeId;
                    });
                if (parentIt != closedList.end()) currentRecord = *parentIt;
            }
            path.push_back(pStartNode);
            std::reverse(path.begin(), path.end());
            return path;
        }

        // Remove currentRecord from openList and add it to closedList
        openList.erase(currentIt);
        closedList.push_back(currentRecord);

        // C. For each connection from the NodeRecord node
        auto connections = m_pGraph->GetConnectionsFromNode(currentRecord.pNode);
        for (const auto& connection : connections)
        {
            GraphNode* pNextNode = m_pGraph->GetNode(connection->GetToNodeId());
            float newCost = currentRecord.costSoFar + connection->GetCost();

            // Check if the next node is in the closed list
            auto closedIt = std::find_if(closedList.begin(), closedList.end(),
                [&pNextNode](const NodeRecord& record) { return record.pNode == pNextNode; });

            // D. Check if the connection leads to a node already on the closedList
            if (closedIt != closedList.end())
            {
                // Remove existing record if the new connection is cheaper
                if (closedIt->costSoFar > newCost)
                {
                    closedList.erase(closedIt);
                }
                else
                {
                    continue;
                }
            }

            // E. Do the same thing as D, but for the openList
            auto openIt = std::find_if(openList.begin(), openList.end(),
                [&pNextNode](const NodeRecord& record) { return record.pNode == pNextNode; });
            if (openIt != openList.end())
            {
                if (openIt->costSoFar > newCost)
                {
                    openList.erase(openIt);
                }
                else
                {
                    continue;
                }
            }

            // F. Only if D and E were not cheaper: Create a new NodeRecord and add it to the openList
            NodeRecord nextRecord{
                pNextNode,
                connection,
                newCost,
                newCost + GetHeuristicCost(pNextNode, pGoalNode)
            };
            openList.push_back(nextRecord);
        }
    }

    // If no path is found, return an empty path
    return path;
}




float AStar::GetHeuristicCost(GraphNode* pStartNode, GraphNode* pEndNode) const
{
	Vector2 toDestination = m_pGraph->GetNodePos(pEndNode->GetId()) - m_pGraph->GetNodePos(pStartNode->GetId());
	return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
}