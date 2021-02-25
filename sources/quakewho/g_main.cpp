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

#include <cstdarg>

game_locals_t	game;

edict_ref::operator bool() const
{
	return e && e->inuse;
}

edict_ref::operator const edict_t &() const
{
	if (!e)
	{
#if defined(_DEBUG) && defined(_WIN32)
		__debugbreak();
#endif
		gi.dprintf("dereferencing null entity\n");
		return game.world();
	}

	return *e;
}

entity_iterator game_locals_t::player_list_t::end()
{
	return entity_iterator(game.clients.size() + 1);
}

level_locals_t	level;
game_import_t	gi;

static cvar_t	*dmflags_cvar;

dmflags_t	dmflags;
cvar_t		*fraglimit;
cvar_t		*timelimit;
cvar_t		*roundlimit;
cvar_t		*password;
cvar_t		*spectator_password;
cvar_t		*needpass;
cvar_t		*maxspectators;
cvar_t		*g_select_empty;
cvar_t		*dedicated;

cvar_t		*sv_maxvelocity;
cvar_t		*sv_gravity;

cvar_t		*sv_rollspeed;
cvar_t		*sv_rollangle;
cvar_t		*gun_x;
cvar_t		*gun_y;
cvar_t		*gun_z;

cvar_t		*run_pitch;
cvar_t		*run_roll;
cvar_t		*bob_up;
cvar_t		*bob_pitch;
cvar_t		*bob_roll;

cvar_t		*flood_msgs;
cvar_t		*flood_persecond;
cvar_t		*flood_waitdelay;

cvar_t		*sv_maplist;

/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/
static void InitGame ()
{
	gi.dprintf ("==== %s ====\n", __FUNCTION__);

	gun_x = gi.cvar ("gun_x", "0", CVAR_NONE);
	gun_y = gi.cvar ("gun_y", "0", CVAR_NONE);
	gun_z = gi.cvar ("gun_z", "0", CVAR_NONE);

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed = gi.cvar ("sv_rollspeed", "200", CVAR_NONE);
	sv_rollangle = gi.cvar ("sv_rollangle", "2", CVAR_NONE);
	sv_maxvelocity = gi.cvar ("sv_maxvelocity", "2000", CVAR_NONE);
	sv_gravity = gi.cvar ("sv_gravity", "800", CVAR_NONE);

	// noset vars
	dedicated = gi.cvar ("dedicated", "0", CVAR_NOSET);

	gi.cvar ("gamename", GAMEVERSION, CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar ("gamedate", __DATE__, CVAR_SERVERINFO | CVAR_LATCH);

	maxspectators = gi.cvar ("maxspectators", "4", CVAR_SERVERINFO);

	// change anytime vars
	dmflags_cvar = gi.cvar ("dmflags", "0", CVAR_SERVERINFO);
	dmflags.bits = static_cast<dmflags_bits_t>(dmflags_cvar->value);
	fraglimit = gi.cvar ("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar ("timelimit", "0", CVAR_SERVERINFO);
	roundlimit = gi.cvar ("roundlimit", "240", CVAR_SERVERINFO);
	password = gi.cvar ("password", "", CVAR_USERINFO);
	spectator_password = gi.cvar ("spectator_password", "", CVAR_USERINFO);
	needpass = gi.cvar ("needpass", "0", CVAR_SERVERINFO);

	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar ("run_pitch", "0.002", CVAR_NONE);
	run_roll = gi.cvar ("run_roll", "0.005", CVAR_NONE);
	bob_up  = gi.cvar ("bob_up", "0.005", CVAR_NONE);
	bob_pitch = gi.cvar ("bob_pitch", "0.002", CVAR_NONE);
	bob_roll = gi.cvar ("bob_roll", "0.002", CVAR_NONE);

	// flood control
	flood_msgs = gi.cvar ("flood_msgs", "4", CVAR_NONE);
	flood_persecond = gi.cvar ("flood_persecond", "4", CVAR_NONE);
	flood_waitdelay = gi.cvar ("flood_waitdelay", "10", CVAR_NONE);

	// dm map list
	sv_maplist = gi.cvar ("sv_maplist", "", CVAR_NONE);

	const cvar_t *sv_cheats = gi.cvar ("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
	game.cheats_enabled = !!sv_cheats->value;

	// initialize all clients for this game
	const cvar_t *maxclients = gi.cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	game.clients.resize(maxclients->value);

	for (auto &client : game.clients)
		new(&client) gclient_t();

	// initialize all entities for this game
	globals.entities.size = sizeof(edict_t);
	globals.entities.num = game.clients.size() + 1;
	globals.entities.pool = gi.TagMalloc<edict_t>(globals.entities.max, TAG_GAME);

	for (auto &e : game.entities.range(0, globals.entities.max))
		edict_t::initialize(&e);
}

//===================================================================


static void ShutdownGame ()
{
	gi.dprintf ("==== %s ====\n", __FUNCTION__);

	for (auto &e : game.entities.range(0, globals.entities.max))
		e.~edict_t();

	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);

	game = {};
}

//======================================================================


/*
=================
ClientEndServerFrames
=================
*/
static void ClientEndServerFrames ()
{
	// calc the player views now that all pushing
	// and damage has been added
	for (auto &ent : game.players)
		if (ent.inuse && ent.client)
			ClientEndServerFrame (ent);
}

/*
=================
CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
static edict_t &CreateTargetChangeLevel(const char *map)
{
	edict_t &ent = G_Spawn();
	ent.classname = "target_changelevel";
	snprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent.map = level.nextmap;
	return ent;
}

/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
static void EndDMLevel ()
{
	static constexpr const char *seps = " ,\n\r";

	// stay on same level flag
	if (dmflags.same_level)
	{
		BeginIntermission(CreateTargetChangeLevel(level.mapname));
		return;
	}

	// see if it's in the map list
	if (*sv_maplist->string)
	{
		char *s = strdup(sv_maplist->string);
		char *f = nullptr;
		char *t = strtok(s, seps);

		while (t != nullptr)
		{
			if (iequals(t, level.mapname))
			{
				// it's in the list, go to the next one
				t = strtok(nullptr, seps);

				// end of list, go to first one
				if (t == nullptr)
				{
					if (f == nullptr) // there isn't a first one, same level
						BeginIntermission(CreateTargetChangeLevel(level.mapname));
					else
						BeginIntermission(CreateTargetChangeLevel(f));
				}
				else
					BeginIntermission(CreateTargetChangeLevel(t));

				free(s);
				return;
			}

			if (!f)
				f = t;
			
			t = strtok(nullptr, seps);
		}

		free(s);
	}

	if (level.nextmap[0]) // go to a specific map
	{
		BeginIntermission(CreateTargetChangeLevel(level.nextmap));
		return;
	}
	
	// search for a changelevel
	edict_ref ent = G_Find(nullptr, FOFS(classname), "target_changelevel");

	if (!ent)
	{	// the map designer didn't include a changelevel,
		// so create a fake ent that goes back to the same level
		BeginIntermission(CreateTargetChangeLevel(level.mapname));
		return;
	}
	
	BeginIntermission(ent);
}


/*
=================
CheckNeedPass
=================
*/
static void CheckNeedPass ()
{
	// if password or spectator_password has changed, update needpass
	// as needed
	if (!password->modified && !spectator_password->modified)
		return;

	password->modified = spectator_password->modified = false;

	int32_t need = 0;

	if (*password->string && !iequals(password->string, "none"))
		need |= 1;
	if (*spectator_password->string && !iequals(spectator_password->string, "none"))
		need |= 2;

	gi.cvar_set("needpass", va("%d", need));
}

static size_t G_NumClients()
{
	size_t num_clients = 0;

	for (edict_t &player : game.players)
		if (player.inuse && player.client && player.client->pers.connected)
			num_clients++;

	return num_clients;
}

void G_CheckPlayerReady()
{
	if (level.state != GAMESTATE_WAITING)
		return;

	size_t num_ready = 0;

	for (auto &player : game.players)
		if (player.inuse && player.client && player.client->pers.connected && player.client->resp.ready)
			num_ready++;

	const size_t num_clients = G_NumClients();

	if ((num_clients <= 2 && num_clients == num_ready) || (num_ready >= num_clients / 2))
	{
		// ready to go!
		level.state = GAMESTATE_SPAWNING;
	
		for (auto &p : level.monsters)
			G_FreeEdict(p);

		level.monsters.clear();

		gi.bprintf(PRINT_HIGH, "Players ready! Please be patient while the enemies spawn...\n");
		game.world().PlaySound(gi.soundindex ("misc/secret.wav"), CHAN_AUTO, ATTN_NONE);
	}
}


/*
================
EntitiesRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
static vec_t EntitiesRangeFromSpot (nav_grid_node *spot)
{
	vec_t	bestdistance = 9999999;

	for (auto &e : game.entities.range(1))
		if (e.inuse && (e.client || (e.svflags & SVF_MONSTER)) && !(e.svflags & SVF_NOCLIENT) && e.health)
			bestdistance = min(bestdistance, spot->position.Distance(e.s.origin));

	return bestdistance;
}

/*
================
SelectFarthestMonsterSpawnPoint
================
*/
static nav_grid_node *SelectFarthestMonsterSpawnPoint()
{
	nav_grid_node *bestspot = nullptr;
	vec_t bestdistance = 0;

	for (auto &pt : nav_points)
	{
		if (level.skip_points.count(pt.second))
			continue;

		vec_t bestplayerdistance = EntitiesRangeFromSpot (pt.second);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = pt.second;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
		return bestspot;

	return nav_points[irandom(nav_points.size() - 1)].second;
}

void EmplaceMonsterInGoodSpot(edict_t &ent)
{
	ent.Unlink();

	while (true)
	{
		if (level.skip_points.size() >= nav_points.size())
			level.skip_points.clear();

		auto pt = SelectFarthestMonsterSpawnPoint();
		ent.s.origin = pt->position;
		ent.s.origin[2] -= ent.mins[2];

		const trace_t tr = gi.trace(ent.s.origin, ent.mins, ent.maxs, ent.s.origin, nullptr, MASK_CLIP);

		if (tr.allsolid || tr.startsolid || tr.fraction != 1.0f)
		{
			level.skip_points.emplace(pt);
			continue;
		}

		break;
	}

	ent.s.old_origin = ent.s.origin;
	ent.s.angles[1] = frandom(360);
	ent.flags |= FL_PARTIALGROUND;
	ent.s.event = EV_OTHER_TELEPORT;
	ent.Link();
}

constexpr gtime_t RADAR_TIME = 3000;

static void G_SpawnMonsters()
{
	edict_t &ent = G_Spawn();

	monster_def_t &def = G_RandomMonster();
	
	ent.monsterinfo.name = def.name;

	def.func(ent);

	EmplaceMonsterInGoodSpot(ent);

	level.monsters.push_back(ent);

	if (level.monsters.size() >= level.max_monsters)
	{
		level.control_delay = level.time + 5000;
		level.radar_time = level.control_delay + RADAR_TIME;
		level.monster_die_time = level.time + level.monster_kill_time;
		level.round_end = (roundlimit->value * 1000) + level.control_delay;
		level.countdown_sound = false;

		// spawn players
		gi.bprintf(PRINT_HIGH, "Done spawning monsters!\n");
		game.world().PlaySound(gi.soundindex ("misc/secret.wav"), CHAN_AUTO, ATTN_NONE);

		playerteam_t team = prandom(50) ? TEAM_HIDERS : TEAM_HUNTERS;

		for (auto &player : game.players)
		{
			if (!player.client || !player.client->pers.connected || !player.client->resp.ready)
				continue;

			// if we're a new player, assign us a random team
			// FIXME
			if (!player.client->resp.team)
			{
				player.client->resp.team = team;
				team = (team == TEAM_HIDERS) ? TEAM_HUNTERS : TEAM_HIDERS;
			}

			respawn(player);
		}

		level.state = GAMESTATE_PLAYING;
	}
}

static edict_ref G_GetDensestMonster()
{
	vec_t best_distance = FLT_MAX;
	edict_ref best_entity = nullptr;

	for (auto &m : level.monsters)
	{
		vec_t dist = 0;

		for (auto &m2 : level.monsters)
			if (m != m2)
				dist += m2->s.origin.Distance(m->s.origin);

		if (!best_entity || dist < best_distance)
		{
			best_entity = m;
			best_distance = dist;
		}
	}

	return best_entity;
}

static void G_KillRandomMonster()
{
	if (!level.monster_kill_time || level.monster_die_time >= level.time || level.monsters.size() <= level.end_max_monsters)
		return;

	level.monster_die_time = level.time + level.monster_kill_time;

	edict_t &monster = G_GetDensestMonster();

	monster.die(monster, monster, monster, monster.health, vec3_origin);
}

static std::pair<vec_t, vec_t> G_WallPenetration(edict_t &from, edict_t &to)
{
	vec_t solid_dist = 0, air_dist = 0;
	vec3_t start = from.s.origin;
	const vec3_t dir = (from.s.origin - to.s.origin).Normalized() * 0.125f;

	while (true)
	{
		const trace_t to_other = gi.trace(start, to.s.origin, MASK_SOLID);

		// started inside a wall, so the distance from to_other.endpos to from_other.endpos
		// is air
		if (to_other.startsolid)
		{
			const trace_t from_other = gi.trace(to_other.endpos - dir, start, MASK_SOLID);

			const vec_t air = to_other.endpos.Distance(from_other.endpos);

			// and the remaining distance is solid
			const vec_t solid = start.Distance(to_other.endpos) - air;

			air_dist += air;
			solid_dist += solid;
		}
		// started in air, so distance from start to to_other.endpos is air
		else
		{
			const vec_t air = start.Distance(to_other.endpos);

			air_dist += air;
		}

		start = to_other.endpos - dir;

		if (to_other.fraction == 1.0f)
			return std::make_pair(solid_dist, air_dist);
	}
}

static radar_status_t G_EntityWithinRadarRange(edict_t &from, edict_t &other, float &factor)
{
	/*if (!gi.inPHS(other.s.origin, from.s.origin))
		return RADAR_EMPTY;

	if (!gi.inPVS(other.s.origin, from.s.origin))
		return RADAR_STAGE_1;

	vec_t dist = from.s.origin.Distance(other.s.origin);

	if (gi.trace(from.s.origin, other.s.origin, MASK_SOLID).fraction < 1.0f)
		dist *= 2;

	if (dist >= 768.f)
		return RADAR_STAGE_2;

	return RADAR_STAGE_3;*/

	const auto &pen = G_WallPenetration(from, other);

	const vec_t dist_factor = (pen.first * 3.5f) + (pen.second / 1.1f);
	constexpr vec_t dist_max = 2048.f;

	factor = (1.0f - (dist_factor / dist_max));

	if (factor > 0.75f)
		return RADAR_STAGE_3;
	else if (factor > 0.5f)
		return RADAR_STAGE_2;
	else if (factor > 0.05f)
		return RADAR_STAGE_1;

	return RADAR_EMPTY;
}

static void G_UpdateRadars()
{
	if (level.radar_time > level.time)
		return;

	// reset all hider radars
	for (auto &player : game.players)
		if (player.inuse && player.client->resp.team == TEAM_HIDERS)
		{
			player.client->radar.status = RADAR_EMPTY;
			player.client->radar.factor = 0;
		}

	// check hunter radars
	for (auto &player : game.players)
	{
		if (!player.inuse || player.client->resp.team != TEAM_HUNTERS || player.deadflag || player.client->resp.spectator)
		{
			if (player.client->radar.entity)
				player.client->radar = {};

			continue;
		}

		if (!player.client->radar.entity)
		{
			vec_t factor = 0;
			std::vector<edict_ref> radar_choices;

			// find an entity
			for (auto &other_player : game.players)
				if (other_player.inuse && other_player.client->resp.team == TEAM_HIDERS && !other_player.client->resp.spectator &&
					G_EntityWithinRadarRange(player, other_player.control ? static_cast<edict_t &>(other_player.control) : other_player, factor) >= RADAR_STAGE_1)
					radar_choices.push_back(other_player);

			if (radar_choices.size())
			{
				player.client->radar.entity = radar_choices[irandom(radar_choices.size() - 1)];
				player.client->radar.status = RADAR_STAGE_1;
				player.client->radar.entity->client->radar.status = player.client->radar.status;
				player.client->radar.factor = player.client->radar.entity->client->radar.factor = factor;
				player.client->radar.last_factor = player.client->radar.entity->client->radar.last_factor = -1;
			}
		}
		else
		{
			vec_t factor = 0;
			const radar_status_t new_status = G_EntityWithinRadarRange(player, player.client->radar.entity, factor);

			if (player.client->radar.status < new_status)
				player.client->radar.status++;
			else if (player.client->radar.status > new_status)
			{
				player.client->radar.status--;

				if (!player.client->radar.status)
					player.client->radar.entity = nullptr;
			}

			player.client->radar.factor = factor;
			
			// update our enemy's radar status
			if (player.client->radar.entity)
			{
				player.client->radar.entity->client->radar.status = player.client->radar.status;
				player.client->radar.entity->client->radar.factor = player.client->radar.factor;
			}
		}
	}

	level.radar_time = level.time + RADAR_TIME;
}

static void G_CheckForEnd()
{
	if (level.time < level.round_end)
	{
		if (level.round_end - level.time <= 11000 && !level.countdown_sound)
		{
			level.countdown_sound = true;
			game.world().PlaySound(gi.soundindex("world/10_0.wav"), CHAN_AUTO, ATTN_NONE);
		}

		return;
	}

	G_TeamWins(TEAM_HIDERS);
}

static void G_WaitForReplay()
{
	if (level.state_time >= level.time)
		return;

	// swap all player teams and clear their state prepped for next respawn
	for (auto &player : game.players)
	{
		if (!player.inuse || !player.client || !player.client->pers.connected || !player.client->resp.ready)
			continue;

		if (player.inuse && player.client->resp.team)
		{
			if (player.client->resp.team == TEAM_HIDERS)
				player.client->resp.team = TEAM_HUNTERS;
			else
				player.client->resp.team = TEAM_HIDERS;
		}

		if (player.control)
			Unpossess(player);

		respawn(player);
	}

	level.state = GAMESTATE_WAITING;

	G_CheckPlayerReady();
}

/*
=================
CheckDMRules
=================
*/
static void CheckDMRules ()
{
	if (level.intermissiontime)
		return;

	if (timelimit->value && level.time >= (timelimit->value * 60) * 1000)
	{
		gi.bprintf (PRINT_HIGH, "Timelimit hit.\n");
		EndDMLevel ();
		return;
	}

	switch (level.state)
	{
	case GAMESTATE_WAITING:
		break;
	case GAMESTATE_SPAWNING:
		G_SpawnMonsters();
		break;
	case GAMESTATE_PLAYING:
		G_CheckForEnd();
		G_UpdateRadars();
		G_KillRandomMonster();
		break;
	case GAMESTATE_INTERMISSION:
		G_WaitForReplay();
		break;
	}

	/*if (fraglimit->value)
	{
		for (size_t i = 0; i < game.maxclients; i++)
		{
			edict_t &cl = g_edicts[i + 1];

			if (!cl.inuse)
				continue;

			if (cl.client->resp.score >= fraglimit->value)
			{
				gi.bprintf (PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel ();
				return;
			}
		}
	}*/
}


/*
=============
ExitLevel
=============
*/
static void ExitLevel ()
{
	gi.AddCommandString(va("gamemap \"%s\"\n", level.changemap));
	level.changemap = nullptr;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();

	// clear some things before going to next level
	for (auto &ent : game.players)
		if (ent.inuse && ent.health > ent.client->pers.max_health)
			ent.health = ent.client->pers.max_health;
}

/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
static void G_RunFrame ()
{
	if (dmflags_cvar->modified)
	{
		dmflags.bits = static_cast<dmflags_bits_t>(dmflags_cvar->value);
		dmflags_cvar->modified = false;
	}

	DrawPoints();

	level.framenum++;
	level.time += FRAME_MS;

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
	for (auto &ent : game.entities)
	{
		if (!ent.inuse)
			continue;

		level.current_entity = ent;

		ent.s.old_origin = ent.s.origin;

		// if the ground entity moved, make sure we are still on it
		if (ent.groundentity && (ent.groundentity->linkcount != ent.groundentity_linkcount))
		{
			ent.groundentity = nullptr;

			if (ent.svflags & SVF_MONSTER)
				M_CheckGround (ent);
		}

		if (ent.client)
			ClientBeginServerFrame (ent);
		else
			G_RunEntity (ent);

		level.current_entity = nullptr;
	}

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// see if needpass needs updated
	CheckNeedPass ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();
}

game_export_t globals = {
	.apiversion = 3,

	.Init = InitGame,
	.Shutdown = ShutdownGame,

	.SpawnEntities = SpawnEntities,

	.WriteGame = [](const char *, qboolean){},
	.ReadGame = [](const char *){},

	.WriteLevel = [](const char *){},
	.ReadLevel = [](const char *){},
	
	.ClientConnect = ClientConnect,
	.ClientBegin = ClientBegin,
	.ClientUserinfoChanged = ClientUserinfoChanged,
	.ClientDisconnect = ClientDisconnect,
	.ClientCommand = ClientCommand,
	.ClientThink = ClientThink,

	.RunFrame = G_RunFrame,

	.ServerCommand = ServerCommand
};

/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/

extern "C"
{
#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
game_export_t *GetGameAPI (game_import_t *import)
{
	gi = *import;
	return &globals;
}
#pragma clang diagnostic pop
};

trace_t game_import_t::trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, const edict_ref &passent, const brushcontents_t &contentmask)
{
	trace_t tr = _trace(start, mins, maxs, end, passent, contentmask);

	if (tr.startsolid || tr.allsolid || tr.contents || tr.ent != g_edicts[0] || tr.fraction < 1.0f || tr.surface || tr.endpos != end)
		return tr;

	// might be a runaway trace
	return _trace(start, mins, maxs, end, passent, contentmask);
}

void game_import_t::WriteEntity(const edict_t &ent)
{
	WriteShort(ent.s.number);
}