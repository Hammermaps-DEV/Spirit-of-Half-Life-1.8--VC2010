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
#include "CTriggerChangeAlias.h"

/*********************
* Worldcraft entity: trigger_changealias
*
* target-     alias entity to affect
* netname-    value to change the alias to
**********************/

LINK_ENTITY_TO_CLASS(trigger_changealias, CTriggerChangeAlias);

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