#include "stdafx.h"
#include "BaseAgent.h"

BaseAgent::BaseAgent(float radius) : m_Radius(radius)
{
	//Create Rigidbody
	const Elite::RigidBodyDefine define = Elite::RigidBodyDefine(0.01f, 0.1f, Elite::eDynamic, false);
	const Transform transform = Transform(Elite::ZeroVector2, {0,90});
	m_pRigidBody = new RigidBody(define, transform);

	//Add shape
	Elite::EPhysicsCircleShape shape;
	shape.radius = m_Radius;
	m_pRigidBody->AddShape(&shape);
}


BaseAgent::~BaseAgent()
{
	SAFE_DELETE(m_pRigidBody);
}

void BaseAgent::Update(float dt)
{
}

void BaseAgent::Render(float dt)
{
	auto o = GetRotation();
	auto p = GetPosition();
	auto r = Elite::ToRadians(150.f);

	//EliteDebugRenderer2D::GetInstance()->DrawSolidCircle(GetPosition(), m_Radius, { 0,0 }, m_BodyColor);
	DEBUGRENDERER2D->DrawSolidCircle(GetPosition(), m_Radius, { 0,0 }, m_BodyColor);

	std::vector<Elite::Vector2> points;
	points.push_back(Elite::Vector2(static_cast<float>(cos(o)), static_cast<float>(sin(o)) * m_Radius) + p);
	points.push_back(Elite::Vector2(static_cast<float>(cos(o + r)), static_cast<float>(sin(o + r)) * m_Radius) + p);
	points.push_back(Elite::Vector2(static_cast<float>(cos(o - r)), static_cast<float>(sin(o - r)) * m_Radius) + p);

	DEBUGRENDERER2D->DrawSolidPolygon(&points[0], 3, { 0,0,0,1 }, DEBUGRENDERER2D->NextDepthSlice());
}
void BaseAgent::TrimToWorld(float worldBounds, bool isWorldLooping) const {
	TrimToWorld({ 0, 0 }, { worldBounds, worldBounds }, isWorldLooping);
}
void BaseAgent::TrimToWorld(const Elite::Vector2& bottomLeft, const Elite::Vector2& topRight, bool isWorldLooping /* = true */) const
{
	auto pos = GetPosition();

	if (isWorldLooping)
	{
		if (pos.x > topRight.x)
			pos.x = bottomLeft.x;
		else if (pos.x < bottomLeft.x)
			pos.x = topRight.x;

		if (pos.y > topRight.y)
			pos.y = bottomLeft.y;
		else if (pos.y < bottomLeft.y)
			pos.y = topRight.y;
	}
	else
	{
		pos.x = Elite::Clamp(pos.x, bottomLeft.x, topRight.x);
		pos.y = Elite::Clamp(pos.y, bottomLeft.y, topRight.y);
	}

	SetPosition(pos);
}