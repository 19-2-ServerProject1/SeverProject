#include "stdafx.h"
#include "Object.h"
#include "Config.h"
#include <iostream>
#include <math.h>


Object::Object()
{
	InitPhysics();
	m_remainingBulletCoolTime = m_defaultBulletCoolTime;
}
Object::~Object()
{
}

void Object::InitPhysics()
{
	m_type = TYPE_NORMAL;

	m_pos = Vector2d(0, 0);
	m_vel = Vector2d(0, 0);
	m_acc = Vector2d(0, 0);
	m_vol = Vector2d(0, 0);

	m_color[0] = 1; m_color[1] = 1; m_color[2] = 1; m_color[3] = 1;

	m_mass = 1.0f;
	m_friction = 0.0f;
}

void Object::SetPos(Vector2d pos)
{
	m_pos = pos;
}

void Object::SetPos(float x, float y)
{
	m_pos.x = x;
	m_pos.y = y;
}

void Object::SetVol(Vector2d vol)
{
	m_vol = vol;
}

void Object::SetColor(float r, float g, float b, float a)
{
	m_color[0] = r;
	m_color[1] = g;
	m_color[2] = b;
	m_color[3] = a;
}

void Object::SetVol(float x, float y)
{
	m_vol.x = x;
	m_vol.y = y;
}

void Object::SetVel(Vector2d vel)
{
	m_vel = vel;
}

void Object::SetVel(float x, float y)
{
	m_vel.x = x;
	m_vel.y = y;
}

void Object::SetMass(float mass)
{
	m_mass = mass;
}

void Object::SetFriction(float frict)
{
	m_friction = frict;
}

void Object::SetType(int type)
{
	m_type = type;
}

void Object::SetTex(int tex)
{
	m_texID = tex;
}

///////////////////////////////////////////////////////////////////////////////////////
void Object::Update(float fTimeElapsed)
{
	// Reduce Bullet Cooltime
	m_remainingBulletCoolTime -= fTimeElapsed;

	// Apply Friction
	float velSize = m_vel.length();
	if (velSize > 0.0f)
	{
		Vector2d acc = m_vel;
		acc.normalize();
		//Calc friction size
		float nForce = m_mass * GRAVITY;
		float frictionSize = nForce * m_friction;

		acc *= -frictionSize;
		acc /= m_mass;
		Vector2d newVel = m_vel + acc * fTimeElapsed;

		if (newVel.x * m_vel.x < 0.f)
		{
			m_vel.x = 0;
		}
		else
		{
			m_vel.x = newVel.x;
		}
		if (newVel.y * m_vel.y < 0.f)
		{
			m_vel.y = 0;
		}
		else
		{
			m_vel.y = newVel.y;
		}
	}

	m_pos += m_vel * fTimeElapsed;
}
void Object::AddForce(Vector2d force, float fElapsedTime)
{
	Vector2d acc = force / m_mass;
	m_vel += acc * fElapsedTime;
}

bool Object::CanShootBullet()
{
	return m_remainingBulletCoolTime <= 0.0000001f;
}

void Object::ResetShootBulletCoolTime()
{
	m_remainingBulletCoolTime = m_defaultBulletCoolTime;
}