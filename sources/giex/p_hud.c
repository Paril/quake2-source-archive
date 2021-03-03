#include "g_local.h"
#define GIEX_HELPPAGES 7
#define GIEX_COMMANDPAGES 3



/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	if (deathmatch->value || coop->value) {
		closeGiexMenu(ent);
		ent->client->showscores = true;
	}
	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
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

	if (deathmatch->value || coop->value)
	{
		DeathmatchScoreboardMessage (ent, NULL);
		gi.unicast (ent, true);
	}

}

void BeginIntermission (edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->value ; i++)
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
			for (i=0 ; i<maxclients->value ; i++)
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
	ent = G_FindClassId (NULL, CI_P_INTERMISSION);
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_FindClassId (NULL, CI_P_START);
		if (!ent)
			ent = G_FindClassId (NULL, CI_P_DEATHMATCH);
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_FindClassId (ent, CI_P_INTERMISSION);
			if (!ent)	// wrap around the list
				ent = G_FindClassId (ent, CI_P_INTERMISSION);
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		//int sum = 0;
		//char strnewskill[8];

		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}
}


/*
==================
DeathmatchScoreboardMessage

==================
*/
void showGiexMenu (edict_t *ent);
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength = 0;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	//int		picnum;
	int		y = 32;
	gclient_t	*cl;
	edict_t		*cl_ent;

	if (ent->client->showmenu > 0) {
		showGiexMenu(ent);
		return;
	}
	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse /*|| game.clients[i].resp.spectator*/)
			continue;
		score = cl_ent->radius_dmg; /*game.clients[i].resp.score;*/
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	Com_sprintf(entry, sizeof(entry),
      "xv 32 yv 16 string2 \"Player\" "
      "xv 178 yv 16 string2 \"Lvl\" "
      "xv 208 yv 16 string2 \"Ping\" "
      "xv 248 yv 16 string2 \"Team\" "
      "xv 32 yv 24 string2 \"--------------------------------\" ");
	j = strlen(entry);
	if (stringlength + j < 1024) {
      strcpy (string + stringlength, entry);
      stringlength += j;
	}
	// print level name and exit rules
	//string[0] = 0;

	//stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 20)
		total = 20;

	for (i=0 ; i<total ; i++) {
		char *team;
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		if (cl_ent->client->pers.spectator || cl_ent->client->resp.spectator) {
			team = "Spec";
		} else if (teams->value == 0) { // FFA
			team = " -";
		} else if ((teams->value == 1) || (teams->value == 2)) { // Tdm and Dom
			if (level.framenum < 151) {
				team = " -";
			} else {
				if (cl_ent->count == 1) {
					team = "Red";
				} else if (cl_ent->count == 2) {
					team = "Green";
				} else {
					team = "None";
				}
			}
		} else if (teams->value == 3) { // PvM
			team = "Plr";
		} else {
			team = "Wtf?";
		}
		y = 32 + 8 * i;

		Com_sprintf(entry, sizeof(entry),
	      "xv 32 yv %i string2 \"%s\" "
   	   "xv 168 yv %i string \"%4i %4i %s\" ",
      	y, cl->pers.netname,
      	y, cl_ent->radius_dmg,
      	cl->ping,
      	team);

		j = strlen(entry);
		if (stringlength + j > 1024)
	      break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}
	y += 16;
	Com_sprintf(entry, sizeof(entry),
		"xv 32 yv %i string2 \"Time:\" "
		"xv 80 yv %i string \"%i\" ",
		y, y, (int) (level.framenum / 600));

	j = strlen(entry);
	if (stringlength + j < 1024) {
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
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
	DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent) {
	ent->client->showinventory = false;
	ent->client->showhelp = false;

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores) {
		closeGiexMenu(ent);
		return;
	}

	closeGiexMenu(ent);
	ent->client->showscores = true;
	DeathmatchScoreboard (ent);
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";

	// send the layout
	Com_sprintf (string, sizeof(string),
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

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}

/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
	// this is for backwards compatability
	if ((deathmatch->value) || (ent->client->showhelp) || (ent->client->showscores)) {
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	closeGiexMenu(ent);

	if (ent->client->showhelp && (ent->client->pers.game_helpchanged == game.helpchanged)) {
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->pers.helpchanged = 0;
	HelpComputer (ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent) {
	gitem_t		*item;
	int			index, cells = 0;
	int			power_armor_type;
	trace_t tr;
	vec3_t	forward, offset, end;
	vec3_t	box1, box2;

	//
	// health
	//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	//
	// ammo
	//
	if (!ent->client->ammo_index) {
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	} else {
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}

	//
	// armor
	//
	power_armor_type = PowerArmorType (ent);
	if (power_armor_type) {
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))] - (ent->client->pers.max_cells - 200);
		if (cells == 0)
		{	// ran out of cells for power armor
			ent->flags &= ~FL_POWER_ARMOR;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;;
		}
	}

	index = ITEM_INDEX(FindItem("Body Armor"));
	if (power_armor_type && (!index || (level.framenum & 8) ) ) {	// flash between power armor and other armor icon
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
		ent->client->ps.stats[STAT_ARMOR] = cells;
	} else if (index) {
		item = GetItemByIndex (index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	} else {
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}

	//
	// pickup message
	//
	if (level.time > ent->client->pickup_msg_time) {
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// timers
	//
	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum)/10;
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum)/10;
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum)/10;
	}
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
	}

	//
	// selected item
	//
	if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);

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
	// Character level
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->radius_dmg;

	//
	// help icon / current weapon if not shown
	//
	if (ent->client->pers.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

	ent->client->ps.stats[STAT_SPECTATOR] = 0;

	if (ent->client->pers.health >= 0)
		ent->client->ps.stats[STAT_POWERUP] = ent->client->pers.expRemain;

	if (ent->client->pers.health >= 0)
		ent->client->ps.stats[STAT_MAGIC] = ent->client->magic;

	//
	// Game mode and team name
	//

	if (coop->value) { // coop
		ent->client->ps.stats[STAT_GAMEMODE] = CS_GAMEMODES;
		ent->client->ps.stats[STAT_TEAM] = 0;
	} else if (deathmatch->value) {
		if (!teams->value) { // FFA
			ent->client->ps.stats[STAT_GAMEMODE] = CS_GAMEMODES + 1;
			ent->client->ps.stats[STAT_TEAM] = 0;
		} else if (teams->value == 1) { //Team DM
			ent->client->ps.stats[STAT_GAMEMODE] = CS_GAMEMODES + 2;
			if (ent->count > 0) {
				ent->client->ps.stats[STAT_TEAM] = CS_TEAMNAMES + ent->count - 1;
			} else {
				ent->client->ps.stats[STAT_TEAM] = 0;
			}
		} else if (teams->value == 2) { //Dom
			ent->client->ps.stats[STAT_GAMEMODE] = CS_GAMEMODES + 3;
			if (ent->count > 0) {
				ent->client->ps.stats[STAT_TEAM] = CS_TEAMNAMES + ent->count - 1;
			} else {
				ent->client->ps.stats[STAT_TEAM] = 0;
			}
		} else if (teams->value == 3) { //PvM
			ent->client->ps.stats[STAT_GAMEMODE] = CS_GAMEMODES + 4;
			ent->client->ps.stats[STAT_TEAM] = 0;
		}
	} else {
		ent->client->ps.stats[STAT_GAMEMODE] = 0;
		ent->client->ps.stats[STAT_TEAM] = 0;
	}

	//
	// Identify
	//

	if (((int)ceil(level.time * 10 - 0.5) % 5) == (((ent-g_edicts) - 1) % 5)) {
		AngleVectors(ent->client->v_angle, forward, NULL, NULL);
		VectorCopy(ent->s.origin, offset);
		offset[2] += ent->viewheight - 8;
		VectorMA(offset, 2048, forward, end);
		tr = gi.trace (offset, NULL, NULL, end, ent, MASK_SHOT);

		if ((tr.fraction >= 1.0) || (!tr.ent) || (!tr.ent->client && !(tr.ent->svflags & SVF_MONSTER))) {
			VectorSet(box1, -20, -20, -20);
			VectorSet(box2, 20, 20, 20);
			tr = gi.trace (offset, box1, box2, end, ent, MASK_SHOT);
		}

		if ((tr.fraction < 1.0) && tr.ent && (tr.ent->client || tr.ent->svflags & SVF_MONSTER)) {
			char result[1024];
			char *tmp;
			int from = 0;
// Identifying a player
			if (tr.ent->client) {
				sprintf(result + from, "%s", tr.ent->client->pers.netname);
				from += strlen(tr.ent->client->pers.netname);

				if (ent->client->pers.skill[70] > 1) {
					int i;
					for (i = 0; i < GIEX_NUMCLASSES; i++) {
						if(tr.ent->client->pers.skills.classLevel[i] > 0) {
							classinfo_t *clInfo = getClassInfo(i);
							tmp = getAbrevValue(tr.ent->client->pers.skills.classLevel[i]);
							sprintf(result + from, " %c: %s", clInfo->name[0], tmp);
							from += strlen(tmp) + 4;
						}
					}
				} else if (ent->client->pers.skill[70] > 0) {
					tmp = getAbrevValue(tr.ent->radius_dmg);
					sprintf(result + from, " L: %s", tmp);
					from += strlen(tmp) + 4;
				}
// Identifying a monster
			} else if ((tr.ent->svflags & SVF_MONSTER) && ((tr.ent->radius_dmg) || (ent->client->pers.skill[70] > 1))) {
				sprintf(result + from, "%s", tr.ent->monsterinfo.name);
				from += strlen(tr.ent->monsterinfo.name);

				if (ent->client->pers.skill[70] > 1) {
					tmp = getAbrevValue(tr.ent->monsterinfo.level);
					sprintf(result + from, " L: %s", tmp);
					from += strlen(tmp) + 4;
				}
				if ((ent->client->pers.skill[70] > 0) && (tr.ent->health > 0) && (tr.ent->enemy)) {
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_PARASITE_ATTACK);
					gi.WriteShort (tr.ent - g_edicts);
					gi.WritePosition (tr.ent->s.origin);
					gi.WritePosition (tr.ent->enemy->s.origin);
					gi.unicast(ent, false);
				}
				if ((tr.ent->radius_dmg && tr.ent->monsterinfo.ability)) {
					sprintf(result + from, " ");
					from++;
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_HEALTH) {
						sprintf(result + from, "H");
						from++;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_SPEED) {
						sprintf(result + from, "S");
						from++;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_DAMAGE) {
						sprintf(result + from, "D");
						from++;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_RES_EXPL) {
						sprintf(result + from, "Ex");
						from += 2;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_RES_IMPACT) {
						sprintf(result + from, "Im");
						from += 2;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_RES_ENERGY) {
						sprintf(result + from, "En");
						from += 2;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_RES_FMAGIC) {
						sprintf(result + from, "Fm");
						from += 2;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_RES_BMAGIC) {
						sprintf(result + from, "Bm");
						from += 2;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_RES_LMAGIC) {
						sprintf(result + from, "Lm");
						from += 2;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_REGENAURA) {
						sprintf(result + from, "Ra");
						from += 2;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_SHARDARMOR) {
						sprintf(result + from, "Sa");
						from += 2;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_ARMORPIERCE) {
						sprintf(result + from, "Ap");
						from += 2;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_MANABURN) {
						sprintf(result + from, "Mb");
						from += 2;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_STEALTH) {
						sprintf(result + from, "St");
						from += 2;
					}
					if (tr.ent->monsterinfo.ability & GIEX_MABILITY_SILENCED) {
						sprintf(result + from, "Si");
						from += 2;
					}
					sprintf(result + from, " ");
					from++;
				}
			}
			if ((ent->client->pers.skill[70] > 2) && (tr.ent->health > 0)) {
				tmp = getAbrevValue(tr.ent->health);
				sprintf(result + from, " H: %s", tmp);
				from += strlen(tmp) + 4;
			}
	//		sprintf(result + from, " %.0f %.0f %d ", tr.ent->ideal_yaw, tr.ent->monsterinfo.goal_strollyaw, tr.ent->monsterinfo.lefty);

			if (from > 0) {
				gi.configstring (CS_IDENTS + (ent - g_edicts) - 1, result);
				ent->client->ps.stats[STAT_IDENTIFY] = CS_IDENTS + (ent - g_edicts) - 1;
				ent->client->ps.stats[STAT_CHASE] = 0;
			} else {
				ent->client->ps.stats[STAT_IDENTIFY] = 0;
			}
		} else {
			ent->client->ps.stats[STAT_IDENTIFY] = 0;
		}
	}
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
void G_SetSpectatorStats (edict_t *ent) {
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

	if (cl->chase_target && cl->chase_target->inuse) {
		cl->ps.stats[STAT_CHASE] = CS_IDENTS + (ent - g_edicts) - 1;
		gi.configstring (CS_IDENTS + (ent - g_edicts) - 1, va("%s", cl->chase_target->client->pers.netname));
	} else {
		cl->ps.stats[STAT_CHASE] = 0;
	}
}

void powerup_make_touchable (edict_t *powerup);
void restructurePowerups(edict_t *ent);
void dropPowerup(edict_t *ent, int num) {
	edict_t *powerup;
	skills_t *skills;
	vec3_t forward, right, offset;
	trace_t trace;

	if (ent->client->pers.spectator || ent->client->resp.spectator) {
		return;
	}
	skills = &ent->client->pers.skills;
	if (skills->powerup_type[num] == 0) {
		gi.cprintf(ent, PRINT_HIGH, "No powerup in slot %d\n", num);
		return;
	}

	powerup = G_Spawn();
	powerup->classid = CI_GIEX_POWERUP;
	powerup->classname = "powerup";
	VectorCopy (ent->s.angles, powerup->s.angles);
	powerup->movetype = MOVETYPE_TOSS;
	gi.setmodel (powerup, "models/items/keys/pyramid/tris.md2");
	powerup->think = powerup_make_touchable;
	powerup->nextthink = level.time + 1;
	powerup->solid = SOLID_TRIGGER;
	powerup->radius_dmg = skills->powerup_type[num];
	powerup->dmg_radius = skills->powerup_level[num];

	VectorSet (powerup->mins, -15, -15, -15);
	VectorSet (powerup->maxs, 15, 15, 15);
	powerup->s.effects |= EF_ROTATE;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 0, -16);
	G_ProjectSource (ent->s.origin, offset, forward, right, powerup->s.origin);
	trace = gi.trace (ent->s.origin, powerup->mins, powerup->maxs,
						powerup->s.origin, ent, CONTENTS_SOLID);
	VectorCopy (trace.endpos, powerup->s.origin);

	VectorScale (forward, 100, powerup->velocity);
	powerup->velocity[2] = 300;
	gi.linkentity (powerup);

	skills->powerup_type[num] = 0;
	skills->powerup_level[num] = 0;
	restructurePowerups(ent);
}





void showGiexInventory(edict_t *ent) {
	char	string[1024];
	char	temp[1024];
	int y = 48, i, j, stringlength;
	int start = ent->client->menuchoice[0] - 6;
	int stop = ent->client->menuchoice[0] + 7;

	iteminfo_t *info;

	if (start < 0) {
		stop -= start;
		start = 0;
	}
	if (stop > GIEX_ITEMPERCHAR)
		stop = GIEX_ITEMPERCHAR;

	if (ent->client->pers.spectator || ent->client->resp.spectator) {
		closeGiexMenu(ent);
		return;
	}
	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn inventory ");			// background
	stringlength = strlen(string);
	Com_sprintf (temp, sizeof(temp),
		"xv 52 yv 28 string \"Inventory\" ");
	j = strlen(temp);
	if (stringlength + j < 1024) {
		strcpy (string + stringlength, temp);
		stringlength += j;
	}
	for (i = start; i < stop; i++) {
		if (ent->client->pers.skills.itemId[i] == 0) {
			break;
		}
		info = getItemInfo(ent->client->pers.skills.itemId[i]);
		Com_sprintf (temp, sizeof(temp),
			"xv 52 yv %i %s \"%s\" ", y, (i == ent->client->menuchoice[0]) ? "string" : "string2", info->name);
		j = strlen(temp);
		if (stringlength + j < 1024) {
      	strcpy (string + stringlength, temp);
      	stringlength += j;
		}
		y += 10;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
void showGiexItemMenu(edict_t *ent) {
	char	string[1024];
	char	temp[1024];
	int y = 38, i, j, stringlength;
	int item = ent->client->menuchoice[0];
	int levelreq[GIEX_NUMCLASSES];
	qboolean hasprint = false;
	int classreqCount = 0;
	powerupinfo_t *puinfo;
	iteminfo_t *iteminfo = getItemInfo(ent->client->pers.skills.itemId[ent->client->menuchoice[0]]);


	if (ent->client->pers.spectator || ent->client->resp.spectator) {
		closeGiexMenu(ent);
		return;
	}
 // send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn inventory "
		"xv 52 yv 28 string2 \"%s\" ", iteminfo->name);
	stringlength = strlen(string);
	for (i = 0; i < GIEX_NUMCLASSES; i++) {
		levelreq[i] = getItemLevelReq(ent, ent->client->menuchoice[0], i);
		if (levelreq[i] > 0) {
			classreqCount++;
		}
	}
	for (i = 0; i < GIEX_NUMCLASSES; i++) {
		if (levelreq[i] > 0) {
			classinfo_t *cinfo = getClassInfo(i);
			if (!hasprint) {
				Com_sprintf (temp, sizeof(temp),
					"xv 52 yv %i string2 \" Requires%s:\" ", y, (classreqCount > 1) ? " one of" : "");
				j = strlen(temp);
				if (stringlength + j < 1024) {
					strcpy (string + stringlength, temp);
					stringlength += j;
				}
				y += 10;
				hasprint = true;
			}
			Com_sprintf (temp, sizeof(temp),
				"xv 52 yv %i string2 \" %s level %d\" ", y, cinfo->name, levelreq[i]);
			j = strlen(temp);
			if (stringlength + j < 1024) {
				strcpy (string + stringlength, temp);
				stringlength += j;
			}
			y += 10;
		}
	}
	for (i = 0; i < GIEX_PUPERITEM; i++) {
		if (ent->client->pers.skills.item_putype[item][i] == 0) {
			break;
		}
		puinfo = getPowerupInfo(ent->client->pers.skills.item_putype[item][i]);
		Com_sprintf (temp, sizeof(temp),
			"xv 52 yv %i string2 \"   %s %d/%d\" "
			"xv 52 yv %i string2 \"    Exp left %d\" ", y, puinfo->name, ent->client->pers.skills.item_pucurlvl[item][i], ent->client->pers.skills.item_pumaxlvl[item][i], y + 10, ent->client->pers.item_puexpRemain[item][i]);
		j = strlen(temp);
		if (stringlength + j < 1024) {
      	strcpy (string + stringlength, temp);
      	stringlength += j;
		}
		y += 20;
	}
	y += 10;
	Com_sprintf (temp, sizeof(temp),
		"xv 52 yv %i %s \"Equip\" "
		"xv 52 yv %i %s \"Drop\" "
		"xv 52 yv %i %s \"Info\" ", y, (ent->client->menuchoice[1] == 0) ? "string" : "string2", y+10, (ent->client->menuchoice[1] == 1) ? "string" : "string2", y+20, (ent->client->menuchoice[1] == 2) ? "string" : "string2");
	j = strlen(temp);
	if (stringlength + j < 1024) {
		strcpy (string + stringlength, temp);
		stringlength += j;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
void showGiexPowerups(edict_t *ent) {
	char	string[1024];
	char	temp[1024];
	int y = 48, i, j, stringlength;
	int start = ent->client->menuchoice[0] - 5;
	int stop = ent->client->menuchoice[0] + 6;

	powerupinfo_t *info;

	if (start < 0) {
		stop -= start;
		start = 0;
	}
	if (stop > GIEX_PUCARRYPERCHAR)
		stop = GIEX_PUCARRYPERCHAR;

	if (ent->client->pers.spectator || ent->client->resp.spectator) {
		closeGiexMenu(ent);
		return;
	}
	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn inventory "
		"xv 52 yv 28 string2 \"Powerups:\" "
	);			// background
	stringlength = strlen(string);
	for (i = start; i < stop; i++) {
		if (ent->client->pers.skills.powerup_type[i] == 0) {
			break;
		}
		info = getPowerupInfo(ent->client->pers.skills.powerup_type[i]);
		//gi.dprintf("%d\n", ent->client->pers.skills.powerup_type[i]);
		Com_sprintf (temp, sizeof(temp),
			"xv 52 yv %i %s \"Level %d %s\" ", y, (i == ent->client->menuchoice[0]) ? "string" : "string2", ent->client->pers.skills.powerup_level[i], info->name);
		j = strlen(temp);
		if (stringlength + j < 1024) {
      	strcpy (string + stringlength, temp);
      	stringlength += j;
		}
		y += 10;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
void showGiexPowerupMenu(edict_t *ent) {
	char	string[1024];
	char	temp[1024];
	int y = 58, i, j, c, stringlength;
	powerupinfo_t *puinfo = getPowerupInfo(ent->client->pers.skills.powerup_type[ent->client->menuchoice[0]]);
	int start = ent->client->menuchoice[1] - 5;
	int stop = ent->client->menuchoice[1] + 6;
	if (start < 0) {
		stop -= start;
		start = 0;
	}
	if (stop > GIEX_ITEMPERCHAR)
		stop = GIEX_ITEMPERCHAR;

	if (ent->client->pers.spectator || ent->client->resp.spectator) {
		closeGiexMenu(ent);
		return;
	}
	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn inventory "
		"xv 52 yv 28 string2 \"Level %d %s\" "
		"xv 52 yv 38 %s \"Drop\" ", ent->client->pers.skills.powerup_level[ent->client->menuchoice[0]], puinfo->name, (ent->client->menuchoice[1] == 0) ? "string" : "string2");
	stringlength = strlen(string);
	c = 0;
	if (puinfo->isspell) {
		int left = GIEX_PUPERCHAR - getPluginCount(ent) + 1;
		Com_sprintf (temp, sizeof(temp),
			"xv 52 yv 58 %s \"Memory (%d)\" ", (ent->client->menuchoice[1] == 1) ? "string" : "string2", left);
		j = strlen(temp);
		if (stringlength + j < 1024) {
      	strcpy (string + stringlength, temp);
      	stringlength += j;
		}
		y += 10;
		c++;
	}
	for (i = 0; i < stop; i++) {
		iteminfo_t *iteminfo = getItemInfo(ent->client->pers.skills.itemId[i]);
		int socketed;
		if (iteminfo->name[0] == '\0') {
			break;
		}
		//gi.dprintf("%s, %d\n", iteminfo->name, ent->client->pers.skills.powerup_type[ent->client->menuchoice[0]]);
		if (getItemPowerupInfoSlot(iteminfo, ent->client->pers.skills.powerup_type[ent->client->menuchoice[0]]) == -1) {
			stop++;
			continue;
		}
		c++;
		if (i < start) {
			continue;
		}
		socketed = getItemSocketedCount(ent, i);
		Com_sprintf (temp, sizeof(temp),
			"xv 52 yv %i %s \"%s (%d)\" ", y, (ent->client->menuchoice[1] == c) ? "string" : "string2", iteminfo->name, (iteminfo->maxpowerups - socketed + 1));
		j = strlen(temp);
		if (stringlength + j < 1024) {
      	strcpy (string + stringlength, temp);
      	stringlength += j;
		}
		y += 10;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
void showGiexMainMenu(edict_t *ent) {
	char	string[1024];

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn inventory "
		"xv 92 yv 28 string \"Welcome to Giex!\" "
		//"xv 72 yv 38 string \"----------------\" "
		"xv 62 yv 48 string2 \"Visit our homepage at\" "
		"xv 62 yv 58 string \"www.planetquake.com/giex\" "
		"xv 52 yv 78 %s \"Login\" "
		"xv 52 yv 88 %s \"Create character\" "
		"xv 52 yv 98 %s \"Help\" "
		"xv 52 yv 108 %s \"Chasecam\" ",
		(ent->client->menuchoice[0] == 0) ? "string" : "string2",
		(ent->client->menuchoice[0] == 1) ? "string" : "string2",
		(ent->client->menuchoice[0] == 2) ? "string" : "string2",
		(ent->client->menuchoice[0] == 3) ? "string" : "string2"
	);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
void showGiexHelpMenu(edict_t *ent) {
	char	string[1024];

	// send the layout
	if (ent->client->menuchoice[1] == 0) {
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 72 yv  28 string \"Welcome to Giex!\" "
//			"xv 72 yv  38 string \"---------------------------\" "
			"xv 52 yv  48 string \"Giex is a mod for Quake2\" "
			"xv 52 yv  58 string \"that blends elements of\" "
			"xv 52 yv  68 string \"MMORPGS with the classic\" "
			"xv 52 yv  78 string \"action-packed Q2.\" "
			"xv 52 yv  88 string \"The first thing you will\" "
			"xv 52 yv  98 string \"need to do, is create a\" "
			"xv 52 yv 108 string \"character, which can be\" "
			"xv 52 yv 118 string \"done from the main menu.\" "
		);
	} else if (ent->client->menuchoice[1] == 1) {
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv  28 string \"From the main menu,\" "
			"xv 52 yv  38 string \"choose 'Create character'\" "
			"xv 52 yv  48 string \"and then select the class\" "
			"xv 52 yv  58 string \"you want to start out as.\" "
			"xv 52 yv  68 string \"(note that you later on can\" "
			"xv 52 yv  78 string \"choose to level up any of\" "
			"xv 52 yv  88 string \"the other classes.)\" "
//			"xv 52 yv  98 string \"\" "
			"xv 52 yv 108 string \"After selecting a class,\" "
			"xv 52 yv 118 string \"type in the password at\" "
			"xv 52 yv 128 string \"the prompt, as requested.\" "
			"xv 52 yv 138 string \"You are now in the game\" "
			"xv 52 yv 148 string \"and can start gaining exp\" "
			"xv 52 yv 158 string \"to grow stronger!\" "
		);
	} else if (ent->client->menuchoice[1] == 2) {
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv  28 string \"When fighting, you will\" "
			"xv 52 yv  38 string \"gain experience (exp),\" "
			"xv 52 yv  48 string \"monsters and other players\" "
			"xv 52 yv  58 string \"that you manage to kill may\" "
			"xv 52 yv  68 string \"drop powerups and items for\" "
			"xv 52 yv  78 string \"you to collect.\" "
			"xv 52 yv  88 string \"The key to a powerful char\" "
			"xv 52 yv  98 string \"lies not only in gaining\" "
			"xv 52 yv 108 string \"levels, but also finding\" "
			"xv 52 yv 118 string \"unique combinations of\" "
			"xv 52 yv 128 string \"classes, items and powerups\" "
		);
	} else if (ent->client->menuchoice[1] == 3) {
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv  28 string \"There are four important\" "
			"xv 52 yv  38 string \"commands that you need to\" "
			"xv 52 yv  48 string \"bind:\" "
			"xv 52 yv  58 string \"pickup - You pick up any\" "
			"xv 52 yv  68 string \"powerups or items you are\" "
			"xv 52 yv  78 string \"touching.\" "
			"xv 52 yv  88 string \"inven - Shows item menu\" "
			"xv 52 yv  98 string \"and allows you to equip\" "
			"xv 52 yv 108 string \"your items.\" "
			"xv 52 yv 118 string \"powerups - Powerup menu,\" "
			"xv 52 yv 128 string \"allows you to plug in\" "
			"xv 52 yv 138 string \"powerups into your body\" "
			"xv 52 yv 148 string \"or your items.\" "
			"xv 52 yv 158 string \"plugins - Memory menu,\" "
			"xv 52 yv 168 string \"shows your spell memory\" "
		);
	} else if (ent->client->menuchoice[1] == 4) {
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv  28 string \"Also note these commands:\" "
			"xv 52 yv  38 string \"identify - Shows info on\" "
			"xv 52 yv  48 string \"any powerup or item you\" "
			"xv 52 yv  58 string \"stand on.\" "
			"xv 52 yv  68 string \"wear <item name> - For\" "
			"xv 52 yv  78 string \"changing your equipment\" "
			"xv 52 yv  88 string \"without using the menu.\" "
			"xv 52 yv  98 string \"worn - Will show your\" "
			"xv 52 yv 108 string \"currently worn items.\" "
			"xv 52 yv 118 string \"autopickup - Will pick\" "
			"xv 52 yv 128 string \"up suitable powerups as\" "
			"xv 52 yv 138 string \"soon as you touch them.\" "
			"xv 52 yv 148 string \"autoplugin - Will plug\" "
			"xv 52 yv 158 string \"in any suitable powerup\" "
			"xv 52 yv 168 string \"that you pick up.\" "
		);
	} else if (ent->client->menuchoice[1] == 5) {
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv  28 string \"level - Lists your\" "
			"xv 52 yv  38 string \"current class levels.\" "
			"xv 52 yv  68 string \"When playing, monsters\" "
			"xv 52 yv  78 string \"will spawn in. These\" "
			"xv 52 yv  88 string \"are of different\" "
			"xv 52 yv  98 string \"skill levels depending\" "
			"xv 52 yv 108 string \"on what level players\" "
			"xv 52 yv 118 string \"are currently on the\" "
			"xv 52 yv 128 string \"server. Each tier of\" "
			"xv 52 yv 138 string \"monsters are denoted\" "
			"xv 52 yv 148 string \"with a colored shell.\" "
			"xv 52 yv 158 string \"A list of each color\" "
			"xv 52 yv 168 string \"follows.\" "
		);
	} else if (ent->client->menuchoice[1] == 6) {
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv  28 string \"Normal monsters\" "
			"xv 52 yv  38 string \"Level     Color\" "
			"xv 52 yv  48 string \"0 - 7     None\" "
			"xv 52 yv  58 string \"8 - 15    Green\" "
			"xv 52 yv  68 string \"16 - 23   Red\" "
			"xv 52 yv  78 string \"24 - 30   Yellow\" "
			"xv 52 yv  98 string \"'Boss' monsters\" "
			"xv 52 yv 108 string \"Level     Color\" "
			"xv 52 yv 118 string \"0 - 7     Blue\" "
			"xv 52 yv 128 string \"8 - 15    Cyan\" "
			"xv 52 yv 138 string \"16 - 23   Purple\" "
			"xv 52 yv 148 string \"24 - 30   White\" "
		);
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
void showGiexCommandsMenu(edict_t *ent) {
	char	string[1024];

	// send the layout
	if (ent->client->menuchoice[1] == 0) {
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 72 yv  28 string \"Commands\" "
//			"xv 72 yv  38 string \"---------------------------\" "
			"xv 52 yv  48 string2 \"giexhelp - Describes basics\" "
			"xv 52 yv  58 string2 \"of Giex gameplay.\" "
			"xv 52 yv  68 string2 \"giexcmds - This helpfile.\" "
			"xv 52 yv  78 string2 \"inven - Allows you to\" "
			"xv 52 yv  88 string2 \"manipulate your inventory\" "
			"xv 52 yv  98 string2 \"of items.\" "
			"xv 52 yv 108 string2 \"powerups - Menu to plugin,\" "
			"xv 52 yv 118 string2 \"socket or drop your\" "
			"xv 52 yv 128 string2 \"powerups.\" "
			"xv 52 yv 138 string2 \"memory - To see and remove\" "
			"xv 52 yv 148 string2 \"your currently plugged in\" "
			"xv 52 yv 158 string2 \"spell powerups.\" "
 		);
	} else if (ent->client->menuchoice[1] == 1) {
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv  28 string2 \"vote - Access the voting\" "
			"xv 52 yv  38 string2 \"system.\" "
			"xv 52 yv  48 string2 \"class - choose your class.\" "
			"xv 52 yv  58 string2 \"level - display your levels.\" "
			"xv 52 yv  68 string2 \"pickup - Will pick up any\" "
			"xv 52 yv  78 string2 \"powerup or item you're\" "
			"xv 52 yv  88 string2 \"touching, as long as you.\" "
			"xv 52 yv  98 string2 \"have space in your\" "
			"xv 52 yv 108 string2 \"inventory.\" "
			"xv 52 yv 118 string2 \"identify - Displays some\" "
			"xv 52 yv 128 string2 \"basic information about\" "
			"xv 52 yv 138 string2 \"any powerup or item that\" "
			"xv 52 yv 148 string2 \"you touch.\" "
//			"xv 52 yv 158 string \"\" "
		);
	} else if (ent->client->menuchoice[1] == 2) {
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv  28 string2 \"nextspell - Choose the\" "
			"xv 52 yv  38 string2 \"next available spell as\" "
			"xv 52 yv  48 string2 \"your active spell.\" "
			"xv 52 yv  58 string2 \"prevspell - Like nextspell.\" "
			"xv 52 yv  68 string2 \"+castspell - Cast your\" "
			"xv 52 yv  78 string2 \"currently active spell.\" "
			"xv 52 yv  88 string2 \"(Note: Auras are always\" "
			"xv 52 yv  98 string2 \"on when selected as\" "
			"xv 52 yv 108 string2 \"active spell.)\" "
			"xv 52 yv 118 string2 \"\" "
			"xv 52 yv 128 string2 \"autopickup & autoplugin\" "
			"xv 52 yv 138 string2 \"These are convenience\" "
			"xv 52 yv 148 string2 \"commands that can be used\" "
			"xv 52 yv 158 string2 \"to ease powerup management.\" "
		);
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
void showGiexClassesMenu(edict_t *ent) {
	char	string[1024];
	char	temp[1024];
	int y = 48, i, j, stringlength;

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn inventory "
		"xv 52 yv 28 string2 \"Choose your class\" "
	);
	stringlength = strlen(string);
	for (i = 0; i < GIEX_NUMCLASSES; i++) {
		classinfo_t *info = getClassInfo(i);
		if (info->name[0] == '\0') {
			break;
		}
		Com_sprintf (temp, sizeof(temp),
			"xv 52 yv %i %s \"%s\" ", y, (i == ent->client->menuchoice[1]) ? "string" : "string2", info->name);
		j = strlen(temp);
		if (stringlength + j < 1024) {
      	strcpy (string + stringlength, temp);
      	stringlength += j;
		}
		y += 10;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
void showEnterNewPwdMenu(edict_t *ent) {
	char	string[1024];

	// send the layout
	if (ent->client->menuchoice[0] == 0) { //Load char
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv 28 string2 \"Enter character password\" "
			"xv 52 yv 38 string2 \"in prompt above\" "
		);
	} else { // Create char
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv 28 string2 \"Enter desired password\" "
			"xv 52 yv 38 string2 \"in prompt above\" "
		);
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
void showGiexPlugins(edict_t *ent) {
	char	string[1024];
	char	temp[1024];
	int y = 48, i, j, stringlength;
	int start = ent->client->menuchoice[0] - 7;
	int stop = ent->client->menuchoice[0] + 8;

	powerupinfo_t *info;

	if (start < 0) {
		stop -= start;
		start = 0;
	}
	if (stop > GIEX_PUPERCHAR)
		stop = GIEX_PUPERCHAR;

	if (ent->client->pers.spectator || ent->client->resp.spectator) {
		closeGiexMenu(ent);
		return;
	}
 // send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn inventory ");			// background
	stringlength = strlen(string);
	Com_sprintf (temp, sizeof(temp),
		"xv 52 yv 28 string \"Spells in memory\" ");
	j = strlen(temp);
	if (stringlength + j < 1024) {
		strcpy (string + stringlength, temp);
		stringlength += j;
	}
	for (i = start; i < stop; i++) {
		if (ent->client->pers.skills.putype[i] == 0) {
			break;
		}
		info = getPowerupInfo(ent->client->pers.skills.putype[i]);
		Com_sprintf (temp, sizeof(temp),
			"xv 52 yv %i %s \"Level %d/%d %s\" ", y, (i == ent->client->menuchoice[0]) ? "string" : "string2", ent->client->pers.skills.pucurlvl[i], ent->client->pers.skills.pumaxlvl[i], info->name);
		j = strlen(temp);
		if (stringlength + j < 1024) {
      	strcpy (string + stringlength, temp);
      	stringlength += j;
		}
		y += 10;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
void showGiexUnplug(edict_t *ent) {
	char	string[1024];
	skills_t *skills;
	int pu = ent->client->menuchoice[0];

	powerupinfo_t *info = getPowerupInfo(ent->client->pers.skills.putype[pu]);
	skills = &ent->client->pers.skills;

 // send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn inventory "
		"xv 52 yv 28 string2 \"%s powerup\" "
		"xv 52 yv 38 string2 \"Level    %d/%d\" "
		"xv 52 yv 48 string2 \"Exp left %d\" "
		"xv 52 yv 58 %s \"Unplug\" "
		"xv 52 yv 68 %s \"Close\" ", info->name, skills->pucurlvl[pu], skills->pumaxlvl[pu], ent->client->pers.puexpRemain[pu], (ent->client->menuchoice[1] == 0) ? "string" : "string2", (ent->client->menuchoice[1] == 1) ? "string" : "string2");

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
void showGiexWornMenu(edict_t *ent) {
	char	string[1024];
	char	temp[1024];
	int y = 48, i, j, stringlength;
	iteminfo_t *info;

	if (ent->client->pers.spectator || ent->client->resp.spectator) {
		closeGiexMenu(ent);
		return;
	}
	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn inventory ");			// background
	stringlength = strlen(string);
	Com_sprintf (temp, sizeof(temp),
		"xv 52 yv 28 string \"Worn items\" ");
	j = strlen(temp);
	if (stringlength + j < 1024) {
		strcpy (string + stringlength, temp);
		stringlength += j;
	}
	for (i = 0; i < GIEX_BASEITEMS; i++) {
		info = getWornItemInfo(ent, i);
		if (info->name[0] != '\0') {
			//gi.cprintf(ent, PRINT_HIGH, "%s: %s\n", getBaseTypeName(i), info->name[0] != '\0' ? info->name : "Empty");
			Com_sprintf (temp, sizeof(temp),
				"xv 52 yv %i string2 \"%s\" xv 117 string2 \"%s\" ", y, getBaseTypeName(i), info->name[0] != '\0' ? info->name : "Empty");
			j = strlen(temp);
			if (stringlength + j < 1024) {
				strcpy (string + stringlength, temp);
				stringlength += j;
			}
			y += 10;
		}
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}
void showGiexStats(edict_t *ent) {
	skills_t *skills;
	char	string[1024];
	char	temp[1024];
	int j, stringlength;
	int i, pu;

	if (ent->client->pers.spectator || ent->client->resp.spectator) {
		closeGiexMenu(ent);
		return;
	}

	skills = &ent->client->pers.skills;
	// send the layout
	if (ent->client->menuchoice[0] == 0) {
		int sumexp = 0;
		int sumkills = 0;
		int sumdeaths = 0;
		for (i = 0; i < GIEX_NUMCLASSES; i++) {
			sumexp += skills->classExp[i];
		}
		for (i = 0; i < GIEX_PUPERCHAR; i++) {
			if (skills->putype[i] == 0) {
				break;
			}
			sumexp += skills->puexp[i];
		}
		for (i = 0; i < GIEX_ITEMPERCHAR; i++) {
			if (skills->itemId[i] == 0) {
				break;
			}
			for (pu = 0; pu < GIEX_PUPERITEM; pu++) {
				if (skills->item_putype[i][pu] == 0) {
					break;
				}
				sumexp += skills->item_puexp[i][pu];
			}
		}
		sumkills += skills->stats[GIEX_STAT_LOWER_PLAYER_KILLS];
		sumkills += skills->stats[GIEX_STAT_HIGHER_PLAYER_KILLS];
		sumdeaths += skills->stats[GIEX_STAT_LOWER_PLAYER_DEATHS];
		sumdeaths += skills->stats[GIEX_STAT_HIGHER_PLAYER_DEATHS];

		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv  28 string \"General stats\" "
//			"xv 72 yv  38 string \"---------------------------\" "
			"xv 52 yv  48 string2 \"Level\" xv 117 string2 \"%d\" "
			"xv 52 yv  58 string2 \"Exp\" xv 117 string2 \"%d\" "
			"xv 52 yv  68 string2 \"Kills\" xv 117 string2 \"%d\" "
			"xv 52 yv  78 string2 \"Deaths\" xv 117 string2 \"%d\" ", ent->radius_dmg, sumexp, sumkills, sumdeaths
 		);
	} else if (ent->client->menuchoice[0] == 1) {
		int y = 48;
		int classexp = 0;
		int itemexp = 0;
		int spellexp = 0;
		classinfo_t *info;
		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv  28 string \"Character stats\" "
		);
		stringlength = strlen(string);
		for (i = 0; i < GIEX_NUMCLASSES; i++) {
			classexp += skills->classExp[i];
			if (skills->classLevel[i] > 0) {
				info = getClassInfo(i);
				Com_sprintf (temp, sizeof(temp),
					"xv 52 yv %i string2 \"%s\" xv 167 string2 \"%d\" ", y, info->name, skills->classLevel[i]
				);
				j = strlen(temp);
				if (stringlength + j < 1024) {
					strcpy (string + stringlength, temp);
					stringlength += j;
				}
				y += 10;
			}
		}
		y += 10;
		for (i = 0; i < GIEX_ITEMPERCHAR; i++) {
			if (skills->itemId[i] == 0) {
				break;
			}
			for (pu = 0; pu < GIEX_PUPERITEM; pu++) {
				if (skills->item_putype[i][pu] == 0) {
					break;
				}
				itemexp += skills->item_puexp[i][pu];
			}
		}
		for (i = 0; i < GIEX_PUPERCHAR; i++) {
			if (skills->putype[i] == 0) {
				break;
			}
			spellexp += skills->puexp[i];
		}
		Com_sprintf (temp, sizeof(temp),
			"xv 52 yv %i string2 \"Class exp\" xv 167 string2 \"%d\" "
			"xv 52 yv %i string2 \"Item exp\" xv 167 string2 \"%d\" "
			"xv 52 yv %i string2 \"Spell exp\" xv 167 string2 \"%d\" ",
			y, classexp, y + 10, itemexp, y + 20, spellexp
		);
		j = strlen(temp);
		if (stringlength + j < 1024) {
			strcpy (string + stringlength, temp);
			stringlength += j;
		}
	} else if (ent->client->menuchoice[0] == 2) {
		int sumkills = 0;
		int sumdeaths = 0;
		sumkills += skills->stats[GIEX_STAT_LOWER_PLAYER_KILLS];
		sumkills += skills->stats[GIEX_STAT_HIGHER_PLAYER_KILLS];
		sumkills += skills->stats[GIEX_STAT_LOW_MONSTER_KILLS];
		sumkills += skills->stats[GIEX_STAT_MED_MONSTER_KILLS];
		sumkills += skills->stats[GIEX_STAT_HI_MONSTER_KILLS];
		sumkills += skills->stats[GIEX_STAT_VHI_MONSTER_KILLS];
		sumdeaths += skills->stats[GIEX_STAT_LOWER_PLAYER_DEATHS];
		sumdeaths += skills->stats[GIEX_STAT_HIGHER_PLAYER_DEATHS];
		sumdeaths += skills->stats[GIEX_STAT_MONSTER_DEATHS];
		sumdeaths += skills->stats[GIEX_STAT_OTHER_DEATHS];

		Com_sprintf (string, sizeof(string),
			"xv 32 yv 8 picn inventory "
			"xv 52 yv  28 string \"Kill stats\" "
			"xv 52 yv 48 string \"Kills\" xv 227 string \"%d\" "
			"xv 52 yv 58 string2 \"Lower lvl\" xv 227 string2 \"%d\" "
			"xv 52 yv 68 string2 \"Higher lvl\" xv 227 string2 \"%d\" "
			"xv 52 yv 78 string2 \"Newbie monsters\" xv 227 string2 \"%d\" "
			"xv 52 yv 88 string2 \"Low lvl monsters\" xv 227 string2 \"%d\" "
			"xv 52 yv 98 string2 \"Mid lvl monsters\" xv 227 string2 \"%d\" "
			"xv 52 yv 108 string2 \"High lvl monsters\" xv 227 string2 \"%d\" "
			"xv 52 yv 128 string \"Deaths\" xv 227 string \"%d\" "
			"xv 52 yv 138 string2 \"Lower lvl\" xv 227 string2 \"%d\" "
			"xv 52 yv 148 string2 \"Higher lvl\" xv 227 string2 \"%d\" "
			"xv 52 yv 158 string2 \"Monsters\" xv 227 string2 \"%d\" "
			"xv 52 yv 168 string2 \"Other\" xv 227 string2 \"%d\" ",
			sumkills, skills->stats[GIEX_STAT_LOWER_PLAYER_KILLS], skills->stats[GIEX_STAT_HIGHER_PLAYER_KILLS],
			skills->stats[GIEX_STAT_LOW_MONSTER_KILLS], skills->stats[GIEX_STAT_MED_MONSTER_KILLS],
			skills->stats[GIEX_STAT_HI_MONSTER_KILLS], skills->stats[GIEX_STAT_VHI_MONSTER_KILLS],
			sumdeaths, skills->stats[GIEX_STAT_LOWER_PLAYER_DEATHS], skills->stats[GIEX_STAT_HIGHER_PLAYER_DEATHS],
			skills->stats[GIEX_STAT_MONSTER_DEATHS], skills->stats[GIEX_STAT_OTHER_DEATHS]
		);
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}

void showGiexMenu(edict_t *ent) {
	switch(ent->client->showmenu) {
		case 1:
			showGiexInventory(ent);
			break;
		case 2:
			showGiexItemMenu(ent);
			break;
		case 3:
			showGiexPowerups(ent);
			break;
		case 4:
			showGiexPowerupMenu(ent);
			break;
		case 5:
			showGiexMainMenu(ent);
			break;
		case 6:
			showGiexHelpMenu(ent);
			break;
		case 7:
			showGiexClassesMenu(ent);
			break;
		case 8:
			showEnterNewPwdMenu(ent);
			break;
		case 9:
			showGiexPlugins(ent);
			break;
		case 10:
			showGiexUnplug(ent);
			break;
		case 11:
			showGiexCommandsMenu(ent);
			break;
		case 12:
			showGiexWornMenu(ent);
			break;
		case 13:
			showGiexStats(ent);
			break;
		default:
			break;
	}
}
void wearItem(edict_t *ent, int item);
void giexitem_make_touchable (edict_t *item);
qboolean getPowerupLevelable(edict_t *ent, int putype, int lvl);
void selectMenuChoice(edict_t *ent) {
/*	if (level.time < ent->pain_debounce_time) {
		return;
	}
	ent->pain_debounce_time = level.time + 0.2;*/

	if (ent->client->showmenu == 1) { //Inventory
		ent->client->showmenu = 2;
		showGiexItemMenu(ent);
	} else if (ent->client->showmenu == 2) { // Specific item info
		if (ent->client->menuchoice[1] == 1) { // Drop
			int pu;
			edict_t *item;
			int i = ent->client->menuchoice[0];
			int x;
			vec3_t forward, right, offset;
			trace_t trace;
			iteminfo_t *info = getItemInfo(ent->client->pers.skills.itemId[i]);

			if (ent->client->pers.spectator || ent->client->resp.spectator) {
				closeGiexMenu(ent);
				return;
			}
			if (ent->client->menuchoice[0] == 0) {
				gi.cprintf(ent, PRINT_HIGH, "Can't drop the %s\n", info->name);
				closeGiexMenu(ent);
				return;
			}
			for (x = 0; x < GIEX_BASEITEMS; x++) {
				if (i == ent->client->pers.skills.wornItem[x]) {
					gi.cprintf(ent, PRINT_HIGH, "Cannot drop a worn item\n");
					closeGiexMenu(ent);
					return;
				}
			}
			item = G_Spawn();
			item->classid = CI_GIEX_ITEM;
			item->classname = "giexitem";
			VectorCopy (ent->s.origin, item->s.origin);
			VectorCopy (ent->s.angles, item->s.angles);
			VectorSet (item->mins, -15, -15, -15);
			VectorSet (item->maxs, 15, 15, 15);
			item->s.effects |= EF_ROTATE;
			AngleVectors (ent->client->v_angle, forward, right, NULL);
			VectorSet(offset, 24, 0, -16);
			G_ProjectSource (ent->s.origin, offset, forward, right, item->s.origin);
			trace = gi.trace (ent->s.origin, item->mins, item->maxs,
								item->s.origin, ent, CONTENTS_SOLID);
			VectorCopy (trace.endpos, item->s.origin);

			VectorScale (forward, 100, item->velocity);
			item->velocity[2] = 275.0 + 175.0 * random();
			item->movetype = MOVETYPE_TOSS;
			gi.setmodel (item, info->modelname);
			item->think = giexitem_make_touchable;
			item->nextthink = level.time + 1;
			item->solid = SOLID_TRIGGER;
			item->radius_dmg = ent->client->pers.skills.itemId[i];
			item->owner = ent;
			gi.linkentity(item);

			ent->client->pers.skills.itemId[i] = 0;
			for (pu = 0; pu < GIEX_PUPERITEM; pu++) {
				item->itempuinfo.putype[pu] = ent->client->pers.skills.item_putype[i][pu];
				item->itempuinfo.pumaxlvl[pu] = ent->client->pers.skills.item_pumaxlvl[i][pu];
				item->itempuinfo.pucurlvl[pu] = ent->client->pers.skills.item_pucurlvl[i][pu];
				item->itempuinfo.puexp[pu] = ent->client->pers.skills.item_puexp[i][pu];
				ent->client->pers.skills.item_putype[i][pu] = 0;
				ent->client->pers.skills.item_pumaxlvl[i][pu] = 0;
				ent->client->pers.skills.item_pucurlvl[i][pu] = 0;
				ent->client->pers.skills.item_puexp[i][pu] = 0;
			}
			closeGiexMenu(ent);
			updateItemLevels(ent);
			restructureItems(ent);
		} else if (ent->client->menuchoice[1] == 0) { //Equip
			if (ent->client->pers.spectator || ent->client->resp.spectator) {
				closeGiexMenu(ent);
				return;
			}
			wearItem(ent, ent->client->menuchoice[0]);
			closeGiexMenu(ent);
		} else if (ent->client->menuchoice[1] == 2) { //Info
			showItemInfo(ent, ent->client->menuchoice[0]);
			closeGiexMenu(ent);
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("toggleconsole\n");
			gi.unicast(ent, true);
		} else {
			gi.cprintf(ent, PRINT_HIGH, "Invalid option\n");
		}
	} else if (ent->client->showmenu == 3) { // Powerup list menu
		if (ent->client->pers.spectator || ent->client->resp.spectator) {
			closeGiexMenu(ent);
			return;
		}
		ent->client->showmenu = 4;
		showGiexPowerupMenu(ent);
	} else if (ent->client->showmenu == 4) { //Powerup submenu
		int i, c = 1;
		powerupinfo_t *puinfo = getPowerupInfo(ent->client->pers.skills.powerup_type[ent->client->menuchoice[0]]);
		int start = ent->client->menuchoice[1] - 7;
		if (start < 0) {
			start = 0;
		}

		if (ent->client->pers.spectator || ent->client->resp.spectator) {
			closeGiexMenu(ent);
			return;
		}
		if (ent->client->menuchoice[1] == 0) {
//			gi.dprintf("Drop %s\n", puinfo->name);
			dropPowerup(ent, ent->client->menuchoice[0]);
			closeGiexMenu(ent);
			return;
		}
		if (puinfo->isspell) {
			c++;
			if (ent->client->menuchoice[1] == 1) {
				pluginPowerup(ent, ent->client->menuchoice[0], -1);
				closeGiexMenu(ent);
				return;
			}
		}
		for (i = 0; i < GIEX_ITEMPERCHAR; i++) {
			iteminfo_t *iteminfo = getItemInfo(ent->client->pers.skills.itemId[i]);
			if (iteminfo->name[0] == '\0') {
				break;
			}
			//gi.dprintf("%s, %d\n", iteminfo->name, ent->client->pers.skills.powerup_type[ent->client->menuchoice[0]]);
			//gi.dprintf("%d, %d, %s\n", c, ent->client->menuchoice[1], iteminfo->name);
			if (getItemPowerupInfoSlot(iteminfo, ent->client->pers.skills.powerup_type[ent->client->menuchoice[0]]) == -1) {
				continue;
			}
			if (ent->client->menuchoice[1] == c) {
//				gi.dprintf("%s (%d) chosen.. %d %d %d\n", iteminfo->name, i, ent->client->menuchoice[1], start, c);
				pluginPowerup(ent, ent->client->menuchoice[0], i);
				closeGiexMenu(ent);
			}
			c++;
		}
	} else if (ent->client->showmenu == 5) { // Main menu
		if (ent->client->menuchoice[0] == 0) { // Load char
			ent->client->showmenu = 8;
			showEnterNewPwdMenu(ent);

			ent->client->pers.entering_pwd = true;
			ent->client->pers.entered_pwd[0] = '\0';
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("messagemode\n");
			gi.unicast(ent, true);
		} else if (ent->client->menuchoice[0] == 1) { // Create char
			ent->client->showmenu = 7;
			showGiexClassesMenu(ent);
		} else if (ent->client->menuchoice[0] == 2) { // Help
			ent->client->showmenu = 6;
			showGiexHelpMenu(ent);
		} else if (ent->client->menuchoice[0] == 3) { // Chasecam
			if (ent->client->chase_target)
				ChaseNext(ent);
			else
				GetChaseTarget(ent);
			closeGiexMenu(ent);
		}
	} else if (ent->client->showmenu == 6) { // Help menu
		if (ent->client->pers.loggedin) {
			closeGiexMenu(ent);
			return;
		}
		ent->client->menuchoice[1] = 0;
		ent->client->showmenu = 5;
		showGiexMainMenu(ent);
	} else if (ent->client->showmenu == 7) { // Class choice menu
		ent->client->showmenu = 8;
		showEnterNewPwdMenu(ent);

		ent->client->pers.entering_pwd = true;
		ent->client->pers.entered_pwd[0] = '\0';
		gi.WriteByte (svc_stufftext);
		gi.WriteString ("messagemode\n");
		gi.unicast(ent, true);
	} else if (ent->client->showmenu == 9) { // Plugin list menu
		if (ent->client->pers.spectator || ent->client->resp.spectator) {
			closeGiexMenu(ent);
			return;
		}
		ent->client->menuchoice[1] = 1;
		ent->client->showmenu = 10;
		showGiexUnplug(ent);
	} else if (ent->client->showmenu == 10) { // Unplug menu
		if (ent->client->menuchoice[1] == 0) {
			ent->client->pers.skills.putype[ent->client->menuchoice[0]] = 0;
			ent->client->pers.skills.pucurlvl[ent->client->menuchoice[0]] = 0;
			ent->client->pers.skills.pumaxlvl[ent->client->menuchoice[0]] = 0;
			restructurePlugins(ent);
			updateItemLevels(ent);
		}
		closeGiexMenu(ent);
	} else if (ent->client->showmenu == 11) { // Commands menu
		closeGiexMenu(ent);
		return;
	} else if (ent->client->showmenu == 13) { // Stats menu
		closeGiexMenu(ent);
		return;
	}
}
void selectNextMenu(edict_t *ent) {
	if (level.time < ent->client->menu_delay_time) {
		return;
	}
	ent->client->menu_delay_time = level.time + 0.2;

	if (ent->client->showmenu == 1) { //Inventory
		ent->client->menuchoice[0]++;
		if (ent->client->menuchoice[0] > (GIEX_ITEMPERCHAR - 1) || ent->client->pers.skills.itemId[ent->client->menuchoice[0]] == 0) {
			ent->client->menuchoice[0] = 0;
		}
		showGiexInventory(ent);
	} else if (ent->client->showmenu == 2) { //Specific item
		ent->client->menuchoice[1]++;
		if (ent->client->menuchoice[1] > 2) {
			ent->client->menuchoice[1] = 0;
		}
		showGiexItemMenu(ent);
	} else if (ent->client->showmenu == 3) {
		if (ent->client->menuchoice[0] < (GIEX_PUCARRYPERCHAR - 1) && ent->client->pers.skills.powerup_type[ent->client->menuchoice[0] + 1] > 0) {
			ent->client->menuchoice[0]++;
			showGiexPowerups(ent);
		}
	} else if (ent->client->showmenu == 4) { //powerup submenu
		if (ent->client->menuchoice[1] < GIEX_ITEMPERCHAR + 1) {
			ent->client->menuchoice[1]++;
			showGiexPowerupMenu(ent);
		}
	} else if (ent->client->showmenu == 5) { // Main menu
		if (ent->client->menuchoice[0] < 3) {
			ent->client->menuchoice[0]++;
			showGiexMainMenu(ent);
		} else {
			ent->client->menuchoice[0] = 0;
			showGiexMainMenu(ent);
		}
	} else if (ent->client->showmenu == 6) {
		if (ent->client->menuchoice[1] < (GIEX_HELPPAGES - 1)) {
			ent->client->menuchoice[1]++;
			showGiexHelpMenu(ent);
		}
	} else if (ent->client->showmenu == 7) {
		classinfo_t *info = getClassInfo(ent->client->menuchoice[1] + 1);
		if (ent->client->menuchoice[1] < (GIEX_NUMCLASSES - 1) && info->name[0] != '\0') {
			ent->client->menuchoice[1]++;
			showGiexClassesMenu(ent);
		}
	} else if (ent->client->showmenu == 9) {
		if (ent->client->menuchoice[0] < (GIEX_PUPERCHAR - 1) && ent->client->pers.skills.putype[ent->client->menuchoice[0] + 1] > 0) {
			ent->client->menuchoice[0]++;
			showGiexPlugins(ent);
		}
	} else if (ent->client->showmenu == 10) {
		if (ent->client->menuchoice[1] < 1) {
			ent->client->menuchoice[1]++;
			showGiexUnplug(ent);
		}
	} else if (ent->client->showmenu == 11) {
		if (ent->client->menuchoice[1] < (GIEX_COMMANDPAGES - 1)) {
			ent->client->menuchoice[1]++;
			showGiexCommandsMenu(ent);
		}
	} else if (ent->client->showmenu == 13) {
		if (ent->client->menuchoice[0] < 2) {
			ent->client->menuchoice[0]++;
			showGiexStats(ent);
		}
	}
}
void selectPrevMenu(edict_t *ent) {
	if (level.time < ent->client->menu_delay_time) {
		return;
	}
	ent->client->menu_delay_time = level.time + 0.2;

	if (ent->client->showmenu == 1) { // Inventory
		ent->client->menuchoice[0]--;
		if (ent->client->menuchoice[0] < 0) {
			int i;
			for (i = GIEX_ITEMPERCHAR - 1; i >= 0; i--) {
				if (ent->client->pers.skills.itemId[i] > 0) {
					break;
				}
			}
			ent->client->menuchoice[0] = i;
		}
		showGiexInventory(ent);
	} else if (ent->client->showmenu == 2) { // Item menu
		ent->client->menuchoice[1]--;
		if (ent->client->menuchoice[1] < 0) {
			ent->client->menuchoice[1] = 2;
		}
		showGiexItemMenu(ent);
	} else if (ent->client->showmenu == 3) { // Powerups list
		ent->client->menuchoice[0]--;
		if (ent->client->menuchoice[0] < 0) {
			int i;
			for (i = GIEX_PUCARRYPERCHAR - 2; i >= 0; i--) {
				if (ent->client->pers.skills.powerup_type[i] > 0) {
					break;
				}
			}
			ent->client->menuchoice[0] = i;
		}
		showGiexPowerups(ent);
	} else if (ent->client->showmenu == 4) { // Powerup submenu
		if (ent->client->menuchoice[1] > 0) {
			ent->client->menuchoice[1]--;
			showGiexPowerupMenu(ent);
		}
	} else if (ent->client->showmenu == 5) { // Main menu
		if (ent->client->menuchoice[0] > 0) {
			ent->client->menuchoice[0]--;
			showGiexMainMenu(ent);
		} else {
			ent->client->menuchoice[0] = 3;
			showGiexMainMenu(ent);
		}
	} else if (ent->client->showmenu == 6) { // Help menu
		if (ent->client->menuchoice[1] > 0) {
			ent->client->menuchoice[1]--;
			showGiexHelpMenu(ent);
		}
	} else if (ent->client->showmenu == 7) { // Classes menu
		if (ent->client->menuchoice[1] > 0) {
			ent->client->menuchoice[1]--;
			showGiexClassesMenu(ent);
		}
	} else if (ent->client->showmenu == 9) { // Plugin list
		ent->client->menuchoice[0]--;
		if (ent->client->menuchoice[0] < 0) {
			int i;
			for (i = GIEX_PUPERCHAR - 2; i >= 0; i--) {
				if (ent->client->pers.skills.putype[i] > 0) {
					break;
				}
			}
			ent->client->menuchoice[0] = i;
		}
		showGiexPlugins(ent);
	} else if (ent->client->showmenu == 10) { // Unplug menu
		if (ent->client->menuchoice[1] > 0) {
			ent->client->menuchoice[1]--;
			showGiexUnplug(ent);
		}
	} else if (ent->client->showmenu == 11) { // Commands menu
		if (ent->client->menuchoice[1] > 0) {
			ent->client->menuchoice[1]--;
			showGiexCommandsMenu(ent);
		}
	} else if (ent->client->showmenu == 13) { // Stats menu
		if (ent->client->menuchoice[0] > 0) {
			ent->client->menuchoice[0]--;
			showGiexStats(ent);
		}
	}
}

void closeGiexMenu(edict_t *ent) {
	ent->client->showscores = false;
	ent->client->showinventory = false;
	ent->client->showhelp = false;
	ent->client->showmenu = 0;
	ent->client->menuchoice[0] = 0;
	ent->client->menuchoice[1] = 0;
	ent->client->menuchoice[2] = 0;
	ent->client->menuchoice[3] = 0;
	ent->client->menuchoice[4] = 0;
}
