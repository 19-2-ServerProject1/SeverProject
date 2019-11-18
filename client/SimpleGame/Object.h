#pragma once
class Object
{
private:
	int m_type;

	float m_posX, m_posY, m_posZ;
	float m_mass;
	float m_velX, m_velY, m_velZ;
	float m_accX, m_accY, m_accZ;
	float m_volX, m_volY, m_volZ;
	float m_friction;

	float m_r, m_g, m_b, m_a;

	float m_remainingBulletCoolTime = 0.0f;
	float m_defaultBulletCoolTime = 0.2f;

	int m_texID = -1;

public:
	Object();
	~Object();
	void InitPhysics();

	//Update
	void Update(float fTimeElapsed);

	// Get & Set
	void GetType(int *type);
	void SetType(int type);

	void GetTex(int *tex);
	void SetTex(int tex);

	void GetPos(float *x, float *y, float *z);
	void SetPos(float x, float y, float z);

	void GetMass(float *mass);
	void SetMass(float mass);

	void GetFriction(float *mass);
	void SetFriction(float mass);

	void GetAcc(float *x, float *y, float *z);
	void SetAcc(float x, float y, float z);

	void GetVel(float *x, float *y, float *z);
	void SetVel(float x, float y, float z);

	void GetVol(float *x, float *y, float *z);
	void SetVol(float x, float y, float z);

	void GetColor(float *r, float *g, float *b, float *a);
	void SetColor(float r, float g, float b, float a);

	void AddForce(float x, float y, float z, float fElapsedTime);

	bool CanShootBullet();
	void ResetShootBulletCoolTime();
};