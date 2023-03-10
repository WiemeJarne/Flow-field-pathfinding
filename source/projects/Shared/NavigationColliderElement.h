/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// LevelElement.h: container for adding static colliders that are flagged as
// navigation blockers
/*=============================================================================*/
#ifndef ELITE_NAVIGATION_COLLIDER
#define ELITE_NAVIGATION_COLLIDER
using namespace Elite;

class NavigationColliderElement final
{
public:
	//--- Constructor & Destructor
	NavigationColliderElement(const Elite::Vector2& position, float width, float height);
	~NavigationColliderElement();

	//--- Functions ---
	void RenderElement();
	Elite::Vector2 GetPosition() const { return m_Position; }

private:
	//--- Datamembers ---
	RigidBody* m_pRigidBody = nullptr;
	float m_Width = 0.0f;
	float m_Height = 0.0f;
	Elite::Vector2 m_Position = Elite::ZeroVector2;
};
#endif