#include "stdafx.h"
#include "ScnMgr.h"
#include "Dependencies\freeglut.h"

#include <WinSock2.h>

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

	m_players[HERO_ID] = Player();

	//Add Hero Object
	m_players[HERO_ID].SetPos(0.0f, 0.0f);
	m_players[HERO_ID].SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_players[HERO_ID].SetVol(0.5f, 0.5f);
	m_players[HERO_ID].SetVel(0.0f, 0.0f);
	m_players[HERO_ID].SetMass(1.0f);
	m_players[HERO_ID].SetFriction(0.6f);

	//Add Background
	m_background = new Object();
	m_background->SetPos(0, 0);
	m_background->SetVol(m_Width / 100, m_Height / 100);

	textures[0] = m_Renderer->GenPngTexture("./Textures/bg.png");
	textures[1] = m_Renderer->GenPngTexture("./Textures/player.png");
	textures[2] = m_Renderer->GenPngTexture("./Textures/block.png");
	m_background->SetTex(textures[0]);
	m_players[HERO_ID].SetTex(textures[1]);
}
ScnMgr::~ScnMgr()
{
	if(m_Renderer)
		delete m_Renderer;
	m_Renderer = NULL;
}

void ScnMgr::Update(float fTimeElapsed)
{
	//Move Player
	Vector2d keyDir(0, 0);
	if (m_keyW)
	{
		keyDir.y += 1.0f;
	}
	if (m_keyS)
	{
		keyDir.y -= 1.0f;
	}
	if (m_keyA)
	{
		keyDir.x -= 1.0f;
	}
	if (m_keyD)
	{
		keyDir.x += 1.0f;
	}
	if (keyDir.length() > 0.0000001f)
	{
		float fAmount = 10.0f;
		keyDir.normalize();
		keyDir *= fAmount;
		m_players[HERO_ID].AddForce(keyDir, fTimeElapsed);
	}

	//Shoot Bullet
	if (m_mouseLeft == true && m_players[HERO_ID].CanShootBullet()) {
		float fAmountBullet = 8.0f;
		Vector2d bulletDir = m_mousepos / 100;
		bulletDir -= m_players[HERO_ID].m_pos;
		bulletDir.normalize();
		
		Vector2d hVel = m_players[HERO_ID].m_vel + bulletDir * fAmountBullet;
		
		Vector2d vol(0.05f, 0.05f);
		float mass = 1.0f;
		float fricCoef = 0.9f;
		int type = TYPE_BULLET;

		AddObject(m_players[HERO_ID].m_pos, vol, hVel, 1, 0, 0, 1, mass, fricCoef, type);
		m_players[HERO_ID].ResetShootBulletCoolTime();
	}

	m_players[HERO_ID].Update(fTimeElapsed);
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

	m_Renderer->DrawTextureRect(m_background->m_pos.x * 100, m_background->m_pos.y * 100, 0, m_background->m_vol.x * 100, m_background->m_vol.y * 100, 0, m_background->m_color[0], m_background->m_color[1], m_background->m_color[2], m_background->m_color[3], m_background->m_texID);

	for (Object *&o : m_ObjList)
	{
		if (o == NULL) continue;
		m_Renderer->DrawTextureRect(o->m_pos.x*100, o->m_pos.y*100, 0, o->m_vol.x*100, o->m_vol.y*100, 0, o->m_color[0], o->m_color[1], o->m_color[2], o->m_color[3], o->m_texID);
	}

	for (auto &p : m_players) {
		auto o = p.second;
		m_Renderer->DrawTextureRect(o.m_pos.x * 100, o.m_pos.y * 100, 0, o.m_vol.x * 100, o.m_vol.y * 100, 0, o.m_color[0], o.m_color[1], o.m_color[2], o.m_color[3], o.m_texID);
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

int ScnMgr::AddObject(Vector2d pos, Vector2d vol, Vector2d vel, float r, float g, float b, float a, float mass, float fricCoef, int type)
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
	m_ObjList[idx]->SetPos(pos);
	m_ObjList[idx]->SetColor(r, g, b, a);
	m_ObjList[idx]->SetVol(vol);
	m_ObjList[idx]->SetVel(vel);
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
		int Type = m_ObjList[i]->m_type;
		if (Type == TYPE_BULLET)
		{
			if (m_ObjList[i]->m_vel.length() < 0.00001f) DeleteObject(i);
			continue;
		}
	}
}