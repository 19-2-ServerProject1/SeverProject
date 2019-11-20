#pragma once
#include <WinSock2.h>
#include "Vector2f.h"

class obj {
protected:
	Vector2d m_pos;
	Vector2d m_vel;

public:
	obj() { m_pos.init(); m_vel.init(); };
	~obj() {};

	void virtual update(float fElapsedTime) {
		m_pos += m_vel * fElapsedTime;
	};
};

class obj_item :obj {
private:

public:
	obj_item() : obj() { };
	~obj_item() {};

	void update(float fElapsedTime) {
		
	};
};

class obj_player :obj {
private:

public:
	obj_player() : obj() { };
	~obj_player() {};

	void update(float fElapsedTime) {
		
	};
};