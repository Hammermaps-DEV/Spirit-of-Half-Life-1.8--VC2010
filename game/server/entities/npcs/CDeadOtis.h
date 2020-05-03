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

#ifndef GAME_SERVER_ENTITIES_NPCS_CDEADOTIS_H
#define GAME_SERVER_ENTITIES_NPCS_CDEADOTIS_H

class CDeadOtis : public CBaseMonster
{
public:
	void Spawn() override;
	int Classify() override { return CLASS_PLAYER_ALLY; }

	void KeyValue(KeyValueData* pkvd) override;

	int m_iPose; // which sequence to display	-- temporary, don't need to save
	static char* m_szPoses[3];
};

#endif //GAME_SERVER_ENTITIES_NPCS_CDEADOTIS_H