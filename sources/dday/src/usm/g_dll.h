/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_dll.h,v $
 *   $Revision: 1.9 $
 *   $Date: 2002/07/23 21:11:37 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

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

#ifdef AMIGA
	#include <stdlib.h>
	#include <ctype.h>
	#include <dll.h>
#else
#ifdef __unix__
	#include <stdlib.h>
	#include <dlfcn.h>
	#include <ctype.h>
#else
	#include <windows.h>
#endif
#endif

typedef struct
{
    int apiversion;
    char creator[32];

    void (*UserDLLDigest)(char MsgDigest[64]);

    void (*UserDLLInit)(void);
    void (*UserDLLShutdown)(void);
    void (*UserDLLStartLevel)(edict_t *ent);
    void (*UserDLLLeaveLevel)(void);
    void (*UserDLLPlayerSpawns)(edict_t *self);
    void (*UserDLLPlayerDies)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
	SMos_t *mos_list;
	char teamid[16];
	char playermodel[32];

} userdll_export_t;

typedef struct
{
      //not sure what to pass in.
    game_locals_t *game;
    level_locals_t *level;
    game_import_t *gi;
    game_export_t *globals;

	edict_t		*g_edicts;
	byte			*is_silenced;
//	float	*scope_setting;
 
	int team_index;
	//for now, we will allow the user to add new commands
    //new items and new monsters.
    void (*InsertCommands)(g_cmds_t *, int, char *);
	void (*(*FindFunction)(char *t));
	gitem_t *(*InsertItem)(gitem_t *it,spawn_t *spawnInfo);
	spawn_t *(*InsertEntity)(spawn_t *t); 
    	int (*RemoveEntity)(char *name);    
	
	/* commented out - future functionality
    void (*InsertMonster)();    
    void (*InsertClient)();     // for bots?
    */
} userdll_import_t;


typedef struct userdll_list_node
{
        struct userdll_list_node *next;
        char libname[128];
        char entryname[256];
        char version[32];
        char MD5Sig[64];

#if defined(__unix__) || defined(AMIGA)
		void *hDLL;
#else
        HMODULE hDLL;
#endif

        userdll_export_t (*EntryPoint)(userdll_import_t);
        userdll_export_t dll_funcs;
} userdll_list_node_t;


//this is where we keep the list of user loaded dlls.

static struct userdll_list_node *GlobalUserDLLList = NULL;
static userdll_import_t  UserDLLImports;


void InsertCmds(g_cmds_t *cmds, int numCmds, char *src);
