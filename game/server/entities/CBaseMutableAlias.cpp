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
*   NEW file for the Mod "Spirit of Half-Life", by Laurie R. Cheers. (LRC)
*   Created 19/11/00
*   Modified by Andrew J Hamilton (AJH) 2/04/04
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "CBaseMutableAlias.h"

TYPEDESCRIPTION CBaseMutableAlias::m_SaveData[] =
{
	DEFINE_FIELD(CBaseMutableAlias, m_pNextAlias, FIELD_CLASSPTR),
};
IMPLEMENT_SAVERESTORE(CBaseMutableAlias, CPointEntity);