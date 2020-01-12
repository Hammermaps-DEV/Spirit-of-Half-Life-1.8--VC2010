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
#ifndef EFFECTS_H
#define EFFECTS_H

#define SF_BEAM_STARTON			0x0001
#define SF_BEAM_TOGGLE			0x0002
#define SF_BEAM_RANDOM			0x0004
#define SF_BEAM_RING			0x0008
#define SF_BEAM_SPARKSTART		0x0010
#define SF_BEAM_SPARKEND		0x0020
#define SF_BEAM_DECALS			0x0040
#define SF_BEAM_SHADEIN			0x0080
#define SF_BEAM_SHADEOUT		0x0100
#define SF_BEAM_SOLID			0x0200
#define SF_BEAM_TEMPORARY		0x8000
//LRC - tripbeams
#define SF_BEAM_TRIPPED			0x80000
//LRC - smoother lasers
#define SF_LASER_INTERPOLATE	0x0400

#define SF_SPRITE_STARTON		0x0001
#define SF_SPRITE_ONCE			0x0002
#define SF_SPRITE_TEMPORARY		0x8000

class CSprite : public CPointEntity
{
public:
	void Spawn(void) override;
	void Precache(void) override;

	int ObjectCaps(void) override
	{
		int flags = 0;
		if (pev->spawnflags & SF_SPRITE_TEMPORARY)
			flags = FCAP_DONT_SAVE;
		return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | flags;
	}

	void EXPORT AnimateThink(void);
	void EXPORT ExpandThink(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Animate(float frames);
	void Expand(float scaleSpeed, float fadeSpeed);
	void SpriteInit(const char* pSpriteName, const Vector& origin);

	STATE GetState(void) override { return (pev->effects & EF_NODRAW) ? STATE_OFF : STATE_ON; };

	void SetAttachment(edict_t* pEntity, int attachment)
	{
		if (pEntity)
		{
			pev->skin = ENTINDEX(pEntity);
			pev->body = attachment;
			pev->aiment = pEntity;
			pev->movetype = MOVETYPE_FOLLOW;
		}
	}

	void TurnOff(void);
	void TurnOn(void);
	float Frames(void) { return m_maxFrame; }

	void SetTransparency(int rendermode, int r, int g, int b, int a, int fx)
	{
		pev->rendermode = rendermode;
		pev->rendercolor.x = r;
		pev->rendercolor.y = g;
		pev->rendercolor.z = b;
		pev->renderamt = a;
		pev->renderfx = fx;
	}

	void SetTexture(int spriteIndex) { pev->modelindex = spriteIndex; }
	void SetScale(float scale) { pev->scale = scale; }

	void SetColor(int r, int g, int b)
	{
		pev->rendercolor.x = r;
		pev->rendercolor.y = g;
		pev->rendercolor.z = b;
	}

	void SetBrightness(int brightness) { pev->renderamt = brightness; }

	void AnimateAndDie(float framerate)
	{
		SetThink(&CSprite ::AnimateUntilDead);
		pev->framerate = framerate;
		pev->dmgtime = gpGlobals->time + (m_maxFrame / framerate);
		SetNextThink(0);
	}

	void EXPORT AnimateUntilDead(void);

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
	static CSprite* SpriteCreate(const char* pSpriteName, const Vector& origin, BOOL animate);

	//private:

	float m_lastTime;
	float m_maxFrame;
};


class CBeam : public CBaseEntity
{
public:
	void Spawn(void) override;
	void Precache(void) override;

	int ObjectCaps(void) override
	{
		int flags = 0;
		if (pev->spawnflags & SF_BEAM_TEMPORARY)
			flags = FCAP_DONT_SAVE;
		return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | flags;
	}

	void EXPORT TriggerTouch(CBaseEntity* pOther);

	// These functions are here to show the way beams are encoded as entities.
	// Encoding beams as entities simplifies their management in the client/server architecture
	void SetType(int type) { pev->rendermode = (pev->rendermode & 0xF0) | (type & 0x0F); }
	void SetFlags(int flags) { pev->rendermode = (pev->rendermode & 0x0F) | (flags & 0xF0); }
	void SetStartPos(const Vector& pos) { pev->origin = pos; }
	void SetEndPos(const Vector& pos) { pev->angles = pos; }
	void SetStartEntity(int entityIndex);
	void SetEndEntity(int entityIndex);

	void SetStartAttachment(int attachment)
	{
		pev->sequence = (pev->sequence & 0x0FFF) | ((attachment & 0xF) << 12);
	}

	void SetEndAttachment(int attachment) { pev->skin = (pev->skin & 0x0FFF) | ((attachment & 0xF) << 12); }

	void SetTexture(int spriteIndex) { pev->modelindex = spriteIndex; }
	void SetWidth(int width) { pev->scale = width; }
	void SetNoise(int amplitude) { pev->body = amplitude; }

	void SetColor(int r, int g, int b)
	{
		pev->rendercolor.x = r;
		pev->rendercolor.y = g;
		pev->rendercolor.z = b;
	}

	void SetBrightness(int brightness) { pev->renderamt = brightness; }
	void SetFrame(float frame) { pev->frame = frame; }
	void SetScrollRate(int speed) { pev->animtime = speed; }

	int GetType(void) { return pev->rendermode & 0x0F; }
	int GetFlags(void) { return pev->rendermode & 0xF0; }
	int GetStartEntity(void) { return pev->sequence & 0xFFF; }
	int GetEndEntity(void) { return pev->skin & 0xFFF; }

	const Vector& GetStartPos(void);
	const Vector& GetEndPos(void);

	Vector Center(void) override { return (GetStartPos() + GetEndPos()) * 0.5; }; // center point of beam

	int GetTexture(void) { return pev->modelindex; }
	int GetWidth(void) { return pev->scale; }
	int GetNoise(void) { return pev->body; }
	// inline void GetColor( int r, int g, int b ) { pev->rendercolor.x = r; pev->rendercolor.y = g; pev->rendercolor.z = b; }
	int GetBrightness(void) { return pev->renderamt; }
	int GetFrame(void) { return pev->frame; }
	int GetScrollRate(void) { return pev->animtime; }

	CBaseEntity* GetTripEntity(TraceResult* ptr); //LRC

	// Call after you change start/end positions
	void RelinkBeam(void);
	//	void		SetObjectCollisionBox( void );

	void DoSparks(const Vector& start, const Vector& end);
	CBaseEntity* RandomTargetname(const char* szName);
	void BeamDamage(TraceResult* ptr);
	// Init after BeamCreate()
	void BeamInit(const char* pSpriteName, int width);
	void PointsInit(const Vector& start, const Vector& end);
	void PointEntInit(const Vector& start, int endIndex);
	void EntsInit(int startIndex, int endIndex);
	void HoseInit(const Vector& start, const Vector& direction);

	static CBeam* BeamCreate(const char* pSpriteName, int width);

	void LiveForTime(float time)
	{
		SetThink(&CBeam::SUB_Remove);
		SetNextThink(time);
	}

	void BeamDamageInstant(TraceResult* ptr, float damage)
	{
		pev->dmg = damage;
		pev->dmgtime = gpGlobals->time - 1;
		BeamDamage(ptr);
	}
};


#define SF_MESSAGE_ONCE			0x0001		// Fade in, not out
#define SF_MESSAGE_ALL			0x0002		// Send to all clients


class CLaser : public CBeam
{
public:
	void Spawn(void) override;
	void PostSpawn(void) override;
	void Precache(void) override;
	void KeyValue(KeyValueData* pkvd) override;

	void TurnOn(void);
	void TurnOff(void);
	STATE GetState(void) override { return (pev->effects & EF_NODRAW) ? STATE_OFF : STATE_ON; };

	void FireAtPoint(Vector startpos, TraceResult& point);

	void EXPORT StrikeThink(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hActivator; //AJH allow *locus start/end positions

	CSprite* m_pStartSprite;
	CSprite* m_pEndSprite;
	int m_iszStartSpriteName;
	int m_iszEndSpriteName;
	Vector m_firePosition;
	int m_iProjection;
	int m_iStoppedBy;
	int m_iszStartPosition;
	int m_iTowardsMode;
};

class CRainSettings : public CBaseEntity
{
public:
	void Spawn(void) override;
	void KeyValue(KeyValueData* pkvd) override;

	int ObjectCaps(void) override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	float Rain_Distance;
	int Rain_Mode;
};

class CRainModify : public CBaseEntity
{
public:
	void Spawn(void) override;
	void KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int ObjectCaps(void) override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int Rain_Drips;
	float Rain_windX, Rain_windY;
	float Rain_randX, Rain_randY;
	float fadeTime;
};

#endif		//EFFECTS_H
