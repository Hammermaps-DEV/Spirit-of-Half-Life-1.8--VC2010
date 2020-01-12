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
#include "weapons.h"
#include "player.h"
#include "gamerules.h"

enum python_e
{
	PYTHON_IDLE1,
	PYTHON_FIDGET,
	PYTHON_FIRE1,
	PYTHON_RELOAD,
	PYTHON_HOLSTER,
	PYTHON_DRAW,
	PYTHON_IDLE2,
	PYTHON_IDLE3
};

LINK_ENTITY_TO_CLASS(weapon_python, CPython);
LINK_ENTITY_TO_CLASS(weapon_357, CPython);

int CPython::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "357";
	p->iMaxAmmo1 = MAX_CARRY_357;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = MAX_AMMO_NOCLIP;
	p->iMaxClip = MAX_CLIP_PYTHON;
	p->iFlags = 0;
	p->iSlot = SLOT_PYTHON;
	p->iPosition = POSITION_PYTHON;
	p->iId = m_iId = WEAPON_PYTHON;
	p->iWeight = WEIGHT_PYTHON;

	return 1;
}

int CPython::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, nullptr, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

void CPython::Spawn()
{
	pev->classname = MAKE_STRING("weapon_357"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_PYTHON;
	SET_MODEL(ENT(pev), "models/w_357.mdl");

	m_iDefaultAmmo = DEFAULT_GIVE_PYTHON;

	FallInit(); // get ready to fall down.
}

void CPython::Precache()
{
	PRECACHE_MODEL("models/v_357.mdl");
	PRECACHE_MODEL("models/w_357.mdl");
	PRECACHE_MODEL("models/p_357.mdl");

	PRECACHE_MODEL("models/w_357ammobox.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/357_reload1.wav");
	PRECACHE_SOUND("weapons/357_cock1.wav");
	PRECACHE_SOUND("weapons/357_shot1.wav");
	PRECACHE_SOUND("weapons/357_shot2.wav");

	m_usFirePython = PRECACHE_EVENT(1, "events/python.sc");
}

BOOL CPython::Deploy()
{
#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		// enable laser sight geometry.
		pev->body = 1;
	}
	else
	{
		pev->body = 0;
	}

	return DefaultDeploy("models/v_357.mdl", "models/p_357.mdl", PYTHON_DRAW, "python", UseDecrement(), pev->body);
}

void CPython::Holster(int skiplocal /* = 0 */)
{
	m_fInReload = FALSE; // cancel any reload in progress.

	if (m_fInZoom)
	{
		SecondaryAttack();
	}

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + (16.0 / 30);
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	SendWeaponAnim(PYTHON_HOLSTER);
}

void CPython::SecondaryAttack()
{
#ifdef CLIENT_DLL
	if (!bIsMultiplayer())
#else
	if (!g_pGameRules->IsMultiplayer())
#endif
	{
		return;
	}

	if (m_pPlayer->pev->fov != 0)
	{
		m_fInZoom = FALSE;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
	}
	else if (m_pPlayer->pev->fov != 40)
	{
		m_fInZoom = TRUE;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 40;
	}

	m_flNextSecondaryAttack = 0.5;
}

void CPython::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		if (!m_fFireOnEmpty)
			Reload();
		else
		{
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = static_cast<int>(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(pev->origin.x); // origin
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_BYTE(16); // radius
	WRITE_BYTE(255); // R
	WRITE_BYTE(255); // G
	WRITE_BYTE(180); // B
	WRITE_BYTE(0); // life * 10
	WRITE_BYTE(0); // decay
	MESSAGE_END();
#endif

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);


	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_357, 0, 0,
	                                      m_pPlayer->pev, m_pPlayer->random_seed);

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFirePython, 0.0, (float*)&g_vecZero, (float*)&g_vecZero,
	                    vecDir.x, vecDir.y, 0, 0, 0, 0);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = 0.75;
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}

void CPython::Reload()
{
	if (m_pPlayer->ammo_357 <= 0)
		return;

	if (m_pPlayer->pev->fov != 0)
	{
		m_fInZoom = FALSE;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
	}

	int bUseScope = FALSE;
#ifdef CLIENT_DLL
	bUseScope = bIsMultiplayer();
#else
	bUseScope = g_pGameRules->IsMultiplayer();
#endif

	if (DefaultReload(6, PYTHON_RELOAD, (111.0 / 36), bUseScope))
	{
		m_flSoundDelay = 1.5;
	}
}

void CPython::WeaponIdle()
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	// ALERT( at_console, "%.2f\n", gpGlobals->time - m_flSoundDelay );
	if (m_flSoundDelay != 0 && m_flSoundDelay <= UTIL_WeaponTimeBase())
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_reload1.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);
		m_flSoundDelay = 0;
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	if (flRand <= 0.5)
	{
		iAnim = PYTHON_IDLE1;
		m_flTimeWeaponIdle = (71.0 / 30.0);
	}
	else if (flRand <= 0.7)
	{
		iAnim = PYTHON_IDLE2;
		m_flTimeWeaponIdle = (61.0 / 30.0);
	}
	else if (flRand <= 0.9)
	{
		iAnim = PYTHON_IDLE3;
		m_flTimeWeaponIdle = (89.0 / 30.0);
	}
	else
	{
		iAnim = PYTHON_FIDGET;
		m_flTimeWeaponIdle = (171.0 / 30.0);
	}

	int bUseScope = FALSE;
#ifdef CLIENT_DLL
	bUseScope = bIsMultiplayer();
#else
	bUseScope = g_pGameRules->IsMultiplayer();
#endif

	SendWeaponAnim(iAnim, UseDecrement() ? 1 : 0, bUseScope);
}

class CPythonAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_357ammobox.mdl");
		CBasePlayerAmmo::Spawn();
	}

	void Precache() override
	{
		PRECACHE_MODEL("models/w_357ammobox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_357BOX_GIVE, "357", MAX_CARRY_357) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(ammo_357, CPythonAmmo);
