//*************************************************************************************
//*************************************************************************************
// File: menu.cpp
//*************************************************************************************
//*************************************************************************************

//#include "stdafx.h"
#include <stdio.h>

extern "C"
{
#include "g_local.h"
#include "p_menu.h"
}
#include "menu.h"

//*************************************************************************************
//*************************************************************************************
// Function: PMenu_Open
//*************************************************************************************
//*************************************************************************************

void PMenu_Open( edict_t *ent, SMenu *entries, int cur, int num )
{
	int      i;
	SMenu    *p;
	SMenuHnd *hnd;

	if ( !ent->client )
		return;

	if ( ent->client->kots_menu ) 
	{
		gi.dprintf( "warning, ent already has a menu\n" );
		PMenu_Close( ent );
	}

	if ( ent->client->showinventory)
	{
		ent->client->showinventory = false;
		return;
	}

	hnd = new SMenuHnd;

	hnd->num     = num;
	hnd->entries = entries;

	if ( cur < 0 || !entries[cur].SelectFunc ) 
	{
		for ( i = 0, p = entries; i < num; i++, p++ )
			if ( p->SelectFunc )
				break;
	} 
	else
		i = cur;

	if ( i >= num )
		hnd->cur = -1;
	else
		hnd->cur = i;

	ent->client->showscores  = true;
	ent->client->kots_inmenu = true;
	ent->client->kots_menu   = hnd;

	PMenu_Update( ent );
	gi.unicast  ( ent, true );
}

//*************************************************************************************
//*************************************************************************************
// Function: PMenu_Close
//*************************************************************************************
//*************************************************************************************

void PMenu_Close( edict_t *ent )
{
	if ( !ent || !ent->client )
		return;

	if ( !ent->client->kots_menu )
		return;

	delete (SMenuHnd *)ent->client->kots_menu;

	ent->client->kots_menu       = NULL;
	ent->client->showscores = false;
}

//*************************************************************************************
//*************************************************************************************
// Function: PMenu_Update
//*************************************************************************************
//*************************************************************************************

void PMenu_Update( edict_t *ent )
{
	int      i;
	int      x;
	char     *t;
	char     string[ 1400 ];
	SMenu    *p;
	SMenuHnd *hnd;
	qboolean alt = false;

	if ( !ent->client->kots_menu ) 
	{
		gi.dprintf( "warning:  ent has no menu\n" );
		return;
	}
	hnd = (SMenuHnd *)ent->client->kots_menu;

	strcpy( string, "xv 32 yv 8 picn inventory " );

	for ( i = 0, p = hnd->entries; i < hnd->num; i++, p++ ) 
	{
		if ( !p->text || !*(p->text) )
			continue; // blank line
		
		t = p->text;
		
		if (*t == '*') 
		{
			alt = true;
			t++;
		}
		sprintf( string + strlen(string), "yv %d ", 32 + i * 8 );
		
		if ( p->align == PMENU_ALIGN_CENTER )
			x = 196/2 - strlen(t)*4 + 64;
		else if ( p->align == PMENU_ALIGN_RIGHT )
			x = 64 + (196 - strlen(t)*8);
		else
			x = 64;

		sprintf( string + strlen(string), "xv %d ",	x - ((hnd->cur == i) ? 8 : 0) );

		if ( hnd->cur == i )
			sprintf( string + strlen(string), "string2 \"\x0d%s\" ", t );
		else if ( alt )
			sprintf( string + strlen(string), "string2 \"%s\" ", t );
		else
			sprintf( string + strlen(string), "string \"%s\" ", t );

		alt = false;
	}
	gi.WriteByte  ( svc_layout );
	gi.WriteString( string );
}

//*************************************************************************************
//*************************************************************************************
// Function: PMenu_Next
//*************************************************************************************
//*************************************************************************************

void PMenu_Next( edict_t *ent )
{
	int      i;
	SMenu    *p;
	SMenuHnd *hnd;

	if ( !ent->client->kots_menu ) 
	{
		gi.dprintf( "warning:  ent has no menu\n" );
		return;
	}
	hnd = (SMenuHnd *)ent->client->kots_menu;

	if ( hnd->cur < 0 )
		return; // no selectable entries

	i = hnd->cur;
	p = hnd->entries + hnd->cur;

	do 
	{
		i++, p++;
		
		if ( i == hnd->num )
			i = 0, p = hnd->entries;

		if ( p->SelectFunc )
			break;
	} while ( i != hnd->cur );

	hnd->cur = i;

	PMenu_Update( ent );
	gi.unicast  ( ent, true );
}

//*************************************************************************************
//*************************************************************************************
// Function: PMenu_Prev
//*************************************************************************************
//*************************************************************************************

void PMenu_Prev( edict_t *ent )
{
	int      i;
	SMenu    *p;
	SMenuHnd *hnd;

	if ( !ent->client->kots_menu ) 
	{
		gi.dprintf( "warning:  ent has no menu\n" );
		return;
	}
	hnd = (SMenuHnd *)ent->client->kots_menu;

	if ( hnd->cur < 0 )
		return; // no selectable entries

	i = hnd->cur;
	p = hnd->entries + hnd->cur;

	do 
	{
		if ( i == 0 ) 
		{
			i = hnd->num - 1;
			p = hnd->entries + i;
		} 
		else
			i--, p--;
		
		if ( p->SelectFunc )
			break;
	} while ( i != hnd->cur );

	hnd->cur = i;

	PMenu_Update( ent );
	gi.unicast  ( ent, true );
}

//*************************************************************************************
//*************************************************************************************
// Function: PMenu_Select
//*************************************************************************************
//*************************************************************************************

void PMenu_Select( edict_t *ent )
{
	SMenu    *p;
	SMenuHnd *hnd;

	if ( !ent->client->kots_menu ) 
	{
		gi.dprintf( "warning:  ent has no menu\n" );
		return;
	}
	hnd = (SMenuHnd *)ent->client->kots_menu;

	if ( hnd->cur < 0 )
		return; // no selectable entries

	p = hnd->entries + hnd->cur;

	if ( p->SelectFunc )
		p->SelectFunc( ent, p );
}

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************

