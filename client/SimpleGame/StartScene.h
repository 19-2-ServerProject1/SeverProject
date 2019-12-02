#pragma once
#include "Renderer.h"
#include "ScnMgr.h"
class StartScene
{

private:
	int answer;

public:
	
	Renderer * m_Renderer = NULL;
	ScnMgr * m_ScnMgr = NULL;
	void Button_Input(int * x, int * y, int * event);

	StartScene();
	~StartScene();
};
