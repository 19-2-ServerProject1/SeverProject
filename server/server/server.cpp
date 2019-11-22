#pragma comment(lib, "ws2_32")
#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <queue>
#include "Timer.h"
using namespace std;

//Scene
#include "Object.h"
#include "Config.h"
#include <map>

//Packet
#include "PacketMgr.h"

#define SERVERPORT 9000
#define MAX_PLAYER 1
#define MAX_BUFFER_SIZE 200

#define CLIENT_EXIT 0

int g_curUser;

//Threads
vector<thread> threads;

//Message Queue
mutex RecvLock;
queue<int> RecvQueue;
queue<Vector2d> RecvAddData;
queue<int> SendQueue;
queue<Vector2d> SendAddData;

//Scene Value
int m_Width = WINDOW_WIDTH;
int m_Height = WINDOW_HEIGHT;
Object* m_background = NULL;
Object* m_wall[4];
Item* m_item[4];
map<int, Player> PlayerList;

//Error Handler
void err_quit(const char* msg);
void err_display(const char* msg);

//Message Process
void ProcessInputPacket(const int& packet, queue<Vector2d>& addData) {
	int sender = get_packet_player_num(packet);
	int input = get_packet_input(packet);
	switch (input)
	{
	case input_Wdown:
		PlayerList[sender].m_keyW = true;
		break;
	case input_Wup:
		PlayerList[sender].m_keyW = false;
		break;

	case input_Sdown:
		PlayerList[sender].m_keyS = true;
		break;
	case input_Sup:
		PlayerList[sender].m_keyS = false;
		break;

	case input_Adown:
		PlayerList[sender].m_keyA = true;
		break;
	case input_Aup:
		PlayerList[sender].m_keyA = false;
		break;

	case input_Ddown:
		PlayerList[sender].m_keyD = true;
		break;
	case input_Dup:
		PlayerList[sender].m_keyD = false;
		break;

	case input_Mleft:
		Vector2d Data = addData.front(); addData.pop();
		PlayerList[sender].ShootBullet(Data);
		break;
	}
}
void PacketReceiver(SOCKET socket)
{
	int retval;
	int packet = 0;
	char buffer[MAX_BUFFER_SIZE];
	bool isAdd = false;
	Vector2d addData;

	unsigned char* data = (unsigned char*)buffer;
	while (true) {
		isAdd = false;
		retval = recv(socket, buffer, sizeof(int), 0);
		if (retval == CLIENT_EXIT || retval == SOCKET_ERROR)
		{
			closesocket(socket);
			cout << "PLAYER EXIT : " << socket << endl;
			return;
		}
		memcpy(&packet, buffer, sizeof(int));

		int packet_type = get_packet_type(packet);
		switch (packet_type)
		{
		case p_input:
			int packet_input = get_packet_input(packet);
			if (packet_input == input_Mleft) {
				isAdd = true;
				recv(socket, (char*)& addData, sizeof(addData), 0);
			}
			break;
		};

		RecvLock.lock();
		RecvQueue.emplace(packet);
		if (isAdd)
			RecvAddData.emplace(addData);
		RecvLock.unlock();
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
			break;

		case p_input:
			ProcessInputPacket(packet);
			break;

		case p_obj:
			break;

		case p_event:
			break;
		}
	}
}
void SendGameState() {
	for (auto& player : PlayerList) {
		int packet = 0;
		pTurnOn(packet, p_obj);
		pTurnOn(packet, obj_player);
		pTurnOn(packet, obj_position);
		packet |= player.first;
		send(player.second.m_socket, (const char*)& packet, sizeof(int), 0);
		send(player.second.m_socket, (const char*)& player.second.m_pos, sizeof(Vector2d), 0);
	}
} 

//SendPacket
void send_packet_player_id(SOCKET client, const int& playernum)
{
	int packet = 0;
	switch (playernum) {
	case 1:
		pTurnOn(packet, player1);
		break;
	case 2:
		pTurnOn(packet, player2);
		break;
	case 3:
		pTurnOn(packet, player3);
		break;
	}
	send(client, (const char*)& packet, sizeof(int), 0);
}

//Scene Process
void Initialize();
void Update(float fTimeElapsed);
void DoGarbageCollection();

int main()
{


	int retval;
	//TCP 소켓 세팅
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == INVALID_SOCKET) err_quit("socket()");

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVERPORT);

	retval = ::bind(listen_socket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_socket, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	SOCKET client_sock;
	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(SOCKADDR_IN);

	//유저접속 대기
	while (g_curUser < MAX_PLAYER) {
		client_sock = accept(listen_socket, reinterpret_cast<SOCKADDR*>(&clientAddr), &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		cout << "PLAYER ACCPET : " << client_sock << endl;
		++g_curUser;

		//플레이어 접속처리
		send_packet_player_id(client_sock, g_curUser);
		int playernum = get_player_num(g_curUser);
		PlayerList[playernum] = Player();
		PlayerList[playernum].m_socket = client_sock;
		PlayerList[playernum].SetPos(0.0f, 0.0f);
		PlayerList[playernum].SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		PlayerList[playernum].SetVol(0.3f, 0.3f);
		PlayerList[playernum].SetVel(0.0f, 0.0f);
		PlayerList[playernum].SetMass(1.0f);
		PlayerList[playernum].SetFriction(0.6f);

		if (retval == SOCKET_ERROR)
			err_quit("Accept()_PlayerNum 할당.");

		threads.emplace_back(PacketReceiver, client_sock);
	}

	cout << "All User Connected, Start Game Logic\n";
	//게임 로직
	Initialize();
	Timer timer;
	timer.setFPS(60.0f);
	timer.tick();
	while (true) {
		float fTimeElapsed = timer.tick();
		ProcessPacket();
		Update(fTimeElapsed);
		SendGameState();
	}

	for (thread& t : threads)
		t.join();

	//게임 종료처리
	closesocket(listen_socket);
	WSACleanup();
}

// 소켓 오류 출력 함수
void err_quit(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)& lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
void err_display(const char* msg) {
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)& lpMsgBuf, 0, NULL);

	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

//Scene Process
void Initialize()
{
	//Add Background
	m_background = new Object();
	m_background->SetPos(0, 0);
	m_background->SetVol(m_Width / 100, m_Height / 100);

	//Add Unvisible Wall
	m_wall[0] = new Object();
	m_wall[0]->SetPos((m_Width + m_Height) / 200, 0);
	m_wall[0]->SetVol(m_Height / 100, m_Height / 100);
	m_wall[1] = new Object();
	m_wall[1]->SetPos(-(m_Width + m_Height) / 200, 0);
	m_wall[1]->SetVol(m_Height / 100, m_Height / 100);
	m_wall[2] = new Object();
	m_wall[2]->SetPos(0, (m_Width + m_Height) / 200);
	m_wall[2]->SetVol(m_Width / 100, m_Width / 100);
	m_wall[3] = new Object();
	m_wall[3]->SetPos(0, -(m_Width + m_Height) / 200);
	m_wall[3]->SetVol(m_Width / 100, m_Width / 100);

	//Add Item
	m_item[0] = new Item();
	m_item[0]->SetPos(m_Width / 400, m_Height / 400);
	m_item[0]->SetVol(0.2f, 0.2f);
	m_item[0]->m_visible = true;
	m_item[1] = new Item();
	m_item[1]->SetPos(m_Width / 400, -m_Height / 400);
	m_item[1]->SetVol(0.2f, 0.2f);
	m_item[1]->m_visible = true;
	m_item[2] = new Item();
	m_item[2]->SetPos(-m_Width / 400, -m_Height / 400);
	m_item[2]->SetVol(0.2f, 0.2f);
	m_item[2]->m_visible = true;
	m_item[3] = new Item();
	m_item[3]->SetPos(-m_Width / 400, m_Height / 400);
	m_item[3]->SetVol(0.2f, 0.2f);
	m_item[3]->m_visible = true;
}
void Update(float fTimeElapsed)
{
	//플레이어 업데이트
	for (auto& p_player : PlayerList)
	{
		auto& player = p_player.second;
		//Move Player
		player.keyMove(fTimeElapsed);
		//총알 발사
		if (player.m_mouseLeft == true && player.CanShootBullet()) {
			player.ShootBullet(player.m_mousepos);
		}
		player.Update(fTimeElapsed);
	}

	//Collision Detect
	for (auto& p_pair : PlayerList)
	{
		if (p_pair.second.m_visible == false) continue;
		auto& player = p_pair.second;

		//플레이어, 벽 체크
		for (auto& wall : m_wall)
		{
			if (player.isOverlap(*wall)) {
				player.correctpos(*wall);
			}
		}

		//플레이어, 아이템 체크
		for (auto& item : m_item)
		{
			if (item->m_visible == false) continue;
			if (player.isOverlap(*item)) {
				player.weapon = item->type;
				item->m_visible = false;
			}
		}

		//총알, 플레이어 체크
		for (auto& bullet : player.bullets)
		{
			if (bullet.m_visible == false) continue;
			for (auto& p_other : PlayerList)
			{
				if (p_other.second.m_visible == false) continue;
				if (p_other.first == p_pair.first) continue;
				if (p_other.second.isOverlap(bullet)) {
					bullet.m_visible = false;
					//데미지처리 & 체력이 0이하면
					if (p_other.second.getDamage(bullet))
						p_other.second.die();
				}
			}
		}
	}

	DoGarbageCollection();
}
void DoGarbageCollection()
{
	//delete bullets
	for (auto& p_pair : PlayerList)
	{
		auto& player = p_pair.second;
		if (player.m_visible == false) continue;

		for (auto& b : player.bullets)
		{
			if (b.m_visible == false) continue;
			if (b.m_vel.length() < 0.00001f)
				b.m_visible = false;
		}
	}
}