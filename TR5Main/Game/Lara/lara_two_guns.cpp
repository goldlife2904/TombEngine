#include "framework.h"
#include "Game/Lara/lara_two_guns.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/savegame.h"
#include "Sound/sound.h"
#include "Specific/prng.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/input.h"

using namespace TEN::Math::Random;

struct PistolDef
{
	short ObjectNumber;
	int Draw1Anim2;
	int Draw1Anim;
	int Draw2Anim;
	int RecoilAnim;
};

PistolDef PistolsTable[4] =
{
	{ ID_LARA, 0, 0, 0, 0 },
	{ ID_PISTOLS_ANIM, 4, 5, 13, 24 },
	{ ID_REVOLVER_ANIM , 7, 8, 15, 29 },
	{ ID_UZI_ANIM, 4, 5, 13, 24 }
};

void AnimatePistols(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* weapon = &Weapons[(int)weaponType];
	auto* p = &PistolsTable[(int)lara->Control.Weapon.GunType];

	int soundPlayed = false;
	float angleLeft[2], angleRight[2];

	if (laraItem->MeshBits)
	{
		if (SmokeCountL)
		{
			Vector3Int pos;

			switch (SmokeWeapon)
			{
			case LaraWeaponType::Pistol:
				pos = Vector3Int(4, 128, 40);
				break;

			case LaraWeaponType::Revolver:
				pos = Vector3Int(16, 160, 56);
				break;

			case LaraWeaponType::Uzi:
				pos = Vector3Int(8, 140, 48);
				break;
			}

			GetLaraJointPosition(&pos, LM_LHAND);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountL);
		}

		if (SmokeCountR)
		{
			Vector3Int pos;
			switch (SmokeWeapon)
			{
			case LaraWeaponType::Pistol:
				pos = Vector3Int(-16, 128, 40);
				break;

			case LaraWeaponType::Revolver:
				pos = Vector3Int(32, 160, 56);
				break;

			case LaraWeaponType::Uzi:
				pos = Vector3Int(-16, 140, 48);
				break;
			}

			GetLaraJointPosition(&pos, LM_RHAND);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountR);
		}
	}

	// Shooting action for right arm.
	int frameRight = lara->RightArm.FrameNumber;  // frame number of DRAW_END?
	if ((TrInput & IN_ACTION && !lara->TargetEntity) || lara->RightArm.Locked)
	{
		// POINT ARMS FORWARD
		// at or beyond (2) DRAW_END start frame AND before (0) SHOOT_START end frame...
		if ((frameRight >= 0) && (frameRight < p->Draw1Anim2))
		{
			// ...increment toward (0) SHOOT_START end frame
			frameRight++;
		}
		// at (0) SHOOT_START end frame
		else if (frameRight == p->Draw1Anim2)
		{
			// actually shoot, bang bang
			if (TrInput & IN_ACTION)
			{
				if (weaponType != LaraWeaponType::Revolver)
				{
					angleRight[0] = lara->RightArm.Rotation.y + laraItem->Pose.Orientation.GetY();
					angleRight[1] = lara->RightArm.Rotation.x;

					if (FireWeapon(weaponType, lara->TargetEntity, laraItem, angleRight) != FireWeaponType::NoAmmo)
					{
						SmokeCountR = 28;
						SmokeWeapon = weaponType;
						TriggerGunShell(1, ID_GUNSHELL, weaponType); // Right Hand

						lara->RightArm.FlashGun = weapon->FlashTime;

						SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose, 0, 0.9f, 0.5f);
						SoundEffect(weapon->SampleNum, &laraItem->Pose, 0);
						soundPlayed = true;

						if (weaponType == LaraWeaponType::Uzi)
							lara->Control.Weapon.UziRight = true;

						Statistics.Game.AmmoUsed++;
					}
				}

				// go to (3) SHOOT_CONTINUE start frame
				frameRight = p->RecoilAnim;
			}
			else if (lara->Control.Weapon.UziRight)
			{
				SoundEffect(weapon->SampleNum + 1, &laraItem->Pose, 0);
				lara->Control.Weapon.UziRight = false;
			}
		}
		// at or beyond (3) SHOOT_CONTINUE start frame
		else if (frameRight >= p->RecoilAnim)
		{
			if (weaponType == LaraWeaponType::Uzi)
			{
				SoundEffect(weapon->SampleNum, &laraItem->Pose, 0);
				lara->Control.Weapon.UziRight = true;
			}

			// increment toward (3) SHOOT_CONTINUE end frame (finish recoil before allowing to shoot again)
			frameRight++;

			// at (3) SHOOT_CONTINUE end frame, go to (0) START_SHOOT end frame
			if (frameRight == (p->RecoilAnim + weapon->RecoilFrame))
				frameRight = p->Draw1Anim2;
		}
	}
	// HAS LET GO OF ACTION
	else
	{
		// BUGFIX: rapid-fire no more. -Sezz
		// let (3) SHOOT_CONTINUE finish
		if ((frameRight >= p->RecoilAnim) && (frameRight < p->RecoilAnim + weapon->RecoilFrame))
			frameRight++;
		// at (3) SHOOT_CONTINUE end frame, go to (0) START_SHOOT end frame
		if (frameRight == (p->RecoilAnim + weapon->RecoilFrame))
			frameRight = p->Draw1Anim2;
		// go back to ready stance
		else if ((frameRight > 0) && (frameRight <= p->Draw1Anim2))
			frameRight--;

		// OLD:
		//if (frameRight >= p->recoilAnim)
		//	frameRight = p->draw1Anim2;
		//else if ((frameRight > 0) && (frameRight <= p->draw1Anim2))
		//	frameRight--;

		if (lara->Control.Weapon.UziRight)
		{
			SoundEffect(weapon->SampleNum + 1, &laraItem->Pose, 0);
			lara->Control.Weapon.UziRight = false;
		}
	}

	SetArmInfo(laraItem, &lara->RightArm, frameRight);

	// Shooting for left arm.
	int frameLeft = lara->LeftArm.FrameNumber;
	if ((TrInput & IN_ACTION && !lara->TargetEntity) || lara->LeftArm.Locked)
	{
		if ((frameLeft >= 0) && (frameLeft < p->Draw1Anim2))
			frameLeft++;
		else if (frameLeft == p->Draw1Anim2)
		{
			if (TrInput & IN_ACTION)
			{
				angleLeft[0] = lara->LeftArm.Rotation.y + laraItem->Pose.Orientation.GetY();
				angleLeft[1] = lara->LeftArm.Rotation.x;

				if (FireWeapon(weaponType, lara->TargetEntity, laraItem, angleLeft) != FireWeaponType::NoAmmo)
				{
					if (weaponType == LaraWeaponType::Revolver)
					{
						SmokeCountR = 28;
						SmokeWeapon = LaraWeaponType::Revolver;
						lara->RightArm.FlashGun = weapon->FlashTime;
					}
					else
					{
						SmokeCountL = 28;
						SmokeWeapon = weaponType;
						TriggerGunShell(0, ID_GUNSHELL, weaponType); // left hand
						lara->LeftArm.FlashGun = weapon->FlashTime;
					}

					if (!soundPlayed)
					{
						SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose, 0, 0.9f, 0.5f);
						SoundEffect(weapon->SampleNum, &laraItem->Pose, 0);
					}

					if (weaponType == LaraWeaponType::Uzi)
						lara->Control.Weapon.UziLeft = true;

					Statistics.Game.AmmoUsed++;
				}

				frameLeft = p->RecoilAnim;
			}
			else if (lara->Control.Weapon.UziLeft)
			{
				SoundEffect(weapon->SampleNum + 1, &laraItem->Pose, 0);
				lara->Control.Weapon.UziLeft = false;
			}
		}
		else if (frameLeft >= p->RecoilAnim)
		{
			if (weaponType == LaraWeaponType::Uzi)
			{
				SoundEffect(weapon->SampleNum, &laraItem->Pose, 0);
				lara->Control.Weapon.UziLeft = true;
			}

			frameLeft++;

			if (frameLeft == (p->RecoilAnim + weapon->RecoilFrame))
				frameLeft = p->Draw1Anim2;
		}
	}
	else       																// Havent GOT a LOCK ON..
	{
		if ((frameLeft >= p->RecoilAnim) && (frameLeft < p->RecoilAnim + weapon->RecoilFrame))
			frameLeft++;
		if (frameLeft == (p->RecoilAnim + weapon->RecoilFrame))
			frameLeft = p->Draw1Anim2;
		else if ((frameLeft > 0) && (frameLeft <= p->Draw1Anim2))
			frameLeft--;

		// OLD:
		//if (frameLeft >= p->recoilAnim) 									// If Gun is Recoiling Stop it now...
		//	frameLeft = p->draw1Anim2;
		//else if (frameLeft > 0 && frameLeft <= p->draw1Anim2)
		//	frameLeft--;													// UnLock ARM

		if (lara->Control.Weapon.UziLeft)
		{
			SoundEffect(weapon->SampleNum + 1, &laraItem->Pose, 0);
			lara->Control.Weapon.UziLeft = false;
		}
	}

	SetArmInfo(laraItem, &lara->LeftArm, frameLeft);
}

void PistolHandler(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* weapon = &Weapons[(int)weaponType];

	LaraGetNewTarget(laraItem, weapon);
	if (TrInput & IN_ACTION)
		LaraTargetInfo(laraItem, weapon);

	AimWeapon(laraItem, weapon, &lara->LeftArm);
	AimWeapon(laraItem, weapon, &lara->RightArm);

	if (lara->LeftArm.Locked && !lara->RightArm.Locked)
	{
		lara->ExtraTorsoRot = lara->LeftArm.Rotation / 2;

		if (Camera.oldType != CameraType::Look)
			lara->ExtraHeadRot = lara->ExtraTorsoRot;
	}
	else if (!lara->LeftArm.Locked && lara->RightArm.Locked)
	{
		lara->ExtraTorsoRot = lara->RightArm.Rotation / 2;

		if (Camera.oldType != CameraType::Look)
			lara->ExtraHeadRot = lara->ExtraTorsoRot;
	}
	else if (lara->LeftArm.Locked && lara->RightArm.Locked)
	{
		lara->ExtraTorsoRot = (lara->LeftArm.Rotation + lara->RightArm.Rotation) / 4;

		if (Camera.oldType != CameraType::Look)
			lara->ExtraHeadRot = lara->ExtraTorsoRot;
	}

	AnimatePistols(laraItem, weaponType);
	
	if (lara->LeftArm.FlashGun || lara->RightArm.FlashGun)
	{
		auto pos = Vector3Int(
			(byte)GetRandomControl() - 128,
			(GetRandomControl() & 0x7F) - 63,
			(byte)GetRandomControl() - 128
		);
		GetLaraJointPosition(&pos, lara->LeftArm.FlashGun != 0 ? LM_LHAND : LM_RHAND);

		TriggerDynamicLight(pos.x+GenerateFloat(-128,128), pos.y + GenerateFloat(-128, 128), pos.z + GenerateFloat(-128, 128), GenerateFloat(8,11), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 128, GetRandomControl() & 0x3F);
	}
}

void ReadyPistols(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	lara->Control.HandStatus = HandStatus::WeaponReady;
	lara->LeftArm.Rotation = EulerAngles::Zero;
	lara->RightArm.Rotation = EulerAngles::Zero;
	lara->LeftArm.FrameNumber = 0;
	lara->RightArm.FrameNumber = 0;
	lara->TargetEntity = nullptr;
	lara->LeftArm.Locked = false;
	lara->RightArm.Locked = false;
	lara->LeftArm.FrameBase = Objects[WeaponObject(weaponType)].frameBase;
	lara->RightArm.FrameBase = Objects[WeaponObject(weaponType)].frameBase;
}

void DrawPistols(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* p = &PistolsTable[(int)lara->Control.Weapon.GunType];

	int frame = lara->LeftArm.FrameNumber + 1;

	if (frame < p->Draw1Anim || frame > p->RecoilAnim - 1)
		frame = p->Draw1Anim;
	else if (frame == p->Draw2Anim)
	{
		DrawPistolMeshes(laraItem, weaponType);
		SoundEffect(SFX_TR4_LARA_DRAW, &laraItem->Pose, 0);
	}
	else if (frame == p->RecoilAnim - 1)
	{
		ReadyPistols(laraItem, weaponType);
		frame = 0;
	}

	SetArmInfo(laraItem, &lara->RightArm, frame);
	SetArmInfo(laraItem, &lara->LeftArm, frame);
}

void UndrawPistols(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* weapon = &Weapons[(int)weaponType];
	auto* p = &PistolsTable[(int)lara->Control.Weapon.GunType];

	int frameLeft = lara->LeftArm.FrameNumber;

	// Finish recoil anim before reholstering weapon.
	if ((frameLeft >= p->RecoilAnim) && (frameLeft < p->RecoilAnim + weapon->RecoilFrame))
		frameLeft++;
	if (frameLeft == (p->RecoilAnim + weapon->RecoilFrame))
		frameLeft = p->Draw1Anim2;

	// OLD:
	/*if (frameLeft >= p->recoilAnim)
	{
		frameLeft = p->draw1Anim2;
	}*/
	else if (frameLeft > 0 && frameLeft < p->Draw1Anim)
	{
		lara->LeftArm.Rotation -= lara->LeftArm.Rotation / frameLeft;
		frameLeft--;
	}
	else if (frameLeft == 0)
	{
		lara->LeftArm.Rotation = EulerAngles::Zero;
		frameLeft = p->RecoilAnim - 1;
	}
	else if (frameLeft > p->Draw1Anim && (frameLeft < p->RecoilAnim))
	{
		frameLeft--;

		if (frameLeft == p->Draw2Anim - 1)
		{
			UndrawPistolMeshLeft(laraItem, weaponType);
			SoundEffect(SFX_TR4_LARA_HOLSTER, &laraItem->Pose, 0);
		}
	}

	SetArmInfo(laraItem, &lara->LeftArm, frameLeft);

	int frameRight = lara->RightArm.FrameNumber;

	if ((frameRight >= p->RecoilAnim) && (frameRight < p->RecoilAnim + weapon->RecoilFrame))
		frameRight++;
	if (frameRight == (p->RecoilAnim + weapon->RecoilFrame))
		frameRight = p->Draw1Anim2;

	// OLD:
	/*if (frameRight >= p->recoilAnim)
	{
		frameRight = p->draw1Anim2;
	}*/
	else if (frameRight > 0 && frameRight < p->Draw1Anim)
	{
		lara->RightArm.Rotation -= lara->RightArm.Rotation / frameRight;
		frameRight--;
	}
	else if (frameRight == 0)
	{
		lara->RightArm.Rotation = EulerAngles::Zero;
		frameRight = p->RecoilAnim - 1;
	}
	else if (frameRight > p->Draw1Anim && (frameRight < p->RecoilAnim))
	{
		frameRight--;

		if (frameRight == p->Draw2Anim - 1)
		{
			UndrawPistolMeshRight(laraItem, weaponType);
			SoundEffect(SFX_TR4_LARA_HOLSTER, &laraItem->Pose, 0);
		}
	}

	SetArmInfo(laraItem, &lara->RightArm, frameRight);

	if (frameLeft == p->Draw1Anim && frameRight == p->Draw1Anim)
	{
		lara->Control.HandStatus = HandStatus::Free;
		lara->LeftArm.FrameNumber = 0;
		lara->RightArm.FrameNumber = 0;
		lara->TargetEntity = NULL;
		lara->RightArm.Locked = false;
		lara->LeftArm.Locked = false;
	}

	if (!(TrInput & IN_LOOK))
	{
		lara->ExtraHeadRot = (lara->LeftArm.Rotation + lara->RightArm.Rotation) / 4;
		lara->ExtraTorsoRot = (lara->LeftArm.Rotation + lara->RightArm.Rotation) / 4;
	}
}

void SetArmInfo(ITEM_INFO* laraItem, ArmInfo* arm, int frame)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* p = &PistolsTable[(int)lara->Control.Weapon.GunType];

	int animBase = Objects[(int)p->ObjectNumber].animIndex;

	if (frame < p->Draw1Anim)
		arm->AnimNumber = animBase;
	else if (frame < p->Draw2Anim)
		arm->AnimNumber = animBase + 1;
	else if (frame < p->RecoilAnim)
		arm->AnimNumber = animBase + 2;
	else
		arm->AnimNumber = animBase + 3;

	arm->FrameNumber = frame;
	arm->FrameBase = g_Level.Anims[arm->AnimNumber].framePtr;
}

void DrawPistolMeshes(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	if (weaponType != LaraWeaponType::Revolver)
		lara->Control.Weapon.HolsterInfo.LeftHolster = HolsterSlot::Empty;

	lara->Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Empty;

	lara->MeshPtrs[LM_RHAND] = Objects[WeaponObjectMesh(laraItem, weaponType)].meshIndex + LM_RHAND;
	if (weaponType != LaraWeaponType::Revolver)
		lara->MeshPtrs[LM_LHAND] = Objects[WeaponObjectMesh(laraItem, weaponType)].meshIndex + LM_LHAND;
}

void UndrawPistolMeshRight(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	lara->MeshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
	lara->Control.Weapon.HolsterInfo.RightHolster = HolsterSlotForWeapon(weaponType);
}

void UndrawPistolMeshLeft(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	if (weaponType != LaraWeaponType::Revolver)
	{
		lara->MeshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
		lara->Control.Weapon.HolsterInfo.LeftHolster = HolsterSlotForWeapon(weaponType);
	}
}
