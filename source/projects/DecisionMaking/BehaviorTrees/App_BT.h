#pragma once
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "projects/DecisionMaking/SmartAgent.h"
#include "projects/Movement/SteeringBehaviors/SteeringHelpers.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/EPathSmoothing.h"

//Forward declarations
class NavigationColliderElement;
class SteeringAgent;
class PathFollow;

namespace Elite
{
	class NavGraph;
}


//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_BT final : public IApp
{
public:
	//Constructor & Destructor
	App_BT() = default;
	virtual ~App_BT();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	Elite::Blackboard* CreateBlackboard(SmartAgent* smartAgent);

	//Datamembers
	// --AI--
	// ...

	// --Agents--
	SteeringAgent* m_pPlayer = nullptr;
	SmartAgent* m_pGuard = nullptr;
	PathFollow* m_pPlayerPathFollow = nullptr;
	PathFollow* m_pGuardPathFollow = nullptr;
	TargetData m_Target = {};
	float m_AgentRadius = 1.0f;
	float m_PlayerSpeed = 16.0f;
	float m_GuardSpeed = 8.0f;
	float m_DetectionRadius = 10.0f;
	float m_MaxSearchTime = 5.0f;

	// --Level--
	std::vector<NavigationColliderElement*> m_vNavigationColliders = {};

	// --Pathfinder--
	std::vector<Elite::Vector2> m_vPath;
	std::vector<Elite::Vector2> m_PatrolPath;

	// --Graph--
	Elite::NavGraph* m_pNavGraph = nullptr;

	// --Debug drawing information--
	static bool sShowPolygon;
	static bool sDrawPlayerPath;
	static bool sDrawPatrolPath;
	static bool sDrawDetectionRadius;

	void UpdateImGui();
private:
	//C++ make the class non-copyable
	App_BT(const App_BT&) = delete;
	App_BT& operator=(const App_BT&) = delete;
};
