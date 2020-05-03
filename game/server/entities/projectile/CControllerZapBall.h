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

#ifndef GAME_SERVER_ENTITIES_PROJECTILE_CCONTROLLERZAPBALL_H
#define GAME_SERVER_ENTITIES_PROJECTILE_CCONTROLLERZAPBALL_H
#include "basemonster.h"

class CControllerZapBall : public CBaseMonster
{
	void Spawn() override;
	void Precache() override;
	void EXPORT AnimateThink();
	void EXPORT ExplodeTouch(CBaseEntity* pOther);

	EHANDLE m_hOwner;
};

#endif //GAME_SERVER_ENTITIES_PROJECTILE_CCONTROLLERZAPBALL_H