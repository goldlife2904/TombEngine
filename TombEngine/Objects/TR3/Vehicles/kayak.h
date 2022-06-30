#pragma once
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	void InitialiseKayak(short itemNumber);

	void KayakPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool KayakControl(ItemInfo* laraItem);
	void KayakUserInput(ItemInfo* kayakItem, ItemInfo* laraItem);

	void KayakToBackground(ItemInfo* kayakItem, ItemInfo* laraItem);
	void KayakToItemCollision(ItemInfo* kayakItem, ItemInfo* laraItem);

	void KayakDoCurrent(ItemInfo* kayakItem, ItemInfo* laraItem);
	int KayakDoShift(ItemInfo* kayakItem, Vector3Int* pos, Vector3Int* old);

	void DoKayakMount(ItemInfo* kayakItem, ItemInfo* laraItem, VehicleMountType mountType);
	bool KayakCanGetOut(ItemInfo* kayakItem, int dir);
	void KayakLaraRapidsDrown(ItemInfo* laraItem);

	void KayakDraw(ItemInfo* kayakItem);
	void KayakDoWake(ItemInfo* kayakItem, int xOffset, int zOffset, short rotate);
	void KayakDoRipple(ItemInfo* kayakItem, int xOffset, int zOffset);
	void KayakUpdateWakeFX();
}
