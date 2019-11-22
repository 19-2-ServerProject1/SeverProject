#pragma once

#include "stdafx.h"
#include "ScnMgr.h"
#include "Dependencies\freeglut.h"
#include "..\\..\\server\server\PacketMgr.h"
#include <WinSock2.h>

ScnMgr::ScnMgr(int socket, int id)
{
	//NW
	soc = socket;
	MYID = id;

	m_Renderer = new Renderer(m_Width, m_Height);
	if (!m_Renderer->IsInitialized())
	{
		std::cout << "Renderer could not be initialized.. \n";
	}

	textures[0] = m_Renderer->GenPngTexture("./Textures/bg.png");
	textures[1] = m_Renderer->GenPngTexture("./Textures/player.png");
	textures[2] = m_Renderer->GenPngTexture("./Textures/block.png");
	textures[3] = m_Renderer->GenPngTexture("./Textures/mushroom.png");

	bullettextures[0] = m_Renderer->GenPngTexture("./Textures/bullet1.png");
	bullettextures[1] = m_Renderer->GenPngTexture("./Textures/bullet2.png");
	bullettextures[2] = m_Renderer->GenPngTexture("./Textures/bullet3.png");

	//Add Background
	m_background = new Object();
	m_background->SetPos(0, 0);
	m_background->SetVol(m_Width / 100, m_Height / 100);
	m_background->SetTex(textures[0]);

	//Add Unvisible Wall
	m_wall[0] = new Object();
	m_wall[0]->SetPos((m_Width+m_Height) / 200, 0);
	m_wall[0]->SetVol(m_Height / 100, m_Height / 100);
	m_wall[1] = new Object();
	m_wall[1]->SetPos(-(m_Width + m_Height) / 200, 0);
	m_wall[1]->SetVol(m_Height / 100, m_Height / 100);
	m_wall[2] = new Object();
	m_wall[2]->SetPos(0, (m_Width + m_Height) / 200);
	m_wall[2]->SetVol(m_Width / 100, m_Width / 100);
	m_wall[3] = new Object();
	m_wall[3]->SetPos(0, -(m_Width + m_Height) / 200);
	m_wall[3]->SetVol(m_Width / 100, m_Width / 100);

	//Add Item
	m_item[0] = new Item();
	m_item[0]->SetPos(m_Width / 400, m_Height / 400);
	m_item[0]->SetVol(0.2f, 0.2f);
	m_item[0]->SetTex(textures[3]);
	m_item[0]->m_visible = true;
	m_item[1] = new Item();
	m_item[1]->SetPos(m_Width / 400, -m_Height / 400);
	m_item[1]->SetVol(0.2f, 0.2f);
	m_item[1]->SetTex(textures[3]);
	m_item[1]->m_visible = true;
	m_item[2] = new Item();
	m_item[2]->SetPos(-m_Width / 400, -m_Height / 400);
	m_item[2]->SetVol(0.2f, 0.2f);
	m_item[2]->SetTex(textures[3]);
	m_item[2]->m_visible = true;
	m_item[3] = new Item();
	m_item[3]->SetPos(-m_Width / 400, m_Height / 400);
	m_item[3]->SetVol(0.2f, 0.2f);
	m_item[3]->SetTex(textures[3]);
	m_item[3]->m_visible = true;

	//Add Hero Object
	m_players[MYID] = Player();
}
ScnMgr::~ScnMgr()
{
	if(m_Renderer)
		delete m_Renderer;
	m_Renderer = NULL;
}

void ScnMgr::Update(float fTimeElapsed)
{
	////Move Player
	//Vector2d keyDir(0, 0);
	//if (m_keyW)
	//{
	//	keyDir.y += 1.0f;
	//}
	//if (m_keyS)
	//{
	//	keyDir.y -= 1.0f;
	//}
	//if (m_keyA)
	//{
	//	keyDir.x -= 1.0f;
	//}
	//if (m_keyD)
	//{
	//	keyDir.x += 1.0f;
	//}
	//if (keyDir.length() > 0.0000001f)
	//{
	//	float fAmount = 10.0f;
	//	keyDir.normalize();
	//	keyDir *= fAmount;
	//	m_players[HERO_ID].AddForce(keyDir, fTimeElapsed);
	//}

	////Shoot Bullet
	//if (m_mouseLeft == true && m_players[HERO_ID].CanShootBullet()) {
	//	m_players[HERO_ID].ShootBullet(m_mousepos);
	//}

	//for (auto& p_player : m_players)
	//{
	//	p_player.second.Update(fTimeElapsed);
	//}

	////Collision Detect
	//for (auto& p_pair : m_players)
	//{
	//	if (p_pair.second.m_visible == false) continue;
	//	auto& player = p_pair.second;

	//	//플레이어, 벽 체크
	//	for (auto& wall : m_wall)
	//	{
	//		if (player.isOverlap(*wall)) {
	//			player.correctpos(*wall);
	//		}
	//	}

	//	//플레이어, 아이템 체크
	//	for (auto& item : m_item)
	//	{
	//		if (item->m_visible == false) continue;
	//		if (player.isOverlap(*item)) {
	//			player.weapon = item->type;
	//			item->m_visible = false;
	//		}
	//	}

	//	//총알, 플레이어 체크
	//	for (auto & bullet : player.bullets)
	//	{
	//		if (bullet.m_visible == false) continue;
	//		for (auto& p_other : m_players)
	//		{
	//			if (p_other.second.m_visible == false) continue;
	//			if (p_other.first == p_pair.first) continue;
	//			if (p_other.second.isOverlap(bullet)) {
	//				bullet.m_visible = false;
	//				//데미지처리 & 체력이 0이하면
	//				if (p_other.second.getDamage(bullet))
	//					p_other.second.die();
	//			}
	//		}
	//	}
	//}

	//DoGarbageCollection();
}
void ScnMgr::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	m_Renderer->DrawTextureRect(m_background->m_pos.x * 100, m_background->m_pos.y * 100, 0, m_background->m_vol.x * 100, m_background->m_vol.y * 100, 0, m_background->m_color[0], m_background->m_color[1], m_background->m_color[2], m_background->m_color[3], m_background->m_texID);

	for (auto& item : m_item)
	{
		if (item->m_visible == false) continue;
		m_Renderer->DrawTextureRect(item->m_pos.x * 100, item->m_pos.y * 100, 0, item->m_vol.x * 100, item->m_vol.y * 100, 0, item->m_color[0], item->m_color[1], item->m_color[2], item->m_color[3], item->m_texID);
	}

	for (auto &p : m_players) {
		auto& o = p.second;
		if (o.m_visible == false) continue;
		for (auto& b : o.bullets)
		{
			if (b.m_visible == false) continue;
			m_Renderer->DrawTextureRect(b.m_pos.x * 100, b.m_pos.y * 100, 0, 5, 5, 0, 1, 1, 1, 1, bullettextures[b.type]);
		}
		m_Renderer->DrawTextureRect(o.m_pos.x * 100, o.m_pos.y * 100, 0, o.m_vol.x * 100, o.m_vol.y * 100, 0, o.m_color[0], o.m_color[1], o.m_color[2], o.m_color[3], textures[1]);
	}
}

void ScnMgr::KeyDownInput(unsigned char key, int x, int y)
{
	int packet = 0;
	if (key == 'w' || key == 'W')
	{
		packet = make_packet_input(MYID, input_Wdown);
	}
	else if (key == 'a' || key == 'A')
	{
		packet = make_packet_input(MYID, input_Adown);
	}
	else if (key == 's' || key == 'S')
	{
		packet = make_packet_input(MYID, input_Sdown);
	}
	else if (key == 'd' || key == 'D')
	{
		packet = make_packet_input(MYID, input_Ddown);
	}
	send(soc, (const char*)& packet, sizeof(int), 0);
}
void ScnMgr::KeyUpInput(unsigned char key, int x, int y)
{
	int packet = 0;
	if (key == 'w' || key == 'W')
	{
		packet = make_packet_input(MYID, input_Wup);
	}
	else if (key == 'a' || key == 'A')
	{
		packet = make_packet_input(MYID, input_Aup);
	}
	else if (key == 's' || key == 'S')
	{
		packet = make_packet_input(MYID, input_Sup);
	}
	else if (key == 'd' || key == 'D')
	{
		packet = make_packet_input(MYID, input_Dup);
	}
	send(soc, (const char*)& packet, sizeof(int), 0);
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
			int packet = make_packet_input(MYID, input_Mleft);
			send(soc, (const char*)& packet, sizeof(int), 0);
			send(soc, (const char*)& m_mousepos, sizeof(Vector2d), 0);
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