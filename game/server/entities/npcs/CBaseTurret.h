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

#ifndef GAME_SERVER_ENTITIES_NPCS_CAGRUNT_H
#define GAME_SERVER_ENTITIES_NPCS_CAGRUNT_H

#define TURRET_RANGE	(100 * 12)
#define TURRET_TURNRATE	30		//angles per 0.1 second
#define TURRET_MAXWAIT	15		// seconds turret will stay active w/o a target
#define TURRET_MACHINE_VOLUME	0.5

typedef enum
{
	TURRET_ANIM_NONE = 0,
	TURRET_ANIM_FIRE,
	TURRET_ANIM_SPIN,
	TURRET_ANIM_DEPLOY,
	TURRET_ANIM_RETIRE,
	TURRET_ANIM_DIE,
} TURRET_ANIM;

class CBaseTurret : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;
	void EXPORT TurretUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr,
		int bitsDamageType) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	int Classify() override;

	int BloodColor() override { return DONT_BLEED; }

	void GibMonster() override
	{
	} // UNDONE: Throw turret gibs?

	// Think functions
	void EXPORT ActiveThink();
	void EXPORT SearchThink();
	void EXPORT AutoSearchThink();
	void EXPORT TurretDeath();

	virtual void EXPORT SpinDownCall() { m_iSpin = 0; }
	virtual void EXPORT SpinUpCall() { m_iSpin = 1; }

	// void SpinDown();
	// float EXPORT SpinDownCall( void ) { return SpinDown(); }

	// virtual float SpinDown() { return 0;}
	// virtual float Retire() { return 0;}

	void EXPORT Deploy();
	void EXPORT Retire();

	void EXPORT Initialize();

	virtual void Ping();
	virtual void EyeOn();
	virtual void EyeOff();

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	// other functions
	void SetTurretAnim(TURRET_ANIM anim);
	int MoveTurret();

	virtual void Shoot(Vector& vecSrc, Vector& vecDirToEnemy)
	{
	};

	float m_flMaxSpin; // Max time to spin the barrel w/o a target
	int m_iSpin;

	CSprite* m_pEyeGlow;
	int m_eyeBrightness;

	int m_iDeployHeight;
	int m_iRetractHeight;
	int m_iMinPitch;

	int m_iBaseTurnRate; // angles per second
	float m_fTurnRate; // actual turn rate
	int m_iOrientation; // 0 = floor, 1 = Ceiling
	int m_iOn;

	virtual STATE getState()
	{
		if (m_iOn) { return STATE_ON; }
		return STATE_OFF;
	}

	int m_fBeserk; // Sometimes this bitch will just freak out
	int m_iAutoStart; // true if the turret auto deploys when a target
	// enters its range

	Vector m_vecLastSight;
	float m_flLastSight; // Last time we saw a target
	float m_flMaxWait; // Max time to seach w/o a target
	int m_iSearchSpeed; // Not Used!

	// movement
	float m_flStartYaw;
	Vector m_vecCurAngles;
	Vector m_vecGoalAngles;


	float m_flPingTime; // Time until the next ping, used when searching
	float m_flSpinUpTime; // Amount of time until the barrel should spin down when searching
};

#endif //GAME_SERVER_ENTITIES_NPCS_CAGRUNT_H