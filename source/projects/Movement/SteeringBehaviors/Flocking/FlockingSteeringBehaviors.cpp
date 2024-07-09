#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

#pragma region COHESION
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	if (m_pFlock->GetNrOfNeighbors() == 0)
	{
		steering.IsValid = false;
		return steering;
	}

	Elite::Vector2 avgNeighborPosition{ m_pFlock->GetAverageNeighborPos() };
	m_Target = avgNeighborPosition;

	return Seek::CalculateSteering(deltaT, pAgent);
}
#pragma endregion

#pragma region SEPARATION
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	if (m_pFlock->GetNrOfNeighbors() == 0)
	{
		steering.IsValid = false;
		return steering;
	}


	Elite::Vector2 totalseperateDirection{};

	for (int idx{}; idx < m_pFlock->GetNrOfNeighbors(); ++idx)
	{
		auto neighbor{ m_pFlock->GetNeighbors()[idx] };
		Elite::Vector2 direction{ neighbor->GetPosition() - pAgent->GetPosition() };
		float distanceSquaredNeighbor{ direction.MagnitudeSquared()};

		totalseperateDirection += direction / distanceSquaredNeighbor;		
	}

	m_Target = pAgent->GetPosition() + totalseperateDirection.GetNormalized();

	return Flee::CalculateSteering(deltaT, pAgent);
}
#pragma endregion

#pragma region VELOCITYMATCH
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};


	if (m_pFlock->GetNrOfNeighbors() == 0)
	{
		steering.IsValid = false;
		return steering;
	}


	steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity().GetNormalized() * pAgent->GetMaxLinearSpeed();

	return steering;
}

#pragma endregion

