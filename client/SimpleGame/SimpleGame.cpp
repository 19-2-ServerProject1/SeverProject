#pragma once

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

#include <thread>
#include <queue>
#include <mutex>
#include "..\..\server\server\PacketMgr.h"

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define MAX_BUFFER_SIZE = 50;

ScnMgr *g_ScnMgr = NULL;
SOCKET g_socket;
int g_PrevTime;

constexpr int WIDTH_BIAS = WINDOW_WIDTH / 2;
constexpr int HEIGHT_BIAS = WINDOW_HEIGHT / 2;


//Message Queue
mutex RecvLock;
queue<int> RecvQueue;
queue<Vector2d> RecvAddData;

void PacketReceiver(SOCKET socket)
{
	int retval;
	int packet = 0;
	char buffer[50];
	char savedPacket[50];
	Vector2d addData;
	bool isAdd = false;

	unsigned char* data = (unsigned char*)buffer;
	while (true) {
		isAdd = false;
		retval = recv(socket, buffer, sizeof(int), 0);
		if (retval == 0 || retval == SOCKET_ERROR)
		{
			closesocket(socket);
			return;
		}
		memcpy(&packet, buffer, sizeof(int));

		int packet_type = get_packet_type(packet);
		if (packet_type == p_obj) {
			int packet_info = get_packet_obj_info(packet);
			if (packet_info == obj_position) {
				isAdd = true;
				recv(socket, (char*)& addData, sizeof(addData), 0);
			}
		}

		RecvLock.lock();
		RecvQueue.emplace(packet);
		if (isAdd)
			RecvAddData.emplace(addData);
		RecvLock.unlock();
	}
}
void ProcessObjectPacket(const int& packet, queue<Vector2d>& addData) {
	int mover = get_packet_player_num(packet);
	int objtype = get_packet_obj_type(packet);
	int objinfo = get_packet_obj_info(packet);
	
	switch (objinfo)
	{
	case obj_position:
	{
			switch (objtype)
			{
				case obj_player: {
					Vector2d Data = addData.front(); addData.pop();
					g_ScnMgr->m_players[mover].m_visible = true;
					g_ScnMgr->m_players[mover].m_dst = Data;
				}
					break;

				case obj_bullet: {
					int idx = get_packet_bullet_idx(packet);
					int type = get_packet_bullet_type(packet);
					Vector2d Data = addData.front(); addData.pop();
					if (g_ScnMgr->m_players[mover].bullets[idx].m_visible == false)
						g_ScnMgr->m_players[mover].bullets[idx].m_pos = g_ScnMgr->m_players[mover].m_pos;
					g_ScnMgr->m_players[mover].bullets[idx].m_visible = true;
					g_ScnMgr->m_players[mover].bullets[idx].type = type;
					g_ScnMgr->m_players[mover].bullets[idx].m_dst = Data;
				}
					break;
			}	
	}
	break;

	case obj_destroy:
		switch (objtype)
		{
			case obj_player:
				g_ScnMgr->m_players[mover].m_visible = false;
				break;

			case obj_bullet: {
				int idx = get_packet_bullet_idx(packet);
				g_ScnMgr->m_players[mover].bullets[idx].m_visible = false;
				break;
			}

			case obj_item: {
				int idx = get_packet_bullet_idx(packet);
				g_ScnMgr->m_item[idx]->m_visible = false;
				break;
			}
		}
		break;
	}

}
void ProcessEventPacket(const int& packet) {
	int client = get_packet_player_num(packet);
	int event_type = get_packet_event_type(packet);

	switch (event_type)
	{
	case event_hit: {
		int damage = get_packet_bullet_idx(packet);
		g_ScnMgr->m_players[client].m_hp -= damage;
		break;
	}
	}
}
void ProcessSystemPacket(const int& packet) {
	int client = get_packet_player_num(packet);
	int system_type = get_packet_system_type(packet);

	switch (system_type)
	{
	case system_end: {
		g_ScnMgr->isEnd = true;
		g_ScnMgr->winner = client;
		break;
	}
	}
}
void ProcessPacket() {
	queue<int> cRecvQueue;
	queue<Vector2d> cAddData;

	RecvLock.lock();
	if (RecvQueue.empty() == false) {
		cRecvQueue = RecvQueue;
		while (RecvQueue.empty() == false)
			RecvQueue.pop();

		if (RecvAddData.empty() == false) {
			cAddData = RecvAddData;
			while (RecvAddData.empty() == false)
				RecvAddData.pop();
		}
	}
	RecvLock.unlock();

	while (cRecvQueue.empty() == false)
	{
		int packet = cRecvQueue.front(); cRecvQueue.pop();
		int packet_type = get_packet_type(packet);

		switch (packet_type)
		{
		case p_system:
			ProcessSystemPacket(packet);
			break;

		case p_obj:
			ProcessObjectPacket(packet, cAddData);
			break;

		case p_event:
			ProcessEventPacket(packet);
			break;
		}
	}
}

void RenderScene(int temp)
{
	int curTime = glutGet(GLUT_ELAPSED_TIME);
	int eTime = curTime - g_PrevTime;
	g_PrevTime = curTime;

	ProcessPacket();
	g_ScnMgr->Update(eTime / 1000.f);
	
	//Scne을 변경하는 건 이곳에서 하면 될 듯

	g_ScnMgr->StartScene();
	//g_ScnMgr->RenderScene();
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
	cout << "Connect To Server\n";

	int myid;
	recv(g_socket, (char*)& myid, sizeof(int), 0);
	cout << "My id : " << myid << endl;
	thread recvThread{ PacketReceiver, g_socket };

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
	g_ScnMgr = new ScnMgr(g_socket, myid);

	glutDisplayFunc(Display);
	glutIdleFunc(Idle);
	glutKeyboardFunc(KeyDownInput);
	glutKeyboardUpFunc(KeyUpInput);
	glutIgnoreKeyRepeat(1);
	glutMouseFunc(MouseInput);
	glutMotionFunc(MouseMotion);

	g_PrevTime = glutGet(GLUT_ELAPSED_TIME);
	glutTimerFunc(16, RenderScene, 0);

	glutMainLoop();

	recvThread.join();

	if(g_ScnMgr)
		delete g_ScnMgr;
	g_ScnMgr = NULL;

	closesocket(g_socket);
	WSACleanup();
    return 0;
}

