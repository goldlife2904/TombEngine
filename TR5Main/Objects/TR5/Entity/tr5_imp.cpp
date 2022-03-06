#include "framework.h"
#include "tr5_imp.h"
#include "Game/items.h"
#include "Game/animation.h"
#include "Game/effects/effects.h"
#include "Game/control/box.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"
#include "Objects/Generic/Object/burning_torch.h"

using namespace TEN::Entities::Generic;

BITE_INFO ImpBite = { 0, 100, 0, 9 };

enum ImpState
{
	IMP_STATE_WALK = 0,
	IMP_STATE_IDLE = 1,
	IMP_STATE_RUN = 2,
	IMP_STATE_ATTACK_1 = 3,
	IMP_STATE_ATTACK_2 = 5,
	IMP_STATE_SCARED = 6,
	IMP_STATE_START_CLIMB = 7,
	IMP_STATE_START_ROLL = 8,
	IMP_STATE_DEATH = 9,
	IMP_STATE_THROW_STONES = 11
};

// TODO
enum ImpAnim
{
	IMP_ANIM_DEATH = 18
};

void InitialiseImp(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	ImpState state;

	if (item->TriggerFlags == 2 || item->TriggerFlags == 12)
	{
		state = IMP_STATE_START_ROLL;
		item->AnimNumber = Objects[ID_IMP].animIndex + 8;
	}
	else if (item->TriggerFlags == 1 || item->TriggerFlags == 11)
	{
		state = IMP_STATE_START_CLIMB;
		item->AnimNumber = Objects[ID_IMP].animIndex + 7;
	}
	else
	{
		state = IMP_STATE_IDLE;
		item->AnimNumber = Objects[ID_IMP].animIndex + 1;
	}

	item->TargetState = state;
	item->ActiveState = state;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
}

static void ImpThrowStones(ITEM_INFO* item)
{
	PHD_VECTOR pos1 = { 0, 0, 0 };
	GetJointAbsPosition(item, &pos1, 9);

	PHD_VECTOR pos2 = { 0, 0, 0 };
	GetLaraJointPosition(&pos2, LM_HEAD);

	int dx = pos1.x - pos2.x;
	int dy = pos1.y - pos2.y;
	int dz = pos1.z - pos2.z;

	short angles[2];
	phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);
	
	int distance = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
	if (distance < 8)
		distance = 8;

	angles[0] += GetRandomControl() % (distance / 4) - (distance / 8);
	angles[1] += GetRandomControl() % (distance / 2) - (distance / 4);
	
	short fxNumber = CreateNewEffect(item->RoomNumber);
	if (fxNumber != NO_ITEM)
	{
		auto* fx = &EffectList[fxNumber];

		fx->pos.xPos = pos1.x;
		fx->pos.yPos = pos1.y;
		fx->pos.zPos = pos1.z;
		fx->roomNumber = item->RoomNumber;
		fx->pos.xRot = (angles[1] + distance) / 2;
		fx->pos.yRot = angles[0];
		fx->pos.zRot = 0;
		fx->speed = 4 * sqrt(distance);

		if (fx->speed < 256)
			fx->speed = 256;

		fx->fallspeed = 0;
		fxNumber = Objects[ID_IMP_ROCK].meshIndex + (GetRandomControl() & 7);
		fx->objectNumber = ID_IMP_ROCK;
		fx->shade = 16912;
		fx->counter = 0;
		fx->frameNumber = fxNumber;
		fx->flag1 = 2;
		fx->flag2 = 0x2000;
	}
}

void ImpControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		short angle1 = 0;
		short angle2 = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;
		short joint3 = 0;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		if (item->HitPoints > 0)
		{
			if (item->AIBits)
				GetAITarget(creature);
			else if (creature->HurtByLara)
				creature->Enemy = LaraItem;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);
			
			if (creature->Enemy == LaraItem)
				angle2 = AI.angle;
			else
				angle2 = phd_atan(LaraItem->Position.zPos - item->Position.zPos, LaraItem->Position.xPos - item->Position.xPos) - item->Position.yRot;

			int d1 = item->Position.yPos - LaraItem->Position.yPos + CLICK(1.5f);

			if (LaraItem->ActiveState == LS_CROUCH_IDLE ||
				LaraItem->ActiveState == LS_CROUCH_ROLL ||
				LaraItem->ActiveState > LS_MONKEY_TURN_180 &&
				LaraItem->ActiveState < LS_HANG_TO_CRAWL ||
				LaraItem->ActiveState == LS_CROUCH_TURN_LEFT ||
				LaraItem->ActiveState == LS_CROUCH_TURN_RIGHT)
			{
				d1 = item->Position.yPos - LaraItem->Position.yPos;
			}

			int d2 = sqrt(AI.distance);

			AI.xAngle = phd_atan(d2, d1);

			GetCreatureMood(item, &AI, VIOLENT);

			if (item->ActiveState == IMP_STATE_SCARED)
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, VIOLENT);

			angle1 = CreatureTurn(item, creature->MaxTurn);
			joint0 = AI.xAngle / 2;
			joint1 = AI.angle / 2;
			joint2 = AI.xAngle / 2;
			joint3 = AI.angle / 2;

			if (Wibble & 0x10)
				item->SwapMeshFlags = 1024;
			else
				item->SwapMeshFlags = 0;

			switch (item->ActiveState)
			{
			case IMP_STATE_WALK:
				creature->MaxTurn = ANGLE(7.0f);
				if (AI.distance <= pow(SECTOR(2), 2))
				{
					if (AI.distance < pow(SECTOR(2), 2))
						item->TargetState = IMP_STATE_IDLE;
				}
				else
					item->TargetState = IMP_STATE_RUN;
				
				break;

			case IMP_STATE_IDLE:
				creature->MaxTurn = -1;
				creature->Flags = 0;

				if (AI.bite && AI.distance < pow(170, 2) && item->TriggerFlags < 10)
				{
					if (GetRandomControl() & 1)
						item->TargetState = IMP_STATE_ATTACK_1;
					else
						item->TargetState = IMP_STATE_ATTACK_2;
				}
				else if (item->AIBits & FOLLOW)
					item->TargetState = IMP_STATE_WALK;
				else
				{
					if (item->TriggerFlags == 3)
						item->TargetState = IMP_STATE_THROW_STONES;
					else if (AI.distance <= pow(SECTOR(2), 2))
					{
						if (AI.distance > pow(SECTOR(0.5f), 2) || item->TriggerFlags < 10)
							item->TargetState = IMP_STATE_WALK;
					}
					else
						item->TargetState = IMP_STATE_RUN;
				}

				break;

			case IMP_STATE_RUN:
				creature->MaxTurn = ANGLE(7.0f);

				if (AI.distance >= pow(SECTOR(0.5f), 2))
				{
					if (AI.distance < pow(SECTOR(2), 2))
						item->TargetState = IMP_STATE_WALK;
				}
				else
					item->TargetState = IMP_STATE_IDLE;
				
				break;

			case IMP_STATE_ATTACK_1:
			case IMP_STATE_ATTACK_2:
				creature->MaxTurn = -1;

				if (creature->Flags == 0 &&
					item->TouchBits & 0x280)
				{
					CreatureEffect2(item, &ImpBite, 10, item->Position.yRot, DoBloodSplat);

					LaraItem->HitPoints -= 3;
					LaraItem->HitStatus = true;
				}

				break;

			case IMP_STATE_SCARED:
				creature->MaxTurn = ANGLE(7.0f);
				break;

			case IMP_STATE_START_CLIMB:
			case IMP_STATE_START_ROLL:
				creature->MaxTurn = 0;
				break;

			case IMP_STATE_THROW_STONES:
				creature->MaxTurn = -1;

				if (item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase == 40)
					ImpThrowStones(item);

				break;

			default:
				break;
			}
		}
		else
		{
			item->HitPoints = 0;

			if (item->ActiveState != IMP_STATE_DEATH)
			{
				item->AnimNumber = Objects[ID_IMP].animIndex + IMP_ANIM_DEATH;
				item->ActiveState = IMP_STATE_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			}
		}

		if (creature->MaxTurn == -1)
		{
			creature->MaxTurn = 0;
			if (abs(angle2) >= ANGLE(2.0f))
			{
				if (angle2 >= 0)
					item->Position.yRot += ANGLE(2.0f);
				else
					item->Position.yRot -= ANGLE(2.0f);
			}
			else
				item->Position.yRot += angle2;
		}

		CreatureTilt(item, 0);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 3, joint3);
		CreatureJoint(item, 2, joint2);
		CreatureAnimation(itemNumber, angle1, 0);
	}
}
