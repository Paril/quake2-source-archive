/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// ui_game_saveload.c -- the single save/load menus

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

#define	MAX_SAVEGAMES	21 // was 15

static menuframework_s	s_loadgame_menu;
static menuaction_s		s_loadgame_actions[MAX_SAVEGAMES];
static menuaction_s		s_loadgame_back_action;

static menuframework_s	s_savegame_menu;
static menuaction_s		s_savegame_actions[MAX_SAVEGAMES];
static menuaction_s		s_savegame_back_action;


/*
=============================================================================

SAVESHOT HANDLING

=============================================================================
*/

char		m_savestrings[MAX_SAVEGAMES][32];
qboolean	m_savevalid[MAX_SAVEGAMES];
qboolean	m_saveshotvalid[MAX_SAVEGAMES+1];

char		m_mapname[MAX_QPATH];
qboolean	m_mapshotvalid;

void Create_Savestrings (void)
{
	int		i;
	fileHandle_t	f;
	char	name[MAX_OSPATH];
	char	mapname[MAX_TOKEN_CHARS];
	char	*ch;

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
	{
		Com_sprintf (name, sizeof(name), "save/save%i/server.ssv", i);
		FS_FOpenFile (name, &f, FS_READ);
		if (!f)
		{
			//Com_Printf("Save file %s not found.\n", name);
			strcpy (m_savestrings[i], "- EMPTY -");
			m_savevalid[i] = false;
		}
		else
		{
			FS_Read (m_savestrings[i], sizeof(m_savestrings[i]), f);

			if (i==0) { // grab mapname
				FS_Read (mapname, sizeof(mapname), f);
				if (mapname[0] == '*') // skip * marker
					Com_sprintf (m_mapname, sizeof(m_mapname), mapname+1);
				else
					Com_sprintf (m_mapname, sizeof(m_mapname), mapname);
				if (ch = strchr (m_mapname, '$'))
					*ch = 0; // terminate string at $ marker
			}
			FS_FCloseFile(f);
			m_savevalid[i] = true;
		}
	}
}

void InitSaveshots (qboolean loadmenu)
{
	int i;
	char shotname [MAX_QPATH];
	char mapshotname [MAX_QPATH];

	for ( i = 1; i < MAX_SAVEGAMES; i++ )
	{	// free previously loaded shots
		Com_sprintf(shotname, sizeof(shotname), "save/save%i/shot.jpg", i);
		R_FreePic (shotname);
		Com_sprintf(shotname, sizeof(shotname), "/save/save%i/shot.jpg", i);
		if (R_DrawFindPic(shotname))
			m_saveshotvalid[i] = true;
		else
			m_saveshotvalid[i] = false;
	}
	if (loadmenu)
	{	// register mapshot for autosave
		if (m_savevalid[0]) {
			Com_sprintf(mapshotname, sizeof(mapshotname), "/levelshots/%s.pcx", m_mapname);
			if (R_DrawFindPic(mapshotname))
				m_mapshotvalid = true;
			else
				m_mapshotvalid = false;
		}
		else
			m_mapshotvalid = false;
	}
	// register null saveshot
	if (R_DrawFindPic("/gfx/noscreen.pcx"))
		m_saveshotvalid[MAX_SAVEGAMES] = true;
	else
		m_saveshotvalid[MAX_SAVEGAMES] = false;
}

void DrawSaveshot (qboolean loadmenu)
{
	char shotname [MAX_QPATH];
	char mapshotname [MAX_QPATH];
	int i;
	if (loadmenu)
		i = s_loadgame_actions[s_loadgame_menu.cursor].generic.localdata[0];
	else
		i = s_savegame_actions[s_savegame_menu.cursor].generic.localdata[0];

	SCR_DrawFill2 (SCREEN_WIDTH/2+44, SCREEN_HEIGHT/2-60, 244, 184, ALIGN_CENTER, 60,60,60,255);

	if ( m_savevalid[i] && m_saveshotvalid[i] )
	{
		Com_sprintf(shotname, sizeof(shotname), "/save/save%i/shot.jpg", i);

		SCR_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-58, 240, 180, ALIGN_CENTER, shotname, 1.0);
	}
	else if ( loadmenu && i==0 && m_savevalid[i] && m_mapshotvalid ) // autosave shows mapshot
	{
		Com_sprintf(mapshotname, sizeof(mapshotname), "/levelshots/%s.pcx", m_mapname);

		SCR_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-58, 240, 180, ALIGN_CENTER, mapshotname, 1.0);
	}
	else if (m_saveshotvalid[MAX_SAVEGAMES])
		SCR_DrawPic (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-58, 240, 180, ALIGN_CENTER, "/gfx/noscreen.pcx", 1.0);
	else
		SCR_DrawFill2 (SCREEN_WIDTH/2+46, SCREEN_HEIGHT/2-58, 240, 180, ALIGN_CENTER, 0,0,0,255);
}


/*
=============================================================================

LOADGAME MENU

=============================================================================
*/

extern	char *load_saveshot;
char loadshotname[MAX_QPATH];

void LoadGameCallback( void *self )
{
	menuaction_s *a = ( menuaction_s * ) self;

	// set saveshot name here
	if ( m_saveshotvalid[ a->generic.localdata[0] ] ) {
		Com_sprintf(loadshotname, sizeof(loadshotname), "/save/save%i/shot.jpg", a->generic.localdata[0]);
		load_saveshot = loadshotname; }
	else
		load_saveshot = NULL;

	if ( m_savevalid[ a->generic.localdata[0] ] )
		Cbuf_AddText (va("load save%i\n",  a->generic.localdata[0] ) );
	UI_ForceMenuOff ();
}

void LoadGame_MenuInit ( void )
{
	int i;

	s_loadgame_menu.x = SCREEN_WIDTH*0.5 - 240;
	s_loadgame_menu.y = SCREEN_HEIGHT*0.5 - 58;
//	s_loadgame_menu.x = viddef.width / 2 - SCR_ScaledVideo(240);
//	s_loadgame_menu.y = viddef.height / 2 - SCR_ScaledVideo(7.25*MENU_FONT_SIZE); // was 58
	s_loadgame_menu.nitems = 0;

	Create_Savestrings();

	for ( i = 0; i < MAX_SAVEGAMES; i++ )
	{
		s_loadgame_actions[i].generic.name			= m_savestrings[i];
		s_loadgame_actions[i].generic.flags			= QMF_LEFT_JUSTIFY;
		s_loadgame_actions[i].generic.localdata[0]	= i;
		s_loadgame_actions[i].generic.callback		= LoadGameCallback;

		s_loadgame_actions[i].generic.x = 0;
		s_loadgame_actions[i].generic.y = (i) * MENU_LINE_SIZE;
		if (i>1)	// separate from autosave
			s_loadgame_actions[i].generic.y += 10;

		s_loadgame_actions[i].generic.type = MTYPE_ACTION;

		Menu_AddItem( &s_loadgame_menu, &s_loadgame_actions[i] );
	}

	s_loadgame_back_action.generic.type	= MTYPE_ACTION;
	s_loadgame_back_action.generic.flags  = QMF_LEFT_JUSTIFY;
	s_loadgame_back_action.generic.x		= 0;
	s_loadgame_back_action.generic.y		= (MAX_SAVEGAMES+3)*MENU_LINE_SIZE;
	s_loadgame_back_action.generic.name		= " CANCEL";
	s_loadgame_back_action.generic.callback = UI_BackMenu;

	Menu_AddItem( &s_loadgame_menu, &s_loadgame_back_action );

	InitSaveshots (true); // register saveshots
}

void LoadGame_MenuDraw( void )
{
	Menu_Banner( "m_banner_load_game" );
//	Menu_AdjustCursor( &s_loadgame_menu, 1 );
	Menu_Draw( &s_loadgame_menu );
	DrawSaveshot (true);
}

const char *LoadGame_MenuKey( int key )
{
	if ( key == K_ESCAPE || key == K_ENTER )
	{
		s_savegame_menu.cursor = s_loadgame_menu.cursor - 1;
		if ( s_savegame_menu.cursor < 0 )
			s_savegame_menu.cursor = 0;
	}
	return Default_MenuKey( &s_loadgame_menu, key );
}

void M_Menu_LoadGame_f (void)
{
	LoadGame_MenuInit();
	UI_PushMenu( LoadGame_MenuDraw, LoadGame_MenuKey );
}


/*
=============================================================================

SAVEGAME MENU

=============================================================================
*/

void SaveGameCallback( void *self )
{
	menuaction_s *a = ( menuaction_s * ) self;

	Cbuf_AddText (va("save save%i\n", a->generic.localdata[0] ));
	UI_ForceMenuOff ();
}

void SaveGame_MenuDraw( void )
{
	Menu_Banner( "m_banner_save_game" );
	Menu_AdjustCursor( &s_savegame_menu, 1 );
	Menu_Draw( &s_savegame_menu );
	DrawSaveshot (false);
}

void SaveGame_MenuInit( void )
{
	int i;

	s_savegame_menu.x = SCREEN_WIDTH*0.5 - 240;
	s_savegame_menu.y = SCREEN_HEIGHT*0.5 - 58;
//	s_savegame_menu.x = viddef.width / 2 - SCR_ScaledVideo(240);
//	s_savegame_menu.y = viddef.height / 2 - SCR_ScaledVideo(7.25*MENU_FONT_SIZE);//was 58
	s_savegame_menu.nitems = 0;

	Create_Savestrings();

	// don't include the autosave slot
	for ( i = 0; i < MAX_SAVEGAMES-2; i++ )
	{
		s_savegame_actions[i].generic.name = m_savestrings[i+2];
		s_savegame_actions[i].generic.localdata[0] = i+2;
		s_savegame_actions[i].generic.flags = QMF_LEFT_JUSTIFY;
		s_savegame_actions[i].generic.callback = SaveGameCallback;

		s_savegame_actions[i].generic.x = 0;
		s_savegame_actions[i].generic.y = ( i ) * MENU_LINE_SIZE;

		s_savegame_actions[i].generic.type = MTYPE_ACTION;

		Menu_AddItem( &s_savegame_menu, &s_savegame_actions[i] );
	}
	s_savegame_back_action.generic.type	= MTYPE_ACTION;
	s_savegame_back_action.generic.flags  = QMF_LEFT_JUSTIFY;
	s_savegame_back_action.generic.x		= 0;
	s_savegame_back_action.generic.y		= (MAX_SAVEGAMES+2)*MENU_LINE_SIZE;
	s_savegame_back_action.generic.name		= " CANCEL";
	s_savegame_back_action.generic.callback = UI_BackMenu;

	Menu_AddItem( &s_savegame_menu, &s_savegame_back_action );


	InitSaveshots (false);
}

const char *SaveGame_MenuKey( int key )
{
	if ( key == K_ENTER || key == K_ESCAPE )
	{
		s_loadgame_menu.cursor = s_savegame_menu.cursor - 1;
		if ( s_loadgame_menu.cursor < 0 )
			s_loadgame_menu.cursor = 0;
	}
	return Default_MenuKey( &s_savegame_menu, key );
}

void M_Menu_SaveGame_f (void)
{
	if (!Com_ServerState())
		return;		// not playing a game

	SaveGame_MenuInit();
	UI_PushMenu( SaveGame_MenuDraw, SaveGame_MenuKey );
	Create_Savestrings ();
}
