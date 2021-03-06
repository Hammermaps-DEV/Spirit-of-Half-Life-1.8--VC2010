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

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"saverestore.h"
#include	"client.h"
#include	"decals.h"
#include	"gamerules.h"
#include	"game.h"
#include	"movewith.h"
#include	"skill.h"
#include	"weapons.h"

#include	"CBaseMutableAlias.h"

void EntvarsKeyvalue(entvars_t* pev, KeyValueData* pkvd);

extern "C" void PM_Move(struct playermove_s* ppmove, int server);
extern "C" void PM_Init(struct playermove_s* ppmove);
extern "C" char PM_FindTextureType(char* name);

extern Vector VecBModelOrigin(entvars_t* pevBModel);
extern DLL_GLOBAL Vector g_vecAttackDir;
extern DLL_GLOBAL int g_iSkillLevel;

DLL_FUNCTIONS gFunctionTable =
{
	GameDLLInit, //pfnGameInit
	DispatchSpawn, //pfnSpawn
	DispatchThink, //pfnThink
	DispatchUse, //pfnUse
	DispatchTouch, //pfnTouch
	DispatchBlocked, //pfnBlocked
	DispatchKeyValue, //pfnKeyValue
	DispatchSave, //pfnSave
	DispatchRestore, //pfnRestore
	DispatchObjectCollsionBox, //pfnAbsBox

	SaveWriteFields, //pfnSaveWriteFields
	SaveReadFields, //pfnSaveReadFields

	SaveGlobalState, //pfnSaveGlobalState
	RestoreGlobalState, //pfnRestoreGlobalState
	ResetGlobalState, //pfnResetGlobalState

	ClientConnect, //pfnClientConnect
	ClientDisconnect, //pfnClientDisconnect
	ClientKill, //pfnClientKill
	ClientPutInServer, //pfnClientPutInServer
	ClientCommand, //pfnClientCommand
	ClientUserInfoChanged, //pfnClientUserInfoChanged
	ServerActivate, //pfnServerActivate
	ServerDeactivate, //pfnServerDeactivate

	PlayerPreThink, //pfnPlayerPreThink
	PlayerPostThink, //pfnPlayerPostThink

	StartFrame, //pfnStartFrame
	ParmsNewLevel, //pfnParmsNewLevel
	ParmsChangeLevel, //pfnParmsChangeLevel

	GetGameDescription, //pfnGetGameDescription    Returns string describing current .dll game.
	PlayerCustomization, //pfnPlayerCustomization   Notifies .dll of new customization for player.

	SpectatorConnect, //pfnSpectatorConnect      Called when spectator joins server
	SpectatorDisconnect, //pfnSpectatorDisconnect   Called when spectator leaves the server
	SpectatorThink, //pfnSpectatorThink        Called when spectator sends a command packet (usercmd_t)

	Sys_Error, //pfnSys_Error				Called when engine has encountered an error

	PM_Move, //pfnPM_Move
	PM_Init, //pfnPM_Init				Server version of player movement initialization
	PM_FindTextureType, //pfnPM_FindTextureType

	SetupVisibility, //pfnSetupVisibility        Set up PVS and PAS for networking for this client
	UpdateClientData, //pfnUpdateClientData       Set up data sent only to specific client
	AddToFullPack, //pfnAddToFullPack
	CreateBaseline,
	//pfnCreateBaseline			Tweak entity baseline for network encoding, allows setup of player baselines, too.
	RegisterEncoders, //pfnRegisterEncoders		Callbacks for network encoding
	GetWeaponData, //pfnGetWeaponData
	CmdStart, //pfnCmdStart
	CmdEnd, //pfnCmdEnd
	ConnectionlessPacket, //pfnConnectionlessPacket
	GetHullBounds, //pfnGetHullBounds
	CreateInstancedBaselines, //pfnCreateInstancedBaselines
	InconsistentFile, //pfnInconsistentFile
	AllowLagCompensation, //pfnAllowLagCompensation
};

static void SetObjectCollisionBox(entvars_t* pev);

#ifndef _WIN32
extern "C" {
#endif
int GetEntityAPI(DLL_FUNCTIONS* pFunctionTable, int interfaceVersion)
{
	if (!pFunctionTable || interfaceVersion != INTERFACE_VERSION)
	{
		return FALSE;
	}

	memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
	return TRUE;
}

int GetEntityAPI2(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
{
	if (!pFunctionTable || *interfaceVersion != INTERFACE_VERSION)
	{
		// Tell engine what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return FALSE;
	}

	memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
	return TRUE;
}

#ifndef _WIN32
}
#endif

int DispatchSpawn(edict_t* pent)
{
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));

	if (pEntity)
	{
		// Initialize these or entities who don't link to the world won't have anything in here
		pEntity->pev->absmin = pEntity->pev->origin - Vector(1, 1, 1);
		pEntity->pev->absmax = pEntity->pev->origin + Vector(1, 1, 1);

		pEntity->Spawn();

		// Try to get the pointer again, in case the spawn function deleted the entity.
		// UNDONE: Spawn() should really return a code to ask that the entity be deleted, but
		// that would touch too much code for me to do that right now.
		pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));

		if (pEntity)
		{
			pEntity->pev->colormap = ENTINDEX(pent);
			if (g_pGameRules && !g_pGameRules->IsAllowedToSpawn(pEntity))
				return -1; // return that this entity should be deleted
			if (pEntity->pev->flags & FL_KILLME)
				return -1;
			if (g_iSkillLevel == SKILL_EASY && pEntity->m_iLFlags & LF_NOTEASY)
				return -1; //LRC
			if (g_iSkillLevel == SKILL_MEDIUM && pEntity->m_iLFlags & LF_NOTMEDIUM)
				return -1; //LRC
			if (g_iSkillLevel == SKILL_HARD && pEntity->m_iLFlags & LF_NOTHARD)
				return -1; //LRC
		}

		// Handle global stuff here
		if (pEntity && pEntity->pev->globalname)
		{
			const globalentity_t* pGlobal = gGlobalState.EntityFromTable(pEntity->pev->globalname);
			if (pGlobal)
			{
				// Already dead? delete
				if (pGlobal->state == GLOBAL_DEAD)
					return -1;

				if (!FStrEq(STRING(gpGlobals->mapname), pGlobal->levelName))
					pEntity->MakeDormant(); // Hasn't been moved to this level yet, wait but stay alive
				// In this level & not dead, continue on as normal
			}
			else
			{
				// Spawned entities default to 'On'
				gGlobalState.EntityAdd(pEntity->pev->globalname, gpGlobals->mapname, GLOBAL_ON);
				//				ALERT( at_console, "Added global entity %s (%s)\n", STRING(pEntity->pev->classname), STRING(pEntity->pev->globalname) );
			}
		}
	}

	return 0;
}

void DispatchKeyValue(edict_t* pentKeyvalue, KeyValueData* pkvd)
{
	if (!pkvd || !pentKeyvalue)
		return;

	EntvarsKeyvalue(VARS(pentKeyvalue), pkvd);

	// If the key was an entity variable, or there's no class set yet, don't look for the object, it may
	// not exist yet.
	if (pkvd->fHandled || pkvd->szClassName == nullptr)
		return;

	// Get the actualy entity object
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pentKeyvalue));

	if (!pEntity)
		return;

	pEntity->KeyValue(pkvd);
}

// HACKHACK -- this is a hack to keep the node graph entity from "touching" things (like triggers)
// while it builds the graph
BOOL gTouchDisabled = FALSE;

void DispatchTouch(edict_t* pentTouched, edict_t* pentOther)
{
	if (gTouchDisabled)
		return;

	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pentTouched));
	CBaseEntity* pOther = static_cast<CBaseEntity*>(GET_PRIVATE(pentOther));

	if (pEntity && pOther && !((pEntity->pev->flags | pOther->pev->flags) & FL_KILLME))
		pEntity->Touch(pOther);
}

void DispatchUse(edict_t* pentUsed, edict_t* pentOther)
{
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pentUsed));
	CBaseEntity* pOther = static_cast<CBaseEntity*>(GET_PRIVATE(pentOther));

	if (pEntity && !(pEntity->pev->flags & FL_KILLME))
		pEntity->Use(pOther, pOther, USE_TOGGLE, 0);
}

void DispatchThink(edict_t* pent)
{
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));

	if (pEntity)
	{
		if (FBitSet(pEntity->pev->flags, FL_DORMANT))
			ALERT(at_error, "Dormant entity %s is thinking!!\n", STRING(pEntity->pev->classname));

		//if (pEntity->pev->classname) ALERT(at_console, "DispatchThink %s\n", STRING(pEntity->pev->targetname));
		pEntity->Think();
	}
}

void DispatchBlocked(edict_t* pentBlocked, edict_t* pentOther)
{
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pentBlocked));
	CBaseEntity* pOther = static_cast<CBaseEntity*>(GET_PRIVATE(pentOther));

	if (pEntity)
		pEntity->Blocked(pOther);
}

void DispatchSave(edict_t* pent, SAVERESTOREDATA* pSaveData)
{
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));

	if (pEntity && pSaveData)
	{
		ENTITYTABLE* pTable = &pSaveData->pTable[pSaveData->currentIndex];

		if (pTable->pent != pent)
			ALERT(at_error, "ENTITY TABLE OR INDEX IS WRONG!!!!\n");

		if (pEntity->ObjectCaps() & FCAP_DONT_SAVE)
			return;

		// These don't use ltime & nextthink as times really, but we'll fudge around it.
		if (pEntity->pev->movetype == MOVETYPE_PUSH)
		{
			//LRC - rearranged so that we can correct m_fNextThink too.
			float delta = gpGlobals->time - pEntity->pev->ltime;
			pEntity->pev->ltime += delta;
			pEntity->pev->nextthink += delta;
			pEntity->m_fPevNextThink = pEntity->pev->nextthink;
			pEntity->m_fNextThink += delta;
		}

		pTable->location = pSaveData->size; // Remember entity position for file I/O
		pTable->classname = pEntity->pev->classname; // Remember entity class for respawn

		CSave saveHelper(pSaveData);
		pEntity->Save(saveHelper);

		pTable->size = pSaveData->size - pTable->location; // Size of entity block is data size written to block
	}
}

// Find the matching global entity.  Spit out an error if the designer made entities of
// different classes with the same global name
CBaseEntity* FindGlobalEntity(string_t classname, string_t globalname)
{
	CBaseEntity* pReturn = UTIL_FindEntityByString(nullptr, "globalname", STRING(globalname));

	if (pReturn)
	{
		if (!FClassnameIs(pReturn->pev, STRING(classname)))
		{
			ALERT(at_debug, "Global entity found %s, wrong class %s\n", STRING(globalname),
			      STRING(pReturn->pev->classname));

			pReturn = nullptr;
		}
	}

	return pReturn;
}

int DispatchRestore(edict_t* pent, SAVERESTOREDATA* pSaveData, int globalEntity)
{
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));

	if (pEntity && pSaveData)
	{
		entvars_t tmpVars;
		Vector oldOffset;

		CRestore restoreHelper(pSaveData);
		if (globalEntity)
		{
			CRestore tmpRestore(pSaveData);
			tmpRestore.PrecacheMode(0);
			tmpRestore.ReadEntVars("ENTVARS", &tmpVars);

			// HACKHACK - reset the save pointers, we're going to restore for real this time
			pSaveData->size = pSaveData->pTable[pSaveData->currentIndex].location;
			pSaveData->pCurrentData = pSaveData->pBaseData + pSaveData->size;

			const globalentity_t* pGlobal = gGlobalState.EntityFromTable(tmpVars.globalname);

			// Don't overlay any instance of the global that isn't the latest
			// pSaveData->szCurrentMapName is the level this entity is coming from
			// pGlobla->levelName is the last level the global entity was active in.
			// If they aren't the same, then this global update is out of date.
			if (!FStrEq(pSaveData->szCurrentMapName, pGlobal->levelName))
				return 0;

			// Compute the new global offset
			oldOffset = pSaveData->vecLandmarkOffset;
			CBaseEntity* pNewEntity = FindGlobalEntity(tmpVars.classname, tmpVars.globalname);
			if (pNewEntity)
			{
				//				ALERT( at_console, "Overlay %s with %s\n", STRING(pNewEntity->pev->classname), STRING(tmpVars.classname) );
				// Tell the restore code we're overlaying a global entity from another level
				restoreHelper.SetGlobalMode(1); // Don't overwrite global fields
				pSaveData->vecLandmarkOffset = (pSaveData->vecLandmarkOffset - pNewEntity->pev->mins) + tmpVars.mins;
				pEntity = pNewEntity; // we're going to restore this data OVER the old entity
				pent = ENT(pEntity->pev);
				// Update the global table to say that the global definition of this entity should come from this level
				gGlobalState.EntityUpdate(pEntity->pev->globalname, gpGlobals->mapname);
			}
			else
			{
				// This entity will be freed automatically by the engine.  If we don't do a restore on a matching entity (below)
				// or call EntityUpdate() to move it to this level, we haven't changed global state at all.
				return 0;
			}
		}

		if (pEntity->ObjectCaps() & FCAP_MUST_SPAWN)
		{
			pEntity->Restore(restoreHelper);
			pEntity->Spawn();
		}
		else
		{
			pEntity->Restore(restoreHelper);
			pEntity->Precache();
		}

		// Again, could be deleted, get the pointer again.
		pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));
		if (pEntity) pEntity->pev->colormap = ENTINDEX(pent);
#if 0
		if (pEntity && pEntity->pev->globalname && globalEntity)
		{
			ALERT(at_debug, "Global %s is %s\n", STRING(pEntity->pev->globalname), STRING(pEntity->pev->model));
		}
#endif

		// Is this an overriding global entity (coming over the transition), or one restoring in a level
		if (globalEntity)
		{
			//			ALERT( at_console, "After: %f %f %f %s\n", pEntity->pev->origin.x, pEntity->pev->origin.y, pEntity->pev->origin.z, STRING(pEntity->pev->model) );
			pSaveData->vecLandmarkOffset = oldOffset;
			if (pEntity)
			{
				pEntity->InitMoveWith(); //g-cont. rebuild all parents on next level
				UTIL_SetOrigin(pEntity, pEntity->pev->origin);
				pEntity->OverrideReset();
			}
		}
		else if (pEntity && pEntity->pev->globalname)
		{
			const globalentity_t* pGlobal = gGlobalState.EntityFromTable(pEntity->pev->globalname);
			if (pGlobal)
			{
				// Already dead? delete
				if (pGlobal->state == GLOBAL_DEAD)
					return -1;

				if (!FStrEq(STRING(gpGlobals->mapname), pGlobal->levelName))
				{
					pEntity->MakeDormant(); // Hasn't been moved to this level yet, wait but stay alive
				}
				// In this level & not dead, continue on as normal
			}
			else
			{
				ALERT(at_error, "Global Entity %s (%s) not in table!!!\n", STRING(pEntity->pev->globalname),
				      STRING(pEntity->pev->classname));
				// Spawned entities default to 'On'
				gGlobalState.EntityAdd(pEntity->pev->globalname, gpGlobals->mapname, GLOBAL_ON);
			}
		}
	}

	return 0;
}

void DispatchObjectCollsionBox(edict_t* pent)
{
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));
	if (pEntity)
	{
		pEntity->SetObjectCollisionBox();
	}
	else
		SetObjectCollisionBox(&pent->v);
}

void SaveWriteFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields,
                     int fieldCount)
{
	CSave saveHelper(pSaveData);
	saveHelper.WriteFields("SWF", pname, pBaseData, pFields, fieldCount);
}

void SaveReadFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields,
                    int fieldCount)
{
	CRestore restoreHelper(pSaveData);
	restoreHelper.ReadFields(pname, pBaseData, pFields, fieldCount);
}

edict_t* EHANDLE::Get()
{
	if (m_pent)
	{
		if (m_pent->serialnumber == m_serialnumber)
			return m_pent;
		return nullptr;
	}
	return nullptr;
};

edict_t* EHANDLE::Set(edict_t* pent)
{
	m_pent = pent;
	if (pent)
		m_serialnumber = m_pent->serialnumber;
	return pent;
};

EHANDLE::operator CBaseEntity*()
{
	return static_cast<CBaseEntity*>(GET_PRIVATE(Get()));
};

CBaseEntity* EHANDLE::operator =(CBaseEntity* pEntity)
{
	if (pEntity)
	{
		m_pent = ENT(pEntity->pev);
		if (m_pent)
			m_serialnumber = m_pent->serialnumber;
	}
	else
	{
		m_pent = nullptr;
		m_serialnumber = 0;
	}
	return pEntity;
}

EHANDLE::operator int()
{
	return Get() != nullptr;
}

CBaseEntity* EHANDLE::operator ->()
{
	return static_cast<CBaseEntity*>(GET_PRIVATE(Get()));
}

//LRC
void CBaseEntity::Activate()
{
	//LRC - rebuild the new assistlist as the game starts
	if (m_iLFlags & LF_ASSISTLIST)
	{
		UTIL_AddToAssistList(this);
	}

	//LRC - and the aliaslist too
	if (m_iLFlags & LF_ALIASLIST)
	{
		UTIL_AddToAliasList(static_cast<CBaseMutableAlias*>(this));
	}

	if (m_activated) return;
	m_activated = TRUE;
	InitMoveWith();
	PostSpawn();
}

//LRC- called by activate() to support movewith
void CBaseEntity::InitMoveWith()
{
	SetParent(m_MoveWith);
}

//g-cont. upgrade system to xashParentSystem 0.3 beta
void CBaseEntity::SetParent(int m_iNewParent, int m_iAttachment)
{
	if (!m_iNewParent) //unlink entity from chain
	{
		//ResetParent(); //g-cont. temporary disabled
		return; //disable
	}

	CBaseEntity* pParent;

	if (!m_iAttachment) //try to extract aiment from name
	{
		char* name = (char*)STRING(m_iNewParent);
		for (char* c = name; *c; c++)
		{
			if (*c == '.')
			{
				m_iAttachment = atoi(c + 1);
				name[strlen(name) - 2] = 0;
				pParent = UTIL_FindEntityByTargetname(nullptr, name);
				SetParent(pParent, m_iAttachment);
				return;
			}
		}
	}

	pParent = UTIL_FindEntityByTargetname(nullptr, STRING(m_iNewParent));
	SetParent(pParent, m_iAttachment); //check pointer to valid later
}

void CBaseEntity::SetParent(CBaseEntity* pParent, int m_iAttachment)
{
	m_pMoveWith = pParent;

	if (!m_pMoveWith)
	{
		ALERT(at_debug, "Missing movewith entity %s\n", STRING(m_MoveWith));
		return;
	}

	//	if (pev->targetname)
	//		ALERT(at_console,"Init: %s %s moves with %s\n", STRING(pev->classname), STRING(pev->targetname), STRING(m_MoveWith));
	//	else
	//		ALERT(at_console,"Init: %s moves with %s\n", STRING(pev->classname), STRING(m_MoveWith));

	//check for himself parent
	if (m_pMoveWith == this)
	{
		ALERT(at_debug, "%s has himself parent!\n", STRING(pev->classname));
		return;
	}

	CBaseEntity* pSibling = m_pMoveWith->m_pChildMoveWith;
	while (pSibling) // check that this entity isn't already in the list of children
	{
		if (pSibling == this) break;
		pSibling = pSibling->m_pSiblingMoveWith;
	}
	if (!pSibling) // if movewith is being set up for the first time...
	{
		// add this entity to the list of children
		m_pSiblingMoveWith = m_pMoveWith->m_pChildMoveWith; // may be null: that's fine by me.
		m_pMoveWith->m_pChildMoveWith = this;

		if (m_iAttachment) //parent has attcahment
		{
			if (m_iLFlags & LF_POINTENTITY || pev->flags & FL_MONSTER)
			{
				pev->skin = ENTINDEX(m_pMoveWith->edict());
				pev->body = m_iAttachment;
				pev->aiment = m_pMoveWith->edict();
				pev->movetype = MOVETYPE_FOLLOW;
			}
			else //error
			{
				ALERT(at_debug, "%s not following with aiment %d!(yet)\n", STRING(pev->classname), m_iAttachment);
			}
			return;
		}

		//appllayed to origin
		if (pev->movetype == MOVETYPE_NONE)
		{
			if (pev->solid == SOLID_BSP)
				pev->movetype = MOVETYPE_PUSH;
			else pev->movetype = MOVETYPE_NOCLIP; // or _FLY, perhaps?
			SetBits(m_iLFlags, LF_MOVENONE); //member movetype
		}

		if (m_pMoveWith->pev->movetype == MOVETYPE_WALK) //parent is walking monster?
		{
			SetBits(m_iLFlags, LF_POSTORG); //copy pos from parent every frame
			pev->solid = SOLID_NOT; //set non solid
		}

		m_vecParentOrigin = m_pMoveWith->pev->origin;
		m_vecParentAngles = m_pMoveWith->pev->angles;

		// was the parent shifted at spawn-time?
		if (m_pMoveWith->m_vecSpawnOffset != g_vecZero)
		{
			//ALERT(at_console,"Corrected using SpawnOffset\n");
			// shift this by the same amount the parent was shifted by.
			UTIL_AssignOrigin(this, pev->origin + m_pMoveWith->m_vecSpawnOffset);
			//...and inherit the same offset.
			m_vecSpawnOffset = m_vecSpawnOffset + m_pMoveWith->m_vecSpawnOffset;
		}

		m_vecOffsetOrigin = pev->origin - m_vecParentOrigin;
		m_vecOffsetAngles = pev->angles - m_vecParentAngles;

		if ((m_pMoveWith->m_iLFlags & LF_ANGULAR && m_vecOffsetOrigin != g_vecZero) || m_pMoveWith->m_iLFlags &
			LF_POINTENTITY)
		{
			SetBits(m_iLFlags, LF_POSTORG); //magic stuff
			//GetPInfo( this );
		}

		if (g_serveractive) //maybe parent is moving ?
		{
			pev->velocity = pev->velocity + m_pMoveWith->pev->velocity;
			pev->avelocity = pev->avelocity + m_pMoveWith->pev->avelocity;
		}
	}
}

void CBaseEntity::ResetParent()
{
	CBaseEntity* pTemp;

	if (m_iLFlags & LF_MOVENONE) //this entity was static e.g. func_wall
	{
		ClearBits(m_iLFlags, LF_MOVENONE);
		pev->movetype = MOVETYPE_NONE;
	}

	if (!g_pWorld)
	{
		ALERT(at_debug, "ResetParent has no AssistList!\n");
		return;
	}

	//LRC - remove this from the AssistList.
	for (pTemp = g_pWorld; pTemp->m_pAssistLink != nullptr; pTemp = pTemp->m_pAssistLink)
	{
		if (this == pTemp->m_pAssistLink)
		{
			//			ALERT(at_console,"REMOVE: %s removed from the Assist List.\n", STRING(pev->classname));
			pTemp->m_pAssistLink = this->m_pAssistLink;
			this->m_pAssistLink = nullptr;
			break;
		}
	}

	//LRC
	if (m_pMoveWith)
	{
		// if I'm moving with another entity, take me out of the list. (otherwise things crash!)
		pTemp = m_pMoveWith->m_pChildMoveWith;
		if (pTemp == this)
		{
			m_pMoveWith->m_pChildMoveWith = this->m_pSiblingMoveWith;
		}
		else
		{
			while (pTemp->m_pSiblingMoveWith)
			{
				if (pTemp->m_pSiblingMoveWith == this)
				{
					pTemp->m_pSiblingMoveWith = this->m_pSiblingMoveWith;
					break;
				}
				pTemp = pTemp->m_pSiblingMoveWith;
			}
		}
		//		ALERT(at_console,"REMOVE: %s removed from the %s ChildMoveWith list.\n", STRING(pev->classname), STRING(m_pMoveWith->pev->targetname));
	}

	//LRC - do the same thing if another entity is moving with _me_.
	if (m_pChildMoveWith)
	{
		CBaseEntity* pCur = m_pChildMoveWith;
		CBaseEntity* pNext;
		while (pCur != nullptr)
		{
			pNext = pCur->m_pSiblingMoveWith;

			// bring children to a stop
			UTIL_SetMoveWithVelocity(pCur, g_vecZero, 100);
			UTIL_SetMoveWithAvelocity(pCur, g_vecZero, 100);

			pCur->m_pMoveWith = nullptr;
			pCur->m_pSiblingMoveWith = nullptr;
			pCur = pNext;
		}
	}
}

//LRC
void CBaseEntity::DontThink()
{
	m_fNextThink = 0;
	if (m_pMoveWith == nullptr && m_pChildMoveWith == nullptr)
	{
		pev->nextthink = 0;
		m_fPevNextThink = 0;
	}

	//	ALERT(at_console, "DontThink for %s\n", STRING(pev->targetname));
}

//LRC
// PUSH entities won't have their velocity applied unless they're thinking.
// make them do so for the foreseeable future.
void CBaseEntity::SetEternalThink()
{
	if (pev->movetype == MOVETYPE_PUSH)
	{
		pev->nextthink = pev->ltime + 1E6;
		m_fPevNextThink = pev->nextthink;
	}

	CBaseEntity* pChild;
	for (pChild = m_pChildMoveWith; pChild != nullptr; pChild = pChild->m_pSiblingMoveWith)
		pChild->SetEternalThink();
}

//LRC - for getting round the engine's preconceptions.
// MoveWith entities have to be able to think independently of moving.
// This is how we do so.
void CBaseEntity::SetNextThink(float delay, BOOL correctSpeed)
{
	// now monsters use this method, too.
	if (m_pMoveWith || m_pChildMoveWith || pev->flags & FL_MONSTER)
	{
		// use the Assist system, so that thinking doesn't mess up movement.
		if (pev->movetype == MOVETYPE_PUSH)
			m_fNextThink = pev->ltime + delay;
		else
			m_fNextThink = gpGlobals->time + delay;
		SetEternalThink();
		UTIL_MarkForAssist(this, correctSpeed);

		//ALERT(at_console, "SetAssistedThink for %s: %f\n", STRING(pev->targetname), m_fNextThink);
	}
	else
	{
		// set nextthink as normal.
		if (pev->movetype == MOVETYPE_PUSH)
		{
			pev->nextthink = pev->ltime + delay;
		}
		else
		{
			pev->nextthink = gpGlobals->time + delay;
		}

		m_fPevNextThink = m_fNextThink = pev->nextthink;

		//if (pev->classname) ALERT(at_console, "SetNormThink for %s: %f\n", STRING(pev->targetname), m_fNextThink);
	}
}

//LRC
void CBaseEntity::AbsoluteNextThink(float time, BOOL correctSpeed)
{
	if (m_pMoveWith || m_pChildMoveWith)
	{
		// use the Assist system, so that thinking doesn't mess up movement.
		m_fNextThink = time;
		SetEternalThink();
		UTIL_MarkForAssist(this, correctSpeed);
	}
	else
	{
		// set nextthink as normal.
		pev->nextthink = time;
		m_fPevNextThink = m_fNextThink = pev->nextthink;
	}
}

//LRC - check in case the engine has changed our nextthink. (which it does
// on a depressingly frequent basis.)
// for some reason, this doesn't always produce perfect movement - but it's close
// enough for government work. (the player doesn't get stuck, at least.)
void CBaseEntity::ThinkCorrection()
{
	if (pev->nextthink != m_fPevNextThink)
	{
		// The engine has changed our nextthink, in its typical endearing way.
		// Now we have to apply that change in the _right_ places.
		//		ALERT(at_console, "StoredThink corrected for %s \"%s\": %f -> %f\n", STRING(pev->classname), STRING(pev->targetname), m_fNextThink, m_fNextThink + pev->nextthink - m_fPevNextThink);
		m_fNextThink += pev->nextthink - m_fPevNextThink;
		m_fPevNextThink = pev->nextthink;
	}
}

// give health
int CBaseEntity::TakeHealth(float flHealth, int bitsDamageType)
{
	if (!pev->takedamage)
		return 0;

	// heal
	if (pev->health >= pev->max_health)
		return 0;

	const float flOldHealth = pev->health;

	pev->health += flHealth;

	if (pev->health > pev->max_health)
		pev->health = pev->max_health;

	return pev->health - flOldHealth;
}

// give armor
int CBaseEntity::TakeArmor(float flArmor)
{
	if (!pev->takedamage)
		return 0;

	const float flOldArmor = pev->armorvalue;

	pev->armorvalue += flArmor;

	if (pev->armorvalue > MAX_NORMAL_BATTERY)
		pev->armorvalue = MAX_NORMAL_BATTERY;

	return pev->armorvalue - flOldArmor;
}

// inflict damage on this entity.  bitsDamageType indicates type of damage inflicted, ie: DMG_CRUSH

int CBaseEntity::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	Vector vecTemp;

	if (!pev->takedamage)
		return 0;

	// UNDONE: some entity types may be immune or resistant to some bitsDamageType

	// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
	// (that is, no actual entity projectile was involved in the attack so use the shooter's origin). 
	if (pevAttacker == pevInflictor)
	{
		vecTemp = pevInflictor->origin - (VecBModelOrigin(pev));
	}
	else
		// an actual missile was involved.
	{
		vecTemp = pevInflictor->origin - (VecBModelOrigin(pev));
	}

	// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();

	// save damage based on the target's armor level

	// figure momentum add (don't let hurt brushes or other triggers move player)
	if ((!FNullEnt(pevInflictor)) && (pev->movetype == MOVETYPE_WALK || pev->movetype == MOVETYPE_STEP) && (pevAttacker
		->solid != SOLID_TRIGGER))
	{
		Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();

		float flForce = flDamage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

		if (flForce > 1000.0)
			flForce = 1000.0;

		pev->velocity = pev->velocity + vecDir * flForce;
	}

	// do the damage
	pev->health -= flDamage;
	if (pev->health <= 0)
	{
		Killed(pevAttacker, GIB_NORMAL);
		return 0;
	}

	return 1;
}

void CBaseEntity::Killed(entvars_t* pevAttacker, int iGib)
{
	pev->takedamage = DAMAGE_NO;
	pev->deadflag = DEAD_DEAD;
	UTIL_Remove(this);
}

CBaseEntity* CBaseEntity::GetNextTarget()
{
	if (FStringNull(pev->target))
		return nullptr;

	return UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
}

// Global Savedata for Delay
TYPEDESCRIPTION CBaseEntity::m_SaveData[] =
{
	DEFINE_FIELD(CBaseEntity, m_pGoalEnt, FIELD_CLASSPTR),

	DEFINE_FIELD(CBaseEntity, m_MoveWith, FIELD_STRING), //LRC
	DEFINE_FIELD(CBaseEntity, m_pMoveWith, FIELD_CLASSPTR), //LRC
	DEFINE_FIELD(CBaseEntity, m_pChildMoveWith, FIELD_CLASSPTR), //LRC
	DEFINE_FIELD(CBaseEntity, m_pSiblingMoveWith, FIELD_CLASSPTR), //LRC

	DEFINE_FIELD(CBaseEntity, m_iLFlags, FIELD_INTEGER), //LRC
	DEFINE_FIELD(CBaseEntity, m_iStyle, FIELD_INTEGER), //LRC
	DEFINE_FIELD(CBaseEntity, m_vecOffsetOrigin, FIELD_VECTOR), //LRC
	DEFINE_FIELD(CBaseEntity, m_vecOffsetAngles, FIELD_VECTOR), //LRC
	DEFINE_FIELD(CBaseEntity, m_vecPostAssistOrg, FIELD_VECTOR), //LRC
	DEFINE_FIELD(CBaseEntity, m_vecPostAssistAng, FIELD_VECTOR), //LRC
	DEFINE_FIELD(CBaseEntity, m_activated, FIELD_BOOLEAN), //LRC
	DEFINE_FIELD(CBaseEntity, m_fNextThink, FIELD_TIME), //LRC
	DEFINE_FIELD(CBaseEntity, m_fPevNextThink, FIELD_TIME), //LRC

	DEFINE_FIELD(CBaseEntity, m_vecPostAssistVel, FIELD_VECTOR), //LRC
	DEFINE_FIELD(CBaseEntity, m_vecPostAssistAVel, FIELD_VECTOR), //LRC

	DEFINE_FIELD(CBaseEntity, m_pfnThink, FIELD_FUNCTION), // UNDONE: Build table of these!!!
	DEFINE_FIELD(CBaseEntity, m_pfnTouch, FIELD_FUNCTION),
	DEFINE_FIELD(CBaseEntity, m_pfnUse, FIELD_FUNCTION),
	DEFINE_FIELD(CBaseEntity, m_pfnBlocked, FIELD_FUNCTION),
};


int CBaseEntity::Save(CSave& save)
{
	ThinkCorrection(); //LRC

	if (save.WriteEntVars("ENTVARS", pev))
	{
		if (pev->targetname)
			return save.WriteFields(STRING(pev->targetname), "BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

		return save.WriteFields(STRING(pev->classname), "BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));
	}

	return 0;
}

int CBaseEntity::Restore(CRestore& restore)
{
	int status = restore.ReadEntVars("ENTVARS", pev);

	if (status)
		status = restore.ReadFields("BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

	if (pev->modelindex != 0 && !FStringNull(pev->model))
	{
		Vector mins = pev->mins; // Set model is about to destroy these
		Vector maxs = pev->maxs;

		PRECACHE_MODEL((char*)STRING(pev->model));
		SET_MODEL(ENT(pev), STRING(pev->model));
		UTIL_SetSize(pev, mins, maxs); // Reset them
	}

	return status;
}

// Initialize absmin & absmax to the appropriate box
void SetObjectCollisionBox(entvars_t* pev)
{
	if ((pev->solid == SOLID_BSP) &&
		(pev->angles.x || pev->angles.y || pev->angles.z))
	{
		int i;

		float max = 0;
		for (i = 0; i < 3; i++)
		{
			float v = Vfabs(((float *)pev->mins)[i]);
			if (v > max)
				max = v;
			v = Vfabs(((float *)pev->maxs)[i]);
			if (v > max)
				max = v;
		}
		for (i = 0; i < 3; i++)
		{
			static_cast<float*>(pev->absmin)[i] = static_cast<float*>(pev->origin)[i] - max;
			static_cast<float*>(pev->absmax)[i] = static_cast<float*>(pev->origin)[i] + max;
		}
	}
	else
	{
		pev->absmin = pev->origin + pev->mins;
		pev->absmax = pev->origin + pev->maxs;
	}

	pev->absmin.x -= 1;
	pev->absmin.y -= 1;
	pev->absmin.z -= 1;
	pev->absmax.x += 1;
	pev->absmax.y += 1;
	pev->absmax.z += 1;
}

void CBaseEntity::SetObjectCollisionBox()
{
	::SetObjectCollisionBox(pev);
}

int CBaseEntity::Intersects(CBaseEntity* pOther)
{
	if (pOther->pev->absmin.x > pev->absmax.x ||
		pOther->pev->absmin.y > pev->absmax.y ||
		pOther->pev->absmin.z > pev->absmax.z ||
		pOther->pev->absmax.x < pev->absmin.x ||
		pOther->pev->absmax.y < pev->absmin.y ||
		pOther->pev->absmax.z < pev->absmin.z)
		return 0;
	return 1;
}

void CBaseEntity::MakeDormant()
{
	SetBits(pev->flags, FL_DORMANT);

	// Don't touch
	pev->solid = SOLID_NOT;

	// Don't move
	pev->movetype = MOVETYPE_NONE;

	// Don't draw
	SetBits(pev->effects, EF_NODRAW);

	// Don't think
	DontThink();

	// Relink
	UTIL_SetOrigin(this, pev->origin);
}

int CBaseEntity::IsDormant()
{
	return FBitSet(pev->flags, FL_DORMANT) != 0;
}

BOOL CBaseEntity::IsInWorld()
{
	// position 
	if (pev->origin.x >= 4096) return FALSE;
	if (pev->origin.y >= 4096) return FALSE;
	if (pev->origin.z >= 4096) return FALSE;
	if (pev->origin.x <= -4096) return FALSE;
	if (pev->origin.y <= -4096) return FALSE;
	if (pev->origin.z <= -4096) return FALSE;

	// speed
	if (pev->velocity.x >= 2000) return FALSE;
	if (pev->velocity.y >= 2000) return FALSE;
	if (pev->velocity.z >= 2000) return FALSE;
	if (pev->velocity.x <= -2000) return FALSE;
	if (pev->velocity.y <= -2000) return FALSE;
	if (pev->velocity.z <= -2000) return FALSE;

	return TRUE;
}

BOOL CBaseEntity::ShouldToggle(USE_TYPE useType, BOOL currentState)
{
	if (useType != USE_TOGGLE && useType != USE_SET)
	{
		if ((currentState && useType == USE_ON) || (!currentState && useType == USE_OFF))
			return FALSE;
	}

	return TRUE;
}

BOOL CBaseEntity::ShouldToggle(USE_TYPE useType)
{
	STATE currentState = GetState();
	if (useType != USE_TOGGLE && useType != USE_SET)
	{
		switch (currentState)
		{
		case STATE_ON:
		case STATE_TURN_ON:
			if (useType == USE_ON)
				return FALSE;
			break;
		case STATE_OFF:
		case STATE_TURN_OFF:
			if (useType == USE_OFF)
				return FALSE;
			break;
		}
	}
	return TRUE;
}

int CBaseEntity::DamageDecal(int bitsDamageType)
{
	if (pev->rendermode == kRenderTransAlpha)
		return -1;

	if (pev->rendermode != kRenderNormal)
		return DECAL_BPROOF1;

	return DECAL_GUNSHOT1 + RANDOM_LONG(0, 4);
}

// NOTE: szName must be a pointer to constant memory, e.g. "monster_class" because the entity
// will keep a pointer to it after this call.
CBaseEntity* CBaseEntity::Create(char* szName, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner)
{
	edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING(szName));

	if (FNullEnt(pent))
	{
		ALERT(at_debug, "NULL Ent in Create!\n");
		return nullptr;
	}

	CBaseEntity* pEntity = Instance(pent);
	pEntity->pev->owner = pentOwner;
	pEntity->pev->origin = vecOrigin;
	pEntity->pev->angles = vecAngles;

	DispatchSpawn(pEntity->edict());

	return pEntity;
}
