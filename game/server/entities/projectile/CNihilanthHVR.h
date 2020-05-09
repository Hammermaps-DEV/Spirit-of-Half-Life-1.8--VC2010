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

#ifndef GAME_SERVER_ENTITIES_PROJECTILE_CNIHILANTHHVR_H
#define GAME_SERVER_ENTITIES_PROJECTILE_CNIHILANTHHVR_H

#define N_SCALE		15
#define N_SPHERES	20

class CNihilanth;

class CNihilanthHVR : public CBaseMonster
{
public:
	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;
	void Precache() override;

	void CircleInit(CBaseEntity* pTarget);
	void AbsorbInit();
	void TeleportInit(CNihilanth* pOwner, CBaseEntity* pEnemy, CBaseEntity* pTarget, CBaseEntity* pTouch);
	void GreenBallInit();
	void ZapInit(CBaseEntity* pEnemy);

	void EXPORT HoverThink();
	BOOL CircleTarget(Vector vecTarget);
	void EXPORT DissipateThink();

	void EXPORT ZapThink();
	void EXPORT TeleportThink();
	void EXPORT TeleportTouch(CBaseEntity* pOther);

	void EXPORT RemoveTouch(CBaseEntity* pOther);
	void EXPORT BounceTouch(CBaseEntity* pOther);
	void EXPORT ZapTouch(CBaseEntity* pOther);

	CBaseEntity* RandomClassname(const char* szName);

	// void EXPORT SphereUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void MovetoTarget(Vector vecTarget);
	virtual void Crawl();

	void Zap();
	void Teleport();

	float m_flIdealVel;
	Vector m_vecIdeal;
	CNihilanth* m_pNihilanth;
	EHANDLE m_hTouch;
	int m_nFrames;
};

#endif //GAME_SERVER_ENTITIES_PROJECTILE_CNIHILANTHHVR_H