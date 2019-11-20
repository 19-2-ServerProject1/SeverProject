#pragma once

void CreateObject(char obj_type, char obj_id, float x, float y)
{
	switch (obj_type)
	{
	case 0:
		PlayerList[obj_id] = obj{};
		break;
	case 1:
		BulletList[obj_id] = obj{};
		break;
	case 2:
		ItemList[obj_id] = obj{};
		break;
	}
};
void DestroyObject(char obj_type, char obj_id)
{
	switch (obj_type)
	{
	case 0:
		PlayerList.erase(obj_id);
		break;
	case 1:
		BulletList.erase(obj_id);
		break;
	case 2:
		ItemList.erase(obj_id);
		break;
	}
}