#include "framework.h"
#include "Objects/Generic/Switches/jump_switch.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Specific/level.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"

namespace TEN::Entities::Switches
{
	OBJECT_COLLISION_BOUNDS JumpSwitchBounds =  
	{
		-128, 128,
		-256, 256,
		384, 512,
		EulerAngle::DegToRad(-10.0f), EulerAngle::DegToRad(10.0f),
		EulerAngle::DegToRad(-30.0f), EulerAngle::DegToRad(30.0f),
		EulerAngle::DegToRad(-10.0f), EulerAngle::DegToRad(10.0f)
	};

	Vector3Int JumpSwitchPos = { 0, -208, 256 };  

	void JumpSwitchCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if (TrInput & IN_ACTION &&
			(laraItem->Animation.ActiveState == LS_REACH || laraItem->Animation.ActiveState == LS_JUMP_UP) &&
			(laraItem->Status || laraItem->Animation.Airborne) &&
			laraItem->Animation.VerticalVelocity > 0 &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			!switchItem->Animation.ActiveState)
		{
			if (TestLaraPosition(&JumpSwitchBounds, switchItem, laraItem))
			{
				AlignLaraPosition(&JumpSwitchPos, switchItem, laraItem);

				laraItem->Animation.ActiveState = LS_SWITCH_DOWN;
				laraItem->Animation.AnimNumber = LA_JUMPSWITCH_PULL;
				laraItem->Animation.VerticalVelocity = 0;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.Airborne = false;
				laraInfo->Control.HandStatus = HandStatus::Busy;
				switchItem->Animation.TargetState = SWITCH_ON;
				switchItem->Status = ITEM_ACTIVE;

				AddActiveItem(itemNumber);
			}
		}
	}
}
