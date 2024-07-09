#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

#include <algorithm>

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
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{
	//Calculate bounds of a cell
	m_CellWidth = width / cols;
	m_CellHeight = height / rows;

	//TODO: create the cells

	for (int row{}; row < rows; ++row)
	{
		for (int column{}; column < cols; ++column)
		{
			float left{ column * m_CellWidth };
			float bottom{ row * m_CellHeight };

			Cell cell{ left, bottom, m_CellWidth, m_CellHeight };
			m_Cells.emplace_back(cell);

		}
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	int agentCellIndex = agent->GetCurrentCellIndex();

	// agent is already in a cell
	if (agentCellIndex != -1)
		return;
	

	// Add the agent to the current cell
	const int idx = PositionToIndex(agent->GetPosition());
	m_Cells[idx].agents.insert(agent);

	// Set the agent's current cell index
	agent->SetCurrentCellIndex(idx);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	int oldIdx = agent->GetCurrentCellIndex();
	int currentIdx = PositionToIndex(agent->GetPosition());

	if (oldIdx != currentIdx)
	{
		// Remove agent from old cell
		if (oldIdx != -1)
			m_Cells[oldIdx].agents.erase(agent);

		// Add agent to new cell
		m_Cells[currentIdx].agents.insert(agent);

		// Update the agent's current cell index
		agent->SetCurrentCellIndex(currentIdx);
	}
}


void CellSpace::RegisterNeighbors(SteeringAgent* pAgent, float neighborhoodRadius)
{
	//TODO: Register the neighbors for the provided agent
	//Only check the cells that are within the radius of the neighborhood
	m_NrOfNeighbors = 0;

	const Elite::Vector2 positionAgent{ pAgent->GetPosition() };

	m_NeighborField.bottomLeft = { positionAgent - Elite::Vector2{neighborhoodRadius,neighborhoodRadius} };
	m_NeighborField.width = 2 * neighborhoodRadius;
	m_NeighborField.height = 2 * neighborhoodRadius;

	for (const auto& cell : m_Cells)
	{
		// 1. Find which cells are in the agent�s neighborhood  
		if (Elite::IsOverlapping(cell.boundingBox, m_NeighborField))
		{
			// 2. Get all agents from those cells
			for (const auto& pOtherAgent : cell.agents)
			{
				if (pOtherAgent)
				{
					// 3. Find which agents are within the neighborhood radius
					float distanceSquared{ pAgent->GetPosition().DistanceSquared(pOtherAgent->GetPosition()) };
					if (pOtherAgent != pAgent && distanceSquared <= neighborhoodRadius * neighborhoodRadius)
					{
						if (m_NrOfNeighbors < static_cast<int>(m_Neighbors.size()))
						{
							m_Neighbors[m_NrOfNeighbors] = pOtherAgent;
							++m_NrOfNeighbors;
						}
					}
				}

			}
		}
	}
}

const std::vector<SteeringAgent*>& CellSpace::GetNeighbors() const
{
	return m_Neighbors;
}

int CellSpace::GetNrOfNeighbors() const
{
	return m_NrOfNeighbors;
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{
	//TODO: Render the cells with the number of agents inside of it
	//TIP: use DEBUGRENDERER2D->DrawPolygon(...) and Cell::GetRectPoints())
	//TIP: use DEBUGRENDERER2D->DrawString(...) 

	for (auto& cell : m_Cells)
	{
		auto rectPoints{ cell.GetRectPoints() };

		Elite::Vector2 textOffset{ cell.boundingBox.width * 0.1f, cell.boundingBox.height * 0.3f };
		Elite::Vector2 positionCellText{ cell.boundingBox.bottomLeft + textOffset };

		std::string nrAgentsInCell{ std::to_string(static_cast<int>(cell.agents.size())) };

		DEBUGRENDERER2D->DrawPolygon(&rectPoints[0], 4, Elite::Color{ 1.f,0.f,0.f }, 0.4f);

		//draw numbers of agent in each cell
		DEBUGRENDERER2D->DrawString(positionCellText, nrAgentsInCell.c_str());

	}
}

void CellSpace::RenderNeighborsAgent(SteeringAgent* pAgent, float neighborhoodRadius)
{
	RegisterNeighbors(pAgent, neighborhoodRadius);

	DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), pAgent->GetRadius(), Elite::Color{ 1.f, 0.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

	for (int index{}; index < m_NrOfNeighbors; ++index)
	{
		DEBUGRENDERER2D->DrawCircle(m_Neighbors[index]->GetPosition(), m_Neighbors[index]->GetRadius(), Elite::Color{ 0.f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());
	}
	for (auto& cell : m_Cells)
	{
		if (Elite::IsOverlapping(cell.boundingBox, m_NeighborField))
		{
			auto rectPoints{ cell.GetRectPoints() };
			DEBUGRENDERER2D->DrawPolygon(&rectPoints[0], 4, Elite::Color{ 0.f,1.f,1.f }, 0.39f);
		}
	}
	std::vector<Elite::Vector2> pointsNeighborField =
	{
		{ m_NeighborField.bottomLeft },
		{ m_NeighborField.bottomLeft.x , m_NeighborField.bottomLeft.y + m_NeighborField.height  },
		{ m_NeighborField.bottomLeft.x + m_NeighborField.width , m_NeighborField.bottomLeft.y + m_NeighborField.height },
		{ m_NeighborField.bottomLeft.x + m_NeighborField.width , m_NeighborField.bottomLeft.y  },
	};
	DEBUGRENDERER2D->DrawPolygon(&pointsNeighborField[0], 4, Elite::Color{ 1.f,1.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());


	DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), neighborhoodRadius, Elite::Color{ 1.f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());
}


int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	//TODO: Calculate the index of the cell based on the position

	int row = static_cast<int>(pos.y) / static_cast<int>(m_CellHeight);
	int col = static_cast<int>(pos.x) / static_cast<int>(m_CellWidth);

	row = Elite::Clamp(row, 0, m_NrOfRows - 1);
	col = Elite::Clamp(col, 0, m_NrOfCols - 1);

	int index = row * m_NrOfCols + col;

	return index;

}