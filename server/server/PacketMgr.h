#pragma once

enum packet_type {
	p_system	 = 0x00010000,
	p_input		 = 0x00020000,
	p_obj		 = 0x00040000,
	p_event		 = 0x00080000,
};
enum player_num {//�Ŀ� Color��
	player1 = 0x00100000,
	player2 = 0x00200000,
	player3 = 0x00400000,
	player4 = 0x00800000,
};
enum packet_input {
	input_Wdown		 = 0x00001000,
	input_Wup		 = 0x00002000,
	input_Sdown		 = 0x00000100,
	input_Sup		 = 0x00000200,
	input_Adown		 = 0x00000010,
	input_Aup		 = 0x00000020,
	input_Ddown		 = 0x00000001,
	input_Dup		 = 0x00000002,
};
enum packet_obj_type {
	obj_player		 = 0x00001000,
	obj_bullet		 = 0x00002000,
	obj_item	     = 0x00004000,


	
	//obj_create = 0x000100,
	//obj_id		= 0x0000??
};
enum packet_obj_info {
	obj_position		 = 0x00000100,
	obj_destroy			 = 0x00000200,
};
enum packet_event {
	event_hit = 0x00001000,
	//event_target	= 0x000?00,
	//event_damage	= 0x0000??,
	event_chat = 0x00002000,
	//event_chatsize= 0x0000??,

};

int get_packet_type(const int& packet) {
	return packet & 0x000F0000;
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


bool On(const int& p_iflag, int pt) { return (p_iflag & pt) == pt; }

//temp
void pTurnOn(int& p_iflag, const int pt) { p_iflag |= pt; }
void pTurnOff(int& p_iflag, const int pt) { if (On(p_iflag, pt))   p_iflag ^= pt; }

//temp

//void ProcessPacket() {};
void interpret(const int& packet, const Vector2d& pos) {
	//�÷��̾� �κа�, Ÿ�� �κ� ����.
	int player = packet - packet % 0x100000;

	if (On(packet, p_system)) {
		//PlayerList[player].pSystemUpdate(On(packet, p_system));//--> true false ����.
	}
	else if (On(packet, p_input)) {
		//PlayerList[player].pInputUpdate(packet);//--> ON OFF �ع�����
	}
	else if (On(packet, p_obj)) {
		//game logic func
		//PLAYER ����
		//ITEM ����
		//BUllET ����
	}
	else if (On(packet, p_event)) {
		//EVENT ��������.
	}
}
bool isNeedPos(const int& packet) {
	return On(packet, obj_position);
}
void makePSystem() {
	//server : player num �Ҵ�. ���� �ѹ�. Login Ok
	//client : player num �Ҵ� ������, �װ� ���� Login Ok �Ŀ� ���.
}
void makePInput(const int pt) {

}

//void SendEvent() {};
void objectInterpret(const int& packet) {

}
void eventInterpret(const int& packet) {

}
void MakePacket() {};

