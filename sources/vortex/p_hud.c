#include "g_local.h"

// get our data from invasion. -az
extern struct invdata_s
{
	int printedmessage;
	int mspawned;
	float limitframe;
	edict_t *boss;

} invasion_data;

/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission(edict_t *ent)
{
	ent->client->showscores = true;
	//	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0] * 8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1] * 8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2] * 8;
	VectorCopy(level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.viewangles[ROLL] = 0;
	ent->client->ps.kick_angles[ROLL] = 0;

	VectorCopy(level.intermission_origin, ent->s.origin);
	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	//	RemoveAllAuras(ent);
	//	RemoveAllCurses(ent);
	AuraRemove(ent, 0);
	CurseRemove(ent, 0);

	// RAFAEL
	ent->client->quadfire_framenum = 0;

	// RAFAEL
	//ent->client->trap_blew_up = false;
	//ent->client->trap_time = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	if (deathmatch->value && !(ent->svflags & SVF_MONSTER))
	{
		DeathmatchScoreboardMessage(ent, NULL);
		gi.unicast(ent, true);
	}

}
void SetLVChanged(int i);
int GetLVChanged(void);

//3.0 new intermission routine
void VortexBeginIntermission(char *nextmap)
{
	int		i;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// allready activated

	level.intermissiontime = level.time;
	level.changemap = nextmap;
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

	// move all clients to the intermission point
	for (i = 0; i<maxclients->value; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission(client);
	}
}

//old intermission code. Should be obsolete, but left in just in case
void BeginIntermission(edict_t *targ)
{
	int		i;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// allready activated

	gi.dprintf("WARNING: BeginIntermission() was called when we should be calling VortexBeginIntermission()!\n");

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	// if on same unit, return immediately
	if (!deathmatch->value && (targ->map && targ->map[0] != '*'))
	{	// go immediately to the next level
		level.exitintermission = 1;
		return;
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

	// move all clients to the intermission point
	for (i = 0; i<maxclients->value; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission(client);
	}
}

int V_HighestFragScore(void)
{
	int i, highScore = 0;
	edict_t *cl_ent;

	for (i = 0; i<game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || G_IsSpectator(cl_ent))
			continue;
		if (!highScore || (cl_ent->client->resp.frags > highScore))
			highScore = cl_ent->client->resp.frags;
	}
	return highScore;
}


/*
==================
DeathmatchScoreboardMessage *Improved!*
==================
*/

#define MAX_ENTRY_SIZE		1024
#define MAX_STRING_SIZE		1400

void DeathmatchScoreboardMessage(edict_t *ent, edict_t *killer)
{

	char entry[MAX_ENTRY_SIZE];
	char string[MAX_STRING_SIZE];
	char name[23], classname[20];//3.78
	int stringlength;
	int i, j, k;
	int sorted[MAX_CLIENTS];
	int sortedscores[MAX_CLIENTS];
	int score, total, highscore = 0;
	int y;
	//float accuracy;
	int time_left = 999, frag_left = 999;
	gclient_t *cl;
	edict_t *cl_ent;

	// if we are looking at the scoreboard or inventory, deactivate the scanner
	if (ent->client->showscores || ent->client->showinventory)
	{
		if (ent->client->pers.scanner_active)
			ent->client->pers.scanner_active = 2;
	}
	else
	{
		*string = 0;

		// Scanner active ?
		if (ent->client->pers.scanner_active & 1)
			ShowScanner(ent, string);

		// normal quake code ...	
		gi.WriteByte(svc_layout);
		gi.WriteString(string);
		return;
	}

	// sort the clients by score 
	total = 0;
	for (i = 0; i<game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;

		//3.0 scoreboard code fix
		if (!cl_ent->client || !cl_ent->inuse)
			continue;

		if (cl_ent->client->resp.spectator)
			score = 0;
		else
			score = game.clients[i].resp.score;

		for (j = 0; j<total; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k = total; k>j; k--)
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

	// make a header for the data 
	//K03 Begin
	if (timelimit->value)
		time_left = (timelimit->value * 60 - level.time);
	else
		time_left = 60 * 99;
	if (fraglimit->value)
		frag_left = (fraglimit->value - V_HighestFragScore());

	if (time_left < 0)
		time_left = 0;

	Com_sprintf(entry, sizeof(entry),
		"xv 0 yv 16 string2 \"Time:%2im %2is Frags:%3i Players:%3i\" "
		"xv 0 yv 34 string2 \"Name       Lv Cl Score Frg Spr Png\" ",
		(int)(time_left / 60), (int)(time_left - (int)((time_left / 60) * 60)),
		frag_left, JoinedPlayers());

	//K03 End
	j = strlen(entry);

	//gi.dprintf("header string length=%d\n", j);

	if (stringlength + j < MAX_ENTRY_SIZE)
	{
		strcpy(string + stringlength, entry);
		stringlength += j;
	}

	// add the clients in sorted order 
	if (total > 24)
		total = 24;
	/* The screen is only so big :( */

	for (i = 0; i<total; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		if (!cl_ent)
			continue;

		y = 44 + 8 * i;

		// 3.78 truncate client's name and class string
		strcpy(name, V_TruncateString(cl->pers.netname, 11));
		strcpy(classname, V_TruncateString(GetClassString(cl_ent->myskills.class_num), 3));
		padRight(name, 10);

		Com_sprintf(entry, sizeof(entry),
			"xv %i yv %i string \"%s%s %2i %s %5i %3i %3i %3i\" ",
			cl_ent->client->resp.spectator ? -24 : 0,
			y, cl_ent->client->resp.spectator ? "(s)" : "", name,
			cl_ent->client->resp.spectator ? 0 : cl_ent->myskills.level,
			cl_ent->client->resp.spectator ? "??" : classname, cl_ent->client->resp.spectator ? 0 : cl->resp.score,
			cl_ent->client->resp.spectator ? 0 : cl->resp.frags,
			cl_ent->client->resp.spectator ? 0 : cl_ent->myskills.streak, cl->ping);

		j = strlen(entry);

		//gi.dprintf("player string length=%d\n", j);

		if (stringlength + j > MAX_ENTRY_SIZE)
			break;

		strcpy(string + stringlength, entry);
		stringlength += j;
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);

}
//K03 End


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard(edict_t *ent)
{
	//GHz START
	if (ent->client->menustorage.menu_active)
		closemenu(ent);
	//GHz END
	DeathmatchScoreboardMessage(ent, ent->enemy);
	gi.unicast(ent, false);
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

	if (ent->client->pers.scanner_active & 1)
		ent->client->pers.scanner_active = 0;//2;

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		return;
	}

	ent->client->showscores = true;
	DeathmatchScoreboard(ent);
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

	sk = "hard+";

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

	if (ent->client->showhelp && (ent->client->resp.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->resp.helpchanged = 0;
	HelpComputer(ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/

char *V_SetColorText(char *buffer)
{
	size_t	len;
	int		i;

	len = strlen(buffer);
	for (i = 0; i < len; i++)
	{
		if (buffer[i] != '\n')
			buffer[i] |= 0x80;
	}

	return buffer;
}

qboolean V_ValidIDTarget(edict_t *ent, edict_t *other, qboolean vis)
{
	if (!other || !other->inuse || !other->takedamage || other->solid == SOLID_NOT || other->health < 1)
		return false;

	if (vis && ent && !visible(ent, other))
		return false;

	return true;
}

edict_t *V_GetIDTarget(edict_t *ent)
{
	vec3_t forward, right, offset, start, end;
	trace_t tr;

	// find entity near crosshair
	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7, ent->viewheight - 8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if (V_ValidIDTarget(NULL, tr.ent, false))
	{
		ent->client->idtarget = tr.ent;
		return tr.ent;
	}
	else
		return NULL;
}

void PlayerID_SetStats(edict_t *player, edict_t *target, qboolean chasecam)
{
	int		health, armor = 0, ammo = 0, lvl = 0;
	float	dist;
	char	name[24], buf[100];
	int		team_status = 0;

	if (player->ai.is_bot)
		return;

	dist = entdist(player, target);
	health = target->health;

	team_status = OnSameTeam(player, target);

	if (target->client)
	{
		sprintf(name, target->client->pers.netname);
		armor = target->client->pers.inventory[body_armor_index];
		ammo = target->client->pers.inventory[target->client->ammo_index];
		lvl = target->myskills.level;
	}
	else
	{
		name[0] = 0;

		// name
		if (PM_MonsterHasPilot(target))
			strcat(name, target->owner->client->pers.netname);
		else if (target->mtype)
			strcat(name, V_GetMonsterName(target));
		else
			strcat(name, target->classname);

		// armor
		if (target->monsterinfo.power_armor_type)
			armor = target->monsterinfo.power_armor_power;

		// ammo
		if (target->mtype && (target->mtype == M_SENTRY || target->mtype == M_AUTOCANNON))
			ammo = target->light_level;

		// level
		if (target->monsterinfo.level > 0)
			lvl = target->monsterinfo.level;
	}

	// initialize the string by terminating it, required by strcat()
	buf[0] = 0;

	if (chasecam)
		strcat(buf, va("Chasing "));

	// build the string
	strcat(buf, va("%s ", name));
	if (team_status > 1)
		V_SetColorText(buf);

	strcat(buf, va("(%d) ", lvl));

	if (!chasecam)
	{
		// newbies get a very basic free ID
		/*if (player->myskills.abilities[ID].current_level < 1)
		{
		if (M_IgnoreInferiorTarget(target, player) && !team_status)
		strcat(buf, "is ignoring you");
		}
		else*/
		{
			strcat(buf, va("@ %.0f", dist));
			strcat(buf, va(": %dh", health));
			if (armor)
				strcat(buf, va("/%da", armor));
			if (ammo)
				strcat(buf, va("/%d", ammo));
		}
	}

	// set stat to the configstring's index
	// this is the index to the string we just made
	player->client->ps.stats[STAT_CHASE] = CS_GENERAL + 1;

	gi.WriteByte(13);//svc_configstring
	gi.WriteShort(CS_GENERAL + 1);
	gi.WriteString(buf);// put the string in the configstring list
	gi.unicast(player, false);// announce to this player only
}

void V_PlayerID(edict_t *ent, edict_t *targ)
{

	edict_t *target;

	if (targ)
	{
		// chasecam
		PlayerID_SetStats(ent, targ, true);
		return;
	}

	// always try to find a new target
	target = V_GetIDTarget(ent);
	if (!target)
	{
		// if we can't find a new one, try using the old one if it's valid
		if (V_ValidIDTarget(ent, ent->client->idtarget, true))
			target = ent->client->idtarget;
		else
		{
			ent->client->ps.stats[STAT_CHASE] = 0;
			return;
		}
	}

	PlayerID_SetStats(ent, target, false);
}

void Weapon_Sword(edict_t *ent);
void G_SetStats(edict_t *ent)
{
	gitem_t		*item;
	//	int			num;
	int			index, cells;
	int			power_armor_type = 0;

	//	int			time_left;//K03

	//
	// health
	//


	// ent->client->ps.stats[STAT_HEALTH_ICON] = 0;

	if (ent->health <= 10000)
		ent->client->ps.stats[STAT_HEALTH] = ent->health;
	else
		ent->client->ps.stats[STAT_HEALTH] = 666;

	//GHz START
	// 3.5 show percent ability is charged
	if (ent->client->charge_index)
		ent->client->ps.stats[STAT_CHARGE_LEVEL] = ent->myskills.abilities[ent->client->charge_index - 1].charge;
	else
		ent->client->ps.stats[STAT_CHARGE_LEVEL] = 0;

	if (G_EntExists(ent->supplystation) /*&& (ent->supplystation->wait >= level.time)*/)
	{
		ent->client->ps.stats[STAT_STATION_ICON] = gi.imageindex("i_tele");
		//if (ent->supplystation->wait < 100)
		ent->client->ps.stats[STAT_STATION_TIME] = (int)ent->supplystation->wait;//-level.time;
		//else
		//	ent->client->ps.stats[STAT_STATION_TIME] = 0;
	}
	else
	{
		ent->client->ps.stats[STAT_STATION_ICON] = 0;
		ent->client->ps.stats[STAT_STATION_TIME] = 0;
	}

	if (ptr->value || domination->value || tbi->value)
	{
		//if (G_EntExists(ent))
		if (ent->inuse && ent->teamnum)
		{
			index = 0;
			if (ent->teamnum == 1)
			{
				if (DEFENSE_TEAM == ent->teamnum || tbi->value) // show team color pic in tbi, always.
					index = gi.imageindex("i_ctf1"); // show team color pic
				else
					index = gi.imageindex("i_ctf1d"); // not in control
			}
			else if (ent->teamnum == 2 || tbi->value)
			{
				if (DEFENSE_TEAM == ent->teamnum || tbi->value)
					index = gi.imageindex("i_ctf2");
				else
					index = gi.imageindex("i_ctf2d");
			}
			ent->client->ps.stats[STAT_TEAM_ICON] = index;
		}
		else
		{
			ent->client->ps.stats[STAT_TEAM_ICON] = 0;
		}

	}

	if (ctf->value)
	{
		if (ent->inuse && ent->teamnum)
		{
			edict_t *base;
			index = 0;

			if ((base = CTF_GetFlagBaseEnt(ent->teamnum)) != NULL)
			{
				if (ent->teamnum == RED_TEAM)
				{
					if (base->count == BASE_FLAG_SECURE)
						index = gi.imageindex("i_ctf1"); // show team color pic
					else
					{
						if (level.framenum & 8)
							index = gi.imageindex("i_ctf1d"); // flag taken
						else
							index = gi.imageindex("i_ctf1");
					}
				}
				else if (ent->teamnum == BLUE_TEAM)
				{
					if (base->count == BASE_FLAG_SECURE)
						index = gi.imageindex("i_ctf2");
					else
					{
						if (level.framenum & 8)
							index = gi.imageindex("i_ctf2d");
						else
							index = gi.imageindex("i_ctf2");
					}
				}
			}
			ent->client->ps.stats[STAT_TEAM_ICON] = index;
		}
		else
		{
			ent->client->ps.stats[STAT_TEAM_ICON] = 0;
		}
	}
	//GHz END

	// player boss
	if (G_EntExists(ent->owner))
		ent->client->ps.stats[STAT_HEALTH] = ent->owner->health;

	//
	// ammo
	//
	if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_blaster_hud");
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		//ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
		if (ent->client->pers.weapon == Fdi_SHOTGUN)
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_shells_hud");
		else if (ent->client->pers.weapon == Fdi_SUPERSHOTGUN)
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_shells_hud");
		else if (ent->client->pers.weapon == Fdi_MACHINEGUN)
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_bullets_hud");
		else if (ent->client->pers.weapon == Fdi_CHAINGUN)
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_bullets_hud");
		else if (ent->client->pers.weapon == Fdi_GRENADES)
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_grenades_hud");
		else if (ent->client->pers.weapon == Fdi_GRENADELAUNCHER)
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_grenades_hud");
		else if (ent->client->pers.weapon == Fdi_ROCKETLAUNCHER)
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_rockets_hud");
		else if (ent->client->pers.weapon == Fdi_HYPERBLASTER)
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_cells_hud");
		else if (ent->client->pers.weapon == Fdi_RAILGUN)
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_slugs_hud");
		else if (ent->client->pers.weapon == Fdi_BFG)
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_cells_hud");
		else if (ent->client->pers.weapon == Fdi_20MM)
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_shells_hud");
		else
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_blaster_hud");
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}
	//GHz START
	// blaster weapon ammo
	if (ent->client->pers.weapon == Fdi_BLASTER)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_cells_hud");
		ent->client->ps.stats[STAT_AMMO] = ent->monsterinfo.lefty;
	}
	//GHz END

	//
	// armor
	//

	power_armor_type = PowerArmorType(ent);

	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(Fdi_CELLS)];

		if (cells == 0)
		{	// ran out of cells for power armor
			ent->flags &= ~FL_POWER_ARMOR;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;
		}
	}

	index = ArmorIndex(ent);
	if (power_armor_type && (!index || ent->mtype || (level.framenum & 8)))//4.2 morphed players only have powered armor
	{	// flash between power armor and other armor icon
		ent->client->ps.stats[STAT_ARMOR] = cells;
	}
	else if (index && !ent->mtype)
	{
		item = GetItemByIndex(index);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}

	// morphed players
	if (ent->mtype)
	{
		if ((ent->mtype == MORPH_MEDIC) && (ent->client->weapon_mode == 0 || ent->client->weapon_mode == 2))
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_cells_hud");
			ent->client->ps.stats[STAT_AMMO] = ent->myskills.abilities[MEDIC].ammo;
		}
		else if (ent->mtype == MORPH_FLYER)
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_cells_hud");
			ent->client->ps.stats[STAT_AMMO] = ent->myskills.abilities[FLYER].ammo;
		}
		else if (ent->mtype == MORPH_CACODEMON)
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_rockets_hud");
			ent->client->ps.stats[STAT_AMMO] = ent->myskills.abilities[CACODEMON].ammo;
		}
		else
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_blaster_hud");
			ent->client->ps.stats[STAT_AMMO] = 0;
		}

	}

	// player-monsters
	if (PM_PlayerHasMonster(ent))
	{
		if (ent->owner->mtype == P_TANK)
		{
			if (ent->client->weapon_mode == 0)
			{
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_rockets_hud");
				ent->client->ps.stats[STAT_AMMO] = ent->owner->monsterinfo.jumpup;
			}
			else if (ent->client->weapon_mode == 2)
			{
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_bullets_hud");
				ent->client->ps.stats[STAT_AMMO] = ent->owner->monsterinfo.lefty;
			}
			else if (ent->client->weapon_mode == 3)
			{
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_cells_hud");
				ent->client->ps.stats[STAT_AMMO] = ent->owner->monsterinfo.radius;
			}
			else
			{
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_blaster_hud");
				ent->client->ps.stats[STAT_AMMO] = 0;
			}

			if (power_armor_type)
			{
				ent->client->ps.stats[STAT_ARMOR] = cells;
			}
			else
			{
				ent->client->ps.stats[STAT_ARMOR] = 0;
			}

		}
		else
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_blaster_hud");
			ent->client->ps.stats[STAT_AMMO] = 0;
		}
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
	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum) / 10;
	}
	// RAFAEL
	else if (ent->client->quadfire_framenum > level.framenum)
	{
		// note to self
		// need to change imageindex
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_quadfire");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quadfire_framenum - level.framenum) / 10;
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
	//K03 Begin
	else if (ent->client->thrusting == 1 ||
		ent->client->cloakable ||
		ent->client->hook_state)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("k_powercube");
		ent->client->ps.stats[STAT_TIMER] = ent->client->pers.inventory[power_cube_index];
	}
	//K03 End
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
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex(itemlist[ent->client->pers.selected_item].icon);

	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores || ent->client->pers.scanner_active)
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
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// help icon / current weapon if not shown
	//
	if (ent->client->resp.helpchanged && (level.framenum & 8))
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex("i_help");
	else if ((ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon && !G_IsSpectator(ent))
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex(ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

	//K03 Begin
	//ent->client->ps.stats[STAT_LEVEL] = ent->myskills.level;
	ent->client->ps.stats[STAT_STREAK] = ent->myskills.streak;

	/*if (timelimit->value)
	time_left = (timelimit->value*60 - level.time);
	else
	time_left = 60*99;

	ent->client->ps.stats[STAT_TIMEMIN] = (int)(time_left/60);*/
	ent->client->ps.stats[STAT_TIMEMIN] = ent->client->pers.inventory[power_cube_index];
	if (ent->client->ps.stats[STAT_TIMEMIN] < 0)
		ent->client->ps.stats[STAT_TIMEMIN] = 0;

	// az invasion stuff.
	if (invasion->value && level.time > pregame_time->value && !level.intermissiontime)
	{
		int invtime = invasion_data.limitframe - level.time;
		if (invtime > 0)
			ent->client->ps.stats[STAT_INVASIONTIME] = invtime;
		else
			ent->client->ps.stats[STAT_INVASIONTIME] = 0;
	}
	else
		ent->client->ps.stats[STAT_INVASIONTIME] = 0;

	if (V_VoteInProgress() && !G_IsSpectator(ent)) // show message only to non spectators
		ent->client->ps.stats[STAT_VOTESTRING] = CS_GENERAL + MAX_CLIENTS + 1;
	else
		ent->client->ps.stats[STAT_VOTESTRING] = 0;

	if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_NUM] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_NUM] = ent->client->pers.inventory[ent->client->pers.selected_item];
	//K03 End

	//GHz Begin
	V_PlayerID(ent, NULL);
	// id code
	//GHz End
	//GHz START
	if (level.time > ent->lastdmg + 3)
		ent->client->ps.stats[STAT_ID_DAMAGE] = 0;
	//GHz END

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

	//GHz START
	// set stats for non-client targets
	if (ent->client->chase_target && !ent->client->chase_target->client)
	{
		// player-monster chase stats
		if (PM_MonsterHasPilot(ent->client->chase_target))
		{
			// use stats of the monster's owner
			memcpy(ent->client->ps.stats, ent->client->chase_target->owner->client->ps.stats,
				sizeof(ent->client->ps.stats));
			G_SetSpectatorStats(ent);

			// set a configstring to the player's name
			//gi.configstring (CS_GENERAL+(ent-g_edicts-1), 
			//	va("Chasing %s", ent->client->chase_target->owner->client->pers.netname));
			// set stat to the configstring's index
			//ent->client->ps.stats[STAT_CHASE] = CS_GENERAL+(ent-g_edicts-1);
		}
		else
		{
			if (ent->client->chase_target->health <= 999)
				ent->client->ps.stats[STAT_HEALTH] = ent->client->chase_target->health;
			else
				ent->client->ps.stats[STAT_HEALTH] = 666;

			if (ent->client->chase_target->monsterinfo.power_armor_power)
			{
				ent->client->ps.stats[STAT_ARMOR] = ent->client->chase_target->monsterinfo.power_armor_power;
			}
			else
			{
				ent->client->ps.stats[STAT_ARMOR] = 0;
			}
			// monsters don't have ammo
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_blaster_hud");
			ent->client->ps.stats[STAT_AMMO] = 0;
			// set a configstring to the monster's classname
			//if (ent->client->chase_target->mtype)
			//	gi.configstring (CS_GENERAL+(ent-g_edicts-1), va("Chasing %s (%d)", 
			//		V_GetMonsterName(ent->client->chase_target), ent->client->chase_target->monsterinfo.level));
			//else
			//	gi.configstring (CS_GENERAL+(ent-g_edicts-1), va("Chasing %s", ent->client->chase_target->classname));
			// set stat to the configstring's index
			//ent->client->ps.stats[STAT_CHASE] = CS_GENERAL+(ent-g_edicts-1);
		}
		return; // since this player has a chase target, it must be a spectator which can't be chased!
	}
	//GHz END
	for (i = 1; i <= maxclients->value; i++) {
		cl = g_edicts[i].client;
		if (!g_edicts[i].inuse || cl->chase_target != ent)
			continue;
		if (ent->client->pers.weapon)
			ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
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
	else if (cl->chase_target && cl->chase_target->inuse)
		V_PlayerID(ent, cl->chase_target);

	cl->ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independant in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;
	if (cl->pers.health <= 0 || level.intermissiontime || cl->showscores)
		cl->ps.stats[STAT_LAYOUTS] |= 1;
	if (cl->showinventory && cl->pers.health > 0)
		cl->ps.stats[STAT_LAYOUTS] |= 2;
}


