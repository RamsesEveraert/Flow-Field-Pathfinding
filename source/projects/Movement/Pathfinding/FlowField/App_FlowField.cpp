// Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

// Includes
#include "App_FlowField.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EAstar.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EBFS.h"
#include "projects/Movement/SteeringBehaviors/PathFollow/PathFollowSteeringBehavior.h"
#include "framework/EliteAI/EliteGraphs/EliteGraph/EGraphEnums.h"

// Singletons
#include "framework/EliteTimer/ETimer.h"

// standardlibrary
#include <algorithm>

using namespace Elite;

// Destructor
App_FlowField::~App_FlowField()
{

}

// Initialization

void App_FlowField::Start()
{
	// Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(200.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(m_NrOfCols * 0.5f * m_SizeCell, m_NrOfRows * 0.5f * m_SizeCell));

	//Create Graph
	MakeGridGraph();

	// HeatMap
	InitializeHeatMap();

	// World
	m_WorldSize = { m_NrOfCols * static_cast<float>(m_SizeCell),m_NrOfRows * static_cast<float>(m_SizeCell) };
	m_PreviousNrOfCols = m_NrOfCols;
	m_PreviousNrOfRows = m_NrOfRows;
	m_PreviousCellSize = m_SizeCell;

	// Create Agents
	ResetAgents();

}
void App_FlowField::MakeGridGraph()
{
	m_pTerrainGraph = std::make_unique<TerrainGridGraph>(m_NrOfCols, m_NrOfRows, static_cast<float>(m_SizeCell), true, false);

}
void App_FlowField::InitializeHeatMap()
{
	m_HeatMap.resize(m_NrOfCols * m_NrOfRows, 255); // Initialize with a high value
	InitializeHeatMapPolygons();
}
void App_FlowField::InitializeHeatMapPolygons()
{
	if (!m_CellPolygons.empty())
	{
		m_CellPolygons.clear();
	}
	m_CellPolygons.reserve(m_NrOfCols * m_NrOfRows);
	for (int rowNr = 0; rowNr < m_NrOfRows; ++rowNr)
	{
		for (int colNr = 0; colNr < m_NrOfCols; ++colNr)
		{
			Elite::Vector2 cellPosition = m_pTerrainGraph->GetNodePos(rowNr * m_NrOfCols + colNr);
			Elite::Vector2 cellSize(static_cast<float>(m_SizeCell), static_cast<float>(m_SizeCell));
			m_CellPolygons.emplace_back(MakeRectanglePolygon(cellPosition, cellSize));
		}
	}
}
void App_FlowField::ResetAgents()
{
	// Define a margin to prevent agents from being initialized too close to the boundaries
	const float agentSafetyMargin = m_SizeCell * 0.5f;

	// Clear existing agents, if any
	m_vAgents.clear();

	// Create agents within the grid boundaries
	for (int index = 0; index < m_NrOfAgents; ++index)
	{
		std::unique_ptr<SteeringAgent> pNewAgent = std::make_unique<SteeringAgent>();
		pNewAgent->SetMaxLinearSpeed(25.f);
		pNewAgent->SetMass(1.f); // A mass of 0 is unusual, setting it to 1
		pNewAgent->SetAutoOrient(true);

		Vector2 randomPos;
		randomPos.x = static_cast<float>(rand() % static_cast<int>(m_WorldSize.x - agentSafetyMargin * 2)) + agentSafetyMargin;
		randomPos.y = static_cast<float>(rand() % static_cast<int>(m_WorldSize.y - agentSafetyMargin * 2)) + agentSafetyMargin;

		pNewAgent->SetPosition(randomPos);
		pNewAgent->TrimToWorld(Elite::Vector2{}, Elite::Vector2{ m_NrOfCols * static_cast<float>(m_SizeCell), m_NrOfRows * static_cast<float>(m_SizeCell) }, false);
		m_vAgents.emplace_back(std::move(pNewAgent));
	}
}

// Update Functions

void App_FlowField::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	// FlowField
	HandleInput();

	// Update Agents
	UpdateAgents();

	//IMGUI
	UpdateImGui();

	// Update IMGUI Setting changes
	UpdateAgentSettings();
	UpdateGridSettings();
}
void App_FlowField::HandleInput()
{
	HandleMiddleMouseButton();
	HandleLeftMouseButton();
	HandleRightMouseButton();
}
void App_FlowField::HandleMiddleMouseButton()
{
	if (!INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle)) return;

	Elite::Vector2 mousePos = GetMousePosition(InputMouseButton::eMiddle);
	int nodeIndex = m_pTerrainGraph->GetNodeIdAtPosition(mousePos);
	m_DestinationNodeIndex = nodeIndex;
	CalculateHeatMap(nodeIndex);
	CalculateVectorField();
}
void App_FlowField::HandleLeftMouseButton()
{
	if (!INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft)) return;

	Elite::Vector2 mousePos = GetMousePosition(InputMouseButton::eLeft);
	auto node = static_cast<TerrainGraphNode*>(m_pTerrainGraph->GetNodeAtPosition(mousePos));

	if (node != nullptr)
	{
		ToggleWallAtNode(node);
	}
}
void App_FlowField::HandleRightMouseButton()
{
	if (!INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eRight)) return;

	Elite::Vector2 mousePos = GetMousePosition(InputMouseButton::eRight);
	auto node = static_cast<TerrainGraphNode*>(m_pTerrainGraph->GetNodeAtPosition(mousePos));

	if (node != nullptr)
	{
		ToggleTerrainTypeAtNode(node);
	}
}
void App_FlowField::ToggleWallAtNode(TerrainGraphNode* node)
{
	bool wallExists = false;
	for (auto it = m_lWalls.begin(); it != m_lWalls.end(); ++it)
	{
		if ((*it)->GetPosition() == node->GetPosition())
		{
			m_lWalls.erase(it);
			wallExists = true;
			m_pTerrainGraph->AddConnectionsToAdjacentCells(node->GetId());
			ReCalculateFlowField();
			break;
		}
	}

	if (!wallExists)
	{
		m_pTerrainGraph->RemoveAllConnectionsWithNode(node->GetId());
		AddWall(node->GetPosition());
		ReCalculateFlowField();
	}
}
void App_FlowField::ToggleTerrainTypeAtNode(TerrainGraphNode* node)
{
	if (node->GetTerrainType() == TerrainType::Mud)
	{
		node->SetTerrainType(TerrainType::Ground);
	}
	else
	{
		node->SetTerrainType(TerrainType::Mud);
	}
	ReCalculateFlowField();
}
void App_FlowField::UpdateAgents()
{
	// Early exit if the destination index is invalid
	if (m_DestinationNodeIndex == invalid_node_id) return;

	for (auto& pAgent : m_vAgents)
	{
		if (pAgent == nullptr) continue;

		// Get Agent's posiion
		Vector2 agentPos = pAgent->GetPosition();

		// Get node index based on agent's position
		int nodeIndex = m_pTerrainGraph->GetNodeIdAtPosition(agentPos);

		// Get direction from vector field
		if (nodeIndex == invalid_node_id || nodeIndex == m_DestinationNodeIndex) continue;

		Vector2 desiredDirection = m_VectorField[nodeIndex];
		if (desiredDirection == ZeroVector2) continue; // Skip if there is no valid direction

		// Move agent in direction stored in vector field
		Vector2 linearSpeed = desiredDirection.GetNormalized() * pAgent->GetMaxLinearSpeed();
		pAgent->SetLinearVelocity(linearSpeed);

		// Update the agent's position, clamping to world boundaries
		Vector2 newPos = agentPos + linearSpeed * Elite::ETimer<PLATFORM_WINDOWS>::GetInstance()->GetElapsed();
		newPos.x = Clamp(newPos.x, 0.f, m_WorldSize.x);
		newPos.y = Clamp(newPos.y, 0.f, m_WorldSize.y);
		pAgent->SetPosition(newPos);
	}
}
void App_FlowField::UpdateImGui()
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		int menuWidth = 200;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);


		// student information
		ImGui::Text("Everaert Ramses");
		ImGui::Text("Research Flow Field");

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: Add / Remove Wall");
		ImGui::Text("RMB: Add / Remove Mud");
		ImGui::Text("MMB: Set Destination");
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("Debug Grid");
		ImGui::Checkbox("Grid", &m_bDrawGrid);
		ImGui::Checkbox("NodeNumbers", &m_bDrawNodeNumbers);
		ImGui::Checkbox("Connections", &m_bDrawConnections);
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("FlowField");
		ImGui::Checkbox("HeatMap", &m_bDrawHeatMap);
		ImGui::Checkbox("VectorField", &m_bDrawVectorField);

		ImGui::Text("Agent Settings");
		ImGui::SliderInt("Agents", &m_NrOfAgents, 0, 2000);

		ImGui::Text("Agent Settings");
		ImGui::SliderInt("Columns", &m_NrOfCols, 10, 20);
		ImGui::SliderInt("Rows", &m_NrOfRows, 10, 20);
		ImGui::SliderInt("CellSize", &m_SizeCell, 20, 40);

		ImGui::Spacing();
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}
void App_FlowField::UpdateGridSettings()
{
	//check if the amount of columns or rows or the cellSize has changed
	if (m_NrOfCols != m_PreviousNrOfCols || m_NrOfRows != m_PreviousNrOfRows || m_SizeCell != m_PreviousCellSize)
	{
		//delete the existing grid and make a new grid
		m_pTerrainGraph.reset(new TerrainGridGraph(m_NrOfCols, m_NrOfRows, static_cast<float>(m_SizeCell), true, false));
		ResetFields();

		m_PreviousNrOfCols = m_NrOfCols;
		m_PreviousNrOfRows = m_NrOfRows;
		m_PreviousCellSize = m_SizeCell;

		//recalculate the IntegrationField and VectorField
		ReCalculateFlowField();

		//change the world datamembers
		m_WorldSize.x = static_cast<float>(m_SizeCell) * m_NrOfCols;
		m_WorldSize.y = static_cast<float>(m_SizeCell) * m_NrOfRows;
	}
}
void App_FlowField::UpdateAgentSettings()
{
	if (m_NrOfAgents != m_PreviousNrOfAgents)
	{
		ResetAgents();
	}

	m_PreviousNrOfAgents = m_NrOfAgents;
}

// Render Functions

void App_FlowField::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);
	//Render grid
	m_GraphRenderer.RenderGraph(
		m_pTerrainGraph.get(),
		m_bDrawGrid,
		m_bDrawNodeNumbers,
		m_bDrawConnections,
		m_bDrawConnectionsCosts
	);

	RenderHeatMap();
	RenderVectorField();

	//Render destination node
	if (m_DestinationNodeIndex != invalid_node_id)
	{
		m_GraphRenderer.HighlightNodes(m_pTerrainGraph.get(), std::vector<Elite::GraphNode*>{ m_pTerrainGraph->GetNode(m_DestinationNodeIndex) }, END_NODE_COLOR);
	}

}
void App_FlowField::RenderHeatMap() const
{
	// Find the maximum heatmap value for normalization
	int maxHeatmapValue = *std::max_element(m_HeatMap.begin(), m_HeatMap.end());

	// Render heatmap
	if (m_bDrawHeatMap)
	{
		Vector2 textPos{ 1.f, static_cast<float>(m_SizeCell) };

		for (size_t idx = 0; idx < m_CellPolygons.size(); ++idx)
		{
			// Get the heatmap value for the current cell
			int heatmapValue = m_HeatMap[idx];

			// Map the heatmap value to a blue color
			Elite::Color cellColor = ValueToColor(heatmapValue, maxHeatmapValue);

			// Draw color
			DEBUGRENDERER2D->DrawSolidPolygon(m_CellPolygons[idx].get(), cellColor, -1.f, false);

			// Draw heatmap values
			if (idx % m_NrOfCols == 0 && idx != 0)
			{
				textPos.y += m_SizeCell;
				textPos.x = 1.f;
			}
			if (heatmapValue != invalid_node_id) DEBUGRENDERER2D->DrawString(textPos, std::to_string(heatmapValue).c_str());
			textPos.x += m_SizeCell;
		}
	}
}
void App_FlowField::RenderVectorField() const
{
	if (m_DestinationNodeIndex == invalid_node_id) return;
	if (m_bDrawVectorField)
	{
		for (size_t nodeIndex = 0; nodeIndex < m_HeatMap.size(); ++nodeIndex)
		{
			if (m_HeatMap[nodeIndex] == invalid_node_id)
			{
				continue; // Skip rendering non-traversable nodes
			}

			auto node = static_cast<TerrainGraphNode*>(m_pTerrainGraph->GetNode(nodeIndex));
			Vector2 nodePosition = node->GetPosition();

			// Draw center (red circle)
			DEBUGRENDERER2D->DrawSolidCircle(nodePosition, 1.5f, Vector2{ 0, 0 }, { 1, 0, 0, 1 }, 0.4f);

			// Get the vector direction
			Vector2 vectorDirection = m_VectorField[nodeIndex];

			// Scale vector (visualisation)
			Vector2 endPosition = nodePosition + vectorDirection * 7.5f;

			// Draw direction
			DEBUGRENDERER2D->DrawSegment(nodePosition, endPosition, { 1, 1, 1, 1 }, 0.2f);
		}
	}

}

// Calculations

void App_FlowField::CalculateHeatMap(int goalNodeIndex)
{
	//All nodes are unvisited
	std::fill(m_HeatMap.begin(), m_HeatMap.end(), invalid_node_id);

	// Create a pathfinder using Bread-First Search
	BFS pathfinder(m_pTerrainGraph.get());

	// Iterate over all nodes
	for (size_t nodeIndex = 0; nodeIndex < m_HeatMap.size(); ++nodeIndex)
	{
		// Skip the goal node
		auto currentNode = static_cast<TerrainGraphNode*>(m_pTerrainGraph->GetNode(nodeIndex));
		if (nodeIndex == goalNodeIndex)
		{
			m_HeatMap[nodeIndex] = (nodeIndex == goalNodeIndex) ? 0 : invalid_node_id;
			continue;
		}

		// Find the path from the current node to the goal
		auto path = pathfinder.FindPath(currentNode, m_pTerrainGraph->GetNode(goalNodeIndex));
		if (!path.empty())
		{
			// Calculate and update the heatmap value for the node
			m_HeatMap[nodeIndex] = path.size() - 1;
			if (currentNode->GetTerrainType() == TerrainType::Mud)
			{
				m_HeatMap[nodeIndex] += static_cast<int>(TerrainType::Mud); // Extra cost for mud
			}
		}
	}
}
void App_FlowField::CalculateVectorField()
{
	m_VectorField.resize(m_HeatMap.size());

	for (size_t nodeIndex = 0; nodeIndex < m_HeatMap.size(); ++nodeIndex)
	{
		// Skip the goal node and non-traversable nodes
		if (nodeIndex == m_DestinationNodeIndex || m_HeatMap[nodeIndex] == invalid_node_id)
		{
			m_VectorField[nodeIndex] = Elite::Vector2();
			continue;
		}

		// Retrieve all neighbors
		const auto& neighbors = m_pTerrainGraph->GetConnectionsFromNode(nodeIndex);

		// Find the neighbor with the lowest heatmap value
		float lowestCost = FLT_MAX;
		Elite::Vector2 directionToLowestCostNeighbor;

		for (const auto& neighbor : neighbors)
		{
			int neighborIndex = neighbor->GetToNodeId();
			float neighborCost = static_cast<float>(m_HeatMap[neighborIndex]);

			if (neighborCost < lowestCost)
			{
				lowestCost = neighborCost;
				directionToLowestCostNeighbor = (m_pTerrainGraph->GetNodePos(neighborIndex) - m_pTerrainGraph->GetNodePos(nodeIndex)).GetNormalized();
			}
		}

		// Assign the direction to the vector field
		m_VectorField[nodeIndex] = directionToLowestCostNeighbor;
	}
}

// Helper functions

Elite::Color App_FlowField::ValueToColor(int value, int maxHeatmapValue) const
{
	// Normalize
	float normalizedValue = static_cast<float>(value) / maxHeatmapValue;

	// Create a shade of blue based on the normalized value
	return Elite::Color(0.0f, 0.0f, 1 - normalizedValue, 1.0f); 
}
std::unique_ptr<Elite::Polygon> App_FlowField::MakeRectanglePolygon(const Elite::Vector2& center, const Elite::Vector2& size) const
{
	std::vector<Elite::Vector2> points;
	float halfWidth = size.x * 0.5f;
	float halfHeight = size.y * 0.5f;
	points.push_back(Elite::Vector2(center.x - halfWidth, center.y - halfHeight)); // Bottom left
	points.push_back(Elite::Vector2(center.x + halfWidth, center.y - halfHeight)); // Bottom right
	points.push_back(Elite::Vector2(center.x + halfWidth, center.y + halfHeight)); // Top right
	points.push_back(Elite::Vector2(center.x - halfWidth, center.y + halfHeight)); // Top left

	return std::make_unique<Elite::Polygon>(points);
}
Elite::Vector2 App_FlowField::GetMousePosition(const InputMouseButton& mouseButton)
{
	MouseData mouseData = INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, mouseButton);
	Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });

	return mousePos;
}
float App_FlowField::Clamp(float value, float lower, float upper)
{
	return max(lower, min(value, upper));
}
void App_FlowField::AddWall(Elite::Vector2 pos)
{
	auto searchedWall = std::find_if(m_lWalls.begin(), m_lWalls.end(),
		[&](const std::unique_ptr<NavigationColliderElement>& wall) {
			return wall->GetPosition() == pos;
		});

	// Add a new wall if it doesn't already exist
	if (searchedWall == m_lWalls.end())
	{
		m_lWalls.emplace_back(std::make_unique<NavigationColliderElement>(pos, static_cast<float>(m_SizeCell), static_cast<float>(m_SizeCell)));
	}
}
void App_FlowField::ReCalculateFlowField()
{
	CalculateHeatMap(m_DestinationNodeIndex);
	CalculateVectorField();
}
void App_FlowField::ResetFields()
{
	InitializeHeatMap();
	m_VectorField.resize(m_NrOfCols * m_NrOfRows);

}