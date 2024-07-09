#pragma once
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

class SteeringAgent;
class BlendedSteering;
class PrioritySteering;

//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_CombinedSteering final : public IApp
{
public:
	//Constructor & Destructor
	App_CombinedSteering() = default;
	virtual ~App_CombinedSteering() final;

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	//Datamembers
	TargetData m_MouseTarget = {};
	bool m_UseMouseTarget = false;
	bool m_VisualizeMouseTarget = true;
	
	bool m_IsDebugNeighborsActive = false;
	bool m_TrimWorld = true;
	float m_TrimWorldSize = 50.f;

	// agent 1
	SteeringAgent* m_pAgent1{ nullptr };
	Seek* m_pAgent1Seek{ nullptr };
	Wander* m_pAgent1Wander{ nullptr };
	BlendedSteering* m_pAgent1BlendedSteering{ nullptr };

	// agent 2
	SteeringAgent* m_pAgent2{ nullptr };
	Evade* m_pAgent2Evade{ nullptr };
	Wander* m_pAgent2Wander{ nullptr };
	PrioritySteering* m_pAgent2PrioritySteering{ nullptr };

};