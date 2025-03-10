#include "framework.h"
#include "Objects/TR4/Entity/tr4_big_scorpion.h"

#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::TR4
{
	constexpr auto BIG_SCORPION_ATTACK_DAMAGE		   = 120;
	constexpr auto BIG_SCORPION_TROOP_ATTACK_DAMAGE	   = 15;
	constexpr auto BIG_SCORPION_STINGER_POISON_POTENCY = 16;

	constexpr auto BIG_SCORPION_ATTACK_RANGE = SQUARE(SECTOR(1.35));
	constexpr auto BIG_SCORPION_RUN_RANGE	 = SQUARE(SECTOR(2));

	const auto BigScorpionBite1 = BiteInfo(Vector3::Zero, 8);
	const auto BigScorpionBite2 = BiteInfo(Vector3::Zero, 23);
	const vector<int> BigScorpionAttackJoints = { 8, 20, 21, 23, 24 };

	int CutSeqNum;

	enum BigScorpionState
	{
		// No state 0.
		BSCORPION_STATE_IDLE = 1,
		BSCORPION_STATE_WALK_FORWARD = 2,
		BSCORPION_STATE_RUN_FORWARD = 3,
		BSCORPION_STATE_PINCER_ATTACK = 4,
		BSCORPION_STATE_STINGER_ATTACK = 5,
		BSCORPION_STATE_DEATH = 6,
		BSCORPION_STATE_KILL = 7,
		BSCORPION_STATE_KILL_TROOP = 8
	};

	enum BigScorpionAnim
	{
		BSCORPION_ANIM_WALK_FORWARD = 0,
		BSCORPION_ANIM_RUN_FORWARD = 1,
		BSCORPION_ANIM_IDLE = 2,
		BSCORPION_ANIM_PINCER_ATTACK = 3,
		BSCORPION_ANIM_STINGER_ATTACK = 4,
		BSCORPION_ANIM_DEATH = 5,
		BSCORPION_ANIM_KILL = 6,
		BSCORPION_ANIM_KILL_TROOP = 7
	};

	void InitialiseScorpion(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);

		if (item->TriggerFlags == 1)
			SetAnimation(item, BSCORPION_ANIM_KILL_TROOP);
		else
			SetAnimation(item, BSCORPION_ANIM_IDLE);
	}

	void ScorpionControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* object = &Objects[item->ObjectNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		
		if (item->HitPoints <= 0)
		{
			item->HitPoints = 0;
			if (item->Animation.ActiveState != BSCORPION_STATE_DEATH)
			{
				if (item->TriggerFlags > 0 && item->TriggerFlags < 7)
				{
					CutSeqNum = 4;
					SetAnimation(item, BSCORPION_ANIM_DEATH);
					item->Status = ITEM_INVISIBLE;
					creature->MaxTurn = 0;

					short linkNumber = g_Level.Rooms[item->RoomNumber].itemNumber;
					if (linkNumber != NO_ITEM)
					{
						for (linkNumber = g_Level.Rooms[item->RoomNumber].itemNumber; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
						{
							auto* currentItem = &g_Level.Items[linkNumber];

							if (currentItem->ObjectNumber == ID_TROOPS && currentItem->TriggerFlags == 1)
							{
								DisableEntityAI(linkNumber);
								KillItem(linkNumber);
								currentItem->Flags |= IFLAG_KILLED;
								break;
							}
						}
					}
				}
				else if (item->Animation.ActiveState != BSCORPION_STATE_DEATH && item->Animation.ActiveState != BSCORPION_STATE_KILL)
				{
					SetAnimation(item, BSCORPION_ANIM_DEATH);
				}
			}
			else if (CutSeqNum == 4)
			{
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameEnd - 1;
				item->Status = ITEM_INVISIBLE;
			}
			else if (item->Animation.ActiveState == BSCORPION_STATE_DEATH && item->Status == ITEM_INVISIBLE)
				item->Status = ITEM_ACTIVE;
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
			{
				if (creature->HurtByLara &&
					item->Animation.ActiveState != BSCORPION_STATE_KILL_TROOP)
				{
					creature->Enemy = LaraItem;
				}
				else
				{
					creature->Enemy = nullptr;
					int minDistance = INT_MAX;

					for (int i = 0; i < ActiveCreatures.size(); i++)
					{
						auto* currentCreatureInfo = ActiveCreatures[i];
						if (currentCreatureInfo->ItemNumber != NO_ITEM && currentCreatureInfo->ItemNumber != itemNumber)
						{
							auto* currentItem = &g_Level.Items[currentCreatureInfo->ItemNumber];
							if (currentItem->ObjectNumber != ID_LARA)
							{
								if (currentItem->ObjectNumber != ID_BIG_SCORPION &&
									(!currentItem->IsLara() || creature->HurtByLara))
								{
									int dx = currentItem->Pose.Position.x - item->Pose.Position.x;
									int dy = currentItem->Pose.Position.y - item->Pose.Position.y;
									int dz = currentItem->Pose.Position.z - item->Pose.Position.z;

									int distance = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);
									if (distance < minDistance)
									{
										minDistance = distance;
										creature->Enemy = currentItem;
									}
								}
							}
						}
					}
				}
			}

			AI_INFO AI;
			CreatureAIInfo(item, &AI);
			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);
			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case BSCORPION_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (AI.distance > BIG_SCORPION_ATTACK_RANGE)
				{
					item->Animation.TargetState = BSCORPION_STATE_WALK_FORWARD;
					break;
				}

				if (AI.bite)
				{
					creature->MaxTurn = ANGLE(2.0f);

					// If chanced upon or the troop is close to death, do pincer attack.
					if (TestProbability(0.5f) ||
						(creature->Enemy != nullptr && creature->Enemy->HitPoints <= 15 && creature->Enemy->ObjectNumber == ID_TROOPS))
					{
						item->Animation.TargetState = BSCORPION_STATE_PINCER_ATTACK;
					}
					else
						item->Animation.TargetState = BSCORPION_STATE_STINGER_ATTACK;
				}
				else if (!AI.ahead)
					item->Animation.TargetState = BSCORPION_STATE_WALK_FORWARD;

				break;

			case BSCORPION_STATE_WALK_FORWARD:
				creature->MaxTurn = ANGLE(2.0f);

				if (AI.distance < BIG_SCORPION_ATTACK_RANGE)
					item->Animation.TargetState = BSCORPION_STATE_IDLE;
				else if (AI.distance > BIG_SCORPION_RUN_RANGE)
					item->Animation.TargetState = BSCORPION_STATE_RUN_FORWARD;

				break;

			case BSCORPION_STATE_RUN_FORWARD:
				creature->MaxTurn = ANGLE(3.0f);

				if (AI.distance < BIG_SCORPION_ATTACK_RANGE)
					item->Animation.TargetState = BSCORPION_STATE_IDLE;

				break;

			case BSCORPION_STATE_PINCER_ATTACK:
			case BSCORPION_STATE_STINGER_ATTACK:
				creature->MaxTurn = 0;

				if (abs(AI.angle) >= ANGLE(2.0f))
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += ANGLE(2.0f);
					else
						item->Pose.Orientation.y -= ANGLE(2.0f);
				}
				else
					item->Pose.Orientation.y += AI.angle;

				if (creature->Flags != 0)
					break;

				if (creature->Enemy && !creature->Enemy->IsLara() && AI.distance < BIG_SCORPION_ATTACK_RANGE)
				{
					DoDamage(creature->Enemy, BIG_SCORPION_TROOP_ATTACK_DAMAGE);
					CreatureEffect2(item, BigScorpionBite1, 10, item->Pose.Orientation.y - ANGLE(180.0f), DoBloodSplat);
					creature->Flags = 1;

					if (creature->Enemy->HitPoints <= 0)
					{
						item->Animation.TargetState = BSCORPION_STATE_KILL;
						creature->MaxTurn = 0;
					}
				}
				else if (item->TestBits(JointBitType::Touch, BigScorpionAttackJoints))
				{
					DoDamage(creature->Enemy, BIG_SCORPION_ATTACK_DAMAGE);

					if (item->Animation.ActiveState == BSCORPION_STATE_STINGER_ATTACK)
					{
						if (creature->Enemy->IsLara())
							GetLaraInfo(creature->Enemy)->PoisonPotency += BIG_SCORPION_STINGER_POISON_POTENCY;

						CreatureEffect2(item, BigScorpionBite1, 10, item->Pose.Orientation.y - ANGLE(180.0f), DoBloodSplat);
					}
					else
						CreatureEffect2(item, BigScorpionBite2, 10, item->Pose.Orientation.y - ANGLE(180.0f), DoBloodSplat);

					creature->Flags = 1;

					if (creature->Enemy->IsLara() && creature->Enemy->HitPoints <= 0)
					{
						CreatureKill(item, BSCORPION_ANIM_KILL, BSCORPION_STATE_KILL, LA_BIG_SCORPION_DEATH);
						creature->MaxTurn = 0;
						return;
					}
				}

				break;

			case BSCORPION_STATE_KILL_TROOP:
				creature->MaxTurn = 0;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
					item->TriggerFlags++;

				if ((creature->Enemy != nullptr && creature->Enemy->HitPoints <= 0) ||
					item->TriggerFlags > 6)
				{
					item->Animation.TargetState = BSCORPION_STATE_IDLE;
					creature->MaxTurn = 0;
				}

				break;

			default:
				break;
			}
		}

		if (!CutSeqNum)
			CreatureAnimation(itemNumber, angle, 0);

		auto radius = Vector2(object->radius, object->radius * 1.33f);
		AlignEntityToSurface(item, radius);
	}
}
