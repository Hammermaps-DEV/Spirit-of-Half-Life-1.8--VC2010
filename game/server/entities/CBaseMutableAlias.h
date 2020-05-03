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

#ifndef GAME_SERVER_ENTITIES_CBASEMUTABLEALIAS_H
#define GAME_SERVER_ENTITIES_CBASEMUTABLEALIAS_H

//LRC- moved here from alias.cpp so that util functions can use these defs.
class CBaseMutableAlias : public CPointEntity
{
public:
	BOOL IsMutableAlias() { return TRUE; };
	virtual CBaseEntity* FollowAlias(CBaseEntity* pFrom) { return NULL; };

	virtual void ChangeValue(int iszValue)
	{
		ALERT(at_error, "%s entities cannot change value!", STRING(pev->classname));
	}

	virtual void ChangeValue(CBaseEntity* pValue) { ChangeValue(pValue->pev->targetname); }

	virtual void FlushChanges()
	{
	};

	virtual int Save(CSave& save);
	virtual int Restore(CRestore& restore);
	static TYPEDESCRIPTION m_SaveData[];

	CBaseMutableAlias* m_pNextAlias;
};

#endif //GAME_SERVER_ENTITIES_CBASEMUTABLEALIAS_H