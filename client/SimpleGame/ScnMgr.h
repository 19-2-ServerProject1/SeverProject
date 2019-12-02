#pragma once
#include "Renderer.h"
#include "Object.h"
#include "Config.h"
#include <random>
#include <map>

class ScnMgr
{
public:
	int m_Width = WINDOW_WIDTH;
	int m_Height = WINDOW_HEIGHT;

	//NW
	int soc;
	int MYID;
	bool isEnd = false;
	int winner = -1;

	Renderer *m_Renderer = NULL;
	Object* m_background = NULL;
	Object* m_wall[4];
	
	Object* m_Button[4];
	
	Item* m_item[4];
	map<int, Player> m_players;
	
	int button_textures[4];
	
	int textures[4];
	int bullettextures[3];
	int winlose[2];
	int hpbar;

	//Key Inputs
	bool m_keyW = false;
	bool m_keyA = false;
	bool m_keyS = false;
	bool m_keyD = false;

	//Special Key Inputs
	bool m_keyUp = false;
	bool m_keyDown = false;
	bool m_keyLeft = false;
	bool m_keyRight = false;

	//Mouse Input
	bool m_mouseLeft = false;
	Vector2d m_mousepos;

	void DoGarbageCollection();


	ScnMgr(int s, int id);
	~ScnMgr();
	
	void Update(float fTimeElapsed);
	void RenderScene();
	void StartScene();
	// Input
	void KeyDownInput(unsigned char key, int x, int y);
	void KeyUpInput(unsigned char key, int x, int y);
	void SpecialKeyDownInput(unsigned char key, int x, int y);
	void SpecialKeyUpInput(unsigned char key, int x, int y);
	void MouseInput(int button, int state, int x, int y);
	void MouseMotion(int x, int y);
	
	//

};