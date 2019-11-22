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

	int soc;

	Renderer *m_Renderer = NULL;
	Object* m_background = NULL;
	Object* m_wall[4];
	Item* m_item[4];
	map<int, Player> m_players;
	int textures[4];
	int bullettextures[3];

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

public:
	ScnMgr(int s);
	~ScnMgr();
	
	void Update(float fTimeElapsed);
	void RenderScene();

	// Input
	void KeyDownInput(unsigned char key, int x, int y);
	void KeyUpInput(unsigned char key, int x, int y);
	void SpecialKeyDownInput(unsigned char key, int x, int y);
	void SpecialKeyUpInput(unsigned char key, int x, int y);
	void MouseInput(int button, int state, int x, int y);
	void MouseMotion(int x, int y);

	// Objects
	int AddObject(Vector2d pos, Vector2d vol, Vector2d vel, float r, float g, float b, float a, float mass, float fricCoef, int type);
	void DeleteObject(int idx);
};