#include "g_local.h"
#include "teamplay.h"

char *team_nameforteam[] = {NULL, "RED", "BLUE"};

// returns true if a and b are on the same team
qboolean OnSameTeam(edict_t *a, edict_t *b)
{
	if (!a->client || !b->client)
		return false;
	return (a->client->resp.ctf_team == b->client->resp.ctf_team);
}


#if 0
// returns true if the skin needs to be (re)set
qboolean CheckTeams(edict_t *ent, char *s) {
	int	redplayers = 0, blueplayers = 0, redplayersscore = 0, blueplayersscore = 0;
//	edict_t	*from;

	if (ent->client->pers.oldskin[0]) {				// they once had a skin
		if (strcmp(ent->client->pers.oldskin, s) == 0 /*&& IsValidSkin(ent, s)*/) {	// they didn't change their skin or their ctfskins var
//gi.bprintf(PRINT_HIGH, "%s passive change\n", ent->client->pers.netname);
			return (false);
		} else {										// they did change their skin or their ctfskins var
//gi.bprintf(PRINT_HIGH, "%s desires a change to %s\n", ent->client->pers.netname, s);
			if (ent->client->pers.oldskin[0] != s[0]) {	// they switched to the other team
				if (s[0] == 'm')						// want to switch to red team
					strcpy(ent->client->pers.oldskin, SKIN_RED(ent));
				else									// want to switch to blue team
					strcpy(ent->client->pers.oldskin, SKIN_BLUE(ent));
				T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TEAM_SWITCH);
				ent->client->resp.score = 0;			// reset score
//gi.bprintf(PRINT_HIGH, "%s switched to the %s team\n", ent->client->pers.netname, (IS_RED(ent)?"RED":"BLUE"));
			} else {									// they switched to another skin on the same team or changed their ctfskins var
				if (s[0] == 'm')
					strcpy(ent->client->pers.oldskin, SKIN_RED(ent));
				else
					strcpy(ent->client->pers.oldskin, SKIN_BLUE(ent));
			}
			return (true);
		}
	} else {									// give them an initial team

		TeamScores(&redplayersscore, &blueplayersscore);
		TeamPlayers(&redplayers, &blueplayers);

/*		for (from = g_edicts; from < &g_edicts[globals.num_edicts]; from++)
		{
			if (!from->inuse)
				continue;
			if (!from->client)
				continue;
			if (from->client->pers.oldskin[0] == 'm') {
				redplayers++;
				redplayersscore += from->client->resp.score;
			} else if (from->client->pers.oldskin[0] == 'f') {
				blueplayers++;
				blueplayersscore += from->client->resp.score;
			}
		}
*/

		if (redplayers < blueplayers)
			strcpy(ent->client->pers.oldskin, SKIN_RED(ent));
		else if (blueplayers < redplayers)
			strcpy(ent->client->pers.oldskin, SKIN_BLUE(ent));
		else if (redplayersscore < blueplayersscore)
			strcpy(ent->client->pers.oldskin, SKIN_RED(ent));
		else if (blueplayersscore < redplayersscore)
			strcpy(ent->client->pers.oldskin, SKIN_BLUE(ent));
		else if (random() < 0.5)
			strcpy(ent->client->pers.oldskin, SKIN_RED(ent));
		else
			strcpy(ent->client->pers.oldskin, SKIN_BLUE(ent));

gi.bprintf(PRINT_HIGH, "%s joins the %s team\n", ent->client->pers.netname, (IS_RED(ent)?"RED":"BLUE"));

		return (true);
	}

}
#endif


// gets team score info.  any parameters can be NULL
void team_scores(int *team1score, int *team2score, int *team1players, int *team2players)
{
		edict_t		*from = NULL;

		if (team1score) (*team1score) = 0;
		if (team2score) (*team2score) = 0;
		if (team1players) (*team1players) = 0;
		if (team2players) (*team2players) = 0;
		
		while (from = G_Find(from, FOFS(classname), "player")) {
			if (from->client) {
				if (IS_TEAM1(from)) {
					if (team1score) (*team1score) += from->client->resp.score;
					if (team1players) (*team1players)++;
				} else if (IS_TEAM2(from)) {
					if (team2score) (*team2score) += from->client->resp.score;
					if (team2players) (*team2players)++;
				}
			}
		}
}

void team_change(edict_t *ent, char *s)
{
	if (Q_strcasecmp(s, team_nameforteam[CTF_TEAM1]) == 0 && (ent->client->resp.ctf_team != CTF_TEAM1))
	{
		ent->client->resp.ctf_team = CTF_TEAM1;
		if (!ent->client->resp.spectator)
			T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TEAM_SWITCH);
		ent->client->resp.score = 0;			// reset score
		ent->client->resp.ctf_state = CTF_STATE_START;
		ent->client->pers.spectator = false;
		Info_SetValueForKey(ent->client->pers.userinfo, "spectator", "0");
		gi.WriteByte (svc_stufftext);
		gi.WriteString ("spectator 0\n");
		gi.unicast(ent, true);
	}
	else if (Q_strcasecmp(s, team_nameforteam[CTF_TEAM2]) == 0 && (ent->client->resp.ctf_team != CTF_TEAM2))
	{
		ent->client->resp.ctf_team = CTF_TEAM2;
		if (!ent->client->resp.spectator)
			T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TEAM_SWITCH);
		ent->client->resp.score = 0;			// reset score
		ent->client->resp.ctf_state = CTF_STATE_START;
		ent->client->pers.spectator = false;
		Info_SetValueForKey(ent->client->pers.userinfo, "spectator", "0");
		gi.WriteByte (svc_stufftext);
		gi.WriteString ("spectator 0\n");
		gi.unicast(ent, true);
	}
	else if (Q_strcasecmp(s, "spectator") == 0 || Q_strcasecmp(s, "spec") == 0 ||
				Q_strcasecmp(s, "observer") == 0 || Q_strcasecmp(s, "observe") == 0)
	{
		ent->client->pers.spectator = true;
		Info_SetValueForKey(ent->client->pers.userinfo, "spectator", "1");
		gi.WriteByte (svc_stufftext);
		gi.WriteString ("spectator 1\n");
		gi.unicast(ent, true);
	}

}

// command to switch teams
void Cmd_Team_f(edict_t *ent)
{
	team_change(ent, gi.args());
}

void Svcmd_ForceTeam_f ()
{
	int playernum;
	edict_t *e;

	if (Q_stricmp(gi.argv(2), "#") == 0)
	{
		playernum = atoi(gi.argv(3));
		if (playernum >= 0 && playernum < maxclients->value &&
			g_edicts[playernum + 1].inuse)
		{
			team_change(&g_edicts[playernum + 1], gi.argv(4));
		}
	}
	else
	{
		e = NULL;
		while (e = G_Find(e, FOFS(classname), "player"))
		{
			if (e->client && Q_stricmp(e->client->pers.netname, gi.argv(2)) == 0)
				break;
		}
		if (e)
			team_change(e, gi.argv(3));
	}
}


/*QUAKED info_player_team1 (1 0 0) (-16 -16 -24) (16 16 32)
potential team1 spawning position for ctf games
*/
void SP_info_player_team1(edict_t *self)
{
}

/*QUAKED info_player_team2 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team2(edict_t *self)
{
}

// ZOID
/*
================
SelectCTFSpawnPoint

go to a ctf point, but NOT the two points closest
to other players
================
*/
edict_t *SelectDeathmatchSpawnPoint (void);
float	PlayersRangeFromSpot (edict_t *spot);
edict_t *SelectCTFSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	*cname;

	if (ent->client->resp.ctf_state != CTF_STATE_START)
		return SelectDeathmatchSpawnPoint ();
/*
		if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
			return SelectFarthestDeathmatchSpawnPoint ();
		else
			return SelectRandomDeathmatchSpawnPoint ();
*/

	ent->client->resp.ctf_state = CTF_STATE_PLAYING;

	switch (ent->client->resp.ctf_team) {
	case CTF_TEAM1:
		cname = "info_player_team1";
		break;
	case CTF_TEAM2:
		cname = "info_player_team2";
		break;
	default:
		return SelectDeathmatchSpawnPoint();
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
		return SelectDeathmatchSpawnPoint();

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
// End ZOID

void team_assign(edict_t *ent)
{
	int	redscore, bluescore, redplayers, blueplayers;
	team_scores(&redscore, &bluescore, &redplayers, &blueplayers);

	if (redplayers < blueplayers)
		ent->client->resp.ctf_team = CTF_TEAM1;
	else if (blueplayers < redplayers)
		ent->client->resp.ctf_team = CTF_TEAM2;
	else if (redscore < bluescore)
		ent->client->resp.ctf_team = CTF_TEAM1;
	else if (bluescore < redscore)
		ent->client->resp.ctf_team = CTF_TEAM2;
	else if (random() < 0.5)
		ent->client->resp.ctf_team = CTF_TEAM1;
	else
		ent->client->resp.ctf_team = CTF_TEAM2;

	gi.bprintf(PRINT_HIGH, "%s joins the %s team\n",
		ent->client->pers.netname, 
		team_nameforteam[ent->client->resp.ctf_team]);

	ent->client->resp.ctf_state = CTF_STATE_START;
}

char *team_skin(edict_t *ent)
{
	if (!ent->client)
		return "male/grunt";

	if (IS_TEAM1(ent))
		return SKIN_TEAM1;

	if (IS_TEAM2(ent))
		return SKIN_TEAM2;

	return "male/grunt";
}
