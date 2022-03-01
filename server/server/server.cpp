#pragma comment(lib, "ws2_32")
#include <fstream>
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
#define MAX_BUFFER_SIZE 200

#define CLIENT_EXIT 0
#define COMMON_BULLET_NUM 128
#define COMMON_BULLET_DAMAGE 1

int g_curUser;
int MAX_PLAYER = 3;
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
vector<int> SendQueue;

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

//�Ѿ˺м�
Bullet m_commonBullet[COMMON_BULLET_NUM];
int AddBullet(Vector2d pos, Vector2d vol, Vector2d vel, float r, float g, float b, float a, float mass, float fricCoef)
{
	int idx = -1;
	for (int i = 0; i < COMMON_BULLET_NUM; ++i)
	{
		if (m_commonBullet[i].m_visible == false)
		{
			idx = i;
			break;
		}
	}

	if (idx == -1)
	{
		std::cout << "No more remaining object" << std::endl;
		return idx;
	}

	m_commonBullet[idx].m_visible = true;
	m_commonBullet[idx].SetPos(pos);
	m_commonBullet[idx].SetColor(r, g, b, a);
	m_commonBullet[idx].SetVol(vol);
	m_commonBullet[idx].SetVel(vel);
	m_commonBullet[idx].SetMass(mass);
	m_commonBullet[idx].SetFriction(fricCoef);

	return idx;
}

//�Ѿ˺м�
void ShootBullet(float fTimeElapsed)
{
	static float shootTime = 0.05f;
	static float remainTime = 0.0f;
	static float rot = 3.0f;
	static Vector2d m_pos{ 0, 0 };
	static Vector2d m_dir[4]{ {1, 0},{-1, 0}, {0, 1}, {0, -1} };

	remainTime -= fTimeElapsed;
	if (remainTime > 0) return;


	float fAmountBullet = 8.0f, mass = 1.0f, fricCoef = 0.9f;

	for (auto& dir : m_dir)
	{
		dir.rotate(rot);
		Vector2d hVel = dir * fAmountBullet;
		Vector2d vol(0.05f, 0.05f);
		int idx = AddBullet(m_pos, vol, hVel, 1, 0, 0, 1, mass, fricCoef);
		m_commonBullet[idx].type = 1;
	}

	remainTime = shootTime;
}

//Error Handler
void err_quit(const char* msg);
void err_display(const char* msg);

//SendPacket ID, POS, 
void send_packet_player_id(SOCKET client, const int& id)
{
	int packet = 0;
	pTurnOn(packet, id);
	send(client, (const char*)& packet, sizeof(int), 0);
}
void reserve_packet_player_pos(const Player& mover, vector<int>& dat)
{
	int packet = 0;
	pTurnOn(packet, p_obj);
	pTurnOn(packet, mover.m_id);
	pTurnOn(packet, obj_player);
	pTurnOn(packet, obj_position);

	int* p_pos = (int*)& mover.m_pos.x;
	dat.emplace_back(packet);
	dat.emplace_back(*p_pos++);
	dat.emplace_back(*p_pos++);
}
void reserve_packet_bullet_pos(const Player& shooter, const int& idx, vector<int>& dat)
{
	int packet = 0;
	int bullet_type = shooter.bullets[idx].type << 24;

	pTurnOn(packet, p_obj);
	pTurnOn(packet, shooter.m_id);
	pTurnOn(packet, idx);
	pTurnOn(packet, bullet_type);
	pTurnOn(packet, obj_bullet);
	pTurnOn(packet, obj_position);

	int* p_pos = (int *)&shooter.bullets[idx].m_pos.x;
	dat.emplace_back(packet);
	dat.emplace_back(*p_pos++);
	dat.emplace_back(*p_pos++);
}
void reserve_packet_common_bullet_pos(const int& shooter_id, const int& idx, vector<int>& dat)
{
	int packet = 0;

	pTurnOn(packet, p_obj);
	pTurnOn(packet, shooter_id);
	pTurnOn(packet, idx);
	pTurnOn(packet, obj_bullet);
	pTurnOn(packet, obj_position);

	int* p_pos = (int*)& m_commonBullet[idx].m_pos.x;
	dat.emplace_back(packet);
	dat.emplace_back(*p_pos++);
	dat.emplace_back(*p_pos++);
}
void reserve_packet_item_pos(const int& idx, vector<int>& dat)
{
	int packet = 0;
	int item_type = m_item[idx]->type << 24;

	pTurnOn(packet, p_obj);
	pTurnOn(packet, idx);
	pTurnOn(packet, item_type);
	pTurnOn(packet, obj_item);
	pTurnOn(packet, obj_position);

	int* p_pos = (int*)& m_item[idx]->m_pos.x;
	dat.emplace_back(packet);
	dat.emplace_back(*p_pos++);
	dat.emplace_back(*p_pos++);
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
			cout << "PLAYER EXIT : " << socket << endl;
			if (state == server_accept) {
				idlock.lock();
				g_curUser--;
				idQueue.emplace(id);
				idlock.unlock();
			}
			else {
				PlayerList[id].m_isConnect = false;
			}
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
					if (is_extend_packet_server(packet)) {
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
			SendQueue.emplace_back(packet);
			break;
		}
	}
}

//Game Logic
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

	//������ ��ġ ����
	for (auto& p_pair : PlayerList)
		for (int i = 0; i < MAX_ITEM; i++)
			reserve_packet_item_pos(i, SendQueue);

	for (auto& bullet : m_commonBullet)
		bullet.m_visible = false;
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

	for (int i=0;i<100;++i)
	{
		if (m_commonBullet[i].m_visible == false) continue;
		if (m_commonBullet[i].m_vel.length() < 0.00001f) {
			m_commonBullet[i].m_visible = false;
			int packet = make_packet_destroy_bullet(player4, i);
			SendQueue.emplace_back(packet);
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
	//�÷��̾� ������Ʈ
	for (auto& p_player : PlayerList)
	{
		auto& player = p_player.second;
		//Move Player
		player.keyMove(fTimeElapsed);
		//�Ѿ� �߻�
		if (player.m_mouseLeft == true && player.CanShootBullet()) {
			player.ShootBullet(player.m_mousepos);
		}
		player.Update(fTimeElapsed);
	}

	//�Ѿ˺м�
	ShootBullet(fTimeElapsed);
	for (auto& b : m_commonBullet)
		b.Update(fTimeElapsed);

	//Collision Detect
	for (auto& p_pair : PlayerList)
	{
		if (p_pair.second.m_visible == false) continue;
		auto& player = p_pair.second;

		//�÷��̾�, �� üũ
		for (auto& wall : m_wall)
		{
			if (player.isOverlap(*wall)) {
				player.correctpos(*wall);
			}
		}

		//�÷��̾�, ��ֹ� üũ
		for (auto& block : m_block)
		{
			if (player.isOverlap(*block)) {
				player.correctpos(*block);
			}
		}

		//�÷��̾�, ������ üũ
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

		//�Ѿ� �浹üũ
		for (int i = 0; i < MAX_BULLET; ++i)
		{
			if (player.bullets[i].m_visible == false) continue;

			//�Ѿ�, ��üũ
			for (auto& block : m_block)
			{
				if (block->isOverlap(player.bullets[i])) {
					player.bullets[i].m_visible = false;
					int packet = make_packet_destroy_bullet(player.m_id, i);
					SendQueue.emplace_back(packet);
					break;
				}
			}

			//�Ѿ�, �÷��̾� üũ
			for (auto& p_other : PlayerList)
			{
				if (p_other.second.m_visible == false) continue;
				if (p_other.first == p_pair.first) continue;
				if (p_other.second.isOverlap(player.bullets[i])) {
					player.bullets[i].m_visible = false;
					int packet = make_packet_destroy_bullet(player.m_id, i);
					SendQueue.emplace_back(packet);
					if (p_other.second.getDamage(player.bullets[i])) {
						//���ó��
						p_other.second.die();
						int packet = make_packet_destroy_player(p_other.second.m_id);
						SendQueue.emplace_back(packet);
					}
					else {
						//������ó��
						int packet = make_packet_hit_player(p_other.second.m_id, player.m_damage[player.bullets[i].type]);
						SendQueue.emplace_back(packet);
					}
				}
			}
		}

		//�߸��Ѿ� üũ
		for (int i = 0; i < COMMON_BULLET_NUM; i++)
		{
			if (m_commonBullet[i].m_visible == false) continue;
			//�Ѿ�, ��üũ
			for (auto& block : m_block)
			{
				if (block->isOverlap(m_commonBullet[i])) {
					m_commonBullet[i].m_visible = false;
					int packet = make_packet_destroy_bullet(player4, i);
					SendQueue.emplace_back(packet);
					break;
				}
			}

			//�Ѿ�, �÷��̾� üũ
			for (auto& p_other : PlayerList)
			{
				if (p_other.second.m_visible == false) continue;
				if (p_other.second.isOverlap(m_commonBullet[i])) {
					m_commonBullet[i].m_visible = false;
					int packet = make_packet_destroy_bullet(player4, i);
					SendQueue.emplace_back(packet);
					if (p_other.second.getDamage(COMMON_BULLET_DAMAGE)) {
						//���ó��
						p_other.second.die();
						int packet = make_packet_destroy_player(p_other.second.m_id);
						SendQueue.emplace_back(packet);
					}
					else {
						//������ó��
						int packet = make_packet_hit_player(p_other.second.m_id, COMMON_BULLET_DAMAGE);
						SendQueue.emplace_back(packet);
					}
				}
			}

		}

	}

	//���� �˻�
	int live_count = 0;
	int last_liver = 0;
	for (auto& p_player : PlayerList) {
		if (p_player.second.m_visible == true) {
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
void SendGameState() {
	for (auto& p_other_player : PlayerList)
	{
		auto& other_player = p_other_player.second;
		if (other_player.m_visible == false) continue;
		reserve_packet_player_pos(other_player, SendQueue);
		for (int i = 0; i < MAX_BULLET; ++i)
			if (other_player.bullets[i].m_visible)
				reserve_packet_bullet_pos(other_player, i, SendQueue);
	}

	for (int i = 0; i < COMMON_BULLET_NUM; ++i) {
		if (m_commonBullet[i].m_visible)
			reserve_packet_common_bullet_pos(player4, i, SendQueue);
	}

	for (auto& p_player : PlayerList)
	{
		if (p_player.second.m_isConnect == true)
			send(p_player.second.m_socket, (const char*)SendQueue.data(), SendQueue.size() * sizeof(int), 0);
	}

	SendQueue.clear();
}

int main()
{
	float refreshrate = 0;
	ifstream in("option.txt");
	in >> refreshrate;
	in.close();
	refreshtime = 1.0f / refreshrate;

	//TCP ���� ����
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == INVALID_SOCKET) err_quit("socket()");

	//���̱� OFF
	int opt_val = TRUE;
	setsockopt(listen_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt_val, sizeof(opt_val));

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVERPORT);

	int retval = ::bind(listen_socket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_socket, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	SOCKET client_sock;
	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(SOCKADDR_IN);

	while (true) {
		cout << "Start Player Accept\n";

		//��ġ ������ �ʱ�ȭ
		match_round++;
		PlayerList.clear();
		while (!idQueue.empty()) idQueue.pop();
		idQueue.emplace(player1);
		idQueue.emplace(player2);
		idQueue.emplace(player3);

		//�������� ���
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

			//�÷��̾� ����ó��
			send_packet_player_id(client_sock, id);
			PlayerList[id] = Player();
			PlayerList[id].match_round = match_round;
			PlayerList[id].m_id = id;
			PlayerList[id].m_socket = static_cast<int>(client_sock);
			PlayerList[id].m_isConnect = true;
			PlayerList[id].SetPos(0.0f, 0.0f);
			PlayerList[id].SetColor(1.0f, 1.0f, 1.0f, 1.0f);
			PlayerList[id].SetVol(0.3f, 0.3f);
			PlayerList[id].SetVel(0.0f, 0.0f);
			PlayerList[id].SetMass(1.0f);
			PlayerList[id].SetFriction(0.6f);

			if (retval == SOCKET_ERROR)
				err_quit("Accept()_PlayerNum �Ҵ�.");

			threads.emplace_back(PacketReceiver, client_sock, id);
		}

		cout << "All User Connected, Start Match #" << match_round << endl;
		state = server_play;
		SendQueue.emplace_back(make_packet_game_start());

		//���� ����
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

		//���� ����ó��
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

// ���� ���� ��� �Լ�
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