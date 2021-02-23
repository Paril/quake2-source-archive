#include "g_local.h"
#include "m_player.h"

void ClientUserinfoChanged (edict_t *ent, char *userinfo);

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *ent)
{
	// temp
	if (st.arena == 0) st.arena = 1;
	if (st.arena == 0) {
		G_FreeEdict(ent);
		return;
	}
	level.arenas[st.arena-1].player_start = ent;
	gi.dprintf("arena %d: player start at %f %f %f\n", st.arena-1, ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
	G_FreeEdict (self);
	return;
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}

/*
===========
SelectArena

Chooses a player start
============
*/

#define TAG_MAP 769

T_arena_t*	SelectArena (void)
{
	int count, i, j;
	T_arena_t *arena;

	arena = level.arenas;

	// count arenas
	for (count = 0; arena->player_start && count < MAX_ARENAS; count++, arena++);
	j = (int) rand(); // strrrrange
	j = (int) rand() % count;
	//gi.dprintf("picked arena # %d out of %d \n", j, count);
	// find an arena
	arena = &level.arenas[j];
	i = count;
	while ( !arena->player_start || arena->player ) {
		j = (j + 1) % count;
		arena = &level.arenas[j];
		if (!--i) break;
	}
	
	if (i == 0) return NULL;

	// clear out arena
	for ( i = 0; i < arena->width * arena->height; i++) {
		edict_t* block = arena->map[ i ];
		if (!block) continue;
		gi.unlinkentity(block);
		G_FreeEdict(block);
		arena->map[ i ] = NULL;
	}
	
	arena->current_piece = NULL;
	arena->bstime = 1;
	arena->basedwtime = 8;
	arena->wftime = 10;
		
	return arena;
}


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	gclient_t	*client;
	int		i;
	T_arena_t *arena;


	index = ent-g_edicts-1;
	client = ent->client; // this all is very wierd, o id dudes
	arena = client->arena;

	memset (client, 0, sizeof(*client)); // ditto...

	// clear entity values
	ent->client = &game.clients[index];
	ent->inuse = true;
	ent->classname = "player";
	ent->solid = SOLID_BBOX;
	ent->clipmask = MASK_PLAYERSOLID;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->arena = arena;
	arena->player = client; // phooky should be spanked
	client->timer = 0;
	client->state = NO_GAME;
	client->ps.pmove.origin[0] = arena->player_start->s.origin[0]*8;
	client->ps.pmove.origin[1] = arena->player_start->s.origin[1]*8;
	client->ps.pmove.origin[2] = arena->player_start->s.origin[2]*8;

	client->ps.fov = 100;
	client->ps.gunindex = 0;

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 0;		// invisible tetris players
	ent->s.modelindex2 = 0;		// no guns in tetris
	ent->s.frame = 0;
	VectorCopy (arena->player_start->s.origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(arena->player_start->s.angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = arena->player_start->s.angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);

	// set icons for HUD
	// 0 - score icon 1 - help icon 2 - level icon
	// 3 - game over icon
	client->ps.stats[0] = gi.imageindex ( "t_score" );
	client->ps.stats[1] = gi.imageindex ( "t_help" );
	client->ps.stats[2] = gi.imageindex ( "t_level" );
	client->ps.stats[3] = 0;
	client->ps.stats[4] = gi.imageindex ( "t_newgame" );
	// 9 - level
	client->score = 0;
	client->level = 1;
	client->rows = 0;
	client->ps.stats[9] = 1;
	
	// In the happy world of phooky, there are no guns.  There is
	// no violence, war, poverty, or injustice.  Everyone goes outside
	// on sunny days, and fill tremendous hampers with all sorts of
	// delectable treats for their famous three-day picnics.
	// Hey!  Sounds like a good quake mod.
        client->ps.gunindex = 0;
        client->ps.gunframe = 0;

	gi.linkentity (ent);
	showScores(ent);		

}


void spawnPiece(T_arena_t *arena);
/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent, qboolean loadgame)
{
	int		i;

	if (!ent->client->arena) ent->client->arena = SelectArena();
	if (!ent->client->arena) {
		gi.dprintf(" client refused due to lack of arenas, none available ");
		// sorry, charlie.  Not enough arenas to go around, so you get the boot for
		// now.
	}
	if (loadgame && ent->classname && !strcmp(ent->classname, "player") )
	{
		// a loadgame will just use the entity exactly as it is
		// if more clients connect than were saved, they will be
		// spawned as normal
		ent->inuse = true;

		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		G_InitEdict (ent);
		PutClientInServer (ent);
	}

	// send effect if in a multiplayer game
	if (game.maxclients > 1)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s;
	int		playernum;

	// set name
	s = Info_ValueForKey (userinfo, "name");
	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", s, s) );
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo, qboolean loadgame)
{
	int index;
	T_arena_t *arena;

	arena = SelectArena();
	if (!arena) {
		gi.dprintf ( "Client refused-- no arenas available\n" );
		return false;
	}
	index = ent-g_edicts-1;
	ent->client = &game.clients[index];
	ent->client->arena = arena;
	ClientUserinfoChanged (ent, userinfo);

	level.players++;
	return true;
}

/*
===========
ClientDisconnect

called when a player drops from the server

============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

	if (ent->client->arena) 
		ent->client->arena->player = NULL;
	
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	gi.linkentity (ent);

	ent->inuse = false;

	level.players--;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
}


//==============================================================
/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	
	client = ent->client;
	// if only we could recieve commands in this loop.  sigh.
	
	// In tetris, client is ALWAYS frozen (for now)
	client->ps.pmove.pm_type = PM_FREEZE;
}

