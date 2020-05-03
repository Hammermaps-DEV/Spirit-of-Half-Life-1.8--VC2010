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

#ifndef GAME_SERVER_ENTITIES_CINFOGROUP_H
#define GAME_SERVER_ENTITIES_CINFOGROUP_H

#define SF_GROUP_DEBUG 2

class CInfoGroup : public CPointEntity
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	int GetMember(const char* szMemberName);

	virtual int Save(CSave& save);
	virtual int Restore(CRestore& restore);

	static TYPEDESCRIPTION m_SaveData[];

	int m_cMembers;
	int m_iszMemberName[MAX_MULTI_TARGETS];
	int m_iszMemberValue[MAX_MULTI_TARGETS];
	int m_iszDefaultMember;
};

#endif //GAME_SERVER_ENTITIES_CINFOGROUP_H