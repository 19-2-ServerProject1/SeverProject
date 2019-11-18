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

	m_posX = 0.0f; m_posY = 0.0f; m_posZ = 0.0f;
	
	m_velX = 0.0f; m_velY = 0.0f; m_velZ = 0.0f;
	m_accX = 0.0f; m_accY = 0.0f; m_accZ = 0.0f;
	m_volX = 0.0f; m_volY = 0.0f; m_volZ = 0.0f;

	m_r = 1; m_g = 1; m_b = 1; m_a = 1;
	m_mass = 1.0f;
	m_friction = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////
void Object::Update(float fTimeElapsed)
{
	// Reduce Bullet Cooltime
	m_remainingBulletCoolTime -= fTimeElapsed;

	// Apply Friction
	float velSize = sqrtf(m_velX*m_velX + m_velY * m_velY);
	if (velSize > 0.0f)
	{
		float vX = m_velX / velSize;
		float vY = m_velY / velSize;
		//Calc friction size
		float nForce = m_mass * GRAVITY;
		float frictionSize = nForce * m_friction;
		float fX = -vX * frictionSize;
		float fY = -vY * frictionSize;
		//Calc acc from friction
		float accX = fX / m_mass;
		float accY = fY / m_mass;

		float newVelX = m_velX + accX * fTimeElapsed;
		float newVelY = m_velY + accY * fTimeElapsed;
		if (newVelX * m_velX < 0.f)
		{
			m_velX = 0;
		}
		else
		{
			m_velX = newVelX;
		}
		if (newVelY * m_velY < 0.f)
		{
			m_velY = 0;
		}
		else
		{
			m_velY = newVelY;
		}
	}

	m_posX += m_velX * fTimeElapsed;
	m_posY += m_velY * fTimeElapsed;
	m_posZ += m_velZ * fTimeElapsed;
}


void Object::GetType(int *type)
{
	*type = this->m_type;
}
void Object::SetType(int type)
{
	this->m_type = type;
}
void Object::GetTex(int *tex)
{
	*tex = this->m_texID;
}
void Object::SetTex(int tex)
{
	this->m_texID = tex;
}
void Object::GetPos(float *x, float *y, float *z) 
{
	*x = this->m_posX;
	*y = this->m_posY;
	*z = this->m_posZ;
}
void Object::SetPos(float x, float y, float z)
{
	this->m_posX = x;
	this->m_posY = y;
	this->m_posZ = z;
}

void Object::GetMass(float *mass)
{
	*mass = this->m_mass;
}
void Object::SetMass(float mass)
{
	this->m_mass = mass;
}

void Object::GetFriction(float *friction)
{
	*friction = this->m_friction;
}
void Object::SetFriction(float friction)
{
	this->m_friction = friction;
}

void Object::GetAcc(float *x, float *y, float *z)
{
	*x = this->m_accX;
	*y = this->m_accY;
	*z = this->m_accZ;
}
void Object::SetAcc(float x, float y, float z)
{
	this->m_accX = x;
	this->m_accY = y;
	this->m_accZ = z;
}

void Object::GetVel(float *x, float *y, float *z)
{
	*x = this->m_velX;
	*y = this->m_velY;
	*z = this->m_velZ;
}
void Object::SetVel(float x, float y, float z)
{
	this->m_velX = x;
	this->m_velY = y;
	this->m_velZ = z;
}

void Object::GetVol(float *x, float *y, float *z)
{
	*x = this->m_volX;
	*y = this->m_volY;
	*z = this->m_volZ;
}
void Object::SetVol(float x, float y, float z)
{
	this->m_volX = x;
	this->m_volY = y;
	this->m_volZ = z;
}

void Object::GetColor(float *r, float *g, float *b, float *a)
{
	*r = this->m_r;
	*g = this->m_g;
	*b = this->m_b;
	*a = this->m_a;
}
void Object::SetColor(float r, float g, float b, float a) 
{
	this->m_r = r;
	this->m_g = g;
	this->m_b = b;
	this->m_a = a;
}

void Object::AddForce(float x, float y, float z, float fElapsedTime)
{
	float accX, accY, accZ;
	accX = accY = accZ = 0;

	accX = x / m_mass;
	accY = y / m_mass;
	accZ = z / m_mass;

	m_velX = m_velX + accX * fElapsedTime;
	m_velY = m_velY + accY * fElapsedTime;
	m_velZ = m_velZ + accZ * fElapsedTime;
}

bool Object::CanShootBullet()
{
	return m_remainingBulletCoolTime <= 0.0000001f;
}

void Object::ResetShootBulletCoolTime()
{
	m_remainingBulletCoolTime = m_defaultBulletCoolTime;
}