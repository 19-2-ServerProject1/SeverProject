#pragma once
#include "PacketMgr.h"

bool On(const int& p_iflag, int pt) { return (p_iflag & pt) == pt; }
void pTurnOn(int& p_iflag, const int pt) { p_iflag |= pt; }
void pTurnOff(int& p_iflag, const int pt) { if (On(p_iflag, pt))   p_iflag ^= pt; }

int make_packet_input(const int& id, int key) {
	int packet = 0;
	pTurnOn(packet, p_input);
	pTurnOn(packet, id);
	pTurnOn(packet, key);
	return packet;
}
int get_packet_type(const int& packet) {
	return packet & 0x000F0000;
}
int get_packet_obj_info(const int& packet) {
	return packet & 0x00000F00;
}
int get_packet_obj_type(const int& packet) {
	return packet & 0x0000F000;
}
int get_packet_player_num(const int& packet) {
	return packet & 0x00F00000;
}
int get_packet_input(const int& packet) {
	return packet & 0x0000FFFF;
}

int get_player_num(const int& num)
{
	int player = 0;
	switch (num) {
	case 1:
		pTurnOn(player, player1);
		break;
	case 2:
		pTurnOn(player, player2);
		break;
	case 3:
		pTurnOn(player, player3);
		break;
	}

	return player;
}