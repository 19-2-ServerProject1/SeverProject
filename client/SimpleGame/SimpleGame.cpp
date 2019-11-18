/*
Copyright 2017 Lee Taek Hee (Korea Polytech University)

This program is free software: you can redistribute it and/or modify
it under the terms of the What The Hell License. Do it plz.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.
*/

#include "stdafx.h"
#include <iostream>
#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"

#include "ScnMgr.h"
#include "Config.h"
#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000

ScnMgr *g_ScnMgr = NULL;
SOCKET g_socket;
int g_PrevTime;

constexpr int WIDTH_BIAS = WINDOW_WIDTH / 2;
constexpr int HEIGHT_BIAS = WINDOW_HEIGHT / 2;

void RenderScene(int temp)
{
	int curTime = glutGet(GLUT_ELAPSED_TIME);
	int eTime = curTime - g_PrevTime;
	g_PrevTime = curTime;

	g_ScnMgr->Update(eTime / 1000.f);
	g_ScnMgr->RenderScene();
	glutSwapBuffers();

	glutTimerFunc(8, RenderScene, 0);
}

void Display(void)
{

}

void Idle(void)
{
}

void MouseInput(int button, int state, int x, int y)
{
	std::cout << "X : " << x << ", Y : " << y << endl;
	g_ScnMgr->MouseInput(button, state, x-WIDTH_BIAS, -y+HEIGHT_BIAS);
}
void MouseMotion(int x, int y)
{
	g_ScnMgr->MouseMotion(x - WIDTH_BIAS, -y + HEIGHT_BIAS);
}

void KeyDownInput(unsigned char key, int x, int y)
{
	g_ScnMgr->KeyDownInput(key, x, y);
}
void KeyUpInput(unsigned char key, int x, int y)
{
	g_ScnMgr->KeyUpInput(key, x, y);
}

void SpecialKeyDownInput(int key, int x, int y)
{
	g_ScnMgr->SpecialKeyDownInput(key, x, y);
}
void SpecialKeyUpInput(int key, int x, int y)
{
	g_ScnMgr->SpecialKeyUpInput(key, x, y);
}

int main(int argc, char **argv)
{
	// Initialize Socket
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVERPORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	::connect(g_socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	// Initialize GL things
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Game Software Engineering KPU");

	glewInit();
	if (glewIsSupported("GL_VERSION_3_0"))
	{
		std::cout << " GLEW Version is 3.0\n ";
	}
	else
	{
		std::cout << "GLEW 3.0 not supported\n ";
	}

	// Initialize Renderer
	g_ScnMgr = new ScnMgr(g_socket);

	glutDisplayFunc(Display);
	glutIdleFunc(Idle);
	glutKeyboardFunc(KeyDownInput);
	glutKeyboardUpFunc(KeyUpInput);
	glutIgnoreKeyRepeat(1);
	glutMouseFunc(MouseInput);
	glutMotionFunc(MouseMotion);
	//glutSpecialFunc(SpecialKeyDownInput);
	//glutSpecialUpFunc(SpecialKeyUpInput);

	g_PrevTime = glutGet(GLUT_ELAPSED_TIME);
	glutTimerFunc(16, RenderScene, 0);

	glutMainLoop();

	if(g_ScnMgr)
		delete g_ScnMgr;
	g_ScnMgr = NULL;

	closesocket(g_socket);
	WSACleanup();
    return 0;
}

