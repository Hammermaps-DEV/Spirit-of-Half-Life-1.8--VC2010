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

#ifndef GAME_SERVER_ENTITIES_PROJECTILE_CBMORTAR_H
#define GAME_SERVER_ENTITIES_PROJECTILE_CBMORTAR_H

//=========================================================
// Mortar shot entity
//=========================================================
class CBMortar : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;

	static CBMortar* Shoot(edict_t* pOwner, Vector vecStart, Vector vecVelocity);
	void Touch(CBaseEntity* pOther) override;
	void EXPORT Animate();

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void MortarSpray(const Vector& position, const Vector& direction, int count) const;

	int m_maxFrame;
	int gSpitSprite;
};

#endif //GAME_SERVER_ENTITIES_PROJECTILE_CBMORTAR_H