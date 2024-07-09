#pragma once
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "framework/EliteAI/EliteGraphs/EliteTerrainGridGraph/ETerrainGridGraph.h"
#include "framework/EliteAI/EliteGraphs/EliteTerrainGridGraph/ETerrainGraphNode.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphUtilities/EGraphEditor.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphUtilities/EGraphRenderer.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EHeuristic.h"

#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"
#include "projects/Shared/NavigationColliderElement.h"

//Forward declerations
class SteeringAgent;
class PathFollow;

//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_FlowField final : public IApp
{
public:
	//Constructor & Destructor
	App_FlowField() = default;
	virtual ~App_FlowField();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:

	// ----------- Grid datamembers ------------ //

	int m_NrOfCols{20};
	int m_PreviousNrOfCols{};
	int m_NrOfRows{10};
	int m_PreviousNrOfRows{};
	int m_SizeCell{ 20 };
	int m_PreviousCellSize{};

	std::unique_ptr<Elite::TerrainGridGraph> m_pTerrainGraph;

	std::list<std::unique_ptr<NavigationColliderElement>> m_lWalls;


	std::vector<int> m_HeatMap{};

	int m_DestinationNodeIndex{ invalid_node_id };

	Elite::Vector2 m_WorldSize{};

	//Editor and Visualisation
	Elite::GraphEditor m_GraphEditor{};
	Elite::GraphRenderer m_GraphRenderer{};

	// -------------- Agents datamembers -------------- //

	int m_NrOfAgents{ 500 };
	int m_PreviousNrOfAgents{};

	std::vector<std::unique_ptr<SteeringAgent>> m_vAgents;


	// ---------- Flow Field datamembers -------------- //

	std::vector<Elite::Vector2> m_VectorField{};

	// -------- Debug rendering information -------- //

	// Grid debug
	bool m_bDrawGrid{ true };
	bool m_bDrawNodeNumbers{ false };
	bool m_bDrawConnections{ false };
	bool m_bDrawConnectionsCosts{ false };

	// Flow Field Debug
	bool m_bDrawHeatMap{ false };
	bool m_bDrawVectorField{ false };
	std::vector<std::unique_ptr<Elite::Polygon>> m_CellPolygons;

	// Pathfinding Debug
	bool m_StartSelected{ true };
	int m_SelectedHeuristic{ 4 };
	Elite::Heuristic m_heuristicFunction{ Elite::HeuristicFunctions::Chebyshev };

private:

	// ------------ Functions ------------ //

	void MakeGridGraph();
	void UpdateImGui();
	void UpdateGridSettings();

	// HeatMap 
	void InitializeHeatMap();
	void InitializeHeatMapPolygons();
	void CalculateHeatMap(int goalNodeIndex);

	void RenderHeatMap() const;

	// Flow Field
	void CalculateVectorField();
	void ReCalculateFlowField();
	void RenderVectorField() const;
	void ResetFields();

	// Inputs
	void HandleInput();
	void HandleMiddleMouseButton();
	void HandleLeftMouseButton();
	void HandleRightMouseButton();
	void ToggleWallAtNode(Elite::TerrainGraphNode* node);
	void ToggleTerrainTypeAtNode(Elite::TerrainGraphNode* node);
	Elite::Vector2 GetMousePosition(const Elite::InputMouseButton& mouseButton);

	// Agents
	void UpdateAgents();
	void ResetAgents();
	void UpdateAgentSettings();

	void AddWall(Elite::Vector2 pos);


	// Debug Functions
	Elite::Color ValueToColor(int value, int maxHeatmapValue) const;

	// Helper Functions
	std::unique_ptr<Elite::Polygon> MakeRectanglePolygon(const Elite::Vector2& center, const Elite::Vector2& size) const;
	float Clamp(float value, float lower, float upper);


	//C++ make the class non-copyable
	App_FlowField(const App_FlowField&) = delete;
	App_FlowField& operator=(const App_FlowField&) = delete;
};
