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


	//Main ȭ���� ������!

	
	//Button

}


StartScene::~StartScene()
{
}


