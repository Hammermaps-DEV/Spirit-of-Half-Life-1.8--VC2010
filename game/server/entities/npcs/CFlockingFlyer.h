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

#ifndef GAME_SERVER_ENTITIES_NPCS_CFLOCKINGFLYER_H
#define GAME_SERVER_ENTITIES_NPCS_CFLOCKINGFLYER_H

class CFlockingFlyer : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SpawnCommonCode();
	void EXPORT IdleThink();
	void BoidAdvanceFrame();
	void EXPORT FormFlock();
	void EXPORT Start();
	void EXPORT FlockLeaderThink();
	void EXPORT FlockFollowerThink();
	void EXPORT FallHack();
	void MakeSound();
	void AlertFlock();
	void SpreadFlock();
	void SpreadFlock2();
	void Killed(entvars_t* pevAttacker, int iGib) override;
	void Poop();
	BOOL FPathBlocked();
	//void KeyValue( KeyValueData *pkvd );

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int IsLeader() { return m_pSquadLeader == this; }
	int InSquad() { return m_pSquadLeader != nullptr; }
	int SquadCount();
	void SquadRemove(CFlockingFlyer* pRemove);
	void SquadUnlink();
	void SquadAdd(CFlockingFlyer* pAdd);
	void SquadDisband();

	CFlockingFlyer* m_pSquadLeader;
	CFlockingFlyer* m_pSquadNext;
	BOOL m_fTurning; // is this boid turning?
	BOOL m_fCourseAdjust; // followers set this flag TRUE to override flocking while they avoid something
	BOOL m_fPathBlocked; // TRUE if there is an obstacle ahead
	Vector m_vecReferencePoint; // last place we saw leader
	Vector m_vecAdjustedVelocity; // adjusted velocity (used when fCourseAdjust is TRUE)
	float m_flGoalSpeed;
	float m_flLastBlockedTime;
	float m_flFakeBlockedTime;
	float m_flAlertTime;
	float m_flFlockNextSoundTime;
};

#endif //GAME_SERVER_ENTITIES_NPCS_CFLOCKINGFLYER_H