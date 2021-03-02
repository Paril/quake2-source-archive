

//Rocket Arena 2 Bot Support Routines
// Copyright 1998, David Wright
// For non-commercial use only

#include "g_local.h"
#include "bl_spawn.h"

extern float PlayersRangeFromSpot (edict_t *spot);
extern gitem_armor_t bodyarmor_info;
extern int weapon_vals[];

int idmap;
int num_arenas;
							//	  0  2  3  4  5   6    9   8   7
int weapon_vals_x[] = { 256, 1, 2, 4, 8, 16, 128, 64, 32 };

// from q_dev15.zip
// Use this function to send a command string to a CLIENT.
// E.g. stuffcmd(player, "alias ready \"cmd ready\"\n");
/*void stuffcmd(edict_t *e, char *s)
{
	gi.WriteByte (11);
	gi.WriteString (s);
	gi.unicast (e, true);
}*/
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int RA2_NumArenas(void)
{
	return num_arenas;
} //end of the function RA2_NumArenas
//===========================================================================
// RA2_GetArenaName - looks for the name from the bsp
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *RA2_GetArenaName(int arena)
{
	edict_t *spot = NULL;
	static char newname[35];

	while ((spot = G_Find(spot, FOFS(classname), "info_player_intermission")) != NULL)
	{
		if (spot->arena == arena) return spot->message;
	} //end while
	sprintf(newname, "Arena Number %d", arena);
	return newname;
} //end of the function RA2_GetArenaName
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_ArenaSound(char *soundname, int context)
{
	int i;
	edict_t *e;

	for (i = 0; i < maxclients->value; i++)
	{
		e = g_edicts + 1 + i;
		if(!e->inuse) continue;
		if (!e->client) continue;
		if (e->client->resp.context != context) continue;
		if (e->flags & FL_BOT) continue;
		//
		gi.WriteByte(11);
		gi.WriteString(va("play %s\n", soundname));
		gi.unicast(e, true);
		//stuffcmd(e, va("play %s\n",soundname));
	} //end for
} //end of the function RA2_ArenaSound
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_ArenaCenterPrint(char *s, int context)
{
	int i;
	edict_t *e;

	for (i = 0; i < maxclients->value; i++)
	{
		e = g_edicts + 1 + i;
		if(e->inuse && e->client && e->client->resp.context == context)
		{
			gi.centerprintf(e, s);
		} //end if
	} //end for
} //end of the function RA2_ArenaCenterPrint
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_ArenaCPrint(int priority, char *s, int context)
{
	int i;
	edict_t *e;

	for (i = 0; i < maxclients->value; i++)
	{
		e = g_edicts + 1 + i;
		if(e->inuse && e->client && e->client->resp.context == context)
		{
			gi.cprintf(e,priority, s);
		} //end if
	} //end for
} //end of the function RA2_ArenaCPrint
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void SetObserverMode(edict_t *ent, observer_mode_t omode)
{
	switch(omode) 
	{
		case NORMAL:
		{
			ent->movetype = MOVETYPE_WALK;
			ent->solid = SOLID_BBOX;
			ent->clipmask = MASK_PLAYERSOLID;
			ent->svflags &= ~SVF_NOCLIENT;
			ent->s.modelindex = 255;
			ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			break;
		} //end case
		case FREEFLYING:
		{
			ent->movetype = MOVETYPE_NOCLIP;
			ent->solid = SOLID_NOT;
			ent->clipmask = 0;
			ent->svflags |= SVF_NOCLIENT;
			ent->client->ps.pmove.pm_time = 0;
			ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			ent->client->ps.pmove.pm_flags &= ~PMF_TIME_TELEPORT;
			break;
		} //end case
	} //end switch
} //end of the function SetObserverMode
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
edict_t *SelectRandomArenaSpawnPoint (char *classn, int arena)
{
	edict_t	*spot;
	int		count = 0;
	int		selection;
	
	spot = NULL;
	while ((spot = G_Find (spot, FOFS(classname), classn)) != NULL)
	{
		if (spot->arena != arena && idmap==false) continue;
		count++;
	}
	
	if (!count)
		return NULL;
	
	selection = rand() % count;
	
	//gi.dprintf("%d spots, %d selected\n",count,selection);
	
	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname),classn);
		if (spot->arena != arena && idmap==false) 
			selection++;
	} while(selection--);
	
	return spot;
} //end of the function SelectRandomArenaSpawnPoint
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
edict_t *SelectFarthestArenaSpawnPoint(char *classn, int arena)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;
	int		count = 0;
	
	
	spot = NULL;
	bestspot = NULL;
	bestdistance = 50;
	while ((spot = G_Find (spot, FOFS(classname), classn)) != NULL)
	{
		//gi.bprintf (PRINT_HIGH,"arena %d spot %d\n", arena, spot->arena);
		if (spot->arena != arena && idmap==false) continue;
		bestplayerdistance = PlayersRangeFromSpot (spot);
		
		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}
	
	if (bestspot)
	{
		return bestspot;
	} //end if
	
	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	return SelectRandomArenaSpawnPoint (classn, arena);
} //end of the function SelectFarthestArenaSpawnPoint
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_MoveToArena(edict_t *ent, int arena, qboolean observer)
{
	vec3_t mins = {-16, -16, -24};
	vec3_t maxs = {16, 16, 32};
	edict_t *dest;
	int i;
	
	if (observer)
	{
		if (!(dest = SelectFarthestArenaSpawnPoint ("misc_teleporter_dest", arena)))
		{
			dest = SelectFarthestArenaSpawnPoint ("info_player_deathmatch", arena);
		} //end if
		if (arena)
		{
			if (ent->client->resp.context == 0)
			{
				ent->client->resp.context = arena;
			} //end if
		} //end if
		else
		{
			//gi.centerprintf(ent, "use \'toarena x\' to enter an arena");
			//else it is the first time
			{	
				//show_arena_menu(ent); 
			}
		} //end else
		ent->client->resp.context = arena;
	} //end if
	else
	{
		//get rid of all menus
		ent->client->resp.context = arena;
		dest = SelectFarthestArenaSpawnPoint ("info_player_deathmatch", arena);
		gi.dprintf("%s entered arena %d\n", ent->client->pers.netname, arena);
	} //end else
	
	if (!dest)
	{
		gi.cprintf(ent, PRINT_HIGH, "arena not found\n");
		return;
	} //end if

	if (observer)
	{
		if (arena)
		{
			gi.dprintf("%s moved to arena %d as observer\n", ent->client->pers.netname, arena);
			gi.centerprintf(ent, "moved to arena %d\n", arena);
		} //end if
		else
		{
			gi.centerprintf(ent, "moved to the observer room");
		} //end else
	} //end if

	gi.unlinkentity(ent);
	//
	ent->health = 100;
	ent->deadflag = DEAD_NO;
	ent->client->latched_buttons = 0;
	//
	if (ent->client->pers.weapon)
	{
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
	} //end if
	//remove the observer flag
#ifdef OBSERVER
	ent->flags &= ~FL_OBSERVER;
#endif //OBSERVER
	ent->solid = SOLID_BBOX;
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	// clear entity state values
	ent->s.effects = 0;
	if (!observer)
	{
		ent->s.modelindex = 255;		// will use the skin specified model
		ent->s.modelindex2 = 255;		// custom gun model
	} //end if
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.frame = 0;
	//
	VectorCopy(dest->s.origin, ent->s.origin);
	VectorCopy(dest->s.origin, ent->s.old_origin);
	ent->s.origin[2] += 10;

	//clear the velocity and hold them in place briefly
	VectorClear (ent->velocity);
	//
	if (!observer)
	{
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		//draw the teleport splash at source and on the player
		ent->s.event = EV_PLAYER_TELEPORT;
	} //end if
	//set angles
	for (i = 0; i < 3; i++)
	{
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - ent->client->resp.cmd_angles[i]);
	} //end for
	
	VectorClear(ent->s.angles);
	VectorClear(ent->client->ps.viewangles);
	VectorClear(ent->client->v_angle);
	// telefrag avoidance at destination
	if (!KillBox(ent))
	{
	} //endif
	if (observer)
	{
		ent->flags |= FL_NOTARGET;
		ent->takedamage = DAMAGE_NO; //always start with no damage
		if (strcmp(dest->classname, "misc_teleporter_dest") != 0
#ifdef BOT
			//bots always go into free flying mode.
			//first off all we don't want them to waiste CPU
			//second the bots can be in an invalid AAS area because
			//of the one invisible walls
			|| (ent->flags & FL_BOT)
#endif //BOT
			)
		{
			SetObserverMode(ent, FREEFLYING);
		} //end if
		else
		{
			SetObserverMode(ent, NORMAL);
		} //end else
	} //end if
	else
	{
		SetObserverMode(ent, NORMAL);
	} //end else
	gi.linkentity(ent);
} //end of the function RA2_MoveToArena
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_Init(edict_t *wsent)
{
	if(!wsent) return;

	//we have to precache all items, after all we can't just use
	//the items without precaching them!!!!!!!!!!
	//now why would I hate to support mods
	PrecacheItem(FindItemByClassname("item_armor_body"));
	PrecacheItem(FindItemByClassname("item_armor_combat"));
	PrecacheItem(FindItemByClassname("item_armor_jacket"));
	PrecacheItem(FindItemByClassname("item_armor_shard"));
	PrecacheItem(FindItemByClassname("item_power_screen"));
	PrecacheItem(FindItemByClassname("item_power_shield"));
	PrecacheItem(FindItemByClassname("weapon_blaster"));
	PrecacheItem(FindItemByClassname("weapon_shotgun"));
	PrecacheItem(FindItemByClassname("weapon_supershotgun"));
	PrecacheItem(FindItemByClassname("weapon_machinegun"));
	PrecacheItem(FindItemByClassname("weapon_chaingun"));
	PrecacheItem(FindItemByClassname("ammo_grenades"));
	PrecacheItem(FindItemByClassname("weapon_grenadelauncher"));
	PrecacheItem(FindItemByClassname("weapon_rocketlauncher"));
	PrecacheItem(FindItemByClassname("weapon_hyperblaster"));
	PrecacheItem(FindItemByClassname("weapon_railgun"));
	PrecacheItem(FindItemByClassname("weapon_bfg"));
	PrecacheItem(FindItemByClassname("ammo_shells"));
	PrecacheItem(FindItemByClassname("ammo_bullets"));
	PrecacheItem(FindItemByClassname("ammo_cells"));
	PrecacheItem(FindItemByClassname("ammo_rockets"));
	PrecacheItem(FindItemByClassname("ammo_slugs"));
	PrecacheItem(FindItemByClassname("item_quad"));
	PrecacheItem(FindItemByClassname("item_invulnerability"));
	PrecacheItem(FindItemByClassname("item_silencer"));
	PrecacheItem(FindItemByClassname("item_breather"));
	PrecacheItem(FindItemByClassname("item_enviro"));
	PrecacheItem(FindItemByClassname("item_ancient_head"));
	PrecacheItem(FindItemByClassname("item_adrenaline"));
	PrecacheItem(FindItemByClassname("item_bandolier"));
	PrecacheItem(FindItemByClassname("item_pack"));
	PrecacheItem(FindItemByClassname("item_health"));
	PrecacheItem(FindItemByClassname("item_health_small"));
	PrecacheItem(FindItemByClassname("item_health_large"));
	PrecacheItem(FindItemByClassname("item_health_mega"));

	num_arenas = wsent->arena; //worldspawn arena flag is # of arenas
	if (!num_arenas)
	{
		num_arenas = 1;
		idmap = true;
	} //end if
	else
	{
		idmap = false;
	} //end else
} //end of the function RA2_Init
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_GetSettings(arena_settings_t *settings)
{
	cvar_t *shotgun;
	cvar_t *supershotgun;
	cvar_t *machinegun;
	cvar_t *chaingun;
	cvar_t *grenadelauncher;
	cvar_t *rocketlauncher;
	cvar_t *hyperblaster;
	cvar_t *railgun;
	cvar_t *bfg;
	cvar_t *armor;
	cvar_t *health;
	cvar_t *shells;
	cvar_t *bullets;
	cvar_t *slugs;
	cvar_t *grenades;
	cvar_t *rockets;
	cvar_t *cells;
	
	shotgun =			gi.cvar("shotgun", "1", 0);
	supershotgun =		gi.cvar("supershotgun", "1", 0);
	machinegun =		gi.cvar("machinegun", "1", 0);
	chaingun =			gi.cvar("chaingun", "1", 0);
	grenadelauncher =	gi.cvar("grenadelauncher", "1", 0);
	rocketlauncher =	gi.cvar("rocketlauncher", "1", 0);
	hyperblaster =		gi.cvar("hyperblaster", "1", 0);
	railgun =			gi.cvar("railgun", "0", 0);
	bfg =					gi.cvar("bfg", "0", 0);
	//
	armor =				gi.cvar("armor", "200", CVAR_SERVERINFO);
	health =				gi.cvar("health", "100", CVAR_SERVERINFO);
	shells =				gi.cvar("shells", "100", CVAR_SERVERINFO);
	bullets =			gi.cvar("bullets", "200" ,CVAR_SERVERINFO);
	slugs =				gi.cvar("slugs", "50", CVAR_SERVERINFO);
	grenades =			gi.cvar("grenades", "50", CVAR_SERVERINFO);
	rockets =			gi.cvar("rockets", "50", CVAR_SERVERINFO);
	cells =				gi.cvar("cells", "150", CVAR_SERVERINFO);
	
	settings->weapons = 0;
	if (shotgun->value) settings->weapons += weapon_vals_x[1];
	if (supershotgun->value) settings->weapons += weapon_vals_x[2];
	if (machinegun->value) settings->weapons += weapon_vals_x[3];
	if (chaingun->value) settings->weapons += weapon_vals_x[4];
	if (grenadelauncher->value) settings->weapons += weapon_vals_x[5];
	if (railgun->value) settings->weapons += weapon_vals_x[6];
	if (hyperblaster->value) settings->weapons += weapon_vals_x[7];
	if (rocketlauncher->value) settings->weapons += weapon_vals_x[8];
	if (bfg->value) settings->weapons += weapon_vals_x[0];
	//
	settings->armor = armor->value;
	settings->health = health->value;
	settings->shells = shells->value;
	settings->bullets = bullets->value;
	settings->slugs = slugs->value;
	settings->grenades = grenades->value;
	settings->rockets = rockets->value;
	settings->cells = cells->value;
} //end of the function RA2_GetSettings
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_GiveAmmo(edict_t *e)
{
	arena_settings_t settings;
	gitem_t *w[9];
	gitem_t *it;
	int i;
	
	RA2_GetSettings(&settings);
	
	// give health
	e->health = settings.health;
	
	// give weapons
	memset(w, 0, sizeof(w));
	w[0] = FindItemByClassname("weapon_bfg");
	w[1] = FindItemByClassname("weapon_shotgun");
	w[2] = FindItemByClassname("weapon_supershotgun");
	w[3] = FindItemByClassname("weapon_machinegun");
	w[4] = FindItemByClassname("weapon_chaingun");
	w[5] = FindItemByClassname("weapon_grenadelauncher");
	w[6] = FindItemByClassname("weapon_railgun");
	w[7] = FindItemByClassname("weapon_hyperblaster");
	w[8] = FindItemByClassname("weapon_rocketlauncher");
	for(i = 0; i <= 8; i++)
	{
		if (settings.weapons & weapon_vals_x[i])
		{
			e->client->newweapon = w[i];
			e->client->pers.lastweapon = w[i];
			e->client->pers.selected_item = e->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(w[i]);
			e->client->pers.inventory[ITEM_INDEX(w[i])] = 1;
		} //end if
		else
		{
			e->client->pers.inventory[ITEM_INDEX(w[i])] = 0;
		} //end else
	} //end for
	// give ammo
	if(it = FindItemByClassname("ammo_shells")) e->client->pers.inventory[ITEM_INDEX(it)] = settings.shells;	//shells	100
	if(it = FindItemByClassname("ammo_bullets")) e->client->pers.inventory[ITEM_INDEX(it)] = settings.bullets;	//bullets       200
	if(it = FindItemByClassname("ammo_slugs")) e->client->pers.inventory[ITEM_INDEX(it)] = settings.slugs;	//slugs	 50
	if(it = FindItemByClassname("ammo_grenades")) e->client->pers.inventory[ITEM_INDEX(it)] = settings.grenades;	//grenades      50
	if(it = FindItemByClassname("ammo_rockets")) e->client->pers.inventory[ITEM_INDEX(it)] = settings.rockets;	//rockets       50
	if(it = FindItemByClassname("ammo_cells")) e->client->pers.inventory[ITEM_INDEX(it)] = settings.cells;	//cells	 200
	// give body armor
	if(it = FindItemByClassname("item_armor_body"))
	{
		e->client->pers.inventory[ITEM_INDEX(it)] = settings.armor;
	} //end if
	//set max
	e->client->pers.health			= 100;
	e->client->pers.max_health		= 100;
	e->client->pers.max_bullets	= settings.bullets;
	e->client->pers.max_shells		= settings.shells;
	e->client->pers.max_rockets	= settings.cells;
	e->client->pers.max_grenades	= settings.grenades;
	e->client->pers.max_cells		= settings.rockets;
	e->client->pers.max_slugs		= settings.slugs;
} //end of the function RA2_GiveAmmo
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Cmd_toarena_f(edict_t *ent, int context)
{
	if (!ra->value)
	{
		if (ent) gi.cprintf(ent, PRINT_HIGH, "Rocket Arena should be enabled to use this command\n");
		else gi.dprintf("Rocket Arena should be enabled to use this command\n");
		return;
	} //end if
	if (context < 0 || context > num_arenas)
	{
		if (ent) gi.cprintf(ent, PRINT_HIGH, "arena number not in the range [0, %d]\n", num_arenas);
		else gi.dprintf("arena number not in the range [0, %d]\n", num_arenas);
		return;
	} //end if
	//put the client in the desired arena
	ent->client->resp.context = context;
	//put the client at the end of the waiting list
	ent->client->ra_time = level.time;
	//move to the arena
	RA2_MoveToArena(ent, context, true);
} //end of the function Cmd_toarena_f
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
edict_t *RA2_GetArenaWinner(int arena)
{
	int i;
	edict_t *e;

	for (i = 0; i < maxclients->value; i++)
	{
		e = g_edicts + 1 + i;
		if(!e->inuse) continue;
		if (!e->client) continue;
		if (e->client->resp.context != arena) continue;
		if (e->client->ra_winner) return e;
	} //end for
	return NULL;
} //end of the function RA2_GetArenaWinner
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
#ifdef BOT
edict_t *RA2_GetLongestWaitingHuman(int arena)
{
	int i;
	float besttime;
	edict_t *e, *bestent;

	besttime = 0;
	bestent = NULL;
	for (i = 0; i < maxclients->value; i++)
	{
		e = g_edicts + 1 + i;
		if(!e->inuse) continue;
		if (!e->client) continue;
		if (e->flags & FL_BOT) continue;
		if (e->client->resp.context != arena) continue;
		//invalid time, client is already in the match
		if (e->client->ra_time < 0) continue;
		//if no time set then just set the current time
		if (!e->client->ra_time) e->client->ra_time = level.time;
		//get the client that is waiting longest
		if (!besttime || e->client->ra_time < besttime)
		{
			besttime = e->client->ra_time;
			bestent = e;
		} //end if
	} //end for
	return bestent;
} //end of the function RA2_GetLongestWaitingHuman
#endif //BOT
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
edict_t *RA2_GetLongestWaiting(int arena)
{
	int i;
	float besttime;
	edict_t *e, *bestent;

	besttime = 0;
	bestent = NULL;
	for (i = 0; i < maxclients->value; i++)
	{
		e = g_edicts + 1 + i;
		if(!e->inuse) continue;
		if (!e->client) continue;
		if (e->client->resp.context != arena) continue;
		//invalid time, client is already in the match
		if (e->client->ra_time < 0) continue;
		//if no time set then just set the current time
		if (!e->client->ra_time) e->client->ra_time = level.time;
		//get the client that is waiting longest
		if (!besttime || e->client->ra_time < besttime)
		{
			besttime = e->client->ra_time;
			bestent = e;
		} //end if
	} //end for
	return bestent;
} //end of the function RA2_GetLongestWaiting
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_AddPlayersOnSameTeamToMatch(int arena, edict_t *ent)
{
	int i;
	edict_t *e;

	for (i = 0; i < maxclients->value; i++)
	{
		e = g_edicts + 1 + i;
		if(!e->inuse) continue;
		if (!e->client) continue;
		if (e->client->resp.context != arena) continue;
		if (ent == e || OnSameTeam(ent, e))
		{
			//invalidate the time
			e->client->ra_time = -1;
			//
			e->takedamage = DAMAGE_AIM;
			e->flags &= ~FL_NOTARGET;
			RA2_GiveAmmo(e);
			RA2_MoveToArena(e, e->client->resp.context, false);
		} //end if
	} //end for
} //end of the function RA2_AddPlayersOnSameTeamToMatch
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_StartMatch(int context)
{
	int i;
	edict_t *e;

	gi.dprintf("Starting match in %s\n", RA2_GetArenaName(context));
	//if bot cycling is on
	if (ra_botcycle->value)
	{
		e = RA2_GetLongestWaitingHuman(context);
		if (e)
		{
			//if there is/are winner(s)
			RA2_AddPlayersOnSameTeamToMatch(context, e);
		} //end if
		else
		{
			//there are no winners so add the longest waiting player/team
			e = RA2_GetLongestWaiting(context);
			if (e) RA2_AddPlayersOnSameTeamToMatch(context, e);
		} //end else
		//add the second player/team
		e = RA2_GetLongestWaiting(context);
		if (e) RA2_AddPlayersOnSameTeamToMatch(context, e);
	} //end if
	//if player cycling is on
	else if (ra_playercycle->value)
	{
		//add the winner(s) from the previous match
		e = RA2_GetArenaWinner(context);
		if (e)
		{
			//if there is/are winner(s)
			RA2_AddPlayersOnSameTeamToMatch(context, e);
		} //end if
		else
		{
			//there are no winners so add the longest waiting player/team
			e = RA2_GetLongestWaiting(context);
			if (e) RA2_AddPlayersOnSameTeamToMatch(context, e);
		} //end else
		//add the second player/team
		e = RA2_GetLongestWaiting(context);
		if (e) RA2_AddPlayersOnSameTeamToMatch(context, e);
	} //end if
	else //no player cycling just add everyone to the match
	{
		for (i = 0; i < maxclients->value; i++)
		{
			e = g_edicts + 1 + i;
			if(e->inuse && e->client && e->client->resp.context == context)
			{
				e->takedamage = DAMAGE_AIM;
				e->flags &= ~FL_NOTARGET;
				RA2_GiveAmmo(e);
				RA2_MoveToArena(e, e->client->resp.context, false);
			} //end if
		} //end for
	} //end else
} //end of the function RA2_StartMatch
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_StartMatch_think(edict_t *ent)
{
	char buf[256];

	if (ent->count <= 0)
	{
		RA2_ArenaCenterPrint("FIGHT!", ent->style);
		RA2_ArenaSound("ra/sffight.wav", ent->style);
	} //end if
	else
	{
		sprintf(buf, "%d", ent->count);
		RA2_ArenaCenterPrint(buf, ent->style);
		sprintf(buf, "ra/sf%d.wav", ent->count);
		RA2_ArenaSound(buf, ent->style);
	} //end else
	if (ent->count <= 0)
	{
		RA2_StartMatch(ent->style);
		G_FreeEdict(ent);
		return;
	} //end if
	ent->count--;
	ent->nextthink = level.time + 1;
} //end of the function RA2_StartMatch_think
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Cmd_start_match_f(edict_t *ent, int context)
{
	edict_t *e;

	if (!ra->value)
	{
		if (ent) gi.cprintf(ent, PRINT_HIGH, "Rocket Arena should be enabled to use this command\n");
		else gi.dprintf("Rocket Arena should be enabled to use this command\n");
		return;
	} //end if
	if (context <= 0 || context > num_arenas)
	{
		if (ent) gi.centerprintf(ent, "first enter an arena in the range [1, %d]", num_arenas);
		else gi.dprintf("the arena number must be in the range [1, %d]\n", num_arenas);
		return;
	} //end if
	e = G_Find(NULL, FOFS(classname), "arenacountdown");
	while(e)
	{
		if (e->style == context) return;
		e = G_Find(e, FOFS(classname), "arenacountdown");
	} //end while
	e = G_Spawn();
	e->classname = "arenacountdown";
	e->style = context;
	e->count = 3;
	e->think = RA2_StartMatch_think;
	e->nextthink = level.time + 1;
} //end of the function Cmd_start_match_f
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_StopMatch(int context)
{
	int i;
	edict_t *e;

	//respawn all the players
	for (i = 0; i < maxclients->value; i++)
	{
		e = g_edicts + 1 + i;
		if (!e->inuse) continue;
		if (!e->client) continue;
		if (e->client->resp.context != context) continue;
		if (e->health <= 0 || e->deadflag == DEAD_DEAD)
		{
			respawn(e);
		} //endif
	} //end for
	//
	for (i = 0; i < maxclients->value; i++)
	{
		e = g_edicts + 1 + i;
		if (!e->inuse) continue;
		if (!e->client) continue;
		if (e->client->resp.context != context) continue;
		if (e->movetype != MOVETYPE_WALK) continue;
		//
		RA2_MoveToArena(e, e->client->resp.context, true);
		e->takedamage = DAMAGE_NO;
		e->flags |= FL_NOTARGET;
	} //end for
	//
	if (gi.cvar("mstart_auto", "0", 0)->value)
	{
		Cmd_start_match_f(NULL, context);
	} //end if
} //end of the function RA2_StopMatch
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_StopMatch_think(edict_t *ent)
{
	if (ent->count <= 0)
	{
		RA2_StopMatch(ent->style);
		G_FreeEdict(ent);
		return;
	} //end if
	ent->count--;
	ent->nextthink = level.time + 1;
} //end of the function RA2_StartMatch_think
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Cmd_stop_match_f(edict_t *ent, int context, int delay)
{
	edict_t *e;
	
	if (!ra->value)
	{
		if (ent) gi.cprintf(ent, PRINT_HIGH, "Rocket Arena should be enabled to use this command\n");
		else gi.dprintf("Rocket Arena should be enabled to use this command\n");
		return;
	} //end if
	if (context <= 0 || context > num_arenas)
	{
		if (ent) gi.centerprintf(ent, "first enter an arena in the range [1, %d]", num_arenas);
		else gi.dprintf("the arena number must be in the range [1, %d]\n", num_arenas);
		return;
	} //end if
	e = G_Find(NULL, FOFS(classname), "stopmatch");
	while(e)
	{
		if (e->style == context) return;
		e = G_Find(e, FOFS(classname), "stopmatch");
	} //end while
	e = G_Spawn();
	e->classname = "stopmatch";
	e->style = context;
	e->count = delay;
	e->think = RA2_StopMatch_think;
	e->nextthink = level.time + 1;
} //end of the function Cmd_stop_match_f
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void RA2_CheckRules(void)
{
	int i, context, numarenaplayers, livingplayers;
	edict_t *lastclient, *e;
	char buf[1024];

	//check for automatically stopping matches
	for (context = 1; context <= num_arenas; context++)
	{
		livingplayers = 0;
		numarenaplayers = 0;
		lastclient = NULL;
		//find out if there are still people playing in the arena
		for (i = 0; i < maxclients->value; i++)
		{
			e = g_edicts + 1 + i;
			if (!e->inuse) continue;
			if (!e->client) continue;
			if (e->client->resp.context != context) continue;
			//another player in this arena
			numarenaplayers++;
			//if the player is observing
			if (e->takedamage == DAMAGE_NO) continue;
			//if the player is dead
			if (e->health <= 0 || e->deadflag == DEAD_DEAD) continue;
			//this player is still living in the arena
			livingplayers++;
			//
			if ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))
			{
				//if there are other players in the arena
				if (lastclient)
				{
					//if one of the other players is not on the same team
					if (!OnSameTeam(lastclient, e)) break;
				} //end if
				lastclient = e;
			} //end if
			else
			{
				//if there's more than one client in the arena
				if (lastclient) break;
				lastclient = e;
			} //end else
		} //end for
		//if there are still people playing in the arena
		if (i < maxclients->value) continue;
		//if there aren't anough players in the arena anyway
		if (numarenaplayers <= 1) continue;
		//if there is only one active player in the arena
		//or there are only active players in the arena on the same team
		if (!lastclient)
		{
			//if NOT there are no living players at all in the arena and the game should be auto restarted
			if (!(!livingplayers && gi.cvar("mstart_auto", "0", 0)->value)) continue;
		} //end if
		//if the match is already being stopped
		e = G_Find(NULL, FOFS(classname), "stopmatch");
		while(e)
		{
			if (e->style == context) break;
			e = G_Find(e, FOFS(classname), "stopmatch");
		} //end while
		if (e) continue;
		//if the match is already being started
		e = G_Find(NULL, FOFS(classname), "arenacountdown");
		while(e)
		{
			if (e->style == context) break;
			e = G_Find(e, FOFS(classname), "arenacountdown");
		} //end while
		if (e) continue;
		//for player cycling reset the waiting time and winner flags
		for (i = 0; i < maxclients->value; i++)
		{
			e = g_edicts + 1 + i;
			if (!e->inuse) continue;
			if (!e->client) continue;
			if (e->client->resp.context != context) continue;
			//if this player was in the match
			if (e->client->ra_time < 0) e->client->ra_time = level.time;
			//assume player is not a winner
			e->client->ra_winner = false;
		} //end for
		//if there was at least one last client
		if (lastclient)
		{
			//print the winner(s) on the screen
			if ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))
			{
				strcpy(buf, "the team with the players:\n");
				for (i = 0; i < maxclients->value; i++)
				{
					e = g_edicts + 1 + i;
					if (!e->inuse) continue;
					if (!e->client) continue;
					if (e->client->resp.context != context) continue;
					//if this player was in the match
					if (e->client->ra_time < 0) e->client->ra_time = level.time;
					//assume player is not a winner
					e->client->ra_winner = false;
					//if the player is on the same team
					if (OnSameTeam(lastclient, e))
					{
						//for player cycling keep track of the winners
						e->client->ra_winner = true;
						//
						strcat(buf, e->client->pers.netname);
						strcat(buf, "\n");
					} //end if
				} //end for
				strcat(buf, "WON!");
			} //end if
			else
			{
				sprintf(buf, "%s WON!\n", lastclient->client->pers.netname);
				//for player cycling keep track of the winners
				lastclient->client->ra_winner = true;
			} //end else
			RA2_ArenaCenterPrint(buf, context);
		} //end if
		//stop the match after three seconds
		Cmd_stop_match_f(NULL, context, 3);
	} //end for
} //end of the function RA2_CheckRules
