#include "StartScene.h"



void StartScene::Button_Input(int * x, int * y, int * event)
{


}

StartScene::StartScene()
{

	m_Renderer = new Renderer(500, 500);
	
	
	if (!m_Renderer->IsInitialized())
	{
		std::cout << "Renderer could not be initialized.. \n";
	}


	//Main 화면을 만들어보자!

	
	//Button

}


StartScene::~StartScene()
{
}


