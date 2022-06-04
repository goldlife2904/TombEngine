#include "framework.h"
#include "tr5_bodypart.h"
#include "Game/effects/effects.h"
#include "Specific/trmath.h"
#include "Sound/sound.h"
#include "tr5_missile.h"
#include "Game/collision/collide_room.h"
#include "Game/items.h"

void ControlBodyPart(short fxNumber)
{
	FX_INFO* fx = &EffectList[fxNumber];
	int x = fx->pos.Position.x;
	int y = fx->pos.Position.y;
	int z = fx->pos.Position.z;

	if (fx->counter <= 0)
	{
		if (fx->speed)
			fx->pos.Orientation.SetX(fx->pos.Orientation.GetX() + 4 * fx->fallspeed);
		fx->fallspeed += 6;
	}
	else
	{
		int modulus = 62 - fx->counter;
		int random = modulus <= 1 ? 0 : 2 * GetRandomControl() % modulus;
		if (fxNumber & 1)
		{
			fx->pos.Orientation.SetZ(fx->pos.Orientation.GetZ() - random);
			fx->pos.Orientation.SetX(fx->pos.Orientation.GetX() + random);
		}
		else
		{
			fx->pos.Orientation.SetZ(fx->pos.Orientation.GetZ() + random);
			fx->pos.Orientation.SetX(fx->pos.Orientation.GetX() - random);
		}
		if (--fx->counter < 8)
			fx->fallspeed += 2;
	}

	fx->pos.Position.x += fx->speed * sin(fx->pos.Orientation.GetY());
	fx->pos.Position.y += fx->fallspeed;
	fx->pos.Position.z += fx->speed * cos(fx->pos.Orientation.GetY());

	short roomNumber = fx->roomNumber;
	FloorInfo* floor = GetFloor(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z,&roomNumber);

	if (!fx->counter)
	{
		int ceiling = GetCeiling(floor, fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z);
		if (fx->pos.Position.y < ceiling)
		{
			fx->pos.Position.y = ceiling;
			fx->fallspeed = -fx->fallspeed;
			fx->speed -= (fx->speed / 8);
		}

		int height = GetFloorHeight(floor, fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z);
		if (fx->pos.Position.y >= height)
		{
			if (fx->flag2 & 1)
			{
				fx->pos.Position.x = x;
				fx->pos.Position.y = y;
				fx->pos.Position.z = z;

				if (fx->flag2 & 0x200)
					ExplodeFX(fx, -2, 32);
				else
					ExplodeFX(fx, -1, 32);

				KillEffect(fxNumber);
				if (fx->flag2 & 0x800)
					SoundEffect(SFX_TR4_ROCK_FALL_LAND,&fx->pos);
				return;
			}

			if (y <= height)
			{
				if (fx->fallspeed <= 32)
					fx->fallspeed = 0;
				else
					fx->fallspeed = -fx->fallspeed / 4;
			}
			else
			{
				fx->pos.Orientation.SetY(fx->pos.Orientation.GetY() + Angle::DegToRad(-180.0f));
				fx->pos.Position.x = x;
				fx->pos.Position.z = z;
			}

			fx->speed -= (fx->speed / 4);
			if (abs(fx->speed) < 4)
				fx->speed = 0;
			fx->pos.Position.y = y;
		}

		if (!fx->speed && ++fx->flag1 > 32)
		{
			KillEffect(fxNumber);
			return;
		}

		if (fx->flag2 & 2 && (GetRandomControl() & 1))
		{
			DoBloodSplat(
				(GetRandomControl() & 0x3F) + fx->pos.Position.x - 32,
				(GetRandomControl() & 0x1F) + fx->pos.Position.y - 16,
				(GetRandomControl() & 0x3F) + fx->pos.Position.z - 32,
				1,
				2 * GetRandomControl(),
				fx->roomNumber);
		}
	}

	if (roomNumber != fx->roomNumber)
		EffectNewRoom(fxNumber, roomNumber);
}