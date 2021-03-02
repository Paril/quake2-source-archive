/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/gbr/gbr_main.c,v $
 *   $Revision: 1.9 $
 *   $Date: 2002/07/23 19:12:49 $
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

#include "gbr_main.h"
#include "gbr_classes.h"

#ifdef AMIGA
#include "dll.h"
#include <proto/exec.h>
#endif

/* Place the name of your dll here */

#define DLL_NAME    "GBR"



/*
  First, we need to set up a number of variables that will be needed while
  running. This would be where we set up the references to the Quake2 things
  like the gi structure, the game structure, etc.

  for our example, we need the InsertCommand function and the gi for the 
  commands to work.
*/


extern SMos_t GBR_MOS_List[];
static int AlreadyInit = 0;
static int AlreadyLoad = 0;



/*
  Custom User code goes here.
*/

void InitFunctions(void)
{
	       //////////////////////////////////////////////////////////////////
           // This is where you would acquire any needed function pointers //

    fire_bullet = (void (*) (edict_t *, vec3_t, vec3_t, int, int, int, int, int, qboolean))
                   PlayerFindFunction("fire_bullet");
    ifchangewep = (void (*)(edict_t *))PlayerFindFunction("ifchangewep");
	
    Weapon_Generic = (void (*)(edict_t *, int, int, int, int,int,int,int,int,int,int*, int*, void (*fire)(edict_t *ent)))
                   PlayerFindFunction("Weapon_Generic");
    FindItem = (gitem_t * (*)(char *))
                   PlayerFindFunction("FindItem");
    SpawnItem = (void (*)(edict_t *, gitem_t *))PlayerFindFunction("SpawnItem");
    FindItemByClassname = (gitem_t *(*)(char *))PlayerFindFunction("FindItemByClassname");
	Use_Weapon=(void(*)(edict_t *, gitem_t *))PlayerFindFunction("Use_Weapon");
	AngleVectors=(void(*)(vec3_t , vec3_t, vec3_t, vec3_t))PlayerFindFunction("AngleVectors");
	P_ProjectSource=(void(*)(gclient_t *, vec3_t, vec3_t, vec3_t, vec3_t, vec3_t))
					PlayerFindFunction("P_ProjectSource");
	PlayerNoise=(void(*)(edict_t *, vec3_t, int))PlayerFindFunction("PlayerNoise");
	Cmd_Reload_f=(qboolean(*)(edict_t *))PlayerFindFunction("Cmd_Reload_f");
	Pickup_Weapon=(qboolean(*)(edict_t *, edict_t *))PlayerFindFunction("Pickup_Weapon");
	Drop_Weapon=(void (*)(edict_t *, gitem_t *))PlayerFindFunction("Drop_Weapon");
	fire_rifle=(void (*)(edict_t *, vec3_t, vec3_t, int, int, int))PlayerFindFunction("fire_rifle");
	VectorScale=(void(*)(vec3_t, vec_t, vec3_t))PlayerFindFunction("VectorScale");
	fire_rocket=(void (*)(edict_t *, vec3_t, vec3_t, int, int, float, int))
					PlayerFindFunction("fire_rocket");
	PBM_FireFlameThrower=(void(*)(edict_t *, vec3_t, vec3_t, vec3_t, int, vec3_t, vec3_t, int , int))
					PlayerFindFunction("PBM_FireFlameThrower");
	Pickup_Ammo=(qboolean(*)(edict_t *, edict_t *))PlayerFindFunction("Pickup_Ammo");
	Drop_Ammo=(void(*)(edict_t *, gitem_t *))PlayerFindFunction("Drop_Ammo");
	
	Weapon_Pistol_Fire=(void(*)(edict_t *))PlayerFindFunction("Weapon_Pistol_Fire");
	Weapon_Rifle_Fire=(void(*)(edict_t *))PlayerFindFunction("Weapon_Rifle_Fire");

	Weapon_Submachinegun_Fire=(void (*)(edict_t *))PlayerFindFunction("Weapon_Submachinegun_Fire");
	Weapon_LMG_Fire=(void (*)(edict_t *))PlayerFindFunction("Weapon_LMG_Fire");
	Weapon_HMG_Fire=(void (*)(edict_t *))PlayerFindFunction("Weapon_HMG_Fire");
	Weapon_Rocket_Fire=(void (*)(edict_t *))PlayerFindFunction("Weapon_Rocket_Fire");
	Weapon_Sniper_Fire=(void (*)(edict_t *))PlayerFindFunction("Weapon_Sniper_Fire");
	Weapon_Grenade=(void (*)(edict_t *))PlayerFindFunction("Weapon_Grenade");

	Play_WepSound=(void (*)(edict_t *, char))PlayerFindFunction("Play_WepSound");
	fire_tracer = (void(*)(edict_t *, vec3_t, vec3_t, int, int))PlayerFindFunction("fire_tracer");
	fire_gun = (void (*)(edict_t *, vec3_t, vec3_t, int, int, int, int, int, qboolean))PlayerFindFunction("fire_gun");
	fire_bullet = (void (*)(edict_t *, vec3_t, vec3_t, int, int, int, int, int, qboolean))PlayerFindFunction("fire_bullet");

	//*** this is all for fire_gun2  >:|   ******  faf
	vectoangles=(void (*)(vec3_t, vec3_t))PlayerFindFunction("vectoangles");
	VectorLength = (vec_t (*)(vec3_t))PlayerFindFunction("VectorLength");
	VectorMA = (void (*)(vec3_t, float, vec3_t, vec3_t))PlayerFindFunction("VectorMA");
//	SprayBlood  not in GlobalGameFunctionArray
	calcVspread = (int (*)(edict_t *, trace_t *))PlayerFindFunction("calcVspread");
	VectorNormalize = (vec_t (*)(vec3_t))PlayerFindFunction("VectorNormalize");
	T_Damage = (void (*)(edict_t *, edict_t *, edict_t *, vec3_t, vec3_t, vec3_t, int, int, int, int))PlayerFindFunction("T_Damage");
	VectorCompare = (int (*)(vec3_t, vec3_t))PlayerFindFunction("VectorCompare");

	//faf: for rockets
	G_FreeEdict = (void (*)(edict_t *))PlayerFindFunction("G_FreeEdict");
	rocket_touch =(void (*)(edict_t *, edict_t *, cplane_t *, csurface_t *))PlayerFindFunction("rocket_touch");
//	G_Spawn = (edict_t (*)(void))PlayerFindFunction("G_Spawn");
	G_Spawn = (edict_t *(*)(void))PlayerFindFunction("G_Spawn");

//void G_FreeEdict(edict_t * ed );
//edict_t * G_Spawn(void )

}	

vec3_t vec3_origin = {0,0,0};
//cvar_t  *deathmatch; 
//cvar_t	*auto_reload;



/* ****************************************************** */
/*  There are now five functions that we need to provide. */
/* ****************************************************** */


// This is a security function and supposed to return an MD5 hash of the code in radix64.

void

UserDLLMD5(char *buf)

{

    buf[0]='\0';  // Do nothing for now

}

/* 
   Initialization Function - called to set up the dll. This is usually
   called to set up mod specific global data.
*/

// pbowens: this is a new function initiated within the DLL for those team specific
//			game elements that arent items yet still need precaching (models)
void UserPrecache(void) 
{

	ptrgi->imageindex("scope_gbr");
	ptrgi->imageindex("victory_gbr");

	ptrgi->soundindex("gbr/victory.wav");

	ptrgi->modelindex("players/gbr/tris.md2");

	//
	// V-WEPS
	//

	// GRM 
	ptrgi->modelindex("players/gbr/w_p38.md2");
	ptrgi->modelindex("players/gbr/w_m98k.md2");
	ptrgi->modelindex("players/gbr/w_mp40.md2");
	ptrgi->modelindex("players/gbr/w_mp43.md2");
	ptrgi->modelindex("players/gbr/w_mg42.md2");
	ptrgi->modelindex("players/gbr/w_panzer.md2");	
	ptrgi->modelindex("players/gbr/w_m98ks.md2");
	ptrgi->modelindex("players/gbr/a_masher.md2");

	// GBR
	ptrgi->modelindex("players/gbr/w_webley.md2");
	ptrgi->modelindex("players/gbr/w_303.md2");
	ptrgi->modelindex("players/gbr/w_sten.md2");
	ptrgi->modelindex("players/gbr/w_bren.md2");
	ptrgi->modelindex("players/gbr/w_vickers.md2");
	ptrgi->modelindex("players/gbr/w_piat.md2");
	ptrgi->modelindex("players/gbr/w_303s.md2");
	ptrgi->modelindex("players/gbr/a_mills_bomb.md2");

	// GENERIC
	ptrgi->modelindex("players/gbr/w_flame.md2");
	ptrgi->modelindex("players/gbr/w_morphine.md2");
	ptrgi->modelindex("players/gbr/w_knife.md2");
	ptrgi->modelindex("players/gbr/w_binoc.md2");
}

void UserDLLInit(void)
{

	if (AlreadyInit) 
		return;
	
	AlreadyInit = 1;
        ptrgi->dprintf(" += UserDLLInit reached <%s>\n",DLL_NAME);
	InitFunctions();
	InitItems();
	UserPrecache();
       
//	auto_reload = ptrgi->cvar ("autoreload","0",0);
//    deathmatch = ptrgi->cvar ("deathmatch", "4", CVAR_SERVERINFO | CVAR_LATCH);

}
/* 
   This is the clean up function - if there were global data structures
    that you had allocated, this is where you get rid of them. 
*/

void UserDLLStop (void) {}

/* 
   Called at the start of each level. The player is in the game. Level
    specific variables can be placed here. 
*/
void UserDLLStartLevel (edict_t *ent) {}

/* 
   Called when the user exits the level. Used to clear out variable so
    that a user ends in a pre-configured state .
*/
void UserDLLEndLevel (void) {}

/* 
   Called when the player respawns in a level.
*/
void UserDLLPlayerRespawns(edict_t *self) {}

/* 
   Called when a player dies. 
*/
void UserDLLPlayerDies(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point) 
{}


/*
   There is a need to initialize the structure that we will pass back -- the same
   way that id does it.
*/
static userdll_export_t userdll_export =
{
    1,						//version of the library
    "British DLL Team.",	//creator - put up to 31 chars of your name here
    UserDLLMD5,				//this is supposed to return an MD5 hash of the code
    UserDLLInit,			//initialization function - adds the command in
    UserDLLStop,			//this is the clean up function
    UserDLLStartLevel,		//supposed to be called at the start of each level
    UserDLLEndLevel,		//called when the user exits the level
    UserDLLPlayerRespawns,	//called when user respawns
    UserDLLPlayerDies,		//called when the user dies
	GBR_MOS_List,			//the class list
	"gbr",					//team id
	"gbr"					//this is the player model to use when spawning
};


/*
   Finally, at long last, we define the entry point that is called by
   the external loader. 
*/

userdll_export_t GBRGetAPI(userdll_import_t udit)
{

	PlayerInsertItem=udit.InsertItem;
    PlayerInsertCommands =  udit.InsertCommands;
    PlayerFindFunction = udit.FindFunction;
    ptrgi = udit.gi;
//	CVscope_setting=udit.scope_setting;
        ptrGlobals = udit.globals;
        ptrlevel = udit.level;
    ptrGame = udit.game;
	is_silenced=udit.is_silenced;
	g_edicts=udit.g_edicts;
	gbr_index=udit.team_index;
        ptrgi->dprintf(" += GetAPI reached <%s>\n",DLL_NAME);
    return userdll_export;
}

#ifdef AMIGA
void* __saveds dllFindResource(int id, char *pType)
{
    return NULL;
}

void* __saveds dllLoadResource(void *pHandle)
{
    return NULL;
}

void __saveds dllFreeResource(void *pHandle)
{
    return;
}

ULONG SegList;

dll_tExportSymbol DLL_ExportSymbols[]=
{
    {dllFindResource,"dllFindResource"},
    {dllLoadResource,"dllLoadResource"},
    {dllFreeResource,"dllFreeResource"},
   {(void *)GBRGetAPI,"GBRGetAPI"},
   {0,0}
};

dll_tImportSymbol DLL_ImportSymbols[]=
{
      {0,0,0,0}
};

int __saveds DLL_Init(void)
{
    return 1L;
}

void __saveds DLL_DeInit(void)
{
}   
#endif
