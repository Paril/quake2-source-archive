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

// ui_playersetup.c -- the player setup menu 

#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "../client/client.h"
#include "ui_local.h"


/*
=============================================================================

PLAYER CONFIG MENU

=============================================================================
*/
extern menuframework_s	s_multiplayer_menu;

static menuframework_s	s_player_config_menu;
static menufield_s		s_player_name_field;
static menulist_s		s_player_model_box;
static menulist_s		s_player_skin_box;
static menulist_s		s_player_handedness_box;
static menulist_s		s_player_rate_box;
static menuseparator_s	s_player_skin_title;
static menuseparator_s	s_player_model_title;
static menuseparator_s	s_player_hand_title;
static menuseparator_s	s_player_rate_title;
//static menuaction_s		s_player_download_action;
static menuaction_s		s_player_back_action;

// Knightmare- save skins and models here so as to not have to re-register every frame
struct model_s *playermodel;
struct model_s *weaponmodel;
struct image_s *playerskin;
char *currentweaponmodel;
// end Knightmare

#define MAX_DISPLAYNAME 16
#define MAX_PLAYERMODELS 1024
#define	NUM_SKINBOX_ITEMS 7

typedef struct
{
	int		nskins;
	char	**skindisplaynames;
	char	displayname[MAX_DISPLAYNAME];
	char	directory[MAX_QPATH];
} playermodelinfo_s;

static playermodelinfo_s s_pmi[MAX_PLAYERMODELS];
static char *s_pmnames[MAX_PLAYERMODELS];
static int s_numplayermodels;

static int rate_tbl[] = { 2500, 3200, 5000, 10000, 25000, 0 };
static const char *rate_names[] = { "28.8 Modem", "33.6 Modem", "Single ISDN",
	"Dual ISDN/Cable", "T1/LAN", "User defined", 0 };

void DownloadOptionsFunc( void *self )
{
	M_Menu_DownloadOptions_f();
}

static void HandednessCallback( void *unused )
{
	Cvar_SetValue( "hand", s_player_handedness_box.curvalue );
}

static void RateCallback( void *unused )
{
	if (s_player_rate_box.curvalue != sizeof(rate_tbl) / sizeof(*rate_tbl) - 1)
		Cvar_SetValue( "rate", rate_tbl[s_player_rate_box.curvalue] );
}

static void ModelCallback( void *unused )
{
	char scratch[MAX_QPATH];

	s_player_skin_box.itemnames = s_pmi[s_player_model_box.curvalue].skindisplaynames;
	s_player_skin_box.curvalue = 0;

	// Knightmare- only register model and skin on starup or when changed
	Com_sprintf( scratch, sizeof( scratch ), "players/%s/tris.md2", s_pmi[s_player_model_box.curvalue].directory );
	playermodel = R_RegisterModel( scratch );

	Com_sprintf( scratch, sizeof( scratch ), "players/%s/%s.pcx", s_pmi[s_player_model_box.curvalue].directory, s_pmi[s_player_model_box.curvalue].skindisplaynames[s_player_skin_box.curvalue] );
	playerskin = R_RegisterSkin( scratch );

	// show current weapon model (if any)
	if (currentweaponmodel && strlen(currentweaponmodel)) {
		Com_sprintf( scratch, sizeof( scratch ), "players/%s/%s", s_pmi[s_player_model_box.curvalue].directory, currentweaponmodel );
		weaponmodel = R_RegisterModel( scratch );
		if (!weaponmodel) {
			Com_sprintf( scratch, sizeof( scratch ), "players/%s/weapon.md2", s_pmi[s_player_model_box.curvalue].directory );
			weaponmodel = R_RegisterModel( scratch );
		}
	}
	else {
		Com_sprintf( scratch, sizeof( scratch ), "players/%s/weapon.md2", s_pmi[s_player_model_box.curvalue].directory );
		weaponmodel = R_RegisterModel( scratch );
	}
	// end Knightmare
}

// Knightmare- only register skin on starup and when changed
static void SkinCallback( void *unused )
{
	char scratch[MAX_QPATH];

	Com_sprintf( scratch, sizeof( scratch ), "players/%s/%s.pcx", s_pmi[s_player_model_box.curvalue].directory, s_pmi[s_player_model_box.curvalue].skindisplaynames[s_player_skin_box.curvalue] );
	playerskin = R_RegisterSkin( scratch );
}


// Knightmare- Psychospaz's menu support for TGA and JPG skins
//static qboolean IconOfSkinExists( char *skin, char **pcxfiles, int npcxfiles )
static qboolean IconOfSkinExists( char *skin, char **files, int nfiles, char *suffix )
{
	int i;
	char scratch[1024];

	strcpy( scratch, skin );
	*strrchr( scratch, '.' ) = 0;
	//strcat( scratch, "_i.pcx" );
	strcat( scratch, suffix );

	for ( i = 0; i < nfiles; i++ )
	{
		if ( strcmp( files[i], scratch ) == 0 )
			return true;
	}

	return false;
}


// Knightmare- added Psychospaz's menus support for JPG and TGA skins
static qboolean PlayerConfig_ScanDirectories (void)
{
	char findname[1024];
	char scratch[1024];
	int ndirs = 0, npms = 0;
	char **dirnames;
	char *path = NULL;
	int i;

	extern char **FS_ListFiles( char *, int *, unsigned, unsigned );

	s_numplayermodels = 0;

	// Knightmare- loop back to here if there were no valid player models found in the selected path
	do
	{
		//
		// get a list of directories
		//
		do 
		{
			path = FS_NextPath( path );
			Com_sprintf( findname, sizeof(findname), "%s/players/*.*", path );

			if ( ( dirnames = FS_ListFiles( findname, &ndirs, SFF_SUBDIR, 0 ) ) != 0)
				break;
		} while ( path );

		if ( !dirnames )
			return false;

		//
		// go through the subdirectories
		//
		npms = ndirs;
		if ( npms > MAX_PLAYERMODELS )
			npms = MAX_PLAYERMODELS;

		for ( i = 0; i < npms; i++ )
		{
			int k, s;
			char	*a, *b, *c;
			char	**skinnames;
			char	**imagenames;
			int		nimagefiles;
			int		nskins = 0;

			if ( dirnames[i] == 0 )
				continue;

			// verify the existence of tris.md2
			strcpy( scratch, dirnames[i] );
			strcat( scratch, "/tris.md2" );
			if ( !Sys_FindFirst( scratch, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM ) )
			{
				free( dirnames[i] );
				dirnames[i] = 0;
				Sys_FindClose();
				continue;
			}
			Sys_FindClose();

			// verify the existence of at least one skin
			//strcpy( scratch, dirnames[i] );
			//strcat( scratch, "/*.pcx" );
			strcpy( scratch, va("%s%s", dirnames[i], "/*.*" ));

			imagenames = FS_ListFiles( scratch, &nimagefiles, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM );

			if ( !imagenames )
			{
				free( dirnames[i] );
				dirnames[i] = 0;
				continue;
			}

			// count valid skins, which consist of a skin with a matching "_i" icon
			/*for ( k = 0; k < nimagefiles-1; k++ )
			{
				if ( !strstr( imagenames[k], "_i.pcx" ) )
				{
					if ( IconOfSkinExists( imagenames[k], imagenames, nimagefiles - 1 ) )
					{
						nskins++;
					}
				}
			}*/
			for ( k = 0; k < nimagefiles-1; k++ )
				if (	!strstr( imagenames[k], "_i.pcx")
					||  !strstr( imagenames[k], "_i.jpg" )
					||  !strstr( imagenames[k], "_i.tga") )
					if (	IconOfSkinExists( imagenames[k], imagenames, nimagefiles - 1 , "_i.pcx")
						||	IconOfSkinExists( imagenames[k], imagenames, nimagefiles - 1 , "_i.jpg")
						||	IconOfSkinExists( imagenames[k], imagenames, nimagefiles - 1 , "_i.tga"))
						nskins++;

			if ( !nskins )
				continue;

			skinnames = malloc( sizeof( char * ) * ( nskins + 1 ) );
			memset( skinnames, 0, sizeof( char * ) * ( nskins + 1 ) );

			// copy the valid skins
			if (nimagefiles)
				for ( s = 0, k = 0; k < nimagefiles-1; k++ )
				{
					char *a, *b, *c;

				//	if ( !strstr( imagenames[k], "_i.pcx" ) )
					if (	!strstr( imagenames[k], "_i.pcx")
						||  !strstr( imagenames[k], "_i.jpg" )
						||  !strstr( imagenames[k], "_i.tga") )
					{
					//	if ( IconOfSkinExists( imagenames[k], imagenames, nimagefiles - 1, "_i.pcx"))
						if (	IconOfSkinExists( imagenames[k], imagenames, nimagefiles - 1 , "_i.pcx")
							||	IconOfSkinExists( imagenames[k], imagenames, nimagefiles - 1 , "_i.jpg")
							||	IconOfSkinExists( imagenames[k], imagenames, nimagefiles - 1 , "_i.tga"))
						{
							a = strrchr( imagenames[k], '/' );
							b = strrchr( imagenames[k], '\\' );

							if ( a > b )
								c = a;
							else
								c = b;

							strcpy( scratch, c + 1 );

							if ( strrchr( scratch, '.' ) )
								*strrchr( scratch, '.' ) = 0;

							skinnames[s] = strdup( scratch );
							s++;
						}
					}
				}

			// at this point we have a valid player model
			s_pmi[s_numplayermodels].nskins = nskins;
			s_pmi[s_numplayermodels].skindisplaynames = skinnames;

			// make short name for the model
			a = strrchr( dirnames[i], '/' );
			b = strrchr( dirnames[i], '\\' );

			if ( a > b )
				c = a;
			else
				c = b;

			strncpy( s_pmi[s_numplayermodels].displayname, c + 1, MAX_DISPLAYNAME-1 );
			strcpy( s_pmi[s_numplayermodels].directory, c + 1 );

			FreeFileList( imagenames, nimagefiles );

			s_numplayermodels++;
		}
		
		if ( dirnames )
			FreeFileList( dirnames, ndirs );

	// if no valid player models found in path,
	// try next path, if there is one
	} while (s_numplayermodels == 0 && path);

	return true;	//** DMP warning fix
}

static int pmicmpfnc( const void *_a, const void *_b )
{
	const playermodelinfo_s *a = ( const playermodelinfo_s * ) _a;
	const playermodelinfo_s *b = ( const playermodelinfo_s * ) _b;

	//
	// sort by male, female, then alphabetical
	//
	if ( strcmp( a->directory, "male" ) == 0 )
		return -1;
	else if ( strcmp( b->directory, "male" ) == 0 )
		return 1;

	if ( strcmp( a->directory, "female" ) == 0 )
		return -1;
	else if ( strcmp( b->directory, "female" ) == 0 )
		return 1;

	return strcmp( a->directory, b->directory );
}


qboolean PlayerConfig_MenuInit( void )
{
	extern cvar_t *name;
	extern cvar_t *team;
	extern cvar_t *skin;
	char currentdirectory[1024];
	char currentskin[1024];
	char scratch[MAX_QPATH];
	int i = 0;
	int y = 0;

	int currentdirectoryindex = 0;
	int currentskinindex = 0;

	cvar_t *hand = Cvar_Get( "hand", "0", CVAR_USERINFO | CVAR_ARCHIVE );

	static const char *handedness[] = { "right", "left", "center", 0 };

	PlayerConfig_ScanDirectories();

	if (s_numplayermodels == 0)
		return false;

	if ( hand->value < 0 || hand->value > 2 )
		Cvar_SetValue( "hand", 0 );

	strcpy( currentdirectory, skin->string );

	if ( strchr( currentdirectory, '/' ) )
	{
		strcpy( currentskin, strchr( currentdirectory, '/' ) + 1 );
		*strchr( currentdirectory, '/' ) = 0;
	}
	else if ( strchr( currentdirectory, '\\' ) )
	{
		strcpy( currentskin, strchr( currentdirectory, '\\' ) + 1 );
		*strchr( currentdirectory, '\\' ) = 0;
	}
	else
	{
		strcpy( currentdirectory, "male" );
		strcpy( currentskin, "grunt" );
	}

	qsort( s_pmi, s_numplayermodels, sizeof( s_pmi[0] ), pmicmpfnc );

	memset( s_pmnames, 0, sizeof( s_pmnames ) );
	for ( i = 0; i < s_numplayermodels; i++ )
	{
		s_pmnames[i] = s_pmi[i].displayname;
		if ( Q_stricmp( s_pmi[i].directory, currentdirectory ) == 0 )
		{
			int j;

			currentdirectoryindex = i;

			for ( j = 0; j < s_pmi[i].nskins; j++ )
			{
				if ( Q_stricmp( s_pmi[i].skindisplaynames[j], currentskin ) == 0 )
				{
					currentskinindex = j;
					break;
				}
			}
		}
	}
	
	s_player_config_menu.x = SCREEN_WIDTH*0.5 - 210;
	s_player_config_menu.y = SCREEN_HEIGHT*0.5 - 70;
//	s_player_config_menu.x = viddef.width  / 2 - SCR_ScaledVideo(210);
//	s_player_config_menu.y = viddef.height / 2 - SCR_ScaledVideo(70.0);
	s_player_config_menu.nitems = 0;
	
	s_player_name_field.generic.type = MTYPE_FIELD;
	s_player_name_field.generic.flags = QMF_LEFT_JUSTIFY;
	s_player_name_field.generic.name = "name";
	s_player_name_field.generic.callback = 0;
	s_player_name_field.generic.x		= -MENU_FONT_SIZE;
	s_player_name_field.generic.y		= y;
	s_player_name_field.length	= 20;
	s_player_name_field.visible_length = 20;
	strcpy( s_player_name_field.buffer, name->string );
	s_player_name_field.cursor = strlen( name->string );
	
	s_player_model_title.generic.type = MTYPE_SEPARATOR;
	s_player_model_title.generic.flags = QMF_LEFT_JUSTIFY;
	s_player_model_title.generic.name = "model";
	s_player_model_title.generic.x    = -MENU_FONT_SIZE;
	s_player_model_title.generic.y	 = y += 3*MENU_LINE_SIZE;
	
	s_player_model_box.generic.type = MTYPE_SPINCONTROL;
	s_player_model_box.generic.x	= -7*MENU_FONT_SIZE;
	s_player_model_box.generic.y	= y += MENU_LINE_SIZE;
	s_player_model_box.generic.callback = ModelCallback;
	s_player_model_box.generic.cursor_offset = -6*MENU_FONT_SIZE;
	s_player_model_box.curvalue = currentdirectoryindex;
	s_player_model_box.itemnames = s_pmnames;
	
	s_player_skin_title.generic.type = MTYPE_SEPARATOR;
	s_player_skin_title.generic.flags = QMF_LEFT_JUSTIFY;
	s_player_skin_title.generic.name = "skin";
	s_player_skin_title.generic.x    = -2*MENU_FONT_SIZE;
	s_player_skin_title.generic.y	 = y += 2*MENU_LINE_SIZE;
	
	s_player_skin_box.generic.type = MTYPE_SPINCONTROL;
	s_player_skin_box.generic.x	= -7*MENU_FONT_SIZE;
	s_player_skin_box.generic.y	= y += MENU_LINE_SIZE;
	s_player_skin_box.generic.name	= 0;
	s_player_skin_box.generic.callback = SkinCallback; // Knightmare added, was 0
	s_player_skin_box.generic.cursor_offset = -6*MENU_FONT_SIZE;
	s_player_skin_box.curvalue = currentskinindex;
	s_player_skin_box.itemnames = s_pmi[currentdirectoryindex].skindisplaynames;
	s_player_skin_box.generic.flags |= QMF_SKINLIST;
	
	s_player_hand_title.generic.type = MTYPE_SEPARATOR;
	s_player_hand_title.generic.flags = QMF_LEFT_JUSTIFY;
	s_player_hand_title.generic.name = "handedness";
	s_player_hand_title.generic.x    = 4*MENU_FONT_SIZE;
	s_player_hand_title.generic.y	 = y += 2*MENU_LINE_SIZE;
	
	s_player_handedness_box.generic.type = MTYPE_SPINCONTROL;
	s_player_handedness_box.generic.x	= -7*MENU_FONT_SIZE;
	s_player_handedness_box.generic.y	= y += MENU_LINE_SIZE;
	s_player_handedness_box.generic.name	= 0;
	s_player_handedness_box.generic.cursor_offset = -6*MENU_FONT_SIZE;
	s_player_handedness_box.generic.callback = HandednessCallback;
	s_player_handedness_box.curvalue = Cvar_VariableValue( "hand" );
	s_player_handedness_box.itemnames = handedness;
	
	for (i = 0; i < sizeof(rate_tbl) / sizeof(*rate_tbl) - 1; i++)
		if (Cvar_VariableValue("rate") == rate_tbl[i])
			break;
		
	s_player_rate_title.generic.type = MTYPE_SEPARATOR;
	s_player_rate_title.generic.flags = QMF_LEFT_JUSTIFY;
	s_player_rate_title.generic.name = "connect speed";
	s_player_rate_title.generic.x    = 7*MENU_FONT_SIZE;
	s_player_rate_title.generic.y	 = y += 2*MENU_LINE_SIZE;
		
	s_player_rate_box.generic.type = MTYPE_SPINCONTROL;
	s_player_rate_box.generic.x	= -7*MENU_FONT_SIZE;
	s_player_rate_box.generic.y	= y += MENU_LINE_SIZE;
	s_player_rate_box.generic.name	= 0;
	s_player_rate_box.generic.cursor_offset = -6*MENU_FONT_SIZE;
	s_player_rate_box.generic.callback = RateCallback;
	s_player_rate_box.curvalue = i;
	s_player_rate_box.itemnames = rate_names;
	
	s_player_back_action.generic.type = MTYPE_ACTION;
	s_player_back_action.generic.name	= "back to multiplayer";
	s_player_back_action.generic.flags = QMF_LEFT_JUSTIFY;
	s_player_back_action.generic.x	= -5*MENU_FONT_SIZE;
	s_player_back_action.generic.y	= y += 2*MENU_LINE_SIZE;
	s_player_back_action.generic.statusbar = NULL;
	s_player_back_action.generic.callback = UI_BackMenu;

	// Knightmare- only register model and skin on starup or when changed
	Com_sprintf( scratch, sizeof( scratch ), "players/%s/tris.md2", s_pmi[s_player_model_box.curvalue].directory );
	playermodel = R_RegisterModel( scratch );

	Com_sprintf( scratch, sizeof( scratch ), "players/%s/%s.pcx", s_pmi[s_player_model_box.curvalue].directory, s_pmi[s_player_model_box.curvalue].skindisplaynames[s_player_skin_box.curvalue] );
	playerskin = R_RegisterSkin( scratch );

	// show current weapon model (if any)
	if (currentweaponmodel && strlen(currentweaponmodel)) {
		Com_sprintf( scratch, sizeof( scratch ), "players/%s/%s", s_pmi[s_player_model_box.curvalue].directory, currentweaponmodel );
		weaponmodel = R_RegisterModel( scratch );
		if (!weaponmodel) {
			Com_sprintf( scratch, sizeof( scratch ), "players/%s/weapon.md2", s_pmi[s_player_model_box.curvalue].directory );
			weaponmodel = R_RegisterModel( scratch );
		}
	}
	else
	{
		Com_sprintf( scratch, sizeof( scratch ), "players/%s/weapon.md2", s_pmi[s_player_model_box.curvalue].directory );
		weaponmodel = R_RegisterModel( scratch );
	}
	// end Knightmare

	Menu_AddItem( &s_player_config_menu, &s_player_name_field );
	Menu_AddItem( &s_player_config_menu, &s_player_model_title );
	Menu_AddItem( &s_player_config_menu, &s_player_model_box );
	if ( s_player_skin_box.itemnames )
	{
		Menu_AddItem( &s_player_config_menu, &s_player_skin_title );
		Menu_AddItem( &s_player_config_menu, &s_player_skin_box );
	}
	Menu_AddItem( &s_player_config_menu, &s_player_hand_title );
	Menu_AddItem( &s_player_config_menu, &s_player_handedness_box );
	Menu_AddItem( &s_player_config_menu, &s_player_rate_title );
	Menu_AddItem( &s_player_config_menu, &s_player_rate_box );
	//Menu_AddItem( &s_player_config_menu, &s_player_download_action );
	Menu_AddItem( &s_player_config_menu, &s_player_back_action );

	return true;
}


qboolean PlayerConfig_CheckIncerement (int dir, float x, float y, float w, float h)
{
	float min[2], max[2], x1, y1, w1, h1;
	char *sound = NULL;

	x1 = x;	y1 = y;	w1 = w;	h1 = h;
	SCR_AdjustFrom640 (&x1, &y1, &w1, &h1, ALIGN_CENTER);
	min[0] = x1;	max[0] = x1 + w1;
	min[1] = y1;	max[1] = y1 + h1;

	if (cursor.x>=min[0] && cursor.x<=max[0] &&
		cursor.y>=min[1] && cursor.y<=max[1] &&
		!cursor.buttonused[MOUSEBUTTON1] &&
		cursor.buttonclicks[MOUSEBUTTON1]==1)
	{
		if (dir) // dir==1 is left
		{
			if (s_player_skin_box.curvalue>0)
				s_player_skin_box.curvalue--;
		}
		else
		{
			if (s_player_skin_box.curvalue<s_pmi[s_player_model_box.curvalue].nskins)
				s_player_skin_box.curvalue++;
		}

		sound = menu_move_sound;
		cursor.buttonused[MOUSEBUTTON1] = true;
		cursor.buttonclicks[MOUSEBUTTON1] = 0;

		if ( sound )
			S_StartLocalSound( sound );
		SkinCallback (NULL);

		return true;
	}
	return false;
}


void PlayerConfig_MouseClick (void)
{
	//int ystart, xoffset, widest, totalheight, oldhover;
	float	icon_x = SCREEN_WIDTH/2 - 5, //viddef.width  - SCR_ScaledVideo(325),
			icon_y = SCREEN_HEIGHT - 108, //viddef.height - SCR_ScaledVideo(108),
			icon_offset = 0;//, icon_scale;
	int		i, count;//, w, h;
	char	*sound = NULL;
	buttonmenuobject_t buttons[NUM_SKINBOX_ITEMS];

	for (i=0; i<NUM_SKINBOX_ITEMS; i++)
		buttons[i].index =- 1;

	if (s_pmi[s_player_model_box.curvalue].nskins<7 || s_player_skin_box.curvalue<4)
		i=0;
	else if (s_player_skin_box.curvalue > s_pmi[s_player_model_box.curvalue].nskins-4)
		i=s_pmi[s_player_model_box.curvalue].nskins-7;
	else
		i=s_player_skin_box.curvalue-3;

	if (i>0)
		if (PlayerConfig_CheckIncerement(true, icon_x-39, icon_y, 32, 32))
			return;

	for (count=0; count<NUM_SKINBOX_ITEMS; i++,count++)
	{
		if (i<0 || i>=s_pmi[s_player_model_box.curvalue].nskins)
			continue;

		UI_AddButton (&buttons[count], i, icon_x+icon_offset, icon_y, 32, 32);
		icon_offset += 34;

	/*	icon_scale = (i==s_player_skin_box.curvalue)? 2: 1;

		R_DrawGetPicSize ( &w, &h, va("/players/%s/%s_i.pcx", 
			s_pmi[s_player_model_box.curvalue].directory,
			s_pmi[s_player_model_box.curvalue].skindisplaynames[i]));
		icon_scale = icon_scale * (32.0/(float)w); // fix for smaller icons
		UI_AddButton (&buttons[count], i, 
			icon_x + icon_offset, icon_y,
			SCR_ScaledVideo(w)*icon_scale, 
			SCR_ScaledVideo(h)*icon_scale);
		icon_offset += (SCR_ScaledVideo(w)*icon_scale+SCR_ScaledVideo(2));*/
	}

	icon_offset = NUM_SKINBOX_ITEMS*34;
	if (s_pmi[s_player_model_box.curvalue].nskins-i>0)
		if (PlayerConfig_CheckIncerement(false, icon_x+icon_offset+5, icon_y, 32, 32))
			return;

	for (i=0; i<NUM_SKINBOX_ITEMS; i++)
	{
		if (buttons[i].index == -1)
			continue;

		if (cursor.x>=buttons[i].min[0] && cursor.x<=buttons[i].max[0] &&
			cursor.y>=buttons[i].min[1] && cursor.y<=buttons[i].max[1])
		{
			if (!cursor.buttonused[MOUSEBUTTON1] && cursor.buttonclicks[MOUSEBUTTON1]==1)
			{
				s_player_skin_box.curvalue = buttons[i].index;

				sound = menu_move_sound;
				cursor.buttonused[MOUSEBUTTON1] = true;
				cursor.buttonclicks[MOUSEBUTTON1] = 0;

				if ( sound )
					S_StartLocalSound( sound );
				SkinCallback (NULL);

				return;
			}
			break;
		}
	}
}


void PlayerConfig_MenuDraw( void )
{
	extern float CalcFov (float fov_x, float w, float h);
	refdef_t	refdef;
	char		scratch[MAX_QPATH];
	float		rx, ry, rw, rh;

	Menu_Banner ("m_banner_plauer_setup"); // typo for image name is id's fault

	memset(&refdef, 0, sizeof(refdef));

	rx = 0;							ry = 0;
	rw = SCREEN_WIDTH;				rh = SCREEN_HEIGHT;
	SCR_AdjustFrom640 (&rx, &ry, &rw, &rh, ALIGN_CENTER);
	refdef.x = rx;		refdef.y = ry;
	refdef.width = rw;	refdef.height = rh;
	refdef.fov_x = 50;
	refdef.fov_y = CalcFov (refdef.fov_x, refdef.width, refdef.height);
	refdef.time = cls.realtime*0.001;
 
	if ( s_pmi[s_player_model_box.curvalue].skindisplaynames )
	{
		int			yaw;
		int			maxframe = 29;
		vec3_t		modelOrg;
		// Psychopspaz's support for showing weapon model
		entity_t	entity[2], *ent;

		refdef.num_entities = 0;
		refdef.entities = entity;

		yaw = anglemod(cl.time/10);

		VectorSet (modelOrg, 150, (hand->value==1)?25:-25, 0); // was 80, 0, 0

		// Setup player model
		ent = &entity[0];
		memset (&entity[0], 0, sizeof(entity[0]));

		// moved registration code to init and change only
		//Com_sprintf (scratch, sizeof(scratch), "players/%s/tris.md2", s_pmi[s_player_model_box.curvalue].directory);
		//ent->model = R_RegisterModel (scratch);
		ent->model = playermodel;
		//Com_sprintf (scratch, sizeof(scratch), "players/%s/%s.pcx", s_pmi[s_player_model_box.curvalue].directory, s_pmi[s_player_model_box.curvalue].skindisplaynames[s_player_skin_box.curvalue]);
		//ent->skin = R_RegisterSkin (scratch);
		ent->skin = playerskin;

		ent->flags = RF_FULLBRIGHT|RF_NOSHADOW|RF_DEPTHHACK;
		if (hand->value == 1)
			ent->flags |= RF_MIRRORMODEL;

		ent->origin[0] = modelOrg[0];
		ent->origin[1] = modelOrg[1];
		ent->origin[2] = modelOrg[2];

		VectorCopy( ent->origin, ent->oldorigin );
		ent->frame = 0;
		ent->oldframe = 0;
		ent->backlerp = 0.0;
		ent->angles[1] = yaw;//++;
		//if ( ++yaw > 360 )
		//	yaw -= 360;
		
		if (hand->value == 1)
			ent->angles[1] = 360 - ent->angles[1];

		refdef.num_entities++;


		// Setup weapon model
		ent = &entity[1];
		memset (&entity[1], 0, sizeof(entity[1]));

		// Knightmare- moved registration code to init and change only
		//Com_sprintf (scratch, sizeof(scratch), "players/%s/weapon.md2", s_pmi[s_player_model_box.curvalue].directory);
		//ent->model = R_RegisterModel(scratch);
		ent->model = weaponmodel;

		//if (!ent->model)
		//	refdef.num_entities = 1;
		if (ent->model)
		{
			ent->skinnum = 0;

			ent->flags = RF_FULLBRIGHT|RF_NOSHADOW|RF_DEPTHHACK;
			if (hand->value == 1)
				ent->flags |= RF_MIRRORMODEL;

			ent->origin[0] = modelOrg[0];
			ent->origin[1] = modelOrg[1];
			ent->origin[2] = modelOrg[2];

			VectorCopy( ent->origin, ent->oldorigin );
			ent->frame = 0;
			ent->oldframe = 0;
			ent->backlerp = 0.0;
			ent->angles[1] = yaw;
			
			if (hand->value == 1)
				ent->angles[1] = 360 - ent->angles[1];

			refdef.num_entities++;
		}


		refdef.areabits = 0;
		refdef.lightstyles = 0;
		refdef.rdflags = RDF_NOWORLDMODEL;

		Menu_Draw( &s_player_config_menu );

		// skin selection preview
		{
			float	icon_x = SCREEN_WIDTH*0.5 - 5, //viddef.width  - SCR_ScaledVideo(325),
					icon_y = SCREEN_HEIGHT - 108, //viddef.height - SCR_ScaledVideo(75),
					icon_offset = 0;//, icon_scale;
			int		i, count;//, w, h;
			int		color[3];

			TextColor((int)Cvar_VariableValue("alt_text_color"), &color[0], &color[1], &color[2]);

			if (s_pmi[s_player_model_box.curvalue].nskins<7 || s_player_skin_box.curvalue<4)
				i=0;
			else if (s_player_skin_box.curvalue>s_pmi[s_player_model_box.curvalue].nskins-4)
				i=s_pmi[s_player_model_box.curvalue].nskins-7;
			else
				i=s_player_skin_box.curvalue-3;

			/*if (i>0)
				R_DrawChar( SCR_ScaledVideo(icon_x-37), SCR_ScaledVideo(icon_y+2),
					'<' , 3*SCR_VideoScale(), color[0],color[1],color[2],255,false, true);
				//R_DrawChar( icon_x - SCR_ScaledVideo(37), icon_y + SCR_ScaledVideo(2),
				//	'<' , 3*SCR_VideoScale(), color[0],color[1],color[2],255,false, true);
			*/

			// left arrow
			if (i>0)
				Com_sprintf (scratch, sizeof(scratch), "/gfx/ui/arrows/arrow_left.pcx");
			else
				Com_sprintf (scratch, sizeof(scratch), "/gfx/ui/arrows/arrow_left_d.pcx");
			SCR_DrawPic (icon_x-39, icon_y+2, 32, 32,  ALIGN_CENTER, scratch, 1.0);

			// background
			SCR_DrawFill2 (icon_x-3, icon_y-3, NUM_SKINBOX_ITEMS*34+4, 38, ALIGN_CENTER, 0,0,0,255);
			SCR_DrawFill2 (icon_x-2, icon_y-2, NUM_SKINBOX_ITEMS*34+2, 36, ALIGN_CENTER, 60,60,60,255);
				
			for (count=0; count<NUM_SKINBOX_ITEMS; i++,count++)
			{
				if (i<0 || i>=s_pmi[s_player_model_box.curvalue].nskins)
					continue;

			//	icon_scale = (i==s_player_skin_box.curvalue)? 2: 1;

				Com_sprintf (scratch, sizeof(scratch), "/players/%s/%s_i.pcx", 
					s_pmi[s_player_model_box.curvalue].directory,
					s_pmi[s_player_model_box.curvalue].skindisplaynames[i] );

				if (i==s_player_skin_box.curvalue)
					SCR_DrawFill2 (icon_x + icon_offset-1, icon_y-1, 34, 34, ALIGN_CENTER, color[0],color[1],color[2],255);
				SCR_DrawPic (icon_x + icon_offset, icon_y, 32, 32,  ALIGN_CENTER, scratch, 1.0);
				icon_offset += 34;

			//	R_DrawGetPicSize ( &w, &h, scratch);
			//	icon_scale = icon_scale * (32.0/(float)w); // fix for smaller icons
			//	R_DrawStretchPic ( icon_x + icon_offset, icon_y,
			//		SCR_ScaledVideo(w)*icon_scale, SCR_ScaledVideo(h)*icon_scale, scratch, 1.0);
			//	icon_offset += SCR_ScaledVideo(w)*icon_scale;
			}

			/*if (s_pmi[s_player_model_box.curvalue].nskins-i>0)
				R_DrawChar( SCR_ScaledVideo(icon_x+icon_offset+5), SCR_ScaledVideo(icon_y+2),
					'>' , 3*SCR_VideoScale(), color[0],color[1],color[2],255,false, true);
				//R_DrawChar( icon_x + icon_offset + SCR_ScaledVideo(5), icon_y + SCR_ScaledVideo(2),
				//	'>' , 3*SCR_VideoScale(), color[0],color[1],color[2],255,false, true);
			*/

			// right arrow
			icon_offset = NUM_SKINBOX_ITEMS*34;
			if (s_pmi[s_player_model_box.curvalue].nskins-i>0)
				Com_sprintf (scratch, sizeof(scratch), "/gfx/ui/arrows/arrow_right.pcx");
			else
				Com_sprintf (scratch, sizeof(scratch), "/gfx/ui/arrows/arrow_right_d.pcx"); 
			SCR_DrawPic (icon_x+icon_offset+5, icon_y+2, 32, 32,  ALIGN_CENTER, scratch, 1.0);
		}

		R_RenderFrame( &refdef );
	}
}

void PConfigAccept (void)
{
	int i;
	char scratch[1024];

	Cvar_Set( "name", s_player_name_field.buffer );

	Com_sprintf( scratch, sizeof( scratch ), "%s/%s", 
		s_pmi[s_player_model_box.curvalue].directory, 
		s_pmi[s_player_model_box.curvalue].skindisplaynames[s_player_skin_box.curvalue] );

	Cvar_Set( "skin", scratch );

	for ( i = 0; i < s_numplayermodels; i++ )
	{
		int j;

		for ( j = 0; j < s_pmi[i].nskins; j++ )
		{
			if ( s_pmi[i].skindisplaynames[j] )
				free( s_pmi[i].skindisplaynames[j] );
			s_pmi[i].skindisplaynames[j] = 0;
		}
		free( s_pmi[i].skindisplaynames );
		s_pmi[i].skindisplaynames = 0;
		s_pmi[i].nskins = 0;
	}
}

const char *PlayerConfig_MenuKey (int key)
{
	if ( key == K_ESCAPE )
		PConfigAccept();

	return Default_MenuKey( &s_player_config_menu, key );
}


void M_Menu_PlayerConfig_f (void)
{
	if (!PlayerConfig_MenuInit())
	{
		Menu_SetStatusBar( &s_multiplayer_menu, "No valid player models found" );
		return;
	}
	Menu_SetStatusBar( &s_multiplayer_menu, NULL );
	UI_PushMenu( PlayerConfig_MenuDraw, PlayerConfig_MenuKey );
}
