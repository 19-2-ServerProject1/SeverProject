#pragma once
#include "Renderer.h"
#include "Object.h"
#include "Config.h"
#include <random>
#include <map>

class ScnMgr
{
public:
	enum scn_state {state_title, state_connect, state_connect_error, state_wait, state_play, state_end};

	int m_Width = WINDOW_WIDTH;
	int m_Height = WINDOW_HEIGHT;

	//NW
	int soc;
	int MYID;
	bool isEnd = false;
	int winner = -1;
	int m_state;

	Renderer *m_Renderer = NULL;
	Object* m_wall[4];
	Object* m_block[8];
	Item* m_item[4];
	map<int, Player> m_players;

	int textures[4];
	int bullettextures[3];
	int winlose[2];
	int hpbar;
	int state_texture[4];
	int item_texture[3];

	//Key Inputs
	bool m_keyW = false;
	bool m_keyA = false;
	bool m_keyS = false;
	bool m_keyD = false;

	//Mouse Input
	bool m_mouseLeft = false;
	Vector2d m_mousepos;

	void DoGarbageCollection();

	ScnMgr();
	~ScnMgr();
	
	void Update(float fTimeElapsed);
	void RenderScene();
	void Init();

	// Input
	void KeyDownInput(unsigned char key, int x, int y);
	void KeyUpInput(unsigned char key, int x, int y);
	void MouseInput(int button, int state, int x, int y);
	void MouseMotion(int x, int y);
};