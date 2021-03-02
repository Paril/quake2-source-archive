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

#include "g_local.h"

#define Function(f) {#f, f}

mmove_t mmove_reloc;

field_t fields[] = {
	{"classname", FOFS(classname), F_LSTRING},
	{"model", FOFS(model), F_LSTRING},
	{"spawnflags", FOFS(spawnflags), F_INT},
	{"speed", FOFS(speed), F_FLOAT},
	{"accel", FOFS(accel), F_FLOAT},
	{"decel", FOFS(decel), F_FLOAT},
	{"target", FOFS(target), F_LSTRING},
	{"targetname", FOFS(targetname), F_LSTRING},
	{"pathtarget", FOFS(pathtarget), F_LSTRING},
	{"deathtarget", FOFS(deathtarget), F_LSTRING},
	{"killtarget", FOFS(killtarget), F_LSTRING},
	{"combattarget", FOFS(combattarget), F_LSTRING},
	{"message", FOFS(message), F_LSTRING},
	{"team", FOFS(team), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"delay", FOFS(delay), F_FLOAT},
	{"random", FOFS(random), F_FLOAT},
	{"move_origin", FOFS(move_origin), F_VECTOR},
	{"move_angles", FOFS(move_angles), F_VECTOR},
	{"style", FOFS(style), F_INT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"sounds", FOFS(sounds), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(dmg), F_INT},
	{"mass", FOFS(mass), F_INT},
	{"volume", FOFS(volume), F_FLOAT},
	{"attenuation", FOFS(attenuation), F_FLOAT},
	{"map", FOFS(map), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},
	// Veh
	{"move_origin", FOFS(move_origin), F_VECTOR},

	{"goalentity", FOFS(goalentity), F_EDICT, FFL_NOSPAWN},
	{"movetarget", FOFS(movetarget), F_EDICT, FFL_NOSPAWN},
	{"enemy", FOFS(enemy), F_EDICT, FFL_NOSPAWN},
	{"oldenemy", FOFS(oldenemy), F_EDICT, FFL_NOSPAWN},
	{"activator", FOFS(activator), F_EDICT, FFL_NOSPAWN},
	{"groundentity", FOFS(groundentity), F_EDICT, FFL_NOSPAWN},
	{"teamchain", FOFS(teamchain), F_EDICT, FFL_NOSPAWN},
	{"teammaster", FOFS(teammaster), F_EDICT, FFL_NOSPAWN},
	{"owner", FOFS(owner), F_EDICT, FFL_NOSPAWN},
	{"mynoise", FOFS(mynoise), F_EDICT, FFL_NOSPAWN},
	{"mynoise2", FOFS(mynoise2), F_EDICT, FFL_NOSPAWN},
	{"target_ent", FOFS(target_ent), F_EDICT, FFL_NOSPAWN},
	{"chain", FOFS(chain), F_EDICT, FFL_NOSPAWN},
	{"infester", FOFS(infester), F_EDICT, FFL_NOSPAWN},


	{"prethink", FOFS(prethink), F_FUNCTION, FFL_NOSPAWN},
	{"think", FOFS(thinks[0]), F_FUNCTION, FFL_NOSPAWN},
	{"blocked", FOFS(blocked), F_FUNCTION, FFL_NOSPAWN},
	{"touch", FOFS(touch), F_FUNCTION, FFL_NOSPAWN},
	{"use", FOFS(use), F_FUNCTION, FFL_NOSPAWN},
	{"pain", FOFS(pain), F_FUNCTION, FFL_NOSPAWN},
	{"die", FOFS(die), F_FUNCTION, FFL_NOSPAWN},

	{"stand", FOFS(monsterinfo.stand), F_FUNCTION, FFL_NOSPAWN},
	{"idle", FOFS(monsterinfo.idle), F_FUNCTION, FFL_NOSPAWN},
	{"search", FOFS(monsterinfo.search), F_FUNCTION, FFL_NOSPAWN},
	{"walk", FOFS(monsterinfo.walk), F_FUNCTION, FFL_NOSPAWN},
	{"run", FOFS(monsterinfo.run), F_FUNCTION, FFL_NOSPAWN},
	{"dodge", FOFS(monsterinfo.dodge), F_FUNCTION, FFL_NOSPAWN},
	{"attack", FOFS(monsterinfo.attack), F_FUNCTION, FFL_NOSPAWN},
	{"melee", FOFS(monsterinfo.melee), F_FUNCTION, FFL_NOSPAWN},
	{"sight", FOFS(monsterinfo.sight), F_FUNCTION, FFL_NOSPAWN},
	{"checkattack", FOFS(monsterinfo.checkattack), F_FUNCTION, FFL_NOSPAWN},
	{"currentmove", FOFS(monsterinfo.currentmove), F_MMOVE, FFL_NOSPAWN},

	{"endfunc", FOFS(moveinfo.endfunc), F_FUNCTION, FFL_NOSPAWN},

	// temp spawn vars -- only valid when the spawn function is called
	{"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
	{"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
	{"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
	{"noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP},
	{"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
	{"item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP},

//need for item field in edict struct, FFL_SPAWNTEMP item will be skipped on saves
	{"item", FOFS(item), F_ITEM},

	{"gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP},
	{"sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP},
	{"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
	{"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
	{"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP},

//	{"weight", STOFS(weight), F_INT, FFL_SPAWNTEMP},//JABot

	{0, 0, 0, 0}

};

field_t		levelfields[] =
{
	{"changemap", LLOFS(changemap), F_LSTRING},
                   
	{"sight_client", LLOFS(sight_client), F_EDICT},
	{"sight_entity", LLOFS(sight_entity), F_EDICT},
	{"sound_entity", LLOFS(sound_entity), F_EDICT},
	{"sound2_entity", LLOFS(sound2_entity), F_EDICT},

	{NULL, 0, F_INT}
};

field_t		clientfields[] =
{
	{"pers.weapon", CLOFS(pers.weapon), F_ITEM},
	{"pers.lastweapon", CLOFS(pers.lastweapon), F_ITEM},
	{"newweapon", CLOFS(newweapon), F_ITEM},

	{NULL, 0, F_INT}
};

/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/
typedef int (*integer)();
typedef void (*thevoid)(void);

#ifdef _WIN32
HMODULE hModule;
typedef void (*ff12)(edict_t *ent, char *cmd, char mapname[MAX_QPATH], float time, int framenum, char *theentities);

#define EDITOR_API_VERSION 2

extern __declspec(dllexport) void debug_print (char *text, ...);
extern __declspec(dllexport) char *GetArgv (int argn);
extern __declspec(dllexport) void RealFreeEdict (edict_t *edicttofree);
extern __declspec(dllexport) char *TagMalloc (int size, int tag);
extern __declspec(dllexport) void TagFree (void *block);
extern __declspec(dllexport) void SetModel (edict_t *e, char *s);
extern __declspec(dllexport) void linkentity (edict_t *e);
extern __declspec(dllexport) int EditorOn ();
extern __declspec(dllexport) edict_t *HandleSpawning (char *classname);
#else
void debug_print (char *text, ...);
char *GetArgv (int argn);
void RealFreeEdict (edict_t *edicttofree);
char *TagMalloc (int size, int tag);
void TagFree (void *block);
void SetModel (edict_t *e, char *s);
void linkentity (edict_t *e);
int EditorOn ();
edict_t *HandleSpawning (char *classname);
#endif
void ED_CallSpawn (edict_t *ent);

// Called by editor to spawn monsters and stuff like that.
edict_t *HandleSpawning (char *classname)
{
	edict_t *temp = (edict_t *)TagMalloc(sizeof(edict_t), TAG_LEVEL);
	
	temp = G_Spawn();

	temp->classname = classname;
	ED_CallSpawn (temp);

	return temp;
}

void debug_print (char *fmt, ...)
{
//	int     i;
	char	bigbuffer[0x10000];
	int		len;
	va_list	argptr;
//	edict_t	*cl_ent;
	
	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	gi.old_dprintf (bigbuffer);

	Log_Print (bigbuffer);
}

char *GetArgv (int argn)
{
	return gi.argv(argn);
}

void RealFreeEdict (edict_t *edicttofree)
{
	G_FreeEdict (edicttofree);
}

char *TagMalloc (int size, int tag)
{
	return gi.TagMalloc (size, tag);
}

void TagFree (void *block)
{
	gi.TagFree (block);
}

void SetModel (edict_t *e, char *s)
{
	R_SetModel(e, s);
}

void linkentity (edict_t *e)
{
	gi.linkentity (e);
}

int EditorOn ()
{
	if (iwm_editor->value)
		return 1;
	else
		return 0;
}
void InitTeams ();

void InitGame (void)
{
	int success = 1;
	integer CheckAPIVersion;
	thevoid SetGlobalImports;

	//iwm_logging = gi.cvar ("iwm_logging", "0", CVAR_ARCHIVE);
	InitLogging();
	InitTeams();

	//InitNodes();

	gi.dprintf ("==== InitGame %s ====\n", GAMEVERSION);

	gun_x = gi.cvar ("gun_x", "0", 0);
	gun_y = gi.cvar ("gun_y", "0", 0);
	gun_z = gi.cvar ("gun_z", "0", 0);

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed = gi.cvar ("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar ("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar ("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar ("sv_gravity", "800", 0);

	// noset vars
	dedicated = gi.cvar ("dedicated", "0", CVAR_NOSET);

	// latched vars
	sv_cheats = gi.cvar ("cheats", "0", CVAR_SERVERINFO|CVAR_LATCH);
	gi.cvar ("gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar ("gamedate", __DATE__ , CVAR_SERVERINFO | CVAR_LATCH);

	maxclients = gi.cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	deathmatch = gi.cvar ("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar ("coop", "0", CVAR_LATCH);
	skill = gi.cvar ("skill", "1", CVAR_LATCH);
	// Paril: Increased to 4096
	maxentities = gi.cvar ("maxentities", "4096", CVAR_LATCH);

	// change anytime vars
	dmflags = gi.cvar ("dmflags", "0", CVAR_SERVERINFO);
	fraglimit = gi.cvar ("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar ("timelimit", "0", CVAR_SERVERINFO);
	password = gi.cvar ("password", "", CVAR_USERINFO);
	filterban = gi.cvar ("filterban", "1", 0);

	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar ("run_pitch", "0.002", 0);
	run_roll = gi.cvar ("run_roll", "0.005", 0);
	bob_up  = gi.cvar ("bob_up", "0.005", 0);
	bob_pitch = gi.cvar ("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar ("bob_roll", "0.002", 0);

	// flood control
	flood_msgs = gi.cvar ("flood_msgs", "4", 0);
	flood_persecond = gi.cvar ("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar ("flood_waitdelay", "10", 0);

	// dm map list
	sv_maplist = gi.cvar ("sv_maplist", "", 0);

	servermessage = gi.cvar ("servermessage", "IWM 1.00", CVAR_ARCHIVE);

	// items
	InitItems ();

	Com_sprintf (game.helpmessage1, sizeof(game.helpmessage1), "");

	Com_sprintf (game.helpmessage2, sizeof(game.helpmessage2), "");

	// initialize all entities for this game
	game.maxentities = maxentities->value;
	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// initialize all clients for this game
	game.maxclients = maxclients->value;
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients+1;
	day = gi.cvar ("day", "0", 0);

	srand((unsigned)time(NULL)); //seed the rng

	iwm_gamemode = gi.cvar ("iwm_gamemode", "0", CVAR_LATCH);

//ZOID
	capturelimit = gi.cvar ("capturelimit", "0", CVAR_SERVERINFO);
	instantweap = gi.cvar ("instantweap", "0", CVAR_SERVERINFO);
//ZOID

//ZOID
	CTFInit();
//ZOID

	flag1_item = FindItem("Red Flag");
	flag2_item = FindItem("Blue Flag");

	feature_ban = gi.cvar ("feature_ban", "0", CVAR_ARCHIVE);
	blaster_ban = gi.cvar ("blaster_ban", "0", CVAR_ARCHIVE);
	shotgun_ban = gi.cvar ("shotgun_ban", "0", CVAR_ARCHIVE);
	ssg_ban = gi.cvar ("ssg_ban", "0", CVAR_ARCHIVE);
	machinegun_ban = gi.cvar ("machinegun_ban", "0", CVAR_ARCHIVE);
	chaingun_ban = gi.cvar ("chaingun_ban", "0", CVAR_ARCHIVE);
	grenade_ban = gi.cvar ("grenade_ban", "0", CVAR_ARCHIVE);
	gl_ban = gi.cvar ("gl_ban", "0", CVAR_ARCHIVE);
	rl_ban = gi.cvar ("rl_ban", "0", CVAR_ARCHIVE);
	hyperblaster_ban = gi.cvar ("hyperblaster_ban", "0", CVAR_ARCHIVE);
	railgun_ban = gi.cvar ("railgun_ban", "0", CVAR_ARCHIVE);
	bfg_ban = gi.cvar ("bfg_ban", "0", CVAR_ARCHIVE);
	item_ban = gi.cvar ("item_ban", "0", CVAR_ARCHIVE);
	//original_ban = gi.cvar ("original_ban", "0", CVAR_ARCHIVE);

	cg_hints = gi.cvar ("cg_hints", "2", CVAR_USERINFO);
	cg_startweap = gi.cvar ("cg_startweap", "", CVAR_USERINFO);
	iwm_editor = gi.cvar ("iwm_editor", "0", CVAR_LATCH);

	display_writes = gi.cvar ("display_writes", "0", CVAR_ARCHIVE);

	//display_writes = gi.cvar ("display_writes", "0", CVAR_ARCHIVE);
//	AI_Init();//JABot

#ifdef _WIN32
	hModule = LoadLibrary ("iwm//editor.dll");
	if (hModule)
	{
		if (hModule != NULL) 
		{ 
			EditorCommands = (ff12)GetProcAddress(hModule, "EditorCommands");
			CheckAPIVersion = (integer)GetProcAddress(hModule, "CheckAPIVersion");
			SetGlobalImports = (thevoid)GetProcAddress(hModule, "SetGlobalImports");

			if (SetGlobalImports)
				SetGlobalImports ();
			
			if (CheckAPIVersion)
			{
				gi.dprintf ("Editor module version: %i\n", CheckAPIVersion());
				if (CheckAPIVersion() < EDITOR_API_VERSION)
				{
					gi.dprintf ("Editor version is %i, should be %i!\n", CheckAPIVersion(), EDITOR_API_VERSION);
					success = 0;
					goto stop;
				}
			}
		} 
	}
	else
	{
		gi.dprintf ("Editor DLL not found!\n");
		goto end;
stop:
		gi.dprintf ("Editor DLL did not load correctly!\n");
	}

	if (success)
	gi.dprintf ("Successfully loaded editor DLL!\n");
#endif
end:
	ParilWrite_Initialize();
	Maplist_InitVars();
	Voting_InitVars();

	//cg_hookcolor_r = gi.cvar ("cg_hookcolor_r", "0", CVAR_USERINFO);
	//cg_hookcolor_g = gi.cvar ("cg_hookcolor_g", "150", CVAR_USERINFO);
	//cg_hookcolor_b = gi.cvar ("cg_hookcolor_b", "0", CVAR_USERINFO);

	//cg_flashlight_r = gi.cvar ("cg_flashlight_r", "1", CVAR_USERINFO);
	//cg_flashlight_g = gi.cvar ("cg_flashlight_g", "1", CVAR_USERINFO);
	//cg_flashlight_b = gi.cvar ("cg_flashlight_b", "1", CVAR_USERINFO);
}

//=========================================================

void WriteField1(FILE *f, field_t *field, byte *base) 
{
	
	void *p;
	
	if (field->flags & FFL_SPAWNTEMP)
		return;
	
	p = (void *)(base+field->ofs);
	
	switch (field->type) 
	{
	case F_LSTRING:
	case F_GSTRING:
		*(int *)p=(*(char **)p)?strlen(*(char **)p)+1:0;
		break;
		
	case F_EDICT:
		*(int *)p=(*(edict_t **)p==NULL)?-1:*(edict_t **)p-g_edicts;
		break;
		
	case F_CLIENT:
		*(int *)p=(*(gclient_t **)p==NULL)?-1:*(gclient_t **)p-game.clients;
		break;
		
	case F_ITEM:
		*(int *)p=(*(edict_t **)p==NULL)?-1:*(gitem_t **)p-itemlist;
		break;
		
	case F_FUNCTION:
		*(int *)p=(*(byte **)p==NULL)?0:*(byte **)p-((byte *)(void *)InitGame);
		break;
		
	case F_MMOVE:
		*(int *)p=(*(byte **)p == NULL)?0:*(byte **)p-(byte *)&mmove_reloc;
		break;
		
	default: break;
	} // end switch
}


void WriteField2(FILE *f, field_t *field, byte *base) 
{
	
	void *p;
	
	if (field->flags & FFL_SPAWNTEMP)
		return;
	
	p = (void *)(base+field->ofs);
	
	if (field->type==F_LSTRING && (*(char **)p))
		fwrite(*(char **)p, (int)(strlen(*(char **)p)+1), 1, f);
}

void ReadField(FILE *f, field_t *field, byte *base) 
{
	void *p;
	int len, index;
	
	if (field->flags & FFL_SPAWNTEMP)
		return;
	
	p = (void *)(base+field->ofs);
	
	switch (field->type) 
	{
		
	case F_LSTRING:
		len=*(int *)p;
		*(char **)p=(!len)?NULL:gi.TagMalloc(len, TAG_LEVEL);
		if (len)
			fread(*(char **)p, len, 1, f);
		break;
		
	case F_EDICT:
		index=*(int *)p;
		*(edict_t **)p=(index == -1)?NULL:&g_edicts[index];
		break;
		
	case F_CLIENT:
		index=*(int *)p;
		*(gclient_t **)p=(index == -1)?NULL:&game.clients[index];
		break;
		
	case F_ITEM:
		index=*(int *)p;
		*(gitem_t **)p=(index == -1)?NULL:&itemlist[index];
		break;
		
	case F_FUNCTION:
		index=*(int *)p;
		*(byte **)p=(index==0)?NULL:((byte *)(void *)InitGame)+index;
		break;
		
	case F_MMOVE:
		index=*(int *)p;
		*(byte **)p=(index==0)?NULL:(byte *)&mmove_reloc+index;
		break;
		
	default: break;
	} // end switch
}

/*void WriteField1 (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	if (field->flags & FFL_SPAWNTEMP)
		return;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
	case F_GSTRING:
		if ( *(char **)p )
			len = strlen(*(char **)p) + 1;
		else
			len = 0;
		*(int *)p = len;
		break;
	case F_EDICT:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(edict_t **)p - g_edicts;
		*(int *)p = index;
		break;
	case F_CLIENT:
		if ( *(gclient_t **)p == NULL)
			index = -1;
		else
			index = *(gclient_t **)p - game.clients;
		*(int *)p = index;
		break;
	case F_ITEM:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(gitem_t **)p - itemlist;
		*(int *)p = index;
		break;

	//relative to code segment
	case F_FUNCTION:
		if (*(byte **)p == NULL)
			index = 0;
		else
			index = *(byte **)p - ((byte *)InitGame);
		*(int *)p = index;
		break;

	//relative to data segment
	case F_MMOVE:
		if (*(byte **)p == NULL)
			index = 0;
		else
			index = *(byte **)p - (byte *)&mmove_reloc;
		*(int *)p = index;
		break;

	default:
		gi.error ("WriteEdict: unknown field type");
	}
}


void WriteField2 (FILE *f, field_t *field, byte *base)
{
	int			len;
	void		*p;

	if (field->flags & FFL_SPAWNTEMP)
		return;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_LSTRING:
		if ( *(char **)p )
		{
			len = strlen(*(char **)p) + 1;
			fwrite (*(char **)p, len, 1, f);
		}
		break;
	}
}

void ReadField (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	if (field->flags & FFL_SPAWNTEMP)
		return;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
		len = *(int *)p;
		if (!len)
			*(char **)p = NULL;
		else
		{
			*(char **)p = gi.TagMalloc (len, TAG_LEVEL);
			fread (*(char **)p, len, 1, f);
		}
		break;
	case F_EDICT:
		index = *(int *)p;
		if ( index == -1 )
			*(edict_t **)p = NULL;
		else
			*(edict_t **)p = &g_edicts[index];
		break;
	case F_CLIENT:
		index = *(int *)p;
		if ( index == -1 )
			*(gclient_t **)p = NULL;
		else
			*(gclient_t **)p = &game.clients[index];
		break;
	case F_ITEM:
		index = *(int *)p;
		if ( index == -1 )
			*(gitem_t **)p = NULL;
		else
			*(gitem_t **)p = &itemlist[index];
		break;

	//relative to code segment
	case F_FUNCTION:
		index = *(int *)p;
		if ( index == 0 )
			*(byte **)p = NULL;
		else
			*(byte **)p = ((byte *)InitGame) + index;
		break;

	//relative to data segment
	case F_MMOVE:
		index = *(int *)p;
		if (index == 0)
			*(byte **)p = NULL;
		else
			*(byte **)p = (byte *)&mmove_reloc + index;
		break;

	default:
		gi.error ("ReadEdict: unknown field type");
	}
}*/

//=========================================================

/*
==============
WriteClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteClient (FILE *f, gclient_t *client)
{
	field_t		*field;
	gclient_t	temp;
	
	// all of the ints, floats, and vectors stay as they are
	temp = *client;

	// change the pointers to lengths or indexes
	for (field=clientfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=clientfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)client);
	}
}

/*
==============
ReadClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadClient (FILE *f, gclient_t *client)
{
	field_t		*field;

	fread (client, sizeof(*client), 1, f);

	for (field=clientfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)client);
	}
}

/*
============
WriteGame

This will be called whenever the game goes to a new level,
and when the user explicitly saves the game.

Game information include cross level data, like multi level
triggers, help computer info, and all client states.

A single player death will automatically restore from the
last save position.
============
*/
void WriteGame (char *filename, qboolean autosave)
{
	FILE	*f;
	int		i;
	char	str[16];

	if (!autosave)
		SaveClientData ();

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	memset (str, 0, sizeof(str));
	strcpy (str, __DATE__);
	fwrite (str, sizeof(str), 1, f);

	game.autosaved = autosave;
	fwrite (&game, sizeof(game), 1, f);
	game.autosaved = false;

	for (i=0 ; i<game.maxclients ; i++)
		WriteClient (f, &game.clients[i]);

	fclose (f);
}

void ReadGame (char *filename)
{
	FILE	*f;
	int		i;
	char	str[16];

	gi.FreeTags (TAG_GAME);

	f = fopen (filename, "rb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	fread (str, sizeof(str), 1, f);
	if (strcmp (str, __DATE__))
	{
		fclose (f);
		gi.error ("Savegame from an older version.\n");
	}

	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;

	fread (&game, sizeof(game), 1, f);
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	for (i=0 ; i<game.maxclients ; i++)
		ReadClient (f, &game.clients[i]);

	fclose (f);
}

//==========================================================


/*
==============
WriteEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteEdict (FILE *f, edict_t *ent)
{
	field_t		*field;
	edict_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = *ent;

	// change the pointers to lengths or indexes
	for (field=fields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=fields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)ent);
	}

}

/*
==============
WriteLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteLevelLocals (FILE *f)
{
	field_t		*field;
	level_locals_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = level;

	// change the pointers to lengths or indexes
	for (field=levelfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=levelfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)&level);
	}
}


/*
==============
ReadEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadEdict (FILE *f, edict_t *ent)
{
	field_t		*field;

	fread (ent, sizeof(*ent), 1, f);

	for (field=fields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)ent);
	}
}

/*
==============
ReadLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadLevelLocals (FILE *f)
{
	field_t		*field;

	fread (&level, sizeof(level), 1, f);

	for (field=levelfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)&level);
	}
}

/*
=================
WriteLevel

=================
*/
void WriteLevel (char *filename)
{
	int		i;
	edict_t	*ent;
	FILE	*f;
	void	*base;

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	// write out edict size for checking
	i = sizeof(edict_t);
	fwrite (&i, sizeof(i), 1, f);

	// write out a function pointer for checking
	base = (void *)InitGame;
	fwrite (&base, sizeof(base), 1, f);

	// write out level_locals_t
	WriteLevelLocals (f);

	// write out all the entities
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse)
			continue;
		fwrite (&i, sizeof(i), 1, f);
		WriteEdict (f, ent);
	}
	i = -1;
	fwrite (&i, sizeof(i), 1, f);

	fclose (f);
}


/*
=================
ReadLevel

SpawnEntities will allready have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines
set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================
*/
void ReadLevel (char *filename)
{
	int		entnum;
	FILE	*f;
	int		i;
	void	*base;
	edict_t	*ent;

	f = fopen (filename, "rb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	// free any dynamic memory allocated by loading the level
	// base state
	gi.FreeTags (TAG_LEVEL);

	// wipe all the entities
	memset (g_edicts, 0, game.maxentities*sizeof(g_edicts[0]));
	globals.num_edicts = maxclients->value+1;

	// check edict size
	fread (&i, sizeof(i), 1, f);
	if (i != sizeof(edict_t))
	{
		fclose (f);
		gi.error ("ReadLevel: mismatched edict size");
	}

	// check function pointer base address
	fread (&base, sizeof(base), 1, f);
#ifdef _WIN32
	if (base != (void *)InitGame)
	{
		fclose (f);
		gi.error ("ReadLevel: function pointers have moved");
	}
#else
	gi.dprintf("Function offsets %d\n", ((byte *)base) - ((byte *)InitGame));
#endif

	// load the level locals
	ReadLevelLocals (f);

	// load all the entities
	for (;;)
	{
		if (fread (&entnum, sizeof(entnum), 1, f) != 1)
		{
			fclose (f);
			gi.error ("ReadLevel: failed to read entnum");
		}
		if (entnum == -1)
			break;
		if (entnum >= globals.num_edicts)
			globals.num_edicts = entnum+1;

		ent = &g_edicts[entnum];
		ReadEdict (f, ent);

		// let the server rebuild world links for this ent
		memset (&ent->area, 0, sizeof(ent->area));
		gi.linkentity (ent);
	}

	fclose (f);

	// mark all clients as unconnected
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = &g_edicts[i+1];
		ent->client = game.clients + i;
		ent->client->pers.connected = false;
	}

	// do any load time things at this point
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
			continue;

		// fire any cross-level triggers
		if (ent->classname)
			if (strcmp(ent->classname, "target_crosslevel_target") == 0)
				ent->nextthinks[0] = level.time + ent->delay;
	}
}
