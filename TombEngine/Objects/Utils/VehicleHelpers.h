#pragma once
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

struct CollisionInfo;
struct CollisionResult;
struct ItemInfo;

using std::vector;

//////////
//////////
// TODO: SAVEGAMES!
//////////
//////////
//////////

namespace TEN::Entities::Vehicles
{
	constexpr int VEHICLE_VELOCITY_SCALE = 256; // TODO: Deal with this nonsense *immediately* post-beta. @Sezz 2022.06.25

	constexpr auto VEHICLE_SINK_VELOCITY		 = 15;
	constexpr auto VEHICLE_WATER_HEIGHT_MAX		 = CLICK(2.5f);
	constexpr auto VEHICLE_WATER_VELOCITY_COEFF  = 16.0f;
	constexpr auto VEHICLE_WATER_TURN_RATE_COEFF = 10.0f;
	constexpr auto VEHICLE_SWAMP_VELOCITY_COEFF  = 8.0f;
	constexpr auto VEHICLE_SWAMP_TURN_RATE_COEFF = 6.0f;

	enum class VehicleMountType
	{
		None,
		LevelStart,
		Front,
		Back,
		Left,
		Right,
		Jump
	};

	enum class VehicleDismountType
	{
		None,
		Front,
		Back,
		Left,
		Right,
		Jump,
		Fall,
		Death
	};

	enum class VehicleImpactDirection
	{
		None,
		Front,
		Back,
		Left,
		Right
	};

	// Collision struct specific to vehicles. Used to ascertain point/room collision parameters
	// at wheels and around the base perimeter. Essentially, a pseudo-raycast. May revise later.
	struct VehicleCollisionPointInfo
	{
		Vector3Int Position;
		int Floor;
		int Ceiling;
	};

	int GetVehicleHeight(ItemInfo* vehicleItem, int forward, int right, bool clamp, Vector3Int* pos);

	VehicleMountType GetVehicleMountType(ItemInfo* vehicleItem, ItemInfo* laraItem, CollisionInfo* coll, vector<VehicleMountType> allowedMountTypes, float maxDistance2D, float maxVerticalDistance = STEPUP_HEIGHT);
	VehicleDismountType GetVehicleDismountType(ItemInfo* vehicleItem, vector<VehicleDismountType> allowedDismountTypes, float distance, bool onLand = true);
	bool TestVehicleDismount(ItemInfo* vehicleItem, VehicleDismountType dismountType, short angle, float distance, bool onLand);
	VehicleImpactDirection GetVehicleImpactDirection(ItemInfo* vehicleItem, Vector3Int prevPos);
	VehicleCollisionPointInfo GetVehicleCollision(ItemInfo* vehicleItem, int forward, int right, bool clamp);
	int GetVehicleWaterHeight(ItemInfo* vehicleItem, int forward, int right, bool clamp, Vector3Int* pos);

	void  DoVehicleCollision(ItemInfo* vehicleItem, int radius);
	int	  DoVehicleDynamics(int height, int verticalVelocity, int minBounce, int maxKick, int* yPos, float weightMult = 1.0f);
	short DoVehicleShift(ItemInfo* vehicleItem, Vector3Int pos, Vector3Int oldPos);
	int   DoVehicleWaterMovement(ItemInfo* vehicleItem, ItemInfo* laraItem, int currentVelocity, int radius, short* turnRate);
	void  DoVehicleFlareDiscard(ItemInfo* laraItem);

	short ModulateVehicleTurnRate(short turnRate, short accelRate, short minTurnRate, short maxTurnRate, float axisCoeff, bool invert);
	void  ModulateVehicleTurnRateX(short* turnRate, short accelRate, short minTurnRate, short maxTurnRate, bool invert = true);
	void  ModulateVehicleTurnRateY(short* turnRate, short accelRate, short minTurnRate, short maxTurnRate, bool invert = false);
	void  ModulateVehicleLean(ItemInfo* vehicleItem, short baseRate, short maxAngle);
	void  ResetVehicleLean(ItemInfo* vehicleItem, float rate);
}
