#include "g_local.h"

void DisplayCenterMessageAll(char *message)
{
        edict_t *player;
        int     i = 0;

	for (i = 1; i <= maxclients->value; i++)
        {
		player = &g_edicts[i];

                if (!player->inuse)
			continue;
                gi.centerprintf (player, "%s\n", message);
	}
}

/*======================================================================*/

qboolean MatchInProgress(void)
{
        if ((level.match_state == MATCH_START) || (level.match_state == MATCH_DURING))
                return true;
                
        return false;
}

/*
==============
MatchOverCheck

Reset flags, corpses, equipment etc, and then put all the players in.
==============
*/
void MatchOutOfTime(void)
{
        char    message[128];

        strcpy(message, "Match time limit up\n No one wins!\n (coz I'm too lazy to put a \n win calc in\n Feel free to complaing at \n anyone but me) \n");
        strcpy(level.message, message);
        level.match_time = level.time + 3;

        // TEMP - Skip finished coz I don't want display of winners...
        level.match_state = MATCH_CHECKING;
        //level.match_state = MATCH_FINISHED;
}

qboolean AllDeadCheckForAllGoals(void)
{
        if (!goals_exist)
                return false;

        if (level.living < 1)
                return false;

        return true;
}

void MatchOverCheck(void)
{
        int             calc, i;

        if (!MatchInProgress())
                return;

        if (ctf->value && (level.living > 0))
        {
                if (teamgame.last_score_team == NULL)
                        return;
                // Dirty - One flag style (paranoid)
                //if (ctf->value > 1)
                //{
                        i = TeamGoal(teamgame.last_score_team);
                        calc = level.participants - i;

                        if (teamgame.last_score_team->score < calc)
                        {
                                teamgame.last_score_team = NULL;
                                return;
                        }
                //}
                // Dirty

                level.match_time = level.time + 3.5;
                level.match_state = MATCH_FINISHED;
                // Paranoid
        }

        // CTF - See above and FIX
        // Teams games - One team
        // Otherwise - One person
        if (level.living > 1)
                return;

        if (AllDeadCheckForAllGoals())
                return;

        //gi.dprintf ("AllDeadCheckForAllGoals -> goals done\n");

        //gi.dprintf ("MatchOverCheck -> match is finished\n");
        level.match_time = level.time + 3.5;
        level.match_state = MATCH_FINISHED;
}

void MatchWinner(void)
{
        edict_t *player;
        char    message[128];
        char    lvl_mes[256];
        teams_t         *team;
        int             calc, i, alive;
        qboolean        has_message = false;

        alive = 0;

        if (ctf->value)
        {
                if ((level.living < 1) && ((int)bflags->value & BF_CTF_TOUCH))
                        strcpy(message, "       No living teams! No one wins!\n");
                else
                {
                        /*
                        if (((int)bflags->value & BF_CTF_TOUCH) && (level.living < 2) && something.....
                        {
                                Com_sprintf (message, sizeof(message), "One team left\n Go touch the flags!");
                                DisplayCenterMessageAll(message);
                                return;
                        }
                        */

                        if (teamgame.last_score_team == NULL)
                                return;

                        // Dirty
                        //if (ctf->value > 1)
                        if (ctf->value)
                        // Dirty
                        {
                                i = TeamGoal(teamgame.last_score_team);
                                calc = level.participants - i;

                                if (teamgame.last_score_team->score >= calc)
                                {
                                        strcpy(message, "     ");
                                        strcat(message, teamgame.last_score_team->name);
                                        strcat(message, " win!\n");
                                }
                                else
                                {
                                        teamgame.last_score_team = NULL;
                                        return;
                                }
                        }
                        else
                        {
                                strcpy(message, "     ");
                                strcat(message, teamgame.last_score_team->name);
                                strcat(message, " win!\n");
                        }
                        // Paranoid

                        AdjustTeamScore(teamgame.last_score_team, 1, true);

                        strcpy(lvl_mes, "This round has finished\n");
                        strcat(lvl_mes, message);
                        strcpy(level.message, lvl_mes);

                        //gi.dprintf ("MatchWinner check, match is finished\n");
                        return;
                }
        }

        if (max_teams)
        {
                // Paranoid
                if (teamgame.last_score_team)
                {
                        strcpy(message, "     ");
                        strcat(message, teamgame.last_score_team->name);
                        // Dirty
                        //AdjustTeamScore(teamgame.last_score_team, 1, true);
                        if (teamgame.last_score_team->ally)
                        {
                                //AdjustTeamScore(teamgame.last_score_team->ally, 1, true);
                                strcat(message, " and the ");
                                strcat(message, teamgame.last_score_team->ally->name);
                                strcat(message, " won!\n");
                        }
                        else
                                strcat(message, " win!\n");
                        // Dirty
                }
                else if (level.living < 1)
                // Paranoid
                        strcpy(message, "     No living teams. No one wins!\n");
                else if (!goals_exist)
                {
                        for (i = 1; i <= max_teams; i++)
                        {
                                team = GetTeamByIndex(i);
                                if (team == NULL)
                                {
                                        gi.dprintf ("MatchWinner - No team at index %i.\n", i);
                                        break;
                                }

                                if (team->members < 1)
                                        continue;

                                if (team->living > 0)
                                        break;
                        }

                        if (!team)
                                strcpy(message, "Found no team!\n");
                        else if (team->score < 1)
                        {
                                strcpy(message, "     ");
                                strcat(message, "Only team alive killed nothing\n     No winner!\n");
                        }
                        else
                        {
                                strcpy(message, "     ");
                                strcat(message, team->name);
                                strcat(message, " win!\n");
                                AdjustTeamScore(team, 1, true);
                        }
                }
                has_message = true;
        }
        else if (level.living < 1)
        {
                strcpy(message, "No one left alive to win...\n");
                has_message = true;
        }
        else
        {
                for (i = 1; i <= maxclients->value; i++)
                {
                        player = &g_edicts[i];

                        if (!player->inuse)
                                continue;

                        if ((player->client->resp.state < CS_PLAYING) || (player->deadflag))
                                continue;

                        if (player->kills_in_a_row > 0)
                        {
                                strcpy(message, "        ");
                                strcat(message, player->client->pers.netname);
                                strcat(message, " wins!\n");
                                AdjustScore(player, 1, true);
                        }
                        else
                                strcpy(message, "        No one alive with a kill\n No one wins\n");
                }
                has_message = true;
        }

        strcpy(lvl_mes, "This round is over.\n");
        if (has_message)
                strcat(lvl_mes, message);
        strcpy(level.message, lvl_mes);
        teamgame.last_score = level.time;
}

/*
==============
SetupMatch

Reset flags, corpses, equipment etc, and then put all the players in.
==============
*/


// Paranoid - General reset function for other misc functions 
void general_reset (edict_t *ent)
{
        ent->nextthink = 0; // Dirty
        ent->solid = SOLID_BSP;
        ent->svflags &= ~SVF_NOCLIENT;
        VectorClear (ent->velocity);

        if (ent->die)
        {
                VectorCopy(ent->corpse_pos, ent->s.origin);
                VectorClear (ent->velocity);
                ent->takedamage = DAMAGE_YES;
                ent->health = ent->max_health;
        }
}
// Paranoid


void SetupMatch (void)
{
	edict_t	*ent;
        teams_t *team;
        int     i, living;
        int     participants, n;

        // FIX ME - Option for Deathmatch Match games?
        // int lives;
        // lives = lives;

        //UnHideAllSpots(); // Dirty - Spawn points don't show up anyway
        memset(level.found_goals, 0, sizeof(level.found_goals));
        memset(level.total_goals, 0, sizeof(level.total_goals));

        if (max_teams)
        {
                first_team = true;

                for (i=1; i<=max_teams; i++)
                {
                        team = GetTeamByIndex(i);
                        if (team == NULL)
                        {
                                gi.dprintf ("SetupMatch - No team at index %i.\n", i);
                                break;
                        }
                        team->living = 0;
                        team->score = 0;
                        team->spot = NULL;
                }
        }

        living = 0;
        participants = 0;

        // Go through everything and get rid of what shouldn't be around
        // anymore, respawn that which should come back etc.
	ent = &g_edicts[0];
	for (i=0 ; i<globals.num_edicts ; i++, ent++)
	{
		if (!ent->inuse)
			continue;

                if (i > 0)
		{
                        // Start the players...
                        if (i <= maxclients->value)
                        {
                                if (ent->client->resp.state > CS_NOT_READY)
                                {
                                        ent->client->resp.state = CS_PLAYING;

                                        // Number of times we can respawn...
                                        if (ent->client->resp.team)
                                                ent->client->resp.lives = ent->client->resp.team->start_lives;
                                        else // FIX ME - Option for Deathmatch Match games?
                                                ent->client->resp.lives = 1;

                                        ent->pre_solid = 1;
                                        ent->pre_solid_time = level.time + 3;
                                        // Dirty
                                        if (ent->client->ps.fov != 90)
                                                ZoomOff(ent);
                                        // Dirty
                                        ThrowEmIn(ent);
                                        if (ent->client->resp.team)
                                                ent->client->resp.team->living++;
                                        living++;
                                        participants++;
                                }
                                continue;
                        }
                        // Hide any corpses...
                        else if (i <= (maxclients->value + BODY_QUEUE_SIZE))
                        {
                                ent->solid = SOLID_NOT;
                                ent->svflags |= SVF_NOCLIENT;

                                // If we have a a body_inv, waste it...
                                if (ent->master)
                                {
                                        G_FreeEdict (ent->master);
                                        ent->master = NULL;
                                }

                                // If we have a trap on us, waste it too...
                                if (ent->trapping)
                                {
                                        G_FreeEdict (ent->trapping);
                                        ent->trapping = NULL;
                                }

                                gi.linkentity (ent);
                                continue;
                        }
		}

                // Remove gibs, skulls etc...
                if (ent->deadflag && (!ent->client))
                {
                        G_FreeEdict (ent);
			continue;
                }

                // Remove any dropped items/techs,
                // respawn flags and other items...
                if (ent->item)
                {
                        if (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))
                        {
                                if (ent->item->pickup == CTFPickup_Flag)
                                        CTFDropFlagThink(ent);
                                else
                                        G_FreeEdict (ent);
                        }
                        else if (ent->item->flags & IT_RAND)
                                G_FreeEdict (ent);
                        else if (ent->solid == SOLID_NOT)
                                DoRespawn (ent);
                }

                if (ent->flags & FL_RESPAWN)
                {
                        if (ent->restart)
                                ent->restart (ent);
                        else
                                ent->flags &= ~FL_RESPAWN;
                }
	}

        if (max_teams)
        {
                living = 0;
                participants = 0;
                team = NULL;
                for (i=1; i<=max_teams; i++)
                {
                        team = GetTeamByIndex(i);
                        if (team == NULL)
                        {
                                gi.dprintf ("SetupMatch - No team at index %i.\n", i);
                                break;
                        }

                        if (team->living > 0)
                        {
                                living++;
                                if (ctf->value)
                                {
                                        n = TeamGoal(team);
                                        participants += n;
                                }
                                else
                                        participants++;
                        }
                }
        }

        level.living = living;
        level.participants = participants;
        
        /*
        if (level.living < 2) // someone died or something...stay in checking
        {       // basically, something FUCKED UP!
                level.match_state = MATCH_CHECKING;
                level.match_time = level.time + 5;
                gi.bprintf (PRINT_MEDIUM,"SetupMatch - Ok, so why isn't there enough people ready now?\n");
                return;
        }
        */

        // HideAllSpots(); // Dirty - Spawn points don't show up anyway

        // Dirty
        SetupSpecialsSpawn();

        // FIX ME - Include this mode?
        //if ((ctf->value) && (ctf->value < 2))
        //        SingleFlagSetup();
        // Dirty

        level.match_state = MATCH_START; // Begin match
        level.match_time = level.time + 3;

        teamgame.last_score = level.time - 5;
        teamgame.last_score_team = NULL;
        //gi.dprintf ("SetupMatch - participants %i, living %i\n", participants, living);
}

/*
==============
CheckMatchStart

Checks to see if a minimum amount of ppl are ready,
teams are even etc, before it will start.
==============
*/
void PracticeReset(void);

void CheckMatchStart(void)
{
        edict_t         *player;
        teams_t         *team = NULL;
        char            *message;
        char            display[128];
        int             i, j, ppl_ready, ppl;
        int             tcount[10];
        qboolean        start = false;

        message = ""; // Oops, fixes bug, heh.

        for (i = 0; i < 10; i++)
                tcount[i] = 0;

        player = NULL;
        ppl_ready = ppl = 0;
        for (i = 1; i <= maxclients->value; i++)
        {
                player = &g_edicts[i];

                if ((!player->inuse) || (!player->client))
                        continue;

                // Count ppl for initial check...
                ppl++;

                // If player isn't ready, forget them...
                if (player->client->resp.state == CS_NOT_READY)
                        continue;

                // Double check (Shouldn't make it, but you never know...)
                if (max_teams && (!player->client->resp.team))
                        continue;

                ppl_ready++;

                if (player->client->resp.team)
                {
                        j = TEAM_INDEX(player->client->resp.team);
                        tcount[j]++;
                }
        }

        // Do a couple of quick checks.
        // Coz why do anymore if there simply isn't enough ppl? hmm?
        if (ppl < 2) // not enough ppl
        {
                if (ppl == 1)
                        gi.bprintf (PRINT_MEDIUM,"Not enough people. Next check in 20 seconds\n");
                level.match_time = level.time + 20;
                return;
        }

        if (ppl_ready < 2) // not enough ppl
        {
                if (ppl_ready == 0)
                        gi.bprintf (PRINT_MEDIUM,"No one ready. Next check in 20 seconds\n");
                else
                        gi.bprintf (PRINT_MEDIUM,"Not enough people ready. Next check in 20 seconds\n");
                level.match_time = level.time + 20;
                return;
        }

        // Calc stuff just for teams...
        if (max_teams)
        {
                int     least, most, max_reqs, not_minm, min_reqs;
                int     calc, joined, mustbejoined;

                joined = mustbejoined = 0;
                min_reqs = max_reqs = not_minm = 0;
                most = 0;
                least = 9999;
        
                for (i=1; i<=max_teams; i++)
                {
                        team = GetTeamByIndex(i);
                        if (team == NULL)
                        {
                                gi.dprintf ("CheckMatchStart - No team at index %i.\n", i);
                                break;
                        }

                        if (goals_exist)
                                mustbejoined++;

                        if (team->members < 1)
                                continue;

                        joined++;

                        if (team->maxm)
                                max_reqs++;

                        if (team->minm)
                                min_reqs++;

                        if (team->minm > team->members)
                                not_minm++;

                        if ((tcount[i] > most) && (team->minm < most))
                                most = tcount[i];

                        if ((team->maxm > least) && (tcount[i] > 0) && (tcount[i] < least))
                                least = tcount[i];
                }

                //gi.dprintf ("CheckMatchStart - least %i, most %i, ppl_ready %i, joined %i, not_minm %i\n", least, most, ppl_ready, joined, not_minm);

                if ((joined < 2) || (mustbejoined && (joined < mustbejoined)))
                        message = "Not enough teams joined\n";
                else if (not_minm > 0)
                {
                        if (not_minm == 1)
                                message = "Minimum members for one team haven't been met\n";
                        else if (not_minm == 2)
                                message = "Minimum members for a couple of teams haven't been met\n";
                        else
                                message = "Minimum members for some teams haven't been met\n";
                }
                else
                {
                        // All teams (that need to be) are even...start
                        if (((most == 0) && (least == 9999)) || (least == most))
                                start = true;
                        else
                        {
                                if (ppl_ready > 18)
                                        i = 5;
                                else if (ppl_ready > 14)
                                        i = 4;
                                else if (ppl_ready > 10)
                                        i = 3;
                                else if (ppl_ready > 6)
                                        i = 2;
                                else if (ppl_ready > 2)
                                        i = 1;

                                calc = 0;
                                calc = most - least;
                                if (calc <= i)
                                        start = true;
                                else if (max_reqs || min_reqs)
                                        message = "Non-min/max teams uneven. We wait\n";
                                else
                                        message = "Teams uneven. We wait\n";
                        }
                }
        }
        else if (ppl_ready >= 2)
                start = true; // FIX ME - muck around with this value?
        else
                message = "Not enough people ready";

        if (start)
        {                                              
                if (teamgame.last_score == 0)
                {
                        Com_sprintf (display, sizeof(display), "First round will begin in 10 seconds...\n");
                        gi.bprintf (PRINT_MEDIUM,"%s", display);
                        level.match_time = level.time + 10;
                }
                else
                        level.match_time = level.time + 1;
                level.match_state = MATCH_PRE_START;
        }
        else
        {                                              
                level.match_time = level.time + 20;
                Com_sprintf (display, sizeof(display), "%s \n Next check in 20 seconds", message);
                gi.bprintf (PRINT_MEDIUM,"%s\n", display);
                // FIX ME - Where should this go? if at all?
                // PracticeReset();
        }
}

void PracticeReset(void)
{
        edict_t         *player;
        int             i;

        //UnHideAllSpots(); // Dirty - Spawn points don't show up anyway
        for (i = 1; i <= maxclients->value; i++)
        {
                player = &g_edicts[i];

                if (!player->inuse)
                        continue;

                if (player->client->resp.state == CS_PLAYING)
                        player->client->resp.state = CS_READY;
                player->client->resp.lives = -1;

                //if ((player->movetype == MOVETYPE_NOCLIP) && (player->client->resp.state == CS_READY))
                //        ThrowEmIn(player);
        }
}
