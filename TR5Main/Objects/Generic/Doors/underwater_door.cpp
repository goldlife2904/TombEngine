#include "framework.h"
#include "Objects/Generic/Doors/generic_doors.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/control/lot.h"
#include "Game/gui.h"
#include "Specific/input.h"
#include "Game/pickup/pickup.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/collision/sphere.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara.h"
#include "Specific/trmath.h"
#include "Game/misc.h"
#include "Objects/Generic/Doors/underwater_door.h"
#include "Game/collision/collide_item.h"

namespace TEN::Entities::Doors
{
	Vector3Int UnderwaterDoorPos(-251, -540, -46);

	OBJECT_COLLISION_BOUNDS UnderwaterDoorBounds =
	{
		-256, 256, 
		-1024, 0, 
		-1024, 0, 
		EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f),
		EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f),
		EulerAngle::DegToRad(-80.0f), EulerAngle::DegToRad(80.0f)
	};

	void UnderwaterDoorCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* doorItem = &g_Level.Items[itemNumber];

		if (TrInput & IN_ACTION &&
			laraItem->Animation.ActiveState == LS_UNDERWATER_IDLE &&
			laraInfo->Control.WaterStatus == WaterStatus::Underwater &&
			!(doorItem->Status && doorItem->Animation.Airborne) &&
			laraInfo->Control.HandStatus == HandStatus::Free ||
			laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			laraItem->Orientation.y += EulerAngle::DegToRad(180.0f);

			if (TestLaraPosition(&UnderwaterDoorBounds, doorItem, laraItem))
			{
				if (MoveLaraPosition(&UnderwaterDoorPos, doorItem, laraItem))
				{
					SetAnimation(laraItem, LA_UNDERWATER_DOOR_OPEN);
					laraItem->Animation.VerticalVelocity = 0;
					doorItem->Status = ITEM_ACTIVE;

					AddActiveItem(itemNumber);

					doorItem->Animation.TargetState = LS_RUN_FORWARD;

					AnimateItem(doorItem);

					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;
				}
				else
					laraInfo->InteractedItem = itemNumber;

				laraItem->Orientation.y += EulerAngle::DegToRad(180.0f);
			}
			else
			{
				if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}

				laraItem->Orientation.y += EulerAngle::DegToRad(180.0f);
			}
		}
		else if (doorItem->Status == ITEM_ACTIVE)
			ObjectCollision(itemNumber, laraItem, coll);
	}
}
