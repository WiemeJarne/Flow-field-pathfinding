#pragma once
#include "../Steering/SteeringBehaviors.h"

class Flock;

//****************
//BLENDED STEERING
class BlendedSteering final: public ISteeringBehavior
{
public:
	struct WeightedBehavior
	{
		ISteeringBehavior* pBehavior = nullptr;
		float weight = 0.f;

		WeightedBehavior(ISteeringBehavior* pBehavior, float weight) :
			pBehavior(pBehavior),
			weight(weight)
		{};
	};

	BlendedSteering(std::vector<WeightedBehavior> weightedBehaviors);

	void AddBehaviour(WeightedBehavior weightedBehavior) { m_WeightedBehaviors.push_back(weightedBehavior); }
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	// returns a reference to the weighted behaviors, can be used to adjust weighting. Is not intended to alter the behaviors themselves.
	std::vector<WeightedBehavior>& GetWeightedBehaviorsRef() { return m_WeightedBehaviors; }

private:
	std::vector<WeightedBehavior> m_WeightedBehaviors = {};

	using ISteeringBehavior::SetTarget; // made private because targets need to be set on the individual behaviors, not the combined behavior
};

//*****************
//PRIORITY STEERING
class PrioritySteering final: public ISteeringBehavior
{
public:
	PrioritySteering(std::vector<ISteeringBehavior*> priorityBehaviors)
		:m_PriorityBehaviors(priorityBehaviors) 
	{}

	void AddBehaviour(ISteeringBehavior* pBehavior) { m_PriorityBehaviors.push_back(pBehavior); }
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	std::vector<ISteeringBehavior*> m_PriorityBehaviors = {};

	using ISteeringBehavior::SetTarget; // made private because targets need to be set on the individual behaviors, not the combined behavior
};