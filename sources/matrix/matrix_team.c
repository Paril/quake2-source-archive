#include "g_local.h"

void Cmd_ChooseMenu_f (edict_t *ent)
{
	gi.dprintf ("DEBUG: Create equipment selection Menu(s)\n");
}
void AssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];

	Com_sprintf(t, sizeof(t), "%s", s);

	if ((p = strrchr(t, '/')) != NULL)
		p[1] = 0;
	else
		strcpy(t, "male/");

	switch (ent->client->resp.team) {
	case TEAM_RED:
		gi.configstring (CS_PLAYERSKINS+playernum,
			va("%s\\%s%s", ent->client->pers.netname, t, redteamskin->string) );
		break;
	case TEAM_BLUE:
		gi.configstring (CS_PLAYERSKINS+playernum,
			va("%s\\%s%s", ent->client->pers.netname, t, blueteamskin->string) );
		break;
	case 0:
	default:
		gi.configstring (CS_PLAYERSKINS+playernum, 
			va("%s\\%s", ent->client->pers.netname, s) );
		break;
	}
//	gi.dprintf(ent, PRINT_HIGH, "DEBUG: %s has been assigned skin: %s\n", ent->client->pers.netname, s);
}
// char *teamname
char *TeamName(int team)
{
	switch(team)
	{
	case TEAM_RED:
		return redteamname->string;
	case TEAM_BLUE:
		return blueteamname->string;
	default:
		return "unknown";
	}
}

void JoinTeam (edict_t *ent, int desired_team)
{
	char *skin;

	if (desired_team == ent->client->resp.team)
	{
		gi.cprintf(ent,PRINT_HIGH, "already on the %s team\n", TeamName(ent->client->resp.team));
		return;
	}

	ent->svflags = 0;
	ent->flags &= ~FL_GODMODE;

	skin = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	AssignSkin(ent, skin);

	ent->client->resp.spectator = false;

	ent->client->resp.team = desired_team;

	if (ent->client->pers.spectator)
	{ // spectator
		ent->client->pers.spectator = false;
		// reset his spectator var
		spectator_respawn(ent);

		PutClientInServer (ent);
		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		if (desired_team != TEAM_FREE)
			gi.bprintf(PRINT_HIGH, "%s joined the %s team.\n",
					ent->client->pers.netname, TeamName(ent->client->resp.team));
		if ((!matrix.started) && (tankmode->value))
			gi.centerprintf(ent, "Type Ready in the console to begin tank play\n");

		return;
	}

	ent->health = 0;
	if(ent->client)
	MatrixRespawn (ent, ent);
	else
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;
	respawn (ent);
	ent->client->resp.score = 0;

	if (desired_team != TEAM_FREE)
		gi.bprintf(PRINT_HIGH, "%s changed to the %s team.\n",ent->client->pers.netname, TeamName(ent->client->resp.team));

}

void JoinTeam1(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	JoinTeam(ent, TEAM_RED);
//	if (!ent->client->resp.choosen && choosestuff->value)
//		Cmd_ChooseMenu_f(ent);
}
void JoinTeam2(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	JoinTeam(ent, TEAM_BLUE);
//	if (!ent->client->resp.choosen && choosestuff->value)
//		Cmd_ChooseMenu_f(ent);
}
void FreeTeam(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	JoinTeam(ent, TEAM_FREE);
//	if (!ent->client->resp.choosen && choosestuff->value)
//		Cmd_ChooseMenu_f(ent);
}
pmenu_t joinmenu[] = {
	{ "*" GAMEVERSION,			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Join Red Team",		PMENU_ALIGN_LEFT, NULL, JoinTeam1 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Join Blue Team",		PMENU_ALIGN_LEFT, NULL, JoinTeam2 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
};
void UpdateJoinMenu (edict_t *ent)
{
	int	i, num1 = 0, num2 = 0;
	static char team1players[32];
	static char team2players[32];
	static char team1name[32];
	static char team2name[32];

	if (teamplay->value)
	{
		for (i = 0; i < maxclients->value; i++)
		{
			if (!g_edicts[i+1].inuse)
				continue;
			if (game.clients[i].resp.team == TEAM_RED)
				num1++;
			else if (game.clients[i].resp.team == TEAM_BLUE)
				num2++;
		}
		sprintf(team1players, "  (%i players)", num1);
		sprintf(team2players, "  (%i players)", num2);
		joinmenu[3].text = team1players;
		joinmenu[5].text = team2players;

		sprintf(team1name, "Join %s Team", redteamname->string);
		sprintf(team2name, "Join %s Team", blueteamname->string);
		joinmenu[2].text = team1name;
		joinmenu[4].text = team2name;

		joinmenu[4].SelectFunc = JoinTeam2;
		joinmenu[2].SelectFunc = JoinTeam1;

	}
	else
	{
		joinmenu[2].text ="Join the Game";
		joinmenu[2].SelectFunc = FreeTeam;
		joinmenu[3].text= NULL;
		joinmenu[3].SelectFunc = NULL;
		joinmenu[4].text= NULL;
		joinmenu[4].SelectFunc = NULL;
		joinmenu[5].text= NULL;
		joinmenu[5].SelectFunc = NULL;
		joinmenu[6].text= NULL;
		joinmenu[6].SelectFunc = NULL;

		for (i = 0; i < maxclients->value; i++)
		{
			if (!g_edicts[i+1].inuse)
				continue;
			if (game.clients[i].resp.team == TEAM_FREE)
				num1++;

		}
		sprintf(team1players, "  (%i players)", num1);
		joinmenu[3].text = team1players;
		
	}

}
void Cmd_JoinMenu_f (edict_t *ent)
{
	UpdateJoinMenu(ent);
	PMenu_Open(ent, joinmenu, 3, sizeof(joinmenu) / sizeof(pmenu_t));
}

void TeamplayScoreboardMessage (edict_t *ent, edict_t *killer)
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
		if (game.clients[i].resp.team == TEAM_RED)
			team = 0;
		else if (game.clients[i].resp.team == TEAM_BLUE)
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
	ent->client->ps.stats[STAT_TEAM_RED_SCORE] = matrix.team_red_score;
	ent->client->ps.stats[STAT_TEAM_BLUE_SCORE] =  matrix.team_blue_score;
	// team one
	sprintf(string,
		"xv 8 yv 8 picn tag1 "
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 98 yv 12 num 2 29 "
		"xv 168 yv 8 picn tag2 "
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 256 yv 12 num 2 30 ",
		matrix.team_red_score, total[0],
		matrix.team_blue_score, total[1]);
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
				strcat(entry, "xv 56 picn sbfctf2 ");
#else
			sprintf(entry+strlen(entry),
				"ctf 0 %d %d %d %d ",
				42 + i * 8,
				sorted[0][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

		/*	if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
				sprintf(entry + strlen(entry), "xv 56 yv %d picn sbfctf2 ",
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

		/*	if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
				strcat(entry, "xv 216 picn sbfctf1 ");*/

#else

			sprintf(entry+strlen(entry),
				"ctf 160 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

/*			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
				sprintf(entry + strlen(entry), "xv 216 yv %d picn sbfctf1 ",
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
				cl_ent->client->resp.spectator ||
				cl_ent->client->resp.tank	   ||
				cl_ent->client->resp.team != TEAM_FREE)
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

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}
