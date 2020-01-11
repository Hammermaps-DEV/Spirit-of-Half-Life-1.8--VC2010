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

// special deathmatch shotgun spreads
#define VECTOR_CONE_DM_SHOTGUN	Vector( 0.08716, 0.04362, 0.00  )// 10 degrees by 5 degrees
#define VECTOR_CONE_DM_DOUBLESHOTGUN Vector( 0.17365, 0.04362, 0.00 ) // 20 degrees by 5 degrees

enum shotgun_e
{
	SHOTGUN_IDLE,
	SHOTGUN_FIRE,
	SHOTGUN_FIRE2,
	SHOTGUN_RELOAD,
	SHOTGUN_PUMP,
	SHOTGUN_START_RELOAD,
	SHOTGUN_DRAW,
	SHOTGUN_HOLSTER,
	SHOTGUN_IDLE4,
	SHOTGUN_IDLE_DEEP
};

LINK_ENTITY_TO_CLASS(weapon_shotgun, CShotgun);

void CShotgun::Spawn()
{
	Precache();
	m_iId = WEAPON_SHOTGUN;
	SET_MODEL(ENT(pev), "models/w_shotgun.mdl");

	m_iDefaultAmmo = SHOTGUN_DEFAULT_GIVE;

	FallInit(); // get ready to fall
}

void CShotgun::Precache()
{
	PRECACHE_MODEL("models/v_shotgun.mdl");
	PRECACHE_MODEL("models/w_shotgun.mdl");
	PRECACHE_MODEL("models/p_shotgun.mdl");

	m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl"); // shotgun shell

	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/dbarrel1.wav"); //shotgun
	PRECACHE_SOUND("weapons/sbarrel1.wav"); //shotgun

	PRECACHE_SOUND("weapons/reload1.wav"); // shotgun reload
	PRECACHE_SOUND("weapons/reload3.wav"); // shotgun reload

	PRECACHE_SOUND("weapons/357_cock1.wav"); // gun empty sound
	PRECACHE_SOUND("weapons/scock1.wav"); // cock gun

	m_usSingleFire = PRECACHE_EVENT(1, "events/shotgun1.sc");
	m_usDoubleFire = PRECACHE_EVENT(1, "events/shotgun2.sc");
}

int CShotgun::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

int CShotgun::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = MAX_CARRY_BUCKSHOT;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = MAX_AMMO_NOCLIP;
	p->iMaxClip = MAX_CLIP_SHOTGUN;
	p->iSlot = SLOT_SHOTGUN;
	p->iPosition = POSITION_SHOTGUN;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SHOTGUN;
	p->iWeight = WEIGHT_SHOTGUN;
	return 1;
}

BOOL CShotgun::Deploy()
{
	return DefaultDeploy("models/v_shotgun.mdl", "models/p_shotgun.mdl", SHOTGUN_DRAW, "shotgun");
}

void CShotgun::Holster(int skiplocal)
{
	m_fInReload = FALSE; // cancel any reload in progress.
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(SHOTGUN_HOLSTER);
}

void CShotgun::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		Reload();
		if (m_iClip == 0)
			PlayEmptySound();
		return;
	}

	if (m_fInSpecialReload >= 1)
	{
		Charge(true);
	}
	else
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

		m_iClip--;

		int flags;
#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(pev->origin.x); // origin
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_BYTE(16); // radius
		WRITE_BYTE(255); // R
		WRITE_BYTE(255); // G
		WRITE_BYTE(160); // B
		WRITE_BYTE(0); // life * 10
		WRITE_BYTE(0); // decay
		MESSAGE_END();
#endif

		Vector vecDir;

#ifdef CLIENT_DLL
		if (bIsMultiplayer())
#else
		if (g_pGameRules->IsMultiplayer())
#endif
		{
			vecDir = m_pPlayer->FireBulletsPlayer(4, vecSrc, vecAiming, VECTOR_CONE_DM_SHOTGUN, 2048,
			                                      BULLET_PLAYER_BUCKSHOT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);
		}
		else
		{
			// regular old, untouched spread. 
			vecDir = m_pPlayer->FireBulletsPlayer(6, vecSrc, vecAiming, VECTOR_CONE_10DEGREES, 2048,
			                                      BULLET_PLAYER_BUCKSHOT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);
		}

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usSingleFire, 0.0, (float*)&g_vecZero, (float*)&g_vecZero,
		                    vecDir.x, vecDir.y, 0, 0, 0, 0);

		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			// HEV suit - indicate out of ammo condition
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

		if (m_iClip != 0)
			m_flPumpTime = gpGlobals->time + 0.5;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.95;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.95;

		if (m_iClip != 0)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		else
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.95;

		m_fInSpecialReload = 0;
	}
}

void CShotgun::SecondaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	if (m_iClip == 1)
	{
		PrimaryAttack();
		return;
	}

	if (m_iClip <= 1)
	{
		Reload();
		PlayEmptySound();
		return;
	}

	if (m_fInSpecialReload >= 1)
	{
		Charge(true);
	}
	else
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

		m_iClip -= 2;

		int flags;
#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(pev->origin.x); // origin
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_BYTE(16); // radius
		WRITE_BYTE(255); // R
		WRITE_BYTE(255); // G
		WRITE_BYTE(160); // B
		WRITE_BYTE(0); // life * 10
		WRITE_BYTE(0); // decay
		MESSAGE_END();
#endif

		Vector vecDir;

#ifdef CLIENT_DLL
		if (bIsMultiplayer())
#else
		if (g_pGameRules->IsMultiplayer())
#endif
		{
			// tuned for deathmatch
			vecDir = m_pPlayer->FireBulletsPlayer(8, vecSrc, vecAiming, VECTOR_CONE_DM_DOUBLESHOTGUN, 2048,
			                                      BULLET_PLAYER_BUCKSHOT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);
		}
		else
		{
			// untouched default single player
			vecDir = m_pPlayer->FireBulletsPlayer(12, vecSrc, vecAiming, VECTOR_CONE_10DEGREES, 2048,
			                                      BULLET_PLAYER_BUCKSHOT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);
		}

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usDoubleFire, 0.0, (float*)&g_vecZero, (float*)&g_vecZero,
		                    vecDir.x, vecDir.y, 0, 0, 0, 0);

		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			// HEV suit - indicate out of ammo condition
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

		if (m_iClip != 0)
			m_flPumpTime = gpGlobals->time + 0.95;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 21.0 / 13;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 21.0 / 13;

		if (m_iClip != 0)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.0;
		else
			m_flTimeWeaponIdle = 21.0 / 13;

		m_fInSpecialReload = 0;
		m_pPlayer->pev->punchangle.x -= 5;
	}
}

//=========================================================
// Reload Charge
//=========================================================
void CShotgun::Charge(bool m_BeginAttack)
{
	if ((m_iClip != MAX_CLIP_SHOTGUN && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]) && !m_BeginAttack)
	{
		if (m_fInSpecialReload == 0)
		{
			SendWeaponAnim(SHOTGUN_START_RELOAD);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9.0 / 12;
			m_pPlayer->m_flNextAttack = m_flTimeWeaponIdle;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + m_flTimeWeaponIdle;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + m_flTimeWeaponIdle;
			m_fInSpecialReload = 1;
		}
		else if (m_fInSpecialReload == 1)
		{
			if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
			{
				m_fInSpecialReload = 2;

				if (RANDOM_LONG(0, 1))
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload1.wav", VOL_NORM, ATTN_NORM, 0,
					               85 + RANDOM_LONG(0, 0x1f));
				}
				else
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload3.wav", VOL_NORM, ATTN_NORM, 0,
					               85 + RANDOM_LONG(0, 0x1f));
				}

				SendWeaponAnim(SHOTGUN_RELOAD);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 11.0 / 16;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle;
				m_pPlayer->m_flNextAttack = m_flTimeWeaponIdle;
			}
		}
		else
		{
			if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
			{
				m_iClip += 1;
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;
				m_fInSpecialReload = 1;
				if (m_iClip == MAX_CLIP_SHOTGUN)
				{
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
				}
			}
		}
	}
	else
	{
		// reload debounce has timed out
		SendWeaponAnim(SHOTGUN_PUMP);
		m_fInSpecialReload = 0;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 11.0 / 13;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle;
		m_pPlayer->m_flNextAttack = m_flNextSecondaryAttack;

		// play pumping sound
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/scock1.wav", 1, ATTN_NORM, 0,
		               95 + RANDOM_LONG(0, 0x1f));

		ResetEmptySound();
	}
}

void CShotgun::Reload()
{
	if ((m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == MAX_CLIP_SHOTGUN) ||
		m_flNextPrimaryAttack > UTIL_WeaponTimeBase() || m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
	{
		return;
	}

	Charge(false);
}

void CShotgun::WeaponIdle()
{
	if (m_flPumpTime && m_flPumpTime < UTIL_WeaponTimeBase())
	{
		// play pumping sound
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/scock1.wav", 1, ATTN_NORM, 0,
		               95 + RANDOM_LONG(0, 0x1f));
		m_flPumpTime = 0;
	}

	if ((m_flTimeWeaponIdle > UTIL_WeaponTimeBase() ||
		m_flTimeWeaponIdleLock > UTIL_WeaponTimeBase()) && !m_fInSpecialReload)
	{
		return;
	}

	if (m_fInSpecialReload != 0)
	{
		Charge(false);
	}
	else
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
		if (flRand <= 0.5)
		{
			iAnim = SHOTGUN_IDLE_DEEP;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (61.0 / 12.0); // * RANDOM_LONG(2, 5);
			m_flTimeWeaponIdleLock = m_flTimeWeaponIdle + RANDOM_FLOAT(2, 10);
		}
		else if (flRand <= 0.7)
		{
			iAnim = SHOTGUN_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (21.0 / 9.0);
			m_flTimeWeaponIdleLock = m_flTimeWeaponIdle + RANDOM_FLOAT(2, 10);
		}
		else if (flRand <= 0.9)
		{
			iAnim = SHOTGUN_IDLE4;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (21.0 / 9.0);
			m_flTimeWeaponIdleLock = UTIL_WeaponTimeBase();
		}
		else
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT(10, 15);
			m_flTimeWeaponIdleLock = UTIL_WeaponTimeBase();
		}

		SendWeaponAnim(iAnim);
	}
}

class CShotgunAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_shotbox.mdl");
		CBasePlayerAmmo::Spawn();
	}

	void Precache(void)
	{
		PRECACHE_MODEL("models/w_shotbox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther)
	{
		if (pOther->GiveAmmo(AMMO_BUCKSHOTBOX_GIVE, "buckshot", MAX_CARRY_BUCKSHOT) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(ammo_buckshot, CShotgunAmmo);
