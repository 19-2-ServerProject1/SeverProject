#pragma once
#include "Renderer.h"
#include "Object.h"
#include "Config.h"
#include <random>
#include "Vector2d.h"

class ScnMgr
{
private :
	int m_Width = 500;
	int m_Height = 500;

	int soc;

	Renderer *m_Renderer = NULL;
	Object* m_ObjList[MAX_OBJECTS];

	int m_TestIdx;
	int m_TestIdxArray[MAX_OBJECTS];

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
	int AddObject(float x, float y, float z, float sx, float sy, float sz, float r, float g, float b, float a, float vx, float vy, float vz, float mass, float fricCoef, int type);
	void DeleteObject(int idx);
};