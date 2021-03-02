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
#include "g_tdm.h"

void ClientUserinfoChanged (edict_t *ent, char *userinfo);

void SP_misc_teleporter_dest (edict_t *ent);

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
	SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
	G_FreeEdict (self);
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(edict_t *ent)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
	if (!ent->client)
		return false;

	//ignore userinfo
	/*info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] == 'f' || info[0] == 'F')
		return true;*/

	//ugly skin checks instead :(
	if (!Q_strncasecmp (teaminfo[ent->client->pers.team].skin, "female/", 7) || !Q_strncasecmp (teaminfo[ent->client->pers.team].skin, "crakhor/", 8))
		return true;

	return false;
}

qboolean IsNeutral (edict_t *ent)
{
	if (!ent->client)
		return false;

	//ignore userinfo
	/*info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
		return true;*/

	//ugly skin checks instead :(
	if (!Q_strncasecmp (teaminfo[ent->client->pers.team].skin, "male/", 5) || !Q_strncasecmp (teaminfo[ent->client->pers.team].skin, "cyborg/", 7) ||
		!Q_strncasecmp (teaminfo[ent->client->pers.team].skin, "female/", 7) || !Q_strncasecmp (teaminfo[ent->client->pers.team].skin, "crakhor/", 8))
		return false;

	return true;
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	ff = meansOfDeath & MOD_FRIENDLY_FIRE;
	mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;

	TDM_Killed (attacker, self, mod);

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
			if (IsNeutral(self))
				message = "tripped on its own grenade";
			else if (IsFemale(self))
				message = "tripped on her own grenade";
			else
				message = "tripped on his own grenade";
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

		//take note of this, if we touch teamplayerinfo < MM_PLAYING we crash!
		if (tdm_match_status >= MM_PLAYING && tdm_match_status != MM_SCOREBOARD)
		{
			self->client->resp.score--;
			teaminfo[self->client->pers.team].score--;
			self->client->resp.teamplayerinfo->deaths++;
			self->client->resp.teamplayerinfo->suicides++;
		}
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
		}
		if (message)
		{
			static char tktext[] = " ** team kill **";
			static char	*tkmessage = NULL;

			if (!tkmessage)
				tkmessage = TDM_SetColorText (tktext);

			gi.bprintf (PRINT_MEDIUM, "%s %s %s%s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2, ff ? tkmessage : "");

			//take note of this, if we touch teamplayerinfo < MM_PLAYING we crash!
			if (tdm_match_status >= MM_PLAYING && tdm_match_status != MM_SCOREBOARD)
			{
				if (ff)
				{
					attacker->client->resp.score--;
					teaminfo[attacker->client->pers.team].score--;

					self->client->resp.teamplayerinfo->deaths++;
					attacker->client->resp.teamplayerinfo->team_kills++;
				}
				else
				{
					attacker->client->resp.score++;
					teaminfo[attacker->client->pers.team].score++;

					self->client->resp.teamplayerinfo->deaths++;
					attacker->client->resp.teamplayerinfo->enemy_kills++;
				}

				if (mod == MOD_TELEFRAG)
					attacker->client->resp.teamplayerinfo->telefrags++;
			}
		}
		return;
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);

	//take note of this, if we touch teamplayerinfo < MM_PLAYING we crash!
	if (tdm_match_status >= MM_PLAYING && tdm_match_status != MM_SCOREBOARD)
	{
		self->client->resp.score--;
		teaminfo[self->client->pers.team].score--;
		self->client->resp.teamplayerinfo->deaths++;
	}
}


void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	const gitem_t		*item;
	edict_t			*drop;
	qboolean		quad;
	float			spread;

	//no drops in warmup / itdm
	if (tdm_match_status < MM_PLAYING || tdm_match_status == MM_SCOREBOARD)
		return;

	if ((int)g_gamemode->value == GAMEMODE_ITDM)
		return;

	item = self->client->weapon;
	if (!self->client->inventory[self->client->ammo_index] )
		item = NULL;

	if (ITEM_INDEX(item) == ITEM_WEAPON_BLASTER)
		item = NULL;

	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 1 * (1 * SERVER_FPS)));

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
		drop = Drop_Item (self, GETITEM(ITEM_ITEM_QUAD));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.framenum + (self->client->quad_framenum - level.framenum);
		drop->think = G_FreeEdict;
	}
}


/*
==================
LookAtKiller
==================
wision: maybe make it active until client spawns?
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
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_framenum = level.time + g_respawn_time->value * (1 * SERVER_FPS);
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);

		self->client->pers.last_weapon = self->client->weapon;

		TossClientWeapon (self);

		//r1: dont run cmd_help, since that putaways menus, explicitly show scores
		if (!self->client->pers.menu.active)
		{
			self->client->showmotd = false;
			self->client->showoldscores = false;
			self->client->showscores = true;
			DeathmatchScoreboard (self);
		}

		//Cmd_Help_f (self);		// show scores

		// clear inventory
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < game.num_items; n++)
		{
			self->client->inventory[n] = 0;
		}

		//explicit set to avoid using selectnext if we have a menu up
		self->client->selected_item = -1;

		// update all spectating observers using SPEC_KILLER
		TDM_UpdateSpectatorsOnEvent (SPEC_KILLER, self, attacker);
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;

	if (self->health < -40)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead (self, damage);

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
			gi.sound (self, CHAN_VOICE, soundcache[SND_DEATH1 + genrand_int32() % 4], 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;

	gi.linkentity (self);
}

//=======================================================================



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t	*spot;
	int		selection;

	selection = genrand_int32() % level.numspawns;
	spot = level.spawns[selection];
	return spot;
}

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (edict_t *spot, edict_t **closest_player)
{
	edict_t	*player, *bestplayer;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;

	bestplayerdistance = 9999999;
	bestplayer = NULL;

	for (n = 1; n <= game.maxclients; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		if (!player->client->pers.team)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
		{
			bestplayer = player;
			bestplayerdistance = playerdistance;
		}
	}

	if (closest_player)
		*closest_player = bestplayer;

	return bestplayerdistance;
}

void RandomizeArray (void **base, size_t n)
{
	if (n > 1)
	{
		size_t i;
		for (i = 0; i < n - 1; i++)
		{
			size_t j = i + genrand_int32() / (0xffffffff / (n - i) + 1);
			void *t = base[j];
			base[j] = base[i];
			base[i] = t;
		}
	}
}


/*
================
SelectRandomDeathmatchSpawnPointAvoidingTelefrag

go to a random point, but NOT points with other players
on them
================
*/
edict_t *SelectRandomDeathmatchSpawnPointAvoidingTelefrag (edict_t *player)
{
	edict_t				*spawnlist[TDM_MAX_MAP_SPAWNPOINTS];

	//nasty! but we only need origin and angles.
	static edict_t		fake_spawn;

	//hard coded since the player may be spawning from spec and has no inherent size
	static vec3_t	mins = {-16, -16, -24};
	static vec3_t	maxs = {16, 16, 32};

	int				i;
	edict_t			*spawn;
	edict_t			*occupier;

	memcpy (spawnlist, level.spawns, sizeof(spawnlist));

	RandomizeArray ((void *)spawnlist, level.numspawns);

	//all spots could be taken, so don't while(1)
	for (i = 0; i < level.numspawns; i++)
	{
		spawn = spawnlist[i];

		//64 should be safe enough...
		if (PlayersRangeFromSpot (spawn, NULL) < 64)
			continue;

		return spawn;
	}

	gi.dprintf ("SelectRandomDeathmatchSpawnPointAvoidingTelefrag: All spawn points occupied, doing team search.\n");

	//all spots were taken, so go again, this time looking for spawns taken by teammates
	//and check if we can spawn next to them
	for (i = 0; i < level.numspawns; i++)
	{
		spawn = spawnlist[i];

		//64 should be safe enough...
		if (PlayersRangeFromSpot (spawn, &occupier) < 64)
		{
			int			j;
			trace_t		tr;
			vec3_t		test;
			vec3_t		forward, right, start;

			//its occupied, is it a teammate?
			if (occupier->client->pers.team != player->client->pers.team)
				continue;

			gi.dprintf ("SelectRandomDeathmatchSpawnPointAvoidingTelefrag: Trying to spawn %s next to %s\n", player->client->pers.netname, occupier->client->pers.netname);

			AngleVectors (spawn->s.angles, forward, right, NULL);

			//is there any room?
			for (j = 0; j < 4; j++)
			{
				VectorCopy (spawn->s.origin, start);
				start[2] += player->viewheight;

				//try in this order: left, right, back, forward by 80 units
				if (j == 0)
					VectorMA (start, -80, right, test);
				else if (j == 1)
					VectorMA (start, 80, right, test);
				else if (j == 2)
					VectorMA (start, -80, forward, test);
				else if (j == 3)
					VectorMA (start, 80, forward, test);

				//check the player fits and there is clearance to the original spawn
				//unlink the client who is already there first!
				gi.unlinkentity (occupier);
				tr = gi.trace (start, mins, maxs, test, spawn, MASK_PLAYERSOLID);
				gi.linkentity (occupier);

				if (!tr.allsolid && !tr.startsolid && tr.fraction == 1.0f)
				{
					gi.dprintf ("SelectRandomDeathmatchSpawnPointAvoidingTelefrag: Using fake spawn for %s\n", player->client->pers.netname);
					VectorCopy (test, fake_spawn.s.origin);
					VectorCopy (spawn->s.angles, fake_spawn.s.angles);
					return &fake_spawn;
				}
			}

			gi.dprintf ("SelectRandomDeathmatchSpawnPointAvoidingTelefrag: Failed to find space next to %s for %s\n", occupier->client->pers.netname, player->client->pers.netname);
			
			continue;
		}

		gi.dprintf ("SelectRandomDeathmatchSpawnPointAvoidingTelefrag: BUG: Spawn that was previously marked occupied became available!\n");
		return spawn;
	}

	gi.dprintf ("SelectRandomDeathmatchSpawnPointAvoidingTelefrag: Failed to find ANY suitable spawn point, using random!!\n");

	//oh well, whatever works..
	return SelectRandomDeathmatchSpawnPoint ();
}

/*
================
SelectRandomDeathmatchSpawnPointAvoidingTwoClosest

go to a random point, but NOT the two points closest
to other players
================
wision: this one ruins the game on small maps (i.e. with 4 spawn points)
*/
edict_t *SelectRandomDeathmatchSpawnPointAvoidingTwoClosest (void)
{
	edict_t	*spot, *spot1, *spot2;
	edict_t	*player, *player1, *player2;
	int		selection;
	int		i;
	float	range, range1, range2;
	
	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;
	player1 = player2 = NULL;

	for (i = 0; i < level.numspawns; i++)
	{
		spot = level.spawns[i];

		range = PlayersRangeFromSpot(spot, &player);
		if (range < range1)
		{
			player1 = player;
			range1 = range;
			spot1 = spot;
		}
	}

	//r1ch: we require two loops, or the ordering of the spawns in the level will skew the outcome
	for (i = 0; i < level.numspawns; i++)
	{
		spot = level.spawns[i];

		//already recorded this one
		if (spot == spot1)
			continue;

		range = PlayersRangeFromSpot(spot, &player);
		if (range < range2)
		{
			player2 = player;
			range2 = range;
			spot2 = spot;
		}
	}

	if (g_debug_spawns->value)
	{
		if (spot1)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_DEBUGTRAIL);
			gi.WritePosition (spot1->s.origin);
			gi.WritePosition (player1->s.origin);
			gi.multicast (NULL, MULTICAST_ALL);
		}

		if (spot2)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_DEBUGTRAIL);
			gi.WritePosition (spot2->s.origin);
			gi.WritePosition (player2->s.origin);
			gi.multicast (NULL, MULTICAST_ALL);
		}
	}

	do
	{
		selection = genrand_int32() % level.numspawns;
		spot = level.spawns[selection];
	} while (spot == spot1 || spot == spot2);

	return spot;
}


/*
================
SelectRandomDeathmatchSpawnPointAvoidingTwoClosestBugged

go to a random point, but NOT the two points closest
to other players, buggy version
================
wision: this one ruins the game on small maps (i.e. with 4 spawn points)
*/
edict_t *SelectRandomDeathmatchSpawnPointAvoidingTwoClosestBugged (void)
{
	edict_t	*spot, *spot1, *spot2;
	edict_t	*player, *player1, *player2;
	int		selection;
	int		i;
	float	range, range1, range2;
	
	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;
	player1 = player2 = NULL;

	for (i = 0; i < level.numspawns; i++)
	{
		spot = level.spawns[i];

		range = PlayersRangeFromSpot(spot, &player);
		if (range < range1)
		{
			player1 = player;
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			player2 = player;
			range2 = range;
			spot2 = spot;
		}

	}

	if (g_debug_spawns->value)
	{
		if (spot1)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_DEBUGTRAIL);
			gi.WritePosition (spot1->s.origin);
			gi.WritePosition (player1->s.origin);
			gi.multicast (NULL, MULTICAST_ALL);
		}

		if (spot2)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_DEBUGTRAIL);
			gi.WritePosition (spot2->s.origin);
			gi.WritePosition (player2->s.origin);
			gi.multicast (NULL, MULTICAST_ALL);
		}
	}

	do
	{
		selection = genrand_int32() % level.numspawns;
		spot = level.spawns[selection];
	} while (spot == spot1 || spot == spot2);

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
		bestplayerdistance = PlayersRangeFromSpot (spot, NULL);

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

edict_t *SelectDeathmatchSpawnPoint (edict_t *player)
{
	//in the first 1 second of a match start, or the first 5 seconds of warmup, avoid telefrags above
	//all other conditions
	if ((tdm_match_status >= MM_PLAYING && level.framenum - level.match_start_framenum < SECS_TO_FRAMES (1)) ||
		(tdm_match_status == MM_WARMUP && level.framenum - level.warmup_start_framenum < SECS_TO_FRAMES (5)))
	{
		return SelectRandomDeathmatchSpawnPointAvoidingTelefrag (player);
	}

	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
	{
		return SelectFarthestDeathmatchSpawnPoint ();
	}
	else if (level.numspawns > 2)
	{
		if (TDM_Is1V1())
		{
			int	mode;

			mode = (int)g_1v1_spawn_mode->value & ~SPAWN_RANDOM_ON_SMALL_MAPS;

			if (level.numspawns < 6 && (int)g_1v1_spawn_mode->value & SPAWN_RANDOM_ON_SMALL_MAPS)
				return SelectRandomDeathmatchSpawnPoint ();

			if (mode == 0)
				return SelectRandomDeathmatchSpawnPointAvoidingTwoClosestBugged ();
			else if (mode == 1)
				return SelectRandomDeathmatchSpawnPointAvoidingTwoClosest ();
		}
		else
		{
			if (g_tdm_spawn_mode->value == 0)
				return SelectRandomDeathmatchSpawnPointAvoidingTwoClosestBugged ();
			else if (g_tdm_spawn_mode->value == 1)
				return SelectRandomDeathmatchSpawnPointAvoidingTwoClosest ();
			else if (g_tdm_spawn_mode->value == 2)
				return SelectRandomDeathmatchSpawnPoint ();
		}
	}

	return SelectRandomDeathmatchSpawnPoint ();
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

	if (level.numspawns && ent->client->pers.team)
		spot = SelectDeathmatchSpawnPoint (ent);

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!spot->targetname)
				break;
		}

		if (!spot)
		{
			// there wasn't a spawnpoint without a target, so use any
			spot = G_Find (spot, FOFS(classname), "info_player_start");

			if (!spot)
			{
				gi.dprintf ("WARNING: No info_player_start, using world");
				spot = world;
			}
		}
	}

	VectorCopy (spot->s.origin, origin);
	//origin[2] += 9;
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
		ent->enttype = ENT_BODYQUE;
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
	body = &g_edicts[game.maxclients + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// send an effect on the removed body
	if (body->s.modelindex)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLOOD);
		gi.WritePosition (body->s.origin);
		gi.WriteDir (vec3_origin);
		gi.multicast (body->s.origin, MULTICAST_PVS);
	}
	gi.unlinkentity (ent);

	gi.unlinkentity (body);

	body->s = ent->s;
	body->s.number = body - g_edicts;
	body->s.event = EV_OTHER_TELEPORT;

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
	body->groundentity = ent->groundentity;

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

	gi.linkentity (body);
}


void respawn (edict_t *self)
{
	// remove spectator mode
	if (self->client->chase_target)
		DisableChaseCam (self);

	// spectator's don't leave bodies, r1: neither do unlinked entities
	if (self->movetype != MOVETYPE_NOCLIP && self->movetype != MOVETYPE_WALK && self->area.prev)
		CopyToBodyQue (self);

	self->svflags &= ~SVF_NOCLIENT;

	PutClientInServer (self);

	// add a teleportation effect
	if (!(self->svflags & SVF_NOCLIENT))
	{
		self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;
	}

	self->client->respawn_framenum = level.time;
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
	client_respawn_t	respsaved;
	qboolean		rejoined;

	//char		userinfo[MAX_INFO_STRING];

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	//strcpy (userinfo, client->pers.userinfo);
	//InitClientPersistant (client);
	//ClientUserinfoChanged (ent, userinfo);

	// clear everything but the persistant data
	saved = client->pers;
	respsaved = client->resp;
	memset (client, 0, sizeof(*client));
	client->resp = respsaved;
	client->pers = saved;

	client->clientNum = ent - g_edicts - 1;

	// copy some data from the client to the entity
	//FetchClientEntData (ent);

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
	ent->air_finished = level.time + 12 * (1 * SERVER_FPS);
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags = 0;
	ent->health = 100;
	ent->max_health = 100;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	if (ent->client->pers.team)
		TDM_SetInitialItems (ent);

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

	if ((int)dmflags->value & DF_FIXED_FOV)
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

	if (client->weapon)
		client->ps.gunindex = gi.modelindex(client->weapon->view_model);

	// clear entity state values
	ent->s.sound = 0;
	ent->s.effects = 0;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;		// custom gun model
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ent - g_edicts - 1;

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground



	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	rejoined = false;

	ent->client->last_activity_frame = level.framenum;

	if (ent->client->pers.joinstate == JS_FIRST_JOIN)
	{
		ent->client->pers.joinstate = JS_JOINED;
		rejoined = TDM_SetupClient (ent);
	}

	// spawn a spectator
	if (!client->pers.team)
	{
		client->chase_target = NULL;

		ent->health = 0;
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);

		// wision: force pov if there's speclock enabled somewhere
		if ((teaminfo[TEAM_A].speclocked || teaminfo[TEAM_B].speclocked) && !ent->client->pers.admin)
			DisableChaseCam (ent);

		// wision: move to intermission if it's end of the match
		if (tdm_match_status == MM_SCOREBOARD)
			MoveClientToIntermission (ent);

		return;
	}

	// force the current weapon up, or just set vwep info if we respawned a full client already,
	// although we shouldn't need to since the info should be preserved. we don't killbox if the
	// client rejoined since they should already be in a valid position, and the killbox extends
	// by +- 1 unit, so it could theoretically kill the other player if they are touching :)
	if (!rejoined)
	{
		trace_t			tr;
		vec3_t			temp, temp2;

		//try to properly clip to the floor / spawn
		VectorCopy (ent->s.origin, temp);
		VectorCopy (ent->s.origin, temp2);
		temp[2] -= 64;
		temp2[2] += 16;
		tr = gi.trace (temp2, ent->mins, ent->maxs, temp, ent, MASK_PLAYERSOLID);
		if (!tr.allsolid && !tr.startsolid)
		{
			VectorCopy (tr.endpos, ent->s.origin);
			ent->groundentity = tr.ent;
		}
		else
		{
			ent->s.origin[2] += 9;
		}

		VectorCopy (ent->s.origin, ent->s.old_origin);

		//we most link before killbox since it uses absmin/absmax
		gi.linkentity (ent);

		KillBox (ent);
		client->newweapon = client->weapon;
		ChangeWeapon (ent);
	}
	else
	{
		gi.linkentity (ent);
		SetVWepInfo (ent);
	}

	//just in case we spawned in the middle of a rocket or something, make sure it hits.
	G_TouchSolids (ent);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.

Called on every level change also.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	gclient_t	*client;
	char		userinfo[MAX_INFO_STRING];
	char		saved_ip[24];
	qboolean	saved_mvdclient;

	G_InitEdict (ent);

	client = ent->client;

	//init here on rather than on clientconnect, clientconnect doesn't always
	//guarantee a client is actually making it all the way into the game.
	memset (&client->resp, 0, sizeof(client->resp));

	client->pers.connected = true;

	//only run this the very first time they join the server, so pers is really persistent
	//even across map changes
	if (ent->client->pers.joinstate != JS_JOINED)
	{
		static unsigned client_counter = 0;

		strcpy (userinfo, ent->client->pers.userinfo);
		strcpy (saved_ip, ent->client->pers.ip);
		saved_mvdclient = ent->client->pers.mvdclient;

		memset (&client->pers, 0, sizeof(client->pers));

		strcpy (ent->client->pers.ip, saved_ip);
		ent->client->pers.mvdclient = saved_mvdclient;
		ClientUserinfoChanged (ent, userinfo);

		client->resp.enterframe = level.framenum;
		client->pers.connected = true;
		client->pers.joinstate = JS_FIRST_JOIN;

		//unqiue id for tracking other clients taking this slot
		client->pers.uniqueid = client_counter++;

		if (!ent->client->pers.mvdclient)
			gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
	}
	else
	{
		//wision: set up the dm_statusbar according the config and send it to the client
		TDM_SendStatusBarCS (ent);
	}

	//no cross-level menus!
	PMenu_Close (ent);

	//spawn the client
	if (ent->client->pers.team && tdm_match_status == MM_WARMUP && g_auto_rejoin_map->value)
	{
		//rejoin a team if we were on one last map (not in the middle of a game though!)
		JoinedTeam (ent, false, false);
	}
	else
	{
		ent->client->pers.team = TEAM_SPEC;

		//spawn a spectator
		PutClientInServer (ent);

		// send effect (only to local client)
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.unicast (ent, false);
	}

	// make sure all spectators are counted
	CountPlayers ();
	UpdateTeamMenu ();

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
	ent->client = game.clients + (ent - g_edicts - 1);
	ClientBeginDeathmatch (ent);
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
	
	const char		*s;
	const char		*old_name;
	const char		*old_stats_id;
	int				playernum;
	qboolean		name_changed;
	qboolean		do_config_download;

	//new connection, server is calling us. just save userinfo for later.
	if (!ent->inuse)
	{
		strncpy (ent->client->pers.ip, Info_ValueForKey(userinfo, "ip"), sizeof(ent->client->pers.ip)-1);
		strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);

		if (game.server_features & GMF_MVDSPEC)
		{
			s = Info_ValueForKey (userinfo, "mvdspec");
			if (s[0])
				ent->client->pers.mvdclient = true;
			else
				ent->client->pers.mvdclient = false;
		}
		else
			ent->client->pers.mvdclient = false;

		return;
	}

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	playernum = ent-g_edicts-1;

	old_stats_id = Info_ValueForKey (ent->client->pers.userinfo, "stats_id");
	s = Info_ValueForKey (userinfo, "stats_id");
	if (strcmp (old_stats_id, s))
		do_config_download = true;
	else
		do_config_download = false;

	name_changed = false;

	old_name = Info_ValueForKey (ent->client->pers.userinfo, "name");

	// set name
	s = Info_ValueForKey (userinfo, "name");
	
	if (strcmp (old_name, s))
	{
		if (old_name[0] && tdm_match_status > MM_COUNTDOWN && !g_allow_name_change_during_match->value)
		{
			gi.cprintf (ent, PRINT_HIGH, "You cannot change your name in the middle of the match!\n");
			Info_SetValueForKey (userinfo, "name", old_name);
			G_StuffCmd (ent, "set name \"%s\"\n", old_name);
		}
		else
		{
			edict_t	*e;

			strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

			// wision: update current_matchinfo structure during the match, so the scoreboard is correct
			if (current_matchinfo.teamplayers)
			{
				int				i;
				teamplayer_t	*tmpl;

				for (i = 0; i < current_matchinfo.num_teamplayers; i++)
				{
					tmpl = &current_matchinfo.teamplayers[i];

					if (tmpl->client == ent)
						strncpy (tmpl->name, s, sizeof(tmpl->name)-1);
				}
			}

			//reset any id cache information for this player so their name updates
			for (e = g_edicts + 1; e <= g_edicts + game.maxclients; e++)
			{
				if (e->client->resp.last_id_client == ent)
					e->client->resp.last_id_client = NULL;
			}

			gi.configstring (CS_PLAYERSKINS + playernum, va ("%s\\%s", ent->client->pers.netname, teaminfo[ent->client->pers.team].skin));

			//this handles updating team names and configstrings
			TDM_PlayerNameChanged (ent);
		}
	}

	// fov
	if ((int)dmflags->value & DF_FIXED_FOV)
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
	if (s[0])
		ent->client->pers.hand = atoi(s);

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);

	if (do_config_download)
		TDM_DownloadPlayerConfig (ent);
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
	const char	*value;
	const char	*ipa;

	// find \ip variable
	ipa = strstr(userinfo,"\\ip\\");

	if (ipa == NULL)
	{
		userinfo[0] = '\0';
		Info_SetValueForKey (userinfo,"rejmsg","Your userinfo string is malformed, please restart Quake 2.");
		return false;
	}

	// skip "\ip\"
	ipa += 4;

	// check to see if they are on the banned IP list
	if (SV_FilterPacket(ipa))
	{
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo, "rejmsg", "You are banned from this server.");
		return false;
	}

	// check for a password
	value = Info_ValueForKey (userinfo, "password");
	if (*password->string && strcmp(password->string, "none") && 
		strcmp(password->string, value))
	{
		userinfo[0] = '\0';
		Info_SetValueForKey (userinfo, "rejmsg", "Password required or incorrect.");
		return false;
	}

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	//possible new client taking the place of a non-clean disconnect, clean up if so
	if (ent->client->pers.connected)
	{
		ent->s.solid = 0;
		ent->s.effects = 0;
		ent->s.modelindex = 0;
		ent->s.sound = 0;

		ent->solid = SOLID_NOT;
		
		ent->client->pers.connected = false;

		//zero pers in preparation for new client
		memset (&ent->client->pers, 0, sizeof(ent->client->pers));
	}

	// clear the respawning variables
	//InitClientResp (ent->client);
	//InitClientPersistant (ent->client);

	//ClientUserinfoChanged (ent, userinfo);

	value = Info_ValueForKey (userinfo, "name");

	if (game.maxclients > 1)
		gi.dprintf ("%s[%s] connected\n", value, ipa);

	//ent->client->pers.connected = true;
	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will be called between levels on supported servers.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int			playernum;
	qboolean	wasInUse;

	if (!ent->client)
		return;

	// send effect (only if they were in game)
	if (ent->client->pers.team && ent->inuse)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGOUT);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	gi.unlinkentity (ent);

	wasInUse = ent->inuse;

	ent->inuse = false;

	//join code stuff is preserved in this function, don't ruin ent fields except inuse before then!
	TDM_Disconnected (ent);

	ent->s.solid = 0;
	ent->s.effects = 0;
	ent->s.modelindex = 0;
	ent->s.sound = 0;

	ent->solid = SOLID_NOT;
	
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	if (wasInUse && !ent->client->pers.mvdclient)
		gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

	playernum = ent-g_edicts-1;

	//zero pers in preparation for new client
	memset (&ent->client->pers, 0, sizeof(ent->client->pers));

	//is this really needed? it breaks all bodies left by the player.
	//gi.configstring (CS_PLAYERSKINS+playernum, "");
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
	gi.dprintf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

/*
==============
G_TouchProjectiles
==============
An insanely ugly hack that temporarily turns any FL_PROJECTILE entities
solid and runs a trace against them for clipping purposes against players.
This assumes that the ent will be freed on touch or bad things will happen.
*/
void G_TouchProjectiles (edict_t *ent, vec3_t start)
{
	edict_t	*e;
	edict_t	*ignore;
	trace_t	tr;

	int		i;

	i = 0;

	for (e = g_edicts + game.maxclients + 1; e < g_edicts + game.maxentities; e++)
	{
		if (!e->inuse)
			continue;

		if (!(e->flags & FL_NOCLIP_PROJECTILE))
			continue;

		e->solid = SOLID_BBOX;
		gi.linkentity (e);
		i++;
	}

	if (!i)
		return;

	ignore = ent;

	for (i = 0; i < 10; i++)
	{
		tr = gi.trace (start, ent->mins, ent->maxs, ent->s.origin, ignore, CONTENTS_MONSTER);
		if (tr.ent && tr.ent != world)
		{
			VectorCopy (tr.endpos, start);
			ignore = tr.ent;

			if (!(tr.ent->flags & FL_NOCLIP_PROJECTILE))
				continue;

			//gi.bprintf (PRINT_HIGH, "G_TouchProjectiles: Ent %d touching ent %d\n", ent->s.number, tr.ent->s.number);
			tr.ent->touch (tr.ent, ent, NULL, NULL);
		}
	}

	for (e = g_edicts + game.maxclients + 1; e < g_edicts + game.maxentities; e++)
	{
		if (!e->inuse)
			continue;

		if (!(e->flags & FL_NOCLIP_PROJECTILE))
			continue;

		e->solid = SOLID_TRIGGER;
		gi.linkentity (e);
	}
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

	//no movement during map or match intermission
	if (tdm_match_status == MM_SCOREBOARD || (tdm_match_status == MM_TIMEOUT && ent->client->pers.team))
	{
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		client->ps.pmove.pm_type = PM_FREEZE;
		return;
	}

	pm_passent = ent;

	if (ent->client->chase_target)
	{
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
	}
	else
	{
		vec3_t	start_origin;

		VectorCopy (ent->s.origin, start_origin);

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
			gi.sound (ent, CHAN_VOICE, soundcache[SND_JUMP1], 1, ATTN_NORM, 0);
		}

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

		if (ent->movetype != MOVETYPE_NOCLIP)
			G_TouchProjectiles (ent, start_origin);
	}

	if (client->oldbuttons != client->buttons)
		client->last_activity_frame = level.framenum;

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK)
	{
		if (client->pers.team == TEAM_SPEC)
		{
			client->latched_buttons = (client->latched_buttons & ~BUTTON_ATTACK);
			
			// wision: hide menu after join
			if (client->pers.menu.active)
			{
				PMenu_Close (ent);
				return;
			}

			// cycle between chase modes
			if (!client->chase_target)
				GetChaseTarget (ent);
			else
				NextChaseMode (ent);

			return;
		}
		else 
		{
			if (!client->weapon_thunk)
			{
				client->weapon_thunk = true;
				Think_Weapon (ent);
			}
		}
	}

	if (client->pers.team == TEAM_SPEC)
	{
		// +moveup goes to next chase target
		if (ucmd->upmove >= 10)
		{
			if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD))
			{
				client->ps.pmove.pm_flags |= PMF_JUMP_HELD;

				if (client->chase_target)
					ChaseNext (ent);
			}
		}
		else
			client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;

		// +movedown goes to previous chase target
		// maybe there's better way to do this?
		if (ucmd->upmove <= -10)
		{
			if (client->latched_buttons & BUTTON_ANY)
			{
				client->latched_buttons = 0;

				if (client->chase_target)
					ChasePrev (ent);
			}
		}
	}

	// update chase cam if being followed
	/*for (i = 1; i <= game.maxclients; i++)
	{
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}*/
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

	if (tdm_match_status == MM_SCOREBOARD)
		return;

	client = ent->client;

	// run weapon animations if it hasn't been done by a ucmd_t, only run at 10hz since gun
	// animations aren't designed for anything higher and it screws up reload times. the client
	// does the work of interpolating the frame across multiple server frames.
	if (!client->weapon_thunk && client->pers.team)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

	//gi.dprintf ("think_weapon: server frame %d, weapon frame %d\n", level.time, ent->client->ps.gunframe);

	// only remove idle players in warmup!
	if (ent->client->pers.team && tdm_match_status == MM_WARMUP && g_idle_time->value &&
		FRAMES_TO_SECS(level.framenum - ent->client->last_activity_frame) > g_idle_time->value)
	{
		gi.bprintf (PRINT_HIGH, "Removing %s from team '%s' due to inactivity.\n", ent->client->pers.netname, teaminfo[ent->client->pers.team].name);
		TDM_LeftTeam (ent, false);
		TDM_TeamsChanged ();
		respawn (ent);
	}

	if (ent->deadflag)
	{
		// force spawn set by g_respawn_time
		// spawn 1 sec after the death if player pressed attack button
		if ((level.time > client->respawn_framenum && ((int)dmflags->value & DF_FORCE_RESPAWN)) ||
			(level.time > client->respawn_framenum - ((g_respawn_time->value - 1) * (1 * SERVER_FPS)) && (client->latched_buttons & BUTTON_ATTACK)))
		{
			respawn(ent);
			client->latched_buttons = 0;
		}
		return;
	}

	client->latched_buttons = 0;
}
