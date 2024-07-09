/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringAgent.h: basic agent using steering behaviors
/*=============================================================================*/
#ifndef STEERING_AGENT_H
#define STEERING_AGENT_H

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "../../Shared/BaseAgent.h"
#include "SteeringHelpers.h"
class ISteeringBehavior;

class SteeringAgent : public BaseAgent
{
public:
	//--- Constructor & Destructor ---
	SteeringAgent() = default;
	SteeringAgent(float radius) : BaseAgent(radius) {};
	virtual ~SteeringAgent() = default;

	//--- Agent Functions ---
	void Update(float dt) override;
	void Render(float dt) override;

	float GetMaxLinearSpeed() const;
	void SetMaxLinearSpeed(float maxLinSpeed);

	float GetMaxAngularSpeed() const;
	void SetMaxAngularSpeed(float maxAngSpeed);

	bool IsAutoOrienting() const;
	void SetAutoOrient(bool autoOrient);

	Elite::Vector2 GetDirection() const;

	virtual void SetSteeringBehavior(ISteeringBehavior* pBehavior);
	ISteeringBehavior* GetSteeringBehavior() const;

	void SetDebugRenderingEnabled(bool isEnabled);
	bool GetDebugRenderingEnabled() const;

public:
	//added public functions
	Elite::Vector2 GetOldPosition() const;

	int GetCurrentCellIndex() const;
	void SetCurrentCellIndex(int idx);

protected:
	//--- Datamembers ---
	ISteeringBehavior* m_pSteeringBehavior = nullptr;
	
	Elite::Vector2 m_OldPosition{};

	float m_MaxLinearSpeed = 10.f;
	float m_MaxAngularSpeed = 10.f;
	bool m_AutoOrient = false;
	bool m_RenderDebug = false;

private:
	int m_CurrentCellIndex = -1;
};
#endif