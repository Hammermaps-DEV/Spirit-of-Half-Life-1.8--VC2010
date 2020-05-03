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

#ifndef GAME_SERVER_ENTITIES_NPCS_CBULLSQUID_H
#define GAME_SERVER_ENTITIES_NPCS_CBULLSQUID_H

#define		BSQUID_AE_SPIT		( 1 )
#define		BSQUID_AE_BITE		( 2 )
#define		BSQUID_AE_BLINK		( 3 )
#define		BSQUID_AE_TAILWHIP	( 4 )
#define		BSQUID_AE_HOP		( 5 )
#define		BSQUID_AE_THROW		( 6 )

#define		SQUID_SPRINT_DIST	256 // how close the squid has to get before starting to sprint and refusing to swerve

class CBullsquid : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int ISoundMask() override;
	int Classify() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void IdleSound() override;
	void PainSound() override;
	void DeathSound() override;
	void AlertSound() override;
	void AttackSound();
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	BOOL CheckMeleeAttack1(float flDot, float flDist) override;
	BOOL CheckMeleeAttack2(float flDot, float flDist) override;
	BOOL CheckRangeAttack1(float flDot, float flDist) override;
	void RunAI() override;
	BOOL FValidateHintType(short sHint) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	int IRelationship(CBaseEntity* pTarget) override;
	int IgnoreConditions() override;
	MONSTERSTATE GetIdealState() override;

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	BOOL m_fCanThreatDisplay; // this is so the squid only does the "I see a headcrab!" dance one time. 

	float m_flLastHurtTime;
	// we keep track of this, because if something hurts a squid, it will forget about its love of headcrabs for a while.
	float m_flNextSpitTime; // last time the bullsquid used the spit attack.

	int iSquidSpitSprite;
};

#endif //GAME_SERVER_ENTITIES_NPCS_CBULLSQUID_H