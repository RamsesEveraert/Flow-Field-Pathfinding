#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"
#include "../SpacePartitioning/SpacePartitioning.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
{
	
	//TODO: initialize the flock and the memory pool
	m_vAgents.resize(m_FlockSize);
	m_Neighbors.resize(m_FlockSize - 1);

	InitializeCellSpace();
	InitializeBehaviors();
	InitializeAgents();
	
}

Flock::~Flock()
{
	//TODO: Cleanup any additional data
	Cleanup();	
}

void Flock::Update(float deltaT)
{
	// Update the agent to evade
	m_pAgentToEvade->Update(deltaT);
	m_pEvadeBehavior->SetTarget(m_pAgentToEvade->GetPosition());

	// Update each agent
	for (auto& pAgent : m_vAgents)
	{
		RegisterAgentNeighbors(pAgent);
		TrimAgentToWorld(pAgent);
		m_pCellSpace->UpdateAgentCell(pAgent, pAgent->GetOldPosition());
		pAgent->Update(deltaT);
	}

	// Trim the agent to evade (consider moving this outside the loop as it doesn't need to be done for each agent)
	if (m_TrimWorld)
	{
		m_pAgentToEvade->TrimToWorld(m_WorldSize);
	}
}

void Flock::RegisterAgentNeighbors(SteeringAgent* pAgent)
{
	if (m_IsSpacePartitioningActive)
	{
		m_pCellSpace->RegisterNeighbors(pAgent, m_NeighborhoodRadius);
	}
	else
	{
		RegisterNeighbors(pAgent);
	}
}

void Flock::TrimAgentToWorld(SteeringAgent* pAgent)
{
	if (m_TrimWorld)
	{
		pAgent->TrimToWorld(m_WorldSize);
	}
}

void Flock::Render(float deltaT)
{
	// TODO: Render all the agents in the flock
	// Reset body color
	for (auto& pAgent : m_vAgents)
	{
		if (pAgent != m_vAgents[0]) pAgent->SetBodyColor(m_StandardBodyColor);
	}

	RenderAgents(deltaT);	

	if (m_IsCellSpaceDebugActive) RenderCellSpace(deltaT);
}
void Flock::RenderNeighborhood()
{
	// TODO: Implement

	// Register the neighbors for the first agent in the flock
	if (m_IsSpacePartitioningActive)
	{
		m_pCellSpace->RenderNeighborsAgent(m_vAgents[0], m_NeighborhoodRadius);
	}
	else
	{
		RegisterNeighbors(m_vAgents[0]);
		// DebugRender the neighbors in the memory pool
		DEBUGRENDERER2D->DrawCircle(m_vAgents[0]->GetPosition(), m_vAgents[0]->GetRadius(), Elite::Color{ 1.f, 0.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());
		DEBUGRENDERER2D->DrawCircle(m_vAgents[0]->GetPosition(), m_NeighborhoodRadius, Elite::Color{ 1.f, 1.f, 1.f }, DEBUGRENDERER2D->NextDepthSlice());

		for (int index{}; index < m_NrOfNeighbors; ++index)
		{
			DEBUGRENDERER2D->DrawCircle(m_Neighbors[index]->GetPosition(), m_vAgents[0]->GetRadius(), Elite::Color{ 0.f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());
		}
	}

	
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	//TODO: implement ImGUI checkboxes for debug rendering here

	ImGui::Checkbox("Activate SpacePartitioning", &m_IsSpacePartitioningActive);


	ImGui::Checkbox("Debug Neigbors", &m_IsDebugNeighborsActive);
	ImGui::Checkbox("Debug CellSpace", &m_IsCellSpaceDebugActive);

	ImGui::Text("Behavior Weights");
	ImGui::Spacing();

	//TODO: implement ImGUI sliders for steering behavior weights here
	
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Separation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("VelocityMatch", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// TODO: Implement
	m_NrOfNeighbors = 0;

	for (auto& otherAgent : m_vAgents)
	{ 
		// check agents in neighbor radius and agent != itself
		if (otherAgent && otherAgent != pAgent && DistanceSquared(pAgent->GetPosition(), otherAgent->GetPosition()) <= m_NeighborhoodRadius * m_NeighborhoodRadius)
		{
			m_Neighbors[m_NrOfNeighbors] = otherAgent;
			++m_NrOfNeighbors;
		}
	}
}

int Flock::GetNrOfNeighbors() const 
{
	if (m_IsSpacePartitioningActive) 
		return m_pCellSpace->GetNrOfNeighbors(); 

	return m_NrOfNeighbors; 
}

const std::vector<SteeringAgent*>& Flock::GetNeighbors() const 
{ 
	if (m_IsSpacePartitioningActive)
		return m_pCellSpace->GetNeighbors();
	return m_Neighbors;
}

Vector2 Flock::GetAverageNeighborPos() const
{
	// TODO: Implement
	if (GetNrOfNeighbors() == 0)
		return Elite::ZeroVector2;

	Vector2 sumPositions = Elite::ZeroVector2;

	// ranged based for loop can't because of pooling > errors

	for (int index{}; index < GetNrOfNeighbors(); ++index)
	{
		if (m_IsSpacePartitioningActive)
		{
			if (m_pCellSpace->GetNeighbors()[index])
				sumPositions += m_pCellSpace->GetNeighbors()[index]->GetPosition();
		}
		else
		{
			if (m_Neighbors[index])
				sumPositions += m_Neighbors[index]->GetPosition();
		}
	}


	return sumPositions / static_cast<float>(GetNrOfNeighbors());
}


Vector2 Flock::GetAverageNeighborVelocity() const
{
	// TODO: Implement
	if (GetNrOfNeighbors() == 0)
		return Elite::ZeroVector2;

	Vector2 sumVelocities = Elite::ZeroVector2;

	// ranged based for loop can't because of pooling > errors

	for (int index = 0; index < GetNrOfNeighbors(); ++index)
	{
		if (m_IsSpacePartitioningActive)
		{
			if (m_pCellSpace->GetNeighbors()[index])
				sumVelocities += m_pCellSpace->GetNeighbors()[index]->GetLinearVelocity();
		}
		else
		{
			if (m_Neighbors[index])
				sumVelocities += m_Neighbors[index]->GetLinearVelocity();
		}
		//sumVelocities += m_Neighbors[index]->GetLinearVelocity();
	}

	return sumVelocities / static_cast<float>(GetNrOfNeighbors());
}

void Flock::SetTarget_Seek(const TargetData& target)
{
	// TODO: Implement
	m_pSeekBehavior->SetTarget(target);
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

void Flock::InitializeBehaviors()
{
	m_pSeekBehavior = new Seek();
	m_pWanderBehavior = new Wander();
	m_pEvadeBehavior = new Evade();

	m_pCohesionBehavior = new Cohesion(this);
	m_pSeparationBehavior = new Separation(this);
	m_pVelMatchBehavior = new VelocityMatch(this);

	m_pBlendedSteering = new BlendedSteering({
		{m_pCohesionBehavior, 0.2f},
		{m_pSeparationBehavior, 0.2f},
		{m_pVelMatchBehavior, 0.2f},
		{m_pSeekBehavior, 0.2f},
		{m_pWanderBehavior, 0.2f}
		});

	m_pPrioritySteering = new PrioritySteering({ m_pEvadeBehavior, m_pBlendedSteering });
}
void Flock::InitializeAgents()
{
	for (auto& agent : m_vAgents)
	{
		agent = new SteeringAgent();

		// add to cellspace
		agent->SetSteeringBehavior(m_pPrioritySteering);
		agent->SetMaxLinearSpeed(randomFloat(10.f, 20.f));
		agent->SetMass(0.f);
		agent->SetAutoOrient(true);

		agent->SetPosition(randomVector2(0, m_WorldSize));
		m_pCellSpace->AddAgent(agent);
	}

	m_StandardBodyColor = m_vAgents[1]->GetBodyColor();

	m_pAgentToEvade = new SteeringAgent();
	m_pAgentToEvade->SetSteeringBehavior(m_pSeekBehavior);
	m_pAgentToEvade->SetMaxLinearSpeed(randomFloat(10.f, 30.f));
	m_pAgentToEvade->SetMass(0);
	m_pAgentToEvade->SetAutoOrient(true);
	m_pAgentToEvade->SetBodyColor({ 1.f,0.f,1.f });
}
void Flock::InitializeCellSpace()
{
	m_pCellSpace = new CellSpace(m_WorldSize, m_WorldSize, 10, 10, m_FlockSize - 1); // todo modify parameters
}

void Flock::RenderAgents(float deltaT)
{
	for (auto& agent : m_vAgents)
	{
		//agent->Render(deltaT);

		//TODO: switch with imGUI checkbox
		if (m_IsDebugNeighborsActive) RenderNeighborhood();
	}
}
void Flock::RenderCellSpace(float deltaT)
{
	m_pCellSpace->RenderCells();
}

void Flock::Cleanup()
{
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);

	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pEvadeBehavior);

	SAFE_DELETE(m_pAgentToEvade);

	for (auto pAgent : m_vAgents)
	{
		SAFE_DELETE(pAgent);
	}
	m_vAgents.clear();
	m_Neighbors.clear();
	m_pCellSpace->EmptyCells();

	SAFE_DELETE(m_pCellSpace);
}
