#include "framework.h"
#include "Objects/TR1/Entity/tr1_giant_mutant.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#define MUTANT_NEED_TURN ANGLE(45.0f)
#define MUTANT_TURN ANGLE(3.0f)
#define MUTANT_ATTACK_RANGE pow(2600, 2)
#define MUTANT_CLOSE_RANGE pow(2250, 2)
#define MUTANT_ATTACK_1_CHANCE 11000
#define MUTANT_ATTACK_2_CHANCE 22000
#define MUTANT_TLEFT 0x7ff0
#define MUTANT_TRIGHT 0x3ff8000
#define MUTANT_TOUCH (MUTANT_TLEFT | MUTANT_TRIGHT)
#define MUTANT_PART_DAMAGE 250
#define MUTANT_ATTACK_DAMAGE 500
#define MUTANT_TOUCH_DAMAGE 5

enum GiantMutantState
{
	MUTANT_STATE_NONE = 0,
	MUTANT_STATE_IDLE = 1,
	MUTANT_STATE_TURN_LEFT = 2,
	MUTANT_STATE_TURN_RIGHT = 3,
	MUTANT_STATE_ATTACK_1 = 4,
	MUTANT_STATE_ATTACK_2 = 5,
	MUTANT_STATE_ATTACK_3 = 6,
	MUTANT_STATE_FORWARD = 7,
	MUTANT_STATE_SET = 8,
	MUTANT_STATE_FALL = 9,
	MUTANT_STATE_DEATH = 10,
	MUTANT_STATE_KILL = 11
};

// TODO
enum GianMutantAnim
{
	MUTANT_ANIM_DEATH = 13,
};

void GiantMutantControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short head = 0;
	short angle = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != MUTANT_STATE_DEATH)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + MUTANT_ANIM_DEATH;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = MUTANT_STATE_DEATH;
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (AI.ahead)
			head = AI.angle;

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = (short)phd_atan(creature->Target.z - item->Position.zPos, creature->Target.x - item->Position.xPos) - item->Position.yRot;

		if (item->TouchBits)
		{
			LaraItem->HitPoints -= MUTANT_TOUCH_DAMAGE;
			LaraItem->HitStatus = true;
		}

		switch (item->Animation.ActiveState)
		{
		case MUTANT_STATE_SET:
			item->Animation.TargetState = MUTANT_STATE_FALL;
			item->Animation.Airborne = true;
			break;

		case MUTANT_STATE_IDLE:
			if (LaraItem->HitPoints <= 0)
				break;

			creature->Flags = 0;

			if (angle > MUTANT_NEED_TURN)
				item->Animation.TargetState = MUTANT_STATE_TURN_RIGHT;
			else if (angle < -MUTANT_NEED_TURN)
				item->Animation.TargetState = MUTANT_STATE_TURN_LEFT;
			else if (AI.distance < MUTANT_ATTACK_RANGE)
			{
				if (LaraItem->HitPoints <= MUTANT_ATTACK_DAMAGE)
				{
					if (AI.distance < MUTANT_CLOSE_RANGE)
						item->Animation.TargetState = MUTANT_STATE_ATTACK_3;
					else
						item->Animation.TargetState = MUTANT_STATE_FORWARD;
				}
				else if (GetRandomControl() < 0x4000)
					item->Animation.TargetState = MUTANT_STATE_ATTACK_1;
				else
					item->Animation.TargetState = MUTANT_STATE_ATTACK_2;
			}
			else
				item->Animation.TargetState = MUTANT_STATE_FORWARD;

			break;

		case MUTANT_STATE_FORWARD:
			if (angle < -MUTANT_TURN)
				item->Animation.TargetState -= MUTANT_TURN;
			else if (angle > MUTANT_TURN)
				item->Animation.TargetState += MUTANT_TURN;
			else
				item->Animation.TargetState += angle;

			if (angle > MUTANT_NEED_TURN || angle < -MUTANT_NEED_TURN)
				item->Animation.TargetState = MUTANT_STATE_IDLE;
			else if (AI.distance < MUTANT_ATTACK_RANGE)
				item->Animation.TargetState = MUTANT_STATE_IDLE;

			break;

		case MUTANT_STATE_TURN_RIGHT:
			if (!creature->Flags)
				creature->Flags = item->Animation.FrameNumber;
			else if (item->Animation.FrameNumber - creature->Flags > 16 &&
				item->Animation.FrameNumber - creature->Flags < 23)
			{
				item->Position.yRot += ANGLE(14.0f);
			}

			if (angle < MUTANT_NEED_TURN)
				item->Animation.TargetState = MUTANT_STATE_IDLE;

			break;

		case MUTANT_STATE_TURN_LEFT:
			if (!creature->Flags)
				creature->Flags = item->Animation.FrameNumber;
			else if (item->Animation.FrameNumber - creature->Flags > 13 &&
				item->Animation.FrameNumber - creature->Flags < 23)
			{
				item->Position.yRot -= ANGLE(9.0f);
			}

			if (angle > -MUTANT_NEED_TURN)
				item->Animation.TargetState = MUTANT_STATE_IDLE;

			break;

		case MUTANT_STATE_ATTACK_1:
			if (!creature->Flags && item->TouchBits & MUTANT_TRIGHT)
			{
				creature->Flags = 1;

				LaraItem->HitPoints -= MUTANT_ATTACK_DAMAGE;
				LaraItem->HitStatus = true;
			}

			break;

		case MUTANT_STATE_ATTACK_2:
			if (!creature->Flags && item->TouchBits & MUTANT_TOUCH)
			{
				creature->Flags = 1;

				LaraItem->HitPoints -= MUTANT_ATTACK_DAMAGE;
				LaraItem->HitStatus = true;
			}

			break;

		case MUTANT_STATE_ATTACK_3:
			if (item->TouchBits & MUTANT_TRIGHT || LaraItem->HitPoints <= 0)
			{
				item->Animation.TargetState = MUTANT_STATE_KILL;
				Camera.targetDistance = SECTOR(2);
				Camera.flags = CF_FOLLOW_CENTER;

				LaraItem->Animation.AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
				LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
				LaraItem->Animation.ActiveState = LaraItem->Animation.TargetState = 46;
				LaraItem->RoomNumber = item->RoomNumber;
				LaraItem->Position.xPos = item->Position.xPos;
				LaraItem->Position.yPos = item->Position.yPos;
				LaraItem->Position.zPos = item->Position.zPos;
				LaraItem->Position.yRot = item->Position.yRot;
				LaraItem->Position.xRot = LaraItem->Position.zRot = 0;
				LaraItem->Animation.Airborne = false;
				LaraItem->HitPoints = -1;
				Lara.Air = -1;
				Lara.Control.HandStatus = HandStatus::Busy;
				Lara.Control.Weapon.GunType = LaraWeaponType::None;
			}

			break;

		case MUTANT_STATE_KILL:
			Camera.targetDistance = SECTOR(2);
			Camera.flags = CF_FOLLOW_CENTER;
			break;
		}
	}

	CreatureJoint(item, 0, head);

	if (item->Animation.ActiveState == MUTANT_STATE_FALL)
	{
		AnimateItem(item);

		if (item->Position.yPos > item->Floor)
		{
			item->Animation.TargetState = MUTANT_STATE_IDLE;
			item->Animation.Airborne = false;
			item->Position.yPos = item->Floor;
			Camera.bounce = 500;
		}
	}
	else
		CreatureAnimation(itemNumber, 0, 0);

	if (item->Status == ITEM_DEACTIVATED)
	{
		SoundEffect(171, &item->Position, NULL);
		ExplodingDeath(itemNumber, UINT_MAX, MUTANT_PART_DAMAGE);
		
		TestTriggers(item, true);

		KillItem(itemNumber);
		item->Status = ITEM_DEACTIVATED;
	}
}
