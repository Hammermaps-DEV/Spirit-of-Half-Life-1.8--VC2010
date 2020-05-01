/***
*
* NEW file for the Mod "Spirit of Half-Life", by Laurie R. Cheers. (LRC)
* Created 19/11/00
* Modified by Andrew J Hamilton (AJH) 2/04/04
*
***/
/*

===== alias.cpp ========================================================

Alias entities, replace the less powerful and (IMHO) less intuitive
trigger_changetarget entity.

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

TYPEDESCRIPTION CBaseMutableAlias::m_SaveData[] =
{
	DEFINE_FIELD(CBaseMutableAlias, m_pNextAlias, FIELD_CLASSPTR),
};
IMPLEMENT_SAVERESTORE(CBaseMutableAlias, CPointEntity);

/*********************
* Worldcraft entity: info_group
*
* targetname- name
* target-     alias entity to affect
* other values are handled in a multi_manager-like way.
**********************/
// definition in cbase.h

#define SF_GROUP_DEBUG 2

LINK_ENTITY_TO_CLASS(info_group, CInfoGroup);

TYPEDESCRIPTION CInfoGroup::m_SaveData[] =
{
	DEFINE_FIELD(CInfoGroup, m_cMembers, FIELD_INTEGER),
	DEFINE_ARRAY(CInfoGroup, m_iszMemberName, FIELD_STRING, MAX_MULTI_TARGETS),
	DEFINE_ARRAY(CInfoGroup, m_iszMemberValue, FIELD_STRING, MAX_MULTI_TARGETS),
	DEFINE_FIELD(CInfoGroup, m_iszDefaultMember, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CInfoGroup, CBaseEntity);

void CInfoGroup::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "defaultmember"))
	{
		m_iszDefaultMember = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
		// this assumes that additional fields are targetnames and their values are delay values.
	else if (m_cMembers < MAX_MULTI_TARGETS)
	{
		char tmp[128];
		UTIL_StripToken(pkvd->szKeyName, tmp);
		m_iszMemberName[m_cMembers] = ALLOC_STRING(tmp);
		m_iszMemberValue[m_cMembers] = ALLOC_STRING(pkvd->szValue);
		m_cMembers++;
		pkvd->fHandled = TRUE;
	}
	else
	{
		ALERT(at_error, "Too many members for info_group %s (limit is %d)\n", STRING(pev->targetname),
		      MAX_MULTI_TARGETS);
	}
}

void CInfoGroup::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));

	if (pTarget && pTarget->IsMutableAlias())
	{
		if (pev->spawnflags & SF_GROUP_DEBUG)
			ALERT(at_debug, "DEBUG: info_group %s changes the contents of %s \"%s\"\n", STRING(pev->targetname),
			      STRING(pTarget->pev->classname), STRING(pTarget->pev->targetname));
		static_cast<CBaseMutableAlias*>(pTarget)->ChangeValue(this);
	}
	else if (pev->target)
	{
		ALERT(at_debug, "info_group \"%s\": alias \"%s\" was not found or not an alias!", STRING(pev->targetname),
		      STRING(pev->target));
	}
}

int CInfoGroup::GetMember(const char* szMemberName)
{
	if (!szMemberName)
	{
		ALERT(at_debug, "info_group: GetMember called with null szMemberName!?\n");
		return NULL;
	}
	for (int i = 0; i < m_cMembers; i++)
	{
		if (FStrEq(szMemberName, STRING(m_iszMemberName[i])))
		{
			//			ALERT(at_console,"getMember: found member\n");
			return m_iszMemberValue[i];
		}
	}

	if (m_iszDefaultMember)
	{
		static char szBuffer[128];
		strcpy(szBuffer, STRING(m_iszDefaultMember));
		strcat_s(szBuffer, szMemberName);
		return MAKE_STRING(szBuffer);
		// this is a messy way to do it... but currently, only one
		// GetMember gets performed at a time, so it works.
	}

	ALERT(at_debug, "info_group \"%s\" has no member called \"%s\".\n", STRING(pev->targetname), szMemberName);
	//	ALERT(at_console,"getMember: fail\n");
	return NULL;
}

/*********************
* Worldcraft entity: multi_alias
*
* targetname- name
* other values are handled in a multi_manager-like way.
**********************/
// definition in cbase.h

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

/*********************
* Worldcraft entity: trigger_changealias
*
* target-     alias entity to affect
* netname-    value to change the alias to
**********************/

#define SF_CHANGEALIAS_RESOLVE 1
#define SF_CHANGEALIAS_DEBUG 2

class CTriggerChangeAlias : public CBaseEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS(trigger_changealias, CTriggerChangeAlias);

void CTriggerChangeAlias::Spawn()
{
}

void CTriggerChangeAlias::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target), pActivator);

	if (pTarget && pTarget->IsMutableAlias())
	{
		CBaseEntity* pValue;

		if (FStrEq(STRING(pev->netname), "*locus"))
		{
			pValue = pActivator;
		}
		else if (pev->spawnflags & SF_CHANGEALIAS_RESOLVE)
		{
			pValue = UTIL_FollowReference(nullptr, STRING(pev->netname));
		}

		if (pValue)
			static_cast<CBaseMutableAlias*>(pTarget)->ChangeValue(pValue);
		else
			static_cast<CBaseMutableAlias*>(pTarget)->ChangeValue(pev->netname);
	}
	else
	{
		ALERT(at_error, "trigger_changealias %s: alias \"%s\" was not found or not an alias!", STRING(pev->targetname),
		      STRING(pev->target));
	}
}
