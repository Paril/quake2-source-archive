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
			if ((!self->targetname) || Q_stricmp(self->targetname, spot->targetname) != 0)
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
	
	if(Q_stricmp(level.mapname, "security") == 0)
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
	if(Q_stricmp(level.mapname, "security") == 0)
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

	if((Q_stricmp(level.mapname, "jail2") == 0)   ||
	   (Q_stricmp(level.mapname, "jail4") == 0)   ||
	   (Q_stricmp(level.mapname, "mine1") == 0)   ||
	   (Q_stricmp(level.mapname, "mine2") == 0)   ||
	   (Q_stricmp(level.mapname, "mine3") == 0)   ||
	   (Q_stricmp(level.mapname, "mine4") == 0)   ||
	   (Q_stricmp(level.mapname, "lab") == 0)     ||
	   (Q_stricmp(level.mapname, "boss1") == 0)   ||
	   (Q_stricmp(level.mapname, "fact3") == 0)   ||
	   (Q_stricmp(level.mapname, "biggun") == 0)  ||
	   (Q_stricmp(level.mapname, "space") == 0)   ||
	   (Q_stricmp(level.mapname, "command") == 0) ||
	   (Q_stricmp(level.mapname, "power2") == 0) ||
	   (Q_stricmp(level.mapname, "strike") == 0))
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

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}

qboolean IsNeutral (edict_t *ent)
{
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
		return true;
	return false;
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

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
			case MOD_HELD_GRAVITY_GRENADE:
				message = "imploded";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsNeutral(self))
					message = "tripped on its own grenade";
				else if (IsFemale(self))
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_GRAVITY_GRENADE:
					message = "decided to be a part of a mini black hole";
				break;
			case MOD_R_SPLASH:
				if (IsNeutral(self))
					message = "blew itself up";
				else if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
			default:
				if (IsNeutral(self))
					message = "killed itself";
				else if (IsFemale(self))
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
			case MOD_RAILGUN_FRAG:
				message = "was shredded by";
				message2 = "'s railgun";
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
			case MOD_GRAVITY_GRENADE:
				message = "didn't see";
				message2 = "'s mini black hole generator";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_MACHINEGUN_EMP:
				message = "circuits fried";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
			case MOD_GRAPPLE:
				message = "was caught by";
				message2 = "'s grapple";
				break;
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
				return;
			}
		}
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
		self->client->resp.score--;
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
	if (! self->client->pers.inventory[self->client->ammo_index] && (strcmp(item->pickup_name, "Blaster") != 0))
		item = NULL;
	//if (item && (strcmp (item->pickup_name, "Blaster") == 0))
	//	item = NULL;

	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 10));

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
	}

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

	if (dir[0])
		self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
	else {
		self->client->killer_yaw = 0;
		if (dir[1] > 0)
			self->client->killer_yaw = 90;
		else if (dir[1] < 0)
			self->client->killer_yaw = -90;
	}
	if (self->client->killer_yaw < 0)
		self->client->killer_yaw += 360;
	

}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

//	self->solid = SOLID_NOT;
	//self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
		TossClientWeapon (self);
		if (deathmatch->value)
			Cmd_Help_f (self);		// show scores

		// clear inventory
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < game.num_items; n++)
		{
			if (coop->value && itemlist[n].flags & IT_KEY)
				self->client->resp.coop_respawn.inventory[n] = self->client->pers.inventory[n];
			self->client->pers.inventory[n] = 0;
		}
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;

	if (self->health < -40)
	{	// gib
		gib_target(self, damage, (GIB_PLAYER), point);	
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
			gi.sound(self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
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
	gitem_t		*item;

	memset (&client->pers, 0, sizeof(client->pers));

	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cgrenades = 50;
	client->pers.max_cells		= 250;
	client->pers.max_slugs		= 50;

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
		game.clients[i].pers.savedFlags = (ent->flags & (FL_GODMODE|FL_NOTARGET|FL_POWER_ARMOR));
		if (coop->value || !deathmatch->value)
		{
			game.clients[i].pers.score = ent->client->resp.score;
			game.clients[i].pers.score_dmg_received = ent->client->resp.score_dmg_received;
			game.clients[i].pers.score_dmg_dealt = ent->client->resp.score_dmg_dealt;
			game.clients[i].pers.score_dmg_saved = ent->client->resp.score_dmg_saved;
			game.clients[i].pers.score_kills = ent->client->resp.score_kills;
			game.clients[i].pers.score_item_pickup = ent->client->resp.score_item_pickup;
			game.clients[i].pers.score_item_usage = ent->client->resp.score_item_usage;
			game.clients[i].pers.score_health_bonus = ent->client->resp.score_health_bonus;
			game.clients[i].pers.score_objectives = ent->client->resp.score_objectives;
			game.clients[i].pers.score_secrets = ent->client->resp.score_secrets;
		}
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	ent->flags |= ent->client->pers.savedFlags;
	if (coop->value || !deathmatch->value)
	{
		ent->client->resp.score = ent->client->pers.score;
		ent->client->resp.score_dmg_received = ent->client->pers.score_dmg_received;
		ent->client->resp.score_dmg_dealt = ent->client->pers.score_dmg_dealt;
		ent->client->resp.score_dmg_saved = ent->client->pers.score_dmg_saved;
		ent->client->resp.score_kills = ent->client->pers.score_kills;
		ent->client->resp.score_item_pickup = ent->client->pers.score_item_pickup;
		ent->client->resp.score_item_usage = ent->client->pers.score_item_usage;
		ent->client->resp.score_health_bonus = ent->client->pers.score_health_bonus;
		ent->client->resp.score_objectives = ent->client->pers.score_objectives;
		ent->client->resp.score_secrets = ent->client->pers.score_secrets;
	}
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
		gib_target(self, damage, GIB_PLAYER, point);
		self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_YES;
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

	gi.linkentity (body);
}


void respawn (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{
		// spectator's don't leave bodies
		if (self->movetype != MOVETYPE_NOCLIP)
			CopyToBodyQue (self);
		self->svflags &= ~SVF_NOCLIENT;
		PutClientInServer (self);

		// add a teleportation effect
		self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		self->client->respawn_time = level.time;

		return;
	}

	// restart the entire server
	//gi.AddCommandString ("menu_loadgame\n");
	gi.AddCommandString("pushmenu loadgame\n");
}

/* 
 * only called when pers.spectator changes
 * note that resp.spectator should be the opposite of pers.spectator here
 */
void spectator_respawn (edict_t *ent)
{
	int i, numspec;

	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators

	if (ent->client->pers.spectator) {
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");
		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value)) {
			gi.cprintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");
			ent->client->pers.spectator = false;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);
			return;
		}

		// count spectators
		for (i = 1, numspec = 0; i <= maxclients->value; i++)
			if (g_edicts[i].inuse && g_edicts[i].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			gi.cprintf(ent, PRINT_HIGH, "Server spectator limit is full.");
			ent->client->pers.spectator = false;
			// reset his spectator var
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);
			return;
		}
	} else {
		// he was a spectator and wants to join the game
		// he must have the right password
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "password");
		if (*password->string && strcmp(password->string, "none") && 
			strcmp(password->string, value)) {
			gi.cprintf(ent, PRINT_HIGH, "Password incorrect.\n");
			ent->client->pers.spectator = true;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 1\n");
			gi.unicast(ent, true);
			return;
		}
	}

	// clear client on respawn
	ent->client->resp.score = ent->client->pers.score = 0;

	ent->svflags &= ~SVF_NOCLIENT;
	PutClientInServer (ent);

	// add a teleportation effect
	if (!ent->client->pers.spectator)  {
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
	}

	ent->client->respawn_time = level.time;

	if (ent->client->pers.spectator) 
		gi.bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
	else
		gi.bprintf (PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname);
}

//==============================================================
/*
void player_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	gi.bprintf(PRINT_HIGH, "%s touched %s\n", self->classname, other->classname);
	if (self->flags & FL_PICKUP)
	{
		if (other)

			 
		{


		}
	}
}
*/
/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void player_setup_think (edict_t *ent)
{

	if (!ent->noise_index2)
	{
	//	gi.bprintf(PRINT_HIGH, "STARTING PLAYER SETUP FUNCTION!\n");
		client_cmd(ent->owner, ";-speed;alias +pickup \"cmd +pic\";alias -pickup \"cmd -pic\"\n");
	}
	else if(ent->noise_index2 == 2)
		client_cmd(ent->owner, ";alias +attack2 \"cmd +a2\";alias -attack2 \"cmd -a2\"\n");
	if(ent->noise_index2 == 4)
		client_cmd(ent->owner, ";alias +dual \"cmd +dw\";alias -dual \"cmd -dw\"\n");
	else if (ent->noise_index2 == 6)
		client_cmd(ent->owner, ";alias +attack3 \"cmd +a3\";alias -attack3 \"cmd -a3\"\n");
	else if (ent->noise_index2 == 8)
		client_cmd(ent->owner, ";alias +jump \"cmd +j;+moveup\";alias -jump \"cmd -j;-moveup\"\n");
	else if (ent->noise_index2 == 10)
		client_cmd(ent->owner, ";alias +duck \"cmd +d;+movedown\";alias -duck \"cmd -d;-movedown\"\n");
	else if (ent->noise_index2 == 12)
		client_cmd(ent->owner, ";alias +grapple \"cmd +gh\";alias -grapple \"cmd -gh\"\n");
	else if (ent->noise_index2 == 14)
		client_cmd(ent->owner, "gl_partscale 2\n");
	else if (ent->noise_index2 == 16)
	{
		G_FreeEdict(ent);
		return;
	}
	//gi.bprintf(PRINT_HIGH, "SETUP THINK = %i\n", ent->noise_index2);
	ent->noise_index2++;
	ent->nextthink = level.time + 0.1;
	if (ent->delay < level.time)
		G_FreeEdict(ent);
}
void player_setup(edict_t *ent)
{
	edict_t	*player_setup_ent;
	player_setup_ent = G_Spawn();
	player_setup_ent->nextthink = level.time + 0.1;
	player_setup_ent->think = player_setup_think;
	player_setup_ent->owner = ent;
	player_setup_ent->delay = level.time + 3;
}
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
	// Knightmare- added fix to keep same player model
    char				userinfo[MAX_INFO_STRING];

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
	else if (coop->value)
	{
//		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
//		for (n = 0; n < game.num_items; n++)
//		{
//			if (itemlist[n].flags & IT_KEY)
//				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
//		}
		resp.coop_respawn.game_helpchanged = client->pers.game_helpchanged;
		resp.coop_respawn.helpchanged = client->pers.helpchanged;
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged (ent, userinfo);
		if (resp.score > client->pers.score)
			client->pers.score = resp.score;
	}
	else
	{
		memset (&resp, 0, sizeof(resp));
	}
	// Knightmare- added fix to keep same player model
	memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
	ClientUserinfoChanged (ent, userinfo);

	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;

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
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;		// custom gun model
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ent - g_edicts - 1;

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
	{
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);
	}

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	// spawn a spectator
	if (client->pers.spectator) {
		client->chase_target = NULL;

		client->resp.spectator = true;

		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);
		return;
	} else
		client->resp.spectator = false;

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);

	// Knightmare- added Paril's fix for this getting reset after map changes
	if (!ent->client->pers.connected)
		ent->client->pers.connected = true;

	player_setup(ent);
	ent->client->pers.jump_charge = jump_max_charge->value;
	ent->client->hudguy_d = 0.0;
	ent->client->hudguy_u = 0.0;

	
	give_item(ent, "Power Screen");

}

void give_ammo(edict_t *ent, char *ammoname, int ammount)
{
	int ix = ITEM_INDEX(FindItem(ammoname));
	ent->client->pers.inventory[ix] = 50;
}
void give_item(edict_t *ent, char *itemname)
{

	int ix;
	gitem_t		*it;

	ix = ITEM_INDEX(FindItem("Power Screen"));
	if (!ent->client->pers.inventory[ix])
	{
		give_ammo(ent, "Cells", 50);
		ent->client->pers.inventory[ix] = 1;
	}
	else
		return;
		

	it = &itemlist[ix];
	it->use(ent, it);
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

	// locate ent at a spawn point
	PutClientInServer (ent);

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);

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
	char	*s;
	int		playernum;

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	// set spectator
	s = Info_ValueForKey (userinfo, "spectator");
	// spectators are only supported in deathmatch
	if (deathmatch->value && *s && strcmp(s, "0"))
		ent->client->pers.spectator = true;
	else
		ent->client->pers.spectator = false;

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

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

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
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
	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}

	// check for a spectator
	value = Info_ValueForKey (userinfo, "spectator");
	if (deathmatch->value && *value && strcmp(value, "0")) {
		int i, numspec;

		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value)) {
			Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
			return false;
		}

		// count spectators
		for (i = numspec = 0; i < maxclients->value; i++)
			if (g_edicts[i+1].inuse && g_edicts[i+1].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			Info_SetValueForKey(userinfo, "rejmsg", "Server spectator limit is full.");
			return false;
		}
	} else {
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if (*password->string && strcmp(password->string, "none") && 
			strcmp(password->string, value)) {
			Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
			return false;
		}
	}


	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s connected\n", ent->client->pers.netname);

	ent->svflags = 0; // make sure we start with known default
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
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

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

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
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
void random_effect(edict_t *ent)
{
	int i = rand() % 30;
		switch (i)
		{
		case 0:
		{
			ent->s.effects = EF_ROTATE;
			gi.bprintf(PRINT_HIGH, "EF_ROTATE");
			break;
		}
		case 1:
		{
			ent->s.effects = EF_GIB;
			gi.bprintf(PRINT_HIGH, "EF_GIB");
			break;
		}
		case 2:
		{
			ent->s.effects = EF_BLASTER;
			gi.bprintf(PRINT_HIGH, "EF_BLASTER");
			break;
		}
		case 3:
		{
			ent->s.effects = EF_ROCKET;
			gi.bprintf(PRINT_HIGH, "EF_ROCKET");
			break;
		}
		case 4:
		{
			ent->s.effects = EF_GRENADE;
			gi.bprintf(PRINT_HIGH, "EF_GRENADE");
			break;
		}
		case 5:
		{
			ent->s.effects = EF_HYPERBLASTER;
			gi.bprintf(PRINT_HIGH, "EF_HYPERBLASTER");
			break;
		}
		case 6:
		{
			ent->s.effects = EF_BFG;
			gi.bprintf(PRINT_HIGH, "EF_BFG");
			break;
		}
		case 7:
		{
			ent->s.effects = EF_ROTATE;
			gi.bprintf(PRINT_HIGH, "EF_ROTATE");
			break;
		}
		case 8:
		{
			ent->s.effects = EF_COLOR_SHELL;
			gi.bprintf(PRINT_HIGH, "EF_COLOR_SHELL");
			break;
		}
		case 9:
		{
			ent->s.effects = EF_POWERSCREEN;
			gi.bprintf(PRINT_HIGH, "EF_POWERSCREEN");
			break;
		}
		case 10:
		{
			ent->s.effects = EF_ANIM01;
			gi.bprintf(PRINT_HIGH, "EF_ANIM01");
			break;
		}
		case 11:
		{
			ent->s.effects = EF_ANIM23;
			gi.bprintf(PRINT_HIGH, "EF_ANIM23");
			break;
		}
		case 12:
		{
			ent->s.effects = EF_ANIM_ALL;
			gi.bprintf(PRINT_HIGH, "EF_ANIM_ALL");
			break;
		}
		case 13:
		{
			ent->s.effects = EF_ANIM_ALLFAST;
			gi.bprintf(PRINT_HIGH, "EF_ANIM_ALLFAST");
			break;
		}
		case 14:
		{
			ent->s.effects = EF_FLIES;
			gi.bprintf(PRINT_HIGH, "EF_FLIES");
			break;
		}
		case 15:
		{
			ent->s.effects = EF_QUAD;
			gi.bprintf(PRINT_HIGH, "EF_QUAD");
			break;
		}
		case 16:
		{
			ent->s.effects = EF_PENT;
			gi.bprintf(PRINT_HIGH, "EF_PENT");
			break;
		}
		case 17:
		{
			ent->s.effects = EF_TELEPORTER;
			gi.bprintf(PRINT_HIGH, "EF_TELEPORTER");
			break;
		}
		case 18:
		{
			ent->s.effects = EF_FLAG1;
			gi.bprintf(PRINT_HIGH, "EF_FLAG1");
			break;
		}
		case 19:
		{
			ent->s.effects = EF_FLAG2;
			gi.bprintf(PRINT_HIGH, "EF_FLAG2");
			break;
		}
		case 20:
		{
			ent->s.effects = EF_IONRIPPER;
			gi.bprintf(PRINT_HIGH, "EF_IONRIPPER");
			break;
		}
		case 21:
		{
			ent->s.effects = EF_GREENGIB;
			gi.bprintf(PRINT_HIGH, "EF_GREENGIB");
			break;
		}
		case 22:
		{
			ent->s.effects = EF_BLUEHYPERBLASTER;
			gi.bprintf(PRINT_HIGH, "EF_BLUEHYPERBLASTER");
			break;
		}
		case 23:
		{
			ent->s.effects = EF_SPINNINGLIGHTS;
			gi.bprintf(PRINT_HIGH, "EF_SPINNINGLIGHTS");
			break;
		}
		case 24:
		{
			ent->s.effects = EF_PLASMA;
			gi.bprintf(PRINT_HIGH, "EF_PLASMA");
			break;
		}
		case 25:
		{
			ent->s.effects = EF_TRAP;
			gi.bprintf(PRINT_HIGH, "EF_TRAP");
			break;
		}
		case 26:
		{
			ent->s.effects = EF_TRACKER;
			gi.bprintf(PRINT_HIGH, "EF_TRACKER");
			break;
		}
		case 27:
		{
			ent->s.effects = EF_DOUBLE;
			gi.bprintf(PRINT_HIGH, "EF_DOUBLE");
			break;
		}
		case 28:
		{
			ent->s.effects = EF_TAGTRAIL;
			gi.bprintf(PRINT_HIGH, "EF_TAGTRAIL");
			break;
		}
		case 29:
		{
			ent->s.effects = EF_SPHERETRANS;
			gi.bprintf(PRINT_HIGH, "EF_SPHERETRANS");
			break;
		}
		case 30:
		{
			ent->s.effects = EF_TAGTRAIL;
			gi.bprintf(PRINT_HIGH, "EF_TAGTRAIL");
			break;
		}
		case 31:
		{
			ent->s.effects = EF_HALF_DAMAGE;
			gi.bprintf(PRINT_HIGH, "EF_HALF_DAMAGE");
			break;
		}
		case 32:
		{
			ent->s.effects = EF_TRACKERTRAIL;
			gi.bprintf(PRINT_HIGH, "EF_TRACKERTRAIL");
			break;
		}
		}

}
void 	pickup_check(edict_t *ent)
{
	trace_t		tr;
	vec3_t		forward, right, up, angles, start, offset;
	vec3_t		v;
	vec3_t		point;
	float		range;
	vec3_t		dir;

	VectorAdd(ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors(angles, forward, right, NULL);
	VectorSet(offset, -8, 8, ent->viewheight - 16);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, KICK_LEG_DISTANCE, forward, forward);
	tr = gi.trace(start, NULL, NULL, forward, ent, MASK_SHOT);

	if (tr.fraction < 1)
	{
		//gi.bprintf(PRINT_HIGH, "picking up %s", tr.ent->classname);
		//random_effect(tr.ent);
		//return;
		if (!tr.ent->pickup_master && ent->client->pers.pickup == PICKUP_ATTEMPT)
		{
			ent->client->pers.pickup = PICKUP_PICKINGUPSTART;

		}
		else if (!tr.ent->pickup_master && ent->client->pers.pickup == PICKUP_PICKINGUPLAST && ( strncmp(tr.ent->classname, "monster_soldier", 15) == 0  || strncmp(tr.ent->classname, "misc_explobox", 13) == 0))
		{
			//gi.bprintf(PRINT_HIGH, "PICKUP CHECK, state = %i\n", ent->client->pers.pickup);
			
			gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
			//gi.bprintf(PRINT_HIGH, "%s touched %s", self->classname, other->classname);
			//gi.bprintf(PRINT_HIGH, "PICKED UP1, view model %s view modelb %s\n", other->client->newweapon->view_model, other->client->pers.weapon->view_model);

			ent->client->pers.lastweapon = ent->client->pers.weapon;
			//gi.bprintf(PRINT_HIGH, "PICKED UP2, view model %s view modelb %s\n", other->client->newweapon->view_model, other->client->pers.weapon->view_model);

			//strcpy(other->client->pers.weapon->view_model, other->client->pers.weapon->view_modelb);
			if (strncmp(tr.ent->classname, "monster", 7) == 0)
			{

				ent->style = PICKUP_OBJECT_GUARD;
				vmodel_backup = "models/weapons/v_pguard/tris.md2";
				gi.sound(tr.ent, CHAN_BODY, gi.soundindex("misc/neck1.wav"), 1, ATTN_NORM, 0);

				int n = 1 + rand() % 3;

				gi.sound(tr.ent, CHAN_AUTO, gi.soundindex(va("soldier/SOLPAIN%i.wav", n)), 1, ATTN_NORM, 0);
			}
			else
			{
				//gi.bprintf(PRINT_HIGH, "%s touched %s", self->classname, other->classname);

				ent->style = PICKUP_OBJECT_BARREL;
				vmodel_backup = "models/weapons/v_pbarrel/tris.md2";
			}
			//other->client->pers.weapon->view_model = vmodel_backup;


			ent->client->ps.gunindex = gi.modelindex(vmodel_backup);
			ent->client->pers.pickup = PICKUP_PICKEDUP;
			//gi.bprintf(PRINT_HIGH, "PICKED UP3, view model %s view modelb %s\n", other->client->newweapon->view_model, other->client->pers.weapon->view_model);
			ent->gravity = 1.5;
			G_FreeEdict(tr.ent);
			ent->client->ps.gunframe = 0;
			return;
		}
	}
}
void count_current_entities()
{
	int			i;
	edict_t		*e;
	int ents_inuse = 0;
	e = &g_edicts[(int)maxclients->value + 1];
	for (i = maxclients->value + 1; i < globals.num_edicts; i++, e++)
	{
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (e->inuse)
			ents_inuse++;
	}
	//gi.bprintf(PRINT_HIGH, "CURRENT NUMBER OF ENTITIES == %i, game.maxentities = %i, globals.num_edicts = %i \n", ents_inuse, game.maxentities, globals.num_edicts);
}
void grapple_func(edict_t *ent)
{
	if (ent->client->buttonsx & BUTTON_GRAPPLE || ent->client->ctf_grapplestate)
	{
		if (ent->client->buttonsx & BUTTON_GRAPPLE)
		{
			if (ent->client->ctf_grapplestate <= CTF_GRAPPLE_STATE_FLY)
			{
				//gi.bprintf(PRINT_HIGH, "CHECKING IF GRAPPLE ENTITY EXISTS! grapple = %i\n", ent->client->ctf_grapple);
				if (!ent->client->grapple)
				{
					CTFGrappleFire(ent, vec3_origin, 10, 0);
					//gi.bprintf(PRINT_HIGH, "DIDNT EXIST, FIRE!\n");
				}


				return;
			}
			else if (ent->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL)
			{

			}

		}
		else if (ent->client->ctf_grapplestate == CTF_GRAPPLE_STATE_WIND)
		{


		}
		else if (ent->client->ctf_grapplestate == CTF_GRAPPLE_STATE_DEACTIVATING)
		{
			if (ent->client->ps.gunframe == FRAME_DL_GRAPPLE)
			{
				
				return;
			}

		}
		else if (ent->client->grapple)
		{


			ent->client->grapple->solid = SOLID_BBOX;
			ent->client->grapple->owner = NULL;
			ent->client->grapple->owner_solid = ent;
			//ent->client->grapple->think = CTFGrappleDrawCable;
			VectorClear(ent->client->grapple->velocity);
			ent->client->ctf_grapplestate = CTF_GRAPPLE_STATE_WIND;
			ent->client->ctf_grapplewindtimeout = level.time + GRAPPLE_WIND_TIMEOUT;
			gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/grapple/grreset.wav"), 1.0, ATTN_IDLE, 0);

			return;
		}
		else if (ent->client->ctf_grapplestate >= CTF_GRAPPLE_STATE_ACTIVATING)
		{
			ent->client->ctf_grapplestate = CTF_GRAPPLE_STATE_DEACTIVATING;
			return;
		}

		return;
	}
}
void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;
	level.current_entity = ent;
	client = ent->client;


	//gi.bprintf(PRINT_HIGH, "origin = %s, old origin = %s, client think at %f\n", vtos(ent->s.origin), vtos(ent->s.old_origin), level.time);
	//-scanner(ent);
	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 20.0
			&& (ucmd->buttons & BUTTON_ANY))
		{
			edict_t *player;
			for (i = 0; i < maxclients->value; i++)
			{
				player = g_edicts + 1 + i;
				if (!ent->inuse)
					continue;
				player->client->resp.score_dmg_received = 0;
				player->client->resp.score_dmg_dealt = 0;
				player->client->resp.score_dmg_saved = 0;
				player->client->resp.score_kills = 0;
				player->client->resp.score_item_pickup = 0;
				player->client->resp.score_item_usage = 0;
				player->client->resp.score_health_bonus = 0;
				player->client->resp.score_objectives = 0;
				player->client->resp.score_secrets = 0;
			}
			level.exitintermission = true;
		}
		
		return;
	}
	if ( ent->client->pers.pickup)
		pickup_check(ent);
	pm_passent = ent;

	if (ent->client->chase_target) {

		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	} else {

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


		//gi.bprintf(PRINT_HIGH, "JUMP CHARGE IS %i, pm.cmd.upmove = %i\n", ent->client->pers.jump_charge, pm.cmd.upmove);
		                   
		for (i=0 ; i<3 ; i++)
		{
			ent->s.origin[i] = pm.s.origin[i]*0.125;
			ent->velocity[i] = pm.s.velocity[i]*0.125;
		}
		//if (!VectorCompare(ent->s.origin, ent->s.old_origin))
		//	VectorCopy(ent->s.origin, ent->s.old_origin);
		VectorCopy (pm.mins, ent->mins);
		VectorCopy (pm.maxs, ent->maxs);

		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		
		if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
			ent->client->v_dmg_smooth[0] += crandom() * 10;
			ent->client->v_dmg_smooth[1] += crandom() * 2.5;
			ent->client->v_dmg_smooth[2] += crandom() * 2.5;
		}


		if (ent->groundentity &&ent->client->buttonsx & BUTTON_JUMP && ent->client->buttonsx & BUTTON_DUCK && ent->client->pers.jump_charge == jump_max_charge->value)
		{
			vec3_t forward, right;

			AngleVectors(ent->client->v_angle, forward, right, NULL);
			VectorScale(forward, pm.cmd.forwardmove*1.5, forward);
			VectorScale(right, pm.cmd.sidemove / 2, right);
			ent->velocity[0] += forward[0];
			ent->velocity[1] += forward[1];
			ent->velocity[0] += right[0];
			ent->velocity[1] += right[1];
			ent->velocity[2] += 450;
			ent->client->v_dmg_smooth[PITCH] -= 50;
			ent->client->v_dmg_time = level.time + 0.2;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
			gi.sound(ent, CHAN_AUTO, gi.soundindex("player/jump_a.wav"), 1, ATTN_IDLE, 0);

			PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
			ent->client->pers.jump_charge = 0;
		}
			

		if (ent->client->pers.jump_charge < jump_max_charge->value)
			ent->client->pers.jump_charge++;
		//gi.bprintf(PRINT_HIGH, "jump_charge = %i\n", ent->client->pers.jump_charge);
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

	}

	client->oldbuttons = client->buttons;
	//int add_attack, add_dual;

	//if (client->buttons & BUTTON_ATTACK2)
	//	ucmd->buttons |= BUTTON_ATTACK2;

	//if (client->buttons & BUTTON_ATTACK3)
	//	ucmd->buttons |= BUTTON_ATTACK3;

	//if (client->buttons & BUTTON_DUAL)
	//	ucmd->buttons |= BUTTON_DUAL;



	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK)
	{
		if (client->resp.spectator) {

			client->latched_buttons = 0;

			if (client->chase_target) {
				client->chase_target = NULL;
				client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			} else
				GetChaseTarget(ent);

		} else if (!client->weapon_thunk) {
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

	if (client->resp.spectator) {
		if (ucmd->upmove >= 10) {
			if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD)) {
				client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
				if (client->chase_target)
					ChaseNext(ent);
				else
					GetChaseTarget(ent);
			}
		} else
			client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
	}

	// update chase cam if being followed
	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}

	grapple_func(ent);


	//if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		/*if (ent->health > 0 && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
		{


			ent->viewheight = pm.viewheight - 2;

			//gi.bprintf(PRINT_HIGH, "viewheight= %f", pm.viewheight);
		}
		else if (ent->health > 0 && ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->viewheight = pm.viewheight + 7;
		}
		else
			ent->viewheight = pm.viewheight;*/

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

	//VectorCopy(ent->s.origin, ent->s.old_origin);

	client = ent->client;
	if (level.framenum & 4)
		count_current_entities();
	if (deathmatch->value &&
		client->pers.spectator != client->resp.spectator &&
		(level.time - client->respawn_time) >= 5) {
		spectator_respawn(ent);
		return;
	}

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk && !client->resp.spectator)
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

