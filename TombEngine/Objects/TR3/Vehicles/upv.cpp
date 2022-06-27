#include "framework.h"
#include "Objects/TR3/Vehicles/upv.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/los.h"
#include "Game/effects/bubble.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/savegame.h"
#include "Objects/TR3/Vehicles/upv_info.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Specific/setup.h"

using std::vector;
using namespace TEN::Input;

namespace TEN::Entities::Vehicles
{
	BITE_INFO UPVBites[6] =
	{
		{ 0, 0, 0, 3 },
		{ 0, 96, 256, 0 },
		{ -128, 0, -64, 1 },
		{ 0, 0, -64, 1 },
		{ 128, 0, -64, 2 },
		{ 0, 0, -64, 2 }
	};
	const vector<VehicleMountType> UPVMountTypes =
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Back
	};

	constexpr auto UPV_RADIUS = 300;
	constexpr auto UPV_HEIGHT = 400;
	constexpr auto UPV_LENGTH = SECTOR(1);
	constexpr auto UPV_MOUNT_DISTANCE = CLICK(1.5f);
	constexpr auto UPV_DISMOUNT_DISTANCE = SECTOR(1);
	constexpr auto UPV_WATER_SURFACE_DISTANCE = 210;

	constexpr int UPV_VELOCITY_ACCEL = 4 * VEHICLE_VELOCITY_SCALE;
	constexpr int UPV_VELOCITY_FRICTION_DECEL = 1.5f * VEHICLE_VELOCITY_SCALE;
	constexpr int UPV_VELOCITY_MAX = 64 * VEHICLE_VELOCITY_SCALE;

	constexpr int UPV_HARPOON_RELOAD_TIME = 15;
	constexpr int UPV_HARPOON_VELOCITY = CLICK(1);
	constexpr int UPV_TURBINE_JOINT = 3;
	constexpr int UPV_SHIFT = 128;

	// TODO: These should probably be done in the wad. @Sezz 2022.06.24
	constexpr auto UPV_DEATH_FRAME_1 = 16;
	constexpr auto UPV_DEATH_FRAME_2 = 17;
	constexpr auto UPV_MOUNT_WATER_SURFACE_SOUND_FRAME = 30;
	constexpr auto UPV_MOUNT_WATER_SURFACE_CONTROL_FRAME = 50;
	constexpr auto UPV_DISMOUNT_WATER_SURFACE_FRAME = 51;
	constexpr auto UPV_MOUNT_UNDERWATER_SOUND_FRAME = 30;
	constexpr auto UPV_MOUNT_UNDERWATER_CONTROL_FRAME = 42;
	constexpr auto UPV_DISMOUNT_UNDERWATER_FRAME = 42;

	#define UPV_X_TURN_RATE_DIVE_ACCEL	   ANGLE(5.0f)
	#define UPV_X_TURN_RATE_ACCEL		   ANGLE(0.6f)
	#define UPV_X_TURN_RATE_FRICTION_DECEL ANGLE(0.3f)
	#define UPV_X_TURN_RATE_MAX			   ANGLE(3.25f)

	#define UPV_X_ORIENT_DIVE_MAX	   ANGLE(15.0f)
	#define UPV_X_ORIENT_MAX		   ANGLE(85.0f)
	#define UPV_X_ORIENT_WATER_SURFACE ANGLE(30.0f)

	#define UPV_Y_TURN_RATE_ACCEL		   ANGLE(0.6f)
	#define UPV_Y_TURN_RATE_FRICTION_DECEL ANGLE(0.3f)
	#define UPV_Y_TURN_RATE_MAX			   ANGLE(3.75f)

	#define UPV_DEFLECT_ANGLE		 ANGLE(45.0f)
	#define UPV_DEFLCT_TURN_RATE_MAX ANGLE(2.0f)

	enum UPVState
	{
		UPV_STATE_DEATH = 0,
		UPV_STATE_HIT = 1,
		UPV_STATE_DISMOUNT_WATER_SURFACE = 2,
		UPV_STATE_UNK1 = 3,
		UPV_STATE_MOVE = 4,
		UPV_STATE_IDLE = 5,
		UPV_STATE_UNK2 = 6, // TODO
		UPV_STATE_UNK3 = 7, // TODO
		UPV_STATE_MOUNT = 8,
		UPV_STATE_DISMOUNT_UNDERWATER = 9
	};

	// TODO
	enum UPVAnim
	{
		UPV_ANIM_DEATH_MOVING = 0,
		UPV_ANIM_DEATH = 1,

		UPV_ANIM_IDLE = 5,

		UPV_ANIM_DISMOUNT_SURFACE = 9,
		UPV_ANIM_MOUNT_SURFACE_START = 10,
		UPV_ANIM_MOUNT_SURFACE_END = 11,
		UPV_ANIM_DISMOUNT_UNDERWATER = 12,
		UPV_ANIM_MOUNT_UNDERWATER = 13,
	};

	enum UPVFlags
	{
		UPV_FLAG_CONTROL = (1 << 0),
		UPV_FLAG_SURFACE = (1 << 1),
		UPV_FLAG_DIVE = (1 << 2),
		UPV_FLAG_DEAD = (1 << 3)
	};

	enum UPVBiteFlags
	{
		UPV_FAN = 0,
		UPV_FRONT_LIGHT = 1,
		UPV_LEFT_FIN_LEFT = 2,
		UPV_LEFT_FIN_RIGHT = 3,
		UPV_RIGHT_FIN_RIGHT = 4,
		UPV_RIGHT_FIN_LEFT = 5
	};

	UPVInfo* GetUPVInfo(ItemInfo* UPVItem)
	{
		return (UPVInfo*)UPVItem->Data;
	}

	void UPVInitialise(short itemNumber)
	{
		auto* UPVItem = &g_Level.Items[itemNumber];
		UPVItem->Data = UPVInfo();
		auto* UPV = GetUPVInfo(UPVItem);

		UPV->Flags = UPV_FLAG_SURFACE;
	}

	static void FireUPVHarpoon(ItemInfo* laraItem, ItemInfo* UPVItem)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto UPV = GetUPVInfo(UPVItem);

		auto& ammo = GetAmmo(laraItem, LaraWeaponType::HarpoonGun);
		if (ammo.getCount() == 0 && !ammo.hasInfinite())
			return;
		else if (!ammo.hasInfinite())
			ammo--;

		short itemNumber = CreateItem();

		if (itemNumber != NO_ITEM)
		{
			auto* harpoonItem = &g_Level.Items[itemNumber];
			harpoonItem->ObjectNumber = ID_HARPOON;
			harpoonItem->Shade = 0xC210;
			harpoonItem->RoomNumber = UPVItem->RoomNumber;

			auto pos = Vector3Int((UPV->HarpoonLeft ? 22 : -22), 24, 230);
			GetJointAbsPosition(UPVItem, &pos, UPV_TURBINE_JOINT);

			harpoonItem->Pose.Position = pos;
			InitialiseItem(itemNumber);

			harpoonItem->Pose.Orientation = Vector3Shrt(UPVItem->Pose.Orientation.x, UPVItem->Pose.Orientation.y, 0);

			// TODO: Huh?
			harpoonItem->Animation.VerticalVelocity = -UPV_HARPOON_VELOCITY * phd_sin(harpoonItem->Pose.Orientation.x);
			harpoonItem->Animation.Velocity = UPV_HARPOON_VELOCITY * phd_cos(harpoonItem->Pose.Orientation.x);
			harpoonItem->HitPoints = HARPOON_TIME;
			harpoonItem->ItemFlags[0] = 1;

			AddActiveItem(itemNumber);

			SoundEffect(SFX_TR4_HARPOON_FIRE_UNDERWATER, &laraItem->Pose, SoundEnvironment::Always);

			Statistics.Game.AmmoUsed++;
			UPV->HarpoonLeft = !UPV->HarpoonLeft;
		}
	}

	static void TriggerUPVMist(long x, long y, long z, long velocity, short angle)
	{
		auto* sptr = GetFreeParticle();

		sptr->on = 1;
		sptr->sR = 0;
		sptr->sG = 0;
		sptr->sB = 0;

		sptr->dR = 64;
		sptr->dG = 64;
		sptr->dB = 64;

		sptr->colFadeSpeed = 4 + (GetRandomControl() & 3);
		sptr->fadeToBlack = 12;
		sptr->sLife = sptr->life = (GetRandomControl() & 3) + 20;
		sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		sptr->extras = 0;
		sptr->dynamic = -1;

		sptr->x = x + ((GetRandomControl() & 15) - 8);
		sptr->y = y + ((GetRandomControl() & 15) - 8);
		sptr->z = z + ((GetRandomControl() & 15) - 8);
		long zv = velocity * phd_cos(angle) / 4;
		long xv = velocity * phd_sin(angle) / 4;
		sptr->xVel = xv + ((GetRandomControl() & 127) - 64);
		sptr->yVel = 0;
		sptr->zVel = zv + ((GetRandomControl() & 127) - 64);
		sptr->friction = 3;

		if (GetRandomControl() & 1)
		{
			sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
			sptr->rotAng = GetRandomControl() & 4095;

			if (GetRandomControl() & 1)
				sptr->rotAdd = -(GetRandomControl() & 15) - 16;
			else
				sptr->rotAdd = (GetRandomControl() & 15) + 16;
		}
		else
			sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF;

		sptr->scalar = 3;
		sptr->gravity = sptr->maxYvel = 0;
		long size = (GetRandomControl() & 7) + (velocity / 2) + 16;
		sptr->size = sptr->sSize = size / 4;
		sptr->dSize = size;
	}

	void UPVEffects(short itemNumber)
	{
		if (itemNumber == NO_ITEM)
			return;

		auto* laraItem = LaraItem;
		auto* lara = GetLaraInfo(laraItem);
		auto* UPVItem = &g_Level.Items[itemNumber];
		auto* UPV = GetUPVInfo(UPVItem);

		Vector3Int pos;

		if (lara->Vehicle == itemNumber)
		{
			if (!UPV->Velocity)
				UPV->FanRotation += ANGLE(2.0f);
			else
				UPV->FanRotation += UPV->Velocity / 4069;

			if (UPV->Velocity)
			{
				pos = Vector3Int(UPVBites[UPV_FAN].x, UPVBites[UPV_FAN].y, UPVBites[UPV_FAN].z);
				GetJointAbsPosition(UPVItem, &pos, UPVBites[UPV_FAN].meshNum);

				TriggerUPVMist(pos.x, pos.y + UPV_SHIFT, pos.z, abs(UPV->Velocity) / VEHICLE_VELOCITY_SCALE, UPVItem->Pose.Orientation.y + ANGLE(180.0f));

				if ((GetRandomControl() & 1) == 0)
				{
					PHD_3DPOS pos2;
					pos2.Position.x = pos.x + (GetRandomControl() & 63) - 32;
					pos2.Position.y = pos.y + UPV_SHIFT;
					pos2.Position.z = pos.z + (GetRandomControl() & 63) - 32;
					short probedRoomNumber = GetCollision(pos2.Position.x, pos2.Position.y, pos2.Position.z, UPVItem->RoomNumber).RoomNumber;
				
					CreateBubble((Vector3Int*)&pos2, probedRoomNumber, 4, 8, BUBBLE_FLAG_CLUMP, 0, 0, 0);
				}
			}
		}
	
		for (int lp = 0; lp < 2; lp++)
		{
			int random = 31 - (GetRandomControl() & 3);
			pos = Vector3Int(UPVBites[UPV_FRONT_LIGHT].x, UPVBites[UPV_FRONT_LIGHT].y, UPVBites[UPV_FRONT_LIGHT].z << (lp * 6));
			GetJointAbsPosition(UPVItem, &pos, UPVBites[UPV_FRONT_LIGHT].meshNum);

			GameVector source, target;
			if (lp == 1)
			{
				target.x = pos.x;
				target.y = pos.y;
				target.z = pos.z;
				target.roomNumber = UPVItem->RoomNumber;
				LOS(&source, &target);
				pos = Vector3Int(target.x, target.y, target.z);
			}
			else
			{
				source.x = pos.x;
				source.y = pos.y;
				source.z = pos.z;
				source.roomNumber = UPVItem->RoomNumber;
			}

			TriggerDynamicLight(pos.x, pos.y, pos.z, 16 + (lp << 3), random, random, random);
		}

		if (UPV->HarpoonTimer)
			UPV->HarpoonTimer--;
	}

	static bool TestUPVDismount(ItemInfo* laraItem, ItemInfo* UPVItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		if (lara->WaterCurrentPull.x || lara->WaterCurrentPull.z)
			return false;

		short moveAngle = UPVItem->Pose.Orientation.y + ANGLE(180.0f);
		int velocity = UPV_DISMOUNT_DISTANCE * phd_cos(UPVItem->Pose.Orientation.x);
		int x = UPVItem->Pose.Position.x + velocity * phd_sin(moveAngle);
		int z = UPVItem->Pose.Position.z + velocity * phd_cos(moveAngle);
		int y = UPVItem->Pose.Position.y - UPV_DISMOUNT_DISTANCE * phd_sin(-UPVItem->Pose.Orientation.x);

		auto probe = GetCollision(x, y, z, UPVItem->RoomNumber);
		if ((probe.Position.Floor - probe.Position.Ceiling) < CLICK(1) ||
			probe.Position.Floor < y ||
			probe.Position.Ceiling > y ||
			probe.Position.Floor == NO_HEIGHT ||
			probe.Position.Ceiling == NO_HEIGHT)
		{
			return false;
		}

		return true;
	}

	static bool TestUPVMount(ItemInfo* laraItem, ItemInfo* UPVItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		if (!(TrInput & IN_ACTION) ||
			lara->Control.HandStatus != HandStatus::Free ||
			laraItem->Animation.IsAirborne)
		{
			return false;
		}

		int y = abs(laraItem->Pose.Position.y - (UPVItem->Pose.Position.y - CLICK(0.5f)));
		if (y > CLICK(1))
			return false;

		int distance = pow(laraItem->Pose.Position.x - UPVItem->Pose.Position.x, 2) + pow(laraItem->Pose.Position.z - UPVItem->Pose.Position.z, 2);
		if (distance > pow(CLICK(2), 2))
			return false;

		short deltaAngle = abs(laraItem->Pose.Orientation.y - UPVItem->Pose.Orientation.y);
		if (deltaAngle > ANGLE(35.0f) || deltaAngle < -ANGLE(35.0f))
			return false;

		if (GetCollision(UPVItem).Position.Floor < -32000)
			return false;

		return true;
	}

	static void DoCurrent(ItemInfo* laraItem, ItemInfo* UPVItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		Vector3Int target;

		if (!lara->WaterCurrentActive)
		{
			int absVel = abs(lara->WaterCurrentPull.x);
			int shift;
			if (absVel > 16)
				shift = 4;
			else if (absVel > 8)
				shift = 3;
			else
				shift = 2;

			lara->WaterCurrentPull.x -= lara->WaterCurrentPull.x >> shift;

			if (abs(lara->WaterCurrentPull.x) < 4)
				lara->WaterCurrentPull.x = 0;

			absVel = abs(lara->WaterCurrentPull.z);
			if (absVel > 16)
				shift = 4;
			else if (absVel > 8)
				shift = 3;
			else
				shift = 2;

			lara->WaterCurrentPull.z -= lara->WaterCurrentPull.z >> shift;
			if (abs(lara->WaterCurrentPull.z) < 4)
				lara->WaterCurrentPull.z = 0;

			if (lara->WaterCurrentPull.x == 0 && lara->WaterCurrentPull.z == 0)
				return;
		}
		else
		{
			int sinkVal = lara->WaterCurrentActive - 1;
			target.x = g_Level.Sinks[sinkVal].x;
			target.y = g_Level.Sinks[sinkVal].y;
			target.z = g_Level.Sinks[sinkVal].z;
		
			int angle = ((mGetAngle(target.x, target.z, laraItem->Pose.Position.x, laraItem->Pose.Position.z) - ANGLE(90.0f)) / 16) & 4095;

			int dx = target.x - laraItem->Pose.Position.x;
			int dz = target.z - laraItem->Pose.Position.z;

			int velocity = g_Level.Sinks[sinkVal].strength;
			dx = phd_sin(angle * 16) * velocity * SECTOR(1);
			dz = phd_cos(angle * 16) * velocity * SECTOR(1);

			lara->WaterCurrentPull.x += ((dx - lara->WaterCurrentPull.x) / 16);
			lara->WaterCurrentPull.z += ((dz - lara->WaterCurrentPull.z) / 16);
		}

		lara->WaterCurrentActive = 0;
		UPVItem->Pose.Position.x += lara->WaterCurrentPull.x / CLICK(1);
		UPVItem->Pose.Position.z += lara->WaterCurrentPull.z / CLICK(1);
	}

	static void BackgroundCollision(ItemInfo* laraItem, ItemInfo* UPVItem)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* UPV = GetUPVInfo(UPVItem);
		CollisionInfo cinfo, * coll = &cinfo; // ??

		coll->Setup.Mode = CollisionProbeMode::Quadrants;
		coll->Setup.Radius = UPV_RADIUS;

		coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
		coll->Setup.UpperFloorBound = -UPV_HEIGHT;
		coll->Setup.LowerCeilingBound = UPV_HEIGHT;
		coll->Setup.UpperCeilingBound = NO_UPPER_BOUND;

		coll->Setup.BlockFloorSlopeUp = false;
		coll->Setup.BlockFloorSlopeDown = false;
		coll->Setup.BlockDeathFloorDown = false;
		coll->Setup.BlockMonkeySwingEdge = false;
		coll->Setup.EnableObjectPush = true;
		coll->Setup.EnableSpasm = false;

		coll->Setup.OldPosition = UPVItem->Pose.Position;

		if ((UPVItem->Pose.Orientation.x >= -(SHRT_MAX / 2 + 1)) && (UPVItem->Pose.Orientation.x <= (SHRT_MAX / 2 + 1)))
		{
			lara->Control.MoveAngle = UPVItem->Pose.Orientation.y;
			coll->Setup.ForwardAngle = lara->Control.MoveAngle;
		}
		else
		{
			lara->Control.MoveAngle = UPVItem->Pose.Orientation.y - ANGLE(180.0f);
			coll->Setup.ForwardAngle = lara->Control.MoveAngle;
		}

		int height = phd_sin(UPVItem->Pose.Orientation.x) * UPV_LENGTH;
		if (height < 0)
			height = -height;
		if (height < 200)
			height = 200;

		coll->Setup.UpperFloorBound = -height;
		coll->Setup.Height = height;

		GetCollisionInfo(coll, UPVItem, Vector3Int(0, height / 2, 0));
		ShiftItem(UPVItem, coll);

		if (coll->CollisionType == CT_FRONT)
		{
			if (UPV->TurnRate.x > UPV_DEFLECT_ANGLE)
				UPV->TurnRate.x += UPV_DEFLCT_TURN_RATE_MAX;
			else if (UPV->TurnRate.x < -UPV_DEFLECT_ANGLE)
				UPV->TurnRate.x -= UPV_DEFLCT_TURN_RATE_MAX;
			else
			{
				if (abs(UPV->Velocity) >= UPV_VELOCITY_MAX)
				{
					laraItem->Animation.TargetState = UPV_STATE_HIT;
					UPV->Velocity = -UPV->Velocity / 2;
				}
				else
					UPV->Velocity = 0;
			}
		}
		else if (coll->CollisionType == CT_TOP)
		{
			if (UPV->TurnRate.x >= -UPV_DEFLECT_ANGLE)
				UPV->TurnRate.x -= UPV_DEFLCT_TURN_RATE_MAX;
		}
		else if (coll->CollisionType == CT_TOP_FRONT)
			UPV->Velocity = 0;
		else if (coll->CollisionType == CT_LEFT)
			UPVItem->Pose.Orientation.y += ANGLE(5.0f);
		else if (coll->CollisionType == CT_RIGHT)
			UPVItem->Pose.Orientation.y -= ANGLE(5.0f);
		else if (coll->CollisionType == CT_CLAMP)
		{
			UPVItem->Pose.Position = coll->Setup.OldPosition;
			UPV->Velocity = 0;
			return;
		}

		if (coll->Middle.Floor < 0)
		{
			UPVItem->Pose.Position.y += coll->Middle.Floor;
			UPV->TurnRate.x += UPV_DEFLCT_TURN_RATE_MAX;
		}
	}

	static void UPVControl(ItemInfo* laraItem, ItemInfo* UPVItem)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* UPV = GetUPVInfo(UPVItem);

		TestUPVDismount(laraItem, UPVItem);

		int anim = laraItem->Animation.AnimNumber - Objects[ID_UPV_LARA_ANIMS].animIndex;
		int frame = laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		switch (laraItem->Animation.ActiveState)
		{
		case UPV_STATE_MOVE:
			if (laraItem->HitPoints <= 0)
			{
				laraItem->Animation.TargetState = UPV_STATE_DEATH;
				break;
			}

			if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT))
				ModulateVehicleTurnRateY(&UPV->TurnRate.y, UPV_Y_TURN_RATE_ACCEL, -UPV_Y_TURN_RATE_MAX, UPV_Y_TURN_RATE_MAX);

			if (UPV->Flags & UPV_FLAG_SURFACE)
			{
				int xa = UPVItem->Pose.Orientation.x - UPV_X_ORIENT_WATER_SURFACE;
				int ax = UPV_X_ORIENT_WATER_SURFACE - UPVItem->Pose.Orientation.x;

				if (xa > 0)
				{
					if (xa > ANGLE(1.0f))
						UPVItem->Pose.Orientation.x -= ANGLE(1.0f);
					else
						UPVItem->Pose.Orientation.x -= ANGLE(0.1f);
				}
				else if (ax)
				{
					if (ax > ANGLE(1.0f))
						UPVItem->Pose.Orientation.x += ANGLE(1.0f);
					else
						UPVItem->Pose.Orientation.x += ANGLE(0.1f);
				}
				else
					UPVItem->Pose.Orientation.x = UPV_X_ORIENT_WATER_SURFACE;
			}
			else
			{
				if (TrInput & (VEHICLE_IN_UP | VEHICLE_IN_DOWN))
					ModulateVehicleTurnRateX(&UPV->TurnRate.x, UPV_X_TURN_RATE_ACCEL, -UPV_X_TURN_RATE_MAX, UPV_X_TURN_RATE_MAX);
			}

			if (TrInput & VEHICLE_IN_ACCELERATE)
			{
				if (TrInput & VEHICLE_IN_UP &&
					UPV->Flags & UPV_FLAG_SURFACE &&
					UPVItem->Pose.Orientation.x > -UPV_X_ORIENT_DIVE_MAX)
				{
					UPV->Flags |= UPV_FLAG_DIVE;
				}

				UPV->Velocity += UPV_VELOCITY_ACCEL;
			}
			else
				laraItem->Animation.TargetState = UPV_STATE_IDLE;

			break;

		case UPV_STATE_IDLE:
			if (laraItem->HitPoints <= 0)
			{
				laraItem->Animation.TargetState = UPV_STATE_DEATH;
				break;
			}
			
			if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT))
				ModulateVehicleTurnRateY(&UPV->TurnRate.y, UPV_Y_TURN_RATE_ACCEL, -UPV_Y_TURN_RATE_MAX, UPV_Y_TURN_RATE_MAX);

			if (UPV->Flags & UPV_FLAG_SURFACE)
			{
				int xa = UPVItem->Pose.Orientation.x - UPV_X_ORIENT_WATER_SURFACE;
				int ax = UPV_X_ORIENT_WATER_SURFACE - UPVItem->Pose.Orientation.x;
				if (xa > 0)
				{
					if (xa > ANGLE(1.0f))
						UPVItem->Pose.Orientation.x -= ANGLE(1.0f);
					else
						UPVItem->Pose.Orientation.x -= ANGLE(0.1f);
				}
				else if (ax)
				{
					if (ax > ANGLE(1.0f))
						UPVItem->Pose.Orientation.x += ANGLE(1.0f);
					else
						UPVItem->Pose.Orientation.x += ANGLE(0.1f);
				}
				else
					UPVItem->Pose.Orientation.x = UPV_X_ORIENT_WATER_SURFACE;
			}
			else
			{
				if (TrInput & (VEHICLE_IN_UP | VEHICLE_IN_DOWN))
					ModulateVehicleTurnRateX(&UPV->TurnRate.x, UPV_X_TURN_RATE_ACCEL, -UPV_X_TURN_RATE_MAX, UPV_X_TURN_RATE_MAX);
			}

			if (TrInput & VEHICLE_IN_DISMOUNT && TestUPVDismount(laraItem, UPVItem))
			{
				if (UPV->Velocity > 0)
					UPV->Velocity -= UPV_VELOCITY_ACCEL;
				else
				{
					if (UPV->Flags & UPV_FLAG_SURFACE)
						laraItem->Animation.TargetState = UPV_STATE_DISMOUNT_WATER_SURFACE;
					else
						laraItem->Animation.TargetState = UPV_STATE_DISMOUNT_UNDERWATER;

					//sub->Flags &= ~UPV_FLAG_CONTROL; having this here causes the UPV glitch, moving it directly to the states' code is better

					StopSoundEffect(SFX_TR3_VEHICLE_UPV_LOOP);
					SoundEffect(SFX_TR3_VEHICLE_UPV_STOP, (PHD_3DPOS*)&UPVItem->Pose.Position.x, SoundEnvironment::Always);
				}
			}

			else if (TrInput & VEHICLE_IN_ACCELERATE)
			{
				if (TrInput & VEHICLE_IN_UP &&
					UPVItem->Pose.Orientation.x > -UPV_X_ORIENT_DIVE_MAX &&
					UPV->Flags & UPV_FLAG_SURFACE)
				{
					UPV->Flags |= UPV_FLAG_DIVE;
				}

				laraItem->Animation.TargetState = UPV_STATE_MOVE;
			}

			break;

		case UPV_STATE_MOUNT:
			if (anim == UPV_ANIM_MOUNT_SURFACE_END)
			{
				UPVItem->Pose.Position.y += 4;
				UPVItem->Pose.Orientation.x += ANGLE(1.0f);

				if (frame == UPV_MOUNT_WATER_SURFACE_SOUND_FRAME)
					SoundEffect(SFX_TR3_VEHICLE_UPV_LOOP, (PHD_3DPOS*)&UPVItem->Pose.Position.x, SoundEnvironment::Always);

				if (frame == UPV_MOUNT_WATER_SURFACE_CONTROL_FRAME)
					UPV->Flags |= UPV_FLAG_CONTROL;
			}

			else if (anim == UPV_ANIM_MOUNT_UNDERWATER)
			{
				if (frame == UPV_MOUNT_UNDERWATER_SOUND_FRAME)
					SoundEffect(SFX_TR3_VEHICLE_UPV_LOOP, (PHD_3DPOS*)&UPVItem->Pose.Position.x, SoundEnvironment::Always);

				if (frame == UPV_MOUNT_UNDERWATER_CONTROL_FRAME)
					UPV->Flags |= UPV_FLAG_CONTROL;
			}

			break;

		case UPV_STATE_DISMOUNT_UNDERWATER:
			if (anim == UPV_ANIM_DISMOUNT_UNDERWATER && frame == UPV_DISMOUNT_UNDERWATER_FRAME)
			{
				UPV->Flags &= ~UPV_FLAG_CONTROL;

				Vector3Int vec = { 0, 0, 0 };
				GetLaraJointPosition(&vec, LM_HIPS);

				auto LPos = GameVector(
					vec.x,
					vec.y,
					vec.z,
					UPVItem->RoomNumber
				);

				auto VPos = GameVector(
					UPVItem->Pose.Position.x,
					UPVItem->Pose.Position.y,
					UPVItem->Pose.Position.z,
					UPVItem->RoomNumber
				);
				LOSAndReturnTarget(&VPos, &LPos, 0);

				laraItem->Pose.Position = Vector3Int(LPos.x, LPos.y, LPos.z);

				SetAnimation(laraItem, LA_UNDERWATER_IDLE);
				laraItem->Animation.VerticalVelocity = 0;
				laraItem->Animation.IsAirborne = false;
				laraItem->Pose.Orientation.x = laraItem->Pose.Orientation.z = 0;

				UpdateItemRoom(laraItem, 0);

				lara->Control.WaterStatus = WaterStatus::Underwater;
				lara->Control.HandStatus = HandStatus::Free;
				lara->Vehicle = NO_ITEM;

				UPVItem->HitPoints = 0;
			}

			break;

		case UPV_STATE_DISMOUNT_WATER_SURFACE:
			if (anim == UPV_ANIM_DISMOUNT_SURFACE && frame == UPV_DISMOUNT_WATER_SURFACE_FRAME)
			{
				UPV->Flags &= ~UPV_FLAG_CONTROL;
				int waterDepth, waterHeight, heightFromWater;

				waterDepth = GetWaterSurface(laraItem);
				waterHeight = GetWaterHeight(laraItem);

				if (waterHeight != NO_HEIGHT)
					heightFromWater = laraItem->Pose.Position.y - waterHeight;
				else
					heightFromWater = NO_HEIGHT;

				Vector3Int vec = { 0, 0, 0 };
				GetLaraJointPosition(&vec, LM_HIPS);

				laraItem->Pose.Position.x = vec.x;
				//laraItem->Pose.Position.y += -heightFromWater + 1; // Doesn't work as intended.
				laraItem->Pose.Position.y = vec.y;
				laraItem->Pose.Position.z = vec.z;

				SetAnimation(laraItem, LA_ONWATER_IDLE);
				laraItem->Animation.VerticalVelocity = 0;
				laraItem->Animation.IsAirborne = false;
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;

				UpdateItemRoom(laraItem, -LARA_HEIGHT / 2);

				ResetLaraFlex(laraItem);
				lara->Control.WaterStatus = WaterStatus::TreadWater;
				lara->WaterSurfaceDist = -heightFromWater;
				lara->Control.HandStatus = HandStatus::Free;
				lara->Vehicle = NO_ITEM;

				UPVItem->HitPoints = 0;
			}
			else
			{
				UPV->TurnRate.x -= UPV_X_TURN_RATE_ACCEL;
				if (UPVItem->Pose.Orientation.x < 0)
					UPVItem->Pose.Orientation.x = 0;
			}

			break;

		case UPV_STATE_DEATH:
			if ((anim == UPV_ANIM_DEATH || anim == UPV_ANIM_DEATH_MOVING) &&
				(frame == UPV_DEATH_FRAME_1 || frame == UPV_DEATH_FRAME_2))
			{
				auto vec = Vector3Int();
				GetLaraJointPosition(&vec, LM_HIPS);

				laraItem->Pose.Position = vec;
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;

				SetAnimation(laraItem, LA_UNDERWATER_DEATH, 17);
				laraItem->Animation.IsAirborne = false;
				laraItem->Animation.VerticalVelocity = 0;
			
				UPV->Flags |= UPV_FLAG_DEAD;
			}

			UPVItem->Animation.Velocity = 0;
			break;
		}

		if (UPV->Flags & UPV_FLAG_DIVE)
		{
			if (UPVItem->Pose.Orientation.x > -UPV_X_ORIENT_DIVE_MAX)
				UPVItem->Pose.Orientation.x -= UPV_X_TURN_RATE_DIVE_ACCEL;
			else
				UPV->Flags &= ~UPV_FLAG_DIVE;
		}

		if (UPV->Velocity > 0)
		{
			UPV->Velocity -= UPV_VELOCITY_FRICTION_DECEL;
			if (UPV->Velocity < 0)
				UPV->Velocity = 0;
		}
		else if (UPV->Velocity < 0)
		{
			UPV->Velocity += UPV_VELOCITY_FRICTION_DECEL;
			if (UPV->Velocity > 0)
				UPV->Velocity = 0;
		}

		if (UPV->Velocity > UPV_VELOCITY_MAX)
			UPV->Velocity = UPV_VELOCITY_MAX;
		else if (UPV->Velocity < -UPV_VELOCITY_MAX)
			UPV->Velocity = -UPV_VELOCITY_MAX;

		if (UPV->TurnRate.x > 0)
		{
			UPV->TurnRate.x -= UPV_X_TURN_RATE_FRICTION_DECEL;
			if (UPV->TurnRate.x < 0)
				UPV->TurnRate.x = 0;
		}
		else if (UPV->TurnRate.x < 0)
		{
			UPV->TurnRate.x += UPV_X_TURN_RATE_FRICTION_DECEL;
			if (UPV->TurnRate.x > 0)
				UPV->TurnRate.x = 0;
		}

		if (UPV->TurnRate.y > 0)
		{
			UPV->TurnRate.y -= UPV_Y_TURN_RATE_FRICTION_DECEL;
			if (UPV->TurnRate.y < 0)
				UPV->TurnRate.y = 0;
		}
		else if (UPV->TurnRate.y < 0)
		{
			UPV->TurnRate.y += UPV_Y_TURN_RATE_FRICTION_DECEL;
			if (UPV->TurnRate.y > 0)
				UPV->TurnRate.y = 0;
		}

		// TODO: Deceleration must be done some other way.
		/*if (UPV->TurnRate.x)
			ModulateVehicleTurnRateX(&UPV->TurnRate.x, -UPV_X_TURN_RATE_FRICTION_DECEL, 0, UPV_X_TURN_RATE_MAX);

		if (UPV->TurnRate.y)
			ModulateVehicleTurnRateY(&UPV->TurnRate.y, -UPV_Y_TURN_RATE_FRICTION_DECEL, 0, UPV_Y_TURN_RATE_MAX);*/
	}

	void NoGetOnCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!TestBoundsCollide(item, laraItem, coll->Setup.Radius))
			return;
		if (!TestCollision(item, laraItem))
			return;

		ItemPushItem(item, laraItem, coll, false, false);
	}

	void UPVCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* UPVItem = &g_Level.Items[itemNumber];

		if (laraItem->HitPoints <= 0 || lara->Vehicle != NO_ITEM)
			return;

		if (TestUPVMount(laraItem, UPVItem))
		{
			lara->Vehicle = itemNumber;
			lara->Control.WaterStatus = WaterStatus::Dry;

			if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
			{
				CreateFlare(laraItem, ID_FLARE_ITEM, 0);
				UndrawFlareMeshes(laraItem);

				lara->Flare.ControlLeft = false;
				lara->Control.Weapon.RequestGunType = lara->Control.Weapon.GunType = LaraWeaponType::None;
			}

			laraItem->Pose = UPVItem->Pose;
			lara->Control.HandStatus = HandStatus::Busy;
			UPVItem->HitPoints = 1;

			if (laraItem->Animation.ActiveState == LS_ONWATER_IDLE || laraItem->Animation.ActiveState == LS_ONWATER_FORWARD)
			{
				laraItem->Animation.AnimNumber = Objects[ID_UPV_LARA_ANIMS].animIndex + UPV_ANIM_MOUNT_SURFACE_START;
				laraItem->Animation.ActiveState = UPV_STATE_MOUNT;
				laraItem->Animation.TargetState = UPV_STATE_MOUNT;
			}
			else
			{
				laraItem->Animation.AnimNumber = Objects[ID_UPV_LARA_ANIMS].animIndex + UPV_ANIM_MOUNT_UNDERWATER;
				laraItem->Animation.ActiveState = UPV_STATE_MOUNT;
				laraItem->Animation.TargetState = UPV_STATE_MOUNT;
			}

			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			AnimateItem(laraItem);
		}
		else
		{
			UPVItem->Pose.Position.y += UPV_SHIFT;
			NoGetOnCollision(itemNumber, laraItem, coll);
			UPVItem->Pose.Position.y -= UPV_SHIFT;
		}
	}

	bool UPVControl(ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* UPVItem = &g_Level.Items[lara->Vehicle];
		auto* UPV = GetUPVInfo(UPVItem);
	
		auto oldPos = UPVItem->Pose;
		auto probe = GetCollision(UPVItem);

		if (!(UPV->Flags & UPV_FLAG_DEAD))
		{
			UPVControl(laraItem, UPVItem);

			UPVItem->Animation.Velocity = UPV->Velocity / VEHICLE_VELOCITY_SCALE;
			UPVItem->Pose.Orientation += UPV->TurnRate;

			if (UPVItem->Pose.Orientation.x > UPV_X_ORIENT_MAX)
				UPVItem->Pose.Orientation.x = UPV_X_ORIENT_MAX;
			else if (UPVItem->Pose.Orientation.x < -UPV_X_ORIENT_MAX)
				UPVItem->Pose.Orientation.x = -UPV_X_ORIENT_MAX;

			TranslateItem(UPVItem, UPVItem->Pose.Orientation, UPVItem->Animation.Velocity);
		}

		int newHeight = GetCollision(UPVItem).Position.Floor;
		int waterHeight = GetWaterHeight(UPVItem);

		if ((newHeight - waterHeight) < UPV_HEIGHT || (newHeight < UPVItem->Pose.Position.y - UPV_HEIGHT / 2) || 
			(newHeight == NO_HEIGHT) || (waterHeight == NO_HEIGHT))
		{
			UPVItem->Pose.Position = oldPos.Position;
			UPVItem->Animation.Velocity = 0;
		}

		UPVItem->Floor = probe.Position.Floor;

		if (UPV->Flags & UPV_FLAG_CONTROL && !(UPV->Flags & UPV_FLAG_DEAD))
		{
			if (!TestEnvironment(ENV_FLAG_WATER, UPVItem->RoomNumber) &&
				waterHeight != NO_HEIGHT)
			{
				if ((waterHeight - UPVItem->Pose.Position.y) >= -UPV_WATER_SURFACE_DISTANCE)
					UPVItem->Pose.Position.y = waterHeight + UPV_WATER_SURFACE_DISTANCE;

				if (!(UPV->Flags & UPV_FLAG_SURFACE))
				{
					SoundEffect(SFX_TR4_LARA_BREATH, &laraItem->Pose, SoundEnvironment::Always);
					UPV->Flags &= ~UPV_FLAG_DIVE;
				}

				UPV->Flags |= UPV_FLAG_SURFACE;
			}
			else if ((waterHeight - UPVItem->Pose.Position.y) >= -UPV_WATER_SURFACE_DISTANCE && waterHeight != NO_HEIGHT &&
					 (laraItem->Pose.Position.y - probe.Position.Ceiling) >= CLICK(1))
			{
				UPVItem->Pose.Position.y = waterHeight + UPV_WATER_SURFACE_DISTANCE;

				if (!(UPV->Flags & UPV_FLAG_SURFACE))
				{
					SoundEffect(SFX_TR4_LARA_BREATH, &laraItem->Pose, SoundEnvironment::Always);
					UPV->Flags &= ~UPV_FLAG_DIVE;
				}

				UPV->Flags |= UPV_FLAG_SURFACE;
			}
			else
				UPV->Flags &= ~UPV_FLAG_SURFACE;

			if (!(UPV->Flags & UPV_FLAG_SURFACE))
			{
				if (laraItem->HitPoints > 0)
				{
					lara->Air--;
					if (lara->Air < 0)
					{
						lara->Air = -1;
						laraItem->HitPoints -= 5;
					}
				}
			}
			else
			{
				if (laraItem->HitPoints >= 0)
				{
					lara->Air += 10;
					if (lara->Air > LARA_AIR_MAX)
						lara->Air = LARA_AIR_MAX;
				}
			}
		}

		TestTriggers(UPVItem, false);
		UPVEffects(lara->Vehicle);

		if (!(UPV->Flags & UPV_FLAG_DEAD) &&
			lara->Vehicle != NO_ITEM)
		{
			DoCurrent(laraItem, UPVItem);

			if (TrInput & VEHICLE_IN_FIRE &&
				UPV->Flags & UPV_FLAG_CONTROL &&
				!UPV->HarpoonTimer)
			{
				if (laraItem->Animation.ActiveState != UPV_STATE_DISMOUNT_UNDERWATER &&
					laraItem->Animation.ActiveState != UPV_STATE_DISMOUNT_WATER_SURFACE &&
					laraItem->Animation.ActiveState != UPV_STATE_MOUNT)
				{
					FireUPVHarpoon(laraItem, UPVItem);
					UPV->HarpoonTimer = UPV_HARPOON_RELOAD_TIME;
				}
			}

			if (probe.RoomNumber != UPVItem->RoomNumber)
			{
				ItemNewRoom(lara->Vehicle, probe.RoomNumber);
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
			}

			laraItem->Pose = UPVItem->Pose;

			AnimateItem(laraItem);
			BackgroundCollision(laraItem, UPVItem);
			DoVehicleCollision(UPVItem, UPV_RADIUS);

			if (UPV->Flags & UPV_FLAG_CONTROL)
				SoundEffect(SFX_TR3_VEHICLE_UPV_LOOP, (PHD_3DPOS*)&UPVItem->Pose.Position.x, SoundEnvironment::Always, 1.0f + (float)UPVItem->Animation.Velocity / 96.0f);

			UPVItem->Animation.AnimNumber = Objects[ID_UPV].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_UPV_LARA_ANIMS].animIndex);
			UPVItem->Animation.FrameNumber = g_Level.Anims[UPVItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);

			if (UPV->Flags & UPV_FLAG_SURFACE)
				Camera.targetElevation = -ANGLE(60.0f);
			else
				Camera.targetElevation = 0;

			return true;
		}
		else if (UPV->Flags & UPV_FLAG_DEAD)
		{
			AnimateItem(laraItem);

			if (probe.RoomNumber != UPVItem->RoomNumber)
				ItemNewRoom(lara->Vehicle, probe.RoomNumber);

			BackgroundCollision(laraItem, UPVItem);

			UPV->TurnRate.x = 0;

			SetAnimation(UPVItem, UPV_ANIM_IDLE);
			UPVItem->Animation.VerticalVelocity = 0;
			UPVItem->Animation.Velocity = 0;
			UPVItem->Animation.IsAirborne = true;
			AnimateItem(UPVItem);

			return true;
		}
		else
			return false;
	}
}
