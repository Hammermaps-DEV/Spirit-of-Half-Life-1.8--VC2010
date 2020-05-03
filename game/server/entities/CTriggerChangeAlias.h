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

#ifndef GAME_SERVER_ENTITIES_CTRIGGERCHANGEALIAS_H
#define GAME_SERVER_ENTITIES_CTRIGGERCHANGEALIAS_H

#define SF_CHANGEALIAS_RESOLVE 1
#define SF_CHANGEALIAS_DEBUG 2

class CTriggerChangeAlias : public CBaseEntity
{
public:
	void Spawn() override { };
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

#endif //GAME_SERVER_ENTITIES_CTRIGGERCHANGEALIAS_H