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
#ifndef ITEMS_H
#define ITEMS_H

// AJH Inventory items, some of which can be manually used, others automatically
// and are here just so the player knows they have it/them.
#define ITEM_HEALTHKIT		1
#define ITEM_ANTIDOTE		2
#define ITEM_ANTIRAD		3
#define ITEM_SECURITY		4
#define ITEM_BATTERY 		5
#define ITEM_LONGJUMP		6
#define ITEM_FLARE			7
#define ITEM_CAMERA			8

/*
#define ITEM_SLOT1		9
#define ITEM_SLOT1		10
#define ITEM_SLOT1		11
#define ITEM_SLOT1		12
#define ITEM_SLOT1		13
#define ITEM_SLOT1		14
*/
#define ITEM_SLOT1		15
#define ITEM_SLOT8		16
#define ITEM_SLOT9		17
#define ITEM_SLOT10		18

class CItem : public CBaseEntity
{
public:
	void Spawn() override;
	CBaseEntity* Respawn() override;
	void EXPORT ItemTouch(CBaseEntity* pOther);
	void EXPORT Materialize();
	virtual BOOL MyTouch(CBasePlayer* pPlayer) { return FALSE; };
};

class CItemMedicalKit : public CItem //AJH new inventory based manual use medkit
{
public:
	void Spawn() override;
	void Precache() override;
	int MyTouch(CBasePlayer* pPlayer) override;
	void EXPORT ItemTouch(CBaseEntity* pOther);
	void CItemMedicalKit::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

class CItemAntiRad : public CItem //AJH new anti radiation syringe
{
public:
	void Spawn() override;
	void Precache() override;
	BOOL MyTouch(CBasePlayer* pPlayer) override;
	void CItemAntiRad::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

class CItemAntidote : public CItem //AJH new anti radiation syringe
{
public:
	void Spawn() override;
	void Precache() override;
	BOOL MyTouch(CBasePlayer* pPlayer) override;
	void CItemAntidote::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

class CItemFlare : public CItem //AJH new anti radiation syringe
{
public:
	void Spawn() override;
	void Precache() override;
	BOOL MyTouch(CBasePlayer* pPlayer) override;
	void CItemFlare::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

class CItemCamera : public CItem //AJH new inventory camera (can be placed anywhere in a level by the player)
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	//void Think( void );
	void Precache() override;
	void EXPORT ItemTouch(CBaseEntity* pOther);
	int MyTouch(CBasePlayer* pPlayer) override;
	//CBaseEntity* Respawn();
	//void Materialize();
	void StripFromPlayer();
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;


	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	int ObjectCaps() override { return m_iobjectcaps; }
	static TYPEDESCRIPTION m_SaveData[];

	//EHANDLE m_hPlayer;
	int m_state;
	int m_iobjectcaps;
	CItemCamera* m_pNextCamera;
	CItemCamera* m_pLastCamera;
};
#endif // ITEMS_H
