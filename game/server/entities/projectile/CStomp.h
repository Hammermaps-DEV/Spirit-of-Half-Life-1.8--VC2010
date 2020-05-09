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

#ifndef GAME_SERVER_ENTITIES_PROJECTILE_CSTOMP_H
#define GAME_SERVER_ENTITIES_PROJECTILE_CSTOMP_H

#define	STOMP_INTERVAL		0.025

class CStomp : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void Think() override;
	static CStomp* StompCreate(const Vector& origin, const Vector& end, float speed);

	int gStompSprite;
};


#endif //GAME_SERVER_ENTITIES_PROJECTILE_CSTOMP_H