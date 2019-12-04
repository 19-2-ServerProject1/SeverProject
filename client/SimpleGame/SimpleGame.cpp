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
#include <fstream>
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


#define SERVERPORT 9000
#define MAX_BUFFER_SIZE 2000

ScnMgr *g_ScnMgr = NULL;
SOCKET g_socket;
string SERVERIP;
int g_PrevTime;

constexpr int WIDTH_BIAS = WINDOW_WIDTH / 2;
constexpr int HEIGHT_BIAS = WINDOW_HEIGHT / 2;


//Message Queue
HANDLE connect_success;
mutex RecvLock;
queue<int> RecvQueue;
queue<Vector2d> RecvAddData;

void PacketReceiver(SOCKET socket)
{
	int retval;
	
	size_t need_size = GENERAL_PACKET_SIZE;
	size_t saved_size = 0;

	int packet = 0;
	Vector2d addData;
	char buffer[MAX_BUFFER_SIZE];
	char savedPacket[EXTEND_PACKET_SIZE];
	char* buf_pos = NULL;

	queue<int> tempQueue;
	queue<Vector2d> tempAddQueue;

	while (true) {
		retval = recv(socket, buffer, MAX_BUFFER_SIZE, 0);
		if (retval == 0 || retval == SOCKET_ERROR)
		{
			closesocket(socket);
			return;
		}

		buf_pos = buffer;
		while (retval > 0)
		{
			if (retval + saved_size >= need_size) {
				int copy_size = (need_size - saved_size);
				memcpy(savedPacket + saved_size, buf_pos, copy_size);
				buf_pos += copy_size;
				retval -= copy_size;

				switch (need_size) {
				case GENERAL_PACKET_SIZE:
					packet = *(int*)savedPacket;
					if (is_extend_packet_client(packet)) {
						need_size = EXTEND_PACKET_SIZE;
						saved_size = GENERAL_PACKET_SIZE;
						continue;
					}
					need_size = GENERAL_PACKET_SIZE;
					tempQueue.emplace(packet);
					break;

				case EXTEND_PACKET_SIZE:
					packet = *(int*)savedPacket;
					addData = *(Vector2d*)(savedPacket + GENERAL_PACKET_SIZE);
					tempQueue.emplace(packet);
					tempAddQueue.emplace(addData);
					need_size = GENERAL_PACKET_SIZE;
					break;
				}
				saved_size = 0;
			}
			else {
				memcpy(savedPacket + saved_size, buf_pos, retval);
				saved_size += retval;
				retval = 0;
			}
		}

		RecvLock.lock();
		while (tempQueue.empty() != true) {
			RecvQueue.emplace(tempQueue.front());
			tempQueue.pop();
		}
		while (tempAddQueue.empty() != true) {
			RecvAddData.emplace(tempAddQueue.front());
			tempAddQueue.pop();
		}
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
					if (g_ScnMgr->m_players[mover].m_visible == false) {
						g_ScnMgr->m_players[mover].m_dst = Data;
						g_ScnMgr->m_players[mover].m_pos = Data;
					}
					else
						g_ScnMgr->m_players[mover].m_dst = Data;
					g_ScnMgr->m_players[mover].m_visible = true;
				}
					break;

				case obj_bullet: {
					if (mover != player4) {
						int idx = get_packet_bullet_idx(packet);
						int type = get_packet_bullet_type(packet);
						Vector2d Data = addData.front(); addData.pop();
						if (g_ScnMgr->m_players[mover].bullets[idx].m_visible == false)
							g_ScnMgr->m_players[mover].bullets[idx].m_pos = g_ScnMgr->m_players[mover].m_pos;
						g_ScnMgr->m_players[mover].bullets[idx].m_visible = true;
						g_ScnMgr->m_players[mover].bullets[idx].type = type;
						g_ScnMgr->m_players[mover].bullets[idx].m_dst = Data;
					}
					else {
						int idx = get_packet_bullet_idx(packet);
						Vector2d Data = addData.front(); addData.pop();
						if (g_ScnMgr->m_commonBullet[idx]->m_visible == false)
							g_ScnMgr->m_commonBullet[idx]->m_pos = Data;
						g_ScnMgr->m_commonBullet[idx]->m_visible = true;
						g_ScnMgr->m_commonBullet[idx]->m_dst = Data;
					}
				}
					break;

				case obj_item: {
					int idx = get_packet_bullet_idx(packet);
					int type = get_packet_bullet_type(packet);
					Vector2d Data = addData.front(); addData.pop();
					g_ScnMgr->m_item[idx].m_visible = true;
					g_ScnMgr->m_item[idx].type = type;
					g_ScnMgr->m_item[idx].m_pos = Data;
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
				g_ScnMgr->alive_count--;
				break;

			case obj_bullet: {
				int idx = get_packet_bullet_idx(packet);
				if (mover != player4)
					g_ScnMgr->m_players[mover].bullets[idx].m_visible = false;
				else
					g_ScnMgr->m_commonBullet[idx]->m_visible = false;
				break;
			}

			case obj_item: {
				int idx = get_packet_bullet_idx(packet);
				g_ScnMgr->m_item[idx].m_visible = false;
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
	case event_chat: {
		int chat = get_packet_chat(packet);
		g_ScnMgr->m_players[client].m_chat = chat;
		g_ScnMgr->m_players[client].emotion_time = g_ScnMgr->m_players[client].emotion_duration_time;
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
		closesocket(g_ScnMgr->soc);
		g_ScnMgr->winner = client;
		g_ScnMgr->m_state = ScnMgr::state_end;
		break;
	}
	case system_start:
		g_ScnMgr->m_state = ScnMgr::state_play;
		g_ScnMgr->Init();
		break;
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
	switch (g_ScnMgr->m_state)
	{
	case ScnMgr::state_title:
	case ScnMgr::state_connect_error:
	{
		g_ScnMgr->m_state = ScnMgr::state_connect;
		g_ScnMgr->RenderScene();
		glutSwapBuffers();
		SOCKADDR_IN serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(SERVERPORT);
		serverAddr.sin_addr.s_addr = inet_addr(SERVERIP.data());
		g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		int retval = ::connect(g_socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (retval == SOCKET_ERROR) {
			closesocket(g_socket);
			cout << "연결에러" << endl;
			g_ScnMgr->m_state = ScnMgr::state_connect_error;
		}
		else
		{
			g_ScnMgr->m_state = ScnMgr::state_wait;
			cout << "성공" << endl;
			int myid;
			recv(g_socket, (char*)&myid, sizeof(int), 0);
			g_ScnMgr->MYID = myid;
			g_ScnMgr->soc = g_socket;
			thread recvThread{ PacketReceiver, g_socket };
			recvThread.detach();
		}
		break;
	}
	case ScnMgr::state_play:
		g_ScnMgr->KeyDownInput(key, x, y);
		break;

	case ScnMgr::state_end: {
		g_ScnMgr->m_state = ScnMgr::state_title;
		break;
	}
	}
}
void KeyUpInput(unsigned char key, int x, int y)
{
	g_ScnMgr->KeyUpInput(key, x, y);
}

int main(int argc, char **argv)
{
	// Initialize Socket
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	ifstream in("SERVERIP.txt");
	in >> SERVERIP;
	in.close();

	// Initialize GL things
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	const char *windowTitle = "Game Software Engineering KPU";
	glutCreateWindow("Game Software Engineering KPU");
	HWND windowHandle = FindWindow(NULL, windowTitle);
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
	g_ScnMgr = new ScnMgr();

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

	if(g_ScnMgr)
		delete g_ScnMgr;
	g_ScnMgr = NULL;

	closesocket(g_socket);
	WSACleanup();
    return 0;
}