//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma warning(disable : 4996)		// consider using strcpy_s instead

#if !defined( WRECTH )
#define WRECTH

typedef struct rect_s
{
	int				left, right, top, bottom;
} wrect_t;

#endif