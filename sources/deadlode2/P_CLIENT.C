#include "g_local.h"
#include "dl_player.h"

void ClientUserinfoChanged (edict_t *ent, char *userinfo);
extern void WeaponThink(edict_t *ent);

void SP_misc_teleporter_dest (edict_t *ent);

//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

static void SP_FixCoopSpots (edict_t *self)
{
	edict_t	*spot;
	vec3_t	d;

	spot = NULL;

	while(1)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (!spot)
			return;
		if (!spot->targetname)
			continue;
		VectorSubtract(self->s.origin, spot->s.origin, d);
		if (VectorLength(d) < 384)
		{
			if ((!self->targetname) || DL_strcmp(self->targetname, spot->targetname, -1, false) != 0)
				self->targetname = spot->targetname;
			return;
		}
	}
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self)
{
	edict_t	*spot;

	if(DL_strcmp(level.mapname, "security", -1, false) == 0)
	{
		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 - 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 128;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		return;
	}
}


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
	if (!coop->value)
		return;
	if(DL_strcmp(level.mapname, "security", -1, false) == 0)
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_CreateCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	// deadlode doesn't show DM spawnpoints at telepads.
	// SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}

	if((DL_strcmp(level.mapname, "jail2", -1, false) == 0)   ||
	   (DL_strcmp(level.mapname, "jail4", -1, false) == 0)   ||
	   (DL_strcmp(level.mapname, "mine1", -1, false) == 0)   ||
	   (DL_strcmp(level.mapname, "mine2", -1, false) == 0)   ||
	   (DL_strcmp(level.mapname, "mine3", -1, false) == 0)   ||
	   (DL_strcmp(level.mapname, "mine4", -1, false) == 0)   ||
	   (DL_strcmp(level.mapname, "lab", -1, false) == 0)     ||
	   (DL_strcmp(level.mapname, "boss1", -1, false) == 0)   ||
	   (DL_strcmp(level.mapname, "fact3", -1, false) == 0)   ||
	   (DL_strcmp(level.mapname, "biggun", -1, false) == 0)  ||
	   (DL_strcmp(level.mapname, "space", -1, false) == 0)   ||
	   (DL_strcmp(level.mapname, "command", -1, false) == 0) ||
	   (DL_strcmp(level.mapname, "power2", -1, false) == 0) ||
	   (DL_strcmp(level.mapname, "strike", -1, false) == 0))
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_FixCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
/*
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
*/

//==================================================================//
//  PM:  Get string of player's PPM directory and if it matches
//  the Q2 "female" marine or the crackwhore, player is female.
//-------------------------------------------------------------//
	char		*info;
        char            ppm[16];

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
        sscanf (info, "%15[^/]", ppm);
        if (DL_strcmp(ppm, "female", -1, false) == 0)
		return true;
        if (DL_strcmp(ppm, "crakhor", -1, false) == 0)
		return true;
	return false;
//==================================================================//
}


void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker) {
edict_t	*owner;
char	*msg;
int		location;
char	*locname="chest", *bldgname = "unknown", *weapname = "unknown", *ammoname = "unknown", *basename = "unknown";
int		mod;
char	*modName[] = {
	"default","blaster","shotgun","sshotgun","machinegun","chaingun","grenade","g_splash",
	"rocket","r_splash","hyperblaster","railgun","bfg_laser","bfg_blast","blg_effect",
	"handgrenade","hg_splash","water","slime","lava","crush","telefrag","falling","suicide",
	"held_grenade","explosive","barrel","bomb","exit","splash","laser","hurt","hit","blaster","grapple","shockwave","pushed"
};

	if (!attacker || !inflictor || !self)
		return;

	// determine WHAT actually fired the round (building, player, etc)
	if (!DL_strcmp(inflictor->classname, "projectile", -1, false))
		owner = inflictor->orig_owner;
	else
		owner = inflictor;

	if (inflictor == attacker) {
	//instant hit weapon
		if (attacker->client) {	// attacker is an actual player
			// Get weapon name and basename
			if (attacker->client->curweap) {
				weapname = GetItemName(attacker->client->curweap);
				if (attacker->client->curweap->cliptype)
					ammoname = attacker->client->curweap->cliptype->name;
				basename = ITEMWEAP(attacker->client->curweap)->base_ammo;
			}
		} else {	// attacker in something else (trigger that activated gun?)
			// Get weapon name and basename
			if (attacker->weaptype) {
				weapname = attacker->weaptype->name;
				basename = attacker->weaptype->base_ammo;
			}
			// Get ammo name and basename in case there was no weapon
			if (attacker->ammotype) {
				ammoname = attacker->ammotype->name;
				basename = attacker->ammotype->base_name;
			}
		}
	} else {	// projectile weapon or supply
		if (inflictor->supply) {	// The inflictor was a supply, rather than a direct player weapon
			// note that projectiles can be supplies too (remember the MIRV grenades!)
			owner = inflictor->master;
			if (!DL_strcmp(inflictor->classname, "primed_grenade", -1, false)) {
				// Primed Grenade (not thrown) - fill in weapname and basename with relevant values
				weapname = "Primed Grenade";
				ammoname = inflictor->supply->name;
			} else {
				// Hand Grenade (thrown) - fill in weapname and basename with relevant values
				weapname = "Hand Grenade";
				ammoname = inflictor->supply->name;
			}
		} else if (!DL_strcmp(inflictor->classname, "projectile", -1, false)) {	// A Player weapon
			if (inflictor->weaptype) {
				// Get weapon name and basename
				weapname = inflictor->weaptype->name;
				basename = inflictor->weaptype->base_ammo;
			}
			// Get ammo name and basename in case there was no weapon
			if (inflictor->ammotype) {
				ammoname = inflictor->ammotype->name;
				basename = inflictor->ammotype->base_name;
			}
		}
	}

	// Get hit-location info
	location = GetHitLocation (hitpos, self);
	switch (location) {
	case LOC_HEAD:
		locname = "head";
		break;
	case LOC_LEGS:
		locname = "legs";
		break;
	case LOC_CHEST:
	default:
		locname = "chest";
	}

	if (owner->client) {	// player was killed by a players weapon
		if (attacker == self) {
			// player suicides
			mod = MOD_KILLSELF;
			self->client->resp.score--;
		} else if (attacker->client->resp.team == self->client->resp.team) {
			// player kills a teammate
			mod = MOD_KILLTEAM;
			attacker->client->resp.score--;
		} else {
			// player kills any other (non-friendly)
			mod = MOD_KILLOTHER;
			attacker->client->resp.score++;
		}

		DL_DoPlayerStats(self, attacker, mod);

		if (msg = GetDeathMessage(mod, weapname, ammoname));
		else if (msg = GetDeathMessage(mod, weapname, "default"));
		else if	(msg = GetDeathMessage(mod, "default", ammoname));
		else if (msg = GetDeathMessage(mod, "default", "default"));
		else msg = "%v died.";

		msg = MeansOfDeath(msg, self->client->pers.netname, attacker->client->pers.netname, NULL, weapname, ammoname, basename, locname, IsFemale(self));
		gi.bprintf (PRINT_MEDIUM, "%s\n", msg);
		return;
	}

	if (owner->building) {	// was killed by a buildings weapons
		bldgname = owner->building->name;
		if (self == attacker) {
			// player killed himself (while controlling the building)
			mod = MOD_BLDG_KILLSELF;
			self->client->resp.score--;
		} else if (self == owner->master) {
			// player was killed by his own building (usually under someone elses control)
			mod = MOD_BLDG_KILLOWNER;
			if (attacker->client) // under someone elses control?
				attacker->client->resp.score++;
			else
				self->client->resp.score--;
		} else if (self->client->resp.team == attacker->client->resp.team) {
			// players building killed a teammate
			mod = MOD_BLDG_KILLTEAM;
			if (attacker->client)
				attacker->client->resp.score--;
		} else {
			// players building killed any other (non-friendly)
			mod = MOD_BLDG_KILLOTHER;
			if (attacker->client)
				attacker->client->resp.score++;
			else
				self->client->resp.score--;
		}
			
		DL_DoPlayerStats(self, attacker, mod);

		if (msg = GetDeathMessage(mod, bldgname, weapname));
		else if (msg = GetDeathMessage(mod, bldgname, "default"));
		else if	(msg = GetDeathMessage(mod, "default", weapname));
		else if (msg = GetDeathMessage(mod, "default", "default"));
		else msg = "%v died.";
				
		msg = MeansOfDeath(msg, self->client->pers.netname, attacker->client ? attacker->client->pers.netname : "the world", bldgname, weapname, ammoname, basename, locname, IsFemale(self));
		gi.bprintf (PRINT_MEDIUM, "%s\n", msg);
		return;
	}

	mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
	if ((mod == MOD_PUSHED) && (attacker == self))
		mod = MOD_FALLING;

	if (msg = GetDeathMessage(MOD_WORLD,"",modName[mod]));
	else msg = "%v died.";

	if ((mod == MOD_TELEFRAG || mod == MOD_PUSHED) && attacker->client)
		attacker->client->resp.score++;
	else
		self->client->resp.score--;

	msg = MeansOfDeath(msg, self->client->pers.netname, attacker->client ? attacker->client->pers.netname : "the world", NULL, NULL, NULL, NULL, locname, IsFemale(self));
	gi.bprintf (PRINT_MEDIUM, "%s\n", msg);
	return;
}

/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;
	edict_t	*g;

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
	self->s.modelindex3 = 0;	// remove linked ctf flag

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0 + dlsys.respawn_delay;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
// DEADLODE
		DropGoalItem(self);										// drop any goalitems we might be carrying
		self->client->curweap = self->client->nextweap = NULL;	// drop the current weapon
		if (self->client->building)								// stop building
			Building_finish(self->client->building);
		if (self->use_this)										// Release any items being controlled
			self->use_this = self->use_this->use_this = NULL;
		self->flashlight = NULL;								// Turn off flashlights
		self->scope = NULL;										// Stop using any scopes
		self->client->night_vision = false;						// Disable any nightvision equipment
		if (self->client->viewcam)								// Quit viewing camera's and drop body double
			Camera_select(self, NULL);
		Grapple_Release(self->grapple);							// Let go of grapple
		// kill any buildings attached to this player
		for (g = g_edicts ; g < &g_edicts[globals.num_edicts] ; g++) {
			if (!g->inuse)
				continue;
			if (g->rotate_target == self) {
				if (g->die)
					g->die(g, inflictor, attacker, damage, point);
				else
					G_FreeEdict(g);
			}
		}



		// destroy any building under construction by this player.
		if (self->client->building)	{
			if (self->client->building->die)
				self->client->building->die(self->client->building, inflictor, attacker, damage, point);
			else
				G_FreeEdict (self->client->building);

			self->client->building = NULL;
		}

		DL_WritePlayerStats(self);
		// DEADLODE
		// Lets not forget to write any changes to our profile
		Ini_WriteIniFile(self->client->prefs.IHnd);
// DEADLODE

		if (deathmatch->value && !self->client->showscores)
			Cmd_Help_f (self);		// show scores
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;

	// clear inventory
//	memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));

	if (self->health < -140)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead (self, damage);
//ZOID
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = 0;
//ZOID
		self->takedamage = DAMAGE_NO;
		self->s.renderfx = 0;
		self->s.modelindex4 = 0;	// remove linked armour model

		// free the bodies remaining inventory list
		ClearInventory(self);
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			GetPPMFrameSet(self->client, PPM_DIE, &self->s.frame, &self->client->anim_end);
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;
	gi.linkentity (self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
//DEADLODE
//holy hack and a half, batman!
//	qboolean	inmenu;
//	char		*vote;
//	pstats_t	stats;
//	IniFile		*IHnd;

//	inmenu = client->prefs.inmenu;
//	vote = client->prefs.mapvote;
//	stats = client->prefs.stats;
//	IHnd = client->prefs.IHnd;

	memset (&client->pers, 0, sizeof(client->pers));
//DEADLODE

	client->pers.health			= 100;
	client->pers.max_health		= 100;

//DEADLODE
//	client->prefs.inmenu			= inmenu;
//	client->prefs.mapvote		= vote;
//	client->prefs.stats			= stats;
//	client->prefs.IHnd			= IHnd;

//	client->prefs.menu_pos		= MENU_POS_LEFT|MENU_POS_TOP;
//	client->prefs.no_menu_background	= true;
//DEADLODE
	client->pers.connected = true;
}


void InitClientResp (gclient_t *client)
{
	memset (&client->resp, 0, sizeof(client->resp));
	
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
 
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
//		game.clients[i].pers.powerArmorActive = (ent->flags & FL_POWER_ARMOR);
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
//	if (ent->client->pers.powerArmorActive)
//		ent->flags |= FL_POWER_ARMOR;
	if (coop->value)
		ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (edict_t *spot)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
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
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

edict_t *SelectDeathmatchSpawnPoint (void)
{
	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint ();
	else
		return SelectRandomDeathmatchSpawnPoint ();
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( DL_strcmp(game.spawnpoint, target, -1, false) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

	if (deathmatch->value)
//ZOID
		if (ctf->value)
			spot = SelectDLTeamSpawnPoint(ent);
//			spot = SelectCTFSpawnPoint(ent);
		else
//ZOID
			spot = SelectDeathmatchSpawnPoint ();
	else if (coop->value)
		spot = SelectCoopSpawnPoint (ent);

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (DL_strcmp(game.spawnpoint, spot->targetname, -1, false) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
				gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
}

//======================================================================


void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

	if (self->health < -140)
	{
		// free the bodies remaining inventory list
		ClearInventory(self);

		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage / 5, GIB_ORGANIC);
		self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;
		self->s.renderfx = 0;
		self->s.modelindex4 = 0;

	}
}

void CopyToBodyQue (edict_t *ent)
{
	edict_t		*body;


	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity (ent);

	gi.unlinkentity (body);
	body->s = ent->s;
	body->s.number = body - g_edicts;

	body->svflags = ent->svflags;
	VectorCopy (ent->mins, body->mins);
	VectorCopy (ent->maxs, body->maxs);
	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
	VectorCopy (ent->size, body->size);
	body->solid = ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->movetype = ent->movetype;

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

	// give the body the players inventory
	body->inventory = ent->inventory;
	ent->inventory = NULL;

	gi.linkentity (body);
}


void respawn (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{
		CopyToBodyQue (self);
		PutClientInServer (self);

		// add a teleportation effect
		self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		if (self->client->respawn_time < level.time)
			self->client->respawn_time = level.time;

		return;
	}

	// restart the entire server
	gi.AddCommandString ("menu_loadgame\n");
}

//==============================================================


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
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	client_prefs_t		prefs;
	item_t	*wp, *am;
	edict_t	*g = NULL;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
	}
	else
	{
		memset (&resp, 0, sizeof(resp));
	}

	// clear everything but the persistant data
	saved = client->pers;
	prefs = client->prefs;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;
	client->prefs = prefs;

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;
//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

//	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
//	ent->s.modelindex2 = ent->client->vw_index;		// custom gun model
	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	// DEADLODE
	// Destroy any effect_persistant entities before the player respawns...
	while (g = G_Find(g, FOFS(classname), "effect_persistant")) {
		if (g->enemy = ent)
			G_FreeEdict(g);
	}

	// Make the next profile, the current one.
	DL_strcpy(ent->client->resp.profile, ent->client->resp.next_profile, -1);

	if (DL_StartClient(ent)) {
		gi.linkentity (ent);
		return;
	}

	ent->client->camera = NULL;
	ent->client->dummy = NULL;
	ent->grenade = NULL;
	ent->grapple = NULL;
	ent->flashlight = NULL;
	ent->team = client->resp.team;
	ent->goalitem = NULL;
	ent->scope = NULL;

	// make player visible
	ent->svflags &= ~SVF_NOCLIENT;
	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;

	DL_AssignSkin(ent, Info_ValueForKey(ent->client->pers.userinfo, "skin"));

	ClearInventory(ent);
	if (ent->client->resp.profile[0] == -1)
		LoadProfile(ent, ent->client->prefs.IHnd, 0, 1.0);
	else
		LoadSystemProfile(ent, ent->client->resp.profile, 0, 1.0);
	
	// Preload weapons
	for (wp = ent->inventory; wp; wp=wp->next) {
		if (wp->itemtype != ITEM_WEAPON)
			continue;
		am = FindPreferedCompatibleAmmo(ent, ITEMWEAP(wp)->base_ammo);
		if (am) {
			wp->cliptype = ITEMAMMO(am);
			wp->clipcnt = ITEMWEAP(wp)->clip_size;
			if (wp->clipcnt < ITEMWEAP(wp)->rnds_chambered)
				wp->clipcnt = ITEMWEAP(wp)->rnds_chambered;
			if (wp->clipcnt > am->quantity)
				wp->clipcnt = am->quantity;

			am->quantity -= wp->clipcnt;
		}
	}
	// DEADLODE

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	G_InitEdict (ent);


	ent->client->resp.team = NULL;
	InitClientResp (ent->client);

	ClearInventory(ent);
	if (OpenPlayerProfile(ent))
		DL_ReadPlayerStats(ent);

	stuffcmd(ent, "skin male/grunt");

	// FIXME: Use entry in system.ini for this value
	ent->client->resp.lives = 3;	// hardcoded 3 lives per player

	gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

	if (dlsys.auto_team) {
		ent->team = ent->client->resp.team = AssignToTeam();
		ent->client->resp.team->players++;

		gi.bprintf(PRINT_HIGH, "%s was assigned to Team %s\n", ent->client->pers.netname, ent->client->resp.team->team_name);
	}

	// locate ent at a spawn point
	PutClientInServer (ent);
	memset(ent->client->oldmenu, 0, 1400);

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}


/*`
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int		i;

	ent->client = game.clients + (ent - g_edicts - 1);

	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);
		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
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
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict (ent);
		ent->classname = "player";
		InitClientResp (ent->client);
		PutClientInServer (ent);
	}

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect if in a multiplayer game
		if (game.maxclients > 1)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_LOGIN);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
		}
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

	playernum = ent-g_edicts-1;

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
		DL_strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt", -1);

	// set name
	s = Info_ValueForKey (userinfo, "name");
	DL_strcpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);
	gi.configstring(CS_PLAYER_NAMES+playernum, ent->client->pers.netname);

	// assign the player skins
	s = Info_ValueForKey (userinfo, "skin");
	DL_AssignSkin(ent, s);

	// fov
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
		ent->client->ps.fov = 90;
	else {
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
		ent->client->pers.hand = atoi(s);

	// save off the userinfo in case we want to check something later
	DL_strcpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	char	*value;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");

	// check for a password
	value = Info_ValueForKey (userinfo, "password");
	if (DL_strcmp(password->string, value, -1, true) != 0)
		return false;

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
		InitClientResp (ent->client);
		InitClientPersistant (ent->client);
		//DEADLODE
		memset(&ent->client->prefs, 0, sizeof(ent->client->prefs));
		ent->client->prefs.inmenu = true;
		ent->client->prefs.no_menu_background = true;
		ent->client->prefs.menu_pos = 0;
		//DEADLODE
	}

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s connected\n", ent->client->pers.netname);

	ent->client->pers.connected = true;
	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent) {
int		playernum;

	if (!ent->client)
		return;

	gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

//ZOID
// 	CTFDeadDropFlag(ent);
//ZOID

// DEADLODE
	DropGoalItem(ent);				// drop any goalitems we might be carrying
	ReleaseBuildings(ent);			// Remove points from buildings
	Grapple_Release(ent->grapple);
	Camera_select(ent, NULL);		// Release Dummies from Cameras
	ent->scope = NULL;				// Stop using any scopes
	ent->client->building = NULL;	// stop building
	ent->client->curweap = ent->client->nextweap = NULL;
	if (ent->use_this) {			// Release any items being controlled
		ent->use_this->use_this = NULL;
		ent->use_this = NULL;
	}

	Ini_WriteIniFile(ent->client->prefs.IHnd);
// DEADLODE

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");

// DEADLODE
	RecountTeamPlayers();
// DEADLODE
}


//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}
void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

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
	edict_t	*other;
	int		i, j, mspeed;
	float	weight, time;
	pmove_t	pm;

	level.current_entity = ent;
	client = ent->client;

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// determine minimum intermission time
		if (dlsys.map_voting == true) time = 15.0;
		else time = 5.0;
		if ((level.time > level.intermissiontime + time)
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}
	pm_passent = ent;

//ZOID
	if (ent->client->chase_target) {
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		return;
	}
//ZOID

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != 255)
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)
		client->ps.pmove.pm_type = PM_DEAD;
	else
		client->ps.pmove.pm_type = PM_NORMAL;

	client->ps.pmove.gravity = sv_gravity->value;
	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
		pm.snapinitial = true;

	pm.cmd = *ucmd;

	pm.trace = PM_trace;	// adds default parms
	pm.pointcontents = gi.pointcontents;

	// DEADLODE
	// currently building
	/*
	if (client->building)
	{
		pm.cmd.forwardmove = 0;
		pm.cmd.sidemove = 0;
		pm.cmd.upmove = 0;
	}
	*/
	weight = GetInventoryWeight(ent);
	if (weight > 300) weight = 300;
	mspeed = 300 - weight;

	if ((ent->client->next_clspeedmsg < level.time) && ((abs(mspeed - ent->client->old_weight) > 10) ||
		(VectorLength(tv(pm.cmd.forwardmove, pm.cmd.sidemove, pm.cmd.upmove)) > (mspeed * 3))) ){
		ent->client->old_weight = mspeed;
		ent->client->next_clspeedmsg = level.time + 0.5;
		stuffcmd(ent, va("cl_forwardspeed %d\ncl_sidespeed %d\ncl_upspeed %d\n", mspeed, mspeed, mspeed));
		stuffcmd(ent, va("m_forward %d\nm_side %d\n", (mspeed / 200), (mspeed / 200)));
//		gi.dprintf("%s speeds reset: %d\n", ent->client->pers.netname, mspeed);
	}

	Grapple_DoMovement(ent, &pm);
	// DEADLODE

	// perform a pmove
	gi.Pmove (&pm);

	// save results of pmove
	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

	for (i=0 ; i<3 ; i++)
	{
		ent->s.origin[i] = pm.s.origin[i]*0.125;
		ent->velocity[i] = pm.s.velocity[i]*0.125;
	}

	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
		gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);

// DEADLODE
	for (i=0; i<3; i++)
		if (client->oldkick[i] != 0)
			client->oldkick[i] -= (client->oldkick[i] / 4);
// DEADLODE

	ent->viewheight = pm.viewheight;
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;
	if (pm.groundentity) {
		if (pm.groundentity->solid != SOLID_BSP)
			ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;

		ent->groundentity_linkcount = pm.groundentity->linkcount;
	}

	if (ent->deadflag)
	{
		client->ps.viewangles[ROLL] = 40;
		client->ps.viewangles[PITCH] = -15;
		client->ps.viewangles[YAW] = client->killer_yaw;

		client->ps.gunframe = 0;
		client->ps.gunindex = 0;
	}
	else
	{
		VectorCopy (pm.viewangles, client->v_angle);
		VectorCopy (pm.viewangles, client->ps.viewangles);
	}

//ZOID
//	if (client->ctf_grapple)
//		CTFGrapplePull(client->ctf_grapple);
//ZOID

	gi.linkentity (ent);

	if (ent->movetype != MOVETYPE_NOCLIP)
		G_TouchTriggers (ent);

	// touch other objects
	for (i=0 ; i<pm.numtouch ; i++)
	{
		other = pm.touchents[i];
		for (j=0 ; j<i ; j++)
			if (pm.touchents[j] == other)
				break;
		if (j != i)
			continue;	// duplicated
		if (!other->touch)
			continue;
		if (other->team && (other->team != ent->client->resp.team))
			continue;
		other->touch (other, ent, NULL, NULL);
	}


	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if ((client->latched_buttons & BUTTON_ATTACK)
// DEADLODE
		&& (level.time > client->respawn_time + 1.0)
// DEADLODE
//ZOID
		&& (ent->movetype != MOVETYPE_NOCLIP)
//ZOID
		)
	{
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			WeaponThink(ent);
			// Think_Weapon (ent);
		}
	}

	// DEADLODE
	if (ent->client->viewcam)
		Camera_clientupdate(ent);
//	if (ent->client->building)
//		ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	// DEADLODE

//ZOID
	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}
//ZOID
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask;

	if (level.intermissiontime)
		return;

	client = ent->client;

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk
// DEADLODE
		&& (level.time > client->respawn_time + 1.0)
// DEADLODE
//ZOID
		&& (ent->movetype != MOVETYPE_NOCLIP)
//ZOID
		)
		WeaponThink(ent);
		//Think_Weapon (ent);
	else
		client->weapon_thunk = false;

	ent->grapplethunk = false;

	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			// wait for either +attack or +jump, but not keys...
			buttonMask = BUTTON_ATTACK|BUTTON_USE;

			if ( ( client->latched_buttons & buttonMask ) ||
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ) )
			{
				client->buttons = client->latched_buttons = 0;
				respawn(ent);
			}
		}
		return;
	}

	client->latched_buttons = 0;
	
	// update player runtime stats
	client->prefs.stats.game_time += FRAMETIME;
	if (ent->goalitem && (ent->goalitem->goalitem == ent))
		client->prefs.stats.carry_time += FRAMETIME;
}
