/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/DecisionMaking/SmartAgent.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "projects/Movement/SteeringBehaviors/PathFollow/PathFollowSteeringBehavior.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
// BT TODO:

namespace BT_Actions 
{
	Elite::BehaviorState Patrol(Elite::Blackboard* pBlackboard)
	{
		SmartAgent* pAgent;
		std::vector<Elite::Vector2> patrolPath;
		PathFollow* pPathFollow;

		pBlackboard->GetData("Agent", pAgent);
		assert(pAgent && "Agent wasn't set in blackboard!");
		pBlackboard->GetData("PathFollow", pPathFollow);
		assert(pPathFollow && "PathFollow wasn't set in blackboard!");
		pBlackboard->GetData("PatrolPath", patrolPath);

		pAgent->SetSteeringBehavior(pPathFollow);
		if (pPathFollow->HasArrived())
		{
			pPathFollow->SetPath(patrolPath);
		}

		return Elite::BehaviorState::Success;
	}
}

namespace BT_Conditions
{
	bool IsTargetVisible(Elite::Blackboard* pBlackboard)
	{
		SmartAgent* pAgent;
		SteeringAgent* pPlayer;
		float detectionRadius;

		pBlackboard->GetData("Agent", pAgent);
		assert(pAgent && "Agent wasn't set in blackboard!");
		pBlackboard->GetData("TargetAgent", pPlayer);
		assert(pPlayer && "TargetAgent wasn't set in blackboard!");
		pBlackboard->GetData("DetectRadius", detectionRadius);

		bool isInDetectRadius = Elite::DistanceSquared(pAgent->GetPosition(), pPlayer->GetPosition()) < detectionRadius * detectionRadius;
		bool isInLineOfSight = pAgent->HasLineOfSight(pPlayer->GetPosition());

		return isInDetectRadius && isInLineOfSight;
	}
}

#endif