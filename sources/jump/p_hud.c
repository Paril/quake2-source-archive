#include "g_local.h"



/*
======================================================================

INTERMISSION

======================================================================
*/

void JumpScoreboardMessage (void)
{
	int y, j, stringlength;
	edict_t *from = g_edicts;
	char entry[1400];
	char string[1400];
	struct tm *newtime;

	strcpy (string, "xv 32 yv 32 string2 \"Name            Secs   Date (UK)\"");

	y = 40;
	stringlength = strlen(string);
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;

		if (!from->jumpinfo)
			continue;

		newtime = localtime ((const time_t *)&from->jumpinfo->date);

		if (from->item->count > 1) {
			Com_sprintf (entry, sizeof(entry),
				"xv 0 yv %d string2 \"%s (#%d)\" xv 32 yv %d string \"%-15.15s %-6.1f %2.2d/%2.2d/%4.4d\""
			, y, from->item->pickup_name, from->count, y+8, from->jumpinfo->name, (float)from->jumpinfo->time / 10.0f, newtime->tm_mday, newtime->tm_mon+1, 1900 + newtime->tm_year);
		} else {
			Com_sprintf (entry, sizeof(entry),
				"xv 0 yv %d string2 \"%s\" xv 32 yv %d string \"%-15.15s %-6.1f %2.2d/%2.2d/%4.4d\""
			, y, from->item->pickup_name, y+8, from->jumpinfo->name, (float)from->jumpinfo->time / 10.0f, newtime->tm_mday, newtime->tm_mon+1, 1900 + newtime->tm_year);
		}

		y += 24;

		j = strlen(entry);

		if (stringlength + j > 1024)
			break;

		strcpy (string + stringlength, entry);
		stringlength += j;
	}
		
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


void MoveClientToIntermission (edict_t *ent)
{
	if (deathmatch->value || coop->value)
		ent->client->showscores = 2;
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

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	ent->client->chase_target = NULL;

	ent->client->ps.stats[STAT_CTF_ID_VIEW] = ent->client->ps.stats[STAT_CHASE] = 0;

	// add the layout

	if (deathmatch->value || coop->value)
	{
		JumpScoreboardMessage ();
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
void DeathmatchScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
//	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;

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

	stringlength = 0;

	// add the clients in sorted order
	if (total > 12)
		total = 12;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

//		picnum = gi.imageindex ("i_fixme");
		x = (i>=6) ? 160 : 0;
		y = 32 + 32 * (i%6);

		// add a dogtag
		if (cl_ent == ent)
			tag = "tag1";
		else if (game.clients[i].resp.spectator)
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
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		// send the layout
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe)/600);
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
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
	if (ent->delay > level.time) {
		ent->client->showscores |= 16384;
		return;
	}

	ent->client->showscores &= ~16384;

	ent->delay = level.time + .2;

	if (ent->client->showscores == 1)
		DeathmatchScoreboardMessage (ent);
	else if (ent->client->showscores == 2)
		JumpScoreboardMessage();
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
	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = 0;
		return;
	}

	ent->client->showscores = 1;
	DeathmatchScoreboard (ent);
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
}

static void loc_buildboxpoints(vec3_t /*@out@*/p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}

static qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported

	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;

	for (i = 0; i < 8; i++) {
		trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}

	return false;
}

void CTFSetIDView(edict_t *ent)
{
	vec3_t	forward, dir;
	trace_t	tr;
	edict_t	*who, *best;
	float	bd = 0, d;
	int i;

	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;

	if (ent->client->chase_target)
		return;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 1024, forward);
	VectorAdd(ent->s.origin, forward, forward);
	//forward[2] += ent->viewheight;

	tr = gi.trace(ent->s.origin, NULL, NULL, forward, ent, MASK_SOLID);
	if (tr.fraction < 1 && tr.ent && tr.ent->client) {
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
			CS_GENERAL + (ent - g_edicts - 1);
		return;
	}

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);

	best = NULL;
	for (i = 1; i <= maxclients->value; i++) {
		who = g_edicts + i;
		if (!who->inuse || who->client->pers.spectator || who->health < 1)
			continue;
		VectorSubtract(who->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd && loc_CanSee(ent, who)) {
			bd = d;
			best = who;
		}
	}
	if (bd > 0.97)
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
			CS_GENERAL + (best - g_edicts - 1);
}

//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index;

	//
	// health
	//
	if (ent->client->resp.spectator) {
		ent->client->ps.stats[STAT_HEALTH_ICON] = 0;
		ent->client->ps.stats[STAT_HEALTH] = 0;
	} else {
		ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
		ent->client->ps.stats[STAT_HEALTH] = ent->health;
	}

	//
	// ammo
	//
	//if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */)
	//{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	//}
	//else
	//{
	//	item = &itemlist[ent->client->ammo_index];
	//	ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
	//	ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	//}
	
	//
	// armor
	//

	index = ArmorIndex (ent);
	if (index)
	{
		item = GetItemByIndex (index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
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
	//if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	//else
	//	ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);

	// ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->health <= 0 || level.intermissiontime
			|| ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	}
	else
	{
		if (ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	}

	//
	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	ent->client->ps.stats[STAT_HELPICON] = 0;

	//ent->client->ps.stats[STAT_SPECTATOR] = 0;

	CTFSetIDView (ent);
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
void G_SetSpectatorStats (edict_t *ent)
{
	gclient_t *cl = ent->client;

	if (!cl->chase_target)
		G_SetStats (ent);

	//cl->ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independant in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;
	if (ent->health <= 0 || level.intermissiontime || cl->showscores)
		cl->ps.stats[STAT_LAYOUTS] |= 1;

	if (cl->chase_target && cl->chase_target->inuse)
		cl->ps.stats[STAT_CHASE] = CS_GENERAL + 
			(cl->chase_target - g_edicts) - 1;
	else
		cl->ps.stats[STAT_CHASE] = 0;
}

