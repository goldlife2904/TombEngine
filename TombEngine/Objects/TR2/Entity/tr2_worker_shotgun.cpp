#include "framework.h"
#include "Objects/TR2/Entity/tr2_worker_shotgun.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"

namespace TEN::Entities::Creatures::TR2
{
	const auto WorkerShotgunBite = BiteInfo(Vector3(0.0f, 281.0f, 40.0f), 9);

	constexpr auto WORKER_SHOTGUN_NUM_SHOTS = 6;

	// TODO
	enum ShotgunWorkerState
	{

	};

	// TODO
	enum ShotgunWorkerAnim
	{

	};

	void ShotLaraWithShotgun(ItemInfo* item, AI_INFO* info, BiteInfo bite, short angleY, int damage)
	{
		for (int i = 0; i < WORKER_SHOTGUN_NUM_SHOTS; i++)
			ShotLara(item, info, bite, angleY, damage);
	}

	void InitialiseWorkerShotgun(short itemNum)
	{
		auto* item = &g_Level.Items[itemNum];

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 5;
		ClearItem(itemNum);

		auto* anim = &g_Level.Anims[item->Animation.AnimNumber];
		item->Animation.FrameNumber = anim->frameBase;
		item->Animation.ActiveState = anim->ActiveState;
	}

	void WorkerShotgunControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short tilt = 0;
		short angle = 0;
		short headX = 0;
		short headY = 0;
		short torsoX = 0;
		short torsoY = 0;

		if (creature->FiredWeapon)
		{
			auto pos = Vector3Int(WorkerShotgunBite.Position);
			GetJointAbsPosition(item, &pos, WorkerShotgunBite.meshNum);
			TriggerDynamicLight(pos.x, pos.y, pos.z, (creature->FiredWeapon * 2) + 4, 24, 16, 4);
			creature->FiredWeapon--;
		}

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 7)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 18;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 7;
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case 2:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (AI.ahead)
				{
					headX = AI.xAngle;
					headY = AI.angle;
				}

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 5;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < pow(SECTOR(3), 2) || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = (GetRandomControl() >= 0x4000) ? 9 : 8;
					else
						item->Animation.TargetState = 1;
				}
				else if (creature->Mood == MoodType::Attack || !AI.ahead)
				{
					if (AI.distance <= pow(SECTOR(2), 2))
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 5;
				}
				else
					item->Animation.TargetState = 3;

				break;

			case 3:
				if (AI.ahead)
				{
					headX = AI.xAngle;
					headY = AI.angle;
				}

				if (Targetable(item, &AI))
					item->Animation.TargetState = 4;
				else if (creature->Mood == MoodType::Attack || !AI.ahead)
					item->Animation.TargetState = 2;

				break;

			case 1:
				creature->MaxTurn = ANGLE(3.0f);

				if (AI.ahead)
				{
					headY = AI.angle;
					headX = AI.xAngle;
				}

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 5;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < pow(SECTOR(3), 2) || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = 2;
					else
						item->Animation.TargetState = 6;
				}
				else if (creature->Mood == MoodType::Attack || !AI.ahead)
				{
					if (AI.distance > pow(SECTOR(2), 2))
						item->Animation.TargetState = 5;
				}
				else
					item->Animation.TargetState = 2;

				break;

			case 5:
				tilt = angle / 2;
				creature->MaxTurn = 910;

				if (AI.ahead)
				{
					headX = AI.xAngle;
					headY = AI.angle;
				}

				if (creature->Mood != MoodType::Escape)
				{
					if (Targetable(item, &AI))
						item->Animation.TargetState = 1;
					else if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk)
						item->Animation.TargetState = 1;
				}

				break;

			case 8:
			case 9:
				creature->Flags = 0;

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (Targetable(item, &AI))
				{
					if (item->Animation.ActiveState == 8)
						item->Animation.TargetState = 4;
					else
						item->Animation.TargetState = 10;
				}

				break;

			case 4:
			case 10:
				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (!creature->Flags)
				{
					ShotLaraWithShotgun(item, &AI, WorkerShotgunBite, torsoY, 25);
					creature->FiredWeapon = 2;
					creature->Flags = 1;
				}

				if (item->Animation.ActiveState == 4 && item->Animation.TargetState != 2 &&
					(creature->Mood == MoodType::Escape || AI.distance > pow(SECTOR(3), 2) || !Targetable(item, &AI)))
				{
					item->Animation.TargetState = 2;
				}

				break;

			case 6:
				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (!creature->Flags)
				{
					ShotLaraWithShotgun(item, &AI, WorkerShotgunBite, torsoY, 25);
					creature->FiredWeapon = 2;
					creature->Flags = 1;
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, torsoY);
		CreatureJoint(item, 1, torsoX);
		CreatureJoint(item, 2, headY);
		CreatureJoint(item, 3, headX);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
