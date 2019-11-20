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
		m_players[HERO_ID].ShootBullet(m_mousepos);
	}

	m_players[HERO_ID].Update(fTimeElapsed);

	DoGarbageCollection();
}
void ScnMgr::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	m_Renderer->DrawTextureRect(m_background->m_pos.x * 100, m_background->m_pos.y * 100, 0, m_background->m_vol.x * 100, m_background->m_vol.y * 100, 0, m_background->m_color[0], m_background->m_color[1], m_background->m_color[2], m_background->m_color[3], m_background->m_texID);

	for (auto &p : m_players) {
		auto& o = p.second;
		for (auto& b : o.bullets)
		{
			m_Renderer->DrawTextureRect(b.m_pos.x * 100, b.m_pos.y * 100, 0, b.m_vol.x * 100, b.m_vol.y * 100, 0, b.m_color[0], b.m_color[1], b.m_color[2], b.m_color[3], b.m_texID);
		}
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

void ScnMgr::DoGarbageCollection()
{
	//delete bullets
	for (auto& p_pair : m_players)
	{
		auto& player = p_pair.second;
		if (player.m_visible == false) continue;

		for (auto& b : player.bullets)
		{
			if (b.m_visible == false) continue;
			if (b.m_vel.length() < 0.00001f)
				b.m_visible = false;
		}
	}
}