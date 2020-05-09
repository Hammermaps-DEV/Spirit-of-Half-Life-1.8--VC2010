#pragma once
/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#ifndef GAME_SERVER_ENTITIES_NPCS_COTIS_H
#define GAME_SERVER_ENTITIES_NPCS_COTIS_H

#ifndef GAME_SERVER_ENTITIES_NPCS_CTALKMONSTER_H
#include "CTalkMonster.h"
#endif

//=========================================================
// monster-specific DEFINE's
//=========================================================
#define	OTIS_CLIP_SIZE 9
#define OTIS_VOL 0.40
#define OTIS_ATTN ATTN_NORM

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
enum { SCHED_OTIS_COVER_AND_RELOAD };

#define OTIS_NUM_HEADS	2

#define	OTIS_AE_DRAW		( 2 )
#define	OTIS_AE_SHOOT		( 3 )
#define	OTIS_AE_HOLSTER		( 4 )
#define	OTIS_AE_RELOAD		( 5 )

#define OT_GUN_GROUP		1
#define OT_GUN_HOLSTERED	0
#define OT_GUN_DRAWN 		1
#define OT_GUN_NONE			2

#define	OT_HEAD_GROUP		2
#define	OT_HEAD_NORMAL		0
#define OT_HEAD_BALD		1

//=========================================================
// monster-specific conditions
//=========================================================
class COtis : public CTalkMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int ISoundMask() override;
	void Eagle();
	void AlertSound() override;
	int Classify() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void CheckAmmo() override;

	void RunTask(Task_t* pTask) override;
	void StartTask(Task_t* pTask) override;

	int ObjectCaps() override { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	BOOL CheckRangeAttack1(float flDot, float flDist) override;
	void KeyValue(KeyValueData* pkvd) override;

	void DeclineFollowing() override;

	// Override these to set behavior
	Schedule_t* GetScheduleOfType(int Type) override;
	Schedule_t* GetSchedule() override;
	MONSTERSTATE GetIdealState() override;

	void DeathSound() override;
	void PainSound() override;

	void TalkInit();

	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr,
		int bitsDamageType) override;
	void Killed(entvars_t* pevAttacker, int iGib) override;

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	BOOL m_fHostile;
	int m_iBaseBody; //LRC - for barneys with different bodies
	BOOL m_fGunDrawn;
	float m_painTime;
	float m_checkAttackTime;
	BOOL m_lastAttackCheck;
	int m_cClipSize;
	float m_flPlayerDamage; // how much pain has the player inflicted on me?
	int m_iHead;

	CUSTOM_SCHEDULES;
};

#endif //GAME_SERVER_ENTITIES_NPCS_COTIS_H