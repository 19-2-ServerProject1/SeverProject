#pragma comment(lib, "ws2_32")
#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <queue>
#include <list>
#include "Timer.h"
using namespace std;

//Scene
#include "Object.h"
#include "Config.h"
#include <map>

//Packet
#include "PacketMgr.h"

#define SERVERPORT 9000
#define MAX_PLAYER 2
#define MAX_BUFFER_SIZE 200

#define CLIENT_EXIT 0

int g_curUser;
queue<int> idQueue;
mutex idlock;
int match_round;

float refreshtime;

//Threads
vector<thread> threads;

//Message Queue
mutex RecvLock;
queue<int> RecvQueue;
queue<Vector2d> RecvAddData;
list<int> SendQueue;
list<Vector2d> SendAddData;

//Scene Value
enum server_state {server_accept, server_play};
int state;
bool isEnd = false;

int m_Width = WINDOW_WIDTH;
int m_Height = WINDOW_HEIGHT;
Object* m_wall[4];
Object* m_block[8];
Item* m_item[MAX_ITEM];
map<int, Player> PlayerList;
Vector2d Initialpos[3]{ {-3, -1.5}, {+3, -1.5},{0, 1.5} };

//Error Handler
void err_quit(const char* msg);
void err_display(const char* msg);

//SendPacket
void send_packet_player_id(SOCKET client, const int& id)
{
	int packet = 0;
	pTurnOn(packet, id);
	send(client, (const char*)& packet, sizeof(int), 0);
}
void send_packet_player_pos(const Player& client, const Player& mover)
{
	int packet = 0;
	pTurnOn(packet, p_obj);
	pTurnOn(packet, mover.m_id);
	pTurnOn(packet, obj_player);
	pTurnOn(packet, obj_position);
	
	send(client.m_socket, (const char*)&packet, sizeof(int), 0);
	send(client.m_socket, (const char*)&mover.m_pos, sizeof(Vector2d), 0);
}
void send_packet_bullet_pos(const Player& client, const Player& shooter, const int& idx)
{
	int packet = 0;
	int bullet_type = shooter.bullets[idx].type << 24;

	pTurnOn(packet, p_obj);
	pTurnOn(packet, shooter.m_id);
	pTurnOn(packet, idx);
	pTurnOn(packet, bullet_type);
	pTurnOn(packet, obj_bullet);
	pTurnOn(packet, obj_position);

	send(client.m_socket, (const char*)& packet, sizeof(int), 0);
	send(client.m_socket, (const char*)& shooter.bullets[idx].m_pos, sizeof(Vector2d), 0);
}
void send_packet_item_pos(const Player& client, const int& idx)
{
	int packet = 0;
	int item_type = m_item[idx]->type << 24;

	pTurnOn(packet, p_obj);
	pTurnOn(packet, idx);
	pTurnOn(packet, item_type);
	pTurnOn(packet, obj_item);
	pTurnOn(packet, obj_position);

	send(client.m_socket, (const char*)& packet, sizeof(int), 0);
	send(client.m_socket, (const char*)& m_item[idx]->m_pos, sizeof(Vector2d), 0);
}
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
void PacketReceiver(SOCKET socket, int id)
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
			if (state == server_accept) {
				if(PlayerList.count(id) != 0)
					if (PlayerList[id].match_round == match_round) {
						idlock.lock();
						g_curUser--;
						idQueue.emplace(id);
						idlock.unlock();
					}
			}
			else {
				PlayerList[id].m_isConnect = false;
			}
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
			ProcessInputPacket(packet, cAddData);
			break;

		case p_obj:
			break;

		case p_event:
			break;
		}
	}
}
void SendGameState() {
	for (auto& p_player : PlayerList) {
		auto& player = p_player.second;

		if (SendQueue.empty() == false)
		{
			for (auto beg = SendQueue.begin(); beg != SendQueue.end(); ++beg)
				send(player.m_socket, (const char*) & *beg, sizeof(int), 0);
		}

		for (auto& p_other_player : PlayerList)
		{
			auto& other_player = p_other_player.second;
			if (other_player.m_visible == false) continue;
			send_packet_player_pos(player, other_player);
			for (int i = 0; i < MAX_BULLET; ++i)
				if (other_player.bullets[i].m_visible)
					send_packet_bullet_pos(player, other_player, i);
		}
	}
	SendQueue.clear();
}

//Scene Process
void Initialize()
{
	int count = 0;
	for (auto& p_pair : PlayerList)
		p_pair.second.m_pos = Initialpos[count++];

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

	//Add Block
	float volumn = 0.5f;
	m_block[0] = new Object();
	m_block[0]->SetPos(m_Width / 400, 0);
	m_block[0]->SetVol(volumn, volumn);
	m_block[1] = new Object();
	m_block[1]->SetPos(-m_Width / 400, 0);
	m_block[1]->SetVol(volumn, volumn);
	m_block[2] = new Object();
	m_block[2]->SetPos(0, m_Height / 400);
	m_block[2]->SetVol(volumn, volumn);
	m_block[3] = new Object();
	m_block[3]->SetPos(0, -m_Height / 400);
	m_block[3]->SetVol(volumn, volumn);

	m_block[4] = new Object();
	m_block[4]->SetPos(m_Width / 400, m_Height / 400 * 2);
	m_block[4]->SetVol(volumn, volumn);
	m_block[5] = new Object();
	m_block[5]->SetPos(-m_Width / 400, m_Height / 400 * 2);
	m_block[5]->SetVol(volumn, volumn);
	m_block[6] = new Object();
	m_block[6]->SetPos(m_Width / 400, -m_Height / 400 * 2);
	m_block[6]->SetVol(volumn, volumn);
	m_block[7] = new Object();
	m_block[7]->SetPos(-m_Width / 400, -m_Height / 400 * 2);
	m_block[7]->SetVol(volumn, volumn);

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

	//아이템 위치 전송
	for (auto& p_pair : PlayerList)
		for (int i = 0; i < 4; i++)
			send_packet_item_pos(p_pair.second, i);
}
void DoGarbageCollection()
{
	//delete bullets
	for (auto& p_pair : PlayerList)
	{
		auto& player = p_pair.second;
		if (player.m_visible == false) continue;

		for (int i = 0; i < MAX_BULLET; ++i)
		{
			if (player.bullets[i].m_visible == false) continue;
			if (player.bullets[i].m_vel.length() < 0.00001f) {
				player.bullets[i].m_visible = false;
				int packet = make_packet_destroy_bullet(player.m_id, i);
				SendQueue.emplace_back(packet);
			}
		}
	}
}
void DeleteObjects() {
	for (int i = 0; i < 4; i++) {
		delete m_wall[i];
		m_wall[i] = NULL;
	}
	for (int i = 0; i < MAX_ITEM; i++) {
		delete m_item[i];
		m_item[i] = NULL;
	}

	for (int i = 0; i < 8; i++) {
		delete m_block[i];
		m_block[i] = NULL;
	}
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

		//플레이어, 장애물 체크
		for (auto& block : m_block)
		{
			if (player.isOverlap(*block)) {
				player.correctpos(*block);
			}
		}

		//플레이어, 아이템 체크
		for (int i = 0; i < MAX_ITEM; ++i)
		{
			if (m_item[i]->m_visible == false) continue;
			if (player.isOverlap(*m_item[i])) {
				player.weapon = m_item[i]->type;
				m_item[i]->m_visible = false;
				int packet = make_packet_destroy_item(i);
				SendQueue.emplace_back(packet);
			}
		}

		//총알 충돌체크
		for (int i = 0; i < MAX_BULLET; ++i)
		{
			if (player.bullets[i].m_visible == false) continue;

			//총알, 벽체크
			for (auto& block : m_block)
			{
				if (block->isOverlap(player.bullets[i])) {
					player.bullets[i].m_visible = false;
					int packet = make_packet_destroy_bullet(player.m_id, i);
					SendQueue.emplace_back(packet);
					break;
				}
			}

			//총알, 플레이어 체크
			for (auto& p_other : PlayerList)
			{
				if (p_other.second.m_visible == false) continue;
				if (p_other.first == p_pair.first) continue;
				if (p_other.second.isOverlap(player.bullets[i])) {
					player.bullets[i].m_visible = false;
					int packet = make_packet_destroy_bullet(player.m_id, i);
					SendQueue.emplace_back(packet);
					if (p_other.second.getDamage(player.bullets[i])) {
						//사망처리
						p_other.second.die();
						int packet = make_packet_destroy_player(p_other.second.m_id);
						SendQueue.emplace_back(packet);
					}
					else {
						//데미지처리
						int packet = make_packet_hit_player(p_other.second.m_id, player.m_damage[player.bullets[i].type]);
						SendQueue.emplace_back(packet);
					}
				}
			}
		}
	}

	//종료 검사
	int live_count = 0;
	int last_liver = 0;
	for (auto& p_player : PlayerList) {
		if (p_player.second.m_isConnect == true) {
			++live_count;
			last_liver = p_player.second.m_id;
		}
	}
	if (live_count <= 1) {
		int packet = make_packet_game_end(last_liver);
		SendQueue.emplace_back(packet);
		isEnd = true;
		return;
	}

	DoGarbageCollection();
}


int main()
{
	int retval;
	match_round = 0;

	//TCP 소켓 세팅
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == INVALID_SOCKET) err_quit("socket()");

	//네이글 OFF, CLOSE 시 데이터 모두 전송 설정
	LINGER opt;
	opt.l_onoff = 1;
	opt.l_linger = 10;
	setsockopt(listen_socket, SOL_SOCKET, SO_LINGER, (const char*)&opt, sizeof(opt));
	int opt_val = TRUE;
	setsockopt(listen_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt_val, sizeof(opt_val));

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

	while (true) {
		cout << "Start Player Accept\n";

		//매치 시작전 초기화
		match_round++;
		PlayerList.clear();
		while (!idQueue.empty()) idQueue.pop();
		idQueue.emplace(player1);
		idQueue.emplace(player2);
		idQueue.emplace(player3);

		//유저접속 대기
		state = server_accept;
		isEnd = false;
		g_curUser = 0;
		while (g_curUser < MAX_PLAYER) {
			client_sock = accept(listen_socket, reinterpret_cast<SOCKADDR*>(&clientAddr), &addrlen);
			if (client_sock == INVALID_SOCKET) {
				err_display("accept()");
				break;
			}

			cout << "PLAYER ACCPET : " << client_sock << endl;

			idlock.lock();
			++g_curUser;
			int id = idQueue.front(); idQueue.pop();
			idlock.unlock();

			//플레이어 접속처리
			send_packet_player_id(client_sock, id);
			PlayerList[id] = Player();
			PlayerList[id].match_round = match_round;
			PlayerList[id].m_id = id;
			PlayerList[id].m_socket = client_sock;
			PlayerList[id].m_isConnect = true;
			PlayerList[id].SetPos(0.0f, 0.0f);
			PlayerList[id].SetColor(1.0f, 1.0f, 1.0f, 1.0f);
			PlayerList[id].SetVol(0.3f, 0.3f);
			PlayerList[id].SetVel(0.0f, 0.0f);
			PlayerList[id].SetMass(1.0f);
			PlayerList[id].SetFriction(0.6f);

			if (retval == SOCKET_ERROR)
				err_quit("Accept()_PlayerNum 할당.");

			threads.emplace_back(PacketReceiver, client_sock, id);
		}

		cout << "All User Connected, Start Match #" << match_round << endl;
		state = server_play;
		SendQueue.emplace_back(make_packet_game_start());

		//게임 로직
		refreshtime = 1.0f / 30.0f;
		float remain_time = refreshtime;
		Initialize();
		Timer timer;
		timer.setFPS(60.0f);
		timer.tick();

		while (true) {
			float fTimeElapsed = timer.tick();
			ProcessPacket();
			Update(fTimeElapsed);
			if ((remain_time -= fTimeElapsed) <= 0 ) {
				SendGameState();
				remain_time += refreshtime;
				if (isEnd == true) break;
			}
		}

		//게임 종료처리
		cout << "Match END | Disconnect All Players\n";
		DeleteObjects();

		for (auto& p_player : PlayerList) {
			closesocket(p_player.second.m_socket);
		}

		for (thread& t : threads)
			if (t.joinable())
				t.join();
		threads.clear();
	}

	for (thread& t : threads)
		if (t.joinable())
			t.join();

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