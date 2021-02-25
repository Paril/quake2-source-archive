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

/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t &ent)
{
	gclient_t &client = *ent.client;

	client.showscores = true;
	client.ps.pmove.origin[0] = level.intermission_origin[0]*8;
	client.ps.pmove.origin[1] = level.intermission_origin[1]*8;
	client.ps.pmove.origin[2] = level.intermission_origin[2]*8;
	client.ps.viewangles = level.intermission_angle;
	client.ps.pmove.pm_type = PM_FREEZE;
	client.ps.gunindex = MODEL_NONE;
	client.ps.blend[3] = 0;
	client.ps.rdflags &= ~RDF_UNDERWATER;
	
	ent.s.origin = level.intermission_origin;
	ent.viewheight = 0;
	ent.s.modelindex = MODEL_NONE;
	ent.s.modelindex2 = MODEL_NONE;
	ent.s.modelindex3 = MODEL_NONE;
	ent.s.modelindex4 = MODEL_NONE;
	ent.s.effects = EF_NONE;
	ent.s.sound = SOUND_NONE;
	ent.solid = SOLID_NOT;

	// add the layout
	DeathmatchScoreboardMessage (ent, nullptr);
	ent.Unicast(true);
}

void BeginIntermission (edict_t &targ)
{
	if (level.intermissiontime)
		return;		// already activated

	// respawn any dead clients
	for (auto &client : game.players)
		if (client.inuse && client.health <= 0)
			respawn(client);

	level.intermissiontime = level.time;
	level.changemap = targ.map;
	level.exitintermission = 0;

	// find an intermission spot
	edict_ref ent = G_Find (nullptr, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (nullptr, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (nullptr, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		int32_t i = irandom(3);

		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	level.intermission_origin = ent->s.origin;
	level.intermission_angle = ent->s.angles;

	// move all clients to the intermission point
	for (auto &player : game.players)
		if (player.inuse)
			MoveClientToIntermission (player);
}


/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage (edict_t &client, const edict_ref &killer)
{
	// sort the clients by score
	int32_t		total = 0;
	int32_t		sorted[MAX_CLIENTS];
	int32_t		sortedscores[MAX_CLIENTS];

	for (auto &player : game.players)
	{
		if (!player.inuse)
			continue;

		gclient_t &cl = *player.client;

		if (cl.resp.spectator)
			continue;

		const int32_t score = cl.resp.score;
		int32_t j, k;

		for (j = 0; j < total; j++)
			if (score > sortedscores[j])
				break;

		for (k = total; k > j; k--)
		{
			sorted[k] = sorted[k - 1];
			sortedscores[k] = sortedscores[k - 1];
		}

		sorted[j] = player.s.number;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
	char string[1400] = { 0 };
	size_t stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 12)
		total = 12;

	for (int32_t i = 0; i < total; i++)
	{
		const gclient_t &cl = game.clients[sorted[i] - 1];
		const edict_t &cl_ent = g_edicts[sorted[i]];
		const int32_t x = (i>=6) ? 160 : 0;
		const int32_t y = 32 + 32 * (i%6);
		const char *tag = nullptr;
		char entry[1024];

		// add a dogtag
		if (cl_ent == client)
			tag = "tag1";
		else if (cl_ent == killer)
			tag = "tag2";

		if (tag)
		{
			snprintf (entry, sizeof(entry), "xv %i yv %i picn %s ", x + 32, y, tag);

			const size_t j = strlen(entry);
			if (stringlength + j > 1024)
				break;

			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		// send the layout
		snprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i] - 1, cl.resp.score, cl.ping, static_cast<int32_t>((level.framenum - cl.resp.enterframe) / 600));

		const size_t j = strlen(entry);
		if (stringlength + j > 1024)
			break;

		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	gi.WriteByte (SVC_LAYOUT);
	gi.WriteString (string);
}


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 uint8_t message limit!
==================
*/
static void DeathmatchScoreboard (edict_t &ent)
{
	DeathmatchScoreboardMessage (ent, ent.enemy);
	ent.Unicast(true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
static void Cmd_Score_f (edict_t &ent)
{
	if (ent.client->showscores)
	{
		ent.client->showscores = false;
		return;
	}

	ent.client->showscores = true;
	DeathmatchScoreboard (ent);
}

/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t &ent)
{
	// this is for backwards compatability
	Cmd_Score_f (ent);
}

//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t &ent)
{
	gclient_t &client = *ent.client;

	client.ps.stats[STAT_HUNTER] = client.ps.stats[STAT_HIDER] = 0;
	client.ps.stats[STAT_CONTROL] = client.ps.stats[STAT_RADAR] = 0;

	if (roundlimit->value)
	{
		if (level.state == GAMESTATE_PLAYING)
		{
			const gtime_t time_left = (level.round_end - level.time) / 1000;
			const gtime_t m = time_left / 60;
			const gtime_t s = time_left % 60;
		
			gi.configstring(CS_ROUND_STATUS, va("ROUND TIME: %02i:%02i", static_cast<int32_t>(m), static_cast<int32_t>(s)));
		}
		
		if (level.state >= GAMESTATE_SPAWNING)
			client.ps.stats[STAT_ROUND_TIMER] = CS_ROUND_STATUS;
	}

	if (!client.resp.spectator)
	{
		switch (client.resp.team)
		{
		case TEAM_HIDERS:
			client.ps.stats[STAT_HIDER] = 1;

			if (level.control_delay > level.time)
			{
				client.last_num_jumps = 0;
				gi.configstring(CS_GAME_STATUS, va("You'll take control in %i...", static_cast<int32_t>((level.control_delay - level.time) / 1000)));
			}
			else
			{
				if (client.last_num_jumps != client.num_jumps)
				{
					gi.WriteByte(SVC_CONFIGSTRING);
					gi.WriteShort(CS_GAME_STATUS);
					gi.WriteString(va("STROGG-SHIFTS LEFT: %i", client.num_jumps));
					gi.unicast(ent, true);
					client.last_num_jumps = client.num_jumps;
				}
			
				client.ps.stats[STAT_CONTROL] = static_cast<int16_t>(CS_GAME_STATUS);

				if (client.radar.last_factor != client.radar.factor)
				{
					const char *radar_status;
					const char *sound;
					const uint32_t percent = static_cast<uint32_t>(client.radar.factor * 100.f);

					gi.WriteByte(SVC_CONFIGSTRING);
					gi.WriteShort(CS_RADAR_STATUS);
					switch (client.radar.status)
					{
					case RADAR_EMPTY:
					default:
						radar_status = "Not detected";
						sound = "world/scan1.wav";
						break;
					case RADAR_STAGE_1:
						radar_status = va("%u%% signal strength", percent);
						sound = "world/lite_on2.wav";
						break;
					case RADAR_STAGE_2:
						radar_status = va("%u%% signal strength", percent);
						sound = "world/lite_on2.wav";
						break;
					case RADAR_STAGE_3:
						radar_status = va("Locked on! %u%% signal strength", percent);
						sound = "world/klaxon2.wav";
						break;
					}
					gi.WriteString(radar_status);
					gi.unicast(ent, true);

					if (client.radar.last_status != client.radar.status)
						client.SendSound(gi.soundindex(sound));

					client.radar.last_status = client.radar.status;
					client.radar.last_factor = client.radar.factor;
				}

				client.ps.stats[STAT_RADAR] = static_cast<int16_t>(CS_RADAR_STATUS);
			}

			client.ps.stats[STAT_CONTROL] = static_cast<int16_t>(CS_GAME_STATUS);
			break;
		case TEAM_HUNTERS:
			client.ps.stats[STAT_HUNTER] = 1;

			if (level.time >= level.control_delay)
			{
				const gtime_t radar_left = min(5ull, (level.radar_time - level.time) / 1000);

				if (client.radar.last_factor != client.radar.factor || level.last_radar_left != radar_left)
				{
					const char *radar_status;
					const char *sound;
					const uint32_t percent = static_cast<uint32_t>(clamp(client.radar.factor, 0.0f, 1.0f) * 100.f);

					gi.WriteByte(SVC_CONFIGSTRING);
					gi.WriteShort(CS_RADAR_STATUS);
					switch (client.radar.status)
					{
					case RADAR_EMPTY:
					default:
						radar_status = "No signal";
						sound = "world/scan1.wav";
						break;
					case RADAR_STAGE_1:
						radar_status = va("%u%% signal strength...", clamp(percent, 5u, 25u));
						sound = "world/lite_on2.wav";
						break;
					case RADAR_STAGE_2:
						radar_status = va("%u%% signal strength...", clamp(percent, 50u, 75u));
						sound = "world/lite_on2.wav";
						break;
					case RADAR_STAGE_3:
						radar_status = va("Lock Acquired! They're close..."/*, client.radar.entity->control ? client.radar.entity->control->monsterinfo.name : "Human"*/);
						sound = "world/fusein.wav";
						break;
					}
					radar_status = va("%s (ping in %u...)", radar_status, static_cast<int32_t>(radar_left));
					level.last_radar_left = radar_left;
					gi.WriteString(radar_status);
					gi.unicast(ent, true);

					if (client.radar.last_status != client.radar.status)
						client.SendSound(gi.soundindex(sound));

					client.radar.last_status = client.radar.status;
					client.radar.last_factor = client.radar.factor;
				}

				client.ps.stats[STAT_RADAR] = static_cast<int16_t>(CS_RADAR_STATUS);
			}
			break;
		case TEAM_NONE:
			break;
		}
	}

	//
	// health
	//
	client.ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	client.ps.stats[STAT_HEALTH] = static_cast<int16_t>(ent.control ? ent.control->health : ent.health);

	//
	// ammo
	//
	client.ps.stats[STAT_AMMO_ICON] = 0;
	client.ps.stats[STAT_AMMO] = 0;
	
	//client.ps.stats[STAT_BULLETS] = static_cast<int16_t>(client.pers.ammo[AMMO_BULLETS]);
	client.ps.stats[STAT_SHELLS] = static_cast<int16_t>(client.pers.ammo[AMMO_SHELLS]);
	client.ps.stats[STAT_GRENADES] = static_cast<int16_t>(client.pers.ammo[AMMO_GRENADES]);

	if (client.resp.team == TEAM_HUNTERS && client.pers.weapon && client.pers.weapon->ammo != AMMO_NONE)
	{
		client.ps.stats[STAT_AMMO_ICON] = gi.imageindex (client.pers.weapon->icon);

		//if (client.pers.weapon->ammo != AMMO_NONE)
			client.ps.stats[STAT_AMMO] = static_cast<int16_t>(client.pers.ammo[client.pers.weapon->ammo]);
	}

	//
	// layouts
	//
	client.ps.stats[STAT_LAYOUTS] = 0;

	if (client.pers.health <= 0 || level.intermissiontime || client.showscores)
		client.ps.stats[STAT_LAYOUTS] |= 1;

	//
	// frags
	//
	client.ps.stats[STAT_FRAGS] = static_cast<int16_t>(client.resp.score);

	client.ps.stats[STAT_SPECTATOR] = 0;
}

/*
===============
G_CheckChaseStats
===============
*/
void G_CheckChaseStats (edict_t &ent)
{
	for (auto &player : game.players)
	{
		gclient_t &cl = *player.client;

		if (!player.inuse || cl.chase_target != ent)
			continue;
		
		cl.ps.stats = ent.client->ps.stats;
		G_SetSpectatorStats(player);
	}
}

/*
===============
G_SetSpectatorStats
===============
*/
void G_SetSpectatorStats (edict_t &ent)
{
	gclient_t &cl = *ent.client;

	if (!cl.chase_target)
		G_SetStats (ent);

	cl.ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independant in spectator
	cl.ps.stats[STAT_LAYOUTS] = 0;
	if (cl.pers.health <= 0 || level.intermissiontime || cl.showscores)
		cl.ps.stats[STAT_LAYOUTS] |= 1;

	if (cl.chase_target && cl.chase_target->inuse)
		cl.ps.stats[STAT_CHASE] = static_cast<int16_t>(CS_PLAYERSKINS + cl.chase_target->s.number - 1);
	else
		cl.ps.stats[STAT_CHASE] = 0;
}

