#include "g_local.h"

void Display_Range(edict_t *ent);

/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	if (deathmatch->value || coop->value)
		ent->client->showscores = true;

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
/*
	if (deathmatch->value || coop->value) {
		DeathmatchScoreboardMessage (ent, NULL);
		gi.unicast (ent, true);
	}
*/
	PMenu_Open(ent, ID_MAPVOTING, NULL, NULL);
}

void BeginIntermission (edict_t *targ) {
	int		i;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// allready activated

//ZOID
	if (deathmatch->value && ctf->value)
		CTFCalcScores();
//ZOID

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

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
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
void DL_Scoreboard (edict_t *ent, edict_t *killer);

void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
/*

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
//ZOID
	if (ctf->value) {
		CTFScoreboardMessage (ent, killer);
		return;
	}
//ZOID
*/
	DL_Scoreboard(ent, killer);

/*
	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		score = game.clients[i].resp.score;
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

	// print level name and exit rules
	string[0] = 0;

	stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 12)
		total = 12;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		picnum = gi.imageindex ("i_fixme");
		x = (i>=6) ? 160 : 0;
		y = 32 + 32 * (i%6);

		// add a dogtag
		if (cl_ent == ent)
			tag = "tag1";
		else if (cl_ent == killer)
			tag = "tag2";
		else
			tag = NULL;
		if (tag)
		{
			Com_sprintf (entry, sizeof(entry),
				"xv %i yv %i picn %s ",x+32, y, tag);
			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			DL_strcpy (string + stringlength, entry, -1);
			stringlength += j;
		}

		// send the layout
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe)/600);
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		DL_strcpy (string + stringlength, entry, -1);
		stringlength += j;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
*/
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
void Cmd_Score_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;
//ZOID
	if (ent->client->menu)
		PMenu_CloseAll(ent);
//ZOID

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		ent->client->update_chase = true;
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
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
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
	HelpComputer (ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent) {
int			index, pts_inuse = 0, pnum, dist;
vec3_t		diff, start, end; 
item_t		*item;
supply_t	*armour;
trace_t		tr;
ammo_t		*am;	

	//
	// ammo / weapon
	//
	// Note that we reference nextweap because nextweap *should* also
	// be the current weapon, but in addition, gives use timely updates
	// on weapon changes.
	// 
	ent->client->ps.stats[STAT_HEALTH_ICON] = 0;
	ent->client->ps.stats[STAT_HEALTH] = 0;
	ent->client->ps.stats[STAT_AMMO_ICON] = 0;
	ent->client->ps.stats[STAT_AMMO] = 0;
	ent->client->ps.stats[STAT_CLIP] = 0;
	ent->client->ps.stats[STAT_WEAPON_STRING] = 0;
	ent->client->ps.stats[STAT_AMMO_STRING] = 0;
	ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
	ent->client->ps.stats[STAT_ARMOR] = 0;
	ent->client->ps.stats[STAT_DL2_RESOURCES] = 0;
	ent->client->ps.stats[STAT_GOALITEM] = 0;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores ||
			(ent->client->menu[ent->client->curmenu]))
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	} else {
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

	if ((ent->movetype == MOVETYPE_NOCLIP) || (ent->deadflag == DEAD_DEAD))
		return;

	//
	// health
	//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	if (ent->client->curweap) {
		index = CS_WEAPONS + WEAP_INDEX(ITEMWEAP(ent->client->curweap));
		ent->client->ps.stats[STAT_WEAPON_STRING] = index;

		am = ent->client->nextammo;
		if (!(ITEMWEAP(ent->client->curweap)->flags & WT_NOAMMO) && am) { // curweap->cliptype) {
			if ((ent->client->weaponstate != WEAPON_RELOADING) || (ITEMWEAP(ent->client->curweap)->flags & WT_COUNTRELOAD)) {
				if (item = FindItemByName(ent, am->name, ITEM_AMMO))
					ent->client->ps.stats[STAT_AMMO] = item->quantity;

				ent->client->ps.stats[STAT_CLIP] = ent->client->curweap->clipcnt;
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex(am->pic_b);

				index = CS_AMMO_VARS + AMMO_INDEX(am);
				ent->client->ps.stats[STAT_AMMO_STRING] = index;
			}
		}
	}
	pts_inuse = CheckBuildingPoints(ent);
	if (ent->client->bldg_pts)
		ent->client->ps.stats[STAT_DL2_RESOURCES] = ent->client->bldg_pts - pts_inuse;

	if (ent->client->ps.stats[STAT_TRIGGER] && ent->client->trigger_time < level.time)
		ent->client->ps.stats[STAT_TRIGGER] = 0;

	// Display the players armour -
	if (ent->inventory) {
		int maxArmourVal = 0;
		item_t *maxArmour = NULL;
		
		if ((level.framenum & 0x07) == 0x04) {
			// Find the next armour item in the players inventory -
			// First, look for the last armour type (it may not exist since last frame)
			for (item = ent->inventory; item && (item != ent->last_armour); item = item->next);

			// Setup search for next armour type
			if (item) ent->last_armour = item->next;
			else ent->last_armour = ent->inventory;
			if (!ent->last_armour) ent->last_armour = ent->inventory;

			// Search for next armour type
			for (; ent->last_armour; ent->last_armour = ent->last_armour->next)
				if ((ent->last_armour->itemtype == ITEM_SUPPLY) && (ITEMSUPPLY(ent->last_armour)->type == SUP_ARMOUR))
					break;

			if (!ent->last_armour) {
				// If no armour was found in the last-half of the inventory, search again from the beginning
				ent->last_armour = ent->inventory;
				for (; ent->last_armour; ent->last_armour = ent->last_armour->next)
					if ((ent->last_armour->itemtype == ITEM_SUPPLY) && (ITEMSUPPLY(ent->last_armour)->type == SUP_ARMOUR))
						break;
			}
		}
		if (ent->last_armour) {
			armour = ITEMSUPPLY(ent->last_armour);
			if (armour) {
				if (armour->pic_b)
					ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex(armour->pic_b);
				else
					ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex("i_combatarmor");
				ent->client->ps.stats[STAT_ARMOR] = ent->last_armour->quantity;

			}
		}

		// Find out which type of armour the player has the most of.
		for (item = ent->inventory; item; item = item->next) {
			if ((item->itemtype == ITEM_SUPPLY) && (ITEMSUPPLY(item)->type == SUP_ARMOUR)) {
				if (item->quantity > maxArmourVal) {
					maxArmourVal = item->quantity;
					maxArmour = item;
				}
			}
		}

		// display the armour (if any)
		if (maxArmour) {
			armour = ITEMSUPPLY(maxArmour);
			if (armour->model) {
				if (armour->model[0] == '*') {
					char *p, skin[MAX_QPATH];
					strcpy(skin, Info_ValueForKey(ent->client->pers.userinfo, "skin"));
					for (p = skin; *p && (*p != '/'); p++);
					*p = 0;
					ent->s.modelindex4 = gi.modelindex(va("players/%s/%s", skin, &armour->model[1]));
				} else
					ent->s.modelindex4 = gi.modelindex(armour->model);
			} else ent->s.modelindex4 = gi.modelindex("players/male/vest.md2");
		} else 
			ent->s.modelindex4 = 0;
	} else
		ent->s.modelindex2 = ent->s.modelindex3 = ent->s.modelindex4 = 0;

	//
	// frags
	//

	if (dlsys.playmode & PM_TEAMSCORES)
		ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.team->score;
	else
		ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	// Team score
	if (ent->client->resp.team) {
		ent->client->ps.stats[STAT_TEAMSCORE] = ent->client->resp.team->score;
		if ((ent->client->resp.team != ent->team) && (level.framenum & 0x04))
			ent->client->ps.stats[STAT_TEAMICON] = gi.imageindex(ent->team->pic_b);
		else
			ent->client->ps.stats[STAT_TEAMICON] = gi.imageindex(ent->client->resp.team->pic_b);
	} else {
		ent->client->ps.stats[STAT_TEAMSCORE] = 0;
		ent->client->ps.stats[STAT_TEAMICON] = 0;
	}

	// Goal item
	if (ent->goalitem && ent->goalitem->team_own && (ent->goalitem->goalitem == ent) && (level.framenum & 0x04))
		ent->client->ps.stats[STAT_GOALITEM] = gi.imageindex(dlsys.teams[ent->goalitem->team_own-1].pic_b);

//ZOID
//	SetCTFStats(ent);
//ZOID

// DEADLODE - Player using a SmartScope?
	if (ent->scope && !ent->client->dummy)
		SmartScope_Think(ent);
	else {
		ent->client->ps.stats[STAT_ID_NAME] = 0; 
		ent->client->ps.stats[STAT_ID_TEAM] = 0; 
		ent->client->ps.stats[STAT_ID_RANGE] = 0; 
		ent->client->ps.stats[STAT_ID_HITLOC] = 0;
		ent->client->ps.stats[STAT_ID_WEAPON] = 0;
		
		if (!ent->client->dummy)
			ent->client->ps.fov = 90;

		VectorCopy(ent->s.origin, start);
		start[2] += ent->viewheight;
		AngleVectors(ent->client->v_angle, end, NULL, NULL);
		VectorMA(start, 2000, end, end);
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT); 
		if ((tr.ent->client) && (tr.ent->light_level > 16)) { 
			pnum = tr.ent-g_edicts-1;
			VectorSubtract(start, tr.endpos, diff); 
			dist = VectorLength(diff) / 30;
			if (dist < 25) {
				if (dist < 10)
					ent->client->ps.stats[STAT_ID_NAME] = CS_PLAYER_NAMES + pnum; 
				ent->client->ps.stats[STAT_ID_TEAM] = gi.imageindex(tr.ent->team->pic_b); // ID reports spies as undercover
				ent->oldenemy = tr.ent;
				ent->oldenemy_time = level.time + 1.0;
			}
		}
	}

// DEADLODE
}
