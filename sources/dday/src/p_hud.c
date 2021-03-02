/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_hud.c,v $
 *   $Revision: 1.27 $
 *   $Date: 2002/07/23 23:00:48 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "g_local.h"

void DDayScoreboardMessage (edict_t *ent);

/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	if (deathmatch->value || coop->value)
	{ 	// pbowens: turned on later
		ent->client->showscores = false;
		ent->client->showinventory = false;
		ent->client->showhelp = false;
//		ent->client->showpscores = false;
	}

	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;
/*-----/ PM /-----/ NEW:  Kill any flames /-----*/
	ent->burnout = 0;
/*----------------------------------------------*/

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	//ent->client->grenade_blew_up = false;
	//ent->client->grenade_time = 0;
	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// pbowens: make sure limbo_mode is set
	ent->client->limbo_mode = true;

	// add the layout

	if (deathmatch->value || coop->value)
	{
//		DeathmatchScoreboardMessage (ent, NULL);




		// pbowens: victory song
		if (!level.victory_song_played)//faf:  so it's only played once, keeps it from being blasted
		{
			//faf:  shut off all target_speakers on the map so victory song can play alone
			edict_t *t;

			t = NULL;
			
			while ((t = G_Find (t, FOFS(classname), "target_speaker")))
			{
				t->s.sound = 0;	
			}
			if (Last_Team_Winner != 99)
			{
				//faf:  playing this 3 times so it's loud enough.  It would be better to edit the wav file of course
				gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex(va("%s/victory.wav", team_list[Last_Team_Winner]->teamid)), 1, ATTN_NONE, 0);
				gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex(va("%s/victory.wav", team_list[Last_Team_Winner]->teamid)), 1, ATTN_NONE, 0);
				gi.sound (ent, (CHAN_NO_PHS_ADD|CHAN_RELIABLE), gi.soundindex(va("%s/victory.wav", team_list[Last_Team_Winner]->teamid)), 1, ATTN_NONE, 0);
			}
			level.victory_song_played = true;
		}
		// pbowens: moved to delay for overflows
//		DDayScoreboardMessage(ent);
//		gi.unicast (ent, true);
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

	if (!level.changemap)
		 level.changemap = level.mapname;

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
===================================================================================\
Death Match Scoreboard players
===================================================================================
*/
/*
void DeathmatchPlayerScore (edict_t *ent, edict_t *killer)
{
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
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		// send the layout
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i ",
			//"picn 24by24 %i %i %i %i %i %i ",
			x, y, sorted[i], cl->ping, (level.framenum - cl->resp.enterframe)/600);
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	 }
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}
*/
/*
==================
DeathmatchScoreboardMessage

==================
*/
/*
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char victory_type[10];
	char victory_adj[15];
	int points,enemy_total,team_total;
	char killstr[15],pointstr[15],lossestr[15];
	
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j;
	int		x, y;

	float	kill_p[MAX_TEAMS],	point_p[MAX_TEAMS];  // The kill and point ratios of each team
	float	kill_h,				point_h;			 // The highest of the two 
	int		kill_win,			point_win;			 // The owner of the highest ratio

	// print level name and exit rules
	string[0] = 0;
	stringlength = strlen(string);

	for(i=0;i<MAX_TEAMS;i++)
	{
		if (!team_list[i])
			continue;

		enemy_total = 1;

		// find out the enemy's stats
		for(j=0;j<MAX_TEAMS;j++)
		{
			if(j==i) 
				continue;
			if(!team_list[j]) 
				continue;
			enemy_total += team_list[j]->total;

			// Get the enemy's KILL and POINT ratios
			if (team_list[j]->need_kills)
				kill_p[j] = team_list[j]->kills / team_list[j]->need_kills;
			if (team_list[j]->need_points)
				point_p[j] = team_list[j]->score / team_list[j]->need_points;
		}
	
		team_total= (team_list[i]->total > 1) ? team_list[i]->total : 1;
		points = team_list[i]->score;

		// Get my team's KILL and POINT ratios
		if (team_list[i]->need_kills)
			kill_p[i] = team_list[i]->kills / team_list[i]->need_kills;
		if (team_list[i]->need_points)
			point_p[i] = team_list[i]->score / team_list[i]->need_points;

		// Determine which ratio is the highest of KILLs
		if (kill_p[i] > kill_p[j]) {
			kill_win	= i;
			kill_h		= kill_p[i];
		} else {
			kill_win	= j;
			kill_h		= kill_p[j];
		}

		// and POINTs
		if (point_p[i] > point_p[j]) {
			point_win	= i;
			point_h		= point_p[i];
		} else {
			point_win	= j;
			point_h		= point_p[j];
		}

		// FINALLY determine who is winning
		if (kill_win == i && point_win == i)
		{
			strcpy(victory_adj,"Decisive");
			strcpy(victory_type,"Victory");
		}
		else if (point_h > kill_h)
		{
			if (point_win == i)
			{
				// Victory Conditions
			}
			else
			{
				// Loss Conditions
			}

		}
		else if (kill_h > point_h)
		{
			if (kill_win == i)
			{
				// Victory Conditions
			}
			else
			{
				// Loss Conditions
			}
		}
		else
		{
			// Stalemate
		}


		//points=team_list[i]->score + (team_list[i]->kills/enemy_total) / ( (team_list[i]->losses) ? team_list[i]->losses:1) / ( (team_list[i]->total) ? team_list[i]->total:1);

		// it limits the losses and totals to a bottom of 1, it means this:
		//points=team_list[i]->score + (team_list[i]->kills/enemy_total) / ( (if team_list[i]->losses is true, use team_list[i]->losses, otherwise 1) / (if team_list[i]->total is true, use team_list[i]->total, otherwise 1)

		//add stuff for victory conditions here...

		if (Last_Team_Winner == 99 ||
			level.intermissiontime != level.time)
		{
			strcpy(victory_adj,"Currently");
			strcpy(victory_type,"in Battle");

		}
		else if (i == Last_Team_Winner)		
		{
			if ((team_list[i]->need_kills > 0) && team_list[i]->kills >= team_list[i]->need_kills) 
				strcpy(victory_adj,"Forceful");
			else if ((team_list[i]->need_points > 0) && team_list[i]->score >= team_list[i]->need_points) 
				strcpy(victory_adj,"Strategical");
			else
				strcpy(victory_adj,"Decisive");

			strcpy(victory_type,"Victory");
		}
		else
		{
			strcpy(victory_adj,"Definate");
			strcpy(victory_type,"Loss");
		}

		x = ( ((i+1)%2) ) ? 0 : 110;
		y = ( i>2 ) ?  64:32;

		Com_sprintf(killstr, sizeof(killstr),  "Kills:  %i",team_list[i]->kills);
		Com_sprintf(lossestr, sizeof(lossestr),"Losses: %i",team_list[i]->losses);
		Com_sprintf(pointstr, sizeof(pointstr),"Points: %i",points);
	
		
	//										x     y     teamname    x    y        kills       x     y        losses      x     y        points      x      y          vict      x    y      vict-type     x     y    
		Com_sprintf(entry,sizeof(entry),"xv %i yv %i string \"%s\" xv %i yv %i string2 \"%s\" xv %i yv %i string2 \"%s\" xv %i yv %i string2 \"%s\" xv %i yv %i string \"%s\" xv %i yv %i string \"%s\" ",
										 x     ,y ,team_list[i]->teamname,
																  x+15,  y+10,  killstr,   x+15,  y+20,    lossestr,  x+15,  y+30,   pointstr,     x+5, y+45, victory_adj,x+5,y+55,victory_type);	
		j = strlen(entry);
		if (stringlength + j > 1024) break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

		////////////////////////////!!
	return; // DISABLED //      !!
//////////////////////////      !!


	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}
*/

/*
=============
TeamStats
=============
*/
void TeamStats (edict_t *ent)
{
	int i;

	for (i=0 ; i<MAX_TEAMS ; i++)
	{
		int STAT_ICON, STAT_KILLS, STAT_POINTS;

		if (i == 0)
		{
			STAT_ICON	= STAT_TEAM0_ICON;
			STAT_KILLS	= STAT_TEAM0_KILLS;
			STAT_POINTS	= STAT_TEAM0_POINTS;
		}
		else if (i == 1)
		{
			STAT_ICON	= STAT_TEAM1_ICON;
			STAT_KILLS	= STAT_TEAM1_KILLS;
			STAT_POINTS	= STAT_TEAM1_POINTS;
		}
		else
			return; // don't crash the server

		if (team_list[i])	{
			ent->client->ps.stats[STAT_ICON] = gi.imageindex(va("teams/%s", team_list[i]->teamid));

			ent->client->ps.stats[STAT_KILLS]	= team_list[i]->kills;
			ent->client->ps.stats[STAT_POINTS]	= team_list[i]->score;
		}
		else
		{
			ent->client->ps.stats[STAT_ICON]	= 0;
			ent->client->ps.stats[STAT_KILLS]	= 0;
			ent->client->ps.stats[STAT_POINTS]	= 0;
		}
	}
}

//faf:  scoreboard from aq2
// Maximum number of lines of scores to put under each team's header.
#define MAX_SCORES_PER_TEAM 9
#define TEAM_TOP        (MAX_TEAMS+1)
#define TEAM1    0
#define TEAM2    1
        
void A_ScoreboardMessage (edict_t *ent)//, edict_t *killer)
{
	char scoreleftpic[256];   //faf:  for team dll support
    char scorerightpic[256];  //      loads background pic according to team
	char scoretopleftpic[256];
    char scoretoprightpic[256];  //faf: end

        char        string2[1400], string[1400];//, damage[50];
        gclient_t   *cl;
        edict_t     *cl_ent;
        int         maxsize = 1000, i, j, k;

                int team, len;//, deadview;
                int sorted[TEAM_TOP][MAX_CLIENTS];
                int sortedscores[TEAM_TOP][MAX_CLIENTS];
                int score, total[TEAM_TOP], totalscore[TEAM_TOP];
                int totalalive[TEAM_TOP], totalaliveprinted[TEAM_TOP];
                int stoppedat[TEAM_TOP];
//                int name_pos[TEAM_TOP];

 //               deadview = (ent->solid == SOLID_NOT ||
   //                     ent->deadflag == DEAD_DEAD ||
     //                   !team_round_going);

 //               ent->client->ps.stats[STAT_TEAM_HEADER] = gi.imageindex ("scorehead");

                total[TEAM1] = total[TEAM2] = totalalive[TEAM1] = totalalive[TEAM2] = 
                        totalscore[TEAM1] = totalscore[TEAM2] = 0;

                for (i=0 ; i<game.maxclients ; i++)
                {
                        cl_ent = g_edicts + 1 + i;
                        if (!cl_ent->inuse)
                                continue;
        
                        if (!game.clients[i].resp.team_on)
                            continue;
                        else
                            team = game.clients[i].resp.team_on->index;
        
                        score = game.clients[i].resp.score;
                        if (player_scores->value == 0)
                        {
                               j = total[team];
                        }
                        else 
                        {
                            for (j = 0; j < total[team]; j++)
                            {
                                 if (score > sortedscores[team][j])
                                        break;
                            }
                            for (k=total[team] ; k>j ; k--)
                            {
                                  sorted[team][k] = sorted[team][k-1];
                                  sortedscores[team][k] = sortedscores[team][k-1];
                            }
                        }
                        sorted[team][j] = i;
                        sortedscores[team][j] = score;
                        totalscore[team] += score;
                        total[team]++;
                        if (cl_ent->solid != SOLID_NOT &&
                                cl_ent->deadflag != DEAD_DEAD)
                                totalalive[team]++;
                } 
        
                // I've shifted the scoreboard position 8 pixels to the left in Axshun so it works
                // correctly in 320x240 (Action's does not)--any problems with this?  -FB
                // Also going to center the team names.

  //              name_pos[TEAM1] = ((20 - strlen(team_list[0]->teamname)) / 2) * 8;
    //            if (name_pos[TEAM1] < 0)
      //                  name_pos[TEAM1] = 0;
        //        name_pos[TEAM2] = ((20 - strlen(team_list[1]->teamname)) / 2) * 8;
          //      if (name_pos[TEAM2] < 0)
            //            name_pos[TEAM2] = 0;



        if (!strcmp(team_list[0]->teamid, "usa") && !strcmp(team_list[1]->teamid, "grm"))
        {
                sprintf(string,
                "xv 0  yv   0 picn scorehead  " // background header
                "yv 80 xv   0 picn scoreleft  " // background left list
                "      xv 160 picn scoreright " // background right list
                );
        }
        else
        // if teams are anything else or grm|usa, display the split graphics:
        {
                strcpy(scoreleftpic, "yv 80 xv   0 picn ");
                strcat(scoreleftpic, team_list[0]->teamid);
                strcat(scoreleftpic, "_score  ");

                strcpy(scorerightpic, "      xv 160 picn ");
                strcat(scorerightpic, team_list[1]->teamid);
                strcat(scorerightpic, "_score  ");

                strcpy(scoretopleftpic, "xv 0  yv   0 picn ");
                strcat(scoretopleftpic, team_list[0]->teamid);
                strcat(scoretopleftpic, "_score_top  ");

                strcpy(scoretoprightpic, "xv 0  xv 160 picn ");
                strcat(scoretoprightpic, team_list[1]->teamid);
                strcat(scoretoprightpic, "_score_top  ");


                sprintf(string, scoretopleftpic); //team 0
                strcat(string, scoretoprightpic); // team 1
                strcat(string, scoreleftpic); //background left list pic
                strcat(string, scorerightpic); // background right list pic
        }


                sprintf(string2, 
                        // TEAM1
//                        "if 24 xv 0 yv 8 pic 24 endif "  
  //                      "if 22 xv 32 yv 8 pic 22 endif "
                        "xv 37 yv 38 string \"%4d/%-3d\" "
                        "xv 97 yv 38 string \"%4d/%-3d\" "
                      //  "xv 90 yv 12 num 2 26 "
          //faf              "xv %d yv 66 string \"%s\" "
                        // TEAM2
    //                    "if 25 xv 160 yv 8 pic 25 endif "  
      //                  "if 22 xv 192 yv 8 pic 22 endif "
                        "xv 191 yv 38 string \"%4d/%-3d\" "
                        "xv 250 yv 38 string \"%4d/%-3d\" ",
                      //  "xv 248 yv 12 num 2 27 "
  //faf                      "xv %d yv 66 string \"%s\" ",
						team_list[TEAM1]->kills, team_list[TEAM1]->need_kills, team_list[TEAM1]->score, team_list[TEAM1]->need_points,// name_pos[TEAM1], team_list[TEAM1]->teamname,
						team_list[TEAM2]->kills, team_list[TEAM2]->need_kills, team_list[TEAM2]->score, team_list[TEAM2]->need_points);//, name_pos[TEAM2] + 160, team_list[TEAM2]->teamname);
//for testing						100,100,100,100,name_pos[TEAM1], team_list[TEAM1]->teamname,
//									100,100,100,100,name_pos[TEAM2]+160, team_list[TEAM2]->teamname);

//                        totalscore[TEAM2], total[TEAM2], name_pos[TEAM2] + 160, team_list[1]->teamname);
        
				strcat (string, string2);

/*				strcat(string,	"xv 52  yv 31 num 2 23 " // KILLS
				"xv 100 yv 31 num 3 24 " // POINTS
		
				//"if 25 xv 168 yv 8 pic 25 endif "
				"xv 208 yv 31 num 2 26 " // KILLS
			    "xv 256 yv 31 num 3 27 " // POINTS
						

//				"xv 0   yv 67 string  \" Player              Player \" "
			//	"		yv 72 string2 \" ---- -------------  ---- -------------\" "
				);

  "xv 0   yv 67 string  \" Player              Player \" "
			//	"		yv 72 string2 \" ---- -------------  ---- -------------\" "
				);
*/
				if (player_scores->value)
				{
					strcat (string,		"xv 4   yv 67 string  \"Ping  Player  Score\" ");//faf
					strcat (string,		"xv 164   yv 67 string  \"Ping  Player  Score\" ");//faf

				}
				else
				{
					strcat (string,		"xv 4   yv 67 string  \"Ping  Player\" ");//faf
					strcat (string,		"xv 164   yv 67 string  \"Ping  Player\" ");//faf
				}

//strcat (string,		"xv 0   yv 67 string  \" Ping Player         Ping Player\" "

	// pbowens: team victory pix
	if (level.intermissiontime && Last_Team_Winner != 99) {
		strcat(string, va("xv 0 yv -80 picn victory_%s ", 
			team_list[Last_Team_Winner]->teamid ));
	}


                len = strlen(string);

                totalaliveprinted[TEAM1] = totalaliveprinted[TEAM2] = 0;
                stoppedat[TEAM1] = stoppedat[TEAM2] = -1;
        
                for (i=0 ; i < (MAX_SCORES_PER_TEAM + 1) ; i++)
                {
                        if (i >= total[TEAM1] && i >= total[TEAM2])
                                break; 
        
                        // ok, if we're approaching the "maxsize", then let's stop printing members of each
                        // teams (if there's more than one member left to print in that team...)
                        if (len > (maxsize - 100))
                        {
                                if (i < (total[TEAM1] - 1))
                                        stoppedat[TEAM1] = i;
                                if (i < (total[TEAM2] - 1))
                                        stoppedat[TEAM2] = i;
                        }
                        if (i == MAX_SCORES_PER_TEAM-1) 
                        {
                                if (total[TEAM1] > MAX_SCORES_PER_TEAM)
                                        stoppedat[TEAM1] = i;
                                if (total[TEAM2] > MAX_SCORES_PER_TEAM)
                                        stoppedat[TEAM2] = i;
                        }
        
                        if (i < total[TEAM1] && stoppedat[TEAM1] == -1)  // print next team 1 member...
                        {
                                cl = &game.clients[sorted[TEAM1][i]];
                                cl_ent = g_edicts + 1 + sorted[TEAM1][i];
                                if (cl_ent->solid != SOLID_NOT &&
                                        cl_ent->deadflag != DEAD_DEAD)
                                        totalaliveprinted[TEAM1]++;
        
                                // AQ truncates names at 12, not sure why, except maybe to conserve scoreboard 
                                // string space?  skipping that "feature".  -FB
        
//                                sprintf(string+strlen(string), 
  //                                      "xv 15 yv %d string%s \"%s\" ",  
    //                                    87 + i * 8,//faf 12,
      //                                  "",//deadview ? (cl_ent->solid == SOLID_NOT ? "" : "2") : "",
        //                                game.clients[sorted[TEAM1][i]].pers.netname);

								if (player_scores->value)
								{
                                sprintf(string + strlen(string), 
                                        "xv 3 yv %d string \"%3d %-12.12s%3d\"",
                                        87 + i * 8, 
                                        game.clients[sorted[TEAM1][i]].ping,
                                        game.clients[sorted[TEAM1][i]].pers.netname,
                                        game.clients[sorted[TEAM1][i]].resp.score);//, damage);   
								}
								else
								{
                                sprintf(string + strlen(string), 
                                        "xv 3 yv %d string \"%3d %-12.12s\"",
                                        87 + i * 8, 
                                        game.clients[sorted[TEAM1][i]].ping,
                                        game.clients[sorted[TEAM1][i]].pers.netname
										);//, damage);   
								}

                        }
        
                        if (i < total[TEAM2] && stoppedat[TEAM2] == -1)  // print next team 2 member...
                        {
                                cl = &game.clients[sorted[TEAM2][i]];
                                cl_ent = g_edicts + 1 + sorted[TEAM2][i];
                                if (cl_ent->solid != SOLID_NOT &&
                                        cl_ent->deadflag != DEAD_DEAD)
                                        totalaliveprinted[TEAM2]++;
        
                                // AQ truncates names at 12, not sure why, except maybe to conserve scoreboard 
                                // string space?  skipping that "feature".  -FB
        
//                                sprintf(string+strlen(string), 
//                                        "xv 175 yv %d string%s \"%s\" ",  
  //                                      87 + i * 8,//faf 12,
    //                                    "",//deadview ? (cl_ent->solid == SOLID_NOT ? "" : "2") : "",
      //                                  game.clients[sorted[TEAM2][i]].pers.netname);
							if (player_scores->value)
							{
                                sprintf(string + strlen(string), 
                                        "xv 165 yv %d string \"%3d %-12.12s%3d\"",
                                        87 + i * 8, 
                                        game.clients[sorted[TEAM2][i]].ping,
                                        game.clients[sorted[TEAM2][i]].pers.netname,
                                        game.clients[sorted[TEAM2][i]].resp.score);//, damage);   
							}
							else
							{
								sprintf(string + strlen(string), 
                                        "xv 165 yv %d string \"%3d %-12.12s\"",
                                        87 + i * 8, 
                                        game.clients[sorted[TEAM2][i]].ping,
                                        game.clients[sorted[TEAM2][i]].pers.netname
										);//, damage);   
							}

                        }
        
                        len = strlen(string);
                }
        
                // Print remaining players if we ran out of room...
  /*              if (!deadview) // live player viewing scoreboard...
                {
                        if (stoppedat[TEAM1] > -1)
                        {
                                sprintf(string + strlen(string), "xv 0 yv %d string \"..and %d more\" ",
                                        42 + (stoppedat[TEAM1] * 8), total[TEAM1] - stoppedat[TEAM1]);
                        }
                        if (stoppedat[TEAM2] > -1)
                        {
                                sprintf(string + strlen(string), "xv 160 yv %d string \"..and %d more\" ",
                                        42 + (stoppedat[TEAM2] * 8), total[TEAM2] - stoppedat[TEAM2]);
                        }
                }
                        else // dead player viewing scoreboard...
                {*/
                        if (stoppedat[TEAM1] > -1)
                        {
                                sprintf(string + strlen(string), "xv 0 yv %d string%s \"..and %d/%d more\" ",
                                        42 + (stoppedat[TEAM1] * 8), 
                                        (totalalive[TEAM1] - totalaliveprinted[TEAM1]) ? "2" : "",
                                        totalalive[TEAM1] - totalaliveprinted[TEAM1],
                                        total[TEAM1] - stoppedat[TEAM1]);
                        }
                        if (stoppedat[TEAM2] > -1)
                        {
                                sprintf(string + strlen(string), "xv 160 yv %d string%s \"..and %d/%d more\" ",
                                        42 + (stoppedat[TEAM2] * 8), 
                                        (totalalive[TEAM2] - totalaliveprinted[TEAM2]) ? "2" : "",
                                        totalalive[TEAM2] - totalaliveprinted[TEAM2],
                                        total[TEAM2] - stoppedat[TEAM2]);
                        }
             //   }
        


        if (strlen(string) > 1300)  // for debugging...
                gi.dprintf("Warning: scoreboard string neared or exceeded max length\nDump:\n%s\n---\n", 
                                string);

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

/*
	if(!ent->client->showpscores) 
		DeathmatchScoreboardMessage (ent, ent->enemy);
	else 
		DeathmatchPlayerScore (ent, ent->enemy);
*/

	// pbowens: just do the scoreboard
//faf	DDayScoreboardMessage (ent);
	A_ScoreboardMessage(ent);//, ent->enemy);
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

	if (ent->client->menu)
		PMenu_Close(ent);

	if (!deathmatch->value && !coop->value)
		return;


	ent->client->showscores = true;

	//if scoreboard is already on...
	if (ent->client->showscores)
	{
		//but not playerscore list, turn it on...
		if(!ent->client->showpscores) {
//			ent->client->showscores = false;
			ent->client->showpscores=true;
		}
		else 
		{	//turn off the scoreboard and leave
			ent->client->showscores = false;
			ent->client->showpscores=false;
			return;
		}
		
	}

	
//	ent->client->showscores = true;
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
void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index;

	//
	// HEALTH 
	//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	//
	// SELECTED WEAPON
	//
	if (ent->client->pers.weapon && ent->client->pers.weapon->icon) 
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ( ent->client->pers.weapon->icon );
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;

	//
	// MAGAZINES / CLIPS / ROCKETS / ROUNDS
	//
	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
	{
		item	= FindItem(ent->client->pers.weapon->ammo);
		
		if (item) 
		{
			index	= ITEM_INDEX(item);
			ent->client->ps.stats[STAT_MAGS] =	ent->client->pers.inventory[index]; //anum

			// handle rounds here
			if (ent->client->p_rnd)
			{
				ent->client->ps.stats[STAT_MAGS_ICON] = (item->icon) ? gi.imageindex(item->icon) : 0;
				ent->client->ps.stats[STAT_ROUNDS] = *ent->client->p_rnd;
			}
			else // if there are no rounds, do not display icon/count
			{
				ent->client->ps.stats[STAT_MAGS_ICON] = 0;
				ent->client->ps.stats[STAT_ROUNDS] = 0;
			}

		} 
		else // if there is no ammo, then do not display anything
		{
			ent->client->ps.stats[STAT_MAGS] = 0;
			ent->client->ps.stats[STAT_MAGS_ICON] = 0;
			ent->client->ps.stats[STAT_ROUNDS] = 0;
		}
	}
	else // if there is not a weapon, then do not display anything
	{
		ent->client->ps.stats[STAT_MAGS] = 0;		
		ent->client->ps.stats[STAT_MAGS_ICON] = 0;
		ent->client->ps.stats[STAT_ROUNDS] = 0;
	}


	//
	// PICKUP MESSAGES
	//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// STAMINA
	//
	// pbowens: new stamina bar
	ent->client->ps.stats[STAT_STAMINA_CUR] = gi.imageindex( (ent->client->jump_stamina < 10.0) ? "s_00" : va("s_%i", ((int)(ent->client->jump_stamina / 10.0) * 10)) );

	//
	// TIMERS
	//
	// level_wait timer (i_dday)
	if (level.framenum < ((int)level_wait->value * 10) )
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("i_dday");
		ent->client->ps.stats[STAT_TIMER] = ((int)level_wait->value - (level.framenum / 10));
	} 	
	// forced respawn tuner (i_respcount)
	else if (level.framenum <= ent->client->forcespawn)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("i_respcount");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->forcespawn - level.framenum) / 10;
	}
	else if ((level.time < ent->leave_limbo_time) && (ent->client->limbo_mode))  //faf:  respawn timer
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("i_dday");
		ent->client->ps.stats[STAT_TIMER] = ((int)(ent->leave_limbo_time - level.time)) + 1;
	}

	// nothing at all
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON]	= 0;
		ent->client->ps.stats[STAT_TIMER]		= 0;
	}

	//
	// OBJECTIVES
	//
	if (ent->client->display_info) {
		char pic[26];
#if defined (__unix__) || defined (AMIGA)
		strcpy (pic, "objectives/");
#else
		strcpy (pic, "objectives\\");
#endif
		strcat (pic, level.mapname);

		ent->client->ps.stats[STAT_OBJECTIVE] = gi.imageindex (pic);
	} else 
		ent->client->ps.stats[STAT_OBJECTIVE] = 0;

	//
	// SNIPER CROSSHAIR
	//
	if (ent->client->crosshair && ent->client->pers.weapon && ent->client->pers.weapon->position == LOC_SNIPER)
		ent->client->ps.stats[STAT_SNIPER_SCOPE] = gi.imageindex (va("scope_%s", ent->client->resp.team_on->teamid));
	else
		ent->client->ps.stats[STAT_SNIPER_SCOPE] = 0;

	//
	// REGULAR CROSSHAIR
	//
	if (ent->client->crosshair && ent->client->pers.weapon && ent->client->pers.weapon->position != LOC_SNIPER)
		ent->client->ps.stats[STAT_CROSSHAIR] = gi.imageindex ("crosshair");
	else
		ent->client->ps.stats[STAT_CROSSHAIR] = 0;

	//
	// SELECTED ITEM
	//
	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// LAYOUTS
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || ent->client->showscores || (level.intermissiontime && (level.time >= level.intermissiontime + INTERMISSION_DELAY)) )
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

	//
	// FRAGES
	//
	//faf:  so scores appear on gamespy
	if (player_scores->value)
		ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// help icon / current weapon if not shown
	//
	//ent->client->resp.helpchanged = 0;
	if (ent->client->resp.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > STANDARD_FOV + 1)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;
 
	TeamStats(ent);


	//ent->client->ps.stats[STAT_TEAM0_INFO] = 0;
	//ent->client->ps.stats[STAT_TEAM1_INFO] = 0;
}


/*
==================
DDayScoreboardMessage
 based off of CTF scoreboard
==================
*/
void DDayScoreboardMessage (edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k;//, n;
	int		sorted[2][MAX_CLIENTS];
	int		sortedscores[2][MAX_CLIENTS];
	int		score, total[2];
	int		last[2];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

    char scoreleftpic[256];   //faf:  for team dll support
    char scorerightpic[256];  //      loads background pic according to team
	char scoretopleftpic[256];
    char scoretoprightpic[256];  //faf: end

	// sort the clients by team and score
	total[0]		= total[1] = 0;
	last[0]			= last[1] = 0;

	for (i=0 ; i < game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || cl_ent->flyingnun)
			continue;

		if (game.clients[i].resp.team_on)
			team = game.clients[i].resp.team_on->index;
		else
			continue; // no team

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
		total[team]++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

        //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
        //faf:  team dll support-> loads background scoreboard pics according to team

        //faf:  this displays the original usa|grm scoreboard pics if team 0 is usa & team 1 is grm.
        //      Not really necessary except to keep the original scoreboard pics
        if (!strcmp(team_list[0]->teamid, "usa") && !strcmp(team_list[1]->teamid, "grm"))
        {
                sprintf(string,
                "xv 0  yv   0 picn scorehead  " // background header
                "yv 80 xv   0 picn scoreleft  " // background left list
                "      xv 160 picn scoreright " // background right list
                );
        }
        else
        // if teams are anything else or grm|usa, display the split graphics:
        {
                strcpy(scoreleftpic, "yv 80 xv   0 picn ");
                strcat(scoreleftpic, team_list[0]->teamid);
                strcat(scoreleftpic, "_score  ");

                strcpy(scorerightpic, "      xv 160 picn ");
                strcat(scorerightpic, team_list[1]->teamid);
                strcat(scorerightpic, "_score  ");

                strcpy(scoretopleftpic, "xv 0  yv   0 picn ");
                strcat(scoretopleftpic, team_list[0]->teamid);
                strcat(scoretopleftpic, "_score_top  ");

                strcpy(scoretoprightpic, "xv 0  xv 160 picn ");
                strcat(scoretoprightpic, team_list[1]->teamid);
                strcat(scoretoprightpic, "_score_top  ");


                sprintf(string, scoretopleftpic); //team 0
                strcat(string, scoretoprightpic); // team 1
                strcat(string, scoreleftpic); //background left list pic
                strcat(string, scorerightpic); // background right list pic
        }
//  xxxxxxxxx faf:end

//	sprintf(string,
//		"xv 0  yv   0 picn scorehead  " // background header
//		"yv 80 xv   0 picn scoreleft  " // background left list
//		"      xv 160 picn scoreright " // background right list
		
		//"if 22 xv 8 yv 8 pic 22 endif "
strcat(string,	"xv 52  yv 31 num 2 23 " // KILLS
		"xv 100 yv 31 num 3 24 " // POINTS
		
		//"if 25 xv 168 yv 8 pic 25 endif "
		"xv 208 yv 31 num 2 26 " // KILLS
	    "xv 256 yv 31 num 3 27 " // POINTS
		
		"xv 0   yv 67 string  \" Ping Player         Ping Player\" "
	//	"		yv 72 string2 \" ---- -------------  ---- -------------\" "
	);

	// pbowens: team victory pix
	if (level.intermissiontime && Last_Team_Winner != 99) {
		strcat(string, va("xv 0 yv -80 picn victory_%s ", 
			team_list[Last_Team_Winner]->teamid ));
	}
	
	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total[0] && i >= total[1])
			break; // we're done

		// set up y
		sprintf(entry, "yv %d ", 84 + i * 8);
		if (maxsize - len > strlen(entry)) {
			strcat(string, entry);
			len = strlen(string);
		}

		// left side
		if (i < total[0]) {
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];

			sprintf(entry+strlen(entry),
			"xv 0 string \"  %3d %-13.13s\" ",
//			(cl_ent == ent) ? "string2" : "string",
//			(level.framenum - cl->resp.enterframe)/600, 
			(cl->ping > 999) ? 999 : cl->ping, 
			va("%s%s",(cl->resp.mos == MEDIC) ? "+" : "", cl->pers.netname));

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

			sprintf(entry+strlen(entry),
			"xv 160 string \"  %3d %-13.13s\" ",
//			(cl_ent == ent) ? "string2" : "string",
//			(level.framenum - cl->resp.enterframe)/600, 
			(cl->ping > 999) ? 999 : cl->ping, 
			va("%s%s",(cl->resp.mos == MEDIC) ? "+" : "", cl->pers.netname));


			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
				last[1] = i;
			}
		}
	}
/*
	// put in spectators if we have enough room
	if (last[0] > last[1])
		j = last[0];
	else
		j = last[1];
	j = (j + 2) * 8 + 84;

	k = n = 0;
	if (maxsize - len > 50) {
		for (i = 0; i < maxclients->value; i++) {
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];

			if (!cl->pers.netname)
				continue;

			if (cl->resp.team_on)
				continue;

			if (!k) {
				k = 1;
				sprintf(entry, "xv 0 yv %d string2 \"No Team\" ", j);
				strcat(string, entry);
				len = strlen(string);
				j += 8;
			}

			sprintf(entry+strlen(entry),
			"xv %d yv %d string \"  %3d %-13.13s\" ",
			(n & 1) ? 164 : 0, // x
			j, // y
//			(cl_ent == ent) ? "string2" : "string",
//			(level.framenum - cl->resp.enterframe)/600, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);


			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
			}
			
			if (n & 1)
				j += 8;
			n++;
		}
	}
*/
	if (total[0] - last[0] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			84 + (last[0]+1)*8, total[0] - last[0] - 1);
	if (total[1] - last[1] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
			84 + (last[1]+1)*8, total[1] - last[1] - 1);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}
