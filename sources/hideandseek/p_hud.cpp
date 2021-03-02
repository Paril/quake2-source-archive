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

void MoveClientToIntermission (edict_t *ent)
{
	if (deathmatch->floatVal || coop->floatVal)
		ent->client->showscores = true;
	Vec3Copy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pMove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pMove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pMove.origin[2] = level.intermission_origin[2]*8;
	Vec3Copy (level.intermission_angle, ent->client->ps.viewAngles);
	ent->client->ps.pMove.pmType = PMT_FREEZE;
	ent->client->ps.gunIndex = 0;
	ent->client->ps.viewBlend[3] = 0;
	ent->client->ps.rdFlags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	ent->viewheight = 0;
	ent->s.modelIndex = 0;
	ent->s.modelIndex2 = 0;
	ent->s.modelIndex3 = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	if (deathmatch->floatVal || coop->floatVal)
	{
		DeathmatchScoreboardMessage (ent, NULL);
		gi.unicast (ent, true);
	}

}

void BeginIntermission (edict_t *targ)
{
	int		i;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->floatVal ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inUse)
			continue;
		if (client->health <= 0)
			respawn(client);
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	if (!deathmatch->floatVal)
	{
		level.exitintermission = 1;		// go immediately to the next level
		return;
	}

	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	Vec3Copy (ent->s.origin, level.intermission_origin);
	Vec3Copy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->floatVal ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inUse)
			continue;
		MoveClientToIntermission (client);
	}
}


/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
}


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard (edict_t *ent)
{
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
}


//=======================================================================

/*
===============
G_SetStats
===============

	gi.configstring (CS_ITEMS+1, "Hiding");
	gi.configstring (CS_ITEMS+2, "Seeking");

	gi.configstring (CS_ITEMS+3, "Too Far"); // 4000+
	gi.configstring (CS_ITEMS+4, "Keep Going"); // 3000
	gi.configstring (CS_ITEMS+5, "Getting Closer"); // 2000
	gi.configstring (CS_ITEMS+6, "Almost There"); // 1000
	gi.configstring (CS_ITEMS+7, "Hotter"); // 500
	gi.configstring (CS_ITEMS+8, "You can smell him"); // 250-

	gi.configstring (CS_ITEMS+9, "Above");
	gi.configstring (CS_ITEMS+10, "Below");
	gi.configstring (CS_ITEMS+11, "Same Level");

	gi.configstring (CS_ITEMS+12, "Standing Still");
	gi.configstring (CS_ITEMS+13, "On the Move");

*/
float range_zi (vec3_t origin, vec3_t origin2)
{
	vec3_t ni1, ni2, line;

	Vec3Copy (origin, ni1);
	Vec3Copy (origin2, ni2);
	ni1[2] = ni2[2] = 0;

	Vec3Subtract (ni1, ni2, line);
	return Vec3Length(line);
}
#include <time.h>
void G_SetStats (edict_t *ent)
{
	vec3_t enemyOrigin;

	Vec3Copy (vec3Origin, enemyOrigin);
	//
	// health
	//
	if (level.hiding && (level.seeker == ent))
	{
		float range = range_zi(level.seeker->s.origin, level.hider->s.origin);

		if (range >= 4000)
			ent->client->ps.stats[STAT_HEALTH_ICON] = CS_ITEMS+3;
		else if (range >= 3000)
			ent->client->ps.stats[STAT_HEALTH_ICON] = CS_ITEMS+4;
		else if (range >= 2000)
			ent->client->ps.stats[STAT_HEALTH_ICON] = CS_ITEMS+5;
		else if (range >= 1000)
			ent->client->ps.stats[STAT_HEALTH_ICON] = CS_ITEMS+6;
		else if (range >= 500)
			ent->client->ps.stats[STAT_HEALTH_ICON] = CS_ITEMS+7;
		else
			ent->client->ps.stats[STAT_HEALTH_ICON] = CS_ITEMS+8;


#if 0
		float up = level.hider->s.origin[2] - level.seeker->s.origin[2];
		if (up < -24)
			ent->client->ps.stats[STAT_HEIGHT] = CS_ITEMS+10;
		else if (up > 24)
			ent->client->ps.stats[STAT_HEIGHT] = CS_ITEMS+9;
		else
			ent->client->ps.stats[STAT_HEIGHT] = CS_ITEMS+11;
#else
		if (visible(level.hider, level.seeker) && infront(level.hider, level.seeker))
			ent->client->ps.stats[STAT_HEIGHT] = CS_ITEMS+14;
		else
			ent->client->ps.stats[STAT_HEIGHT] = 0;
#endif

		if (level.hiderMoved)
			ent->client->ps.stats[STAT_MOVING] = CS_ITEMS+13;
		else
			ent->client->ps.stats[STAT_MOVING] = CS_ITEMS+12;
	}
	else
	{
		ent->client->ps.stats[STAT_HEALTH_ICON] = ent->client->ps.stats[STAT_MOVING] = ent->client->ps.stats[STAT_HEIGHT] = 0;
	}

	if (level.hider == ent)
		ent->client->ps.stats[STAT_WHO] = CS_ITEMS+15;
	else if (level.seeker == ent)
		ent->client->ps.stats[STAT_WHO] = CS_ITEMS+16;
	else
		ent->client->ps.stats[STAT_WHO] = CS_ITEMS+17;

	int sec1, sec2, min1, min2;
	char tmp[2];

	sprintf (tmp, "%.2i", (int)level.seconds);
	sec1 = tmp[0] - 48;
	sec2 = tmp[1] - 48;
	sprintf (tmp, "%.2i", (int)level.minutes);
	min1 = tmp[0] - 48;
	min2 = tmp[1] - 48;

	ent->client->ps.stats[STAT_TIMER_SECONDS1] = sec1;
	ent->client->ps.stats[STAT_TIMER_SECONDS2] = sec2;
	ent->client->ps.stats[STAT_TIMER_MINUTES1] = min1;
	ent->client->ps.stats[STAT_TIMER_MINUTES2] = min2;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->floatVal)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

	ent->client->ps.stats[STAT_SPECTATOR] = 0;
}

/*
===============
G_CheckChaseStats
===============
*/
void G_CheckChaseStats (edict_t *ent)
{
	int i;
	gclient_t *cl;

	for (i = 1; i <= maxclients->floatVal; i++) {
		cl = g_edicts[i].client;
		if (!g_edicts[i].inUse || cl->chase_target != ent)
			continue;
		memcpy(cl->ps.stats, ent->client->ps.stats, sizeof(cl->ps.stats));
		G_SetSpectatorStats(g_edicts + i);
	}
}

/*
===============
G_SetSpectatorStats
===============
*/
void G_SetSpectatorStats (edict_t *ent)
{
	gclient_t *cl = ent->client;

	if (!cl->chase_target)
		G_SetStats (ent);

	cl->ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independant in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;
	if (cl->pers.health <= 0 || level.intermissiontime || cl->showscores)
		cl->ps.stats[STAT_LAYOUTS] |= 1;
	if (cl->showinventory && cl->pers.health > 0)
		cl->ps.stats[STAT_LAYOUTS] |= 2;

	if (cl->chase_target && cl->chase_target->inUse)
		cl->ps.stats[STAT_CHASE] = CS_PLAYERSKINS + 
			(cl->chase_target - g_edicts) - 1;
	else
		cl->ps.stats[STAT_CHASE] = 0;
}

