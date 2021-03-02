
#include "g_local.h"
#include "dll.h"
// SCONFIG START
#include "s_readconfig.h"
#include "readmaps.h"
#include "stdlog.h"
// SCONFIG STOP

#ifdef AMIGA
int exe_found=0;
#endif

// SCONFIG START
extern struct string64_node_t *curLevInCycle;
void HighScoreMessage();
void endLevelshowTop10(edict_t *ent);
// SCONFIG STOP

game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
spawn_temp_t	st;

showed=0;

// HENTAI
int vwep_index;
// END HENTAI
int	sm_meat_index;
int	snd_fry;
int meansOfDeath;

edict_t		*g_edicts;

cvar_t	*deathmatch;
cvar_t	*coop;
cvar_t	*dmflags;
cvar_t	*skill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
// SCONFIG START
cvar_t	*configlocation;
cvar_t  *cyclelocation;
// SCONFIG STOP
cvar_t	*password;
cvar_t	*maxclients;
cvar_t	*maxentities;
cvar_t	*g_select_empty;
cvar_t	*dedicated;

// 3.20 SOURCE
cvar_t	*filterban;

cvar_t	*sv_maxvelocity;
cvar_t	*sv_gravity;

cvar_t	*sv_rollspeed;
cvar_t	*sv_rollangle;
cvar_t	*gun_x;
cvar_t	*gun_y;
cvar_t	*gun_z;

cvar_t	*run_pitch;
cvar_t	*run_roll;
cvar_t	*bob_up;
cvar_t	*bob_pitch;
cvar_t	*bob_roll;

cvar_t	*sv_cheats;

// 3.20 SOURCE
cvar_t	*flood_msgs;
cvar_t	*flood_persecond;
cvar_t	*flood_waitdelay;
// 3.20 SOURCE

// SCONFIG START
cvar_t  *serverconfigmodinfo;
cvar_t	*logfile;
FILE *fragFile;
// Added for new GSLogMOD support
//cvar_t  *stdlogfile;
//cvar_t *stdlogname;
// SCONFIG STOP

void SpawnEntities (char *mapname, char *entities, char *spawnpoint);
void ClientThink (edict_t *ent, usercmd_t *cmd);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void ClientDisconnect (edict_t *ent);
void ClientBegin (edict_t *ent);
void ClientCommand (edict_t *ent);
void RunEntity (edict_t *ent);
void WriteGame (char *filename, qboolean autosave);
void ReadGame (char *filename);
void WriteLevel (char *filename);
void ReadLevel (char *filename);
void InitGame (void);
void G_RunFrame (void);


//===================================================================


void ShutdownGame (void)
{
	gi.dprintf ("==== ShutdownGame ====\n");

	// SCONFIG START
	sl_GameEnd( &gi, level );	// StdLog - Mark Davies
	// SCONFIG STOP

	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);
}


/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
game_export_t *GetGameAPI (game_import_t *import)
{
	// SCONFIG START
	NodePtr new, top, last;
	void getKeys(NodePtr top);
	FILE *myfp;
	char v[120];
	int		i,n,k; 

#ifdef AMIGA
	if (!exe_found) return 0;
#endif

	// Used for rank
	totalrank = 1;

	for (k=0; k<5; k++)
	{
	 GlobalFragLimit[k] = '\0'; // 4 digits ought to be enough
	 GlobalTimeLimit[k] = '\0';
	 GlobalGravity[k] = '\0';
	}
	
	for (n=0; n<40; n++)  // Initialize array
	   directory[n] = '\0'; 

	
	for (n=0; n<50; n++) { // Initialize array
	   configloc[n] = '\0';
	   cycleloc[n] = '\0';
	   ModelGenDir[n]  = '\0';
	   obitsDir[n]  = '\0';
	   PLAYERS_LOGFILE[n] = '\0';
	   HIGHSCORE_DIR[n] = '\0';
	}


	// SCONFIG 2.3
	for (n=0; n<10; n++)
     MAX_CLIENT_RATE_STRING[n] = '\0';

    MAX_CLIENT_RATE=25000; // default value
    clientlog=0;
	// SCONFIG 2.3
	
	
	wasbot=0;
	
	for (n=0; n<30; n++) {
		somevar0[n] = '\0';
		somevar1[n] = '\0';
		somevar2[n] = '\0';
		somevar3[n] = '\0';
		somevar4[n] = '\0';
		somevar5[n] = '\0';
		somevar6[n] = '\0';
		somevar7[n] = '\0';
		somevar8[n] = '\0';
		somevar9[n] = '\0';
		somevar10[n] = '\0';
		somevar11[n] = '\0';
		somevar12[n] = '\0';
		somevar13[n] = '\0';
		somevar14[n] = '\0';
	}

	chasekeepscore=1; // Keep score by default

	// Set in case they aren't read in from config.txt file
	strcpy(GlobalFragLimit, "30\0");
	strcpy(GlobalTimeLimit, "30\0");
	strcpy(GlobalGravity, "800\0");

	cloaking=0;          // Off by default
    CLOAK_DRAIN=2; // Set rate by default if not in config file
	rankhud = 1;
	playershud = 1;
	timehud = 1;
	cloakgrapple = 0; // set to off by default
	hookcolor=4; // gold by default

	fastchange=0; // of by default
//	helphud=1;

	allowgrapple=0;		// disallow it by default.
	HOOK_TIME=4;		// Number of seconds that hook can live
	HOOK_SPEED=1900;	// Velocity of the hook itself
	EXPERT_SKY_SOLID=1; // Change later
	HOOK_DAMAGE=10;		// Damage done by hook
	PULL_SPEED=420;		// How fast the player is pulled 
						// once the hook is anchored
	
	// Configure rocket velocity and damage
	rocketspeed=650; // By default
	ConfigRD = 100; // Default value
	RadiusDamage = 120; // Default value
	DamageRadius = 120; // Default value
	
	// Set logging to off for both styles.
	QWLOG=0;
	strcpy(recordLOG, "0\0");  // no need to initialize since it's

	// Defaults not to record unless stated in config file 
	// differently.

    ingamenamebanningstate=0; // default init value
	// Don't ban anything by default
	ban_shotgun=0;
	ban_supershotgun=0;
	ban_machinegun=0;
	ban_chaingun=0;
	ban_grenadelauncher=0;
	ban_rocketlauncher=0;
	ban_hyperblaster=0;
	ban_railgun=0;
	ban_BFG=0;


	// VW Patch by default is off
	// VWPATCH=1;
    matchfullnamevalue=1; // (meaning look for EXACT match in banlist.txt file)

	scoreboard = 0; // standard scoreboard by default
	GRENADE_TIMER = 3.0;
	GRENADE_MINSPEED = 400;
	GRENADE_MAXSPEED = 800;

	//GrenadeLauncher

	GLauncherTimer=3.0;
	GLauncherFireDistance=600;
	GLauncherDamage=120;
	GLauncherRadius=40;
	// Grenade
	GrenadeTimer = 3.0;
	GrenadeMinSpeed = 400;
	GrenadeMaxSpeed = 800;
	GrenadeDamage = 125;
	GrenadeRadius = 40;
	// Hyperblaster
	HyperBlasterDamage = 15;
	BlasterProjectileSpeed = 1000;
	BlasterDamage = 15;
	// Guns
	MachinegunDamage=8;
	MachinegunKick=2;
	ChaingunDamage=6;
	ChaingunKick=2;
	ShotgunDamage=4;
	ShotgunKick=8;
	SuperShotgunDamage=6;
	SuperShotgunKick=12;
	RailgunDamage=100;
	RailgunKick=200;
	BFGDamage=200;
	BFGDamageRadius=1000;
	BFGProjectileSpeed=400;

	// SCONFIG STOP
	gi = *import;
	globals.apiversion = GAME_API_VERSION;

	// SCONFIG START
	configlocation = gi.cvar ("configlocation", "", CVAR_SERVERINFO | CVAR_NOSET);
	cyclelocation = gi.cvar ("cyclelocation", "", CVAR_SERVERINFO | CVAR_NOSET);
	// SCONFIG STOP
	
	globals.Init = InitGame;

	// SCONFIG START
	sprintf(configloc, "%s", configlocation->string);
	sprintf(cycleloc, "%s", cyclelocation->string);
	gi.dprintf ("Configuration file, [%s]\n", configlocation->string);
	gi.dprintf ("Map cycling file, [%s]\n", cyclelocation->string);	
	
	myfp=fopen(configloc, "r");
			
	if (myfp == NULL) {
		printf("Could not load config.txt file, check your path.\n");
		exit(1);
	  }

	top=NULL;  // Initialize the top node to NULL once.

	while (!feof(myfp)) {
		for (i=0; i<120; i++) {
      	   v[i]='\0';
		}

		fgets(v,120,myfp);

		/* Read values into linked list in order of
	       appearance */
		new = malloc(sizeof(Node));
		strcpy(new -> value,v);
		new -> next = NULL;

		if (top == NULL)
			top = last = new;
		else {
			last -> next = new;
			last = new;
		}

	}

	fclose(myfp);  // Close file properly
	getKeys(top); // Variables all assigned
	// SCONFIG STOP
	
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	// SCONFIG START
	gi.cvar_set("fraglimit", GlobalFragLimit);
	gi.cvar_set("timelimit", GlobalTimeLimit);
	gi.cvar_set("sv_gravity", GlobalGravity);
	gi.cvar_set("logfile", recordLOG);
	// SCONFIG STOP

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	//globals.ServerCommand = ServerCommand;
	globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

	return &globals;
}

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	gi.error (ERR_FATAL, "%s", text);
}

void Com_Printf (char *msg, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	gi.dprintf ("%s", text);
}

#endif

//======================================================================


/*
=================
ClientEndServerFrames
=================
*/
void ClientEndServerFrames (void)
{
	int		i;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		ClientEndServerFrame (ent);
	}

}

/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void EndDMLevel (void)
{
	edict_t		*ent;
	
	// stay on same level flag
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = level.mapname;
	}
	// SCONFIG START
	else 
	{	// go to a specific map
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		
		gi.cvar_set("fraglimit", curLevInCycle->strFraglimit);
		gi.cvar_set("timelimit", curLevInCycle->strTimelimit);
		gi.cvar_set("sv_gravity", curLevInCycle->strGravity);

		ent->map = curLevInCycle->strMap;
		
		// Handle repeat
		if (curLevInCycle->strRepeat == 0) {
			curLevInCycle->strRepeat = curLevInCycle->strMasterRepeat;
			curLevInCycle = curLevInCycle->next;
		} else 
			curLevInCycle->strRepeat--;		  
		// End handle repeat feature
     }
	// SCONFIG STOP
	gi.sound(ent, CHAN_AUTO, gi.soundindex("world/xian1.wav"), 1, ATTN_NONE, 0);
	
	//ent->client->showed=0; // Assure that all clients have NOT seen the end of level
	
	BeginIntermission (ent);
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules (void)
{
	int			i;
	gclient_t	*cl;

	if (level.intermissiontime)
		return;
	
	if (!deathmatch->value)
		return;

	if (timelimit->value)
	{
		if (level.time >= timelimit->value*60)
		{
			fraghit=0;
			gi.bprintf (PRINT_HIGH, "Timelimit hit.\n");
			HighScoreMessage (); // Save off, create new high scores...
			EndDMLevel (); // MoveClientToIntermission (edict_t *ent)
			
			return;
		}
	}

	// No high score for levels with fraglimit set..., everyone would eventually
	// have same score... or at least the person who played the most, and
	// once list is full, the couldn't add their name..
	if (fraglimit->value)
	{
		for (i=0 ; i<maxclients->value ; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;

			if (cl->resp.score >= fraglimit->value)
			{
				fraghit=1;
				gi.bprintf (PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel ();
				return;
			}
		}
	}
}


/*
=============
ExitLevel
=============
*/
void ExitLevel (void)
{
	int		i;
	edict_t	*ent;
	char	command [256];

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();

	// clear some things before going to next level
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;
	}

}

/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
void G_RunFrame (void)
{
	int		i;
	edict_t	*ent;

	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	// choose a client for monsters to target this frame
	AI_SetSightClient ();

	// exit intermissions

	if (level.exitintermission)
	{
		ExitLevel ();
		return;
	}

	//
	// treat each object in turn
	// even the world gets a chance to think
	//
	ent = &g_edicts[0];
	for (i=0 ; i<globals.num_edicts ; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		level.current_entity = ent;

		VectorCopy (ent->s.origin, ent->s.old_origin);

		// if the ground entity moved, make sure we are still on it
		if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount))
		{
			ent->groundentity = NULL;
			if ( !(ent->flags & (FL_SWIM|FL_FLY)) && (ent->svflags & SVF_MONSTER) )
			{
				M_CheckGround (ent);
			}
		}

		if (i > 0 && i <= maxclients->value)
		{
			ClientBeginServerFrame (ent);
			continue;
		}

		G_RunEntity (ent);
	}

	/*if ((level.time > level.intermissiontime + 5.0) && showed == 1) {
		//if (level.time > level.intermissiontime + 5.0) {
			endLevelshowTop10(ent);
			gi.unicast(ent,true);//was true
			showed=2;
		//}
	}

	if ((level.time > level.intermissiontime + 10.0) && showed == 2) {
		showed=0;
		level.exitintermission=true;
	}*/

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();
}

#ifdef AMIGA
void SetExeName(char *name)
{
	exe_found=1;
}
#endif

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

dll_tExportSymbol DLL_ExportSymbols[]=
{
    {dllFindResource,"dllFindResource"},
    {dllLoadResource,"dllLoadResource"},
    {dllFreeResource,"dllFreeResource"},
   {(void *)GetGameAPI,"GetGameAPI"},
   {(void *)SetExeName,"SetExeName"},
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

