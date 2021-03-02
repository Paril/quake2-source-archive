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

// ui_options_keys.c -- the key binding menu

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"

/*
=======================================================================

KEYS MENU

=======================================================================
*/
char *bindnames[][2] =
{
{"+forward", 		"Move Forward"},
{"+back", 			"Move Backward"},
{"+moveleft", 		"Move Left"},
{"+moveright", 		"Move Right"},
{"+moveup",			"Jump"},
{"+movedown",		"Crouch"},

{"+attack", 		"Use Item"},
{"weapprev", 		"Select Previous Item"},
{"weapnext", 		"Select Next Item"},
//{"+attack2", 		"alternate attack"},
{"+use", 			"Action"},



//{"+left", 			"turn left"},
//{"+right", 			"turn right"},
//{"+speed", 			"run"},
//{"+strafe", 		"sidestep"},
//{"+lookup", 		"look up"},
//{"+lookdown", 		"look down"},
{"centerview", 		"Center View"},
//{"+mlook", 			"mouse look"},
//{"+klook", 			"keyboard look"},
//{"inven",			"inventory"},
//{"invuse",			"use item"},
//{"invdrop",			"drop item"},
//{"invprev",			"prev item"},
//{"invnext",			"next item"},
//{"cmd help", 		"help computer" }, 


{"save quick",		"Quick Save"},
{"load quick",		"Quick Load"},


{ 0, 0 }
};

int				keys_cursor;
static int		bind_grab;

static menuframework_s	s_keys_menu;
static menuaction_s s_keys_binds[64];
static menuaction_s		s_keys_back_action;

static void M_UnbindCommand (char *command)
{
	int		j;
	int		l;
	char	*b;

	l = strlen(command);

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		// Knightmare- fix bug with key bound to +attack2 being confused with +attack
		if (!strncmp (b, "+attack2", 8) && strncmp (command, "+attack2", 8))
			continue;
		if (!strncmp (b, command, l) )
			Key_SetBinding (j, "");
	}
}

static void M_FindKeysForCommand (char *command, int *twokeys)
{
	int		count;
	int		j;
	int	 	l;
	char	*b;

	twokeys[0] = twokeys[1] = -1;
	l = strlen(command);
	count = 0;

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		// Knightmare- fix bug with key bound to +attack2 being confused with +attack
		if (!strncmp (b, "+attack2", 8) && strncmp (command, "+attack2", 8))
			continue;
		if (!strncmp (b, command, l))
		{
			twokeys[count] = j;
			count++;
			if (count == 2)
				break;
		}
	}
}

int listSize (char* list[][2])
{
	int i=0;
	while (list[i][1])
		i++;

	return i;	
}


static void KeysBackCursorDrawFunc ( menuaction_s *self ) // back action
{
	/*SCR_DrawChar (SCREEN_WIDTH*0.5 - 24, s_keys_menu.y + self->generic.y, ALIGN_CENTER,
					12+((int)(Sys_Milliseconds()/250)&1), 255,255,255,255, false, true);*/

	SCR_DrawChar (SCREEN_WIDTH*0.5 - 24+ (5*sin(anglemod(cl.time*0.01))),
		s_keys_menu.y + self->generic.y,
		ALIGN_CENTER,
					13,
					255,255,255,255, false, true);



/*	R_DrawChar (SCR_ScaledVideo(SCREEN_WIDTH*0.5 - 24), //viddef.width * 0.50 - SCR_ScaledVideo(24),
				SCR_ScaledVideo(s_keys_menu.y + self->generic.y),
				12 + ( ( int ) ( Sys_Milliseconds() / 250 ) & 1),
				SCR_VideoScale(), 255,255,255,255, false, true);*/
}

static void KeyCursorDrawFunc( menuframework_s *menu )
{
	if (bind_grab)
		SCR_DrawChar (menu->x, menu->y + menu->cursor * MENU_LINE_SIZE, ALIGN_CENTER,
						'=', 255,255,255,255, false, true);
	//	R_DrawChar ( SCR_ScaledVideo(menu->x), SCR_ScaledVideo(menu->y + menu->cursor * MENU_LINE_SIZE),
	//						'=', SCR_VideoScale(), 255,255,255,255, false, true);
	else
	{
		/*
		SCR_DrawChar (menu->x, menu->y + menu->cursor * MENU_LINE_SIZE, ALIGN_CENTER,
						12+((int)(Sys_Milliseconds()/250)&1), 255,255,255,255, false, true);
						*/

		SCR_DrawChar (menu->x+ (5*sin(anglemod(cl.time*0.01))),
			menu->y + menu->cursor * MENU_LINE_SIZE,
			ALIGN_CENTER,
						13,
						255,255,255,
						255, false, true);

	}
	/*	R_DrawChar ( SCR_ScaledVideo(menu->x), SCR_ScaledVideo(menu->y + menu->cursor * MENU_LINE_SIZE),
							12 + ( ( int ) ( Sys_Milliseconds() / 250 ) & 1 ),
							SCR_VideoScale(), 255,255,255,255, false, true);*/
}

static void DrawKeyBindingFunc( void *self )
{
	int keys[2];
	menuaction_s *a = ( menuaction_s * ) self;

	M_FindKeysForCommand( bindnames[a->generic.localdata[0]][0], keys);
		
	if (keys[0] == -1)
	{
		Menu_DrawString (a->generic.x + a->generic.parent->x + 16,
						a->generic.y + a->generic.parent->y, "^1<NONE>", 255);

	}
	else
	{
		int x;
		const char *name;
		int alpha;

		if (cursor.menuitem == a)
			alpha=255;
		else
			alpha=160;
			

		name = Key_KeynumToString (keys[0]);



		Menu_DrawString (a->generic.x + a->generic.parent->x + 16,
						a->generic.y + a->generic.parent->y, name , alpha);





		x = strlen(name) * MENU_FONT_SIZE;


		if (keys[1] != -1)
		{
			Menu_DrawString (a->generic.x + a->generic.parent->x + MENU_FONT_SIZE*3 + x,
							a->generic.y + a->generic.parent->y, "or", alpha);
			Menu_DrawString (a->generic.x + a->generic.parent->x + MENU_FONT_SIZE*6 + x,
							a->generic.y + a->generic.parent->y, Key_KeynumToString(keys[1]), alpha);

		}
	}
}

static void KeyBindingFunc( void *self )
{
	menuaction_s *a = ( menuaction_s * ) self;
	int keys[2];

	M_FindKeysForCommand( bindnames[a->generic.localdata[0]][0], keys );

	if (keys[1] != -1)
		M_UnbindCommand( bindnames[a->generic.localdata[0]][0]);

	bind_grab = true;

	Menu_SetStatusBar( &s_keys_menu, "Press a key or button." );
}

void addBindOption (int i, char* list[][2])
{		
	s_keys_binds[i].generic.type	= MTYPE_ACTION;
	s_keys_binds[i].generic.flags  = QMF_GRAYED;
	s_keys_binds[i].generic.x		= 0;
	s_keys_binds[i].generic.y		= i*MENU_LINE_SIZE;
	s_keys_binds[i].generic.ownerdraw = DrawKeyBindingFunc;
	s_keys_binds[i].generic.localdata[0] = i;
	s_keys_binds[i].generic.name	= list[s_keys_binds[i].generic.localdata[0]][1];
	s_keys_binds[i].generic.callback = KeyBindingFunc;

	if (strstr ("MENUSPACE", list[i][0]))
		s_keys_binds[i].generic.type	= MTYPE_SEPARATOR;
}

static void Keys_MenuInit( void )
{
	int BINDS_MAX;
	int i = 0;

	s_keys_menu.x = SCREEN_WIDTH*0.5;
	s_keys_menu.y = SCREEN_HEIGHT*0.5 - 72;
//	s_keys_menu.x = viddef.width * 0.50;
//	s_keys_menu.y = viddef.height * 0.50 - SCR_ScaledVideo(9 * MENU_FONT_SIZE); //was 7.25
	s_keys_menu.nitems = 0;
	s_keys_menu.cursordraw = KeyCursorDrawFunc;

	BINDS_MAX = listSize(bindnames);
	for (i=0;i<BINDS_MAX;i++)
		addBindOption(i, bindnames);

	s_keys_back_action.generic.type = MTYPE_ACTION;
	s_keys_back_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_keys_back_action.generic.x	= 0;
	s_keys_back_action.generic.y	= (BINDS_MAX+2)*MENU_LINE_SIZE;
	s_keys_back_action.generic.name	= " Done";
	s_keys_back_action.generic.callback = UI_BackMenu;
	s_keys_back_action.generic.cursordraw = KeysBackCursorDrawFunc;

	for (i=0;i<BINDS_MAX;i++)
		Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_binds[i] );

	Menu_AddItem( &s_keys_menu, ( void * ) &s_keys_back_action );

	Menu_SetStatusBar( &s_keys_menu, "Press ENTER or LEFT CLICK to change the key. Press BACKSPACE to clear." );
	// Don't center it- it's too large
	//Menu_Center( &s_keys_menu );
}

static void Keys_MenuDraw (void)
{
	Menu_Banner( "m_banner_options" ); // Knightmare added
	Menu_AdjustCursor( &s_keys_menu, 1 );
	Menu_Draw( &s_keys_menu );
}

static const char *Keys_MenuKey( int key )
{
	menuaction_s *item = ( menuaction_s * ) Menu_ItemAtCursor( &s_keys_menu );

	//pressing mouse1 to pick a new bind wont force bind/unbind itself - spaz
	if ( bind_grab && !(cursor.buttonused[MOUSEBUTTON1]&&key==K_MOUSE1))
	{	
		if ( key != K_ESCAPE && key != '`' )
		{
			char cmd[1024];

			Com_sprintf (cmd, sizeof(cmd), "bind \"%s\" \"%s\"\n", Key_KeynumToString(key), bindnames[item->generic.localdata[0]][0]);
			Cbuf_InsertText (cmd);
		}
		
		// Knightmare- added Psychospaz's mouse support
		//dont let selecting with mouse buttons screw everything up
		UI_RefreshCursorButtons();
		if (key==K_MOUSE1)
			cursor.buttonclicks[MOUSEBUTTON1] = -1;

		//Menu_SetStatusBar( &s_keys_menu, "enter to change, backspace to clear" );
		Menu_SetStatusBar( &s_keys_menu, "Press ENTER or LEFT CLICK to change the key. Press BACKSPACE to clear." );
		bind_grab = false;
		return menu_out_sound;
	}

	switch ( key )
	{
	case K_KP_ENTER:
	case K_ENTER:
		if (item == &s_keys_back_action) { // back action hack
			UI_BackMenu(item); return NULL; }
		KeyBindingFunc( item );
		return menu_in_sound;
	case K_BACKSPACE:		// delete bindings
	case K_DEL:				// delete bindings
	case K_KP_DEL:
		M_UnbindCommand( bindnames[item->generic.localdata[0]][0] );
		return menu_out_sound;
	default:
		return Default_MenuKey( &s_keys_menu, key );
	}
}

void M_Menu_Keys_f (void)
{
	Keys_MenuInit();
	UI_PushMenu( Keys_MenuDraw, Keys_MenuKey );
}
