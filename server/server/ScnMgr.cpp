#include "stdafx.h"
#include "ScnMgr.h"
#include <WinSock2.h>

ScnMgr::ScnMgr(int socket)
{
	//Add Background
	m_background = new Object();
	m_background->SetPos(0, 0);
	m_background->SetVol(m_Width / 100, m_Height / 100);

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
	m_item[0]->m_visible = true;
	m_item[1] = new Item();
	m_item[1]->SetPos(m_Width / 400, -m_Height / 400);
	m_item[1]->SetVol(0.2f, 0.2f);
	m_item[1]->m_visible = true;
	m_item[2] = new Item();
	m_item[2]->SetPos(-m_Width / 400, -m_Height / 400);
	m_item[2]->SetVol(0.2f, 0.2f);
	m_item[2]->m_visible = true;
	m_item[3] = new Item();
	m_item[3]->SetPos(-m_Width / 400, m_Height / 400);
	m_item[3]->SetVol(0.2f, 0.2f);
	m_item[3]->m_visible = true;

	//Add Hero Object
	m_players[HERO_ID] = Player();
	m_players[HERO_ID].SetPos(0.0f, 0.0f);
	m_players[HERO_ID].SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_players[HERO_ID].SetVol(0.3f, 0.3f);
	m_players[HERO_ID].SetVel(0.0f, 0.0f);
	m_players[HERO_ID].SetMass(1.0f);
	m_players[HERO_ID].SetFriction(0.6f);
}
ScnMgr::~ScnMgr()
{
}

void ScnMgr::Update(float fTimeElapsed)
{
	//플레이어 업데이트
	for (auto& p_player : m_players)
	{
		auto& player = p_player.second;
		//Move Player
		player.keyMove(fTimeElapsed);
		//총알 발사
		if (player.m_mouseLeft == true && player.CanShootBullet()) {
			player.ShootBullet(player.m_mousepos);
		}
		player.Update(fTimeElapsed);
	}

	//Collision Detect
	for (auto& p_pair : m_players)
	{
		if (p_pair.second.m_visible == false) continue;
		auto& player = p_pair.second;

		//플레이어, 벽 체크
		for (auto& wall : m_wall)
		{
			if (player.isOverlap(*wall)) {
				player.correctpos(*wall);
			}
		}

		//플레이어, 아이템 체크
		for (auto& item : m_item)
		{
			if (item->m_visible == false) continue;
			if (player.isOverlap(*item)) {
				player.weapon = item->type;
				item->m_visible = false;
			}
		}

		//총알, 플레이어 체크
		for (auto & bullet : player.bullets)
		{
			if (bullet.m_visible == false) continue;
			for (auto& p_other : m_players)
			{
				if (p_other.second.m_visible == false) continue;
				if (p_other.first == p_pair.first) continue;
				if (p_other.second.isOverlap(bullet)) {
					bullet.m_visible = false;
					//데미지처리 & 체력이 0이하면
					if (p_other.second.getDamage(bullet))
						p_other.second.die();
				}
			}
		}
	}

	DoGarbageCollection();
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