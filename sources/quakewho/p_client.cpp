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

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t &self)
{
	self.svflags |= SVF_NOCLIENT;
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t &self)
{
	self.svflags |= SVF_NOCLIENT;
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t &self)
{
	self.svflags |= SVF_NOCLIENT;
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(edict_t &ent)
{
	ent.svflags |= SVF_NOCLIENT;
}


//=======================================================================

static void ClientObituary (edict_t &self, edict_t &inflictor, edict_t &attacker)
{
	/*meansofdeath_t mod;
	char		*message;
	char		*message2;
	bool		ff;

	// FIXME?
	//if (coop->value && attacker->client)
	//	meansOfDeath |= MOD_FRIENDLY_FIRE;

	ff = !!(meansOfDeath & MOD_FRIENDLY_FIRE);
	mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
	message = nullptr;
	message2 = "";

	switch (mod)
	{
	default:
		break;
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
		self->client->resp.score--;
		self->enemy = nullptr;
		return;
	}

	self->enemy = attacker;
	if (attacker && attacker->client)
	{
		switch (mod)
		{
		default:
			break;
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
			gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);

			if (ff)
				attacker->client->resp.score--;
			else
				attacker->client->resp.score++;
			return;
		}
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	self->client->resp.score--;*/
}


/*
==================
LookAtKiller
==================
*/
static void LookAtKiller (edict_t &self, edict_t &inflictor, edict_t &attacker)
{
	vec3_t		dir;

	if (attacker != game.world() && attacker != self)
		dir = attacker.s.origin - self.s.origin;
	else if (inflictor != game.world() && inflictor != self)
		dir = inflictor.s.origin - self.s.origin;
	else
	{
		self.client->killer_yaw = self.s.angles[YAW];
		return;
	}

	if (dir[0])
		self.client->killer_yaw = 180.f / M_PI * atan2(dir[1], dir[0]);
	else
	{
		self.client->killer_yaw = 0;
		if (dir[1] > 0)
			self.client->killer_yaw = 90;
		else if (dir[1] < 0)
			self.client->killer_yaw = -90;
	}

	if (self.client->killer_yaw < 0)
		self.client->killer_yaw += 360;
}

static size_t G_PlayersAliveOnTeam(const playerteam_t &team)
{
	size_t count = 0;

	for (auto &player : game.players)
		if (player.inuse && player.client && player.client->pers.connected && player.client->resp.team == team && !player.client->temp_spectator && !player.deadflag)
			count++;

	return count;
}

void G_TeamWins(const playerteam_t &team)
{
	if (level.state != GAMESTATE_PLAYING)
		return;

	for (auto &player : game.players)
		if (player.inuse && player.client && player.client->pers.connected && player.client->resp.team == team && !player.client->temp_spectator && !player.deadflag)
			player.client->resp.score++;

	gi.bprintf(PRINT_HIGH, "The %s win!\n", team == TEAM_HIDERS ? "hiders" : "hunters");
	game.world().PlaySound(gi.soundindex ("misc/secret.wav"), CHAN_AUTO, ATTN_NONE);
	level.state = GAMESTATE_INTERMISSION;
	level.state_time = level.time + 5000;
}

/*
==================
player_die
==================
*/
void player_die (edict_t &self, edict_t &inflictor, edict_t &attacker, const int32_t &damage, const vec3_t &point)
{
	self.avelocity.Clear();

	self.takedamage = false;
	self.movetype = MOVETYPE_TOSS;

	self.s.modelindex2 = MODEL_NONE;	// remove linked weapon model

	self.s.angles[0] = 0;
	self.s.angles[2] = 0;

	self.s.sound = SOUND_NONE;
	self.client->weapon_sound = SOUND_NONE;

	self.maxs[2] = -8;

	self.solid = SOLID_NOT;
	self.svflags |= SVF_DEADMONSTER;

	if (!self.deadflag)
	{
		self.client->respawn_time = level.time + 1000;
		LookAtKiller (self, inflictor, attacker);
		self.client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
		Cmd_Help_f (self);		// show scores

		// clear inventory
		self.client->pers.ammo.fill(0);
	}

	if (!self.deadflag)
	{
		constexpr struct {
			int32_t	start;
			int32_t	end;
		} death_anims[] = {
			{ FRAME_death101, FRAME_death106 },
			{ FRAME_death201, FRAME_death206 },
			{ FRAME_death301, FRAME_death308 }
		};

		// start a death animation
		self.client->anim_priority = ANIM_DEATH;
		if (self.client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			self.s.frame = FRAME_crdeath1-1;
			self.client->anim_end = FRAME_crdeath5;
		}
		else
		{
			auto &anim = death_anims[irandom(lengthof(death_anims) - 1)];
			self.s.frame = anim.start-1;
			self.client->anim_end = anim.end;
		}
		self.PlaySound(gi.soundindex(va("*death%i.wav", irandom(1, 4))), CHAN_VOICE);
	}

	self.deadflag = true;

	self.Link();

	if (self.client->resp.team == TEAM_HIDERS)
		for (auto &player : game.players)
			if (player.inuse && player.client->resp.team == TEAM_HUNTERS && player.client->radar.entity == self)
				player.client->radar = {};

	if (!G_PlayersAliveOnTeam(self.client->resp.team))
		G_TeamWins(self.client->resp.team == TEAM_HIDERS ? TEAM_HUNTERS : TEAM_HIDERS);
	else
		gi.bprintf(PRINT_HIGH, "%s, a %s, died!\n", self.client->pers.netname, self.client->resp.team == TEAM_HIDERS ? "hider" : "hunter");
}

//=======================================================================

constexpr ammo_data_t default_ammos = {
	//100,
	20,
	5
};

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
static void InitClientPersistant (gclient_t &client)
{
	client.pers = {
		.userinfo = std::move(client.pers.userinfo),
		.connected = true,
		.health = DEFAULT_HEALTH,
		.max_health = DEFAULT_HEALTH,
		.ammo = default_ammos,
		.weapon = &g_weapons[WEAP_BLASTER]
	};
}


static void InitClientResp (gclient_t &client)
{
	client.resp = {
		.enterframe = level.framenum
	};
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
void SaveClientData ()
{
	for (auto &ent : game.players)
	{
		if (!ent.inuse)
			continue;

		gclient_t &cl = *ent.client;

		cl.pers.health = ent.health;
		cl.pers.max_health = ent.max_health;
		cl.pers.savedFlags = (ent.flags & FL_GODMODE);
	}
}

static void FetchClientEntData (edict_t &ent)
{
	ent.health = ent.client->pers.health;
	ent.max_health = ent.client->pers.max_health;
	ent.flags |= ent.client->pers.savedFlags;
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
static vec_t PlayersRangeFromSpot (edict_t &player, edict_t &spot)
{
	vec_t bestplayerdistance = FLT_MAX;

	for (auto &check : game.players)
	{
		// team players can spawn near each other
		if (OnSameTeam(player, check))
			continue;

		if (!check.inuse)
			continue;

		if (check.health <= 0)
			continue;

		const vec3_t v = spot.s.origin - check.s.origin;
		const vec_t playerdistance = v.Length();

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
static edict_ref SelectRandomDeathmatchSpawnPoint(edict_t &ent)
{
	size_t count = 0;
	edict_ref spot = nullptr;
	vec_t range1 = FLT_MAX, range2 = FLT_MAX;
	edict_ref spot1 = nullptr, spot2 = nullptr;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")))
	{
		const vec_t range = PlayersRangeFromSpot(ent, spot);

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

		count++;
	}

	if (!count)
		return nullptr;

	if (count <= 2)
		spot1 = spot2 = nullptr;
	else
		count -= 2;

	size_t selection = irandom(count - 1);

	spot = nullptr;
	do
	{
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
static void SelectSpawnPoint (edict_t &ent, vec3_t &origin, vec3_t &angles)
{
	const edict_ref &spot = SelectRandomDeathmatchSpawnPoint(ent);

	// find a single player start spot
	if (!spot)
		gi.error ("Couldn't find spawn point\n");

	origin = spot->s.origin;
	origin[2] += 9;
	angles = spot->s.angles;
}

//======================================================================

void respawn (edict_t &self)
{
	// spectator's don't leave bodies
	if (self.movetype != MOVETYPE_NOCLIP)
		self.Unlink();

	self.svflags &= ~SVF_NOCLIENT;
	PutClientInServer (self);

	// add a teleportation effect
	self.s.event = EV_PLAYER_TELEPORT;

	self.client->respawn_time = level.time;
}

/* 
 * only called when pers.spectator changes
 * note that resp.spectator should be the opposite of pers.spectator here
 */
static void spectator_respawn (edict_t &ent)
{
	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators
	gclient_t &client = *ent.client;

	if (client.pers.spectator)
	{
		const char *value;

		if (*spectator_password->string &&
			strcmp(spectator_password->string, "none") &&
			client.pers.userinfo.Get("spectator", value) &&
			strcmp(spectator_password->string, value))
		{
			ent.client->Print("Spectator password incorrect.\n");
			client.pers.spectator = false;
			gi.WriteByte (SVC_STUFFTEXT);
			gi.WriteString ("spectator 0\n");
			ent.Unicast(true);
			return;
		}

		// count spectators
		size_t numspec = 0;

		for (auto &player : game.players)
			if (player.inuse && player.client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value)
		{
			ent.client->Print("Server spectator limit is full.");
			client.pers.spectator = false;
			// reset his spectator var
			gi.WriteByte (SVC_STUFFTEXT);
			gi.WriteString ("spectator 0\n");
			ent.Unicast(true);
			return;
		}
	}
	else
	{
		const char *value;

		// he was a spectator and wants to join the game
		// he must have the right password
		if (*password->string &&
			strcmp(password->string, "none") && 
			client.pers.userinfo.Get("password", value) &&
			strcmp(password->string, value)) {
			ent.client->Print("Password incorrect.\n");
			client.pers.spectator = true;
			gi.WriteByte (SVC_STUFFTEXT);
			gi.WriteString ("spectator 1\n");
			ent.Unicast(true);
			return;
		}
	}

	// clear client on respawn
	client.resp.score = client.pers.score = 0;
	client.resp.team = TEAM_NONE;

	ent.svflags &= ~SVF_NOCLIENT;
	PutClientInServer (ent);

	// add a teleportation effect
	if (!client.pers.spectator)
	{
		// send effect
		gi.WriteByte (SVC_MUZZLEFLASH);
		gi.WriteEntity (ent);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent.s.origin, MULTICAST_PVS);
	}

	client.respawn_time = level.time;

	if (client.pers.spectator) 
		gi.bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", client.pers.netname);
	else
		gi.bprintf (PRINT_HIGH, "%s joined the game\n", client.pers.netname);
}

//==============================================================

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
static void ParseClientUserinfo (edict_t &ent)
{
	gclient_t &client = *ent.client;

	client.pers.hand = RIGHT_HANDED;
	client.pers.spectator = false;

	// set name
	const char *name;

	if (!client.pers.userinfo.Get("name", name))
	{
		gi.error("Fatal error; no name in userinfo\n");
		return;
	}

	strncpy (client.pers.netname, name, sizeof(client.pers.netname) - 1);

	// set spectator
	const char *spectator;

	if (client.pers.userinfo.Get("spectator", spectator) && strcmp(spectator, "0"))
		client.pers.spectator = true;

	// set skin
	const char *skin;

	if (!client.pers.userinfo.Get("skin", skin))
	{
		gi.error("Fatal error; no skin in userinfo\n");
		return;
	}

	if (ent.client->resp.team == TEAM_HIDERS)
		skin = "female/athena";
	else if (ent.client->resp.team == TEAM_HUNTERS)
		skin = "male/grunt";

	const int32_t playernum = ent.s.number - 1;

	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS + playernum, va("%s\\%s", client.pers.netname, skin));

	// fov
	client.ps.fov = 90.f;

	if (client.pers.userinfo.Get("fov", client.ps.fov))
		client.ps.fov = clamp(client.ps.fov, 1.f, 160.f);

	// handedness
	int32_t hand;

	if (client.pers.userinfo.Get("hand", hand))
		client.pers.hand = clamp<handedness_t>(static_cast<handedness_t>(hand), RIGHT_HANDED, CENTER_HANDED);
}


/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t &ent, const char *userinfo)
{
	userinfo_t uinfo;

	if (!uinfo.Parse(userinfo) || !uinfo.Has("name") || !uinfo.Has("skin"))
		uinfo.Parse(DEFAULT_USERINFO);

	ent.client->pers.userinfo = std::move(uinfo);
	ParseClientUserinfo(ent);
}

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer (edict_t &ent)
{
	constexpr vec3_t mins {-16, -16, -24};
	constexpr vec3_t maxs {16, 16, 32};

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	vec3_t spawn_origin, spawn_angles;
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	const int32_t index = ent.s.number - 1;
	gclient_t &client = *ent.client;

	// deathmatch wipes most client data every spawn
	InitClientPersistant (client);

	// clear everything but the persistant data
	client = {
		.pers = std::move(client.pers),
		.resp = std::move(client.resp)
	};

	if (client.pers.health <= 0)
		InitClientPersistant(client);

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent.groundentity = nullptr;
	ent.client = &game.clients[index];
	ent.takedamage = true;
	ent.movetype = MOVETYPE_WALK;
	ent.viewheight = 22;
	ent.inuse = true;
	ent.classname = "player";
	ent.mass = 200;
	ent.solid = SOLID_BBOX;
	ent.deadflag = false;
	ent.air_finished = level.time + 12000;
	ent.clipmask = MASK_PLAYERSOLID;
	ent.model = "players/male/tris.md2";
	ent.die = player_die;
	ent.waterlevel = WATER_NONE;
	ent.watertype = CONTENTS_NONE;
	ent.flags &= ~FL_NO_KNOCKBACK;
	ent.svflags &= ~SVF_DEADMONSTER;

	ent.mins = mins;
	ent.maxs = maxs;
	ent.velocity.Clear();

	// clear entity state values
	ent.s.effects = EF_NONE;
	ent.s.modelindex = MODEL_PLAYER;		// will use the skin specified model
	ent.s.modelindex2 = MODEL_PLAYER;		// custom gun model
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent.s.skinnum = ent.s.number - 1;

	ent.s.frame = 0;
	ent.s.origin = spawn_origin;
	ent.s.origin[2] += 1;	// make sure off ground
	ent.s.old_origin = ent.s.origin;

	// clear playerstate values
	client.ps = player_state_t();
	ParseClientUserinfo(ent);

	for (size_t i = 0; i < 3; i++)
	{
		client.ps.pmove.origin[i] = spawn_origin[i] * 8;

		// set the delta angle
		client.ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client.resp.cmd_angles[i]);
	}

	client.v_angle = client.ps.viewangles = ent.s.angles = { 0, spawn_angles[YAW], 0 };
	client.resp.spectator = false;
	client.num_jumps = 3;

	// spawn a spectator
	if (client.pers.spectator || client.resp.team == TEAM_NONE || level.state >= GAMESTATE_PLAYING)
	{
		client.chase_target = nullptr;
		client.resp.spectator = true;

		ent.movetype = MOVETYPE_NOCLIP;
		ent.solid = SOLID_NOT;
		ent.svflags |= SVF_NOCLIENT;
		ent.Link();

		// we're not a real spectator, we're spectating because we died or respawning during intermission
		if ((client.resp.team && level.state == GAMESTATE_PLAYING) || level.state > GAMESTATE_PLAYING)
			client.temp_spectator = true;

		return;
	}

	// hiders will convert a random enemy
	if (client.resp.team == TEAM_HIDERS)
	{
		const size_t monster_id = irandom(level.monsters.size() - 1);

		edict_t &monster = level.monsters[monster_id];

		level.monsters.erase(level.monsters.begin() + monster_id);

		Possess(ent, monster, false);

		ent.client->CenterPrint("You're a hider! You've been given a random body.\nJUMP to reveal yourself; SHOOT to clone a monster.\nCROUCH will lock your view and allow you to drop off of ledges\nDon't get caught!\n");

		return;
	}

	ent.client->CenterPrint("You're a hunter! Some monsters may not be what they seem.\nFind the hiders, but don't hurt any innocent Stroggos!\n");
	
	if (client.pers.weapon)
		client.ps.gunindex = gi.modelindex(client.pers.weapon->view_model);

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	ent.Link();

	// force the current weapon up
	client.newweapon = client.pers.weapon;
	ChangeWeapon (ent);
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t &ent)
{
	G_InitEdict (ent);

	ent.client = &game.clients[ent.s.number - 1];

	InitClientResp (*ent.client);

	// locate ent at a spawn point
	PutClientInServer (ent);

	if (level.intermissiontime)
		MoveClientToIntermission (ent);
	else
	{
		// send effect
		gi.WriteByte (SVC_MUZZLEFLASH);
		gi.WriteEntity (ent);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent.s.origin, MULTICAST_PVS);
	}

	gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent.client->pers.netname);

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
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
qboolean ClientConnect (edict_t &ent, char *userinfo_string)
{
	userinfo_t userinfo;

	if (!userinfo.Parse(userinfo_string))
	{
		userinfo.Set("rejmsg", "Invalid userinfo.");
		userinfo.Encode(userinfo_string);
		return false;
	}

	// check for a spectator
	const char *value;

	if (userinfo.Get("spectator", value) && strcmp(value, "0"))
	{
		size_t numspec = 0;

		if (*spectator_password->string &&
			strcmp(spectator_password->string, "none") &&
			strcmp(spectator_password->string, value))
		{
			userinfo.Set("rejmsg", "Spectator password required or incorrect.");
			userinfo.Encode(userinfo_string);
			return false;
		}

		// count spectators
		for (auto &player : game.players)
			if (player.inuse && player.client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value)
		{
			userinfo.Set("rejmsg", "Server spectator limit is full.");
			userinfo.Encode(userinfo_string);
			return false;
		}
	}
	else
	{
		// check for a password
		if (*password->string &&
			strcmp(password->string, "none") && 
			userinfo.Get("password", value) &&
			strcmp(password->string, value))
		{
			userinfo.Set("rejmsg", "Password required or incorrect.");
			userinfo.Encode(userinfo_string);
			return false;
		}
	}

	// they can connect
	ent.client = &game.clients[&ent - g_edicts - 1];

	gclient_t &client = *ent.client;

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent.inuse == false)
	{
		// clear the respawning variables
		InitClientResp (client);
		if (!client.pers.weapon)
			InitClientPersistant (client);
	}

	client.pers.userinfo = std::move(userinfo);
	ParseClientUserinfo (ent);

	if (game.clients.size() > 1)
		gi.dprintf ("%s connected\n", client.pers.netname);

	ent.svflags = SVF_NONE; // make sure we start with known default
	ent.client->pers.connected = true;
	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t &ent)
{
	if (!ent.client)
		return;

	gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent.client->pers.netname);

	if (ent.client->resp.team)
	{
		if (level.state >= GAMESTATE_SPAWNING)
			player_die(ent, ent, ent, 0, vec3_origin);

		ent.client->resp.team = TEAM_NONE;
	}

	// send effect
	gi.WriteByte (SVC_MUZZLEFLASH);
	gi.WriteEntity (ent);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent.s.origin, MULTICAST_PVS);

	ent.Unlink();
	ent.s.modelindex = MODEL_NONE;
	ent.solid = SOLID_NOT;
	ent.inuse = false;
	ent.classname = "disconnected";
	ent.client->pers.connected = false;

	gi.configstring (CS_PLAYERSKINS + ent.s.number - 1, "");
}


//==============================================================

static edict_ref pm_passent;

// pmove doesn't need to know about passent and contentmask
static trace_t	PM_trace (const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink (edict_t &ent, const usercmd_t &ucmd)
{
	gclient_t &client = *ent.client;

	level.current_entity = ent;

	if (level.intermissiontime)
	{
		client.ps.pmove.pm_type = PM_FREEZE;

		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5000 && (ucmd.buttons & BUTTON_ANY))
			level.exitintermission = true;

		level.current_entity = nullptr;
		return;
	}

	pm_passent = ent;

	if (client.chase_target || ent.control)
	{
		for (size_t i = 0; i < 3; i++)
		{
			client.resp.cmd_angles[i] = SHORT2ANGLE(ucmd.angles[i]);
			client.ps.viewangles[i] = client.resp.cmd_angles[i] + SHORT2ANGLE(client.ps.pmove.delta_angles[i]);
		}

		client.v_angle = client.ps.viewangles;
	}
	else
	{
		if (ent.movetype == MOVETYPE_NOCLIP)
			client.ps.pmove.pm_type = PM_SPECTATOR;
		else if (ent.s.modelindex != MODEL_PLAYER)
			client.ps.pmove.pm_type = PM_GIB;
		else if (ent.deadflag)
			client.ps.pmove.pm_type = PM_DEAD;
		else
			client.ps.pmove.pm_type = PM_NORMAL;

		client.ps.pmove.gravity = sv_gravity->value;

		// set up for pmove
		pmove_t pm {
			.s = client.ps.pmove,
			.cmd = ucmd,
			.trace = PM_trace,
			.pointcontents = gi.pointcontents
		};

		for (size_t i = 0; i < 3; i++)
		{
			pm.s.origin[i] = ent.s.origin[i] * 8;
			pm.s.velocity[i] = ent.velocity[i] * 8;
		}

		if (memcmp(&client.old_pmove, &pm.s, sizeof(pm.s)))
			pm.snapinitial = true;

		// perform a pmove
		gi.Pmove (pm);

		// save results of pmove
		client.old_pmove = client.ps.pmove = pm.s;

		for (size_t i = 0; i < 3; i++)
		{
			ent.s.origin[i] = pm.s.origin[i]*0.125;
			ent.velocity[i] = pm.s.velocity[i]*0.125;

			client.resp.cmd_angles[i] = SHORT2ANGLE(ucmd.angles[i]);
		}

		ent.mins = pm.mins;
		ent.maxs = pm.maxs;

		if (ent.groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == WATER_NONE) && level.time > client.jump_sound_debounce)
			ent.PlaySound(gi.soundindex("*jump1.wav"), CHAN_VOICE);

		ent.viewheight = pm.viewheight;
		ent.waterlevel = pm.waterlevel;
		ent.watertype = pm.watertype;
		ent.groundentity = pm.groundentity;
		
		if (pm.groundentity)
			ent.groundentity_linkcount = pm.groundentity->linkcount;

		if (ent.deadflag)
			client.ps.viewangles = { -15.f, client.killer_yaw, 40.f };
		else
			client.ps.viewangles = client.v_angle = pm.viewangles;

		ent.Link();

		if (ent.movetype != MOVETYPE_NOCLIP)
			G_TouchTriggers (ent);

		// touch other objects
		for (int32_t i = 0; i < pm.numtouch; i++)
		{
			edict_ref other = pm.touchents[i];
			
			int32_t j;
			for (j = 0; j < i; j++)
				if (pm.touchents[j] == other)
					break;

			if (j != i)
				continue;	// duplicated
			if (!other->touch)
				continue;

			other->touch (other, ent, nullptr, nullptr);
		}
	}

	const button_t oldbuttons = client.buttons;
	client.buttons = ucmd.buttons;
	client.latched_buttons |= client.buttons & ~oldbuttons;

	// fire weapon from final position if needed
	if (client.latched_buttons & BUTTON_ATTACK)
	{
		if (client.resp.spectator || ent.control)
		{
			client.weapon_thunk = true;
			Think_Weapon (ent);

			client.latched_buttons = BUTTON_NONE;

			if (client.resp.spectator)
			{			
				if (client.chase_target)
				{
					client.chase_target = nullptr;
					client.ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				}
				else
					GetChaseTarget(ent);
			}
		}
		else if (!client.weapon_thunk)
		{
			client.weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

	if (client.resp.spectator || ent.control)
	{
		if (ucmd.upmove >= 10)
		{
			if (!(client.ps.pmove.pm_flags & PMF_JUMP_HELD))
			{
				client.ps.pmove.pm_flags |= PMF_JUMP_HELD;

				if (client.resp.spectator)
				{
					if (client.chase_target)
						ChaseNext(ent);
					else
						GetChaseTarget(ent);
				}
			}
		}
		else
			client.ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
	}

	// update chase cam if being followed
	for (auto &other : game.players)
		if (other.inuse && other.client->chase_target == ent)
			UpdateChaseCam(other);

	if (ent.control)
	{
		UpdateTargetCam(ent);
		ent.client->cmd = ucmd;
	}

	level.current_entity = nullptr;
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t &ent)
{
	if (level.intermissiontime)
		return;

	gclient_t &client = *ent.client;

	// player wants to switch to spectator while already in a game
	if (client.resp.team && client.pers.spectator != client.resp.spectator &&
		(level.time - client.respawn_time) >= 5000 && !client.temp_spectator)
	{
		spectator_respawn(ent);
		return;
	}

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client.weapon_thunk && !client.resp.spectator)
		Think_Weapon (ent);
	else
		client.weapon_thunk = false;

	if (ent.deadflag)
	{
		// wait for any button just going down
		if (level.time > client.respawn_time)
		{
			if (client.latched_buttons & BUTTON_ATTACK)
			{
				respawn(ent);
				client.latched_buttons = BUTTON_NONE;
			}
		}

		return;
	}

	client.latched_buttons = BUTTON_NONE;
}
