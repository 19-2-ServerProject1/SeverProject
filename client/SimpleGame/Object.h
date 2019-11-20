#pragma once
#include "Vector2d.h"

class Object
{
public:
	bool m_visible;

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
};

class Bullet : public Object
{
public:
	Bullet() : Object() {};
	~Bullet(){};

	int type;
	int damage;
};

class Player : public Object
{
private:
	float m_remainingBulletCoolTime = 0.0f;
	float m_defaultBulletCoolTime[3] = { 0.4f, 0.3f, 0.5f };

public:
	int weapon = 0;
	Bullet bullets[100];

	bool m_key[4];
	bool m_mouse;

	Player();
	~Player();
	void Update(float fTimeElapsed);
	bool CanShootBullet();
	void ResetShootBulletCoolTime();
	int AddBullet(Vector2d pos, Vector2d vol, Vector2d vel, float r, float g, float b, float a, float mass, float fricCoef);
	void ShootBullet(Vector2d MousePos);
};