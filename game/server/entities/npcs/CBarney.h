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

#ifndef GAME_SERVER_ENTITIES_NPCS_CBARNEY_H
#define GAME_SERVER_ENTITIES_NPCS_CBARNEY_H

enum
{
	SCHED_BARNEY_COVER_AND_RELOAD
};

#define	BARNEY_AE_DRAW		( 2 )
#define	BARNEY_AE_SHOOT		( 3 )
#define	BARNEY_AE_HOLSTER	( 4 )
#define	BARNEY_AE_RELOAD	( 5 )

#define	BARNEY_BODY_GUNHOLSTERED	0
#define	BARNEY_BODY_GUNDRAWN		1
#define BARNEY_BODY_GUNGONE			2

class CBarney : public CTalkMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int ISoundMask() override;
	void BarneyFirePistol();
	void CheckAmmo() override;
	void AlertSound() override;
	
	int Classify() override
	{
		return m_iClass ? m_iClass : CLASS_PLAYER_ALLY;
	}
	
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;

	void RunTask(Task_t* pTask) override;
	
	void StartTask(Task_t* pTask) override
	{
		CTalkMonster::StartTask(pTask);
	}
	
	int ObjectCaps() override { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	BOOL CheckRangeAttack1(float flDot, float flDist) override;

	void DeclineFollowing() override;

	// Override these to set behavior
	Schedule_t* GetScheduleOfType(int Type) override;
	Schedule_t* GetSchedule() override;
	MONSTERSTATE GetIdealState() override;

	void DeathSound() override;
	void PainSound() override;

	void TalkInit() override;

	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr,
	                 int bitsDamageType) override;
	void Killed(entvars_t* pevAttacker, int iGib) override;

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int m_iBaseBody; //LRC - for barneys with different bodies
	BOOL m_fGunDrawn;
	float m_painTime;
	float m_checkAttackTime;
	BOOL m_lastAttackCheck;
	int m_cClipSize;
	float m_flPlayerDamage; // how much pain has the player inflicted on me?

	CUSTOM_SCHEDULES;
};

#endif //GAME_SERVER_ENTITIES_NPCS_CBARNEY_H
