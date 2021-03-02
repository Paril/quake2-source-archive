///////////////////////////////////////////////////////////////////////
//
//  LTK - ACTION Quake II Bot
//
//  Original file is Copyright(c), Connor Caple 1999, All Rights Reserved
//
//
//	All other files are Copyright(c) Id Software, Inc.
////////////////////////////////////////////////////////////////////////
/*
 * $Header: /LicenseToKill/src/acesrc/ltklist.cpp 1     24/09/99 10:15 Riever $
 *
 * $Log: /LicenseToKill/src/acesrc/ltklist.cpp $
 * 
 * 1     24/09/99 10:15 Riever
 *
 */

//#define _cplusplus
//#include "ltklist.h"
#include "g_local.h"


//==========================
// Add an item to the front of the queue
//==========================
void	LTKLpush_front( searchNode_t *nodedata )
{
}

//==========================
// Add an item to the back of the queue
//==========================
void	LTKLpush_back( searchNode_t *nodedata)
{
}

//==========================
// Remove an item from the front of the queue
//==========================
void	LTKLpop_front( ltklist_t *listdata )
{
}

//==========================
// Get an item from the back of the queue
//==========================
void	LTKLpop_back( ltklist_t *listdata )
{
}

//==========================
// Get a pointer to the front of the queue
//==========================
searchNode_t	*LTKLfront( ltklist_t *listdata )
{
	return listdata->head;
}

//==========================
// Check the status of the list
//==========================
qboolean	LTKLempty( ltklist_t *listdata )
{
	return ( listdata->head == NULL);
}
