#include "framework.h"
#include "Game/Lara/lara_helpers.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_tests.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#include "Objects/TR2/Vehicles/skidoo.h"
#include "Objects/TR3/Vehicles/big_gun.h"
#include "Objects/TR3/Vehicles/kayak.h"
#include "Objects/TR3/Vehicles/minecart.h"
#include "Objects/TR3/Vehicles/quad_bike.h"
#include "Objects/TR3/Vehicles/upv.h"
#include "Objects/TR4/Vehicles/jeep.h"
#include "Objects/TR4/Vehicles/motorbike.h"

using namespace TEN::Renderer;
using namespace TEN::Input;

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

void HandleLaraMovementParameters(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	// Update AFK pose timer.
	if (lara->Control.Count.Pose < LARA_POSE_TIME && TestLaraPose(item, coll) &&
		!(TrInput & (IN_WAKE | IN_LOOK)) &&
		g_GameFlow->HasAFKPose())
	{
		lara->Control.Count.Pose++;
	}
	else
		lara->Control.Count.Pose = 0;

	// Reset running jump timer.
	if (!IsRunJumpCountableState((LaraState)item->Animation.ActiveState))
		lara->Control.Count.Run = 0;

	// Reset running jump action queue.
	if (!IsRunJumpQueueableState((LaraState)item->Animation.ActiveState))
		lara->Control.RunJumpQueued = false;

	// Reset lean.
	if (!lara->Control.IsMoving || (lara->Control.IsMoving && !(TrInput & (IN_LEFT | IN_RIGHT))))
		ResetLaraLean(item, 0.15f);

	// Reset crawl flex.
	if (!(TrInput & IN_LOOK) && coll->Setup.Height > LARA_HEIGHT - LARA_HEADROOM &&	// HACK
		(!item->Animation.Velocity || (item->Animation.Velocity && !(TrInput & (IN_LEFT | IN_RIGHT)))))
	{
		ResetLaraFlex(item, 0.15f);
	}

	// Apply and reset turn rate.
	item->Pose.Orientation.y += lara->Control.TurnRate.y;
	if (!(TrInput & (IN_LEFT | IN_RIGHT)))
		lara->Control.TurnRate.y = 0;
}

bool HandleLaraVehicle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Vehicle != NO_ITEM)
	{
		switch (g_Level.Items[lara->Vehicle].ObjectNumber)
		{
		case ID_QUAD:
			QuadBikeControl(item, coll);
			break;

		case ID_JEEP:
			JeepControl(item);
			break;

		case ID_MOTORBIKE:
			MotorbikeControl(item, coll);
			break;

		case ID_KAYAK:
			KayakControl(item);
			break;

		case ID_SNOWMOBILE:
			SkidooControl(item, coll);
			break;

		case ID_UPV:
			UPVControl(item, coll);
			break;

		case ID_MINECART:
			MinecartControl(item);
			break;

		case ID_BIGGUN:
			BigGunControl(item, coll);
			break;

			// Boats are processed like normal items in loop.
		default:
			LaraGun(item);
		}

		return true;
	}

	return false;
}

// TODO: This approach may cause undesirable artefacts where an object pushes Lara rapidly up/down a slope or a platform rapidly ascends/descends.
// Nobody panic. I have ideas. @Sezz 2022.03.24
void EaseOutLaraHeight(ItemInfo* item, int height)
{
	if (height == NO_HEIGHT)
		return;

	// Translate Lara to new height.
	static constexpr int rate = 50;
	int threshold = std::max(abs(item->Animation.Velocity) * 1.5f, CLICK(0.25f) / 4);
	int sign = std::copysign(1, height);

	if (TestEnvironment(ENV_FLAG_SWAMP, item) && height > 0)
		item->Pose.Position.y += SWAMP_GRAVITY;
	else if (abs(height) > (STEPUP_HEIGHT / 2))		// Outer range.
		item->Pose.Position.y += rate * sign;
	else if (abs(height) <= (STEPUP_HEIGHT / 2) &&	// Inner range.
		abs(height) >= threshold)
	{
		item->Pose.Position.y += std::max<int>(abs(height / 2.75), threshold) * sign;
	}
	else
		item->Pose.Position.y += height;
}

// TODO: Some states can't make the most of this function due to missing step up/down animations.
// Try implementing leg IK as a substitute to make step animations obsolete. @Sezz 2021.10.09
void DoLaraStep(ItemInfo* item, CollisionInfo* coll)
{
	if (!TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		if (TestLaraStepUp(item, coll))
		{
			item->Animation.TargetState = LS_STEP_UP;
			if (GetChange(item, &g_Level.Anims[item->Animation.AnimNumber]))
			{
				item->Pose.Position.y += coll->Middle.Floor;
				return;
			}
		}
		else if (TestLaraStepDown(item, coll))
		{
			item->Animation.TargetState = LS_STEP_DOWN;
			if (GetChange(item, &g_Level.Anims[item->Animation.AnimNumber]))
			{
				item->Pose.Position.y += coll->Middle.Floor;
				return;
			}
		}
	}

	EaseOutLaraHeight(item, coll->Middle.Floor);
}

void DoLaraMonkeyStep(ItemInfo* item, CollisionInfo* coll)
{
	EaseOutLaraHeight(item, coll->Middle.Ceiling);
}

void DoLaraCrawlToHangSnap(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.ForwardAngle = Angle::Normalize(item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f));
	GetCollisionInfo(coll, item);

	SnapItemToLedge(item, coll);
	LaraResetGravityStatus(item, coll);

	// Bridges behave differently.
	if (coll->Middle.Bridge < 0)
	{
		TranslateItem(item, item->Pose.Orientation.GetY(), -LARA_RADIUS_CRAWL);
		item->Pose.Orientation.SetY(item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f));
	}
}

void DoLaraTightropeBalance(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);
	const int factor = ((lara->Control.Tightrope.TimeOnTightrope >> 7) & 0xFF) * 128;

	if (TrInput & IN_LEFT)
		lara->Control.Tightrope.Balance += Angle::DegToRad(1.4f);
	if (TrInput & IN_RIGHT)
		lara->Control.Tightrope.Balance -= Angle::DegToRad(1.4f);

	if (lara->Control.Tightrope.Balance < 0)
	{
		lara->Control.Tightrope.Balance -= factor;
		if (lara->Control.Tightrope.Balance <= Angle::DegToRad(-45.0f))
			lara->Control.Tightrope.Balance = Angle::DegToRad(-45.0f);

	}
	else if (lara->Control.Tightrope.Balance > 0)
	{
		lara->Control.Tightrope.Balance += factor;
		if (lara->Control.Tightrope.Balance >= Angle::DegToRad(45.0f))
			lara->Control.Tightrope.Balance = Angle::DegToRad(45.0f);
	}
	else
		lara->Control.Tightrope.Balance = GetRandomControl() & 1 ? -1 : 1;
}

void DoLaraTightropeLean(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	item->Pose.Orientation.SetZ(lara->Control.Tightrope.Balance / 4);
	lara->ExtraTorsoRot.SetZ(lara->Control.Tightrope.Balance + Angle::DegToRad(180.0f));
}

void DoLaraTightropeBalanceRegen(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.Tightrope.TimeOnTightrope <= 32)
		lara->Control.Tightrope.TimeOnTightrope = 0;
	else
		lara->Control.Tightrope.TimeOnTightrope -= 32;

	if (lara->Control.Tightrope.Balance > 0)
	{
		if (lara->Control.Tightrope.Balance <= Angle::DegToRad(0.75f))
			lara->Control.Tightrope.Balance = 0;
		else
			lara->Control.Tightrope.Balance -= Angle::DegToRad(0.75f);
	}

	if (lara->Control.Tightrope.Balance < 0)
	{
		if (lara->Control.Tightrope.Balance >= Angle::DegToRad(-0.75f))
			lara->Control.Tightrope.Balance = 0;
		else
			lara->Control.Tightrope.Balance += Angle::DegToRad(0.75f);
	}
}

void DoLaraFallDamage(ItemInfo* item)
{
	if (item->Animation.VerticalVelocity >= LARA_DAMAGE_VELOCITY)
	{
		if (item->Animation.VerticalVelocity >= LARA_DEATH_VELOCITY)
			item->HitPoints = 0;
		else USE_FEATURE_IF_CPP20([[likely]])
		{
			float base = item->Animation.VerticalVelocity - (LARA_DAMAGE_VELOCITY - 1);
			item->HitPoints -= LARA_HEALTH_MAX * (pow(base, 2) / 196);
		}

		float rumblePower = ((float)item->Animation.VerticalVelocity / (float)LARA_DEATH_VELOCITY) * 0.7f;
		Rumble(rumblePower, 0.3f);
	}
}

LaraInfo*& GetLaraInfo(ItemInfo* item)
{
	if (item->ObjectNumber == ID_LARA)
		return (LaraInfo*&)item->Data;
	else
	{
		TENLog(std::string("Attempted to fetch LaraInfo data from entity with object ID ") + std::to_string(item->ObjectNumber), LogLevel::Warning);

		auto* firstLaraItem = FindItem(ID_LARA);
		return (LaraInfo*&)firstLaraItem->Data;
	}
}

LaraState GetLaraCornerShimmyState(ItemInfo* item, CollisionInfo* coll)
{
	if (TrInput & IN_LEFT)
	{
		switch (TestLaraHangCorner(item, coll, -90.0f))
		{
		case CornerType::Inner:
			return LS_SHIMMY_INNER_LEFT;

		case CornerType::Outer:
			return LS_SHIMMY_OUTER_LEFT;
		}

		switch (TestLaraHangCorner(item, coll, -45.0f))
		{
		case CornerType::Inner:
			return LS_SHIMMY_45_INNER_LEFT;

		case CornerType::Outer:
			return LS_SHIMMY_45_OUTER_LEFT;
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		switch (TestLaraHangCorner(item, coll, 90.0f))
		{
		case CornerType::Inner:
			return LS_SHIMMY_INNER_RIGHT;

		case CornerType::Outer:
			return LS_SHIMMY_OUTER_RIGHT;
		}

		switch (TestLaraHangCorner(item, coll, 45.0f))
		{
		case CornerType::Inner:
			return LS_SHIMMY_45_INNER_RIGHT;

		case CornerType::Outer:
			return LS_SHIMMY_45_OUTER_RIGHT;
		}
	}

	return (LaraState)-1;
}

float GetLaraSlideDirection(ItemInfo* item, CollisionInfo* coll)
{
	float headingAngle = coll->Setup.ForwardAngle;
	auto probe = GetCollision(item);

	// Ground is flat.
	if (probe.FloorTilt == Vector2::Zero)
		return headingAngle;

	// Get either:
	// a) the surface aspect angle (extended slides), or
	// b) the derived nearest cardinal direction from it (original slides).
	headingAngle = GetSurfaceAspectAngle(probe.FloorTilt.x, probe.FloorTilt.y);
	if (g_GameFlow->HasSlideExtended())
		return headingAngle;
	else
		return (GetQuadrant(headingAngle) * Angle::DegToRad(90.0f));
}

float ModulateLaraTurnRate(float turnRate, float accelRate, float minTurnRate, float maxTurnRate, float axisCoeff, bool invert)
{
	axisCoeff *= invert ? -1 : 1;
	int sign = std::copysign(1, axisCoeff);
	float minTurnRateNormalized = minTurnRate * abs(axisCoeff);
	float maxTurnRateNormalized = maxTurnRate * abs(axisCoeff);

	float newTurnRate = (turnRate + (accelRate * sign)) * sign;
	newTurnRate = std::clamp(newTurnRate, minTurnRateNormalized, maxTurnRateNormalized);
	return (newTurnRate * sign);
}

// TODO: Make these two functions methods of LaraInfo someday. @Sezz 2022.06.26
void ModulateLaraTurnRateX(ItemInfo* item, float accelRate, float minTurnRate, float maxTurnRate, bool invert)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.TurnRate.x = ModulateLaraTurnRate(lara->Control.TurnRate.x, accelRate, minTurnRate, maxTurnRate, AxisMap[InputAxis::MoveVertical], invert);
}

void ModulateLaraTurnRateY(ItemInfo* item, float accelRate, float minTurnRate, float maxTurnRate, bool invert)
{
	auto* lara = GetLaraInfo(item);

	float axisCoeff = AxisMap[InputAxis::MoveHorizontal];
	if (item->Animation.IsAirborne)
	{
		int sign = std::copysign(1, axisCoeff);
		axisCoeff = std::min(1.2f, abs(axisCoeff)) * sign;
	}

	lara->Control.TurnRate.y = ModulateLaraTurnRate(lara->Control.TurnRate.y, accelRate, minTurnRate, maxTurnRate, axisCoeff, invert);
}

void ModulateLaraSwimTurnRates(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & (IN_FORWARD | IN_BACK))
		ModulateLaraTurnRateX(item, LARA_SWIM_TURN_RATE_ACCEL, 0, LARA_MED_TURN_RATE_MAX);

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_MED_FAST_TURN_RATE_MAX);

		// TODO: ModulateLaraLean() doesn't really work here. @Sezz 2022.06.22
		if (TrInput & IN_LEFT)
			item->Pose.Orientation.z -= LARA_LEAN_RATE;
		else if (TrInput & IN_RIGHT)
			item->Pose.Orientation.z += LARA_LEAN_RATE;
	}
}

void ModulateLaraSubsuitSwimTurnRates(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_FORWARD && item->Pose.Orientation.x > -Angle::DegToRad(85.0f))
		lara->Control.Subsuit.DXRot = -Angle::DegToRad(45.0f);
	else if (TrInput & IN_BACK && item->Pose.Orientation.x < Angle::DegToRad(85.0f))
		lara->Control.Subsuit.DXRot = Angle::DegToRad(45.0f);
	else
		lara->Control.Subsuit.DXRot = 0;

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_SUBSUIT_TURN_RATE_ACCEL, 0, LARA_MED_TURN_RATE_MAX);

		if (TrInput & IN_LEFT)
			item->Pose.Orientation.z -= LARA_LEAN_RATE;
		else if (TrInput & IN_RIGHT)
			item->Pose.Orientation.z += LARA_LEAN_RATE;
	}
}

// TODO: Simplify this function. Some members of SubsuitControlData may be unnecessary. @Sezz 2022.06.22
void UpdateLaraSubsuitAngles(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.Subsuit.VerticalVelocity != 0)
	{
		item->Pose.Position.y += lara->Control.Subsuit.VerticalVelocity / 4;
		lara->Control.Subsuit.VerticalVelocity = ceil((15 / 16) * lara->Control.Subsuit.VerticalVelocity - 1);
	}

	lara->Control.Subsuit.Velocity[0] = -4 * item->Animation.VerticalVelocity;
	lara->Control.Subsuit.Velocity[1] = -4 * item->Animation.VerticalVelocity;

	if (lara->Control.Subsuit.XRot >= lara->Control.Subsuit.DXRot)
	{
		if (lara->Control.Subsuit.XRot > lara->Control.Subsuit.DXRot)
		{
			if (lara->Control.Subsuit.XRot > 0 && lara->Control.Subsuit.DXRot < 0)
				lara->Control.Subsuit.XRot = ceil(0.75 * lara->Control.Subsuit.XRot);

			lara->Control.Subsuit.XRot -= Angle::DegToRad(2.0f);
			if (lara->Control.Subsuit.XRot < lara->Control.Subsuit.DXRot)
				lara->Control.Subsuit.XRot = lara->Control.Subsuit.DXRot;
		}
	}
	else
	{
		if (lara->Control.Subsuit.XRot < 0 && lara->Control.Subsuit.DXRot > 0)
			lara->Control.Subsuit.XRot = ceil(0.75 * lara->Control.Subsuit.XRot);

		lara->Control.Subsuit.XRot += Angle::DegToRad(2.0f);
		if (lara->Control.Subsuit.XRot > lara->Control.Subsuit.DXRot)
			lara->Control.Subsuit.XRot = lara->Control.Subsuit.DXRot;
	}

	if (lara->Control.Subsuit.DXRot != 0)
	{
		float rotation = lara->Control.Subsuit.DXRot / 8;
		if (rotation < Angle::DegToRad(-2.0f))
			rotation = Angle::DegToRad(-2.0f);
		else if (rotation > Angle::DegToRad(2.0f))
			rotation = Angle::DegToRad(2.0f);

		item->Pose.Orientation.SetX(item->Pose.Orientation.GetX() + rotation);
	}

	lara->Control.Subsuit.Velocity[0] += abs(lara->Control.Subsuit.XRot / 8);
	lara->Control.Subsuit.Velocity[1] += abs(lara->Control.Subsuit.XRot / 8);

	if (lara->Control.TurnRate.GetY() > 0)
		lara->Control.Subsuit.Velocity[0] += abs(lara->Control.TurnRate.GetY()) * 2;
	else if (lara->Control.TurnRate.GetY() < 0)
		lara->Control.Subsuit.Velocity[1] += abs(lara->Control.TurnRate.GetY()) * 2;

	if (lara->Control.Subsuit.Velocity[0] > SECTOR(1.5f))
		lara->Control.Subsuit.Velocity[0] = SECTOR(1.5f);

	if (lara->Control.Subsuit.Velocity[1] > SECTOR(1.5f))
		lara->Control.Subsuit.Velocity[1] = SECTOR(1.5f);

	if (lara->Control.Subsuit.Velocity[0] != 0 || lara->Control.Subsuit.Velocity[1] != 0)
	{
		auto mul1 = (float)abs(lara->Control.Subsuit.Velocity[0]) / SECTOR(8);
		auto mul2 = (float)abs(lara->Control.Subsuit.Velocity[1]) / SECTOR(8);
		auto vol = ((mul1 + mul2) * 5.0f) + 0.5f;
		SoundEffect(SFX_TR5_VEHICLE_DIVESUIT_ENGINE, &item->Pose, SoundEnvironment::Water, 1.0f + (mul1 + mul2), vol);
	}
}

void ModulateLaraLean(ItemInfo* item, CollisionInfo* coll, float baseRate, float maxAngle)
{
	if (!item->Animation.Velocity)
		return;

	float axisCoeff = AxisMap[InputAxis::MoveHorizontal];
	int sign = copysign(1, axisCoeff);
	float maxAngleNormalized = maxAngle * axisCoeff;

	if (coll->CollisionType == CT_LEFT || coll->CollisionType == CT_RIGHT)
		maxAngleNormalized *= 0.6f;

	item->Pose.Orientation.z += std::min(baseRate, abs(maxAngleNormalized - item->Pose.Orientation.z) / 3) * sign;
}

// TODO: Adapt to the above.
void OldDoLaraLean(ItemInfo* item, CollisionInfo* coll, float maxAngle, float rate)
{
	if (!item->Animation.Velocity && !item->Animation.VerticalVelocity)
		return;

	maxAngle = (coll->CollisionType == CT_LEFT || coll->CollisionType == CT_RIGHT) ? (maxAngle * 0.6f) : maxAngle;
	item->Pose.Orientation.SetZ(Angle::InterpolateConstantEaseOut(item->Pose.Orientation.GetZ(), maxAngle, rate, 0.4f, Angle::DegToRad(0.1f)));
}

void ModulateLaraCrawlFlex(ItemInfo* item, float baseRate, float maxAngle)
{
	auto* lara = GetLaraInfo(item);

	if (!item->Animation.Velocity)
		return;

	// TODO: Adapt this.
	//lara->ExtraTorsoRot.SetZ(Angle::InterpolateConstantEaseOut(lara->ExtraTorsoRot.GetZ(), maxAngle, rate, 0.25f / 2, Angle::DegToRad(0.1f)));

	float axisCoeff = AxisMap[InputAxis::MoveHorizontal];
	int sign = copysign(1, axisCoeff);
	float maxAngleNormalized = maxAngle * axisCoeff;

	if (abs(lara->ExtraTorsoRot.z) < LARA_CRAWL_FLEX_MAX)
		lara->ExtraTorsoRot.z += std::min(baseRate, abs(maxAngleNormalized - lara->ExtraTorsoRot.z) / 6) * sign;

	if (!(TrInput & IN_LOOK) &&
		item->Animation.ActiveState != LS_CRAWL_BACK)
	{
		lara->ExtraHeadRot.z = lara->ExtraTorsoRot.z / 2;
		lara->ExtraHeadRot.y = lara->ExtraHeadRot.z;
	}
}

// TODO: Unused; I will pick this back up later. @Sezz 2022.06.22
void ModulateLaraSlideVelocity(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	constexpr int minVelocity = 50;
	constexpr int maxVelocity = LARA_TERMINAL_VELOCITY;

	if (g_GameFlow->HasSlideExtended())
	{
		auto probe = GetCollision(item);
		float minSlideAngle = Angle::DegToRad(33.75f);
		float steepness = GetSurfaceSteepnessAngle(probe.FloorTilt.x, probe.FloorTilt.y);
		float direction = GetSurfaceAspectAngle(probe.FloorTilt.x, probe.FloorTilt.y);

		float velocityMultiplier = 1 / (float)Angle::DegToRad(33.75f);
		int slideVelocity = std::min<int>(minVelocity + 10 * (steepness * velocityMultiplier), LARA_TERMINAL_VELOCITY);
		//short deltaAngle = abs((short)(direction - item->Pose.Orientation.y));

		g_Renderer.PrintDebugMessage("%d", slideVelocity);

		lara->ExtraVelocity.x += slideVelocity;
		lara->ExtraVelocity.y += slideVelocity * sin(steepness);
	}
	else
		lara->ExtraVelocity.x += minVelocity;
}

void SetLaraJumpDirection(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_FORWARD &&
		TestLaraJumpForward(item, coll))
	{
		lara->Control.JumpDirection = JumpDirection::Forward;
	}
	else if (TrInput & IN_BACK &&
		TestLaraJumpBack(item, coll))
	{
		lara->Control.JumpDirection = JumpDirection::Back;
	}
	else if (TrInput & IN_LEFT &&
		TestLaraJumpLeft(item, coll))
	{
		lara->Control.JumpDirection = JumpDirection::Left;
	}
	else if (TrInput & IN_RIGHT &&
		TestLaraJumpRight(item, coll))
	{
		lara->Control.JumpDirection = JumpDirection::Right;
	}
	else if (TestLaraJumpUp(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		lara->Control.JumpDirection = JumpDirection::Up;
	else
		lara->Control.JumpDirection = JumpDirection::None;
}

// TODO: Add a timeout? Imagine a small, sad rain cloud with the properties of a ceiling following Lara overhead.
// RunJumpQueued will never reset, and when the sad cloud flies away after an indefinite amount of time, Lara will jump. @Sezz 2022.01.22
void SetLaraRunJumpQueue(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	int distance = SECTOR(1);
	auto probe = GetCollision(item, item->Pose.Orientation.GetY(), distance, -coll->Setup.Height);

	if ((TestLaraRunJumpForward(item, coll) ||													// Area close ahead is permissive...
		(probe.Position.Ceiling - y) < -(coll->Setup.Height + (LARA_HEADROOM * 0.8f)) ||		// OR ceiling height far ahead is permissive
		(probe.Position.Floor - y) >= CLICK(0.5f)) &&											// OR there is a drop below far ahead.
		probe.Position.Floor != NO_HEIGHT)
	{
		lara->Control.RunJumpQueued = IsRunJumpQueueableState((LaraState)item->Animation.TargetState);
	}
	else
		lara->Control.RunJumpQueued = false;
}

void SetLaraVault(ItemInfo* item, CollisionInfo* coll, VaultTestResult vaultResult)
{
	auto* lara = GetLaraInfo(item);

	lara->ProjectedFloorHeight = vaultResult.Height;
	lara->Control.HandStatus = vaultResult.SetBusyHands ? HandStatus::Busy : lara->Control.HandStatus;
	lara->Control.TurnRate.SetY();

	if (vaultResult.SnapToLedge)
	{
		SnapItemToLedge(item, coll, 0.2f, false);
		lara->TargetOrientation = EulerAngles(0, coll->NearestLedgeAngle, 0);
	}

	if (vaultResult.SetJumpVelocity)
	{
		int height = lara->ProjectedFloorHeight - item->Pose.Position.y;
		if (height > -CLICK(3.5f))
			height = -CLICK(3.5f);
		else if (height < -CLICK(7.5f))
			height = -CLICK(7.5f);

		lara->Control.CalculatedJumpVelocity = -3 - sqrt(-9600 - 12 * height); // TODO: Find a better formula for this that won't require the above block.
	}
}

void SetContextWaterClimbOut(ItemInfo* item, CollisionInfo* coll, WaterClimbOutTestResult climbOutContext)
{
	auto* lara = GetLaraInfo(item);

	UpdateItemRoom(item, -LARA_HEIGHT / 2);
	SnapItemToLedge(item, coll, 1.7f, false);

	item->Animation.ActiveState = LS_ONWATER_EXIT;
	item->Animation.IsAirborne = false;
	item->Animation.Velocity = 0;
	item->Animation.VerticalVelocity = 0;
	lara->ProjectedFloorHeight = climbOutContext.Height;
	lara->TargetOrientation = EulerAngles(0.0f, coll->NearestLedgeAngle, 0.0f);
	lara->Control.TurnRate.SetY();
	lara->Control.HandStatus = HandStatus::Busy;
	lara->Control.WaterStatus = WaterStatus::Dry;
}

void SetLaraLand(ItemInfo* item, CollisionInfo* coll)
{
	//item->IsAirborne = false; // TODO: Removing this avoids an unusual landing bug Core had worked around in an obscure way. I hope to find a proper solution. @Sezz 2022.02.18
	item->Animation.Velocity = 0;
	item->Animation.VerticalVelocity = 0;

	LaraSnapToHeight(item, coll);
}

void SetLaraFallAnimation(ItemInfo* item)
{
	SetAnimation(item, LA_FALL_START);
	item->Animation.IsAirborne = true;
	item->Animation.VerticalVelocity = 0;
}

void SetLaraFallBackAnimation(ItemInfo* item)
{
	SetAnimation(item, LA_FALL_BACK);
	item->Animation.IsAirborne = true;
	item->Animation.VerticalVelocity = 0;
}

void SetLaraMonkeyFallAnimation(ItemInfo* item)
{
	// HACK: Disallow release during 180 turn action.
	if (item->Animation.ActiveState == LS_MONKEY_TURN_180)
		return;

	SetAnimation(item, LA_MONKEY_TO_FREEFALL);
	SetLaraMonkeyRelease(item);
}

void SetLaraMonkeyRelease(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.IsAirborne = true;
	item->Animation.Velocity = 2;
	item->Animation.VerticalVelocity = 1;
	lara->Control.TurnRate.y = 0;
	lara->Control.HandStatus = HandStatus::Free;
}

// temp
void SetLaraSlideAnimation(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TestEnvironment(ENV_FLAG_SWAMP, item))
		return;

	static float oldAngle = 1;

	if (abs(coll->FloorTilt.x) <= 2 && abs(coll->FloorTilt.y) <= 2)
		return;

	float angle = Angle::DegToRad(0.0f);
	if (coll->FloorTilt.x > 2)
		angle = Angle::DegToRad(-90.0f);
	else if (coll->FloorTilt.x < -2)
		angle = Angle::DegToRad(90.0f);

	if (coll->FloorTilt.y > 2 && coll->FloorTilt.y > abs(coll->FloorTilt.x))
		angle = Angle::DegToRad(180.0f);
	else if (coll->FloorTilt.y < -2 && -coll->FloorTilt.y > abs(coll->FloorTilt.x))
		angle = Angle::DegToRad(0.0f);

	float delta = Angle::Normalize(angle - item->Pose.Orientation.GetY());

	ShiftItem(item, coll);

	if (delta < Angle::DegToRad(-90.0f) || delta > Angle::DegToRad(90.0f))
	{
		if (item->Animation.ActiveState == LS_SLIDE_BACK && oldAngle == angle)
			return;

		SetAnimation(item, LA_SLIDE_BACK_START);
		item->Pose.Orientation.SetY(angle + Angle::DegToRad(180.0f));
	}
	else
	{
		if (item->Animation.ActiveState == LS_SLIDE_FORWARD && oldAngle == angle)
			return;

		SetAnimation(item, LA_SLIDE_FORWARD);
		item->Pose.Orientation.SetY(angle);
	}

	LaraSnapToHeight(item, coll);
	lara->Control.MoveAngle = angle;
	lara->Control.TurnRate.y = 0;
	oldAngle = angle;
}

// TODO: Do it later.
void newSetLaraSlideAnimation(ItemInfo* item, CollisionInfo* coll)
{
	short headinAngle = GetLaraSlideDirection(item, coll);
	short deltaAngle = headinAngle - item->Pose.Orientation.y;

	if (!g_GameFlow->HasSlideExtended())
		item->Pose.Orientation.y = headinAngle;

	// Snap to height upon entering slide.
	if (item->Animation.ActiveState != LS_SLIDE_FORWARD &&
		item->Animation.ActiveState != LS_SLIDE_BACK)
	{
		LaraSnapToHeight(item, coll);
	}

	// Slide forward.
	if (abs(deltaAngle) <= Angle::DegToRad(90.0f))
	{
		if (item->Animation.ActiveState == LS_SLIDE_FORWARD && abs(deltaAngle) <= Angle::DegToRad(180.0f))
			return;

		SetAnimation(item, LA_SLIDE_FORWARD);
	}
	// Slide backward.
	else
	{
		if (item->Animation.ActiveState == LS_SLIDE_BACK && abs((short)(deltaAngle - Angle::DegToRad(180.0f))) <= Angle::DegToRad(-180.0f))
			return;

		SetAnimation(item, LA_SLIDE_BACK_START);
	}
}

void SetLaraHang(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	ResetLaraFlex(item);
	item->Animation.IsAirborne = false;
	item->Animation.Velocity = 0;
	item->Animation.VerticalVelocity = 0;
	lara->Control.HandStatus = HandStatus::Busy;
	lara->ExtraTorsoRot = EulerAngles::Zero;
}

void SetLaraHangReleaseAnimation(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.CanClimbLadder)
	{
		SetAnimation(item, LA_FALL_START);
		item->Pose.Position.y += CLICK(1);
	}
	else
	{
		SetAnimation(item, LA_JUMP_UP, 9);
		item->Pose.Position.y += GetBoundsAccurate(item)->Y2 * 1.8f;
	}

	item->Animation.IsAirborne = true;
	item->Animation.Velocity = 2;
	item->Animation.VerticalVelocity = 1;
	lara->Control.HandStatus = HandStatus::Free;
}

void SetLaraCornerAnimation(ItemInfo* item, CollisionInfo* coll, bool flip)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		SetAnimation(item, LA_FALL_START);
		item->Animation.IsAirborne = true;
		item->Animation.Velocity = 2;
		item->Animation.VerticalVelocity = 1;
		item->Pose.Position.y += CLICK(1);
		item->Pose.Orientation.y += lara->NextCornerPos.Orientation.y / 2;
		lara->Control.HandStatus = HandStatus::Free;
		return;
	}

	if (flip)
	{
		if (lara->Control.IsClimbingLadder)
			SetAnimation(item, LA_LADDER_IDLE);
		else
			SetAnimation(item, LA_HANG_IDLE);

		item->Pose.Position = lara->NextCornerPos.Position;
		item->Pose.Orientation.SetY(lara->NextCornerPos.Orientation.GetY());
		coll->Setup.OldPosition = lara->NextCornerPos.Position;
	}
}

void SetLaraSwimDiveAnimation(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	SetAnimation(item, LA_ONWATER_DIVE);
	item->Animation.TargetState = LS_UNDERWATER_SWIM_FORWARD;
	item->Animation.VerticalVelocity = LARA_SWIM_VELOCITY_MAX * 0.4f;
	item->Pose.Orientation.SetX(Angle::DegToRad(-45.0f));
	lara->Control.WaterStatus = WaterStatus::Underwater;
}

// TODO: Make it less stupid in the future. Do it according to a curve?
void ResetLaraTurnRate(ItemInfo* item, bool divesuit)
{
	auto* lara = GetLaraInfo(item);

	// Reset x axis turn rate.
	if (abs(lara->Control.TurnRate.GetX()) > Angle::DegToRad(2.0f))
		lara->Control.TurnRate.SetX(Angle::InterpolateConstant(lara->Control.TurnRate.GetX(), 0.0f, Angle::DegToRad(2.0f)));
	else
		lara->Control.TurnRate.SetX(Angle::InterpolateConstant(lara->Control.TurnRate.GetX(), 0.0f, Angle::DegToRad(0.5f)));

	// Ease rotation near poles.
	if (item->Pose.Orientation.GetX() >= Angle::DegToRad(80.0f) && lara->Control.TurnRate.GetX() > 0.0f ||
		item->Pose.Orientation.GetX() <= Angle::DegToRad(80.0f) && lara->Control.TurnRate.GetX() < 0.0f)
	{
		int sign = copysign(1, lara->Control.TurnRate.GetX());

		item->Pose.Orientation.SetX(
			item->Pose.Orientation.GetX() + std::min(abs(lara->Control.TurnRate.GetX()), (Angle::DegToRad(90.0f) - abs(item->Pose.Orientation.GetX())) / 3) * sign);
	}
	else
		item->Pose.Orientation.SetX(item->Pose.Orientation.GetX() + lara->Control.TurnRate.GetX());

	// Reset y axis turn rate.
	if (abs(lara->Control.TurnRate.GetY()) > Angle::DegToRad(2.0f) && !divesuit)
		lara->Control.TurnRate.SetY(Angle::InterpolateConstant(lara->Control.TurnRate.GetY(), 0.0f, Angle::DegToRad(2.0f)));
	else
		lara->Control.TurnRate.SetY(Angle::InterpolateConstant(lara->Control.TurnRate.GetY(), 0.0f, Angle::DegToRad(0.5f)));

	item->Pose.Orientation.SetY(item->Pose.Orientation.GetY() + lara->Control.TurnRate.GetY());

	// Nothing uses z axis turn rate at this point; keep it at zero.
	lara->Control.TurnRate.SetZ();
}

void ResetLaraLean(ItemInfo* item, float alpha, bool resetRoll, bool resetPitch)
{
	if (resetPitch)
		item->Pose.Orientation.SetX(Angle::InterpolateLinear(item->Pose.Orientation.GetX(), 0.0f, alpha, Angle::DegToRad(0.1f)));

	if (resetRoll)
		item->Pose.Orientation.SetZ(Angle::InterpolateLinear(item->Pose.Orientation.GetZ(), 0.0f, alpha, Angle::DegToRad(0.1f)));
}

void ResetLaraFlex(ItemInfo* item, float alpha)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraHeadRot.InterpolateLinear(EulerAngles::Zero, alpha, Angle::DegToRad(0.1f));
	lara->ExtraTorsoRot.InterpolateLinear(EulerAngles::Zero, alpha, Angle::DegToRad(0.1f));
}

void RumbleLaraHealthCondition(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints > LARA_HEALTH_CRITICAL && !lara->PoisonPotency)
		return;

	bool doPulse = (GlobalCounter & 0x0F) % 0x0F == 1;
	if (doPulse)
		Rumble(0.2f, 0.1f);
}
