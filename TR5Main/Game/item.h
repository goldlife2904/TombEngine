#pragma once
#include <cstdint>
#include "phd_global.h"
#include <string>
enum GAME_OBJECT_ID : short;

struct ROOM_VECTOR {
	int roomNumber;
	int yNumber;
};
struct ITEM_INFO {
	int floor;
	uint32_t touchBits;
	uint32_t meshBits;
	GAME_OBJECT_ID objectNumber;
	short currentAnimState;
	short goalAnimState;
	short requiredAnimState;
	short animNumber;
	short frameNumber;
	short roomNumber;
	ROOM_VECTOR location;
	short nextItem;
	short nextActive;
	short speed;
	short fallspeed;
	short hitPoints;
	int boxNumber;
	short timer;
	uint16_t flags; // ItemFlags enum
	short shade;
	uint16_t triggerFlags;
	short carriedItem;
	short afterDeath;
	short firedWeapon;
	short itemFlags[8];
	void* data;
	PHD_3DPOS pos;
	bool active;
	short status; // ItemStatus enum
	bool gravityStatus;
	bool hitStatus;
	bool collidable;
	bool lookedAt;
	bool dynamicLight;
	bool poisoned;
	uint8_t aiBits; // AIObjectType enum
	bool reallyActive;
	bool inDrawRoom;
	bool friendly;
	uint32_t swapMeshFlags;
	short drawRoom;
	short TOSSPAD;
	PHD_3DPOS startPos;
	short locationAI;
	std::string luaName;
};