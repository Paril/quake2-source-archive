#include "g_local.h"

/*
==================
FourPlusTeamsScoreBoard - If there are more then 4 teams, use this one...
==================
*/
void FourPlusTeamsScoreBoard (edict_t *ent)
{
        char            entry[1024];
        char            string[1400];
	gclient_t	*cl;
	edict_t		*cl_ent;
        teams_t         *team;
        int             i, j, k, n, len;
	int		sorted[2][MAX_CLIENTS];
	int		sortedscores[2][MAX_CLIENTS];
        int             score, total[2], totalscore[2];
	int		last[2];
        int             maxsize = 1000;

        // sort teams and clients by score and into two seperate columns.

        total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;

	for (i=0 ; i<game.maxclients ; i++)
        {
		cl_ent = g_edicts + 1 + i;

                if (!cl_ent->inuse || !cl_ent->client->resp.team)
			continue;

		score = game.clients[i].resp.score;
                for (j=0 ; j<total[1] ; j++)
		{
                        if (score > sortedscores[1][j])
				break;
		}
                for (k=total[1] ; k>j ; k--)
		{
                        sorted[1][k] = sorted[1][k-1];
                        sortedscores[1][k] = sortedscores[1][k-1];
		}
                sorted[1][j] = i;
                sortedscores[1][j] = score;
                totalscore[1] += score;
                total[1]++;
	}

        for (i=1 ; i<=max_teams ; i++)
	{
                team = GetTeamByIndex(i);               

                if (team == NULL)
                {
                        gi.dprintf ("TeamScoreBoard - No team at index %i.\n", i);
                        break;
                }

                if (team->members < 1)
                        continue;

                score = team->total_score;
                for (j=0 ; j<total[0] ; j++)
		{
                        if (score > sortedscores[0][j])
				break;
		}
                for (k=total[0] ; k>j ; k--)
		{
                        sorted[0][k] = sorted[0][k-1];
                        sortedscores[0][k] = sortedscores[0][k-1];
		}
                sorted[0][j] = i;
                sortedscores[0][j] = score;
                totalscore[0] += score;
                total[0]++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

        ent->client->ps.stats[STAT_TEAM1] = gi.imageindex ("teams");
        ent->client->ps.stats[STAT_TEAM2] = gi.imageindex ("players");

        sprintf(string, "if 24 xv 8 yv 8 pic 27 endif "
        "if 26 xv 168 yv 8 pic 28 endif ");

	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total[0] && i >= total[1])
			break; // we're done

		// set up y
		sprintf(entry, "yv %d ", 42 + i * 8);
		if (maxsize - len > strlen(entry))
                {
			strcat(string, entry);
			len = strlen(string);
		}

                // left side -> teams...
		if (i < total[0])
                {
                        team = GetTeamByIndex(sorted[0][i]);

                        sprintf(entry+strlen(entry),
                        "xv 0 %s \"%3d/%3d %-12.12s\" ",
                        (team == ent->client->resp.team) ? "string2" : "string",
                        team->total_score, team->members, team->name);

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
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];

#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
			"xv 160 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);
#else
			sprintf(entry+strlen(entry),
				"ctf 160 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);
#endif

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

                        if (!cl_ent->inuse || cl_ent->solid != SOLID_NOT || cl_ent->client->resp.team)
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


/*
==================
FourTeamScoreboard - Less then 5, use this one.
==================
*/
void FourTeamScoreboard (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
        int             sorted[4][MAX_CLIENTS];
        int             sortedscores[4][MAX_CLIENTS];
        int             score, total[4], totalscore[4];
        int             last[4];
	gclient_t	*cl;
	edict_t		*cl_ent;
        teams_t         *team1, *team2, *team3, *team4;
        int team_place;
	int maxsize = 1000;

	// sort the clients by team and score
        total[0] = total[1] = total[2] = total[3] = 0;
        last[0] = last[1] = last[2] = last[3] = 0;
        totalscore[0] = totalscore[1] = totalscore[2] = totalscore[3] = 0;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

                if (game.clients[i].resp.team == NULL)
			continue;

                team_place = ((TEAM_INDEX(game.clients[i].resp.team)) - 1);

		score = game.clients[i].resp.score;
                for (j=0 ; j<total[team_place] ; j++)
		{
                        if (score > sortedscores[team_place][j])
				break;
		}
                for (k=total[team_place] ; k>j ; k--)
		{
                        sorted[team_place][k] = sorted[team_place][k-1];
                        sortedscores[team_place][k] = sortedscores[team_place][k-1];
		}
                sorted[team_place][j] = i;
                sortedscores[team_place][j] = score;
                totalscore[team_place] += score;
                total[team_place]++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	// team one
        team1 = GetTeamByIndex(1);
        ent->client->ps.stats[STAT_TEAM1] = team1->total_score;
                              
        team2 = GetTeamByIndex(2);
        ent->client->ps.stats[STAT_TEAM2] = team2->total_score;

        if (max_teams > 3)
        {
                team3 = GetTeamByIndex(3);
                ent->client->ps.stats[STAT_TEAM3] = team3->total_score;

                team4 = GetTeamByIndex(4);
                ent->client->ps.stats[STAT_TEAM4] = team4->total_score;

                sprintf(string,
                // Team 1
                "yv 8 xv 30 picn \"blank\" "
                "yv 8 xv 8 picn \"../players/%s/%s_i\" "
                "yv 12 xv 120 num 2 27 "
                "yv 20 xv 42 %s \"%-12.12s\" "
                "yv 28 xv 42 string \"%4d/%-3d\" "
                // Team 2
                "yv 8 xv 190 picn \"blank\" "
                "yv 8 xv 168 picn \"../players/%s/%s_i\" "
                "yv 12 xv 278 num 2 28 "
                "yv 20 xv 202 %s \"%-12.12s\" "
                "yv 28 xv 202 string \"%4d/%-3d\" "
                // Team 3
                "yv 156 xv 30 picn \"blank\" "
                "yv 156 xv 8 picn \"../players/%s/%s_i\" "
                "yv 160 xv 120 num 2 29 "
                "yv 168 xv 42 %s \"%-12.12s\" "
                "yv 176 xv 42 string \"%4d/%-3d\" "
                // Team 4
                "yv 156 xv 190 picn \"blank\" "
                "yv 156 xv 168 picn \"../players/%s/%s_i\" "
                "yv 160 xv 278 num 2 30 "
                "yv 168 xv 202 %s \"%-12.12s\" "
                "yv 176 xv 202 string \"%4d/%-3d\" ",
                team1->model, team1->skin,
                (team1 == ent->client->resp.team) ? "string2" : "string",
                team1->name, totalscore[0], team1->members,
                team2->model, team2->skin,
                (team2 == ent->client->resp.team) ? "string2" : "string",
                team2->name, totalscore[1], team2->members,
                team3->model, team3->skin,
                (team3 == ent->client->resp.team) ? "string2" : "string",
                team3->name, totalscore[2], team3->members,
                team4->model, team4->skin,
                (team4 == ent->client->resp.team) ? "string2" : "string",
                team4->name, totalscore[3], team4->members);
        }
        else if (max_teams > 2)
        {
                team3 = GetTeamByIndex(3);
                ent->client->ps.stats[STAT_TEAM3] = team3->total_score;

                sprintf(string,
                // Team 1
                "yv 8 xv 30 picn \"blank\" "
                "yv 8 xv 8 picn \"../players/%s/%s_i\" "
                "yv 12 xv 120 num 2 27 "
                "yv 20 xv 42 %s \"%-12.12s\" "
                "yv 28 xv 42 string \"%4d/%-3d\" "
                // Team 2
                "yv 8 xv 190 picn \"blank\" "
                "yv 8 xv 168 picn \"../players/%s/%s_i\" "
                "yv 12 xv 278 num 2 28 "
                "yv 20 xv 202 %s \"%-12.12s\" "
                "yv 28 xv 202 string \"%4d/%-3d\" "
                // Team 3
                "yv 156 xv 30 picn \"blank\" "
                "yv 156 xv 8 picn \"../players/%s/%s_i\" "
                "yv 160 xv 120 num 2 29 "
                "yv 168 xv 42 %s \"%-12.12s\" "
                "yv 176 xv 42 string \"%4d/%-3d\" ",
                team1->model, team1->skin,
                (team1 == ent->client->resp.team) ? "string2" : "string",
                team1->name, totalscore[0], team1->members,
                team2->model, team2->skin,
                (team2 == ent->client->resp.team) ? "string2" : "string",
                team2->name, totalscore[1], team2->members,
                team3->model, team3->skin,
                (team3 == ent->client->resp.team) ? "string2" : "string",
                team3->name, totalscore[2], team3->members);
        }
        else
        {
                sprintf(string,
                // Team 1
                "yv 8 xv 30 picn \"blank\" "
                "yv 8 xv 8 picn \"../players/%s/%s_i\" "
                "yv 12 xv 120 num 2 27 "
                "yv 20 xv 42 %s \"%-12.12s\" "
                "yv 28 xv 42 string \"%4d/%-3d\" "
                // Team 2
                "yv 8 xv 190 picn \"blank\" "
                "yv 8 xv 168 picn \"../players/%s/%s_i\" "
                "yv 12 xv 278 num 2 28 "
                "yv 20 xv 202 %s \"%-12.12s\" "
                "yv 28 xv 202 string \"%4d/%-3d\" ",
                team1->model, team1->skin,
                (team1 == ent->client->resp.team) ? "string2" : "string",
                team1->name, totalscore[0], team1->members,
                team2->model, team2->skin,
                (team2 == ent->client->resp.team) ? "string2" : "string",
                team2->name, totalscore[1], team2->members);
        }

	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{

#if 0 //ndef NEW_SCORE
		// set up y
		sprintf(entry, "yv %d ", 42 + i * 8);
		if (maxsize - len > strlen(entry))
                {
			strcat(string, entry);
			len = strlen(string);
		}
#else
		*entry = 0;
#endif

		// left side
		if (i < total[0])
                {
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];

#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
                        "xv 8 %s \"%3d %3d %-12.12s\" ",
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
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];
#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
                        "xv 168 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);
#else

			sprintf(entry+strlen(entry),
                                "ctf 168 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

#endif
			if (maxsize - len > strlen(entry))
                        {
				strcat(string, entry);
				len = strlen(string);
				last[1] = i;
			}
		}

		// left side
                if ((i < total[2]) && (max_teams > 2))
                {
                        cl = &game.clients[sorted[2][i]];
                        cl_ent = g_edicts + 1 + sorted[2][i];

#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
                        "xv 8 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);
#else
			sprintf(entry+strlen(entry),
				"ctf 0 %d %d %d %d ",
                                190 + i * 8,
                                sorted[2][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);
#endif

			if (maxsize - len > strlen(entry))
                        {
				strcat(string, entry);
				len = strlen(string);
                                last[2] = i;
			}
		}

		// right side
                if ((i < total[3]) && (max_teams > 3))
                {
                        cl = &game.clients[sorted[3][i]];
                        cl_ent = g_edicts + 1 + sorted[3][i];
#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
                        "xv 168 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);
#else

			sprintf(entry+strlen(entry),
				"ctf 160 %d %d %d %d ",
                                190 + i * 8,
                                sorted[3][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

#endif
			if (maxsize - len > strlen(entry))
                        {
				strcat(string, entry);
				len = strlen(string);
                                last[3] = i;
			}
		}
	}


	// put in spectators if we have enough room
        if (max_teams > 3)
        {
                if (last[3] > last[2])
                        j = last[3];
                else
                        j = last[2];

        }
        else if (max_teams > 2)
                j = last[2];
        else if (last[3] > last[2])
                j = last[3];
        else
                j = last[2];

        j = (j + 2) * 8 + 42;

	k = n = 0;
	if (maxsize - len > 50)
        {
		for (i = 0; i < maxclients->value; i++)
                {
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];

                        if (!cl_ent->inuse || cl_ent->solid != SOLID_NOT || cl_ent->client->resp.team)
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

        // check for extra ppl who couldn't be fit in
        if (total[0] - last[0] > 1)
		sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			42 + (last[0]+1)*8, total[0] - last[0] - 1);
        if (total[1] - last[1] > 1)
		sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
			42 + (last[1]+1)*8, total[1] - last[1] - 1);

        if ((total[2] - last[2] > 1) && (max_teams > 2))
		sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
                        136 + (last[0]+1)*8, total[0] - last[0] - 1);

        if ((total[3] - last[3] > 1) && (max_teams > 3))
		sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
                        136 + (last[1]+1)*8, total[1] - last[1] - 1);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


/*
==================
TeamScoreBoard - The one thing I haven't got much of a clue on.
==================
*/
void TeamScoreBoard (edict_t *ent)
{
        if (max_teams < 5)
                FourTeamScoreboard (ent);
        else
                FourPlusTeamsScoreBoard (ent);
}
