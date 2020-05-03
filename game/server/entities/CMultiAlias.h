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

#ifndef GAME_SERVER_ENTITIES_CMULTIALIAS_H
#define GAME_SERVER_ENTITIES_CMULTIALIAS_H

#define MAX_MULTI_TARGETS	16 // maximum number of targets a single multi_manager entity may be assigned.

class CMultiAlias : public CBaseMutableAlias
{
public:
	void KeyValue(KeyValueData* pkvd) override;

	virtual int Save(CSave& save);
	virtual int Restore(CRestore& restore);

	static TYPEDESCRIPTION m_SaveData[];

	CBaseEntity* FollowAlias(CBaseEntity* pFrom) override;

	int m_cTargets;
	int m_iszTargets[MAX_MULTI_TARGETS];
	int m_iTotalValue;
	int m_iValues[MAX_MULTI_TARGETS];
	int m_iMode;
};

#endif //GAME_SERVER_ENTITIES_CMULTIALIAS_H