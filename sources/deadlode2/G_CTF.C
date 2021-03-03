#include "g_local.h"
/*
typedef struct ctfgame_s
{
	int team1, team2;
	int total1, total2; // these are only set when going into intermission!
	float last_flag_capture;
	int last_capture_team;
} ctfgame_t;

ctfgame_t ctfgame;
qboolean techspawn = false;
*/

cvar_t *ctf;
cvar_t *ctf_forcejoin;

char *ctf_statusbar =
"yb	-24 "

// health
"if 1 "
	"xv 0 hnum "					// Health
	"xv 50 pic 0 "					// Health Icon
"endif "

// armor
"if 5 "
	"xv 0 yb -50 rnum "				// Armour Value
	"xv 50 pic 4 "					// Armour Icon
"endif "

// ammo
"if 16 "
	"xv 100 yb -24 anum "			// Rounds total
	"xv 150 pic 2 "					// Ammo-Icon
	"xv 100 yb -50 num 3 16 "		// Rounds in Clip
	"xv 150 yb -42 stat_string 18 "	// Ammo Description
"endif "

// Weapon Description
"if 17 "
	"xv 150 yb -50 stat_string 17 "
"endif "

// ID Target Name
"if 6 "
	"xv 165 yv 80 stat_string 6 "
"endif "
// ID Target Team
"if 7 "
	"xv 165 yv 90 pic 7 "
"endif "
// ID Target Weapon
"if 8 "
	"xv 165 yv 100 stat_string 8 "
"endif "
// ID Target Range
"if 9 "
	"xv 105 yv 80 num 3 9 "
"endif "
// ID Target Crosshair
"if 10 "
	"xv 128 yv 88 pic 10 "
"endif "

// Trigger Status
"if 11 "
	"xr -25 yb -100 pic 11 " 
"endif "

"if 21 "
//  Player frags
	"yt 2 xr -80 num 3 14 "
// Team score
	"yt 28 xr -26 pic 21 "
	"yt 28 xr -80 num 3 22 "
"endif "

// Building Resource Points
"if 20 "
	"xv 182 yb -24 num 4 20 "
	"xv 250 picn supplies/i_tools "
"endif "

// have flag graph
"if 12 "
  "xr -24 yt 26 pic 12 "
"endif "
;

static char *tnames[] = {
	"item_tech1", "item_tech2", "item_tech3", "item_tech4",
	NULL
};

/*--------------------------------------------------------------------------*/

/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
static edict_t *loc_findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
#if 0
		if (from->solid == SOLID_NOT)
			continue;
#endif
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}

static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
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

/*--------------------------------------------------------------------------*/

static gitem_t *flag1_item;
static gitem_t *flag2_item;

void CTFInit(void)
{
	ctf = gi.cvar("ctf", "1", CVAR_SERVERINFO);
	ctf_forcejoin = gi.cvar("ctf_forcejoin", "", 0);
}


edict_t *SelectRandomDeathmatchSpawnPoint (void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
float	PlayersRangeFromSpot (edict_t *spot);

// called when we enter the intermission
void CTFCalcScores(void)
{
	int i;

	for (i = 0; i < MAX_TEAMS; i++)
		dlsys.teams[i].totalfrags = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		game.clients[i].resp.team->totalfrags += game.clients[i].resp.score;
	}
}
/*------------------------------------------------------------------------*/

/*QUAKED info_player_team1 (1 0 0) (-16 -16 -24) (16 16 32)
potential team1 spawning position for ctf games
*/


/*
==================
CTFScoreboardMessage
==================
*/

void DL_TeamScoreboard (int x, int y, team_t *team, char *string, int maxlen, edict_t *ent, edict_t *killer) {
	int i, j, k, teamnum;
	int cl_score, sorted[MAX_CLIENTS], sortedscores[MAX_CLIENTS], numsorted = 0;
	int ave_ping, total_ping = 0;
	int ave_frags, total_frags = 0;
	gclient_t	*cl;
	edict_t		*cl_ent;

	char seper[18];
	memset(seper, 158, 18);
	seper[17] = 0;

	for (i = 0; i < game.maxclients; i++) {
		cl_ent = g_edicts+1 + i;
		if (!cl_ent->inuse || (game.clients[i].resp.team != team))
			continue;

		// Calc average ping
		total_ping += game.clients[i].ping;
		total_frags += game.clients[i].resp.score;
		
		// Sort player scores for this team
		cl_score = game.clients[i].resp.score;
		for (j=0; j < numsorted; j++) {
			if (cl_score > sortedscores[j]) {
				for (k = numsorted; k > j; k--)
					sorted[k] = sorted[k-1], sortedscores[k] = sortedscores[k-1];
				break;
			}
		}
		sorted[j] = i, sortedscores[j] = cl_score;
		numsorted++;
	}

	strcat(string, va("xv %d yv %d picn %s ", x, y, team->pic_b));
	strcat(string, va("xv %d yv %d string2 %s ", x+24, y, team->team_name));
	strcat(string, va("xv %d yv %d string \"%3d %4d\" ", x+24, y+8, numsorted, total_frags));

	if (numsorted) {
		ave_ping = total_ping / numsorted;
		ave_frags = total_frags / numsorted;
		strcat(string, va("xv %d yv %d string \"%3d %4d\" ", x+24, y+16, ave_ping, ave_frags));
	} else {
		strcat(string, va("xv %d yv %d string \"N/A  N/A\" ", x+24, y+16));
	}

	teamnum = STAT_TEAM1_SCORE + (team - &dlsys.teams[0]);
	ent->client->ps.stats[teamnum] = team->score;
	strcat(string, va("xv %d yv %d num 3 %d ", x+104, y, teamnum));
	strcat(string, va("xv %d yv %d string \"%c%s%c\" ", x, y+24, 157, seper, 159));

	for (i = 0; i < numsorted; i++) {
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		if ((cl_ent == ent) && (level.framenum & 16))
			strcat(string, va("xv %d yv %d string2 \"%3d %-11.11s %3d\" ", x, y+32+(i * 8), cl->ping, cl->pers.netname, cl->resp.score));
		else
			strcat(string, va("xv %d yv %d string \"%3d %-11.11s %3d\" ", x, y+32+(i * 8), cl->ping, cl->pers.netname, cl->resp.score));
	}
}

void DL_Scoreboard (edict_t *ent, edict_t *killer) {
char	string[1400];
int		i, team, maxteams = 0;
edict_t	*cl_ent;

	memset(string, 0, 1400);
	for (i=0; i<game.maxclients; i++) {
		cl_ent = g_edicts+1 + i;
		if (!cl_ent->inuse)
			continue;
		team = game.clients[i].resp.team - &dlsys.teams[0];
		if (team > maxteams)
			maxteams = team;
	}

	DL_TeamScoreboard(4, 8, &dlsys.teams[0], string, 1400, ent, killer);
	DL_TeamScoreboard(164, 8, &dlsys.teams[1], string, 1400, ent, killer);
	if (maxteams == 2)
		DL_TeamScoreboard(84, 100, &dlsys.teams[2], string, 1400, ent, killer);
	if (maxteams == 3) {
		DL_TeamScoreboard(4, 100, &dlsys.teams[2], string, 1400, ent, killer);
		DL_TeamScoreboard(164, 100, &dlsys.teams[3], string, 1400, ent, killer);
	}
	PMenu_CloseAll(ent);
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}
/*
void CTFScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[4][MAX_CLIENTS];
	int		sortedscores[4][MAX_CLIENTS];
	int		score, total[4], totalscore[4];
	int		last[4];
	int		numTeams = 0;
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

	// sort the clients by team and score
	for (i=0; i<4; i++)
		total[i] = last[i] = totalscore[i] = 0;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (game.clients[i].resp.team)
			team = game.clients[i].resp.team - &dlsys.teams[0];
		else
			continue; // unknown team?

		if (team > numTeams)
			numTeams = team;

		score = game.clients[i].resp.score;
		for (j=0 ; j<total[team] ; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}
		for (k=total[team] ; k>j ; k--)
		{
			sorted[team][k] = sorted[team][k-1];
			sortedscores[team][k] = sortedscores[team][k-1];
		}
		sorted[team][j] = i;
		sortedscores[team][j] = score;
		totalscore[team] += score;
		total[team]++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	// team one
	sprintf(string, "if 24 xv 8 yv 8 pic 24 endif "
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 98 yv 12 num 2 18 "
		"if 25 xv 168 yv 8 pic 25 endif "
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 256 yv 12 num 2 20 ",
		totalscore[0], total[0],
		totalscore[1], total[1]);
	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total[0] && i >= total[1])
			break; // we're done

#if 0 //ndef NEW_SCORE
		// set up y
		sprintf(entry, "yv %d ", 42 + i * 8);
		if (maxsize - len > strlen(entry)) {
			strcat(string, entry);
			len = strlen(string);
		}
#else
		*entry = 0;
#endif

		// left side
		if (i < total[0]) {
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];

#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
			"xv 0 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);

#else
			sprintf(entry+strlen(entry),
				"ctf 0 %d %d %d %d ",
				42 + i * 8,
				sorted[0][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

#endif

			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
				last[0] = i;
			}
		}

		// right side
		if (i < total[1]) {
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];

#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
			"xv 160 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);

//			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
//				strcat(entry, "xv 216 picn sbfctf1 ");

#else

			sprintf(entry+strlen(entry),
				"ctf 160 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

//			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
//				sprintf(entry + strlen(entry), "xv 216 yv %d picn sbfctf1 ",
//					42 + i * 8);
#endif
			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
				last[1] = i;
			}
		}
	}

	// put in spectators if we have enough room
	if (last[0] > last[1])
		j = last[0];
	else
		j = last[1];
	j = (j + 2) * 8 + 42;

	k = n = 0;
	if (maxsize - len > 50) {
		for (i = 0; i < maxclients->value; i++) {
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			if (!cl_ent->inuse ||
				cl_ent->client->resp.team)
				continue;

			if (!k) {
				k = 1;
				sprintf(entry, "xv 0 yv %d string2 \"Spectators\" ", j);
				strcat(string, entry);
				len = strlen(string);
				j += 8;
			}

			sprintf(entry+strlen(entry),
				"ctf %d %d %d %d %d ",
				(n & 1) ? 160 : 0, // x
				j, // y
				i, // playernum
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);
			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
			}
			
			if (n & 1)
				j += 8;
			n++;
		}
	}

	if (total[0] - last[0] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			42 + (last[0]+1)*8, total[0] - last[0] - 1);

	if (total[1] - last[1] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
			42 + (last[1]+1)*8, total[1] - last[1] - 1);

	PMenu_CloseAll(ent);
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}
*/

void DeathmatchScoreboard (edict_t *ent);

void CTFShowScores(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent, true);

	ent->client->showscores = true;
	ent->client->showinventory = false;
	DeathmatchScoreboard (ent);
}

qboolean CTFCheckRules(void)
{
int i;

	if (capturelimit->value) {
		for (i = 0; i < MAX_TEAMS; i++) {
			if (dlsys.teams[i].score >= capturelimit->value) {
				gi.bprintf (PRINT_HIGH, "Capturelimit hit.\n");
				return true;
			}
		}
	}
	return false;
}





void Use_Target_Resupply(edict_t *self, edict_t *other, edict_t *activator);
void SP_info_player_team1(edict_t *self) {
edict_t *trig, *targ;
static int nsmp;

	self->classname = "dl_player_start";
	self->team_mask = 1;

	// spawn some form of resupply pad at the spawn point location
	if ((red_sup < 3) && (nsmp <= 0)) {
		nsmp = 3;	// place every 3rd team-spawn point, upto 3
		red_sup++;

		trig = G_Spawn();
		trig->classname = "dl_trigger_model";
		VectorCopy(self->s.origin, trig->s.origin);
		VectorCopy(self->s.angles, trig->s.angles);
		trig->s.renderfx = RF_GLOW;
		trig->radio_msg = NULL;
		trig->model = "models/objects/dmspot/tris.md2";
		st.skinnum = 1;	// load st values for SP_dl_trigger_model;
		trig->s.effects = EF_SPHERETRANS;
		trig->s.renderfx = RF_GLOW;
		trig->message = "Loading Up!|Resupplying...|Go Get 'Em!";
		trig->wait = 1;
		trig->team_mask = 1;
		trig->target = "red_resupply";
		SP_dl_trigger_model(trig);

		if (!red_targ_sup) {
			red_targ_sup++;
			targ = G_Spawn();
			targ->classname = "dl_target_resupply";
			targ->targetname = "red_resupply";
			targ->use = Use_Target_Resupply;
			targ->delay = 0.1;
			targ->health = 10;
		}
	} else
		nsmp--;
}

/*QUAKED info_player_team2 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team2(edict_t *self) {
edict_t *trig, *targ;
static int nsmp; 

	self->classname = "dl_player_start";
	self->team_mask = 2;

	// spawn some form of resupply pad at the spawn point location
	if ((blue_sup < 3) && (nsmp <= 0)) {
		nsmp = 3;	// place every 3rd team-spawn point, upto 3
		blue_sup++;

		trig = G_Spawn();
		trig->classname = "dl_trigger_model";
		VectorCopy(self->s.origin, trig->s.origin);
		VectorCopy(self->s.angles, trig->s.angles);
		trig->radio_msg = NULL;
		trig->model = "models/objects/dmspot/tris.md2";
		st.skinnum = 1;	// load st values for SP_dl_trigger_model;
		trig->s.effects = EF_SPHERETRANS;
		trig->s.renderfx = RF_GLOW;
		trig->message = "Loading Up!|Resupplying...|Go Get 'Em!";
		trig->wait = 1;
		trig->team_mask = 2;
		trig->target = "blue_resupply";
		SP_dl_trigger_model(trig);

		if (!blue_targ_sup) {
			blue_targ_sup++;
			targ = G_Spawn();
			targ->classname = "dl_target_resupply";
			targ->targetname = "blue_resupply";
			targ->use = Use_Target_Resupply;
			targ->delay = 0.1;
			targ->health = 10;
		}
	} else
		nsmp--;
}

/*-----------------------------------------------------------------------*/
/*QUAKED misc_ctf_banner (1 .5 0) (-4 -64 0) (4 64 248) TEAM2
The origin is the bottom of the banner.
The banner is 248 tall.
*/
static void misc_ctf_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_ctf_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	if (ent->spawnflags & 1)
		ent->s.modelindex = gi.modelindex (dlsys.teams[1].banner);	// team blue
	else
		ent->s.modelindex = gi.modelindex (dlsys.teams[0].banner);	// team red

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_ctf_small_banner (1 .5 0) (-4 -32 0) (4 32 124) TEAM2
The origin is the bottom of the banner.
The banner is 124 tall.
*/
void SP_misc_ctf_small_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	if (ent->spawnflags & 1)
		ent->s.modelindex = gi.modelindex (dlsys.teams[1].sbanner);	// team blue
	else
		ent->s.modelindex = gi.modelindex (dlsys.teams[0].sbanner);	// team red

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*--------------------------------------------------------------------------
 * just here to help old map conversions
 *--------------------------------------------------------------------------*/

static void old_teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	int			i;
	vec3_t		forward;

	if (!other->client)
		return;
	dest = G_Find (NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

//ZOID
//	CTFPlayerResetGrapple(other);
//ZOID

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
//	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	other->client->ps.pmove.pm_time = 160>>3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->enemy->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i=0 ; i<3 ; i++)
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

	other->s.angles[PITCH] = 0;
	other->s.angles[YAW] = dest->s.angles[YAW];
	other->s.angles[ROLL] = 0;
	VectorCopy (dest->s.angles, other->client->ps.viewangles);
	VectorCopy (dest->s.angles, other->client->v_angle);

	// give a little forward velocity
	AngleVectors (other->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 200, other->velocity);

	// kill anything at the destination
	if (!KillBox (other))
	{
	}

	gi.linkentity (other);
}

/*QUAKED trigger_teleport (0.5 0.5 0.5) ?
Players touching this will be teleported
*/
void SP_trigger_teleport (edict_t *ent)
{
	edict_t *s;
	int i;

	if (!ent->target)
	{
		gi.dprintf ("teleporter without a target.\n");
		G_FreeEdict (ent);
		return;
	}

	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->touch = old_teleporter_touch;
	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);

	// noise maker and splash effect dude
	s = G_Spawn();
	ent->enemy = s;
	for (i = 0; i < 3; i++)
		s->s.origin[i] = ent->mins[i] + (ent->maxs[i] - ent->mins[i])/2;
	s->s.sound = gi.soundindex ("world/hum1.wav");
	gi.linkentity(s);
	
}

/*QUAKED info_teleport_destination (0.5 0.5 0.5) (-16 -16 -24) (16 16 32)
Point trigger_teleports at these.
*/
void SP_info_teleport_destination (edict_t *ent)
{
	ent->s.origin[2] += 16;
}

