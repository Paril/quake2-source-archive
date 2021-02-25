#include "g_local.h"
#include "g_genutil.h"

//================================================================================
//================================================================================


char * GetClassPic(int pclass)
{
	switch(pclass)
	{
	case CLASS_Q1:
		return "q1hud/q1icon";
		break;
	case CLASS_DOOM:
		return "doomhud/doomicon";
		break;
	case CLASS_WOLF:
		return "wolfhud/wolficon";
		break;
	case CLASS_Q2:
	default:
		return "q1hud/q2icon";
		break;
	}
}

char * GetClassChar(int pclass)
{
	switch(pclass)
	{
	case CLASS_Q1:
		return "Q1";
		break;
	case CLASS_DOOM:
		return "D";
		break;
	case CLASS_WOLF:
		return "W";
		break;
	case CLASS_Q2:
		return "Q2";
		break;
	default:
		return " ";
		break;
	}
}



void GenTeamScoreboard (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[4][MAX_CLIENTS];
	int		sortedscores[4][MAX_CLIENTS];
	int		score, total[4], totalscore[2];
	int		last[4];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1024;

	// sort the clients by team and score
	total[0] = total[1] = total[2] = total[3] = 0;
	last[0] = last[1] =  last[2] = last[3] = 0;
	totalscore[0] = totalscore[1] = 0;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		
		if (!cl_ent->inuse)
			continue;
		
		team = game.clients[i].resp.player_class;
/*			 = 0;
		else if (game.clients[i].resp.player_class == CLASS_Q2)
			team = 1;
		else
			continue; // unknown team?*/
		if(team < 0 || team > 3)
			continue;
		
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
	sprintf(string, 
		"xv 8 yv 8 picn ctfdoom "	
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 168 yv 8 picn ctfwolf "	
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 8 yv 108 picn ctfq1 "	
		"xv 40 yv 128 string \"%4d/%-3d\" "
		"xv 168 yv 108 picn ctfq2 "	
		"xv 200 yv 128 string \"%4d/%-3d\" ",
		totalscore[0], total[0],
		totalscore[1], total[1],
		totalscore[2], total[2],
		totalscore[3], total[3]
		);

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
			if (!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->resp.player_class != NO_CLASS)
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
void GenTeamScoreboard (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[4][MAX_CLIENTS];
	int		sortedscores[4][MAX_CLIENTS];
	int		score, total[4], totalscore[2];
	int		last[4];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1024;

	// sort the clients by team and score
	total[0] = total[1] = total[2] = total[3] = 0;
	last[0] = last[1] =  last[2] = last[3] = 0;
	totalscore[0] = totalscore[1] = 0;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		
		if (!cl_ent->inuse)
			continue;
		
		if (game.clients[i].resp.player_class == CLASS_Q1)
			team = 0;
		else if (game.clients[i].resp.player_class == CLASS_Q2)
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
	sprintf(string, 
		"xv 8 yv 8 picn q1hud/ctf/ctfq1 "	
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 168 yv 8 picn q1hud/ctf/ctfq2 "	
		"xv 200 yv 28 string \"%4d/%-3d\" ",
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
			if (!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->resp.player_class != NO_CLASS)
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
*/


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
		totalscore[team] += score;
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
			if (!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->resp.team != CTF_NOTEAM)
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
QuakeWorld Style 
Deathmatch Scoreboard
====================
*/

void Q1DMScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		y;
	gclient_t	*cl;
	edict_t		*cl_ent;

	
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

	//Heading
	strcpy(string, "xv 44 yv 15 picn q1hud/dranking "); 
	stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 16)
		total = 16;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];
		
		y=60+(i*9);
		*entry = 0;

		if(ent->client == cl)
		{
			// send the layout
				sprintf(entry,
			"xv 47 yv  %i string2 \"%i\" "
			"xv 94 yv  %i string2 \"%i\" "
			"xv 142 yv %i string2 \"%i\" "
			"xv 172 yv %i string2 \"%s\" "				
			"xv 192 yv %i string2 \"%s\" ",
			y, cl->ping, 
			y, (level.framenum - cl->resp.enterframe)/600,
			y, cl->resp.score,
			y, GetClassChar(cl->resp.player_class),
//			y, GetClassPic(cl->resp.player_class),
			y, cl->pers.netname);
		}
		else
		{
			// send the layout
			sprintf(entry,
				"xv 47 yv %i string \"%i\" "
				"xv 94 yv %i string \"%i\" "
				"xv	142 yv %i string \"%i\" "
//				"xv 172 yv %i picn \"%s\" "				
				"xv 172 yv %i string \"%s\" "				
				"xv 192 yv %i string \"%s\" ",
				y, cl->ping,  
				y, (level.framenum - cl->resp.enterframe)/600, 
				y, cl->resp.score,
				y, GetClassChar(cl->resp.player_class),
//				y, GetClassPic(cl->resp.player_class),
				y, cl->pers.netname);
		}

		if((strlen(entry) + stringlength) < 1200)
		{
			strcat(string, entry);
			stringlength = strlen(string);
		}
	}

	if(strlen(string) >= 1200)
	{
gi.dprintf("Q1DM SCOREBOARD OVERFLOW:%d\n",stringlength);
		return;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


/*
==================
DOOM  
Deathmatch Scoreboard
====================
*/

void DoomDMScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		y;
	gclient_t	*cl;
	edict_t		*cl_ent;


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

	//Heading
	strcpy(string, "xv 44 yv 10 picn doomhud/dranking "); 
	stringlength = strlen(string);


	// add the clients in sorted order
	if (total > 16)
		total = 16;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];
		
		y=80+(i*9);
		*entry = 0;

		if(ent->client == cl)
		{
				// send the layout
			sprintf(entry,
			"xv 47 yv %i string2 \"%i\" "
			"xv 94 yv %i string2 \"%i\" "
			"xv 142 yv %i string2 \"%i\" "
//			"xv 172 yv %i picn \"%s\" "				
			"xv 172 yv %i string2 \"%s\" "				
			"xv 192 yv %i string2 \"%s\" ",
			y, cl->ping,  
			y, (level.framenum - cl->resp.enterframe)/600,
			y, cl->resp.score,
			y, GetClassChar(cl->resp.player_class),
//			y, GetClassPic(cl->resp.player_class),
			y, cl->pers.netname);
		}
		else
		{
			// send the layout
			sprintf(entry,
				"xv 47 yv %i string \"%i\" "
				"xv 94 yv %i string \"%i\" "
				"xv	142 yv %i string \"%i\" "
//				"xv 172 yv %i picn \"%s\" "				
				"xv 172 yv %i string \"%s\" "
				"xv 192 yv %i string \"%s\" ",
				y, cl->ping,  
				y, (level.framenum - cl->resp.enterframe)/600, 
				y, cl->resp.score,
				y, GetClassChar(cl->resp.player_class),
//				y, GetClassPic(cl->resp.player_class),
				y, cl->pers.netname);
		}

		if((strlen(entry) + stringlength) < 1200)
		{
			strcat(string, entry);
			stringlength = strlen(string);
		}
	}

	if(strlen(string) >= 1200)
	{
gi.dprintf("DOOM SCOREBOARD OVERFLOW:%d\n",stringlength);
		return;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}



/*
==================
Quake1 CTFScoreboardMessage
Thanks to  

  {nDo}BiG_DoG
bigdog@captured.com

for most of the following code (ripped from his Classic CTF mod)
====================
*/

void Q1CTFScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		totalping[2], bestping[2], worstping[2], thisping;
	int		team1plcount, team2plcount;
	int		last;
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

	// sort the clients by team and score
	total = 0;
	last = 0;
	totalping[0] = totalping[1] = 0;
	bestping[0] = bestping[1] = 999;
	worstping[0] = worstping[1] = 0;
	team1plcount=team2plcount = 0;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		
		cl = &game.clients[i];
		team = game.clients[i].resp.team;

		if(team == CTF_TEAM1)
			team1plcount ++;
		else if(team == CTF_TEAM2)
			team2plcount ++;
	
		score = game.clients[i].resp.score;
		thisping = (cl->ping > 999 ? 999 : cl->ping);

		// This finds out where the player falls in
		// the list
		for (j=0 ; j<i ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		// This moves all scores from that point
		// down a level
		for (k=total ; k>j ; k--)
		{
			sortedscores[k] = sortedscores[k-1];
			sorted[k] = sorted[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;

//Skid added		
//I have teams as 1 and 2 as opposed to 0 and 1
		team--;
		
		totalping[team] += thisping;
		if (thisping < bestping[team]) 
			bestping[team] = thisping;
		if (thisping > worstping[team]) 
			worstping[team] = thisping;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	// team one
	sprintf(string,
		"xv 35 yv 0 picn q1hud/ctf/ranking " 
		"xv 35 yv 41 string \"%3d/%3d/%3d\" "
		"xv 151 yv 40 picn q1hud/ctf/ctfteam1 "
		"xv 190 yv 41 string \"%3d\" "
		"xv 235 yv 41 string \"%3d\" "
		"xv 35 yv 51 string \"%3d/%3d/%3d\" "
		"xv 151 yv 50 picn q1hud/ctf/ctfteam2 "
		"xv 190 yv 51 string \"%3d\" "
		"xv 235 yv 51 string \"%3d\" ",
		bestping[0], (team1plcount > 0 ? (totalping[0] / team1plcount) : 0), worstping[0], gengame.team1, team1plcount,
		bestping[1], (team2plcount > 0 ? (totalping[1] / team2plcount) : 0), worstping[1], gengame.team2, team2plcount);
	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total)
			break; // we're done

		*entry = 0;
		team = 0;

		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		if(cl)
		{
			if(cl->resp.team)
			{
				if(cl->resp.team >0)
				{
					team = cl_ent->client->resp.team;
		
					sprintf(entry+strlen(entry),
					"ctf 80 %d %d %d %d xv 136 yv %d picn q1hud/ctf/ctfteam%d ",
					63 + i * 10,
					sorted[i],
					cl->ping > 999 ? 999 : cl->ping,
					cl->resp.score,
					62 + i * 10,
					team);
				}
				else
				{
					sprintf(entry+strlen(entry),
					"ctf 80 %d %d %d %d xv 136 yv %d ",
					63 + i * 10,
					sorted[i],
					cl->ping > 999 ? 999 : cl->ping,
					cl->resp.score,
					62 + i * 10
					);
				}


				if (maxsize - len > strlen(entry)) 
				{
					strcat(string, entry);
					len = strlen(string);
					last = i;
				}
				
			}
		}
	}

	// put in spectators if we have enough room
	j = (last + 2) * 10 + 62;

	k = n = 0;
	if (maxsize - len > 50)
	{
		for (i = 0; i < maxclients->value; i++) 
		{
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			if (!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->resp.team != CTF_NOTEAM)
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

	if ((total - last) > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			62 + (last+1)*8, total - last - 1);
	
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


void Q1IntermissionScreen(edict_t *ent)
{
	char	string[1280];
	char	s[] = "/";
	
	sprintf(string,
			"xv 10 yv 20 picn q1hud/q1inter "
			"xv 200 yv 68 string \"%.1f\" "
			"xv 202 yv 106 string \"%d\" "
			"xv 210 yv 106 string \"%s\" "
			"xv 218 yv 106 string \"%d\" "
			"xv 202 yv 148 string \"%d\" "
			"xv 210 yv 148 string \"%s\" "
			"xv 218 yv 148 string \"%d\" ",
			level.time,
			level.found_secrets, s,level.total_secrets,
			level.killed_monsters,s,level.total_monsters
			);

	if(strlen(string) >= 1200)
	{
gi.dprintf("Q1 SP INTERMISSION OVERFLOW:%d\n");
		return;
	}

	ent->client->showscores = true;
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}



void WolfDMScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		y;
	gclient_t	*cl;
	edict_t		*cl_ent;

//	char   bar[] = "9D9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9E9F";
	
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

	//Heading
	strcpy(string, "xv 44 yv 2 picn wolfhud/wolfrank "); 
	stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 16)
		total = 16;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];
		
		y=80+(i*9);
		*entry = 0;

		if(ent->client == cl)
		{
				// send the layout
			sprintf(entry,
			"xv 47 yv %i string2 \"%i\" "
			"xv 94 yv %i string2 \"%i\" "
			"xv 142 yv %i string2 \"%i\" "
//			"xv 172 yv %i picn \"%s\" "				
			"xv 172 yv %i string2 \"%s\" "				
			"xv 192 yv %i string2 \"%s\" ",
			y, cl->ping,  
			y, (level.framenum - cl->resp.enterframe)/600, 
			y, cl->resp.score,
			y, GetClassChar(cl->resp.player_class),
//			y, GetClassPic(cl->resp.player_class),
			y, cl->pers.netname);
		}
		else
		{
			// send the layout
			sprintf(entry,
				"xv 47 yv %i string \"%i\" "
				"xv 94 yv %i string \"%i\" "
				"xv	142 yv %i string \"%i\" "
//				"xv 172 yv %i picn \"%s\" "				
				"xv 172 yv %i string \"%s\" "				
				"xv 192 yv %i string \"%s\" ",
				y, cl->ping,  
				y, (level.framenum - cl->resp.enterframe)/600, 
				y, cl->resp.score,
				y, GetClassChar(cl->resp.player_class),
//				y, GetClassPic(cl->resp.player_class),
				y, cl->pers.netname);
		}

		if((strlen(entry) + stringlength) < 1200)
		{
			strcat(string, entry);
			stringlength = strlen(string);
		}
	}

	if(strlen(string) >= 1200)
	{
gi.dprintf("WOLFY SCOREBOARD OVERFLOW:%d\n",stringlength);
		return;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

