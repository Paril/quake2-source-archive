#include "g_local.h"
#include "m_player.h"

void ClientUserinfoChanged (edict_t *ent, char *userinfo);

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
			if ((!self->targetname) || stricmp(self->targetname, spot->targetname) != 0)
			{
//				gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
				self->targetname = spot->targetname;
			}
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

	if(stricmp(level.mapname, "security") == 0)
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
	if(stricmp(level.mapname, "security") == 0)
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

	// Expert: no visible DM spawn pads; these 
	// pads randomly block shots and skew movement
	if (expflags & EXPERT_WEAPONS) 
		return;

	SP_misc_teleporter_dest (self);
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

	if((stricmp(level.mapname, "jail2") == 0)   ||
	   (stricmp(level.mapname, "jail4") == 0)   ||
	   (stricmp(level.mapname, "mine1") == 0)   ||
	   (stricmp(level.mapname, "mine2") == 0)   ||
	   (stricmp(level.mapname, "mine3") == 0)   ||
	   (stricmp(level.mapname, "mine4") == 0)   ||
	   (stricmp(level.mapname, "lab") == 0)     ||
	   (stricmp(level.mapname, "boss1") == 0)   ||
	   (stricmp(level.mapname, "fact3") == 0)   ||
	   (stricmp(level.mapname, "biggun") == 0)  ||
	   (stricmp(level.mapname, "space") == 0)   ||
	   (stricmp(level.mapname, "command") == 0) ||
	   (stricmp(level.mapname, "power2") == 0) ||
	   (stricmp(level.mapname, "strike") == 0))
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
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}


void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker, vec3_t point)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	// Expert Obit
	// If the Expert obituary system is enabled and a set of obits has been
	// successfully loaded, use the Expert obit system instead
	if (!(utilflags & EXPERT_DISABLE_CLIENT_OBITUARIES))
	{
		ExpertClientObituary(self, inflictor, attacker, meansOfDeath, point);
		return;
	}

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";

		switch (mod)
		{
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			message = "cratered";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			message = "sank like a rock";
			break;
		case MOD_SLIME:
			message = "melted";
			break;
		case MOD_LAVA:
			message = "does a back flip into the lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
		}
		if (attacker == self)
		{

			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsFemale(self))
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
			default:
				if (IsFemale(self))
					message = "killed herself";
				else
					message = "killed himself";
				break;
			}
		}
		if (message)
		{
			gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
			self->client->resp.score--;
			self->enemy = NULL;
			// Expert: GibStats Logging: log killed self
			if (utilflags & EXPERT_ENABLE_GIBSTAT_LOGGING)
				gsLogKillSelf(self, meansOfDeath);
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was blasted by";
				break;
			case MOD_SHOTGUN:
				message = "was gunned down by";
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_MACHINEGUN:
				message = "was machinegunned by";
				break;
			case MOD_CHAINGUN:
				message = "was cut in half by";
				message2 = "'s chaingun";
				break;
			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;
			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket";
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast";
				break;
			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s BFG";
				break;
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s handgrenade";
				break;
			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s handgrenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
//CTF
			case MOD_GRAPPLE:
				message = "was caught by";
				message2 = "'s grapple";
				break;
//CTF

			}
			if (message)
			{
				gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (deathmatch->value)
				{
					if (ff)
						attacker->client->resp.score--;
					else
						attacker->client->resp.score++;
				}
				// Expert: GibStats Logging: log a frag
				if (utilflags & EXPERT_ENABLE_GIBSTAT_LOGGING)
					gsLogFrag(self, attacker, meansOfDeath);
				return;
			}
		}
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value) {
		self->client->resp.score--;
		// Expert: GibStats Logging: log killed self
		if (utilflags & EXPERT_ENABLE_GIBSTAT_LOGGING) {
			gsLogKillSelf(self, meansOfDeath);
		}
	}
}


void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	float		spread;

	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	// Expert: Under balanced items, drop packs even 
	// if player had no ammo or was using a blaster,
	// since packs contain health and armor too
	// For packs from a player using a blaster, the 
	// ammo pack model is used as a world model
	if (!(expflags & EXPERT_BALANCED_ITEMS)) {
		if (! self->client->pers.inventory[self->client->ammo_index] )
			item = NULL;
		if (item && (strcmp (item->pickup_name, "Blaster") == 0))
			item = NULL;
	}

	// Expert: prevent id's quad drop if both Expert powerups 
	// and balanced items are enabled, since in that case we 
	// drop powerups in the weapon
	if (!((int)(dmflags->value) & DF_QUAD_DROP) ||
	    (expflags & EXPERT_POWERUPS && expflags & EXPERT_BALANCED_ITEMS)) {
		quad = false;
	} else {
		quad = (self->client->quad_framenum > (level.framenum + 10));
	}

	// drop item at an angle if also dropping quad
	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if (item)
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
		// Expert: add items to dropped items
		ExpertAddToDroppedWeapon(drop, self);
	}

	// Expert: note "quad" is always false if both balanced
	// items and Expert powerups are set
	if (quad)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
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

	// Expert: custom gibbing threshold
	int		iGibVal = -40;

	// Expert: Compute the value for gibbing based on lethality setting
	iGibVal *= sv_lethality->value;

	// Expert: Release hook if needed
	if (self->client->hook)
		Release_Grapple(self->client->hook);

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
//CTF
	self->s.modelindex3 = 0;	// remove linked ctf flag
//CTF

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

	// Expert: bodies no longer solid.  Too often they absorb random hits.  
	// Bodies can still be gibbed by explosions for entertainment value.
	self->solid = SOLID_NOT;
//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		// Expert: added point of infliction parameter
		ClientObituary (self, inflictor, attacker, point);

		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;

		// Expert Scoring Matrix FIXME has a loop case
		//UpdateMatrixScores (self, attacker);

		// Expert : award health instantly upon kill in Alternate Restore mode
		if (expflags & EXPERT_ALTERNATE_RESTORE &&
			!onSameTeam(attacker, self)) {
			alternateRestoreKill(attacker, self);
		}

		// Expert CTF
		if (ctf->value) {
			ExpertCTFScoring(self, inflictor, attacker);
		}

		// Expert CTF
		if (flagtrack->value) {
			FlagTrackScoring(self, inflictor, attacker);
		}

		TossClientWeapon (self);

		// Expert CTF
		if (ctf->value) {
			CTFDeadDropFlag(self);
		}
		
		// Expert FlagTrack
		if (flagtrack->value) {
			FlagTrackDeadDropFlag(self, attacker);
		}

		if (deathmatch->value && !self->client->showscores)
			Cmd_Help_f (self);		// show scores
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	// Expert: Inertial Screen
	self->client->force_framenum = 0;

	// clear inventory
	memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));

	// Expert: always gib in arena
	if (self->health < iGibVal || (int)sv_arenaflags->value & EXPERT_ENABLE_ARENA)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead (self, damage);
//CTF
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = 0;
//CTF
		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
			static int i;

			i = (i+1)%3;
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				self->s.frame = FRAME_crdeath1-1;
				self->client->anim_end = FRAME_crdeath5;
			}
			else switch (i)
			{
			case 0:
				self->s.frame = FRAME_death101-1;
				self->client->anim_end = FRAME_death106;
				break;
			case 1:
				self->s.frame = FRAME_death201-1;
				self->client->anim_end = FRAME_death206;
				break;
			case 2:
				self->s.frame = FRAME_death301-1;
				self->client->anim_end = FRAME_death308;
				break;
			}
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;

	gi.linkentity (self);

	// Expert Arena
	// Note arenaKilled calls respawn(); this will only be
	// valid after all model handling and dead-state values
	// are set up 
	if ((int)sv_arenaflags->value & EXPERT_ENABLE_ARENA) {
		arenaKilled(self);
	}
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
	gitem_t		*item;
	// Expert: initial weapon choosable by client
	gitem_t		*firstWeapon;
	char *weaponChoice = Info_ValueForKey(client->pers.userinfo, "fweapon");
	int			savedConnectState;

	savedConnectState = client->pers.connectState;

	memset (&client->pers, 0, sizeof(client->pers));

	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	// Expert: Give out some armor under balanced items, to 
	// narrow the range of power for armors.  We don't give 
	// full armor since opponents will typically be injured.
	if (expflags & EXPERT_BALANCED_ITEMS && 
		!(expflags & EXPERT_ALTERNATE_RESTORE)) {
		item = FindItem("Jacket Armor");
		client->pers.inventory[ITEM_INDEX(item)] = 50;

		item = FindItem("Blaster");
	}

	// Expert: Give out stuff with free gear enabled
	if (expflags & EXPERT_FREE_GEAR) {
		item = giveFreeGear(client);
	}
	
	// Expert: initial weapon choosable by client
	firstWeapon = FindItem(weaponChoice);
	if (firstWeapon != NULL && 
	    client->pers.inventory[ITEM_INDEX(firstWeapon)]) 
	{
		// player picked a valid weapon name that he does in fact have
		item = firstWeapon;
	}

	client->pers.weapon = item;

	// Expert: previous weapon switching
	client->pers.lastweapon = item;

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	// Expert: Alternate Restore mode is health only
	if (expflags & EXPERT_ALTERNATE_RESTORE) {
		client->pers.health			= 130;
		client->pers.max_health		= 160;
	}

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;

	client->pers.connected = true;

	// Expert: preserve connected state
	client->pers.connectState = savedConnectState;
}


void InitClientResp (gclient_t *client)
{
	// Expert: preserve team assignment and forwarding marker
	int team = client->resp.team;
	qboolean forward = client->resp.toBeForwarded;

	memset (&client->resp, 0, sizeof(client->resp));
	
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
 
	// Expert
	client->resp.team = team;
	client->resp.toBeForwarded = forward;

	// FIXME : this locks id state on
	client->resp.id_state = 1;
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
		game.clients[i].pers.powerArmorActive = (ent->flags & FL_POWER_ARMOR);
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	if (ent->client->pers.powerArmorActive)
		ent->flags |= FL_POWER_ARMOR;
	if (coop->value)
		ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

// Expert CTF
float 	RangeToEnemyFlag(edict_t *spot, edict_t *player);

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

	// Expert: in FlagTrack, neither players nor flags spawn near flags
	// (flags use this spawn spot picking function)
	if (flagtrack->value) {
		for (n = 0; n < listSize(flags); n++) {
			player = listElementAt(flags, n);
			VectorSubtract (spot->s.origin, player->s.origin, v);
			playerdistance = VectorLength (v);
			if (playerdistance < bestplayerdistance) {
				bestplayerdistance = playerdistance;
			}
		}
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
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
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
//CTF
		if (ctf->value)
			spot = SelectCTFSpawnPoint(ent);
		else
//CTF
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

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
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

	if (self->health < -40)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;
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

	// Expert: player might be minlit or fullbright
	// or have a powerup effect.
	// Prevent body from picking up player's effects
	body->s.renderfx = 0;
	body->s.effects = 0;

	body->svflags = ent->svflags;
	VectorCopy (ent->mins, body->mins);
	VectorCopy (ent->maxs, body->maxs);
	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
	VectorCopy (ent->size, body->size);
	body->solid = SOLID_TRIGGER;//ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->movetype = ent->movetype;

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

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

		// Expert: prevent grappling on immediate respawn
		self->client->hook_frame = level.framenum + 1;

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

		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
		// Expert: NOTE : struct copy moved to here end since 
		// ClientUserinfoChanged now side-effects client->resp
		resp = client->resp;
	}
	else if (coop->value)
	{
		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < MAX_ITEMS; n++)
		{
			if (itemlist[n].flags & IT_KEY)
				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
		}
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged (ent, userinfo);
		if (resp.score > client->pers.score)
			client->pers.score = resp.score;
	}
	else
	{
		memset (&resp, 0, sizeof(resp));
	}

	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	ent->movetype = MOVETYPE_WALK;
	ent->solid = SOLID_BBOX;
	ent->clipmask = MASK_PLAYERSOLID;

	if ((int)sv_arenaflags->value & EXPERT_ENABLE_ARENA) {
		arenaSpawn(ent);
	}

	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
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

	// make sure prediction is on at spawn
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

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

	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
	// VWEP
	ShowGun(ent);
	//ent->s.modelindex2 = 255;		// custom gun model
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

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);

	// Expert FlagTrack
	// Spawn with no flag
	client->flagCarried = NOTEAM;

	// Expert: Print info line
	if ( !(utilflags & EXPERT_DISABLE_SPAWNMSGS) )
		DisplayRespawnLine(ent);
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

	InitClientResp (ent->client);

	gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

	// Expert: run all level change/initial connect logic
	ExpertPlayerLevelInits(ent);

	// locate ent at a spawn point
	PutClientInServer (ent);

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);

	// Expert: Make the player an observer if he's set spectator 1
	if (atoi(Info_ValueForKey(ent->client->pers.userinfo, "spectator")) != 0) {
		PlayerToObserver(ent);
		return;		// Don't send an effect
	}

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}


/*
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
	char	*s, *greenName;
	int	playernum = ent-g_edicts-1;

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);
	
	// Expert: Detect name change 
	if (strcmp(ent->client->pers.netname, ent->client->resp.oldname) != 0) {
		// notification on console
		if (strlen(ent->client->resp.oldname) > 0) {
			gi.cprintf(NULL, PRINT_HIGH, "%s changed name to %s\n", 
					ent->client->resp.oldname,
					ent->client->pers.netname);
			// log for gibstats
			gsPlayerNameChange(ent->client->resp.oldname, ent->client->pers.netname);
		}
		// Expert playerid: create a set of configstrings that 
		// are just name rather than name with skin
		greenName = va(s);
		// Limit to 12 characters for consistent HUD usage
		greenName[12] = 0;
		// and make it green
		gi.configstring (CS_PLAYERNAMES+playernum, greenText(greenName));
	}

	// Expert: Remember the new name to check for future name changes
	strncpy (ent->client->resp.oldname, s, sizeof(ent->client->resp.oldname)-1);

	// Expert: moved up for teamplay
	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	// Expert: If teamplay is enabled, make a best effort to match
	// the player's new skin and model settings from the skin and model
	// settings allowed for his team.
	if (teamplayEnabled() && expflags & EXPERT_ENFORCED_TEAMS) {
		// only bother with possible skin reassignment if player has attempted
		// to change his skin setting to a valid skin name (contains a /)
		if (playerIsOnATeam(ent) &&
			stricmp(ent->client->resp.oldskin, s) != 0) {
			enforceTeamModelSkin(ent);
		} 
	} else {
		// combine name and skin into a configstring
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );
	}

	// Expert: Remember the new skin that the client tried to set
	// to check for future skin changes
	strcpy (ent->client->resp.oldskin, s);

	// fov
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
	{
		ent->client->pers.hand = atoi(s);
	}

	// VWEP
	ShowGun(ent);
}

//Expert
void ClientDisconnect(edict_t *ent);

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
	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}

	// check for a password
	value = Info_ValueForKey (userinfo, "password");

	// Expert: 3.19 behavior - only check server password against
	// client's provided password if the server password is actually
	// set (not equal to "") and is not equal to "none".
	if (Q_stricmp(password->string, "none") && 
		Q_stricmp(password->string, "")) {
		if (strcmp(password->string, value) != 0)
			return false;
	}

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// Expert: reconnect detection
	if (ent->client->pers.connectState == CONNECTED) {
		gi.dprintf("Reconnect detected!  Calling disconnect.\n");
		ClientDisconnect(ent);
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// Expert Teamplay: set to invalid team,
		// only on initial connect
		ent->client->resp.team = NOTEAM;

		// clear the respawning variables
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);

		// Expert: player forwarding
		if (utilflags & EXPERT_ENABLE_PLAYER_FORWARDING &&
			ent - g_edicts == maxclients->value) 
		{
			gi.dprintf("Player marked for forwarding.\n");
			ent->client->resp.toBeForwarded = true;
		}
	}

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.bprintf (PRINT_HIGH, "%s connected\n", ent->client->pers.netname);

	ent->client->pers.connected = true;
	ent->client->pers.connectState = CONNECTED;
	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

	// Expert
	ExpertPlayerDisconnect(ent);

	gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

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
	ent->client->pers.connectState = DISCONNECTED;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
	// Expert: player names array for playerid
	gi.configstring (CS_PLAYERNAMES+playernum, "");
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

// Expert: Little debugging aid to display all angles
void displayAngles(edict_t *player, usercmd_t *ucmd)
{
	gi.dprintf("ucmd->angles:        %d %d %d\n"
			   "pmove->delta_angles: %d %d %d\n"
			   "v_angle:             %.1f %.1f %.1f\n",
			   ucmd->angles[0], ucmd->angles[1], ucmd->angles[1],
			   player->client->ps.pmove.delta_angles[0],
			   player->client->ps.pmove.delta_angles[1],
			   player->client->ps.pmove.delta_angles[2],
			   player->client->v_angle[0],
			   player->client->v_angle[1],
			   player->client->v_angle[2]);

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
	int		i, j;
	pmove_t	pm;

	level.current_entity = ent;
	client = ent->client;

	// Normal Quake2 clients don't use impulses unless
	// a client manually enters "impulse #", so this is
	// probably a bot control mechanism
	if (ucmd->impulse) {
		if (stricmp(botaction->string, "kick") == 0) {
			BootPlayer(ent, "Proxy bots not allowed.",
					"Appears to be using a bot");
		}
		// always log to console
		gi.dprintf("WARNING: Client %s appears to be using a bot; sent impulse %d\n"
					"IP is %s\n", ent->client->pers.netname, ucmd->impulse,
					Info_ValueForKey (ent->client->pers.userinfo, "ip"));
		// add bankick option
	}

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

	// Expert: Check if player needs to be pulled by hook
	if (client->on_hook == true)
		Pull_Grapple(ent);
	
	// Expert: do delayedInit 5 seconds after connect 
	if (!client->resp.delayedInit && 
	    level.framenum - client->resp.enterframe > 50) 
	{
		ExpertPlayerDelayedInits(ent);
	}	

	pm_passent = ent;

	// Expert: Observer fire toggles Chase Cam
	if (IsObserver(ent) && ucmd->buttons & BUTTON_ATTACK &&
		level.time - ent->client->lastChaseCommand > 0.3) 
	{
		ToggleChaseCam(ent);
		ent->client->lastChaseCommand = level.time;
		return;
	}

	// Expert: ChaseCam
	if (ent->client->chase_target) 
	{
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		// When observers using Chasecam press jump switch to next chase target
		if (ucmd->upmove >= 10 && level.time - ent->client->lastChaseCommand > 0.3) 
		{
			ChaseNext(ent);
			ent->client->lastChaseCommand = level.time;
		}
		return;
	}

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != 255)
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)
		client->ps.pmove.pm_type = PM_DEAD;
	else if ((int)sv_paused->value == 1)
		client->ps.pmove.pm_type = PM_FREEZE;
	else
		client->ps.pmove.pm_type = PM_NORMAL;

	// Expert: If the game is paused and the player is an observer, clamp the player's angles
	if ((int) sv_paused->value && ent->movetype == PM_FREEZE) {
		ucmd->angles[0] = client->old_angles[0];
		ucmd->angles[1] = client->old_angles[1];
		ucmd->angles[2] = client->old_angles[2];
	}

	client->old_angles[0] = ucmd->angles[0];
	client->old_angles[1] = ucmd->angles[1];
	client->old_angles[2] = ucmd->angles[2];

	// Expert: If the hook is in use, just ignore gravity
	if (client->on_hook)
		client->ps.pmove.gravity = 0;
	else
		client->ps.pmove.gravity = sv_gravity->value;

	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
	{
		pm.snapinitial = true;
//		gi.dprintf ("pmove changed!\n");
	}

	pm.cmd = *ucmd;

	pm.trace = PM_trace;	// adds default parms
	pm.pointcontents = gi.pointcontents;

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
	{
		// Expert: If using Expert powerups and the Mutant Jump....
		if (expflags & EXPERT_POWERUPS && ent->client->invincible_framenum > level.framenum)
		{
			// Make a long jump
			vec3_t forward;
			AngleVectors(ent->client->v_angle, forward, NULL, NULL);
			VectorMA(ent->velocity, 1000, forward, ent->velocity);

			gi.sound(ent, CHAN_VOICE, gi.soundindex("mutant/mutsght1.wav"), 1, ATTN_NORM, 0);
		} else {
			gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
		}
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
	}

	// Expert: PM_FREEZE has a bad viewheight (namely 0), so we need to use the viewheight
	// just before the pause. (talk about a hack!)
	if ((int)sv_paused->value != 1)
		ent->viewheight = pm.viewheight;
		
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;
	if (pm.groundentity)
		ent->groundentity_linkcount = pm.groundentity->linkcount;

	if (ent->deadflag)
	{
		client->ps.viewangles[ROLL] = 40;
		client->ps.viewangles[PITCH] = -15;
		client->ps.viewangles[YAW] = client->killer_yaw;
	}
	else
	{
		VectorCopy (pm.viewangles, client->v_angle);
		VectorCopy (pm.viewangles, client->ps.viewangles);
	}

	gi.linkentity (ent);

	// Expert: Stop other client stuff in a pause
	if (((int) sv_paused->value == 1) && ent->movetype != MOVETYPE_NOCLIP)
		return;

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
		other->touch (other, ent, NULL, NULL);
	}


	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// Expert: Change from observer to spectator or spectator to observer
	spectatorStateChange(ent);

	// Expert: observers are finished here
	if (IsObserver(ent)) {
		return;
	}

	// Expert: ChaseCam
	other = g_edicts + 1;
	for (i = 0; i < maxclients->value; i++) {
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
		other = other + 1;
	}
	
	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK &&
		ent->movetype != MOVETYPE_NOCLIP) {
		// players fire
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}
	
	// Expert Alternate restore
	// No health in level, regenerate instead
	if (expflags & EXPERT_ALTERNATE_RESTORE ||
		expflags & EXPERT_AMMO_REGEN) 
	{
		regen(ent);
	}

	// Expert Hook : Grapple command code
	// Expert Pogo : Pogo command code
	// Check to see if player pressing the "use" key
	if ((expflags & EXPERT_HOOK || expflags & EXPERT_POGO) && 
		ent->client->buttons & BUTTON_USE &&
	    !ent->deadflag && 
	    client->hook_frame <= level.framenum)
	{
		Throw_Grapple (ent);
	}
  
	// Expert: Check to see if the grapple key was released
	if (expflags & EXPERT_HOOK && Ended_Grappling (client) && 
		!ent->deadflag && client->hook)
	{
		Release_Grapple (client->hook);
	}
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

	// Expert: Stop other client stuff in a pause
	if (((int) sv_paused->value == 1) && ent->movetype != MOVETYPE_NOCLIP)
		return;
	
	// Check for bots and proxies
	if (!(utilflags & EXPERT_DISABLE_BOT_DETECT) && 
		ent->client->resp.delayedInit) // connected enough to receive stuffcmds, etc
	{
		ExpertBotDetect(ent);
	}

	client = ent->client;

	// Expert : spectator
	if (ent->movetype == MOVETYPE_NOCLIP) {
		client->latched_buttons = 0;
		return;
	}

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk && ent->movetype != MOVETYPE_NOCLIP)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			// in deathmatch, only wait for attack button
			if (deathmatch->value)
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;

			if ( ( client->latched_buttons & buttonMask ) ||
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ) )
			{
				respawn(ent);
				client->latched_buttons = 0;
			}
		}
		return;
	}

	// add player trail so monsters can follow
	if (!deathmatch->value)
		if (!visible (ent, PlayerTrail_LastSpot() ) )
			PlayerTrail_Add (ent->s.old_origin);

	client->latched_buttons = 0;
}
