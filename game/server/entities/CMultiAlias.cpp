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
*   NEW file for the Mod "Spirit of Half-Life", by Laurie R. Cheers. (LRC)
*   Created 19/11/00
*   Modified by Andrew J Hamilton (AJH) 2/04/04
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "CBaseMutableAlias.h"
#include "CMultiAlias.h"

/*********************
* Worldcraft entity: multi_alias
*
* targetname- name
* other values are handled in a multi_manager-like way.
**********************/

LINK_ENTITY_TO_CLASS(multi_alias, CMultiAlias);

TYPEDESCRIPTION CMultiAlias::m_SaveData[] =
{
	DEFINE_FIELD(CMultiAlias, m_cTargets, FIELD_INTEGER),
	DEFINE_ARRAY(CMultiAlias, m_iszTargets, FIELD_STRING, MAX_MULTI_TARGETS),
	DEFINE_FIELD(CMultiAlias, m_iTotalValue, FIELD_INTEGER),
	DEFINE_ARRAY(CMultiAlias, m_iValues, FIELD_INTEGER, MAX_MULTI_TARGETS),
	DEFINE_FIELD(CMultiAlias, m_iMode, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CMultiAlias, CBaseMutableAlias);

void CMultiAlias::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iMode"))
	{
		m_iMode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	// this assumes that additional fields are targetnames and their values are probability values.
	else if (m_cTargets < MAX_MULTI_TARGETS)
	{
		char tmp[128];
		UTIL_StripToken(pkvd->szKeyName, tmp);

		m_iszTargets[m_cTargets] = ALLOC_STRING(tmp);
		m_iValues[m_cTargets] = atoi(pkvd->szValue);

		m_iTotalValue += m_iValues[m_cTargets];
		m_cTargets++;

		pkvd->fHandled = TRUE;
	}
	else
	{
		ALERT(at_error, "Too many targets for multi_alias %s (limit is %d)\n", STRING(pev->targetname),
			MAX_MULTI_TARGETS);
	}
}

CBaseEntity* CMultiAlias::FollowAlias(CBaseEntity* pStartEntity)
{
	CBaseEntity* pBestEntity = nullptr; // the entity we're currently planning to return.
	int iBestOffset = -1; // the offset of that entity.

	int i = 0;
	if (m_iMode)
	{
		// During any given 'game moment', this code may be called more than once. It must use the
		// same random values each time (because otherwise it gets really messy). I'm using srand
		// to arrange this.
		srand(static_cast<int>(gpGlobals->time * 100));
		rand(); // throw away the first result - it's just the seed value
		if (m_iMode == 1) // 'choose one' mode
		{
			int iRandom = 1 + (rand() % m_iTotalValue);
			for (i = 0; i < m_cTargets; i++)
			{
				iRandom -= m_iValues[i];
				if (iRandom <= 0)
					break;
			}
		}
		else // 'percent chance' mode
		{
			for (i = 0; i < m_cTargets; i++)
			{
				if (m_iValues[i] >= rand() % 100)
					break;
			}
		}
	}

	while (i < m_cTargets)
	{
		CBaseEntity* pTempEntity = UTIL_FindEntityByTargetname(pStartEntity, STRING(m_iszTargets[i]));
		if (pTempEntity)
		{
			// We've found an entity; only use it if its offset is lower than the offset we've currently got.
			int iTempOffset = OFFSET(pTempEntity->pev);
			if (iBestOffset == -1 || iTempOffset < iBestOffset)
			{
				iBestOffset = iTempOffset;
				pBestEntity = pTempEntity;
			}
		}
		if (m_iMode == 1)
			break; // if it's in "pick one" mode, stop after the first.
		if (m_iMode == 2)
		{
			i++;
			// if it's in "percent chance" mode, try to find another one to fire.
			while (i < m_cTargets)
			{
				if (m_iValues[i] > rand() % 100)
					break;
				i++;
			}
		}
		else
			i++;
	}

	return pBestEntity;
}