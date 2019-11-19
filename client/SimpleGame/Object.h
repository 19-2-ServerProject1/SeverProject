#pragma once
#include "Vector2d.h"

class Object
{
private:

	float m_remainingBulletCoolTime = 0.0f;
	float m_defaultBulletCoolTime = 0.2f;

public:
	int m_type;
	Vector2d m_pos;
	Vector2d m_vel;
	Vector2d m_acc;
	Vector2d m_vol;
	float m_mass;
	float m_friction;
	int m_texID = -1;
	float m_color[4];

	Object();
	~Object();
	void InitPhysics();

	//Set
	void SetPos(Vector2d pos);
	void SetPos(float x, float y);
	void SetVol(Vector2d vol);
	void SetVol(float x, float y);
	void SetVel(Vector2d vel);
	void SetVel(float x, float y);
	void SetColor(float r, float g, float b, float a);
	void SetMass(float mass);
	void SetFriction(float frict);
	void SetType(int type);
	void SetTex(int tex);

	//Update
	void Update(float fTimeElapsed);

	void AddForce(Vector2d force, float fElapsedTime);

	bool CanShootBullet();
	void ResetShootBulletCoolTime();
};