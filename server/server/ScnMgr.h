#pragma once
#include "Renderer.h"
#include "Object.h"
#include "Config.h"
#include <random>
#include <map>

class ScnMgr
{
private :
	int m_Width = WINDOW_WIDTH;
	int m_Height = WINDOW_HEIGHT;

	Object* m_background = NULL;
	Object* m_wall[4];
	Item* m_item[4];
	map<int, Player> m_players;
	int textures[4];
	int bullettextures[3];

	void DoGarbageCollection();
public:
	ScnMgr(int s);
	~ScnMgr();
	
	void Update(float fTimeElapsed);
};