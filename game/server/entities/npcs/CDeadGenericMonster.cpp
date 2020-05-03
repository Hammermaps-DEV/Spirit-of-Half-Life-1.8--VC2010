/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include "CDeadGenericMonster.h"

LINK_ENTITY_TO_CLASS(monster_generic_dead, CDeadGenericMonster);

TYPEDESCRIPTION CDeadGenericMonster::m_SaveData[] =
{
	DEFINE_FIELD(CDeadGenericMonster, m_iszGibModel, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CDeadGenericMonster, CBaseMonster);

void CDeadGenericMonster::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_bloodColor"))
	{
		m_bloodColor = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszGibModel"))
	{
		m_iszGibModel = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

//=========================================================
// ********** DeadGenericMonster SPAWN **********
//=========================================================
void CDeadGenericMonster::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->effects = 0;
	pev->yaw_speed = 8; //LRC -- what?
	pev->sequence = 0;

	if (pev->netname)
	{
		pev->sequence = LookupSequence(STRING(pev->netname));

		if (pev->sequence == -1)
		{
			ALERT(at_debug, "Invalid sequence name \"%s\" in monster_generic_dead\n", STRING(pev->netname));
		}
	}
	else
	{
		pev->sequence = LookupActivity(pev->frags);
	}

	// Corpses have less health
	pev->health = 8;

	MonsterInitDead();

	ResetSequenceInfo();
	pev->frame = 255; // pose at the _end_ of its death sequence.
}

void CDeadGenericMonster::Precache()
{
	PRECACHE_MODEL((char*)STRING(pev->model));
	if (m_iszGibModel)
		PRECACHE_MODEL((char*)STRING(m_iszGibModel)); //LRC
}
