#include "framework.h"
#include "Game/Lara/lara_fire.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/control/los.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_two_guns.h"
#include "Game/misc.h"
#include "Game/savegame.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Objects/Generic/Object/objects.h"
#include "Scripting/GameFlowScript.h"
#include "Sound/sound.h"
#include "Specific/setup.h"
#include "Specific/input.h"
#include "Specific/level.h"

using namespace TEN::Entities::Generic;

bool MonksAttackLara;
ITEM_INFO* LastTargets[MAX_TARGETS];
ITEM_INFO* TargetList[MAX_TARGETS];

WeaponInfo Weapons[(int)LaraWeaponType::NumWeapons] =
{
	// No weapons
	{
		{ EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f) },
		{ EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f) },
		{ EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f) },
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// Pistols
	{
		{ EulerAngle::DegToRad(-60.0f),  EulerAngle::DegToRad(60.0f),  EulerAngle::DegToRad(-60.0f), EulerAngle::DegToRad(60.0f) },
		{ EulerAngle::DegToRad(-170.0f), EulerAngle::DegToRad(60.0f),  EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f) },
		{ EulerAngle::DegToRad(-60.0f),  EulerAngle::DegToRad(170.0f), EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f) },
		EulerAngle::DegToRad(10.0f),
		EulerAngle::DegToRad(8.0f),
		650,
		SECTOR(8),
		1,
		9,
		3,
		0,
		SFX_TR4_LARA_FIRE,
		0
	},

	// Revolver
	{
		{ EulerAngle::DegToRad(-60.0f), EulerAngle::DegToRad(60.0f), EulerAngle::DegToRad(-60.0f), EulerAngle::DegToRad(60.0f) },
		{ EulerAngle::DegToRad(-10.0f), EulerAngle::DegToRad(10.0f), EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f) },
		{  EulerAngle::DegToRad(0.0f),   EulerAngle::DegToRad(0.0f),   EulerAngle::DegToRad(0.0f),  EulerAngle::DegToRad(0.0f) },
		EulerAngle::DegToRad(10.0f),
		EulerAngle::DegToRad(4.0f),
		650,
		SECTOR(8),
		21,
		16,
		3,
		0,
		SFX_TR4_DESSERT_EAGLE_FIRE,
		0
	},

	// Uzis
	{
		{ EulerAngle::DegToRad(-60.0f), EulerAngle::DegToRad(60.0f), EulerAngle::DegToRad(-60.0f), EulerAngle::DegToRad(60.0f) },
		{ EulerAngle::DegToRad(-170.0f), EulerAngle::DegToRad(60.0f),  EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f) },
		{ EulerAngle::DegToRad(-60.0f),  EulerAngle::DegToRad(170.0f), EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f) },
		EulerAngle::DegToRad(10.0f),
		EulerAngle::DegToRad(8.0f),
		650,
		SECTOR(8),
		1,
		3,
		3,
		0,
		SFX_TR4_LARA_UZI_FIRE,
		0
	},

	// Shotgun
	{
		{ EulerAngle::DegToRad(-60.0f), EulerAngle::DegToRad(60.0f), EulerAngle::DegToRad(-55.0f), EulerAngle::DegToRad(55.0f) },
		{ EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f), EulerAngle::DegToRad(-65.0f), EulerAngle::DegToRad(65.0f) },
		{ EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f), EulerAngle::DegToRad(-65.0f), EulerAngle::DegToRad(65.0f) },
		EulerAngle::DegToRad(10.0f),
		0,
		500,
		SECTOR(8),
		3,
		9,
		3,
		10,
		SFX_TR4_LARA_SHOTGUN,
		0
	},

	// HK
	{
		{ EulerAngle::DegToRad(-60.0f), EulerAngle::DegToRad(60.0f), EulerAngle::DegToRad(-55.0f), EulerAngle::DegToRad(55.0f) },
		{ EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f), EulerAngle::DegToRad(-65.0f), EulerAngle::DegToRad(65.0f) },
		{ EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f), EulerAngle::DegToRad(-65.0f), EulerAngle::DegToRad(65.0f) },
		EulerAngle::DegToRad(10.0f),
		EulerAngle::DegToRad(4.0f),
		500,
		SECTOR(12),
		4,
		0,
		3,
		10,
		0,     // FIRE/SILENCER_FIRE
		0
	},

	// Crossbow
	{
		{ EulerAngle::DegToRad(-60.0f), EulerAngle::DegToRad(60.0f), EulerAngle::DegToRad(-55.0f), EulerAngle::DegToRad(55.0f) },
		{ EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f), EulerAngle::DegToRad(-65.0f), EulerAngle::DegToRad(65.0f) },
		{ EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f), EulerAngle::DegToRad(-65.0f), EulerAngle::DegToRad(65.0f) },
		EulerAngle::DegToRad(10.0f),
		EulerAngle::DegToRad(8.0f),
		500,
		SECTOR(8),
		5,
		0,
		2,
		10,
		SFX_TR4_LARA_CROSSBOW,
		20
	},

	// Flare
	{
		{ EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f) },
		{ EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f) },
		{ EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f), EulerAngle::DegToRad(0.0f) },
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// Flare 2
	{
		{ EulerAngle::DegToRad(-30.0f), EulerAngle::DegToRad(30.0f), EulerAngle::DegToRad(-55.0f), EulerAngle::DegToRad(55.0f) },
		{ EulerAngle::DegToRad(-30.0f), EulerAngle::DegToRad(30.0f), EulerAngle::DegToRad(-55.0f), EulerAngle::DegToRad(55.0f) },
		{ EulerAngle::DegToRad(-30.0f), EulerAngle::DegToRad(30.0f), EulerAngle::DegToRad(-55.0f), EulerAngle::DegToRad(55.0f) },
		EulerAngle::DegToRad(10.0f),
		EulerAngle::DegToRad(8.0f),
		400,
		SECTOR(8),
		3,
		0,
		2,
		0,
		SFX_TR4_LARA_UZI_FIRE,
		0
	},

	// Grenade launcher
	{
		{ EulerAngle::DegToRad(-60.0f), EulerAngle::DegToRad(60.0f), EulerAngle::DegToRad(-55.0f), EulerAngle::DegToRad(55.0f) },
		{ EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f), EulerAngle::DegToRad(-65.0f), EulerAngle::DegToRad(65.0f) },
		{ EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f), EulerAngle::DegToRad(-65.0f), EulerAngle::DegToRad(65.0f) },
		EulerAngle::DegToRad(10.0f),
		EulerAngle::DegToRad(8.0f),
		500,
		SECTOR(8),
		20,
		0,
		2,
		10,
		0,
		30
	},

	// Harpoon gun
	{
		{ EulerAngle::DegToRad(-60.0f), EulerAngle::DegToRad(60.0f), EulerAngle::DegToRad(-65.0f), EulerAngle::DegToRad(65.0f) },
		{ EulerAngle::DegToRad(-20.0f), EulerAngle::DegToRad(20.0f), EulerAngle::DegToRad(-75.0f), EulerAngle::DegToRad(75.0f) },
		{ EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f), EulerAngle::DegToRad(-75.0f), EulerAngle::DegToRad(75.0f) },
		EulerAngle::DegToRad(10.0f),
		EulerAngle::DegToRad(8.0f),
		500,
		SECTOR(8),
		6,
		0,
		2,
		10,
		0,
		0
	},

	// Rocket launcher
	{
		{ EulerAngle::DegToRad(-60.0f), EulerAngle::DegToRad(60.0f), EulerAngle::DegToRad(-55.0f), EulerAngle::DegToRad(55.0f) },
		{ EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f), EulerAngle::DegToRad(-65.0f), EulerAngle::DegToRad(65.0f) },
		{ EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f), EulerAngle::DegToRad(-65.0f), EulerAngle::DegToRad(65.0f) },
		EulerAngle::DegToRad(10.0f),
		EulerAngle::DegToRad(8.0f),
		500,
		SECTOR(8),
		30,
		0,
		2,
		12,
		77,
		30
	},

	// Snowmobile
	{
		{ EulerAngle::DegToRad(-30.0f), EulerAngle::DegToRad(30.0f), EulerAngle::DegToRad(-55.0f), EulerAngle::DegToRad(55.0f) },
		{ EulerAngle::DegToRad(-30.0f), EulerAngle::DegToRad(30.0f), EulerAngle::DegToRad(-55.0f), EulerAngle::DegToRad(55.0f) },
		{ EulerAngle::DegToRad(-30.0f), EulerAngle::DegToRad(30.0f), EulerAngle::DegToRad(-55.0f), EulerAngle::DegToRad(55.0f) },
		EulerAngle::DegToRad(10.0f),
		EulerAngle::DegToRad(8.0f),
		400,
		SECTOR(8),
		3,
		0,
		0,
		0,
		SFX_TR4_LARA_UZI_FIRE,
		0
	}
};

// States in which Lara will hold the flare out in front.
int HoldStates[] =
{
	LS_WALK_FORWARD,
	LS_RUN_FORWARD,
	LS_IDLE,
	LS_TURN_RIGHT_SLOW,
	LS_TURN_LEFT_SLOW,
	LS_WALK_BACK,
	LS_TURN_RIGHT_FAST,
	LS_TURN_LEFT_FAST,
	LS_STEP_RIGHT,
	LS_STEP_LEFT,
	LS_PICKUP,
	LS_SWITCH_DOWN,
	LS_SWITCH_UP,
	LS_WADE_FORWARD,
	LS_CROUCH_IDLE,
	LS_CROUCH_TURN_LEFT,
	LS_CROUCH_TURN_RIGHT,
	LS_SOFT_SPLAT,
	-1
};

bool CheckForHoldingState(LaraState state)
{
#if 0
	if (lara->ExtraAnim != NO_ITEM)
		return false;
#endif

	int* holdState = HoldStates;
	while (*holdState >= 0)
	{
		if (state == *holdState)
			return true;

		holdState++;
	}

	return false;
}

GAME_OBJECT_ID WeaponObject(LaraWeaponType weaponType)
{
	switch (weaponType)
	{
	case LaraWeaponType::Uzi:
		return ID_UZI_ANIM;

	case LaraWeaponType::Shotgun:
		return ID_SHOTGUN_ANIM;

	case LaraWeaponType::Revolver:
		return ID_REVOLVER_ANIM;

	case LaraWeaponType::Crossbow:
		return ID_CROSSBOW_ANIM;

	case LaraWeaponType::HK:
		return ID_HK_ANIM;

	case LaraWeaponType::Flare:
		return ID_LARA_FLARE_ANIM;

	case LaraWeaponType::GrenadeLauncher:
		return ID_GRENADE_ANIM;

	case LaraWeaponType::RocketLauncher:
		return ID_ROCKET_ANIM;

	case LaraWeaponType::HarpoonGun:
		return ID_HARPOON_ANIM;

	default:
		return ID_PISTOLS_ANIM;
	}
}

void AimWeapon(ITEM_INFO* laraItem, WeaponInfo* weaponInfo, ArmInfo* arm)
{
	auto* lara = GetLaraInfo(laraItem);

	float x = 0;
	float y = 0;

	// Have target lock; get XY angles for arms.
	if (arm->Locked)
	{
		y = lara->TargetArmAngles[0];
		x = lara->TargetArmAngles[1];
	}

	int speed = weaponInfo->AimSpeed;

	// Rotate arms on y axis toward target.
	float rotY = arm->Rotation.y;
	if (rotY >= (y - speed) && rotY <= (y + speed))
		rotY = y;
	else if (rotY < y)
		rotY += speed;
	else
		rotY -= speed;
	arm->Rotation.y = rotY;

	// Rotate arms on x axis toward target.
	float rotX = arm->Rotation.x;
	if (rotX >= (x - speed) && rotX <= (x + speed))
		rotX = x;
	else if (rotX < x)
		rotX += speed;
	else
		rotX -= speed;
	arm->Rotation.x = rotX;

	// TODO: Set arms to inherit rotations of parent bones.
	arm->Rotation.z = 0;
}

void SmashItem(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->ObjectNumber >= ID_SMASH_OBJECT1 && item->ObjectNumber <= ID_SMASH_OBJECT8)
		SmashObject(itemNumber);
}

void LaraGun(ITEM_INFO* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	if (lara->LeftArm.FlashGun > 0)
		--lara->LeftArm.FlashGun;
	if (lara->RightArm.FlashGun > 0)
		--lara->RightArm.FlashGun;

	if (lara->Control.Weapon.GunType == LaraWeaponType::Torch)
	{
		DoFlameTorch();
		return;
	}

	if (laraItem->HitPoints <= 0)
		lara->Control.HandStatus = HandStatus::Free;
	else if (lara->Control.HandStatus == HandStatus::Free)
	{
		// Draw weapon.
		if (TrInput & IN_DRAW)
			lara->Control.Weapon.RequestGunType = lara->Control.Weapon.LastGunType;
		// Draw flare.
		else if (TrInput & IN_FLARE &&
			(g_GameFlow->GetLevel(CurrentLevel)->LaraType != LaraType::Young))
		{
			if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
			{
			//	if (!lara->leftArm.frameNumber)	// NO
				{
					lara->Control.HandStatus = HandStatus::WeaponUndraw;
				}
			}
			else if (lara->Inventory.TotalFlares)
			{
				if (lara->Inventory.TotalFlares != -1)
					lara->Inventory.TotalFlares--;

				lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			}
		}

		if (TrInput & IN_DRAW ||
			lara->Control.Weapon.RequestGunType != lara->Control.Weapon.GunType)
		{
			if ((laraItem->Animation.ActiveState == LS_CROUCH_IDLE ||
				laraItem->Animation.ActiveState == LS_CROUCH_TURN_LEFT ||
				laraItem->Animation.ActiveState == LS_CROUCH_TURN_RIGHT) &&
				(lara->Control.Weapon.RequestGunType == LaraWeaponType::HK ||
					lara->Control.Weapon.RequestGunType == LaraWeaponType::Crossbow ||
					lara->Control.Weapon.RequestGunType == LaraWeaponType::Shotgun ||
					lara->Control.Weapon.RequestGunType == LaraWeaponType::HarpoonGun))
			{
				if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
					lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			}
			else if (lara->Control.Weapon.RequestGunType == LaraWeaponType::Flare ||
				(lara->Vehicle == NO_ITEM &&
					(lara->Control.Weapon.RequestGunType == LaraWeaponType::HarpoonGun ||
						lara->Control.WaterStatus == WaterStatus::Dry ||
						(lara->Control.WaterStatus == WaterStatus::Wade &&
							lara->WaterSurfaceDist > -Weapons[(int)lara->Control.Weapon.GunType].GunHeight))))
			{
				if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
				{
					CreateFlare(laraItem, ID_FLARE_ITEM, 0);
					UndrawFlareMeshes(laraItem);
					lara->Flare.ControlLeft = false;
					lara->Flare.Life = 0;
				}

				lara->Control.Weapon.GunType = lara->Control.Weapon.RequestGunType;
				InitialiseNewWeapon(laraItem);
				lara->RightArm.FrameNumber = 0;
				lara->LeftArm.FrameNumber = 0;
				lara->Control.HandStatus = HandStatus::WeaponDraw;
			}
			else
			{
				lara->Control.Weapon.LastGunType = lara->Control.Weapon.RequestGunType;

				if (lara->Control.Weapon.GunType != LaraWeaponType::Flare)
					lara->Control.Weapon.GunType = lara->Control.Weapon.RequestGunType;
				else
					lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
			}
		}
	}
	else if (lara->Control.HandStatus == HandStatus::WeaponReady)
	{
		if (TrInput & IN_DRAW ||
			lara->Control.Weapon.RequestGunType != lara->Control.Weapon.GunType)
		{
			lara->Control.HandStatus = HandStatus::WeaponUndraw;
		}
		else if (lara->Control.Weapon.GunType != LaraWeaponType::HarpoonGun &&
			lara->Control.WaterStatus != WaterStatus::Dry &&
			(lara->Control.WaterStatus != WaterStatus::Wade ||
				lara->WaterSurfaceDist < -Weapons[(int)lara->Control.Weapon.GunType].GunHeight))
		{
			lara->Control.HandStatus = HandStatus::WeaponUndraw;
		}
	}
	else if (TrInput & IN_FLARE &&
		lara->Control.HandStatus == HandStatus::Busy &&
		laraItem->Animation.ActiveState == LS_CRAWL_IDLE &&
		laraItem->Animation.AnimNumber == LA_CRAWL_IDLE)
	{
		lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
	}

	switch (lara->Control.HandStatus)
	{
	case HandStatus::WeaponDraw:
		if (lara->Control.Weapon.GunType != LaraWeaponType::Flare &&
			lara->Control.Weapon.GunType != LaraWeaponType::None)
		{
			lara->Control.Weapon.LastGunType = lara->Control.Weapon.GunType;
		}

		switch (lara->Control.Weapon.GunType)
		{
		case LaraWeaponType::Pistol:
		case LaraWeaponType::Revolver:
		case LaraWeaponType::Uzi:
			if (Camera.type != CameraType::Look && Camera.type != CameraType::Heavy)
				Camera.type = CameraType::Combat;

			DrawPistols(laraItem, lara->Control.Weapon.GunType);
			break;

		case LaraWeaponType::Shotgun:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::HK:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
			if (Camera.type != CameraType::Look && Camera.type != CameraType::Heavy)
				Camera.type = CameraType::Combat;

			DrawShotgun(laraItem, lara->Control.Weapon.GunType);
			break;

		case LaraWeaponType::Flare:
			DrawFlare(laraItem);
			break;

		default:
			lara->Control.HandStatus = HandStatus::Free;
			break;
		}

		break;

	case HandStatus::Special:
		DrawFlare(laraItem);
		break;

	case HandStatus::WeaponUndraw:
		lara->MeshPtrs[LM_HEAD] = Objects[ID_LARA_SKIN].meshIndex + LM_HEAD;

		switch (lara->Control.Weapon.GunType)
		{
		case LaraWeaponType::Pistol:
		case LaraWeaponType::Revolver:
		case LaraWeaponType::Uzi:
			UndrawPistols(laraItem, lara->Control.Weapon.GunType);
			break;

		case LaraWeaponType::Shotgun:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::HK:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
			UndrawShotgun(laraItem, lara->Control.Weapon.GunType);
			break;

		case LaraWeaponType::Flare:
			UndrawFlare(laraItem);
			break;

		default:
			return;
		}

		break;

	case HandStatus::WeaponReady:
		if (!(TrInput & IN_ACTION))
			lara->MeshPtrs[LM_HEAD] = Objects[ID_LARA_SKIN].meshIndex + LM_HEAD;
		else
			lara->MeshPtrs[LM_HEAD] = Objects[ID_LARA_SCREAM].meshIndex + LM_HEAD;

		if (Camera.type != CameraType::Look &&
			Camera.type != CameraType::Heavy)
		{
			Camera.type = CameraType::Combat;
		}

		if (TrInput & IN_ACTION)
		{
			if (!GetAmmo(laraItem, lara->Control.Weapon.GunType))
			{
				lara->Control.Weapon.RequestGunType = Objects[ID_PISTOLS_ITEM].loaded ? LaraWeaponType::Pistol : LaraWeaponType::None;
				return;
			}
		}

		switch (lara->Control.Weapon.GunType)
		{
		case LaraWeaponType::Pistol:
		case LaraWeaponType::Uzi:
			PistolHandler(laraItem, lara->Control.Weapon.GunType);

			break;

		case LaraWeaponType::Shotgun:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::HK:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
		case LaraWeaponType::Revolver:
			RifleHandler(laraItem, lara->Control.Weapon.GunType);
			break;

		default:
			return;
		}

		break;

	case HandStatus::Free:
		if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
		{
			if (lara->Vehicle != NO_ITEM ||
				CheckForHoldingState((LaraState)laraItem->Animation.ActiveState))
			{
				if (lara->Flare.ControlLeft)
				{
					if (lara->LeftArm.FrameNumber)
					{
						if (++lara->LeftArm.FrameNumber == 110)
							lara->LeftArm.FrameNumber = 0;
					}
				}
				else
				{
					lara->LeftArm.FrameNumber = 95;
					lara->Flare.ControlLeft = true;
				}
			}
			else
				lara->Flare.ControlLeft = false;

			DoFlareInHand(laraItem, lara->Flare.Life);
			SetFlareArm(laraItem, lara->LeftArm.FrameNumber);
		}

		break;

	case HandStatus::Busy:
		if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
		{
			if (lara->MeshPtrs[LM_LHAND] == Objects[ID_LARA_FLARE_ANIM].meshIndex + LM_LHAND)
			{
				lara->Flare.ControlLeft = (lara->Vehicle != NO_ITEM || CheckForHoldingState((LaraState)laraItem->Animation.ActiveState));
				DoFlareInHand(laraItem, lara->Flare.Life);
				SetFlareArm(laraItem, lara->LeftArm.FrameNumber);
			}
		}

		break;
	}
}

Ammo& GetAmmo(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	return lara->Weapons[(int)weaponType].Ammo[(int)lara->Weapons[(int)weaponType].SelectedAmmo];
}

void InitialiseNewWeapon(ITEM_INFO* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	lara->LeftArm.FrameNumber = 0;
	lara->RightArm.FrameNumber = 0;
	lara->LeftArm.Rotation.Zero;
	lara->RightArm.Rotation.Zero;
	lara->TargetEntity = nullptr;
	lara->LeftArm.Locked = false;
	lara->RightArm.Locked = false;
	lara->LeftArm.FlashGun = 0;
	lara->RightArm.FlashGun = 0;

	switch (lara->Control.Weapon.GunType)
	{
	case LaraWeaponType::Pistol:
	case LaraWeaponType::Uzi:
		lara->RightArm.FrameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		lara->LeftArm.FrameBase = Objects[ID_PISTOLS_ANIM].frameBase;

		if (lara->Control.HandStatus != HandStatus::Free)
			DrawPistolMeshes(laraItem, lara->Control.Weapon.GunType);

		break;

	case LaraWeaponType::Shotgun:
	case LaraWeaponType::Revolver:
	case LaraWeaponType::HK:
	case LaraWeaponType::GrenadeLauncher:
	case LaraWeaponType::HarpoonGun:
	case LaraWeaponType::RocketLauncher:
		lara->RightArm.FrameBase = Objects[WeaponObject(lara->Control.Weapon.GunType)].frameBase;
		lara->LeftArm.FrameBase = Objects[WeaponObject(lara->Control.Weapon.GunType)].frameBase;

		if (lara->Control.HandStatus != HandStatus::Free)
			DrawShotgunMeshes(laraItem, lara->Control.Weapon.GunType);

		break;

	case LaraWeaponType::Flare:
		lara->RightArm.FrameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;
		lara->LeftArm.FrameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;

		if (lara->Control.HandStatus != HandStatus::Free)
			DrawFlareMeshes(laraItem);

		break;

	default:
		lara->RightArm.FrameBase = g_Level.Anims[laraItem->Animation.AnimNumber].framePtr;
		lara->LeftArm.FrameBase = g_Level.Anims[laraItem->Animation.AnimNumber].framePtr;
		break;
	}
}

GAME_OBJECT_ID WeaponObjectMesh(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	switch (weaponType)
	{
	case LaraWeaponType::Revolver:
		return (lara->Weapons[(int)LaraWeaponType::Revolver].HasLasersight == true ? ID_LARA_REVOLVER_LASER : ID_REVOLVER_ANIM);

	case LaraWeaponType::Uzi:
		return ID_UZI_ANIM;

	case LaraWeaponType::Shotgun:
		return ID_SHOTGUN_ANIM;

	case LaraWeaponType::HK:
		return ID_HK_ANIM;

	case LaraWeaponType::Crossbow:
		return (lara->Weapons[(int)LaraWeaponType::Crossbow].HasLasersight == true ? ID_LARA_CROSSBOW_LASER : ID_CROSSBOW_ANIM);
		
	case LaraWeaponType::GrenadeLauncher:
		return ID_GRENADE_ANIM;

	case LaraWeaponType::HarpoonGun:
		return ID_HARPOON_ANIM;

	case LaraWeaponType::RocketLauncher:
		return ID_ROCKET_ANIM;

	default:
		return ID_PISTOLS_ANIM;
	}
}

void HitTarget(ITEM_INFO* laraItem, ITEM_INFO* target, GameVector* hitPos, int damage, int grenade)
{	
	auto* lara = GetLaraInfo(laraItem);

	target->HitStatus = true;

	if (target->Data.is<CreatureInfo>())
		GetCreatureInfo(target)->HurtByLara = true;

	auto* object = &Objects[target->ObjectNumber];

	if (hitPos != nullptr)
	{
		if (object->hitEffect != HIT_NONE)
		{
			switch (object->hitEffect)
			{
			case HIT_BLOOD:
				if (target->ObjectNumber == ID_GOON2 &&
					(target->Animation.ActiveState == 8 || GetRandomControl() & 1) &&
					(lara->Control.Weapon.GunType == LaraWeaponType::Pistol ||
						lara->Control.Weapon.GunType == LaraWeaponType::Shotgun ||
						lara->Control.Weapon.GunType == LaraWeaponType::Uzi))
				{
					// Baddy2 gun hitting sword
					SoundEffect(SFX_TR4_BAD_SWORD_RICO, &target->Pose, 0);
					TriggerRicochetSpark(hitPos, laraItem->Orientation.y, 3, 0);
					return;
				}
				else
					DoBloodSplat(hitPos->x, hitPos->y, hitPos->z, (GetRandomControl() & 3) + 3, target->Orientation.y, target->RoomNumber);

				break;

			case HIT_RICOCHET:
				TriggerRicochetSpark(hitPos, laraItem->Orientation.y, 3, 0);
				break;

			case HIT_SMOKE:
				TriggerRicochetSpark(hitPos, laraItem->Orientation.y, 3, -5);

				if (target->ObjectNumber == ID_ROMAN_GOD1 ||
					target->ObjectNumber == ID_ROMAN_GOD2)
				{
					SoundEffect(SFX_TR5_SWORD_GOD_HITMETAL, &target->Pose, 0);
				}

				break;
			}
		}
	}

	if (!object->undead || grenade ||
		target->HitPoints == NOT_TARGETABLE)
	{
		if (target->HitPoints > 0)
		{
			Statistics.Level.AmmoHits++;

			if (target->HitPoints >= damage)
				target->HitPoints -= damage;
			else
				target->HitPoints = 0;
		}
	}
}

FireWeaponType FireWeapon(LaraWeaponType weaponType, ITEM_INFO* target, ITEM_INFO* src, float* angles)
{
	auto* lara = GetLaraInfo(src);

	Ammo& ammo = GetAmmo(src, weaponType);
	if (ammo.getCount() == 0 && !ammo.hasInfinite())
		return FireWeaponType::NoAmmo;
	if (!ammo.hasInfinite())
		ammo--;

	auto* weapon = &Weapons[(int)weaponType];

	auto muzzleOffset = Vector3Int();
	GetLaraJointPosition(&muzzleOffset, LM_RHAND);

	auto pos = Vector3Int(src->Pose.Position.x, muzzleOffset.y, src->Pose.Position.z);

	// TODO
	auto rotation = EulerAngle(
		angles[1] + (GetRandomControl() - 16384) * weapon->ShotAccuracy / 65536,
		angles[0] + (GetRandomControl() - 16384) * weapon->ShotAccuracy / 65536,
		0
	);

	// Calculate ray from rotation angles
	auto direction = Vector3(
		sin(rotation.y) * cos(rotation.x),
		-sin(rotation.x),
		cos(rotation.y) * cos(rotation.x)
	);
	direction.Normalize();

	auto source = pos.ToVector3();
	auto destination = source + direction * weapon->TargetDist;
	auto ray = Ray(source, direction);

	int num = GetSpheres(target, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
	int best = NO_ITEM;
	float bestDistance = FLT_MAX;
	for (int i = 0; i < num; i++)
	{
		auto sphere = BoundingSphere(Vector3(CreatureSpheres[i].x, CreatureSpheres[i].y, CreatureSpheres[i].z), CreatureSpheres[i].r);
		float distance;
		if (ray.Intersects(sphere, distance))
		{
			if (distance < bestDistance)
			{
				bestDistance = distance;
				best = i;
			}
		}
	}

	lara->Control.Weapon.HasFired = true;
	lara->Control.Weapon.Fired = true;
	
	auto vSrc = GameVector(pos.x, pos.y, pos.z);
	short roomNumber = src->RoomNumber;
	GetFloor(pos.x, pos.y, pos.z, &roomNumber);
	vSrc.roomNumber = roomNumber;

	if (best < 0)
	{
		auto vDest = GameVector(destination.x, destination.y, destination.z);
		GetTargetOnLOS(&vSrc, &vDest, false, true);
		return FireWeaponType::Miss;
	}
	else
	{
		Statistics.Game.AmmoHits++;

		destination = source + direction * bestDistance;

		auto vDest = GameVector(destination.x, destination.y, destination.z);

		// TODO: enable it when the slot is created !
		/*
		if (target->objectNumber == ID_TRIBEBOSS)
		{
			long dx, dy, dz;

			dx = (vDest.x - vSrc.x) >> 5;
			dy = (vDest.y - vSrc.y) >> 5;
			dz = (vDest.z - vSrc.z) >> 5;
			FindClosestShieldPoint(vDest.x - dx, vDest.y - dy, vDest.z - dz, target);
		}
		else if (target->objectNumber == ID_ARMY_WINSTON || target->objectNumber == ID_LONDONBOSS) //Don't want blood on Winston - never get the stains out
		{
			short ricochet_angle;
			target->hitStatus = true; //need to do this to maintain defence state
			target->HitPoints--;
			ricochet_angle = (mGetAngle(lara->pos.Position.z, lara->pos.Position.x, target->pos.Position.z, target->pos.Position.x) >> 4) & 4095;
			TriggerRicochetSparks(&vDest, ricochet_angle, 16, 0);
			SoundEffect(SFX_TR4_LARA_RICOCHET, &target->pos, 0);		// play RICOCHET Sample
		}
		else if (target->objectNumber == ID_SHIVA) //So must be Shiva
		{
			z = target->pos.Position.z - lara_item->pos.Position.z;
			x = target->pos.Position.x - lara_item->pos.Position.x;
			angle = 0x8000 + atan2(z, x) - target->pos.Orientation.y;

			if ((target->ActiveState > 1 && target->ActiveState < 5) && angle < 0x4000 && angle > -0x4000)
			{
				target->hitStatus = true; //need to do this to maintain defence state
				ricochet_angle = (mGetAngle(lara->pos.Position.z, lara->pos.Position.x, target->pos.Position.z, target->pos.Position.x) >> 4) & 4095;
				TriggerRicochetSparks(&vDest, ricochet_angle, 16, 0);
				SoundEffect(SFX_TR4_LARA_RICOCHET, &target->pos, 0); // play RICOCHET Sample
			}
			else //Shiva's not in defence mode or has its back to Lara
				HitTarget(target, &vDest, weapon->damage, 0);
		}
		else
		{*/

			// NOTE: it seems that items for being hit by Lara in the normal way must have GetTargetOnLOS returning false
			// it's really weird but we decided to replicate original behaviour until we'll fully understand what is happening
			// with weapons
			if (!GetTargetOnLOS(&vSrc, &vDest, false, true))
				HitTarget(src, target, &vDest, weapon->Damage, false);
		//}
		
		return FireWeaponType::PossibleHit;
	}
}

void FindTargetPoint(ITEM_INFO* item, GameVector* target)
{
	auto* bounds = (BOUNDING_BOX*)GetBestFrame(item);
	int x = (int)(bounds->X1 + bounds->X2) / 2;
	int y = (int) bounds->Y1 + (bounds->Y2 - bounds->Y1) / 3;
	int z = (int)(bounds->Z1 + bounds->Z2) / 2;

	float c = cos(item->Orientation.y);
	float s = sin(item->Orientation.y);

	target->x = item->Pose.Position.x + c * x + s * z;
	target->y = item->Pose.Position.y + y;
	target->z = item->Pose.Position.z + c * z - s * x;
	target->roomNumber = item->RoomNumber;
}

void LaraTargetInfo(ITEM_INFO* laraItem, WeaponInfo* weaponInfo)
{
	auto* lara = GetLaraInfo(laraItem);

	if (lara->TargetEntity == nullptr)
	{
		lara->RightArm.Locked = false;
		lara->LeftArm.Locked = false;
		lara->TargetArmAngles[1] = 0;
		lara->TargetArmAngles[0] = 0;
		return;
	}

	float angles[2];

	auto muzzleOffset = Vector3Int();
	GetLaraJointPosition(&muzzleOffset, LM_RHAND);

	auto src = GameVector(
		laraItem->Pose.Position.x,
		muzzleOffset.y,
		laraItem->Pose.Position.z,
		laraItem->RoomNumber
	);
	
	auto targetPoint = GameVector();
	FindTargetPoint(lara->TargetEntity, &targetPoint);
	phd_GetVectorAngles(targetPoint.x - src.x, targetPoint.y - src.y, targetPoint.z - src.z, angles);

	angles[0] -= laraItem->Orientation.y;
	angles[1] -= laraItem->Orientation.x;

	if (LOS(&src, &targetPoint))
	{
		if (angles[0] >= weaponInfo->LockAngles[0] &&
			angles[0] <= weaponInfo->LockAngles[1] &&
			angles[1] >= weaponInfo->LockAngles[2] &&
			angles[1] <= weaponInfo->LockAngles[3])
		{
			lara->RightArm.Locked = true;
			lara->LeftArm.Locked = true;
		}
		else
		{
			if (lara->LeftArm.Locked)
			{
				if (angles[0] < weaponInfo->LeftAngles[0] ||
					angles[0] > weaponInfo->LeftAngles[1] ||
					angles[1] < weaponInfo->LeftAngles[2] ||
					angles[1] > weaponInfo->LeftAngles[3])
					lara->LeftArm.Locked = false;
			}

			if (lara->RightArm.Locked)
			{
				if (angles[0] < weaponInfo->RightAngles[0] ||
					angles[0] > weaponInfo->RightAngles[1] ||
					angles[1] < weaponInfo->RightAngles[2] ||
					angles[1] > weaponInfo->RightAngles[3])
					lara->RightArm.Locked = false;
			}
		}
	}
	else
	{
		lara->RightArm.Locked = false;
		lara->LeftArm.Locked = false;
	}

	lara->TargetArmAngles[0] = angles[0];
	lara->TargetArmAngles[1] = angles[1];
}

void LaraGetNewTarget(ITEM_INFO* laraItem, WeaponInfo* weaponInfo)
{
	auto* lara = GetLaraInfo(laraItem);

	if (BinocularRange)
	{
		lara->TargetEntity = nullptr;
		return;
	}

	auto muzzleOffset = Vector3Int();
	GetLaraJointPosition(&muzzleOffset, LM_RHAND);

	auto src = GameVector(
		laraItem->Pose.Position.x,
		muzzleOffset.y,
		laraItem->Pose.Position.z,
		laraItem->RoomNumber);

	ITEM_INFO* bestItem = NULL;
	short bestYrot = MAXSHORT;
	int bestDistance = MAXINT;
	int maxDistance = weaponInfo->TargetDist;
	int targets = 0;
	for (int slot = 0; slot < ActiveCreatures.size(); ++slot)
	{
		if (ActiveCreatures[slot]->ItemNumber != NO_ITEM)
		{
			auto* item = &g_Level.Items[ActiveCreatures[slot]->ItemNumber];
			if (item->HitPoints > 0)
			{
				int x = item->Pose.Position.x - src.x;
				int y = item->Pose.Position.y - src.y;
				int z = item->Pose.Position.z - src.z;
				if (abs(x) <= maxDistance &&
					abs(y) <= maxDistance &&
					abs(z) <= maxDistance)
				{
					int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);
					if (distance < pow(maxDistance, 2))
					{
						auto target = GameVector();
						FindTargetPoint(item, &target);
						if (LOS(&src, &target))
						{
							float angle[2];
							phd_GetVectorAngles(target.x - src.x, target.y - src.y, target.z - src.z, angle);
							angle[0] -= laraItem->Orientation.y + lara->ExtraTorsoRot.y;
							angle[1] -= laraItem->Orientation.x + lara->ExtraTorsoRot.x;

							if (angle[0] >= weaponInfo->LockAngles[0] && angle[0] <= weaponInfo->LockAngles[1] && angle[1] >= weaponInfo->LockAngles[2] && angle[1] <= weaponInfo->LockAngles[3])
							{
								TargetList[targets] = item;
								++targets;
								if (abs(angle[0]) < bestYrot + EulerAngle::DegToRad(15.0f) && distance < bestDistance)
								{
									bestDistance = distance;
									bestYrot = abs(angle[0]);
									bestItem = item;
								}
							}
						}
					}
				}
			}
		}
	}

	TargetList[targets] = NULL;
	if (!TargetList[0])
		lara->TargetEntity = NULL;
	else
	{
		for (int slot = 0; slot < MAX_TARGETS; ++slot)
		{
			if (!TargetList[slot])
				lara->TargetEntity = NULL;

			if (TargetList[slot] == lara->TargetEntity)
				break;
		}

		if (lara->Control.HandStatus != HandStatus::Free || TrInput & IN_LOOKSWITCH)
		{
			if (!lara->TargetEntity)
			{
				lara->TargetEntity = bestItem;
				LastTargets[0] = NULL;
			}
			else if (TrInput & IN_LOOKSWITCH)
			{
				lara->TargetEntity = NULL;
				bool flag = true;

				for (int match = 0; match < MAX_TARGETS && TargetList[match]; ++match)
				{
					bool loop = false;
					for (int slot = 0; slot < MAX_TARGETS && LastTargets[slot]; ++slot)
					{
						if (LastTargets[slot] == TargetList[match])
						{
							loop = true;
							break;
						}
					}

					if (!loop)
					{
						lara->TargetEntity = TargetList[match];
						if (lara->TargetEntity)
							flag = false;

						break;
					}
				}

				if (flag)
				{
					lara->TargetEntity = bestItem;
					LastTargets[0] = NULL;
				}
			}
		}
	}

	if (lara->TargetEntity != LastTargets[0])
	{
		for (int slot = 7; slot > 0; --slot)
			LastTargets[slot] = LastTargets[slot - 1];
		
		LastTargets[0] = lara->TargetEntity;
	}

	LaraTargetInfo(laraItem, weaponInfo);
}

HolsterSlot HolsterSlotForWeapon(LaraWeaponType weaponType)
{
	switch(weaponType)
	{
		case LaraWeaponType::Pistol:
			return HolsterSlot::Pistols;

		case LaraWeaponType::Uzi:
			return HolsterSlot::Uzis;

		case LaraWeaponType::Revolver:
			return HolsterSlot::Revolver;

		case LaraWeaponType::Shotgun:
			return HolsterSlot::Shotgun;

		case LaraWeaponType::HK:
			return HolsterSlot::HK;

		case LaraWeaponType::HarpoonGun:
			return HolsterSlot::Harpoon;

		case LaraWeaponType::Crossbow:
			return HolsterSlot::Crowssbow;

		case LaraWeaponType::GrenadeLauncher:
			return HolsterSlot::GrenadeLauncher;

		case LaraWeaponType::RocketLauncher:
			return HolsterSlot::RocketLauncher;

		default:
			return HolsterSlot::Empty;
	}
}
