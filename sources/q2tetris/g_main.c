#include "g_local.h"

game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
spawn_temp_t	st;

int	sm_meat_index;
int	snd_fry;

edict_t		*g_edicts;

cvar_t	*fraglimit;
cvar_t	*timelimit;
cvar_t	*maxclients;
cvar_t	*maxentities;

// qtet cvars
cvar_t	*highscore;

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
void ServerCommand(void);

//===================================================================


/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
void ShutdownGame (void)
{
	gi.dprintf ("==== ShutdownGame ====\n");

	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);
}



game_export_t *GetGameAPI (game_import_t *import)
{
	srand( time ( NULL ) );
	
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;
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


qboolean spawnPiece(T_arena_t* arena);
qboolean dropPiece1(T_arena_t* arena);
int scanRows(T_arena_t* arena);

/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/

void showScores(edict_t* ent);

void G_RunFrame (void)
{
	int		i, rowcount;
	edict_t	*ent;
	gclient_t *client;
	T_arena_t *arena;

	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	// advance all running tetris games and timers
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		client = ent->client;
		arena = client->arena;
		if (!arena) continue; // need some mechanism to get this boy an arena!
		client->timer++;
		switch(client->state) {
		case WAIT_BLOCKSPAWN:
			if (client->timer > arena->bstime) {
				client->timer = 0;
				client->state = BLOCK_FALLING;
				arena->dwtime = arena->basedwtime;
				if (spawnPiece(arena)) {
					// end of game, player loses
					client->state = GAME_OVER;
					client->ps.stats[3] = gi.imageindex( "t_gameover" );
					client->ps.stats[4] = gi.imageindex( "t_newgame" );
					// do sounds-- this counts as a FIXME
					gi.sound(ent, CHAN_AUTO, gi.soundindex( "misc/udeath.wav" ), 0.8, ATTN_NORM, 0);				
				}
			}
			break;
		case BLOCK_FALLING:
			if (client->timer > arena->dwtime) {
				ent->client->timer = 0;
				if (dropPiece1(arena)) client->state = SCAN_FALLEN;
			}
			break;
		case BLOCK_DROPPING:
			ent->client->timer = 0;
			while (! dropPiece1(arena)) ;
			client->state = SCAN_FALLEN;
			break;
		case SCAN_FALLEN:
			rowcount = scanRows(arena);
			client->state = WAIT_FALLEN;
			arena->wftime = (rowcount + 3);
			showScores(ent);
		case WAIT_FALLEN:
			if (client->timer > arena->wftime) {
				ent->client->timer = 0;
				ent->client->state = WAIT_BLOCKSPAWN;
			}
			break;
		case GAME_OVER:
		case NO_GAME:
			break;
		}		
		// check for level switch
		if (client->rows >= 5 * client->level ) {
			client->rows -= (5 * client->level);
			client->level ++;
			gi.sound(ent, CHAN_AUTO, gi.soundindex( "misc/keyuse.wav" ), 0.8, ATTN_NORM, 0);				
			// FIXME
			client->arena->basedwtime--;
		}
	}



	// build the playerstate_t structures for all players
	ClientEndServerFrames ();
}

void ServerCommand(void) {
}

#ifdef _WIN32
/* DLLMain appended by CrusadeR (dareyes@tamu.edu)
	DLL entry point
*/

int _stdcall DLLMain(void *hinstDLL, unsigned long dwReason, void *reserved)
{
        return(1);
}
#endif
