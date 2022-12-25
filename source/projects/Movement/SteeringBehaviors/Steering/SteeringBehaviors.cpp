//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"
#include <iostream>

using namespace Elite;

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	const Vector2 agentPosition{ pAgent->GetPosition() };
	const Vector2 toTarget{ m_Target.Position - agentPosition };

	steering.LinearVelocity = toTarget;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		const Vector2 agentDirection{ pAgent->GetDirection() };

		//draws the current velocity vector
		DEBUGRENDERER2D->DrawDirection(agentPosition, agentDirection, pAgent->GetLinearVelocity().Magnitude(), {1.f, 0.f, 1.f});

		//draws the desired velocity vector
		DEBUGRENDERER2D->DrawDirection(agentPosition, toTarget, pAgent->GetMaxLinearSpeed(), {0.f, 1.f, 0.f});

		//draws the desired velocity vector - current velocity vector
		DEBUGRENDERER2D->DrawDirection(agentPosition, toTarget.GetNormalized() - agentDirection, pAgent->GetMaxLinearSpeed() - pAgent->GetLinearVelocity().Magnitude(), {0.f, 1.f, 1.f});
	}

	return steering;
}

SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	const Vector2 agentPosition{ pAgent->GetPosition() };
	const Vector2 toTarget{ m_Target.Position - agentPosition };

	steering.LinearVelocity = toTarget;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * -1;

	if (pAgent->CanRenderBehavior())
	{
		const Vector2 agentDirection{ pAgent->GetDirection() };

		//draws the current velocity vector
		DEBUGRENDERER2D->DrawDirection(agentPosition, agentDirection, pAgent->GetLinearVelocity().Magnitude(), { 1.f, 0.f, 1.f });

		//draws the desired velocity vector
		DEBUGRENDERER2D->DrawDirection(agentPosition, toTarget, pAgent->GetMaxLinearSpeed(), { 0.f, 1.f, 0.f });

		//draws the desired velocity vector - current velocity vector
		DEBUGRENDERER2D->DrawDirection(agentPosition, toTarget.GetNormalized() - agentDirection, pAgent->GetMaxLinearSpeed() - pAgent->GetLinearVelocity().Magnitude(), { 0.f, 1.f, 1.f });

		//draws the opposite desired velocity vector
		DEBUGRENDERER2D->DrawDirection(agentPosition, -toTarget, pAgent->GetMaxLinearSpeed(), { 0.f, 1.f, 0.f });
	}

	return steering;
}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	Vector2 agentPosition{ pAgent->GetPosition() };
	Vector2 toTarget{ m_Target.Position - agentPosition };
	
	float distanceToTarget{ toTarget.Magnitude() };
	
	steering.LinearVelocity = toTarget;
	steering.LinearVelocity.Normalize();

	const float slowRadius{ 10.f };
	const float arrivalRadius{ 2.f };

	if (distanceToTarget <= arrivalRadius)
	{
		steering.LinearVelocity = {0.f, 0.f};
		steering.AngularVelocity = 0.f;
	}
	else if (distanceToTarget <= slowRadius && distanceToTarget > arrivalRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (distanceToTarget / slowRadius);
	}
	else 
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}

	if (pAgent->CanRenderBehavior())
	{
		Vector2 agentDirection{ pAgent->GetDirection() };

		//draws the current velocity vector
		DEBUGRENDERER2D->DrawDirection(agentPosition, agentDirection, pAgent->GetLinearVelocity().Magnitude(), { 1.f, 0.f, 1.f });

		//draws the desired velocity vector
		DEBUGRENDERER2D->DrawDirection(agentPosition, toTarget, pAgent->GetMaxLinearSpeed(), { 0.f, 1.f, 0.f });

		//draws the desired velocity vector - current velocity vector
		DEBUGRENDERER2D->DrawDirection(agentPosition, toTarget.GetNormalized() - agentDirection, pAgent->GetMaxLinearSpeed() - pAgent->GetLinearVelocity().Magnitude(), { 0.f, 1.f, 1.f });

		//draws the slow circle (if the target is in this circle the agent will slow down
		DEBUGRENDERER2D->DrawCircle(agentPosition, slowRadius, { 0.f, 0.f, 1.f }, 0.f);

		//draws the arrival circle (if the target is in this circle the agent will stop
		DEBUGRENDERER2D->DrawCircle(agentPosition, arrivalRadius, { 1.f, 0.f, 0.f }, 0.f);
	}
	
	return steering;
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(false);
	SteeringOutput steering{};

	pAgent->GetDirection().Normalize();

	Vector2 toTarget{ m_Target.Position - pAgent->GetPosition() };
	Vector2 rotationVector{ cosf(pAgent->GetRotation()), sinf(pAgent->GetRotation()) };

	if (AngleBetween(toTarget, rotationVector) < 0.1f && AngleBetween(toTarget, rotationVector) > -0.1f)
	{
		steering.AngularVelocity = 0.f;
	}
	else if (AngleBetween(toTarget, rotationVector) < 0)
	{
		steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
	}
	else if (AngleBetween(toTarget, rotationVector) > 0)
	{
		steering.AngularVelocity = -pAgent->GetMaxAngularSpeed();
	}
	return steering;
}

SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Vector2 circleOrigin{};
	circleOrigin.x = pAgent->GetPosition().x + cosf(pAgent->GetRotation()) * m_OffsetDistance;
	circleOrigin.y = pAgent->GetPosition().y + sinf(pAgent->GetRotation()) * m_OffsetDistance;

	const int maxWanderAngle{ int(m_WanderAngle + m_MaxAngleChange) + 360 };
	const int minWanderAngle{ int(m_WanderAngle - m_MaxAngleChange) + 360 };

	m_WanderAngle = static_cast<float>(rand() % (maxWanderAngle - minWanderAngle) + minWanderAngle);
	
	while (m_WanderAngle > 360)
	{
		m_WanderAngle -= 360;
	}

	Vector2 randomPointOnCircle{};
	randomPointOnCircle.x = circleOrigin.x + cosf(ToRadians(m_WanderAngle)) * m_Radius;
	randomPointOnCircle.y = circleOrigin.y + sinf(ToRadians(m_WanderAngle)) * m_Radius;
	
	m_Target.Position = randomPointOnCircle;

	SteeringOutput	steering{};
	steering = Seek::CalculateSteering(deltaT, pAgent);

	if (pAgent->CanRenderBehavior())
	{
		//draws a circle in front of the agent
		DEBUGRENDERER2D->DrawCircle(circleOrigin, m_Radius, { 0.f, 0.f, 1.f }, 0);

		//draws the random point on the circle
		DEBUGRENDERER2D->DrawPoint(randomPointOnCircle, 5.f, { 0.f, 1.f, 1.f });
	}

	return steering;
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Vector2 toTarget{ m_Target.Position - pAgent->GetPosition() };
	float t{toTarget.Magnitude() / pAgent->GetMaxLinearSpeed()}; // number of update cycles
	
	m_Target.Position += m_Target.LinearVelocity.GetNormalized() * pAgent->GetLinearVelocity().Magnitude() * t;

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawPoint(m_Target.Position, 5.f, { 1.f, 0.f, 1.f });
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}

SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Vector2 toTarget{ m_Target.Position - pAgent->GetPosition() };

	if (m_EvadeRadius <= toTarget.Magnitude())
	{
		SteeringOutput steering{};
		steering.IsValid = false;
		return steering;
	}

	float t{ toTarget.Magnitude() / pAgent->GetMaxLinearSpeed() }; // number of update cycles

	m_Target.Position += m_Target.LinearVelocity.GetNormalized() * pAgent->GetLinearVelocity().Magnitude() * t;

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawPoint(m_Target.Position, 5.f, { 1.f, 0.f, 1.f });
	}

	return Flee::CalculateSteering(deltaT, pAgent);
}