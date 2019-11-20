#pragma comment(lib, "ws2_32")
#include <iostream>
#include "Vector2f.h"
#include "Object.h"
#include "PacketMgr.h"
#include "SceneMgr.h"
#include "NW_Project.h"

#include <WinSock2.h>
#include <thread>
#include <map>
#include <queue>
#include <mutex>
#include <atomic>
#include <vector>
using namespace std;


#define SERVERPORT 9000
#define MAX_PLAYER 2
#define MAX_BUFFER_SIZE 200

#define CLIENT_EXIT 0

int g_curUser;

map<int, SOCKET> SockList;
map<int, obj> PlayerList;
map<int, obj> BulletList;
map<int, obj> ItemList;
vector<thread> threads;

mutex RecvLock;

queue<int> RecvQueue;
queue<Vector2d> RecvAddData;

queue<int> SendQueue;
queue<Vector2d> SendAddData;

packetMgr pMgr;

void PacketReceiver(SOCKET sock, const int playerNum)
{
	int retval;
	int packet = 0;
	char buffer[MAX_BUFFER_SIZE];
	char savedPacket[MAX_BUFFER_SIZE];

	unsigned char *data = (unsigned char*)buffer;
	while (true) {
		retval = recv(sock, buffer, sizeof(int), 0);

		if (retval == CLIENT_EXIT || retval == SOCKET_ERROR)
		{
			SockList.erase(playerNum);
			closesocket(sock);
			cout << "PLAYER EXIT : " << sock << endl;
			return;
		}

		memcpy(&packet, buffer, sizeof(int));

		if (pMgr.isNeedPos(packet))
		{
			retval = recv(sock, buffer, sizeof(Vector2d), 0);
			Vector2d temp;
			memcpy(&temp, buffer, sizeof(Vector2d));
			RecvLock.lock();
			RecvQueue.emplace(packet);
			RecvAddData.emplace(temp);
			RecvLock.unlock();
		}
		else {
			RecvLock.lock();
			RecvQueue.emplace(packet);
			RecvLock.unlock();
		}
	}
}
void ProcessingPacket() {
	int packet{ 0 };
	Vector2d pos{ 0,0 };
	while (1) {
		if (RecvQueue.empty())
			continue;

		RecvLock.lock();
		packet = RecvQueue.front();
		RecvQueue.pop();
		if (pMgr.isNeedPos(packet)) {
			pos = RecvAddData.front();
			RecvAddData.pop();
		}
		RecvLock.unlock();

		pMgr.interpret(packet, pos);
	}
}


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

	retval = ::bind(listen_socket, reinterpret_cast<SOCKADDR *>(&serverAddr), sizeof(SOCKADDR_IN));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_socket, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	SOCKET client_sock;
	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(SOCKADDR_IN);
	//유저접속 대기
	while (g_curUser < MAX_PLAYER) {
		client_sock = accept(listen_socket, reinterpret_cast<SOCKADDR *>(&clientAddr), &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		cout << "PLAYER ACCPET : " << client_sock << endl;
		++g_curUser;

		int playerNum = initPlayerNum(g_curUser);
		SockList.emplace(pair<int, SOCKET>(playerNum, client_sock));
		PlayerList.emplace(pair<int, obj>(playerNum, obj{}));

		//retval = send(client_sock, (char*)&menu, sizeof(int), 0);

		if (retval == SOCKET_ERROR)
			err_quit("Accept()_PlayerNum 할당.");

		threads.emplace_back(PacketReceiver, client_sock, playerNum);
	}

	cout << "All User Connected, Start Game Logic\n";
	//게임 로직
	//GameLogic();

	for (thread& t : threads)
		t.join();

	//게임 종료처리
	closesocket(listen_socket);
	WSACleanup();
}

// 소켓 오류 출력 함수
void err_quit(const char *msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
void err_display(const char *msg) {
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
int initPlayerNum(const int &curNum) {
	if(curNum == 1)
		return player1;
	else if(curNum == 2)
		return player2;
	else if(curNum == 3)
		return player3;
	else if(curNum == 4)
		return player4;

}