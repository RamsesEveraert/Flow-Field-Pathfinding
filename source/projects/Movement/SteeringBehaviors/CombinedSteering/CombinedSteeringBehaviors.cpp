#include "stdafx.h"
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(std::vector<WeightedBehavior> weightedBehaviors)
	:m_WeightedBehaviors(weightedBehaviors)
{
};

//****************
//BLENDED STEERING

SteeringOutput BlendedSteering::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput blendedSteering{};
	float totalWeight{};

	//TODO: Calculate the weighted average steeringbehavior
	for (auto& weightedBehavior : m_WeightedBehaviors)
	{
		if (weightedBehavior.weight <= 0.f) continue;			

		auto steering = weightedBehavior.pBehavior->CalculateSteering(deltaT, pAgent);
		blendedSteering.LinearVelocity += steering.LinearVelocity * weightedBehavior.weight;
		blendedSteering.AngularVelocity += steering.AngularVelocity * weightedBehavior.weight;

		totalWeight += weightedBehavior.weight;
	}

	if (totalWeight > 0.f)
	{
		float invScale = 1 / totalWeight;

		blendedSteering.LinearVelocity *= invScale;
		blendedSteering.AngularVelocity *= invScale;
	}

	if (pAgent->GetDebugRenderingEnabled())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), blendedSteering.LinearVelocity, 7, { 0, 1, 1 }, 0.40f);

	return blendedSteering;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	for (auto pBehavior : m_PriorityBehaviors)
	{
		steering = pBehavior->CalculateSteering(deltaT, pAgent);

		if (steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return steering;
}