#pragma once

#include <bass.h>
#include <bass_fx.h>
#include <d3dx9math.h>

#include "..\Game\control.h"
#include "..\Global\global.h"

typedef enum audio_tracks
{
	CDA_XA1_TL_10B, // TO CLIMB OUT, SWIM TO THE EDGE AND PRESS 'ACTION'.
	CDA_XA1_Z10,
	CDA_XA1_TL_05, // TO PERFORM A SIDEWAYS SOMERSAULT, PRESS 'LEFT' AND JUMP.
	CDA_XA1_TL_08, // FOR ME TO CRAWL, HOLD DOWN 'CROUCH' AND NOW PUSH 'FORWARD'.
	CDA_XA1_TL_11, // TO USE THE LEVER SWITCH, STAND NEXT TO THE SWITCH, AND PRESS 'ACTION'.
	CDA_XA1_ANDYPEW,
	CDA_XA1_SECRET,
	CDA_XA1_TL_02, // TO LEAP THE SHORT DISTANCE, WALK TO THE EDGE, NOW PRESS 'FORWARD' AND 'JUMP' TOGETHER.
	CDA_XA2_HMMM05,
	CDA_XA2_TL_01, // TO CLIMB ONTO THIS CRATE, STAND NEXT TO THE CRATE, PUSH 'FORWARD', AND THEN 'ACTION'.
	CDA_XA2_ATTACK04,
	CDA_XA2_UWATER2B,
	CDA_XA2_SPOOKY2A,
	CDA_XA2_TL_10A, // TO SWIM, JUMP INTO THE WATER. USE 'ACTION' TO SWIM FORWARD, AND THE DIRECTIONS TO STEER.
	CDA_XA2_HMMM02,
	CDA_XA2_TOMS01,
	CDA_XA3_ATTACK03,
	CDA_XA3_ATTACK02,
	CDA_XA3_HMMM01,
	CDA_XA3_STEALTH1,
	CDA_XA3_STEALTH2,
	CDA_XA3_ATTACK01,
	CDA_XA3_TL_06, // TO MONKEYSWING, JUMP STRAIGHT UP AT THE BARS, AND PRESS AND HOLD 'ACTION', THEN PUSH 'FORWARD'.
	CDA_XA3_TL_03, // NOW LET'S TRY A LONGER LEAP. WALK TO THE EDGE, AND TAP 'BACK' ONCE. NOW PRESS 'FORWARD' AND 'JUMP' TOGETHER.
	CDA_XA4_HMMM06,
	CDA_XA4_MIL01,
	CDA_XA4_Z_03,
	CDA_XA4_HIT01,
	CDA_XA4_SPOOKY05,
	CDA_XA4_DRAMA01,
	CDA_XA4_STEALTH4,
	CDA_XA4_MIL05,
	CDA_XA5_HMMM04,
	CDA_XA5_MIL06,
	CDA_XA5_SPOOKY02,
	CDA_XA5_TL_12, // TO WALK THE TIGHTROPE, WALK UP TO THE ROPE AND PRESS 'ACTION'. PUSH 'FORWARD'. IF YOU UNBALANCE, PUSH IN THE OPPOSITE DIRECTION TO CORRECT.
	CDA_XA5_MIL02A,
	CDA_XA5_HMMM03,
	CDA_XA5_MIL02,
	CDA_XA5_TL_04, // AND NOW FOR THE BIG JUMP. WALK TO THE EDGE, THEN TAP 'BACK' ONCE. NOW PRESS 'FORWARD' AND 'JUMP' TOGETHER. WHEN IN MID-AIR, HOLD 'ACTION' TO GRAB ONTO THE LEDGE.
	CDA_XA6_MIL04,
	CDA_XA6_SOLO01,
	CDA_XA6_Z12,
	CDA_XA6_STEALTH3,
	CDA_XA6_AUTHSOLO,
	CDA_XA6_SPOOKY03,
	CDA_XA6_Z13,
	CDA_XA6_Z_04ANIM,
	CDA_XA7_Z_06A,
	CDA_XA7_ANDYOOOH,
	CDA_XA7_ANDYOOER,
	CDA_XA7_TL_07, // FOR ME TO CLIMB THIS WALL, STAND NEXT TO IT, AND PRESS 'FORWARD' AND 'ACTION' TOGETHER. KEEP HOLD OF 'ACTION' AND PUSH UP TO CLIMB THE WALL.
	CDA_XA7_Z_02,
	CDA_XA7_EVIBES01,
	CDA_XA7_Z_06,
	CDA_XA7_AUTHTR,
	CDA_XA8_MIL03,
	CDA_XA8_FIGHTSC,
	CDA_XA8_RICHCUT3,
	CDA_XA8_Z_13,
	CDA_XA8_Z_08,
	CDA_XA8_UWATER2A,
	CDA_XA8_JOBYALRM,
	CDA_XA8_MIL02B,
	CDA_XA9_SWAMPY,
	CDA_XA9_EVIBES02,
	CDA_XA9_GODS01,
	CDA_XA9_Z_03,
	CDA_XA9_RICHCUT4,
	CDA_XA9_TITLE4,
	CDA_XA9_SPOOKY01,
	CDA_XA9_CHOPIN01,
	CDA_XA10_ECHOIR01,
	CDA_XA10_TITLE3,
	CDA_XA10_PERC01,
	CDA_XA10_VC01,
	CDA_XA10_TITLE2,
	CDA_XA10_Z_09,
	CDA_XA10_SPOOKY04,
	CDA_XA10_Z_10,
	CDA_XA11_VC01ATV,
	CDA_XA11_ANDY3,
	CDA_XA11_TITLE1,
	CDA_XA11_FLYBY1,
	CDA_XA11_MONK_2,
	CDA_XA11_ANDY4,
	CDA_XA11_FLYBY3,
	CDA_XA11_FLYBY2,
	CDA_XA12_MOSES01,
	CDA_XA12_ANDY4B,
	CDA_XA12_Z_10,
	CDA_XA12_FLYBY4,
	CDA_XA12_RICHCUT1,
	CDA_XA12_ANDY5,
	CDA_XA12_Z_05,
	CDA_XA12_Z_01,
	CDA_XA13_JOBY3,
	CDA_XA13_ANDY7,
	CDA_XA13_ANDREA3B,
	CDA_XA13_COSSACK,
	CDA_XA13_Z_07,
	CDA_XA13_ANDY6,
	CDA_XA13_ANDREA3,
	CDA_XA13_JOBY7,
	CDA_XA14_UWATER1,
	CDA_XA14_JOBY1,
	CDA_XA14_ANDY10,
	CDA_XA14_RICHCUT2,
	CDA_XA14_ANDREA1,
	CDA_XA14_ANDY8,
	CDA_XA14_JOBY6,
	CDA_XA14_ECREDITS,
	CDA_XA15_BOSS_01,
	CDA_XA15_JOBY2,
	CDA_XA15_JOBY4,
	CDA_XA15_JOBY5,
	CDA_XA15_JOBY9,
	CDA_XA15_A_ANDY,
	CDA_XA15_A_ROME,
	CDA_XA15_ANDY2,
	CDA_XA16_JOBY8,
	CDA_XA16_A_SUB_AMB,
	CDA_XA16_JOBY10,
	CDA_XA16_A_HARBOUR_OUT,
	CDA_XA16_A_ANDY_OUT_NORM,
	CDA_XA16_A_ANDY_OUT_SPOOKY,
	CDA_XA16_A_ROME_DAY,
	CDA_XA16_A_UNDERWATER,
	CDA_XA17_A_ROME_NIGHT,
	CDA_XA17_A_VC_SAGA,
	CDA_XA17_A_INDUSTRY,
	CDA_XA17_ANDREA2,
	CDA_XA17_ANDY1,
	CDA_XA17_ANDREA4,
	CDA_XA17_ANDY9,
	CDA_XA17_ANDY11,
};

typedef enum sound_effects
{
	SFX_LARA_FEET,
	SFX_LARA_CLIMB2,
	SFX_LARA_NO,
	SFX_LARA_SLIPPING,
	SFX_LARA_LAND,
	SFX_LARA_CLIMB1,
	SFX_LARA_HOLSTER_DRAW,
	SFX_LARA_HOLSTER_AWAY,
	SFX_LARA_FIRE,
	SFX_LARA_RELOAD,
	SFX_LARA_RICOCHET,
	SFX_PUSH_BLOCK_END,
	SFX_SMASH_GLASS,
	SFX_LARA_CLIMB_WALL_GRUNT,
	SFX_HK_SILENCED,
	SFX_OFFICE_DOOR_OPEN,
	SFX_OFFICE_DOOR_CLSE,
	SFX_LARA_WET_FEET,
	SFX_LARA_WADE,
	SFX_SMASH_WOOD,
	SFX_LARA_INJURY_NONRND,
	SFX_CRICKET_LOOP,
	SFX_PHILOSPHER_STONE,
	SFX_LARA_JUMP_NONRND,
	SFX_LARA_KNEES_SHUFFLE,
	SFX_LARA_HIDDEN_SWITCH,
	SFX_LARA_CLIMB3,
	SFX_LARA_BODYSL,
	SFX_LARA_SHIMMY2,
	SFX_LARA_JUMP_RND,
	SFX_LARA_FALL,
	SFX_LARA_INJURY_RND,
	SFX_LARA_ROLL,
	SFX_LARA_SPLASH,
	SFX_LARA_GETOUT,
	SFX_LARA_SWIM,
	SFX_LARA_BREATH,
	SFX_LARA_BUBBLES,
	SFX_CREATURE_SWIM,
	SFX_LARA_USE_KEY,
	SFX_LARA_PUSH2_NONRND,
	SFX_LARA_GENERAL_DEATH,
	SFX_LARA_KNEES_DEATH,
	SFX_LARA_UZI_FIRE,
	SFX_LARA_UZI_STOP,
	SFX_LARA_SHOTGUN,
	SFX_LARA_PUSH1,
	SFX_LARA_PUSH2_RND,
	SFX_LARA_RANDOM_PUSHFX,
	SFX_LARA_SHOTGUN_SHELL,
	SFX_UNDERWATER_DOOR,
	SFX_LARA_PULL,
	SFX_LARA_FLOATING,
	SFX_LARA_FALLDETH,
	SFX_LARA_GRABHAND,
	SFX_LARA_GRABBODY,
	SFX_LARA_GRABFEET,
	SFX_OFFICE_DOOR_SQUEAK,
	SFX_WATER_LAPS_LOOP,
	SFX_WATER_SPOUT_LOOP,
	SFX_UNDERWATER,
	SFX_UNDERWATER_SWITCH,
	SFX_LARA_PICKUP,
	SFX_PUSHABLE_SOUND,
	SFX_SNIPER_RIFLE,
	SFX_HELICOPTER_LOOP,
	SFX_ROCK_FALL_CRUMBLE,
	SFX_ROCK_FALL_LAND,
	SFX_HK_FIRE,
	SFX_HK_STOP,
	SFX_LARA_THUD,
	SFX_GENERIC_SWOOSH,
	SFX_BLUEGUARD_CHAIR,
	SFX_NEWBOX_SHUT,
	SFX_BLUE_SCIEN_DONT_SHOOT,
	SFX_BLUEGUARD_CONSOLE,
	SFX_SHOWER_LOOP,
	SFX_WATER_LOOP,
	SFX_FOUNTAIN_LOOP,
	SFX_WATERFALL_LOOP,
	SFX_SERVO_02,
	SFX_SERVO_01,
	SFX_BLUEG_GETUP_HOY,
	SFX_BLUEG_MAFIA_GUNS,
	SFX_BLUEG_HIT_DIE,
	SFX_BLUEG_JUMP,
	SFX_BLUEG_CHAIR_HIT,
	SFX_RICH_IRIS_ELEC,
	SFX_BIO_BREATHE_OUT,
	SFX_GENERIC_BOOT_STEPS,
	SFX_PIERRE_GUNS,
	SFX_PIERRE_FINAL_BREATH,
	SFX_PIERRE_HIT,
	SFX_GENERIC_KNEES_FALL,
	SFX_LION_HIT_FLOOR,
	SFX_AGENT_HITMAN_FEET,
	SFX_LARSON_ARGGHH,
	SFX_LARSON_GUNS,
	SFX_LARSON_GROAN,
	SFX_AGENT_HITMAN_LAND,
	SFX_LION_FEET,
	SFX_LION_GROWL,
	SFX_LION_ATTACK,
	SFX_LION_DIE,
	SFX_RAVENSWITCH_EXP,
	SFX_EXPLOSION1,
	SFX_EXPLOSION2,
	SFX_EARTHQUAKE_LOOP,
	SFX_MENU_ROTATE,
	SFX_MENU_SELECT,
	SFX_Menu_Empty,
	SFX_MENU_CHOOSE,
	SFX_TICK_TOCK,
	SFX_Menu_Empty_bis,
	SFX_MENU_COMBINE,
	SFX_Menu_Empty_bis_bis,
	SFX_MENU_MEDI,
	SFX_LARA_CLIMB_WALLS_NOISE,
	SFX_2GUNTEX_FALL_END,
	SFX_AGENT_HITMAN_JUMP,
	SFX_MAFIA_HOLSTER_DRAW,
	SFX_REVOLVER,
	SFX_HITMAN_CHOKE,
	SFX_MAFIA2_DIE,
	SFX_MAFIA2_JUMP_UP,
	SFX_MAFIA2_GETDOWN,
	SFX_MAFIA2_HIT,
	SFX_LARA_ELECTRIC_LOOP,
	SFX_LARA_ELECTRIC_CRACKLES,
	SFX_HANGMAN_LAUGH_OFFCAM,
	SFX_ELEC_LIGHT_CRACKLES,
	SFX_LOUD_WIND_LOOP,
	SFX_KEYPAD_ENTRY_NO,
	SFX_KEYPAD_ENTRY_YES,
	SFX_RICH_TELEPORT,
	SFX_JOBY_PUZZLE_HATCH,
	SFX_SOFT_WIND_LOOP,
	SFX_GLADIATOR_FEET,
	SFX_GLADIATOR_ATTACK,
	SFX_GLADIATOR_DIE,
	SFX_GLADIATOR_SWORD,
	SFX_GLADIATOR_SHIELD1,
	SFX_GLADIATOR_SHIELD2,
	SFX_GENERIC_BODY_FALL,
	SFX_2GUNTEX_DIE,
	SFX_LARA_SPIKE_DEATH,
	SFX_LARA_DEATH3,
	SFX_ROLLING_BALL,
	SFX_BLK_PLAT_RAISE_LOW,
	SFX_2GUNTEX_FALL_BIG,
	SFX_LOOP_FOR_SMALL_FIRES,
	SFX_XRAY_SCAN,
	SFX_R_XRAY_ROOM_LP,
	SFX_R_HITECH_ROOM_LP,
	SFX_JOBY_WATERFALL_SMALL,
	SFX_JOBY_WATERFALL_BIG,
	SFX_RATS_1,
	SFX_BATS_1,
	SFX_H_GOD_HAMMER_QT,
	SFX_H_GOD_HAMMER_MED,
	SFX_H_GOD_HAMMER_LD,
	SFX_TRAPDOOR_OPEN,
	SFX_TRAPDOOR_CLOSE,
	SFX_RICH_DOOR_BEAM,
	SFX_WALLSUIT_OUT,
	SFX_AUTOGUN_UNFOLD,
	SFX_AUTOGUN_DOOR,
	SFX_SMASH_ROCK,
	SFX_SMASH_METAL,
	SFX_D_ALIEN,
	SFX_RICH_VENT_IMPACT,
	SFX_SWITCH_ELEC_SWAP,
	SFX_ELECTRIC_WIRES,
	SFX_LAVA_LOOP,
	SFX_DOG_DOBER_GROWL,
	SFX_DOG_HOWL,
	SFX_DOG_ATTACK_1,
	SFX_DOG_AWARE,
	SFX_DOG_FOOT_1,
	SFX_DOG_JUMP,
	SFX_DOG_LONG_GROWL,
	SFX_DOG_DEATH,
	SFX_THUNDER_RUMBLE,
	SFX_HAMMER_GOD_PULSE,
	SFX_SWORD_GOD_CHARGE,
	SFX_SWORD_GOD_LASER,
	SFX_SKELETON_APPEAR,
	SFX_CHEF_KNIFE_SWOOSH,
	SFX_SKELETON_ATTACK,
	SFX_GEN_SWORD_SWOOSH_NORM,
	SFX_CHEF_ATTACK_ARGHH,
	SFX_2GUNTEX_FEET_LD,
	SFX_2GUNTEX_FEET_QT,
	SFX_2GUNTEX_LASER_FIRE,
	SFX_2GUNTEX_LASER_FIREx2,
	SFX_2GUNTEX_LASER_START,
	SFX_2GUNTEX_LASER_MISFIRE,
	SFX_2GUNTEX_HIT_GUNS,
	SFX_IMP_DIE,
	SFX_IMP_FALL,
	SFX_IMP_FEET,
	SFX_IMP_ATTACK,
	SFX_IMP_LAUGH,
	SFX_IMP_RUNAWAY,
	SFX_SGOD_SWD_DIE1_Q,
	SFX_SGOD_SWD_DIE2_VQ,
	SFX_HITMAN_GUNS_FIRE,
	SFX_HITMAN_GUNS_END,
	SFX_FIRE_EXTING_RICO,
	SFX_LARA_UNDERWATER_ENGINE,
	SFX_CABINET_CLOSE_WOOD,
	SFX_CABINET_OPEN_WOOD,
	SFX_SWORD_GOD_FEET_LD,
	SFX_SWORD_GOD_FEET_QT,
	SFX_SWORD_GOD_FEET_VQT,
	SFX_SWORD_GOD_SWORD,
	SFX_GEN_SWORD_SWOOSH_LOW,
	SFX_SWORD_GOD_FALL,
	SFX_SWORD_GOD_HITMET,
	SFX_SWORD_GOD_SCREAM,
	SFX_KITCHEN_HOB_LOOP,
	SFX_KEYPAD_STAR,
	SFX_KEYPAD_HASH,
	SFX_KEYPAD_0,
	SFX_KEYPAD_1,
	SFX_KEYPAD_2,
	SFX_KEYPAD_3,
	SFX_KEYPAD_4,
	SFX_KEYPAD_5,
	SFX_KEYPAD_6,
	SFX_KEYPAD_7,
	SFX_KEYPAD_8,
	SFX_KEYPAD_9,
	SFX_SMALL_FAN,
	SFX_KLAXON,
	SFX_LARA_CROSSBOW,
	SFX_ANDY_FLOOR_DOOR_B,
	SFX_ANDY_FLOOR_DOOR_A,
	SFX_J_GRAB_OPEN,
	SFX_WILLOWISP_LOOP,
	SFX_LEAP_SWITCH,
	SFX_LARGE_SWITCH,
	SFX_GENERIC_HEAVY_THUD,
	SFX_GENERIC_HEAVY_FEET,
	SFX_SWIMSUIT_METAL_CLASH,
	SFX_CROW_WALL_ITEM_DROP,
	SFX_LARA_SUB_BREATHE,
	SFX_HITMAN_ELEC_SHORT,
	SFX_WELD_THRU_DOOR_LOOP,
	SFX_IMP_BARREL_DROP,
	SFX_IMP_BARREL_ROLL,
	SFX_IMP_STONE_HIT,
	SFX_POUR_DUST_INSERTANIM,
	SFX_J_GRAB_TOP_IMPACT,
	SFX_COG_ANDY2,
	SFX_J_GRAB_IMPACT,
	SFX_J_GRAB_MOTOR_C,
	SFX_J_GRAB_MOTOR_B_LP,
	SFX_J_GRAB_MOTOR_A,
	SFX_J_GRAB_DROP,
	SFX_J_GRAB_WINCH_UP_LP,
	SFX_FOOTSTEPS_WOOD_CAB,
	SFX_FOOTSTEPS_METAL_CAB,
	SFX_D_FLOOR_METAL,
	SFX_DRAWERS_METAL_OPEN,
	SFX_DRAWERS_METAL_CLOSE,
	SFX_SHOCKWAVE_RUMB,
	SFX_ANDY_BOAT_MILL_2,
	SFX_ANDY_BOAT_MILL_1,
	SFX_SMALL_STONE_SWITCH,
	SFX_TV_WHITENOISE_LOOP,
	SFX_LIFT_MOVE,
	SFX_ALARM,
	SFX_GENERIC_SQKS_LOW,
	SFX_RICH_LOWER_BEAMX,
	SFX_RICH_NRG_BEAM,
	SFX_BEETLES,
	SFX_GOD_HEAD_TESTICLES,
	SFX_GOD_HEAD_CHARGE,
	SFX_GOD_HEAD_BLAST,
	SFX_GOD_HEAD_LASER_LOOPS,
	SFX_COGS_ROME,
	SFX_LOW_RUMBLE_LOWER,
	SFX_GEN_STONE_DOOR_LOW,
	SFX_GENERIC_SQKS_NONRND,
	SFX_UNDERWATER_FAN_ON,
	SFX_SUB_CONTROLROOM_LOOP,
	SFX_D_MANHOLE_METAL,
	SFX_FOOTSTEPS_MUD,
	SFX_FOOTSTEPS_ICE,
	SFX_FOOTSTEPS_GRAVEL,
	SFX_FOOTSTEPS_SAND_GRASS,
	SFX_FOOTSTEPS_WOOD,
	SFX_FOOTSTEPS_SNOW,
	SFX_FOOTSTEPS_METAL,
	SFX_TIGHTROPE_CREAK,
	SFX_TIGHTROPE_WALK,
	SFX_DRAWERS_WOOD_CLSE,
	SFX_DRAWERS_WOOD_OPEN,
	SFX_SWAMPY_ATTACK,
	SFX_BELL,
	SFX_JOBY2_GRILL_2a,
	SFX_HISS_LOOP_SMALL,
	SFX_JOBY2_CRANE_END,
	SFX_GENERATOR_HUM_LOOP,
	SFX_COMPUTER_BEEPIES,
	SFX_MODEM_LOOP,
	SFX_AIRCON_LOOP,
	SFX_GEN_STONE_DOOR,
	SFX_LOW_RUMBLE,
	SFX_SEARCHER,
	SFX_STAIR_SNAP_01,
	SFX_RICH_GEN_LOOP,
	SFX_ZOOM_VIEW_WHIRR,
	SFX_RICH_VENT_STRESS,
	SFX_INDUSTRY_AMBIENCE_LOOP,
	SFX_D_SWIPECARD_SFX,
	SFX_RICH_TARGET_RISE,
	SFX_STAIR_BANISTER_BEND,
	SFX_JOBY2_GRILL_1,
	SFX_JOBY2_GRILL_3,
	SFX_D_LARA_LOCKER_C,
	SFX_D_LARA_LOCKER_O,
	SFX_STONE_SCRAPE_FAST,
	SFX_JOBY2_GRILL_2,
	SFX_LIFT_DOORS,
	SFX_LARA_CROWBAR_ITEM,
	SFX_JOBY2_CRANE,
	SFX_GENERIC_SQKS_RND,
	SFX_D_SLIDEDOOR_JOBY,
	SFX_D_FIREDOOR_JOBY,
	SFX_D_METAL_CLS_P1,
	SFX_D_METAL_CLS_P2,
	SFX_D_SCIFI_CLOSE,
	SFX_D_SCIFI_OPEN,
	SFX_TEETH_SPIKES,
	SFX_BAT_RAM_CREAK1,
	SFX_LARA_LEVER_PART1,
	SFX_LARA_LEVER_PART2,
	SFX_LARA_POLE_SQUEAKS,
	SFX_LARA_ROPEDOWN_LOOP,
	SFX_D_METAL_OPEN,
	SFX_LARA_PULLEY,
	SFX_D_METAL_OPEN_PUSH,
	SFX_D_METAL_KICKOPEN,
	SFX_LARA_USE_OBJECT,
	SFX_D_GATE_BIG_METAL,
	SFX_D_MOTOR_WHIRR,
	SFX_LARA_NO_FRENCH,
	SFX_LARA_NO_JAPANESE,
	// at least good to here
	SFX_LARA_CROW_WRENCH,
	SFX_LARA_ROPE_CREAK,
	SFX_LARA_TROPE_FALL,
	SFX_D_METAL_CAGE_CLS2,
	SFX_D_METAL_CAGE_CLS1,
	SFX_GENERIC_STONE_GRIND,
	SFX_D_STONEMETAL_MAIN,
	SFX_D_PORTCULLIS_UP,
	SFX_D_PORTCULLIS_DOWN,
	SFX_D_WOODEN_GATE_CREAK,
	SFX_D_PADLOCK_BREAK,
	SFX_D_BIG_THUD_GEN_2,
	SFX_D_BIG_THUD_GEN_1,
	SFX_D_DOUBLE_BANG,
	SFX_D_DOUBLE_CREAK,
	SFX_D_CROWBAR_WOOD,
	SFX_D_METAL_LOCKUP_CROW,
	SFX_D_METAL_LOCKUP_OPEN,
	SFX_D_METAL_CAGE_OPEN,
	SFX_SMALL_CLICK_SWITCH,
	SFX_WEREWOLF_FEET_F,
	SFX_WEREWOLF_FEET_B,
	SFX_WEREWOLF_ATTACK1,
	SFX_WEREWOLF_ATTACK2,
	SFX_WEREWOLF_DROP,
	SFX_WEREWOLF_DIE,
	SFX_WEREWOLF_JUMP,
	SFX_SWAT_DIE,
	SFX_ANDY_WHEEL,
	SFX_BARN_DOOR_NORMAL,
	SFX_BARN_DOOR_SLAM,
	SFX_RATCHET_3SHOT,
	SFX_ANDY_WATERWHEEL,
	SFX_ANDY_INSIDEMILL,
	SFX_ANDREA_COIN_PUZZ,
	SFX_MINI_SUB_LOOP,
	SFX_LIFT_BRAKE_SQUEAL,
	SFX_RATSPLASH,
	SFX_SNORE_IN,
	SFX_SNORE_OUT,
	SFX_D_SMALL_VENT_O_C,
	SFX_RICH_HOLOGRAM,
	SFX_HISS_LOOP_BIG,
	SFX_LIFT_MOVE_KNACKED,
	SFX_LIFT_HIT_FLOOR1,
	SFX_LIFT_HIT_FLOOR2,
	SFX_LAZER_LOOP,
	SFX_UNDERWATER_EXPLOSION,
	SFX_UNDERWATER_TORPEDO,
	SFX_TELEPORT_CRACKLES,
	SFX_TELEPORT_FLASH,
	SFX_LARA_UNDERWATER_HIT,
	SFX_RAVESTICK,
	SFX_UNDERWATER_CHAFF,
	SFX_FISHTANK_WATER,
	SFX_2GUNTEX_STAIR_FALL,
	SFX_GGOD_BREATHE_IN,
	SFX_GGOD_BREATHE_OUT,
	SFX_GGOD_FIRE,
	SFX_GGOD_ATTACK,
	SFX_PORTAL_LOOP,

	NUM_SFX
};

#define SOUND_BASS_UNITS			1.0f / 1024.0f	// TR->BASS distance unit coefficient
#define SOUND_MAXVOL_RADIUS			1024.0f			// Max. volume hearing distance
#define SOUND_OMNIPRESENT_ORIGIN    D3DXVECTOR3(1.17549e-038f, 1.17549e-038f, 1.17549e-038f)

#define SOUND_MAX_SAMPLES 3072 // Original was 1024, reallocate original 3-byte DX handle struct to just 1-byte memory pointer
#define SOUND_MAX_CHANNELS  32 // Original was 24, reallocate original 36-byte struct with 24-byte SoundEffectSlot struct

#define SOUND_LEGACY_SOUNDMAP_SIZE	 450
#define SOUND_LEGACY_TRACKTABLE_SIZE 136

#define SOUND_FLAG_NO_PAN			(1<<12)	// Unused flag
#define SOUND_FLAG_RND_PITCH		(1<<13)
#define SOUND_FLAG_RND_GAIN			(1<<14)

#define SOUND_MAX_PITCH_CHANGE		0.09f
#define SOUND_MAX_GAIN_CHANGE		0.0625f

#define SOUND_32BIT_SILENCE_LEVEL	4.9e-04f

#define SOUND_SAMPLE_FLAGS			(BASS_SAMPLE_MONO | BASS_SAMPLE_FLOAT)

#define SOUND_XFADETIME_BGM			5000
#define SOUND_XFADETIME_BGM_START	1500
#define SOUND_XFADETIME_ONESHOT		200
#define SOUND_XFADETIME_CUTSOUND	100
#define SOUND_XFADETIME_HIJACKSOUND	50

#define SOUND_BGM_DAMP_COEFFICIENT	0.6f

typedef struct SoundEffectSlot
{
	short state;
	short effectID;
	float gain;
	HCHANNEL channel;
	D3DXVECTOR3 origin;
};

typedef struct SoundTrackSlot
{
	HSTREAM channel;
	short   trackID;
};

enum sound_track_types
{
	SOUND_TRACK_ONESHOT,
	SOUND_TRACK_BGM,

	NUM_SOUND_TRACK_TYPES
};

enum sound_filters
{
	SOUND_FILTER_REVERB,
	SOUND_FILTER_COMPRESSOR,
	SOUND_FILTER_LOWPASS,

	NUM_SOUND_FILTERS
};

enum sound_states
{
	SOUND_STATE_IDLE,
	SOUND_STATE_ENDING,
	SOUND_STATE_ENDED
};

enum sound_flags
{ 
	SOUND_NORMAL, 
	SOUND_WAIT, 
	SOUND_RESTART, 
	SOUND_LOOPED 
};

enum reverb_type
{
	RVB_OUTSIDE,	   // 0x00   no reverberation
	RVB_SMALL_ROOM,	   // 0x01   little reverberation
	RVB_MEDIUM_ROOM,   // 0x02
	RVB_LARGE_ROOM,	   // 0x03
	RVB_PIPE,		   // 0x04   highest reverberation, almost never used

	NUM_REVERB_TYPES
};

#define SayNo ((void (__cdecl*)()) 0x004790E0)

long __cdecl SoundEffect(__int32 effectID, PHD_3DPOS* position, __int32 env_flags);
void __cdecl StopSoundEffect(__int16 effectID);
bool __cdecl Sound_LoadSample(char *buffer, __int32 compSize, __int32 uncompSize, __int32 currentIndex);
void __cdecl Sound_FreeSamples();
void __cdecl SOUND_Stop();
void __cdecl S_CDPlay(short index, unsigned int mode);
void __cdecl S_CDPlayEx(short index, DWORD mask, DWORD unknown);
void __cdecl S_CDStop();

static void CALLBACK Sound_FinishOneshotTrack(HSYNC handle, DWORD channel, DWORD data, void* userData);

void  Sound_Init();
void  Sound_DeInit();
bool  Sound_CheckBASSError(char* message, bool verbose, ...);
void  Sound_UpdateScene();
void  Sound_FreeSample(__int32 index);
int   Sound_GetFreeSlot();
void  Sound_FreeSlot(int index, unsigned int fadeout = 0);
int   Sound_EffectIsPlaying(int effectID, PHD_3DPOS *position);
float Sound_DistanceToListener(PHD_3DPOS *position);
float Sound_DistanceToListener(D3DXVECTOR3 position);
float Sound_Attenuate(float gain, float distance, float radius);
bool  Sound_UpdateEffectPosition(int index, PHD_3DPOS *position, bool force = false);

void Inject_Sound();