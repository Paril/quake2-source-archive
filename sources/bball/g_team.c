#include "g_local.h"

/*
==================
BBallScoreboardMessage
==================
*/
void BBallScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[2][MAX_CLIENTS+MAX_BOTS];
	int		sortedscores[2][MAX_CLIENTS+MAX_BOTS];
	int		score, total[2], totalscore[2];
	int		last[2];
	gclient_t	*cl;
	gbot_t		*bot;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;
	int index;

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (game.clients[i].resp.team == TEAM1)
			team = 0;
		else if (game.clients[i].resp.team == TEAM2)
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

	// add the bots by team and score
	for ( ; i<game.maxbots+game.maxclients ; i++)
	{
		index = i - game.maxclients;
		
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (game.bots[index].resp.team == TEAM1)
			team = 0;
		else if (game.bots[index].resp.team == TEAM2)
			team = 1;
		else
			continue; // unknown team?

		score = game.bots[index].resp.score;
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
	sprintf(string, "if 25 xv 8 yv 8 pic 25 endif "
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 98 yv 12 num 2 18 "
		"if 26 xv 168 yv 8 pic 26 endif "
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 256 yv 12 num 2 19 ",
		totalscore[0], total[0],
		totalscore[1], total[1]);
	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total[0] && i >= total[1])
			break; // we're done

		*entry = 0;

		// left side
		if (i < total[0])
		{
			cl_ent = g_edicts + 1 + sorted[0][i];
			
			if(cl_ent->client)
			{
				cl = &game.clients[sorted[0][i]];

				sprintf(entry+strlen(entry),
					"ctf 0 %d %d %d %d ",
					42 + i * 8,
					sorted[0][i],
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);

				if (cl_ent->client->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))])
					sprintf(entry + strlen(entry), "xv 56 yv %d picn miniball ",
						42 + i * 8);
			}
			else
			{
				bot = &game.bots[(sorted[0][i]) - game.maxclients];

				sprintf(entry+strlen(entry),
					"ctf 0 %d %d %d %d ",
					42 + i * 8,
					sorted[0][i],
					bot->resp.score,
					0);

				sprintf(entry+strlen(entry), "xv 40 yv %d string \"*\" ", 42 + i * 8);
				
				if (cl_ent->bot->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))])
					sprintf(entry + strlen(entry), "xv 56 yv %d picn miniball ",
						42 + i * 8);
			}

			if (maxsize - len > strlen(entry))
			{
				strcat(string, entry);
				len = strlen(string);
				last[0] = i;
			}
		}

		// right side
		if (i < total[1])
		{
			cl_ent = g_edicts + 1 + sorted[1][i];
			if(cl_ent->client)
			{
				cl = &game.clients[sorted[1][i]];

				sprintf(entry+strlen(entry),
					"ctf 160 %d %d %d %d ",
					42 + i * 8,
					sorted[1][i],
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);

				if (cl_ent->client->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))])
					sprintf(entry + strlen(entry), "xv 216 yv %d picn miniball ",
						42 + i * 8);
			}
			else
			{
				bot = &game.bots[(sorted[1][i]) - game.maxclients];

				sprintf(entry+strlen(entry),
					"ctf 160 %d %d %d %d ",
					42 + i * 8,
					sorted[1][i],
					bot->resp.score,
					0);

				sprintf(entry+strlen(entry), "xv 200 yv %d string \"*\" ", 42 + i * 8);

				if (cl_ent->bot->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))])
					sprintf(entry + strlen(entry), "xv 216 yv %d picn miniball ",
						42 + i * 8);
			}

			if (maxsize - len > strlen(entry))
			{
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
	if (maxsize - len > 50)
	{
		for (i = 0; i < maxclients->value; i++)
		{
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			if (!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->resp.team != NOTEAM ||
				!cl_ent->client)
				continue;

			if (!k)
			{
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
			if (maxsize - len > strlen(entry))
			{
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


//===============================================================
//	Team selection stuff
//===============================================================

char *TeamName(int team)
{
	switch (team) {
	case TEAM1:
		return team1name->string;
	case TEAM2:
		return team2name->string;
	}
	return "UKNOWN";
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

	switch (ent->client->resp.team)
	{
		case TEAM1:
			gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
				ent->client->pers.netname, t, TEAM1_SKIN) );
			break;
		case TEAM2:
			gi.configstring (CS_PLAYERSKINS+playernum,
				va("%s\\%s%s", ent->client->pers.netname, t, TEAM2_SKIN) );
			break;
		default:
			gi.configstring (CS_PLAYERSKINS+playernum, 
				va("%s\\%s", ent->client->pers.netname, s) );
			break;
	}
}

void AssignTeam(gclient_t *who)
{
	edict_t		*player;
	int i;
	int team1count = 0, team2count = 0;

	who->resp.state = STATE_START;

	if (!((int)dmflags->value & DF_FORCEJOIN))
	{
		who->resp.team = NOTEAM;
		return;
	}

	for (i = 1; i <= maxclients->value; i++)
	{
		player = &g_edicts[i];

		if (!player->inuse || player->client == who)
			continue;

		switch (player->client->resp.team)
		{
			case TEAM1:
				team1count++;
				break;
			case TEAM2:
				team2count++;
		}
	}
	if (team1count < team1count)
		who->resp.team = TEAM1;
	else if (team2count < team1count)
		who->resp.team = TEAM2;
	else if (rand() & 1)
		who->resp.team = TEAM1;
	else
		who->resp.team = TEAM2;
}

void BballEffects(edict_t *ent)
{
	ent->s.effects &= (EF_FLAG1 | EF_FLAG2);
	if (ent->health > 0)
	{
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))]
			&& ent->client->resp.team == 1)
			ent->s.effects |= EF_FLAG1;
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))]
			&& ent->client->resp.team == 2)
			ent->s.effects |= EF_FLAG2;
	}
}

void Inbound(int team);

void Team_f (edict_t *ent)
{
	char *t, *s;
	int desired_team;

	if (!deathmatch->value)
		return;
		
	t = gi.args();
	if (!*t)
	{
		gi.cprintf(ent, PRINT_HIGH, "You are on the %s.\n",
			TeamName(ent->client->resp.team));
		return;
	}
	if (Q_stricmp(t, "red") == 0 || Q_stricmp(t, team1name->string) == 0)
		desired_team = TEAM1;
	else if (Q_stricmp(t, "blue") == 0 || Q_stricmp(t, team2name->string) == 0)
		desired_team = TEAM2;
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "Unknown team %s.\n", t);
		return;
	}

	if (ent->client->resp.team == desired_team)
	{
		gi.cprintf(ent, PRINT_HIGH, "You are already on the %s.\n",
			TeamName(ent->client->resp.team));
		return;
	}

////
	ent->svflags = 0;
	ent->flags &= ~FL_GODMODE;
	ent->client->resp.team = desired_team;
	ent->client->resp.state = STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	AssignSkin(ent, s);

	if (ent->solid == SOLID_NOT) // spectator
	{
		PutClientInServer (ent);
		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		gi.bprintf(PRINT_HIGH, "%s joined the %s.\n",
			ent->client->pers.netname, TeamName(desired_team));
		if(fixteamsize->value)
			BotEvenTeams();
		return;
	}

	ent->health = 0;
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
		
	ent->deadflag = DEAD_DEAD;
	respawn (ent);

	ent->client->resp.score = 0;

	gi.bprintf(PRINT_HIGH, "%s changed to the %s.\n",
		ent->client->pers.netname, TeamName(desired_team));
	if(fixteamsize->value)
		BotEvenTeams();

}

void Cmd_Observer_f(edict_t *ent)
{
	int index;
	int wasteam = ent->client->resp.team;
	
	if(ent->movetype == MOVETYPE_NOCLIP) //already observer
		return;
	
	//for 3.20 spec. variable
	ent->client->pers.spectator = true;
	ent->client->resp.spectator = true;
	
	if (ent->client->chasetoggle > 0)
	{
		ent->client->chasetoggle = 0;
		ChasecamRemove (ent);
	}
	
	// make 'observer'
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->resp.team = NOTEAM;
	ent->client->ps.gunindex = 0;
	gi.linkentity (ent);

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	//See if player left with the ball
	index = ITEM_INDEX(FindItem("basketballs"));
	if (ent->client->pers.inventory[index])
	{
		ent->client->pers.inventory[ITEM_INDEX(FindItem("BasketBalls"))] = 0;
		Inbound(3 - wasteam);
	}

	//fix teams if desired
	if(fixteamsize->value)
		BotEvenTeams();

	gi.bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
}



void JoinTeam(edict_t *ent, int desired_team)
{
	char *s;
	
	PMenu_Close(ent);

	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.team = desired_team;
	ent->client->resp.state = STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	AssignSkin(ent, s);

	PutClientInServer (ent);
	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
	gi.bprintf(PRINT_HIGH, "%s joined the %s.\n",
		ent->client->pers.netname, TeamName(desired_team));
	if(fixteamsize->value)
		BotEvenTeams();
}

void JoinTeam1(edict_t *ent, pmenu_t *p)
{
	JoinTeam(ent, TEAM1);
}

void JoinTeam2(edict_t *ent, pmenu_t *p)
{
	JoinTeam(ent, TEAM2);
}

void Menu_ChaseCam(edict_t *ent, pmenu_t *p)
{
	int i;
	edict_t *e;

	if (ent->client->chase_target)
	{
		ent->client->chase_target = NULL;
		PMenu_Close(ent);
		return;
	}

	for (i = 1; i <= maxclients->value; i++)
	{
		e = g_edicts + i;
		if (e->inuse && e->solid != SOLID_NOT)
		{
			ent->client->chase_target = e;
			PMenu_Close(ent);
			ent->client->update_chase = true;
			break;
		}
	}
}

void OpenJoinMenu(edict_t *ent);

void ReturnToMain(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	OpenJoinMenu(ent);
}

void Credits(edict_t *ent, pmenu_t *p);

void DeathmatchScoreboard (edict_t *ent);

void ShowScores(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	ent->client->showscores = true;
	ent->client->showinventory = false;
	DeathmatchScoreboard (ent);
}

pmenu_t creditsmenu[] = {
	{ "*Quake II",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Quake II Basketball",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Programming",					PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "Matt Shade",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Level Design", 					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Gene 'Scuzzy' Beaumont",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Matt Shade",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Art",							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Matt Shade",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, NULL, ReturnToMain }
};


pmenu_t joinmenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Quake II Basketball",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Join Red Team",		PMENU_ALIGN_LEFT, NULL, JoinTeam1 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Join Blue Team",		PMENU_ALIGN_LEFT, NULL, JoinTeam2 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Chase Camera",		PMENU_ALIGN_LEFT, NULL, Menu_ChaseCam },
	{ "Credits",			PMENU_ALIGN_LEFT, NULL, Credits },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "v" BBALL_STRING_VERSION,	PMENU_ALIGN_RIGHT, NULL, NULL },
};

int UpdateJoinMenu(edict_t *ent)
{
	static char levelname[32];
	static char team1players[32];
	static char team2players[32];
	int num1, num2, i;

	joinmenu[4].text = team1name->string;
	joinmenu[4].SelectFunc = JoinTeam1;
	joinmenu[6].text = team2name->string;
	joinmenu[6].SelectFunc = JoinTeam2;

	if (forcejoin->string && *forcejoin->string)
	{
		if (stricmp(forcejoin->string, "red") == 0
			|| stricmp(forcejoin->string, team1name->string) == 0)
		{
			joinmenu[6].text = NULL;
			joinmenu[6].SelectFunc = NULL;
		}
		else if (stricmp(forcejoin->string, "blue") == 0
			|| stricmp(forcejoin->string, team2name->string) == 0)
		{
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
		if (game.clients[i].resp.team == TEAM1)
			num1++;
		else if (game.clients[i].resp.team == TEAM2)
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
		return TEAM1;
	else if (num2 > num1)
		return TEAM1;
	return (rand() & 1) ? TEAM1 : TEAM2;
}

void OpenJoinMenu(edict_t *ent)
{
	int team;

	team = UpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 8;
	else if (team == TEAM1)
		team = 4;
	else
		team = 6;
	PMenu_Open(ent, joinmenu, team, sizeof(joinmenu) / sizeof(pmenu_t));
}

void Credits(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu, -1, sizeof(creditsmenu) / sizeof(pmenu_t));
}

qboolean StartClient(edict_t *ent)
{
	if (ent->client->resp.team != NOTEAM || !deathmatch->value)
		return false;

	if (!((int)dmflags->value & DF_FORCEJOIN))
	{
		//M.S. added for 3.20 spec. variable
		ent->client->pers.spectator = true;
		ent->client->resp.spectator = true;
		
		// start as 'observer'
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.team = NOTEAM;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);

		OpenJoinMenu(ent);
		return true;
	}
	return false;
}

