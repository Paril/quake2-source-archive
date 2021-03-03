#include "g_local.h"
//#include "g_teamdm.h"

typedef struct teamdmgame_s
{
	int team1, team2;
	int total1, total2; // these are only set when going into intermission!
} teamdmgame_t;

teamdmgame_t teamdmgame;
qboolean techspawn = false;

cvar_t *teamdm;
cvar_t *teamdm_forcejoin;

char *teamdm_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
  "xv 246 "
  "num 2 10 "
  "xv 296 "
  "pic 9 "
"endif "

//  help / weapon icon 
"if 11 "
  "xv 148 "
  "pic 11 "
"endif "


//  frags
"xr	-50 "
"yt 2 "
"num 3 14 "


//tech
"yb -129 "
//"if 26 "
"if 28 "
  "xr -26 "
//  "pic 26 "
  "pic 28 "
"endif "

// red team
"yb -102 "
//"if 17 "
"if 19 "
  "xr -26 "
//  "pic 17 "
  "pic 19 "
"endif "
"xr -62 "
//"num 2 18 "
"num 2 20 "
//joined overlay
//"if 22 "
"if 24 "
  "yb -104 "
  "xr -28 "
//  "pic 22 "
  "pic 24 "
"endif "

// blue team
"yb -75 "
//"if 19 "
"if 21 "
  "xr -26 "
//  "pic 19 "
  "pic 21 "
"endif "
"xr -62 "
//"num 2 20 "
"num 2 22 "
//"if 23 "
"if 25 "
  "yb -77 "
  "xr -28 "
//  "pic 23 "
  "pic 25 "
"endif "
/*
// have flag graph
//"if 21 "
"if 23 "
  "yt 26 "
  "xr -24 "
//  "pic 21 "
  "pic 23 "
"endif "
*/
// id view state
//"if 27 "
"if 29 "
  "xv 0 "
  "yb -58 "
  "string \"Viewing\" "
  "xv 64 "
//  "stat_string 27 "
  "stat_string 29 "
"endif "

// +ION
// clipammo
//"yb -50"
"if 18 "
"	xv	70 "
"	yb  -50"
"	num 99 18"
"endif "
// -ION

;

static char *tnames[] = {
	"item_tech1", "item_tech2", "item_tech3", "item_tech4",
	NULL
};

void stuffcmd(edict_t *ent, char *s) 	
{
   	gi.WriteByte (11);	        
	gi.WriteString (s);
    gi.unicast (ent, true);	
}

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

void TEAMDMInit(void)
{
	teamdm = gi.cvar("teamdm", "0", CVAR_SERVERINFO|CVAR_LATCH);
// ion
//	teamdm = gi.cvar("teamdm", "0", CVAR_NOSET);
// ion
	teamdm_forcejoin = gi.cvar("teamdm_forcejoin", "", 0);

	memset(&teamdmgame, 0, sizeof(teamdmgame));
	techspawn = false;
}

/*--------------------------------------------------------------------------*/

char *TEAMDMTeamName(int team)
{
	switch (team) {
	case TEAMDM_TEAM1:
//		return "RED";
		return "HENCHMEN";
	case TEAMDM_TEAM2:
//		return "BLUE";
		return "AGENTS";
	}
	return "UNKNOWN";
}

char *TEAMDMOtherTeamName(int team)
{
	switch (team) {
	case TEAMDM_TEAM1:
//		return "BLUE";
		return "AGENTS";
	case TEAMDM_TEAM2:
//		return "RED";
		return "HENCHMEN";
	}
	return "UNKNOWN";
}

int TEAMDMOtherTeam(int team)
{
	switch (team) {
	case TEAMDM_TEAM1:
		return TEAMDM_TEAM2;
	case TEAMDM_TEAM2:
		return TEAMDM_TEAM1;
	}
	return -1; // invalid value
}

/*--------------------------------------------------------------------------*/

edict_t *SelectRandomDeathmatchSpawnPoint (void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
float	PlayersRangeFromSpot (edict_t *spot);

void TEAMDMAssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];

	Com_sprintf(t, sizeof(t), "%s", s);

	if ((p = strrchr(t, '/')) != NULL)
		p[1] = 0;
	else
		strcpy(t, "male/");

	switch (ent->client->resp.teamdm_team) {
	case TEAMDM_TEAM1:
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
			ent->client->pers.netname, t, TEAMDM_TEAM1_SKIN) );
		break;
	case TEAMDM_TEAM2:
		gi.configstring (CS_PLAYERSKINS+playernum,
			va("%s\\%s%s", ent->client->pers.netname, t, TEAMDM_TEAM2_SKIN) );
		break;
	default:
		gi.configstring (CS_PLAYERSKINS+playernum, 
			va("%s\\%s", ent->client->pers.netname, s) );
		break;
	}
//	gi.cprintf(ent, PRINT_HIGH, "You have been assigned to %s team.\n", ent->client->pers.netname);
}

void TEAMDMAssignTeam(gclient_t *who)
{
	edict_t		*player;
	int i;
	int team1count = 0, team2count = 0;

	who->resp.teamdm_state = TEAMDM_STATE_START;

	if (!((int)dmflags->value & DF_TEAMDM_FORCEJOIN)) {
		who->resp.teamdm_team = TEAMDM_NOTEAM;
		return;
	}

	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];

		if (!player->inuse || player->client == who)
			continue;

		switch (player->client->resp.teamdm_team) {
		case TEAMDM_TEAM1:
			team1count++;
			break;
		case TEAMDM_TEAM2:
			team2count++;
		}
	}
	if (team1count < team1count)
		who->resp.teamdm_team = TEAMDM_TEAM1;
	else if (team2count < team1count)
		who->resp.teamdm_team = TEAMDM_TEAM2;
	else if (rand() & 1)
		who->resp.teamdm_team = TEAMDM_TEAM1;
	else
		who->resp.teamdm_team = TEAMDM_TEAM2;
}


void TEAMDMAssignBotTeam(gclient_t *who)
{
	edict_t		*player;
	int i;
	int team1count = 0, team2count = 0;

	who->resp.teamdm_state = TEAMDM_STATE_START;

	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];

		if (!player->inuse || player->client == who)
			continue;

		switch (player->client->resp.teamdm_team) {
		case TEAMDM_TEAM1:
			team1count++;
			break;
		case TEAMDM_TEAM2:
			team2count++;
		}
	}
	if (team1count < team1count)
		who->resp.teamdm_team = TEAMDM_TEAM1;
	else if (team2count < team1count)
		who->resp.teamdm_team = TEAMDM_TEAM2;
	else if (rand() & 1)
		who->resp.teamdm_team = TEAMDM_TEAM1;
	else
		who->resp.teamdm_team = TEAMDM_TEAM2;
}
/*
================
SelectTEAMDMSpawnPoint

go to a teamdm point, but NOT the two points closest
to other players
================
*/
edict_t *SelectTEAMDMSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	*cname;

//if (ent->isabot) return SelectRandomDeathmatchSpawnPoint();

	if (ent->client->resp.teamdm_state != TEAMDM_STATE_START)
		if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
			return SelectFarthestDeathmatchSpawnPoint ();
//		else if (ent->isabot
		else
			return SelectRandomDeathmatchSpawnPoint ();

	ent->client->resp.teamdm_state = TEAMDM_STATE_PLAYING;

	switch (ent->client->resp.teamdm_team) {
	case TEAMDM_TEAM1:
		cname = "info_player_team1";
		break;
	case TEAMDM_TEAM2:
		cname = "info_player_team2";
		break;
	default:
		return SelectRandomDeathmatchSpawnPoint();
	}

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), cname)) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
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
	}

	if (!count)
		return SelectRandomDeathmatchSpawnPoint();

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), cname);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;


}

/*------------------------------------------------------------------------*/

// called when we enter the intermission
void TEAMDMCalcScores(void)
{
	int i;

	teamdmgame.total1 = teamdmgame.total2 = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.teamdm_team == TEAMDM_TEAM1)
			teamdmgame.total1 += game.clients[i].resp.score;
		else if (game.clients[i].resp.teamdm_team == TEAMDM_TEAM2)
			teamdmgame.total2 += game.clients[i].resp.score;
	}
}

void TEAMDMID_f (edict_t *ent)
{
	if (ent->client->resp.id_state) {
		gi.cprintf(ent, PRINT_HIGH, "Disabling player identication display.\n");
		ent->client->resp.id_state = false;
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Activating player identication display.\n");
		ent->client->resp.id_state = true;
	}
}

static void TEAMDMSetIDView(edict_t *ent)
{
	vec3_t	forward, dir;
	trace_t	tr;
	edict_t	*who, *best;
	float	bd = 0, d;
	int i;

	ent->client->ps.stats[STAT_TEAMDM_ID_VIEW] = 0;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 1024, forward);
	VectorAdd(ent->s.origin, forward, forward);
	tr = gi.trace(ent->s.origin, NULL, NULL, forward, ent, MASK_SOLID);
	if (tr.fraction < 1 && tr.ent && tr.ent->client) {
		ent->client->ps.stats[STAT_TEAMDM_ID_VIEW] = 
			CS_PLAYERSKINS + (ent - g_edicts - 1);
		return;
	}

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	best = NULL;
	for (i = 1; i <= maxclients->value; i++) {
		who = g_edicts + i;
		if (!who->inuse)
			continue;
		VectorSubtract(who->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd && loc_CanSee(ent, who)) {
			bd = d;
			best = who;
		}
	}
	if (bd > 0.90)
		ent->client->ps.stats[STAT_TEAMDM_ID_VIEW] = 
			CS_PLAYERSKINS + (best - g_edicts - 1);
}

void SetTEAMDMStats(edict_t *ent)
{
	gitem_t *tech;
	int i;
	int p1, p2;
	edict_t *e;

	// logo headers for the frag display
	ent->client->ps.stats[STAT_TEAMDM_TEAM1_HEADER] = gi.imageindex ("teamdmsb1");
	ent->client->ps.stats[STAT_TEAMDM_TEAM2_HEADER] = gi.imageindex ("teamdmsb2");
// ion
	// if during intermission, we must blink the team header of the winning team
	if (level.intermissiontime && (level.framenum & 8)) { // blink 1/8th second
		// note that teamdmgame.total[12] is set when we go to intermission
		if (teamdmgame.team1 > teamdmgame.team2)
			ent->client->ps.stats[STAT_TEAMDM_TEAM1_HEADER] = 0;
		else if (teamdmgame.team2 > teamdmgame.team1)
			ent->client->ps.stats[STAT_TEAMDM_TEAM2_HEADER] = 0;
		else if (teamdmgame.total1 > teamdmgame.total2) // frag tie breaker
			ent->client->ps.stats[STAT_TEAMDM_TEAM1_HEADER] = 0;
		else if (teamdmgame.total2 > teamdmgame.total1) 
			ent->client->ps.stats[STAT_TEAMDM_TEAM2_HEADER] = 0;
		else { // tie game!
			ent->client->ps.stats[STAT_TEAMDM_TEAM1_HEADER] = 0;
			ent->client->ps.stats[STAT_TEAMDM_TEAM2_HEADER] = 0;
		}
	}

/*	// tech icon
	i = 0;
	ent->client->ps.stats[STAT_TEAMDM_TECH] = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
			ent->client->ps.stats[STAT_TEAMDM_TECH] = gi.imageindex(tech->icon);
			break;
		}
		i++;
	}*/

	// figure out what icon to display for team logos
	// three states:
	//   flag at base
	//   flag taken
	//   flag dropped
	p1 = gi.imageindex ("i_teamdm1");
/*	e = G_Find(NULL, FOFS(classname), "item_flag_team1");
	if (e != NULL) {
		if (e->solid == SOLID_NOT) {
			int i;

			// not at base
			// check if on player
			p1 = gi.imageindex ("i_teamdm1d"); // default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag1_item)]) {
					// enemy has it
					p1 = gi.imageindex ("i_teamdm1t");
					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM)
			p1 = gi.imageindex ("i_teamdm1d"); // must be dropped
	}*/
	p2 = gi.imageindex ("i_teamdm2");
/*	e = G_Find(NULL, FOFS(classname), "item_flag_team2");
	if (e != NULL) {
		if (e->solid == SOLID_NOT) {
			int i;

			// not at base
			// check if on player
			p2 = gi.imageindex ("i_teamdm2d"); // default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag2_item)]) {
					// enemy has it
					p2 = gi.imageindex ("i_teamdm2t");
					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM)
			p2 = gi.imageindex ("i_teamdm2d"); // must be dropped
	}*/


	ent->client->ps.stats[STAT_TEAMDM_TEAM1_PIC] = p1;
	ent->client->ps.stats[STAT_TEAMDM_TEAM2_PIC] = p2;

/*	if (teamdmgame.last_flag_capture && level.time - teamdmgame.last_flag_capture < 5) {
		if (teamdmgame.last_capture_team == TEAMDM_TEAM1)
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_TEAMDM_TEAM1_PIC] = p1;
			else
				ent->client->ps.stats[STAT_TEAMDM_TEAM1_PIC] = 0;
		else
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_TEAMDM_TEAM2_PIC] = p2;
			else
				ent->client->ps.stats[STAT_TEAMDM_TEAM2_PIC] = 0;
	}*/

	ent->client->ps.stats[STAT_TEAMDM_TEAM1_CAPS] = teamdmgame.team1;
	ent->client->ps.stats[STAT_TEAMDM_TEAM2_CAPS] = teamdmgame.team2;

//	ent->client->ps.stats[STAT_TEAMDM_FLAG_PIC] = 0;
/*	if (ent->client->resp.teamdm_team == TEAMDM_TEAM1 &&
		ent->client->pers.inventory[ITEM_INDEX(flag2_item)] &&
		(level.framenum & 8))
		ent->client->ps.stats[STAT_TEAMDM_FLAG_PIC] = gi.imageindex ("i_teamdm2");

	else if (ent->client->resp.teamdm_team == TEAMDM_TEAM2 &&
		ent->client->pers.inventory[ITEM_INDEX(flag1_item)] &&
		(level.framenum & 8))
		ent->client->ps.stats[STAT_TEAMDM_FLAG_PIC] = gi.imageindex ("i_teamdm1");
*/
	ent->client->ps.stats[STAT_TEAMDM_JOINED_TEAM1_PIC] = 0;
	ent->client->ps.stats[STAT_TEAMDM_JOINED_TEAM2_PIC] = 0;
	if (ent->client->resp.teamdm_team == TEAMDM_TEAM1)
		ent->client->ps.stats[STAT_TEAMDM_JOINED_TEAM1_PIC] = gi.imageindex ("i_teamdmj");
	else if (ent->client->resp.teamdm_team == TEAMDM_TEAM2)
		ent->client->ps.stats[STAT_TEAMDM_JOINED_TEAM2_PIC] = gi.imageindex ("i_teamdmj");
//*/
	ent->client->ps.stats[STAT_TEAMDM_ID_VIEW] = 0;
	if (ent->client->resp.id_state)
		TEAMDMSetIDView(ent);
}

/*------------------------------------------------------------------------*/

/*QUAKED info_player_team1 (1 0 0) (-16 -16 -24) (16 16 32)
potential team1 spawning position for teamdm games
*/
void SP_info_player_team1(edict_t *self)
{
}

/*QUAKED info_player_team2 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for teamdm games
*/
void SP_info_player_team2(edict_t *self)
{
}

void TEAMDMTeam_f (edict_t *ent)
{
	char *t, *s;
	int desired_team;

	t = gi.args();
	if (!*t) {
		gi.cprintf(ent, PRINT_HIGH, "You are on the %s team.\n",
			TEAMDMTeamName(ent->client->resp.teamdm_team));
		return;
	}
//	if (Q_stricmp(t, "red") == 0)
	if (Q_stricmp(t, "henchmen") == 0)
		desired_team = TEAMDM_TEAM1;
//	else if (Q_stricmp(t, "blue") == 0)
	else if (Q_stricmp(t, "agents") == 0)
		desired_team = TEAMDM_TEAM2;
	else {
		gi.cprintf(ent, PRINT_HIGH, "Unknown team %s.\n", t);
		return;
	}

	if (ent->client->resp.teamdm_team == desired_team) {
		gi.cprintf(ent, PRINT_HIGH, "You are already on the %s team.\n",
			TEAMDMTeamName(ent->client->resp.teamdm_team));
		return;
	}

////
	ent->svflags = 0;
	ent->flags &= ~FL_GODMODE;
	ent->client->resp.teamdm_team = desired_team;
	ent->client->resp.teamdm_state = TEAMDM_STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	TEAMDMAssignSkin(ent, s);
	

//	if (ent->solid == SOLID_NOT) { // spectator
  if (ent->client->resp.spectator)
  {

		PutClientInServer (ent);
		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		gi.bprintf(PRINT_HIGH, "%s joined the %s team.\n",
			ent->client->pers.netname, TEAMDMTeamName(desired_team));
		return;
	}

	ent->health = 0;
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;
	respawn (ent);

	ent->client->resp.score = 0;

	gi.bprintf(PRINT_HIGH, "%s changed to the %s team.\n",
		ent->client->pers.netname, TEAMDMTeamName(desired_team));
}

/*
==================
TEAMDMScoreboardMessage
==================
*/
void TEAMDMScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[2][MAX_CLIENTS];
	int		sortedscores[2][MAX_CLIENTS];
	int		score, total[2], totalscore[2];
	int		last[2];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (game.clients[i].resp.teamdm_team == TEAMDM_TEAM1)
			team = 0;
		else if (game.clients[i].resp.teamdm_team == TEAMDM_TEAM2)
			team = 1;
		else
			continue; // unknown team?

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
/*	sprintf(string, "if 24 xv 8 yv 8 pic 24 endif "
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 98 yv 12 num 2 18 "
		"if 25 xv 168 yv 8 pic 25 endif "
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 256 yv 12 num 2 20 ",*/
	sprintf(string, "if 26 xv 8 yv 8 pic 26 endif "
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 98 yv 12 num 2 20 "
		"if 27 xv 168 yv 8 pic 27 endif "
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 256 yv 12 num 2 22 ",
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

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
				strcat(entry, "xv 56 picn sbfteamdm2 ");
#else
			sprintf(entry+strlen(entry),
				"teamdm 0 %d %d %d %d ",
				42 + i * 8,
				sorted[0][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

/*			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
				sprintf(entry + strlen(entry), "xv 56 yv %d picn sbfteamdm2 ",
					42 + i * 8);*/
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

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
				strcat(entry, "xv 216 picn sbfteamdm1 ");

#else

			sprintf(entry+strlen(entry),
				"teamdm 160 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

/*			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
				sprintf(entry + strlen(entry), "xv 216 yv %d picn sbfteamdm1 ",
					42 + i * 8);*/
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
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->resp.teamdm_team != TEAMDM_NOTEAM)
				continue;

			if (!k) {
				k = 1;
				sprintf(entry, "xv 0 yv %d string2 \"Spectators\" ", j);
				strcat(string, entry);
				len = strlen(string);
				j += 8;
			}

			sprintf(entry+strlen(entry),
				"teamdm %d %d %d %d %d ",
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

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/*
======================================================================

SAY_TEAM

======================================================================
*/

// This array is in 'importance order', it indicates what items are
// more important when reporting their names.
struct {
	char *classname;
	int priority;
} loc_names[] = 
{
	{	"item_flag_team1",			1 },
	{	"item_flag_team2",			1 },
	{	"item_quad",				2 }, 
	{	"item_invulnerability",		2 },
	{	"weapon_bfg",				3 },
	{	"weapon_railgun",			4 },
	{	"weapon_rocketlauncher",	4 },
	{	"weapon_hyperblaster",		4 },
	{	"weapon_chaingun",			4 },
	{	"weapon_grenadelauncher",	4 },
	{	"weapon_machinegun",		4 },
	{	"weapon_supershotgun",		4 },
	{	"weapon_shotgun",			4 },
	{	"item_power_screen",		5 },
	{	"item_power_shield",		5 },
	{	"item_armor_body",			6 },
	{	"item_armor_combat",		6 },
	{	"item_armor_jacket",		6 },
	{	"item_silencer",			7 },
	{	"item_breather",			7 },
	{	"item_enviro",				7 },
	{	"item_adrenaline",			7 },
	{	"item_bandolier",			8 },
	{	"item_pack",				8 },
	{ NULL, 0 }
};


static void TEAMDMSay_Team_Location(edict_t *who, char *buf)
{
	edict_t *what = NULL;
	edict_t *hot = NULL;
	float hotdist = 999999, newdist;
	vec3_t v;
	int hotindex = 999;
	int i;
	gitem_t *item;
	int nearteam = -1;
	edict_t *flag1, *flag2;
	qboolean hotsee = false;
	qboolean cansee;

	while ((what = loc_findradius(what, who->s.origin, 1024)) != NULL) {
		// find what in loc_classnames
		for (i = 0; loc_names[i].classname; i++)
			if (strcmp(what->classname, loc_names[i].classname) == 0)
				break;
		if (!loc_names[i].classname)
			continue;
		// something we can see get priority over something we can't
		cansee = loc_CanSee(what, who);
		if (cansee && !hotsee) {
			hotsee = true;
			hotindex = loc_names[i].priority;
			hot = what;
			VectorSubtract(what->s.origin, who->s.origin, v);
			hotdist = VectorLength(v);
			continue;
		}
		// if we can't see this, but we have something we can see, skip it
		if (hotsee && !cansee)
			continue;
		if (hotsee && hotindex < loc_names[i].priority)
			continue;
		VectorSubtract(what->s.origin, who->s.origin, v);
		newdist = VectorLength(v);
		if (newdist < hotdist || 
			(cansee && loc_names[i].priority < hotindex)) {
			hot = what;
			hotdist = newdist;
			hotindex = i;
			hotsee = loc_CanSee(hot, who);
		}
	}

	if (!hot) {
		strcpy(buf, "nowhere");
		return;
	}

	// we now have the closest item
	// see if there's more than one in the map, if so
	// we need to determine what team is closest
	what = NULL;
	while ((what = G_Find(what, FOFS(classname), hot->classname)) != NULL) {
		if (what == hot)
			continue;
		// if we are here, there is more than one, find out if hot
		// is closer to red flag or blue flag
		if ((flag1 = G_Find(NULL, FOFS(classname), "item_flag_team1")) != NULL &&
			(flag2 = G_Find(NULL, FOFS(classname), "item_flag_team2")) != NULL) {
			VectorSubtract(hot->s.origin, flag1->s.origin, v);
			hotdist = VectorLength(v);
			VectorSubtract(hot->s.origin, flag2->s.origin, v);
			newdist = VectorLength(v);
			if (hotdist < newdist)
				nearteam = TEAMDM_TEAM1;
			else if (hotdist > newdist)
				nearteam = TEAMDM_TEAM2;
		}
		break;
	}

	if ((item = FindItemByClassname(hot->classname)) == NULL) {
		strcpy(buf, "nowhere");
		return;
	}

	// in water?
	if (who->waterlevel)
		strcpy(buf, "in the water ");
	else
		*buf = 0;

	// near or above
	VectorSubtract(who->s.origin, hot->s.origin, v);
	if (fabs(v[2]) > fabs(v[0]) && fabs(v[2]) > fabs(v[1]))
		if (v[2] > 0)
			strcat(buf, "above ");
		else
			strcat(buf, "below ");
	else
		strcat(buf, "near ");

	if (nearteam == TEAMDM_TEAM1)
//		strcat(buf, "the red ");
		strcat(buf, "the Henchmen ");
	else if (nearteam == TEAMDM_TEAM2)
//		strcat(buf, "the blue ");
		strcat(buf, "the Agents ");
	else
		strcat(buf, "the ");

	strcat(buf, item->pickup_name);
}

static void TEAMDMSay_Team_Armor(edict_t *who, char *buf)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;

	*buf = 0;

	power_armor_type = PowerArmorType (who);
	if (power_armor_type)
	{
		cells = who->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (cells)
			sprintf(buf+strlen(buf), "%s with %i cells ",
				(power_armor_type == POWER_ARMOR_SCREEN) ?
				"Power Screen" : "Power Shield", cells);
	}

	index = ArmorIndex (who);
	if (index)
	{
		item = GetItemByIndex (index);
		if (item) {
			if (*buf)
				strcat(buf, "and ");
			sprintf(buf+strlen(buf), "%i units of %s",
				who->client->pers.inventory[index], item->pickup_name);
		}
	}

	if (!*buf)
		strcpy(buf, "no armor");
}

static void TEAMDMSay_Team_Health(edict_t *who, char *buf)
{
	if (who->health <= 0)
		strcpy(buf, "dead");
	else
		sprintf(buf, "%i health", who->health);
}

static void TEAMDMSay_Team_Tech(edict_t *who, char *buf)
{
	gitem_t *tech;
	int i;

	// see if the player has a tech powerup
	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			who->client->pers.inventory[ITEM_INDEX(tech)]) {
			sprintf(buf, "the %s", tech->pickup_name);
			return;
		}
		i++;
	}
	strcpy(buf, "no powerup");
}

static void TEAMDMSay_Team_Weapon(edict_t *who, char *buf)
{
	if (who->client->pers.weapon)
		strcpy(buf, who->client->pers.weapon->pickup_name);
	else
		strcpy(buf, "none");
}

static void TEAMDMSay_Team_Sight(edict_t *who, char *buf)
{
	int i;
	edict_t *targ;
	int n = 0;
	char s[1024];
	char s2[1024];

	*s = *s2 = 0;
	for (i = 1; i <= maxclients->value; i++) {
		targ = g_edicts + i;
		if (!targ->inuse || 
			targ == who ||
			!loc_CanSee(targ, who))
			continue;
		if (*s2) {
			if (strlen(s) + strlen(s2) + 3 < sizeof(s)) {
				if (n)
					strcat(s, ", ");
				strcat(s, s2);
				*s2 = 0;
			}
			n++;
		}
		strcpy(s2, targ->client->pers.netname);
	}
	if (*s2) {
		if (strlen(s) + strlen(s2) + 6 < sizeof(s)) {
			if (n)
				strcat(s, " and ");
			strcat(s, s2);
		}
		strcpy(buf, s);
	} else
		strcpy(buf, "no one");
}

void TEAMDMSay_Team(edict_t *who, char *msg)
{
	char outmsg[1024];
	char buf[1024];
	int i;
	char *p;
	edict_t *cl_ent;

	outmsg[0] = 0;

	if (*msg == '\"') {
		msg[strlen(msg) - 1] = 0;
		msg++;
	}

	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 1; msg++) {
		if (*msg == '%') {
			switch (*++msg) {
				case 'l' :
				case 'L' :
					TEAMDMSay_Team_Location(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'a' :
				case 'A' :
					TEAMDMSay_Team_Armor(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'h' :
				case 'H' :
					TEAMDMSay_Team_Health(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 't' :
				case 'T' :
					TEAMDMSay_Team_Tech(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'w' :
				case 'W' :
					TEAMDMSay_Team_Weapon(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;

				case 'n' :
				case 'N' :
					TEAMDMSay_Team_Sight(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;

				default :
					*p++ = *msg;
			}
		} else
			*p++ = *msg;
	}
	*p = 0;

	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.teamdm_team == who->client->resp.teamdm_team)
			gi.cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n", 
				who->client->pers.netname, outmsg);
	}
}

/*-----------------------------------------------------------------------*/
/*QUAKED misc_teamdm_banner (1 .5 0) (-4 -64 0) (4 64 248) TEAM2
The origin is the bottom of the banner.
The banner is 248 tall.
*/
static void misc_teamdm_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_teamdm_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/teamdm/banner/tris.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_teamdm_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_teamdm_small_banner (1 .5 0) (-4 -32 0) (4 32 124) TEAM2
The origin is the bottom of the banner.
The banner is 124 tall.
*/
void SP_misc_teamdm_small_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/teamdm/banner/small.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_teamdm_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}


/*-----------------------------------------------------------------------*/

void TEAMDMJoinTeam(edict_t *ent, int desired_team)
{
	char *s;

// debug
//gi.dprintf("It got as far as TEAMDMJoinTeam.\n");
// debug

	PMenu_Close(ent);

	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.teamdm_team = desired_team;
	ent->client->resp.teamdm_state = TEAMDM_STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	TEAMDMAssignSkin(ent, s);
//gi.dprintf("It got as far as TEAMDMJoinTeam Assign Skin.\n");

	PutClientInServer (ent);
//gi.dprintf("It got as far as TEAMDMJoinTeam Putinsever.\n");

	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
	gi.bprintf(PRINT_HIGH, "%s joined the %s team.\n",
		ent->client->pers.netname, TEAMDMTeamName(desired_team));
}

void TEAMDMJoinTeam1(edict_t *ent, pmenu_t *p)
{
//gi.dprintf("It got as far as TEAMDMJoinTeam1.\n");

	TEAMDMJoinTeam(ent, TEAMDM_TEAM1);
}

void TEAMDMJoinTeam2(edict_t *ent, pmenu_t *p)
{
//gi.dprintf("It got as far as TEAMDMJoinTeam2.\n");

	TEAMDMJoinTeam(ent, TEAMDM_TEAM2);
}

void TEAMDMChaseCam(edict_t *ent, pmenu_t *p)
{


	int i;
	edict_t *e;

//gi.dprintf("It got as far as TEAMDMChaseCam.\n");

//	if (ent->client->chase_target) {
//		ent->client->chase_target = NULL;
//		PMenu_Close(ent);
//		return;
//	}
		PMenu_Close(ent);

	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		if (e->inuse && e->solid != SOLID_NOT) {
			ent->client->chase_target = e;
			PMenu_Close(ent);
			ent->client->update_chase = true;
			break;
		}
	}
}

void TEAMDMReturnToMain(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	TEAMDMOpenJoinMenu(ent);
}

void TEAMDMCredits(edict_t *ent, pmenu_t *p);

void DeathmatchScoreboard (edict_t *ent);

void TEAMDMShowScores(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	ent->client->showscores = true;
	ent->client->showinventory = false;
	DeathmatchScoreboard (ent);
}

pmenu_t creditsmenu[] = {
	{ "*007: Time Goes Backwards",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*007: TGB Team Deathmatch",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Programming",					PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "IoN_PuLse",				PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, NULL, TEAMDMReturnToMain }
};


pmenu_t joinmenu[] = {
	{ "*007: Time Goes Backwards",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*007: TGB Team Deathmatch",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Join Henchmen Team",		PMENU_ALIGN_LEFT, NULL, TEAMDMJoinTeam1 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Join Agent Team",		PMENU_ALIGN_LEFT, NULL, TEAMDMJoinTeam2 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Chase Camera",		PMENU_ALIGN_LEFT, NULL, TEAMDMChaseCam },
	{ "Credits",			PMENU_ALIGN_LEFT, NULL, TEAMDMCredits },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "v" TEAMDM_STRING_VERSION,	PMENU_ALIGN_RIGHT, NULL, NULL },
};

int TEAMDMUpdateJoinMenu(edict_t *ent)
{
	static char levelname[32];
	static char team1players[32];
	static char team2players[32];
	int num1, num2, i;

	joinmenu[4].text = "Join Henchmen Team";
	joinmenu[4].SelectFunc = TEAMDMJoinTeam1;
	joinmenu[6].text = "Join Agents Team";
	joinmenu[6].SelectFunc = TEAMDMJoinTeam2;

	if (teamdm_forcejoin->string && *teamdm_forcejoin->string) {
//		if (stricmp(teamdm_forcejoin->string, "red") == 0) {
		if (stricmp(teamdm_forcejoin->string, "henchmen") == 0) {
			joinmenu[6].text = NULL;
			joinmenu[6].SelectFunc = NULL;
//		} else if (stricmp(teamdm_forcejoin->string, "blue") == 0) {
		} else if (stricmp(teamdm_forcejoin->string, "agents") == 0) {
			joinmenu[4].text = NULL;
			joinmenu[4].SelectFunc = NULL;
		}
	}

	if (ent->client->chase_target)
		joinmenu[8].text = "Leave Chase Camera";
	else
		joinmenu[8].text = "Chase Camera";

	levelname[0] = '*';
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	levelname[sizeof(levelname) - 1] = 0;

	num1 = num2 = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.teamdm_team == TEAMDM_TEAM1)
			num1++;
		else if (game.clients[i].resp.teamdm_team == TEAMDM_TEAM2)
			num2++;
	}

	sprintf(team1players, "  (%d players)", num1);
	sprintf(team2players, "  (%d players)", num2);

	joinmenu[2].text = levelname;
	if (joinmenu[4].text)
		joinmenu[5].text = team1players;
	else
		joinmenu[5].text = NULL;
	if (joinmenu[6].text)
		joinmenu[7].text = team2players;
	else
		joinmenu[7].text = NULL;
	
	if (num1 > num2)
		return TEAMDM_TEAM1;
	else if (num2 > num1)
		return TEAMDM_TEAM1;
	return (rand() & 1) ? TEAMDM_TEAM1 : TEAMDM_TEAM2;
}

void TEAMDMOpenJoinMenu(edict_t *ent)
{
	int team;

	team = TEAMDMUpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 8;
	else if (team == TEAMDM_TEAM1)
		team = 4;
	else
		team = 6;
	PMenu_Open(ent, joinmenu, team, sizeof(joinmenu) / sizeof(pmenu_t));
}

void TEAMDMCredits(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu, -1, sizeof(creditsmenu) / sizeof(pmenu_t));
}

qboolean TEAMDMStartClient(edict_t *ent)
{
	if (ent->client->resp.teamdm_team != TEAMDM_NOTEAM)
		return false;

	if (ent->isabot)
	{
		return true;
	}
//	if (!((int)dmflags->value & DF_TEAMDM_FORCEJOIN)) {
	if (!((int)dmflags->value & DF_TEAMDM_FORCEJOIN)) {
		// start as 'observer'
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.teamdm_team = TEAMDM_NOTEAM;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);

		TEAMDMOpenJoinMenu(ent);
		return true;
	}
	return false;
}

qboolean TEAMDMCheckRules(void)
{
	if (capturelimit->value && 
		(teamdmgame.team1 >= capturelimit->value ||
		teamdmgame.team2 >= capturelimit->value)) {
		gi.bprintf (PRINT_HIGH, "Capturelimit hit.\n");
		return true;
	}
	return false;
}
