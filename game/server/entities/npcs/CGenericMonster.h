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

#ifndef GAME_SERVER_ENTITIES_NPCS_CGENERICMONSTER_H
#define GAME_SERVER_ENTITIES_NPCS_CGENERICMONSTER_H

// For holograms, make them not solid so the player can walk through them
//LRC- this seems to interfere with SF_MONSTER_CLIP
#define	SF_GENERICMONSTER_NOTSOLID				4
#define	SF_HEAD_CONTROLLER					8
#define SF_GENERICMONSTER_INVULNERABLE				32
#define SF_GENERICMONSTER_PLAYERMODEL				64

class CGenericMonster : public CTalkMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override;
	int ISoundMask() override;
	void KeyValue(KeyValueData* pkvd) override;

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int HasCustomGibs() override { return m_iszGibModel; }

	int m_iszGibModel;
};

#endif //GAME_SERVER_ENTITIES_NPCS_CGENERICMONSTER_H