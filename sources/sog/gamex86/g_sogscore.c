#include "g_local.h"
#include "g_sogutil.h"

//================================================================================
//================================================================================



/*
==================
GenTeamScoreboard
Class Based Teamgames - scoreboard
==================
*/

void GenTeamScoreboard (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[4][MAX_CLIENTS];
	int		sortedscores[4][MAX_CLIENTS];
	int		score, total[4];//, totalscore[4];
	int		last[4];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1024;

	// sort the clients by team and score
	total[0] = total[1] = total[2] = total[3] = 0;
	last[0] = last[1] =  last[2] = last[3] = 0;
//	totalscore[0] = totalscore[1] = totalscore[2] = totalscore[3] = 0;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		
		if (!cl_ent->inuse)
			continue;
		
//		if (cl_ent->solid = SOLID_NOT)
//			continue;
			
		if (cl_ent->client->resp.spectator)
			continue;

		if (game.clients[i].resp.player_class == CLASS_WOLF)
		  team = 0;
		else if (game.clients[i].resp.player_class == CLASS_DOOM)
		  team = 1;
		else if (game.clients[i].resp.player_class == CLASS_Q1)
		  team = 2;
		else if (game.clients[i].resp.player_class == CLASS_Q2)
		  team = 3;
		else
		  continue;  // unknown team		  

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
//		totalscore[team] += score;
		total[team]++;

	}


	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	// team one
	sprintf(string, 
		"xv 8 yv 8 picn ctfidg1 "	
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 168 yv 8 picn ctfidg2 "	
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 8 yv 108 picn ctfidg3 "	
		"xv 40 yv 128 string \"%4d/%-3d\" "
		"xv 168 yv 108 picn ctfidg4 "	
		"xv 200 yv 128 string \"%4d/%-3d\" ",
//		totalscore[0], total[0],
//		totalscore[1], total[1],
//		totalscore[2], total[2],
//		totalscore[3], total[3]
		gengame.team1, total[0],
		gengame.team2, total[1],
		gengame.team3, total[2],
		gengame.team4, total[3]
		);

	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total[0] && i >= total[1] && i >= total[2] && i >=total[3])
			break; // we're done

		*entry = 0;

		// idg1 players
		if (i < total[0]) 
		{
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];

			sprintf(entry+strlen(entry),
				"ctf 0 %d %d %d %d ",
				42 + i * 8,
				sorted[0][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

			if (maxsize - len > strlen(entry)) 
			{
				strcat(string, entry);
				len = strlen(string);
				last[0] = i;
			}
		}

		// idg2 players
		if (i < total[1]) 
		{
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];

			sprintf(entry+strlen(entry),
				"ctf 160 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

			if (maxsize - len > strlen(entry)) 
			{
				strcat(string, entry);
				len = strlen(string);
				last[1] = i;
			}
		}

		// idg3 players
		if (i < total[2]) 
		{
			cl = &game.clients[sorted[2][i]];
			cl_ent = g_edicts + 1 + sorted[2][i];

			sprintf(entry+strlen(entry),
				"ctf 0 %d %d %d %d ",
				142 + i * 8,
				sorted[2][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

			if (maxsize - len > strlen(entry)) 
			{
				strcat(string, entry);
				len = strlen(string);
				last[2] = i;
			}
		}

		// idg4 players
		if (i < total[3]) 
		{
			cl = &game.clients[sorted[3][i]];
			cl_ent = g_edicts + 1 + sorted[3][i];

			sprintf(entry+strlen(entry),
				"ctf 160 %d %d %d %d ",
				142 + i * 8,
				sorted[3][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

			if (maxsize - len > strlen(entry)) 
			{
				strcat(string, entry);
				len = strlen(string);
				last[3] = i;
			}
		}

	}

	// put in spectators if we have enough room
	if (last[2] > last[3])
		j = last[2];
	else
		j = last[3];
	j = (j + 2) * 8 + 142;

	k = n = 0;
	if (maxsize - len > 50)
	{
		for (i = 0; i < maxclients->value; i++)
		{
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			if ((!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT) &&
				(!cl_ent->client->resp.spectator))
				continue;

			if (level.intermissiontime != 0 &&
				!cl_ent->client->resp.spectator)
				continue; 


			if (cl_ent->client->pers.health <= 0 &&
				!cl_ent->client->resp.spectator)
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

	if (total[2] - last[2] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			142 + (last[2]+1)*8, total[2] - last[2] - 1);
	
	if (total[3] - last[3] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
			142 + (last[3]+1)*8, total[3] - last[3] - 1);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}




/*
==================
GenRBScoreboardMessage
Red and Blue Teamgames - scoreboard
==================
*/

void GenRBScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[2][MAX_CLIENTS];
	int		sortedscores[2][MAX_CLIENTS];
	int		score, total[2]; //, totalscore[2];
	int		last[2];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	//totalscore[0] = totalscore[1] = 0;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		
		if (!cl_ent->inuse)
			continue;

		if (cl_ent->client->resp.spectator)
			continue;
		
		if (game.clients[i].resp.team == CTF_TEAM1)
			team = 0;
		else if (game.clients[i].resp.team == CTF_TEAM2)
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
		//totalscore[team] += score;
		total[team]++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	// team one
	sprintf(string, 
		"xv 8 yv 8 picn ctfsb1 "	
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 168 yv 8 picn ctfsb2 "	
		"xv 200 yv 28 string \"%4d/%-3d\" ",
		gengame.team1, total[0],
		gengame.team2, total[1]);
	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total[0] && i >= total[1])
			break; // we're done

		*entry = 0;

		// left side
		if (i < total[0]) 
		{
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];

			sprintf(entry+strlen(entry),
				"ctf 0 %d %d %d %d ",
				42 + i * 8,
				sorted[0][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

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

			sprintf(entry+strlen(entry),
				"ctf 160 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

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
			if ((!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->resp.team != CTF_NOTEAM) &&
				(!cl_ent->client->resp.spectator))
				continue;

			if (cl_ent->client->pers.health <= 0 &&
				!cl_ent->client->resp.spectator)
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
