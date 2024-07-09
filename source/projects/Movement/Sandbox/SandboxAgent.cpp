#include "stdafx.h"
#include "SandboxAgent.h"

using namespace Elite;

SandboxAgent::SandboxAgent(): BaseAgent()
{
	m_Target = GetPosition();
}

void SandboxAgent::Update(float dt)
{
	//TODO: set linear velocity towards m_Target

	const float speed = 20.f;

	Vector2 direction = m_Target - GetPosition();
	direction.Normalize();

	Vector2 velocity = direction * speed;

	SetLinearVelocity(velocity);

	//DEBUGRENDERER2D->DrawSegment(GetPosition(), m_Target, Color{ 1.f,0.f,0.f });
	DEBUGRENDERER2D->DrawDirection(GetPosition(), direction, speed, Color{ 1.f,0.f,0.f });

	//Orientation
	AutoOrient();
}

void SandboxAgent::Render(float dt)
{
	BaseAgent::Render(dt); //Default Agent Rendering
}

void SandboxAgent::AutoOrient()
{
	//Determine angle based on direction
	Vector2 velocity = GetLinearVelocity();
	if (velocity.MagnitudeSquared() > 0)
	{
		SetRotation(VectorToOrientation(velocity));
	}
}