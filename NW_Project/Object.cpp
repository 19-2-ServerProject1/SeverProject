#include "Object.h"
#include "PacketMgr.h"

void PlayerState::setPlayerNum(const int &playerNum) {
	switch (playerNum)
	{
	case 1:
		m_PlayerNum = player1;
		break;

	case 2:
		m_PlayerNum = player2;
		break;

	case 3:
		m_PlayerNum = player3;
		break;

	case 4:
		m_PlayerNum = player4;
		break;
	}
}