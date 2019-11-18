#include "stdafx.h"
#include "ScnMgr.h"
#include "Dependencies\freeglut.h"

#include <WinSock2.h>

int g_testTexture = -1;

ScnMgr::ScnMgr(int socket)
{
	soc = socket;

	m_Renderer = new Renderer(m_Width, m_Height);
	if (!m_Renderer->IsInitialized())
	{
		std::cout << "Renderer could not be initialized.. \n";
	}

	for (int i = 0; i < MAX_OBJECTS; ++i)
		m_ObjList[i] = NULL;

	//Add Hero Object
	m_ObjList[HERO_ID] = new Object();
	m_ObjList[HERO_ID]->SetPos(0.0f, 0.0f, 0.0f);
	m_ObjList[HERO_ID]->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_ObjList[HERO_ID]->SetVol(1.0f, 0.2f, 0.2f);
	m_ObjList[HERO_ID]->SetVel(0.0f, 0.0f, 0.0f);
	m_ObjList[HERO_ID]->SetMass(1.0f);
	m_ObjList[HERO_ID]->SetFriction(0.7f);

	g_testTexture = m_Renderer->GenPngTexture("./Textures/circle.png");
	m_ObjList[HERO_ID]->SetTex(g_testTexture);
}
ScnMgr::~ScnMgr()
{
	if(m_Renderer)
		delete m_Renderer;
	m_Renderer = NULL;
}

void ScnMgr::Update(float fTimeElapsed)
{
	float fx = 0, fy = 0, fz = 0;
	float fAmount = 10.0f;
	if (m_keyW)
	{
		fy += 1.0f * fAmount;
	}
	if (m_keyS)
	{
		fy -= 1.0f * fAmount;
	}
	if (m_keyA)
	{
		fx -= 1.0f * fAmount;
	}
	if (m_keyD)
	{
		fx += 1.0f * fAmount;
	}
	float fSize = sqrtf(fx*fx + fy * fy + fz*fz);
	if (fSize > 0.0000001f)
	{
		fx /= fSize;
		fy /= fSize;
		fz /= fSize;

		fx *= fAmount;
		fy *= fAmount;
		fz *= fAmount;
		m_ObjList[HERO_ID]->AddForce(fx, fy, fz, fTimeElapsed);
	}

	if (m_mouseLeft == true && m_ObjList[HERO_ID]->CanShootBullet()) {
		float fAmountBullet = 8.0f;

		float x, y, z;
		m_ObjList[HERO_ID]->GetPos(&x, &y, &z);

		Vector2d dir(m_mousepos.x - 100*x, m_mousepos.y - 100*y);
		dir.normalize();
		
		float hX, hY, hZ;
		m_ObjList[HERO_ID]->GetVel(&hX, &hY, &hZ);
		hX = hX + dir.x * fAmountBullet;
		hY = hY + dir.y * fAmountBullet;
		hZ = 0;

		float s = 0.05f;
		float mass = 1.0f;
		float fricCoef = 0.9f;
		int type = TYPE_BULLET;

		AddObject(x, y, z, s, s, s, 1, 0, 0, 1, hX, hY, hZ, mass, fricCoef, type);
		m_ObjList[HERO_ID]->ResetShootBulletCoolTime();
	}

	for (Object *&o : m_ObjList)
	{
		if (o == NULL) continue;
		o->Update(fTimeElapsed);
	}

	DoGarbageCollection();
}
void ScnMgr::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	for (Object *&o : m_ObjList)
	{
		if (o == NULL) continue;

		float x, y, z = 0.0f;
		float sx, sy, sz = 0.0f;
		float r, g, b, a = 0.0f;
		int tex = -1;
		o->GetPos(&x, &y, &z);
		x *= 100.0f;
		y *= 100.0f;
		z *= 100.0f;
		o->GetVol(&sx, &sy, &sz);
		sx *= 100.0f;
		sy *= 100.0f;
		sz *= 100.0f;
		o->GetColor(&r, &g, &b, &a);
		o->GetTex(&tex);
		m_Renderer->DrawSolidRect(x, y, z, sx, r, g, b, a);
		//m_Renderer->DrawTextureRect(x, y, z, sx, sy, sz, r, g, b, a, tex);
	}
}

void ScnMgr::KeyDownInput(unsigned char key, int x, int y)
{
	if (key == 'w' || key == 'W')
	{
		m_keyW = true;
	}
	else if (key == 'a' || key == 'A')
	{
		m_keyA = true;
	}
	else if (key == 's' || key == 'S')
	{
		m_keyS = true;
	}
	else if (key == 'd' || key == 'D')
	{
		m_keyD = true;
	}
}
void ScnMgr::KeyUpInput(unsigned char key, int x, int y)
{
	if (key == 'w' || key == 'W')
	{
		m_keyW = false;
	}
	else if (key == 'a' || key == 'A')
	{
		m_keyA = false;
	}
	else if (key == 's' || key == 'S')
	{
		m_keyS = false;
	}
	else if (key == 'd' || key == 'D')
	{
		m_keyD = false;
	}
}
void ScnMgr::SpecialKeyDownInput(unsigned char key, int x, int y)
{
	if (key == GLUT_KEY_UP)
	{
		m_keyUp = true;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		m_keyDown = true;
	}
	else if (key == GLUT_KEY_LEFT)
	{
		m_keyLeft = true;
	}
	else if (key == GLUT_KEY_RIGHT)
	{
		m_keyRight = true;
	}
}
void ScnMgr::SpecialKeyUpInput(unsigned char key, int x, int y)
{
	if (key == GLUT_KEY_UP)
	{
		m_keyUp = false;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		m_keyDown = false;
	}
	else if (key == GLUT_KEY_LEFT)
	{
		m_keyLeft = false;
	}
	else if (key == GLUT_KEY_RIGHT)
	{
		m_keyRight = false;
	}
}
void ScnMgr::MouseInput(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			m_mouseLeft = true;
			m_mousepos.x = x; m_mousepos.y = y;
		}
		else if (state == GLUT_UP) m_mouseLeft = false;
	}
}
void ScnMgr::MouseMotion(int x, int y)
{
	if (m_mouseLeft == true) {
		m_mousepos.x = x;
		m_mousepos.y = y;
	}
}

int ScnMgr::AddObject(float x, float y, float z, float sx, float sy, float sz, float r, float g, float b, float a, float vx, float vy, float vz, float mass, float fricCoef, int type)
{
	int idx = -1;
	for (int i = 0; i < MAX_OBJECTS; ++i)
	{
		if (m_ObjList[i] == NULL)
		{
			idx = i;
			break;
		}
	}

	if (idx == -1)
	{
		std::cout << "No more remaining object" << std::endl;
		return idx;
	}

	m_ObjList[idx] = new Object();
	m_ObjList[idx]->SetPos(x, y, z);
	m_ObjList[idx]->SetColor(r, g, b, a);
	m_ObjList[idx]->SetVol(sx, sy, sz);
	m_ObjList[idx]->SetVel(vx, vy, vz);
	m_ObjList[idx]->SetMass(mass);
	m_ObjList[idx]->SetFriction(fricCoef);
	m_ObjList[idx]->SetType(type);

	return idx;
}
void ScnMgr::DeleteObject(int idx)
{
	if (idx < 0)
	{
		std::cout << "unvalid Index - " << idx << std::endl;
		return;
	}
	if (idx >= MAX_OBJECTS)
	{
		std::cout << "unvalid Index - " << idx << std::endl;
		return;
	}
	if (m_ObjList[idx] != NULL)
	{
		delete m_ObjList[idx];
		m_ObjList[idx] = NULL;
	}
}

void ScnMgr::DoGarbageCollection()
{
	//delete bullets
	for (int i = 0; i < MAX_OBJECTS; ++i)
	{
		if (m_ObjList[i] == NULL) continue;

		int Type = -1;
		m_ObjList[i]->GetType(&Type);

		if (Type == TYPE_BULLET)
		{
			float vx, vy, vz;
			m_ObjList[i]->GetVel(&vx, &vy, &vz);
			float vSize = sqrtf(vx*vx + vy * vy + vz * vz);
			if (vSize < 0.00001f) DeleteObject(i);
			continue;
		}
	}
}