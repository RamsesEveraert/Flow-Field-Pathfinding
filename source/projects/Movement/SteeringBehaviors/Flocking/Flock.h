#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;

class CellSpace;

class Flock final
{
public:
	Flock(
		int flockSize = 50, 
		float worldSize = 100.f, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT);
	void RegisterAgentNeighbors(SteeringAgent* pAgent);
	void TrimAgentToWorld(SteeringAgent* pAgent);
	void UpdateAndRenderUI() ;
	void Render(float deltaT);

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const;
	const std::vector<SteeringAgent*>& GetNeighbors() const;

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;

	void SetTarget_Seek(const TargetData& target);
	void SetWorldTrimSize(float size) { m_WorldSize = size; }

private:
	//Datamembers
	int m_FlockSize = 0;
	std::vector<SteeringAgent*> m_vAgents;
	std::vector<SteeringAgent*> m_Neighbors;

	Elite::Color m_StandardBodyColor;

	float m_WorldSize = 0.f;

	const float m_NeighborhoodRadius = 5.f;
	int m_NrOfNeighbors = 0;

	bool m_TrimWorld = false;

	bool m_IsSpacePartitioningActive = true;


	bool m_IsDebugNeighborsActive = false;	
	bool m_IsCellSpaceDebugActive = false;
	
	//Steering Behaviors
	
	Separation* m_pSeparationBehavior = nullptr;
	Cohesion* m_pCohesionBehavior = nullptr;
	VelocityMatch* m_pVelMatchBehavior = nullptr;
	Seek* m_pSeekBehavior = nullptr;
	Wander* m_pWanderBehavior = nullptr;
	Evade* m_pEvadeBehavior = nullptr;

	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;

	SteeringAgent* m_pAgentToEvade = nullptr;

	//Spacial Partitioning
	CellSpace* m_pCellSpace = nullptr;

private:

	// Initialization
	void InitializeBehaviors();
	void InitializeAgents();
	void InitializeCellSpace();

	// rendering
	void RenderAgents(float deltaT);
	void RenderCellSpace(float deltaT);

	// helper functions

	void RenderNeighborhood();
	float* GetWeight(ISteeringBehavior* pBehaviour);


	// cleanup

	void Cleanup();

	// ******** //

	Flock(const Flock& other);
	Flock& operator=(const Flock& other);

};