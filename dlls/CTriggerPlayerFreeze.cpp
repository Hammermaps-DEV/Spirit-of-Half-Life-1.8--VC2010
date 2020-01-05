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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "gamerules.h"
#include "player.h"

class CTriggerPlayerFreeze : public CBaseDelay
{
public:
	void Spawn() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

private:
	bool m_bUnFrozen;
};

LINK_ENTITY_TO_CLASS(trigger_playerfreeze, CTriggerPlayerFreeze);

void CTriggerPlayerFreeze::Spawn()
{
	if (g_pGameRules->IsDeathmatch())
		REMOVE_ENTITY(edict());
	else
		m_bUnFrozen = true;
}

void CTriggerPlayerFreeze::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_bUnFrozen = !m_bUnFrozen;

	CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)&g_engfuncs.pfnPEntityOfEntIndex(1)->v);

	pPlayer->EnableControl(m_bUnFrozen);
}