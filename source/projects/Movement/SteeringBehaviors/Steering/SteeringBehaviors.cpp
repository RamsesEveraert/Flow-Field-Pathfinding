//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"
#include <limits>

#pragma region SEEK
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	const Elite::Vector2 positionAgent{ pAgent->GetPosition() };
	Elite::Vector2 direction{ m_Target.Position - positionAgent };
	direction.Normalize();
	steering.LinearVelocity = direction * pAgent->GetMaxLinearSpeed();

	if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawSegment(positionAgent, positionAgent + steering.LinearVelocity, Elite::Color{ 1.f,0.f,0.f });
	}

	return steering;
}
#pragma endregion
#pragma region FLEE
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering {}; // niet overnemen van seek alsje seek aanpast kan flee ook mee veranderen

	const Elite::Vector2 positionAgent{ pAgent->GetPosition() };
	Elite::Vector2 direction{ m_Target.Position - positionAgent };
	direction.Normalize();
	steering.LinearVelocity = -direction * pAgent->GetMaxLinearSpeed();

	if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawSegment(positionAgent, positionAgent + steering.LinearVelocity, Elite::Color{ 1.f,0.f,0.f });
	}

	return steering;
}
#pragma endregion
#pragma region ARRIVE
void Arrive::SetTargetRadius(float radius)
{
	m_TargetRadius = radius;
}

void Arrive::SetSlowRadius(float radius)
{
	m_SlowRadius = radius;
}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	const Elite::Vector2 positionAgent{ pAgent->GetPosition() };

	Elite::Vector2 toTarget{ m_Target.Position - positionAgent };
	float distance{ toTarget.Magnitude() }; 

	// Calculate values for blending the steering based on distance
	float distanceToSlowDown{ distance - m_TargetRadius };
	float distanceToStop{ m_SlowRadius - m_TargetRadius };

	// Calculate the ratio of how close the agent is to the target, clamped between [0, 1]
	float slowingRatio = distanceToSlowDown / distanceToStop;
	slowingRatio = Elite::Clamp(slowingRatio, 0.f, 1.f);

	// Calculate the desired linear velocity towards the target based on the ratio
	steering.LinearVelocity = toTarget.GetNormalized() * pAgent->GetMaxLinearSpeed() * slowingRatio;

	// Debug rendering
	if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawCircle(positionAgent, m_TargetRadius, Elite::Color{ 1.f,0.f,0.f }, DEBUGRENDERER2D->NextDepthSlice());
		DEBUGRENDERER2D->DrawCircle(positionAgent, m_SlowRadius, Elite::Color{ 0.f,0.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());
	}

	return steering;
}
#pragma endregion
#pragma region FACE
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{

	SteeringOutput steering {};

	// Calculate the desired orientation to face the target
	const float desiredOrientation{ Elite::VectorToOrientation( m_Target.Position - pAgent->GetPosition()) };
	const float currentOrientation{ pAgent->GetRotation() };

	// Calculate the angle between current orientation and desired orientation
	float angle{ desiredOrientation  - currentOrientation };

	const float slowAngle{ 0.2f };
	const float stopAngle{ 0.07f };

	angle = Elite::ClampedAngle(angle);

	if (abs(angle) < slowAngle)
	{
		if (abs(angle) <= stopAngle)
		{
			steering.IsValid = false;
			steering.AngularVelocity = 0.f;
		}
		else
		{
			steering.AngularVelocity = (angle < 0 ? -1 : 1) * pAgent->GetMaxAngularSpeed() * abs(angle) / slowAngle;
		}
	}
	else
	{
		steering.AngularVelocity = (angle < 0 ? -1 : 1) * pAgent->GetMaxAngularSpeed();
	}

	pAgent->SetAutoOrient(false);

	return steering;

}
#pragma endregion
#pragma region PURSUIT
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	const Elite::Vector2 positionAgent{ pAgent->GetPosition() };
	const float maxSpeed{ pAgent->GetMaxLinearSpeed() };

	Elite::Vector2 targetDirection = m_Target.Position - positionAgent;
	float distanceToTarget = targetDirection.Magnitude();

	float predictionTime = distanceToTarget / maxSpeed;
	Elite::Vector2 predictedTargetPosition = m_Target.Position + m_Target.LinearVelocity * predictionTime;

	// Calculate the desired direction and velocity
	Elite::Vector2 desiredDirection = predictedTargetPosition - positionAgent;
	desiredDirection.Normalize();
	steering.LinearVelocity = desiredDirection * maxSpeed;

	// Debug rendering
	if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawCircle(predictedTargetPosition, 0.5f, Elite::Color{ 0.f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());DEBUGRENDERER2D->DrawSegment(pAgent->GetPosition() + steering.LinearVelocity, pAgent->GetPosition() + steering.LinearVelocity, Elite::Color{ 1.f, 0.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());
	}

	return steering;

}
#pragma endregion
#pragma region EVADE
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	const Elite::Vector2 positionAgent{ pAgent->GetPosition() };

	Elite::Vector2 targetDirection = m_Target.Position - positionAgent;

	float distanceSquared{ targetDirection.MagnitudeSquared() };

	if (distanceSquared > m_EvadeRadius * m_EvadeRadius)
	{
		steering.IsValid = false;
		return steering;
	}

	float distanceToTarget{ sqrtf(distanceSquared) };

	const float maxSpeed{ pAgent->GetMaxLinearSpeed() };

	// Limit the prediction time to avoid overshooting
	float predictionTime{ distanceToTarget / maxSpeed };
	Elite::Vector2 predictedTargetPosition = m_Target.Position + m_Target.LinearVelocity * predictionTime;

	// Check if the agent is within the evade radius
	Elite::Vector2 desiredDirection{};

	

	// Reverse the desired direction for evasion
	desiredDirection = predictedTargetPosition - positionAgent;
	desiredDirection.Normalize();
	steering.LinearVelocity = desiredDirection * -maxSpeed;
	

	// Debug rendering
	if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawCircle(predictedTargetPosition, 0.5f, Elite::Color{ 0.f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());
	}

	return steering;

}
#pragma endregion
#pragma region WANDER
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Elite::Vector2 circleOrigin{
		pAgent->GetPosition() + pAgent->GetDirection() * m_OffsetDistance
	};

	float maxWanderAngle{ m_WanderAngle + m_MaxAngleChange };
	float minWanderAngle{ m_WanderAngle - m_MaxAngleChange };

	m_WanderAngle = Elite::randomFloat(minWanderAngle, maxWanderAngle);

	Elite::Vector2 randomPointOnCircle{
		circleOrigin.x + cosf(m_WanderAngle) * m_Radius,
		circleOrigin.y + sinf(m_WanderAngle) * m_Radius
	};

	m_Target = randomPointOnCircle;

	// Debug rendering
	if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawCircle(circleOrigin, m_Radius, Elite::Color{ 0.f, 1.f, 0.f, 0.5f }, DEBUGRENDERER2D->NextDepthSlice());
		DEBUGRENDERER2D->DrawPoint(randomPointOnCircle, 2.f, Elite::Color{ 0.f, 0.f, 1.f, 1.f }, DEBUGRENDERER2D->NextDepthSlice());
	}


	return Seek::CalculateSteering(deltaT, pAgent);
}
#pragma endregion

