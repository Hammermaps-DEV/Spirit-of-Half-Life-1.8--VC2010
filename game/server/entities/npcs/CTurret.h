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

#ifndef GAME_SERVER_ENTITIES_NPCS_CTURRET_H
#define GAME_SERVER_ENTITIES_NPCS_CTURRET_H

#define TURRET_SPREAD	Vector( 0, 0, 0 )
#define TURRET_MAXSPIN	5		// seconds turret barrel will spin w/o a target

class CTurret : public CBaseTurret
{
public:
	void Spawn() override;
	void Precache() override;
	// Think functions
	void SpinUpCall() override;
	void SpinDownCall() override;

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	// other functions
	void Shoot(Vector& vecSrc, Vector& vecDirToEnemy) override;

private:
	int m_iStartSpin;
};

#endif //GAME_SERVER_ENTITIES_NPCS_CTURRET_H