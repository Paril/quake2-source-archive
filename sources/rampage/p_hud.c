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

void MoveClientToIntermission(edict_t *ent)
{
	//if (deathmatch->value || coop->value)
		ent->client->showscores = true;
	VectorCopy(level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0] * 8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1] * 8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2] * 8;
	VectorCopy(level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout


		if (coop->value || !deathmatch->value)
			IntermissionScoreboardMessage(ent);
		else
			DeathmatchScoreboardMessage(ent, NULL);
		gi.unicast(ent, true);
	

}

void think_intermission(edict_t *self)
{
	int i;
	edict_t *ent;

	if(!self->count)
		level.row_dmg_received = STRING_GREEN;

	if (self->count == 1)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);
		level.row_dmg_received = STRING_WHITE;
		level.row_dmg_dealt = STRING_GREEN;
		level.show |= SHOW_DMG_RECEIVED;

	}
	if (self->count == 2)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);
		level.row_dmg_dealt = STRING_WHITE;
		level.row_dmg_saved = STRING_GREEN;
		level.show |= SHOW_DMG_DEALT;
	}
	if (self->count == 3)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);
		level.row_dmg_saved = STRING_WHITE;
		level.row_kills = STRING_GREEN;
		level.show |= SHOW_DMG_SAVED;
	}
	if (self->count == 4)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);
		level.row_kills = STRING_WHITE;
		level.row_item_pickup = STRING_GREEN;
		level.show |= SHOW_KILLS;
	}
	if (self->count == 5)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);
		level.row_item_pickup = STRING_WHITE;
		level.row_item_usage = STRING_GREEN;
		level.show |= SHOW_ITEM_PICKUP;
	}
	if (self->count == 6)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);
		level.row_item_usage = STRING_WHITE;
		level.row_health_bonus = STRING_GREEN;
		level.show |= SHOW_ITEM_USAGE;
	}
	if (self->count == 7)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);
		level.row_health_bonus = STRING_WHITE;
		level.row_objectives = STRING_GREEN;
		level.show |= SHOW_HEALTH_BONUS;
	}
	if (self->count == 8)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);
		level.row_objectives = STRING_WHITE;
		level.row_secrets = STRING_GREEN;
		level.show |= SHOW_OBJECTIVES;
	}
	if (self->count == 9)
	{
		//gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);
		level.row_secrets = STRING_WHITE;
		level.show |= SHOW_SECRETS;
	}
	if (self->count == 11)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);

		level.row_total = STRING_GREEN;

		level.show |= SHOW_TOTAL;
		for (i = 0; i < maxclients->value; i++)
		{
			ent = g_edicts + 1 + i;
			if (!ent->inuse)
				continue;

			ent->client->resp.score_total = ent->client->resp.score_dmg_received +
												ent->client->resp.score_dmg_dealt +
												ent->client->resp.score_dmg_saved +
												ent->client->resp.score_kills +
												ent->client->resp.score_item_pickup +
												ent->client->resp.score_item_usage +
												ent->client->resp.score_health_bonus +
												ent->client->resp.score_objectives +
												ent->client->resp.score_secrets;
				ent->client->resp.score_dmg_received = 0;
				ent->client->resp.score_dmg_dealt = 0;
				ent->client->resp.score_dmg_saved = 0;
				ent->client->resp.score_kills = 0;
				ent->client->resp.score_item_pickup = 0;
				ent->client->resp.score_item_usage = 0;
				ent->client->resp.score_health_bonus = 0;
				ent->client->resp.score_objectives = 0;
				ent->client->resp.score_secrets = 0;
		}

	}
	if (self->count == 13)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);

		level.show |= SHOW_TOTAL;
		for (i = 0; i < maxclients->value; i++)
		{
			ent = g_edicts + 1 + i;
			if (!ent->inuse)
				continue;

			ent->client->resp.score_total = ent->client->resp.score;
		}

	}
	for (i = 0; i < maxclients->value; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;

		IntermissionScoreboardMessage(ent);
		gi.unicast(ent, false);

	}


	//gi.bprintf(PRINT_HIGH, "INTERMISSION THINK: uhm, it's working??");
	self->nextthink += level.time + 1;
	self->count++;
}

void BeginIntermission(edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

	game.autosaved = false;

	// respawn any dead clients
	for (i = 0; i < maxclients->value; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		if (client->health <= 0)
			respawn(client);
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	if (strstr(level.changemap, "*"))
	{
		if (coop->value)
		{
			for (i = 0; i < maxclients->value; i++)
			{
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				// strip players of all keys between units
				for (n = 0; n < MAX_ITEMS; n++)
				{
					if (itemlist[n].flags & IT_KEY)
						client->client->pers.inventory[n] = 0;
				}
			}
		}
	}
	else
	{
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}

	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find(NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find(NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find(ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find(ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy(ent->s.origin, level.intermission_origin);
	VectorCopy(ent->s.angles, level.intermission_angle);

	ent->nextthink = level.time + 0.1;
	ent->think = think_intermission;


	// move all clients to the intermission point
	for (i = 0; i < maxclients->value; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission(client);
	}
}


/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage(edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;

	// sort the clients by score
	total = 0;
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
			continue;
		score = game.clients[i].resp.score;
		for (j = 0; j < total; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k = total; k > j; k--)
		{
			sorted[k] = sorted[k - 1];
			sortedscores[k] = sortedscores[k - 1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
	string[0] = 0;

	stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 12)
		total = 12;

	for (i = 0; i < total; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		picnum = gi.imageindex("i_fixme");
		x = (i >= 6) ? 160 : 0;
		y = 32 + 32 * (i % 6);

		// add a dogtag
		if (cl_ent == ent)
			tag = "tag1";
		else if (cl_ent == killer)
			tag = "tag2";
		else
			tag = NULL;
		if (tag)
		{
			Com_sprintf(entry, sizeof(entry),
				"xv %i yv %i picn %s ", x + 32, y, tag);
			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy(string + stringlength, entry);
			stringlength += j;
		}

		// send the layout
		Com_sprintf(entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe) / 600);
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy(string + stringlength, entry);
		stringlength += j;
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}

void IntermissionScoreboardMessage(edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		picnum;
	int		x, y, x_base;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;
	char    *dmg_received;
	char    *dmg_dealt;
	char	*dmg_saved;
	char	*kills;
	char	*item_pickup;
	char	*item_usage;
	char	*health_bonus;
	char	*objectives;
	char	*secrets;
	char	*total_score;


	if (level.row_dmg_received)
		dmg_received = "string2";
	else
		dmg_received = "string";

	if (level.row_dmg_dealt)
		dmg_dealt = "string2";
	else
		dmg_dealt = "string";

	if (level.row_dmg_saved)
		dmg_saved = "string2";
	else
		dmg_saved = "string";

	if (level.row_kills)
		kills = "string2";
	else
		kills = "string";

	if (level.row_item_pickup)
		item_pickup = "string2";
	else
		item_pickup = "string";

	if (level.row_item_usage)
		item_usage = "string2";
	else
		item_usage = "string";

	if (level.row_health_bonus)
		health_bonus = "string2";
	else
		health_bonus = "string";

	if (level.row_objectives)
		objectives = "string2";
	else
		objectives = "string";

	if (level.row_objectives)
		objectives = "string2";
	else
		objectives = "string";

	if (level.row_secrets)
		secrets = "string2";
	else
		secrets = "string";

	if (level.row_total)
		total_score = "string2";
	else
		total_score = "string";
		
	// sort the clients by score
	total = 0;
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
			continue;
		score = game.clients[i].resp.score;
		for (j = 0; j < total; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k = total; k > j; k--)
		{
			sorted[k] = sorted[k - 1];
			sortedscores[k] = sortedscores[k - 1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
	string[0] = 0;

	stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 12)
		total = 12;
	x_base = 0;

	for (i = 0; i < total; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];


		x = x_base + (128 * i);
		y = -32;// 32 + 32 * (i % 6);

		// add a dogtag
		if (cl_ent == ent)
			tag = "tag1";
		else
			tag = "tag2";

		tag = "tag1";

		if (tag)
		{
			Com_sprintf(entry, sizeof(entry),
				"xv %i yv %i picn %s "
				"xv %i yv %i string2 \" %s \" "
				"xv %i yv %i %s \" %i \" "
				"xv %i yv %i %s \" %i \" "
				"xv %i yv %i %s \" %i \" "
				"xv %i yv %i %s \" %i \" "
				"xv %i yv %i %s \" %i \" "
				"xv %i yv %i %s \" %i \" "
				"xv %i yv %i %s \" %i \" "
				"xv %i yv %i %s \" %i \" "
				"xv %i yv %i %s \" %i \" "
				"xv %i yv %i %s \" %i \" "
				, x, y + 32, tag, x, y + 32, (int)cl_ent->client->pers.netname,
				x, y + 64, dmg_received, (int)cl_ent->client->resp.score_dmg_received * (level.show & SHOW_DMG_RECEIVED),
				x, y + 80, dmg_dealt, (int)cl_ent->client->resp.score_dmg_dealt * (level.show & SHOW_DMG_DEALT),
				x, y + 96, dmg_saved, (int)cl_ent->client->resp.score_dmg_saved * (level.show & SHOW_DMG_SAVED),
				x, y + 112, kills, (int)cl_ent->client->resp.score_kills * (level.show & SHOW_KILLS),
				x, y + 128, item_pickup, (int)cl_ent->client->resp.score_item_pickup * (level.show & SHOW_ITEM_PICKUP),
				x, y + 146, item_usage, (int)cl_ent->client->resp.score_item_usage * (level.show & SHOW_ITEM_USAGE),
				x, y + 162, health_bonus, (int)cl_ent->client->resp.score_health_bonus * (level.show & SHOW_HEALTH_BONUS),
				x, y + 178, objectives, (int)(cl_ent->client->resp.score_objectives * 0.0078125 *(level.show & SHOW_OBJECTIVES)),
				x, y + 196, secrets, (int)(cl_ent->client->resp.score_secrets * 0.00390625 * (level.show & SHOW_SECRETS)),
				x, y + 228, total_score, (int)(cl_ent->client->resp.score_total * 0.00390625 * (level.show & SHOW_TOTAL)))
				;

			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy(string + stringlength, entry);
			stringlength += j;

		}

		// send the layout



		/*x, y + 32, ent->client->resp.score_dmg_receive, x, y + 40, ent->client->resp.score_dmg_dealt, x, y + 48, ent->client->resp.score_dmg_saved,
			x + 56, y, ent->client->resp.score_item_pickup, x + 64, y, ent->client->resp.score_item_usage, x + 72, y, ent->client->resp.score_health_bonus,
			x + 80, y, ent->client->resp.score_objectives, x + 88, y, ent->client->resp.score_secrets);*/
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy(string + stringlength, entry);
		stringlength += j;
	}
	x = -128;
	y = 0;
	Com_sprintf(entry, sizeof(entry),
		"xv %i yv %i %s \" DMG RECEIVED \" "
		"xv %i yv %i %s \" DMG DEALT \" "
		"xv %i yv %i %s \" DMG SAVED \" "
		"xv %i yv %i %s \" KILLS \" "
		"xv %i yv %i %s \" ITEM PICKUP \" "
		"xv %i yv %i %s \" ITEM USAGE \" "
		"xv %i yv %i %s \" HEALTH BONUS \" "
		"xv %i yv %i %s \" OBJECTIVES \" "
		"xv %i yv %i %s \" SECRETS \" "
		"xv %i yv %i %s \" TOTAL \" ",
		x, y + 32, dmg_received,
		x, y + 48, dmg_dealt,
		x, y + 64, dmg_saved,
		x, y + 80, kills,
		x, y + 96, item_pickup,
		x, y + 112, item_usage,
		x, y + 128, health_bonus,
		x, y + 146, objectives,
		x, y + 164, secrets,
		x, y + 198, total_score
	);
	j = strlen(entry);
	if (stringlength + j > 1024)
		return;
	strcpy(string + stringlength, entry);
	stringlength += j;
	gi.WriteByte(svc_layout);
	gi.WriteString(string);

}
/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard(edict_t *ent)
{
	DeathmatchScoreboardMessage(ent, ent->enemy);
	gi.unicast(ent, true);
}

void IntermissionScoreboard(edict_t *ent)
{
	IntermissionScoreboardMessage(ent);
	gi.unicast(ent, true);
}

/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f(edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		return;
	}

	ent->client->showscores = true;
	DeathmatchScoreboard (ent);
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer(edict_t *ent)
{
	char	string[1024];
	char	*sk;

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else if (skill->value == 3)
		sk = "hard+";
	else if (skill->value == 4)
		sk = "cerb";
	else if (skill->value == 5)
		sk = "cerb+";
	else if (skill->value == 6)
		sk = "cerb++";
	else
		sk = "cerb+x";
	// send the layout
	Com_sprintf(string, sizeof(string),
		"xv 32 yv 8 picn help "			// background
		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s\" "		// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 110 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ",
		sk,
		level.level_name,
		game.helpmessage1,
		game.helpmessage2,
		level.killed_monsters, level.total_monsters,
		level.found_goals, level.total_goals,
		level.found_secrets, level.total_secrets);

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
	gi.unicast(ent, true);
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f(edict_t *ent)
{
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f(ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = false;

	if (ent->client->showhelp && (ent->client->pers.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->pers.helpchanged = 0;
	HelpComputer(ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats(edict_t *ent)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;

	//
	// health
	//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	//
	// ammo
	//
	if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex(item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}

	//
	// armor
	//
	power_armor_type = PowerArmorType(ent);
	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))];
		if (cells == 0)
		{	// ran out of cells for power armor
			ent->flags &= ~FL_POWER_ARMOR;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;;
		}
	}

	index = ArmorIndex(ent);
	if (power_armor_type && (!index || (level.framenum & 8)))
	{	// flash between power armor and other armor icon
		// Knightmare- use correct icon for power screen
		if (power_armor_type == POWER_ARMOR_SHIELD)
			ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex("i_powershield");
		else	// POWER_ARMOR_SCREEN
			ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex("i_powerscreen");
		ent->client->ps.stats[STAT_ARMOR] = cells;
	}
	else if (index)
	{
		item = GetItemByIndex(index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex(item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}

	//
	// pickup message
	//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// timers
	//
	if (item_mod->value)
	{
		if (ent->client->pers.quad_health && ent->client->pers.items_activated & FL_QUAD)
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_quad");
			ent->client->ps.stats[STAT_TIMER] = ent->client->pers.quad_health / 10;
		}
		else if (ent->client->pers.invu_health && ent->client->pers.items_activated & FL_INV)
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_invulnerability");
			ent->client->ps.stats[STAT_TIMER] = ent->client->pers.invu_health / 10;
		}
		else if (ent->client->pers.envi_health && ent->client->pers.items_activated & FL_ENV)
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_envirosuit");
			ent->client->ps.stats[STAT_TIMER] = ent->client->pers.envi_health / 10;
		}
		else if (ent->client->pers.rebr_health && ent->client->pers.items_activated & FL_REB)
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_rebreather");
			ent->client->ps.stats[STAT_TIMER] = ent->client->pers.rebr_health / 10;
		}
		else if (ent->client->pers.sile_health)
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_silencer");
			ent->client->ps.stats[STAT_TIMER] = ent->client->pers.sile_health / 10;
		}
		else
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = 0;
			ent->client->ps.stats[STAT_TIMER] = 0;
		}
	}
	else
	{
		if (ent->client->quad_framenum > level.framenum)
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_quad");
			ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum) / 10;
		}
		else if (ent->client->invincible_framenum > level.framenum)
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_invulnerability");
			ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum) / 10;
		}
		else if (ent->client->enviro_framenum > level.framenum)
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_envirosuit");
			ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum) / 10;
		}
		else if (ent->client->breather_framenum > level.framenum)
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_rebreather");
			ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum) / 10;
		}
		else
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = 0;
			ent->client->ps.stats[STAT_TIMER] = 0;
		}
	}
	//
	// selected item
	//
	if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex(itemlist[ent->client->pers.selected_item].icon);

	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
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

	//
	// frags
	//
	if (deathmatch->value)
		ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;
	else
		ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// help icon / current weapon if not shown
	//
	if (ent->client->pers.helpchanged && (level.framenum & 8))
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex("i_help");
	else if ((ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex(ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

	ent->client->ps.stats[STAT_SPECTATOR] = 0;

	if (ent->client->pers.jump_charge < jump_max_charge->value - (jump_max_charge->value * 0.3333) * 2)
	{
		ent->client->ps.stats[STAT_JUMP_ICON] = gi.imageindex("jump_i_r");
		ent->client->ps.stats[STAT_JUMP_HUD] = gi.imageindex("j_r");
	}
	else if (ent->client->pers.jump_charge < (jump_max_charge->value - (jump_max_charge->value / 3)))
	{
		ent->client->ps.stats[STAT_JUMP_ICON] = gi.imageindex("jump_i_y");
		ent->client->ps.stats[STAT_JUMP_HUD] = gi.imageindex("j_y");
	}
	else if (ent->client->pers.jump_charge == jump_max_charge->value)
	{
		ent->client->ps.stats[STAT_JUMP_ICON] = gi.imageindex("jump_i_g");
		ent->client->ps.stats[STAT_JUMP_HUD] = gi.imageindex("j_g");

	}

	if (ent->client->pers.items_activated & FL_ENV || ent->client->pers.items_activated & FL_REB)
	{
		ent->client->ps.stats[STAT_MASK] = 1;

	}
	else
	{
		ent->client->ps.stats[STAT_MASK] = 0;

	}

}

/*
===============
G_CheckChaseStats
===============
*/
void G_CheckChaseStats(edict_t *ent)
{
	int i;
	gclient_t *cl;

	for (i = 1; i <= maxclients->value; i++) {
		cl = g_edicts[i].client;
		if (!g_edicts[i].inuse || cl->chase_target != ent)
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
void G_SetSpectatorStats(edict_t *ent)
{
	gclient_t *cl = ent->client;

	if (!cl->chase_target)
		G_SetStats(ent);

	cl->ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independant in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;
	if (cl->pers.health <= 0 || level.intermissiontime || cl->showscores)
		cl->ps.stats[STAT_LAYOUTS] |= 1;
	if (cl->showinventory && cl->pers.health > 0)
		cl->ps.stats[STAT_LAYOUTS] |= 2;

	if (cl->chase_target && cl->chase_target->inuse)
		cl->ps.stats[STAT_CHASE] = CS_PLAYERSKINS +
		(cl->chase_target - g_edicts) - 1;
	else
		cl->ps.stats[STAT_CHASE] = 0;
}

