#include "framework.h"
#include "Objects/TR3/Entity/tr3_mpstick.h"

#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO mpstickBite1 = { 247, 10, 11, 13 };
BITE_INFO mpstickBite2 = { 0, 0, 100, 6 };

enum MPSTICK_STATES {
	BATON_EMPTY,
	BATON_STOP,
	BATON_WALK, 
	BATON_PUNCH2,
	BATON_AIM2, 
	BATON_WAIT, 
	BATON_AIM1,
	BATON_AIM0,
	BATON_PUNCH1,
	BATON_PUNCH0,
	BATON_RUN, 
	BATON_DEATH, 
	BATON_KICK, 
	BATON_CLIMB3, 
	BATON_CLIMB1,
	BATON_CLIMB2, 
	BATON_FALL3
};

void InitialiseMPStick(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	ClearItem(itemNumber);

	item->Animation.AnimNumber = Objects[ID_MP_WITH_STICK].animIndex + 6;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.ActiveState = item->Animation.TargetState = BATON_STOP;
}

void MPStickControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CreatureInfo* creature = (CreatureInfo*)item->Data;
	short torsoY = 0;
	short torsoX = 0;
	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->BoxNumber != NO_BOX && (g_Level.Boxes[item->BoxNumber].flags & BLOCKED))
	{
		DoLotsOfBlood(item->Position.xPos, item->Position.yPos - (GetRandomControl() & 255) - 32, item->Position.zPos, (GetRandomControl() & 127) + 128, GetRandomControl() * 2, item->RoomNumber, 3);
		item->HitPoints -= 20;
	}

	int dx;
	int dz;
	int x;
	int y;
	int z;
	AI_INFO info;
	AI_INFO laraInfo;
	ITEM_INFO* target;
	ITEM_INFO* enemy;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != BATON_DEATH)
		{
			item->Animation.AnimNumber = Objects[ID_MP_WITH_STICK].animIndex + 26;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = BATON_DEATH;
			creature->LOT.Step = 256;
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(creature);
		else
		{
			creature->Enemy = LaraItem;

			dx = LaraItem->Position.xPos - item->Position.xPos;
			dz = LaraItem->Position.zPos - item->Position.zPos;

			laraInfo.distance = SQUARE(dx) + SQUARE(dx);

			int bestDistance = 0x7fffffff;
			for (int slot = 0; slot < ActiveCreatures.size(); slot++)
			{
				CreatureInfo* currentCreature = ActiveCreatures[slot];
				if (currentCreature->ItemNumber == NO_ITEM || currentCreature->ItemNumber == itemNumber)
					continue;

				target = &g_Level.Items[currentCreature->ItemNumber];
				if (target->ObjectNumber != ID_LARA)
					continue;

				dx = target->Position.xPos - item->Position.xPos;
				dz = target->Position.zPos - item->Position.zPos;

				if (dz > 32000 || dz < -32000 || dx > 32000 || dx < -32000)
					continue;

				int distance = SQUARE(dx) + SQUARE(dz);
				if (distance < bestDistance && distance < laraInfo.distance)
				{
					bestDistance = distance;
					creature->Enemy = target;
				}
			}
		}

		CreatureAIInfo(item, &info);

		if (creature->Enemy == LaraItem)
		{
			laraInfo.angle = info.angle;
			laraInfo.distance = info.distance;
		}
		else
		{
			dx = LaraItem->Position.xPos - item->Position.xPos;
			dz = LaraItem->Position.zPos - item->Position.zPos;
			laraInfo.angle = phd_atan(dz, dx) - item->Position.yRot;
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		enemy = creature->Enemy;
		creature->Enemy = LaraItem;
		if (item->HitStatus || ((laraInfo.distance < SQUARE(1024) || TargetVisible(item, &laraInfo)) && (abs(LaraItem->Position.yPos - item->Position.yPos) < 1024)))
		{
			if (!creature->Alerted)
				SoundEffect(SFX_TR3_AMERCAN_HOY, &item->Position, 0);
			AlertAllGuards(itemNumber);
		}
		creature->Enemy = enemy;

		switch (item->Animation.ActiveState)
		{
		case BATON_WAIT:
			if (creature->Alerted || item->Animation.TargetState == BATON_RUN)
			{
				item->Animation.TargetState = BATON_STOP;
				break;
			}

		case BATON_STOP:
			creature->Flags = 0;
			creature->MaxTurn = 0;
			head = laraInfo.angle;

			if (item->AIBits & GUARD)
			{
				head = AIGuard(creature);
				if (!(GetRandomControl() & 0xFF))
				{
					if (item->Animation.ActiveState == BATON_STOP)
						item->Animation.TargetState = BATON_WAIT;
					else
						item->Animation.TargetState = BATON_STOP;
				}
				break;
			}

			else if (item->AIBits & PATROL1)
				item->Animation.TargetState = BATON_WALK;

			else if (creature->Mood == MoodType::Escape)
			{
				if (Lara.TargetEntity != item && info.ahead && !item->HitStatus)
					item->Animation.TargetState = BATON_STOP;
				else
					item->Animation.TargetState = BATON_RUN;
			}
			else if (creature->Mood == MoodType::Bored || ((item->AIBits & FOLLOW) && (creature->ReachedGoal || laraInfo.distance > SQUARE(2048))))
			{
				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (info.ahead)
					item->Animation.TargetState = BATON_STOP;
				else
					item->Animation.TargetState = BATON_RUN;
			}
			else if (info.bite && info.distance < SQUARE(512))
				item->Animation.TargetState = BATON_AIM0;
			else if (info.bite && info.distance < SQUARE(1024))
				item->Animation.TargetState = BATON_AIM1;
			else if (info.bite && info.distance < SQUARE(1024))
				item->Animation.TargetState = BATON_WALK;
			else
				item->Animation.TargetState = BATON_RUN;
			break;
		case BATON_WALK:
			head = laraInfo.angle;
			creature->Flags = 0;

			creature->MaxTurn = ANGLE(6);

			if (item->AIBits & PATROL1)
			{
				item->Animation.TargetState = BATON_WALK;
				head = 0;
			}
			else if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = BATON_RUN;
			else if (creature->Mood == MoodType::Bored)
			{
				if (GetRandomControl() < 0x100)
				{
					item->Animation.RequiredState = BATON_WAIT;
					item->Animation.TargetState = BATON_STOP;
				}
			}
			else if (info.bite && info.distance < SQUARE(1536) && info.xAngle < 0)
				item->Animation.TargetState = BATON_KICK;
			else if (info.bite && info.distance < SQUARE(512))
				item->Animation.TargetState = BATON_STOP;
			else if (info.bite && info.distance < SQUARE(1280))
				item->Animation.TargetState = BATON_AIM2;
			else
				item->Animation.TargetState = BATON_RUN;
			break;

		case BATON_RUN:
			if (info.ahead)
				head = info.angle;

			creature->MaxTurn = ANGLE(7);
			tilt = angle / 2;

			if (item->AIBits & GUARD)
				item->Animation.TargetState = BATON_WAIT;
			else if (creature->Mood == MoodType::Escape)
			{
				if (Lara.TargetEntity != item && info.ahead)
					item->Animation.TargetState = BATON_STOP;
				break;
			}
			else if ((item->AIBits & FOLLOW) && (creature->ReachedGoal || laraInfo.distance > SQUARE(2048)))
				item->Animation.TargetState = BATON_STOP;
			else if (creature->Mood == MoodType::Bored)
				item->Animation.TargetState = BATON_WALK;
			else if (info.ahead && info.distance < SQUARE(1024))
				item->Animation.TargetState = BATON_WALK;
			break;

		case BATON_AIM0:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->MaxTurn = ANGLE(6);

			creature->Flags = 0;
			if (info.bite && info.distance < SQUARE(512))
				item->Animation.TargetState = BATON_PUNCH0;
			else
				item->Animation.TargetState = BATON_STOP;
			break;

		case BATON_AIM1:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->MaxTurn = ANGLE(6);

			creature->Flags = 0;
			if (info.ahead && info.distance < SQUARE(1024))
				item->Animation.TargetState = BATON_PUNCH1;
			else
				item->Animation.TargetState = BATON_STOP;
			break;

		case BATON_AIM2:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->MaxTurn = ANGLE(6);

			creature->Flags = 0;
			if (info.bite && info.distance < SQUARE(1280))
				item->Animation.TargetState = BATON_PUNCH2;
			else
				item->Animation.TargetState = BATON_WALK;
			break;

		case BATON_PUNCH0:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->MaxTurn = ANGLE(6);

			if (enemy == LaraItem)
			{
				if (!creature->Flags && (item->TouchBits & 0x2400))
				{
					LaraItem->HitPoints -= 80;
					LaraItem->HitStatus = 1;
					CreatureEffect(item, &mpstickBite1, DoBloodSplat);
					SoundEffect(SFX_TR4_LARA_THUD, &item->Position, 0);

					creature->Flags = 1;
				}
			}
			else
			{
				if (!creature->Flags && enemy)
				{
					if (abs(enemy->Position.xPos - item->Position.xPos) < 256 &&
						abs(enemy->Position.yPos - item->Position.yPos) <= 256 &&
						abs(enemy->Position.zPos - item->Position.zPos) < 256)
					{
						enemy->HitPoints -= 5;
						enemy->HitStatus = 1;
						creature->Flags = 1;
						CreatureEffect(item, &mpstickBite1, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Position, 0);
					}
				}
			}

			break;

		case BATON_PUNCH1:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->MaxTurn = ANGLE(6);

			if (enemy == LaraItem)
			{
				if (!creature->Flags && (item->TouchBits & 0x2400))
				{
					LaraItem->HitPoints -= 80;
					LaraItem->HitStatus = 1;
					CreatureEffect(item, &mpstickBite1, DoBloodSplat);
					SoundEffect(SFX_TR4_LARA_THUD, &item->Position, 0);

					creature->Flags = 1;
				}
			}
			else
			{
				if (!creature->Flags && enemy)
				{
					if (abs(enemy->Position.xPos - item->Position.xPos) < 256 &&
						abs(enemy->Position.yPos - item->Position.yPos) <= 256 &&
						abs(enemy->Position.zPos - item->Position.zPos) < 256)
					{
						enemy->HitPoints -= 5;
						enemy->HitStatus = 1;
						creature->Flags = 1;
						CreatureEffect(item, &mpstickBite1, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Position, 0);

					}
				}
			}


			if (info.ahead && info.distance > SQUARE(1024) && info.distance < SQUARE(1280))
				item->Animation.TargetState = BATON_PUNCH2;
			break;

		case BATON_PUNCH2:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->MaxTurn = ANGLE(6);

			if (enemy == LaraItem)
			{
				if (creature->Flags != 2 && (item->TouchBits & 0x2400))
				{
					LaraItem->HitPoints -= 100;
					LaraItem->HitStatus = 1;
					CreatureEffect(item, &mpstickBite1, DoBloodSplat);
					creature->Flags = 2;
					SoundEffect(70, &item->Position, 0);

				}
			}
			else
			{
				if (creature->Flags != 2 && enemy)
				{
					if (abs(enemy->Position.xPos - item->Position.xPos) < 256 &&
						abs(enemy->Position.yPos - item->Position.yPos) <= 256 &&
						abs(enemy->Position.zPos - item->Position.zPos) < 256)
					{
						enemy->HitPoints -= 6;
						enemy->HitStatus = 1;
						creature->Flags = 2;
						CreatureEffect(item, &mpstickBite1, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Position, 0);
					}
				}
			}
			break;
		case BATON_KICK:
			if (info.ahead)
			{
				torsoY = info.angle;
			}
			creature->MaxTurn = ANGLE(6);

			if (enemy == LaraItem)
			{
				if (creature->Flags != 1 && (item->TouchBits & 0x60) && (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 8))
				{
					LaraItem->HitPoints -= 150;
					LaraItem->HitStatus = 1;
					CreatureEffect(item, &mpstickBite2, DoBloodSplat);
					SoundEffect(SFX_TR4_LARA_THUD, &item->Position, 0);

					creature->Flags = 1;
				}
			}
			else
			{
				if (!creature->Flags != 1 && enemy && (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 8))
				{
					if (abs(enemy->Position.xPos - item->Position.xPos) < 256 &&
						abs(enemy->Position.yPos - item->Position.yPos) <= 256 &&
						abs(enemy->Position.zPos - item->Position.zPos) < 256)
					{
						enemy->HitPoints -= 9;
						enemy->HitStatus = 1;
						creature->Flags = 1;
						CreatureEffect(item, &mpstickBite2, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Position, 0);
					}
				}
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torsoY);
	CreatureJoint(item, 1, torsoX);
	CreatureJoint(item, 2, head);

	if (item->Animation.ActiveState < BATON_DEATH)
	{
		switch (CreatureVault(itemNumber, angle, 2, 260))
		{
		case 2:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[ID_MP_WITH_STICK].animIndex + 28;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = BATON_CLIMB1;
			break;

		case 3:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[ID_MP_WITH_STICK].animIndex + 29;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = BATON_CLIMB2;
			break;

		case 4:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[ID_MP_WITH_STICK].animIndex + 27;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = BATON_CLIMB3;
			break;
		case -4:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[ID_MP_WITH_STICK].animIndex + 30;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = BATON_FALL3;
			break;
		}
	}
	else
	{
		creature->MaxTurn = 0;
		CreatureAnimation(itemNumber, angle, tilt);
	}
}