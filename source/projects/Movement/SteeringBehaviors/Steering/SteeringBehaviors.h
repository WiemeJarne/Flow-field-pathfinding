/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "../SteeringHelpers.h"
class SteeringAgent;
class Obstacle;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

class Arrive final : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

class Face final : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

class Wander final : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	void SetWanderOffset(float offset) { m_OffsetDistance = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_MaxAngleChange = rad; }

protected:
	float m_OffsetDistance{ 6.f }; //offset in agent direction
	float m_Radius{ 4.f }; //wanderRadius
	float m_MaxAngleChange{ 15.f }; //max wanderingAngle change per frame
	float m_WanderAngle{ 0.f };
};

class Pursuit final : public Seek
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

class Evade final : public Flee
{
public:
	Evade() = default;
	virtual ~Evade() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	int GetEvadeRadius() const { return m_EvadeRadius; };

private:
	const int m_EvadeRadius{ 25 };
};
#endif