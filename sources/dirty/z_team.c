#include "g_local.h"
#include "m_player.h"

qboolean first_team; // For giving more random DM spots at start of match
team_handling_t teamgame;

pmenu_t null_menu[] = {
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL }
};

teams_t teamlist[] =
{
	{
                NULL
	},

	{
                NULL,
                "Vampires",
                "messiah",
                "blade",
                "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 1, 0, 0, 0
	},

	{
                NULL,
                "The Chosen",
                "sydney",
                "leeloo",
                "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 1, 0, 0, 0
	},

	{
                NULL,
                "SASDudes",
                "sas",
                "sasuc",
                "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 1, 0, 0, 0
	},

	{
                NULL,
                "Mafia",
                "actionmale",
                "invince",
                "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 1, 0, 0, 0
	},

	{
                NULL,
                "The Good",
                "marine",
                "usmc",
                "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 1, 0, 0, 0
	},

	{
                NULL,
                "The Bad",
                "marine",
                "usmc",
                "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 1, 0, 0, 0
	},

	{
                NULL,
                "The Ugly",
                "marine",
                "usmc",
                "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 1, 0, 0, 0
	},

	{
                NULL,
                "Something Wicked",
                "marine",
                "usmc",
                "", NULL, NULL, NULL, NULL, 0, 0, 0, 0, 1, 0, 0, 0
	},

	// end of list marker
	{NULL}
};

/*
===============
GetGlobalTeamByIndex
===============
*/
teams_t *GetGlobalTeamByIndex (int index)
{
        if (index == 0 || index > 8)
		return NULL;

        return &teamlist[index];
}


int TeamGoal(teams_t *team)
{
        int i;
        i = TEAM_INDEX(team);

        switch(i)
        {
                case 1:
                        return TEAM1_GOAL;
                case 2:
                        return TEAM2_GOAL;
                case 3:
                        return TEAM3_GOAL;
                case 4:
                        return TEAM4_GOAL;
                case 5:
                        return TEAM5_GOAL;
                case 6:
                        return TEAM6_GOAL;
                case 7:
                        return TEAM7_GOAL;
                case 8:
                        return TEAM8_GOAL;
        }
        return 0;
}

/*
===============
GetTeamByIndex
===============
*/
teams_t *GetTeamByIndex (int index)
{
        if ((index < 1) || (index > 8))
		return NULL;

        return &teamgame.teamlist[index];
}

/*
===============
FindTeam
===============
*/
teams_t *FindTeam (char *name)
{
        int     i;
        teams_t *team;

        team = teamgame.teamlist;
        for (i=0 ; i<=max_teams; i++, team++)
	{
                if (!team->name)
			continue;
                if (!Q_stricmp(team->name, name))
                        return team;
	}

	return NULL;
}

/*
===============
GetTeamBySize
        most - Send me back team with most <value>
        ignore_zero - if members < 1, don't include them in check
        living - defines <value> as either living members or just members
===============
*/
int GetTeamBySize (qboolean want_most, qboolean ignore_zero, qboolean living)
{
        teams_t *team;
        int     best_least, best_most, best = 0;
        int     least, most, i, current;
        int     value_check;

        least = 999;
        most = 0;
        current = rand() % max_teams + 1;

        for (i=1; i<=max_teams; i++)
        {
                current++;
                if (current > max_teams)
                        current = 1;

                team = GetTeamByIndex(current);
                if (team == NULL)
                {
                        gi.dprintf ("No team at index %i.\n", i);
                        break;
                }

                if (living == true)
                        value_check = team->living;
                else
                        value_check = team->members;

                // If we don't want non-existant teams, then continue
                if ((team->members < 1) && (ignore_zero == true))
                        continue;

                // Find the team with the most
                if (value_check > most)
                {
                        most = value_check;
                        best_most = current;
                }

                // Find the team with the least
                if (value_check < least)
                {
                        least = value_check;
                        best_least = current;
                }
        }

        // If even, pick random, else give them what they want...
        if (least == most)
        {
                for (i=1; i<=max_teams; i++)
                {
                        team = GetTeamByIndex(i);
                        if (team->members > 0)
                        {
                                best = i;
                                break;
                        }
                }
        }
        else if (want_most)
        {
                best = best_most;
                //gi.dprintf ("GetTeamBySize, wanted MOST which is %i\n", best);
        }
        else
        {
                best = best_least;
                //gi.dprintf ("GetTeamBySize, wanted LEAST which is %i\n", best);
        }
        return best;
}

/*--------------------------------------------------------------------------*/

// GRIM
/*
====================
AllyCheckOk - Checks to see if ent's ally/anti-ally does/doesn't match
        others team.
        - if teams arn't on, ignore (return true)
        - if other has no team, fail (return false)
====================
*/

qboolean AllyCheckOk (edict_t *self, edict_t *other)
{
        if (max_teams) // on valid if teams actually exist
        {
                if ((!other->client) || (!other->client->resp.team))
                        return false;
                else if (self->goal_ally)
                {
                        if (other->client->resp.team != self->goal_ally)
                                return false;
                }
                else if (self->goal_antially)
                {
                        if (other->client->resp.team == self->goal_antially)
                                return false;
                }
        }
        return true;
}
// GRIM

/*--------------------------------------------------------------------------*/

void JoinTeam (edict_t *ent, teams_t *team)
{
        if (ent->client->resp.team == team)
        {
                gi.cprintf(ent, PRINT_HIGH, "You are already on the %s\n", team->name);
		return;
	}

        if (!FloodCheck (ent))
                return;

        // Mam members...
        if ((team->maxm >= 1) && (team->members >= team->maxm))
        {
                gi.cprintf(ent, PRINT_HIGH, "%s maximum members already hit\n", team->name);
		return;
        }
        else if ((team->maxm > 0) && (team->members >= 1))
        {
                teams_t *check_team = NULL;
                float   most = 0;
                int     i;
                
                for (i=1; i<=max_teams; i++)
                {
                        check_team = GetTeamByIndex(i);
                        if (check_team == NULL)
                        {
                                gi.dprintf ("No check_team at index %i.\n", i);
                                break;
                        }

                        if (check_team == team)
                                continue;

                        if (check_team->members < 1)
                                continue;

                        if (check_team->members > most)
                                most = check_team->members;
                }
        
                if ((float)team->members >= (team->maxm * most))
                {
                        gi.cprintf(ent, PRINT_HIGH, "%s maximum members already hit\n", team->name);
                        return;
                }
        }

        if ((ent->health > 0) && (ent->movetype != MOVETYPE_NOCLIP))
        {
                ent->health = 0;

                // Force lives to 1 if alive...
                // This way, player_die removes them from the living check
                ent->client->resp.lives = 1;

                meansOfDeath = MOD_SUICIDE;
                TypeOfDamage = 0;
                player_die (ent, ent, ent, 100000, vec3_origin);
                ent->deadflag = DEAD_DEAD;
                // don't even bother waiting for death frames
                // and stop holograms being made.
                ent->s.frame = FRAME_crdeath5;
                ent->client->anim_end  = ent->s.frame;
        }

        // GRIM - FIX ME
        // If NOT dead, you would be missed by the above level check.
        // That would mean match games would remain going, even when everyone
        // was changing items. That is not good. This should fix the problem
        // for now. Improve the actual code later.
        if (turns_on && (ent->client->resp.lives > 0) && (ent->movetype != MOVETYPE_NOCLIP))
        {
                ent->client->resp.lives = 0;
                level.living--;
                MatchOverCheck ();
        }
        // GRIM                

        if (ent->deadflag && (ent->movetype != MOVETYPE_NOCLIP))
        {
                if (!ent->master)
                        CreateBodyInventory (ent);
                CopyToBodyQue (ent);
        }

        if (ent->client->resp.team && (ent->client->resp.team->members > 0))
                ent->client->resp.team->members--;

        // Do we need a check here?
        //KillTeamGoalCheck (self, self->client->resp.team);

        if (!ent->client->resp.team)
                gi.bprintf(PRINT_HIGH, "%s joined the %s\n", ent->client->pers.netname, team->name);
        else
                gi.bprintf(PRINT_HIGH, "%s changed to the %s\n", ent->client->pers.netname, team->name);

        ent->client->resp.team = team;
        team->members++;

        // Just joined, update the menu...
        UpdateChoosingMenu (ent);

	ent->svflags = 0;
        ent->deadflag = DEAD_NO;
        ent->client->resp.state = CS_NOT_READY;

        // Dirty
        AssignSkin(ent, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
        //CheckSkin(ent, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
        // Dirty

        MakeObserver (ent);
        StageTwo(ent, NULL);
}

void ForceAssignTeam(gclient_t *who)
{
        teams_t *team;
	int i;

        i = GetTeamBySize(false, false, false);
        team = GetTeamByIndex(i);
        if (team == NULL)
        {
                gi.dprintf ("Force Assign Team & No team at index %i.\n", i);
                return;
        }

        team->members++;
        who->resp.team = team;
        gi.bprintf (PRINT_HIGH, "%s was assigned to the %s\n", who->pers.netname, team->name);
        //gi.dprintf ("ForceAssignTeam %s.\n", team->name);
}

/*
================
AssignSkin - Change the players skin and/or model to that of the teams.
================
*/
void AssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
        char model_name[64];
        char skin_name[64];

        if (ent->client->resp.team == NULL)
                return;

        if (ent->client->resp.team->model == NULL)
        {
                Com_sprintf(model_name, sizeof(model_name), "%s", s);

                // GRIM - Big thanks to L-Fire for this
                // if ((p = strrchr(model_name, '/')) != NULL)
                // Do you see the difference?
                // Just stops a small annoying thing from happening...
                if ((p = strchr(model_name, '/')) != NULL)
                        p[1] = 0;
                else
                        strcpy(model_name, "male");
        }
        else
                strcpy(model_name, ent->client->resp.team->model);

        if (ent->client->resp.team->skin == NULL)
                strcpy(skin_name, "grunt");
        else
                strcpy(skin_name, ent->client->resp.team->skin);

        //gi.dprintf ("AssignSkin - model name = %s, skin_name = %s\n", model_name, skin_name);

        gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s/%s",
                ent->client->pers.netname, model_name, skin_name));
}

/*
================================
Cmd_Team_f - Change team command
================================
*/
void Cmd_Team_f (edict_t *ent)
{
        teams_t *team;
        char *t;

        if (!max_teams)
                return;

        if (ent->client->pers.spectator)
        {
                gi.cprintf (ent, PRINT_HIGH, "Spectators can't join in\n");
                return;
        }

	t = gi.args();
	if (!*t)
        {
                if (ent->client->resp.team == NULL)
                        gi.cprintf(ent, PRINT_HIGH, "You're not on a team.\n");
                else
                        gi.cprintf(ent, PRINT_HIGH, "You are on the %s\n", ent->client->resp.team->name);
                return;
	}

        team = FindTeam(t);
        if (team == NULL)
        {
		gi.cprintf(ent, PRINT_HIGH, "Unknown team %s.\n", t);
		return;
	}

        JoinTeam (ent, team);
}

/*
==================================================================
                Client info on goals section (heh).
==================================================================
*/

/*
==================
GoalDescription
==================
*/
void GoalDescription (edict_t *ent)
{
	char	string[1024];

	// send the layout
	Com_sprintf (string, sizeof(string),
                "xv 32 yv 8 picn inventory "            // background
                "xv 0 yv 24 cstring2 \"%s\" "           // team name
                "xv 0 yv 32 cstring2 \"GOAL DESCRIPTION\" "
                "xv 0 yv 40 cstring \"%s\" "           // goal description
                "xv 0 yv 164 cstring2 \"LIVES LEFT\" "
                "xv 0 yv 172 cstring \"%i/%i\" ",
                ent->client->resp.team->name,
                teamgame.goal_description[TEAM_INDEX(ent->client->resp.team)],
                ent->client->resp.lives, ent->client->resp.team->start_lives);
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


/*
==================
Cmd_MeGoal_f
==================
*/
void Cmd_MeGoal_f (edict_t *ent)
{
        if (!max_teams)
                return;

        // DTEMP
        if (ctf->value || (!goals_exist))
                return;
        // DTEMP

        if (!ent->client->resp.team)
                return;

        if (!teamgame.goal_description[TEAM_INDEX(ent->client->resp.team)])
        {
                //gi.dprintf ("Cmd_MeGoal_f - no goal_description\n");
                return;
        }

	ent->client->showinventory = false;

        if (ent->client->showhelp)
	{
		ent->client->showscores = false;
                ent->client->showhelp = false;
                return;
	}

	ent->client->showscores = true;
	ent->client->showhelp = true;
	ent->client->pers.helpchanged = 0;
        GoalDescription (ent);
	gi.unicast (ent, true);
}

void SetHelpChanged(int team_index)
{
        edict_t *player;
        teams_t *team;
        int     i;

        team = GetTeamByIndex (team_index);

        for (i = 1; i <= maxclients->value; i++)
        {
                player = &g_edicts[i];

                if (!player->inuse)
                        continue;

                if (player->movetype == MOVETYPE_NOCLIP)
                        continue;

                if (!player->client->resp.team)
                        continue;

                if (player->client->resp.team != team)
                        continue;

                player->client->pers.helpchanged = 1;
        }
}


/*--------------------------------------------------------------------------*/

char *TeamSpawnPoint(teams_t *team)
{
        int team_index;

        team_index = TEAM_INDEX(team);

        switch (team_index)
        {
                case 1:
                        return "info_player_team1";
                case 2:
                        return "info_player_team2";
                case 3:
                        return "info_player_team3";
                case 4:
                        return "info_player_team4";
                case 5:
                        return "info_player_team5";
                case 6:
                        return "info_player_team6";
                case 7:
                        return "info_player_team7";
                case 8:
                        return "info_player_team8";
        }
        return "UNKNOWN";
}

/*
================
SelectTeamSpawnPoint

go to a ctf point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
float	PlayersRangeFromSpot (edict_t *spot);
void GiveTeamARandomDMSpot(teams_t *team);

float OtherTeamSpotsRange (edict_t *spot)
{
        teams_t *spot_team;
        float   bestdistance;
	vec3_t	v;
        int     i;
        float   distance;

        bestdistance = 9999999;

        for (i=1; i<=max_teams; i++)
        {
                spot_team = GetTeamByIndex(i);
                if (spot_team == NULL)
                {
                        gi.dprintf ("GiveTeamARandomDMSpot - No team at index %i.\n", i);
                        break;
                }

                if (!spot_team->spot)
                        continue;

                VectorSubtract (spot->s.origin, spot_team->spot->s.origin, v);
                distance = VectorLength (v);

                if (distance < bestdistance)
                        bestdistance = distance;
	}

        return bestdistance;
}

edict_t *FarthestSpotPossible (void)
{
	edict_t	*bestspot;
        float   bestdistance, bestspotdistance;
	edict_t	*spot;

	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
                bestspotdistance = OtherTeamSpotsRange (spot);

                if (bestspotdistance > bestdistance)
		{
                        //gi.dprintf ("FarthestSP - bestspotdistance = %f\n", bestspotdistance);
			bestspot = spot;
                        bestdistance = bestspotdistance;
		}
	}

	if (bestspot)
		return bestspot;

        return NULL; // Crap...
}


edict_t *SelectTeamSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	*cname;

        // spawn back in the base somewhere...
        // Based on the guys score. If their score is ZERO and RR is on they
        // will still spawn back at base.  They would need it anyway :)
        if ((ent->client->resp.score > 0) && ((int)bflags->value & BF_CTF_R_RESPAWN))
        {
                if ((int)(dmflags->value) & DF_SPAWN_FARTHEST)
			return SelectFarthestDeathmatchSpawnPoint ();
		else
			return SelectRandomDeathmatchSpawnPoint ();
        }

        if (ent->client->resp.team->spot)
                return ent->client->resp.team->spot;

        cname = TeamSpawnPoint(ent->client->resp.team);

        if (!Q_stricmp(cname, "UNKNOWN"))
		return SelectRandomDeathmatchSpawnPoint();

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

        // We didn't find a team point, so go to the flag.
        if ((!count) && ctf->value)
        {
                cname = FlagClassname(ent->client->resp.team);
                spot = G_Find (NULL, FOFS(classname), cname);
                if (spot && (spot->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
                        spot = G_Find (spot, FOFS(classname), cname);

                if (!spot)
                        count = 0;
                else
                {
                        ent->pre_solid = 1;
                        ent->pre_solid_time = level.time + 3;
                        return spot;
                }
        }

        if (count < 1)
        {
                if (turns_on)
                {
                        if (!ent->client->resp.team->spot)
                        {
                                if (first_team) // Must be random
                                {
                                        GiveTeamARandomDMSpot (ent->client->resp.team);
                                        first_team = false;
                                }
                                else
                                        ent->client->resp.team->spot = FarthestSpotPossible();
                        }
                        // Crap, couldn't find one far enough away...
                        // just make sure it isn't someone else's then
                        if (!ent->client->resp.team->spot)
                                GiveTeamARandomDMSpot (ent->client->resp.team);
                                //GiveTeamASpot(ent->client->resp.team);
                }

                if (ent->client->resp.team->spot)
                        return ent->client->resp.team->spot;
                else
                        return SelectRandomDeathmatchSpawnPoint();
        }

	if (count <= 2)
		spot1 = spot2 = NULL;
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

/*
==============
GiveTeamASpot - Finds a spawn spot for the team
==============
*/
void GiveTeamARandomDMSpot(teams_t *team)
{
        edict_t *spot;
        int     selection, i;
        int     count = 0;
        teams_t *spot_team;

        spot = NULL;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
                count++;

	if (!count)
                return;

	selection = rand() % count;

	spot = NULL;
        do {
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");

                for (i=1; i<=max_teams; i++)
                {
                        spot_team = GetTeamByIndex(i);
                        if (spot_team == NULL)
                        {
                                gi.dprintf ("GiveTeamARandomDMSpot - No team at index %i.\n", i);
                                break;
                        }
                        if (spot_team->spot == spot)
                                selection++;
                }
	} while(selection--);

        team->spot = spot;
}

void GiveTeamASpot(teams_t *team)
{
        edict_t *spot;
        char    *spotname;
        int     selection;
        int     count = 0;

        spotname = TeamSpawnPoint(team);

        spot = NULL;
        while ((spot = G_Find (spot, FOFS(classname), spotname)) != NULL)
                count++;

        // No team specific spots
        if (count < 1)
        {
                //gi.dprintf ("GiveTeamASpot - no spots, spotname = %s, count = %i\n", spotname, count);
                spot = NULL;
                if (ctf->value)
                {
                        spotname = FlagClassname(team);
                        spot = G_Find (NULL, FOFS(classname), spotname);
                        //gi.dprintf ("GiveTeamASpot - CTF Check spotname = %s\n", spotname);
                }
                else
                        return;

                // Still no spot! Ah fuck it....
                if (!spot)
                {
                        //gi.dprintf ("GiveTeamASpot - no spots still!\n");
                        team->spot = NULL;
                        return;
                }
        }
        else
        {
                //gi.dprintf ("GiveTeamASpot -  spot found, spotname = %s, count = %i\n", spotname, count);
                selection = rand() % count;
                spot = NULL;
                do
                {
                        spot = G_Find (spot, FOFS(classname), spotname);
                }
                while(selection--);
        }
        team->spot = spot;
}

/*=======================================================================*/

/*
========================

SP_info_team

========================
*/
void SP_info_team (edict_t *ent)
{
        teams_t *team = NULL;

        if (ent->forteam)
                team = GetTeamByIndex(ent->forteam);

        if (team == NULL)
        {
                gi.dprintf ("SP_info_team - Couldn't find my team\n");
                G_FreeEdict (ent);
                return;
        }

        max_teams++;

        if (ent->message)
        {
                strncpy (teamgame.goal_description[ent->forteam], ent->message, sizeof(teamgame.goal_description[ent->forteam])-1);
                //gi.dprintf ("SP_info_team - copy goal_description\n");
        }

        // Dirty
        if (st.jumpgrav)
                team->jumpgrav = st.jumpgrav;
        // Dirty

        if (st.name)
                strcpy(team->name, st.name);
        if (st.nextmap)
                strcpy (team->nextmap, st.nextmap);
        if (ent->model)
                strcpy(team->model, ent->model);
        if (st.skin)
                strcpy(team->skin, st.skin);
        if (ent->ally)
                team->ally = GetTeamByIndex(ent->ally);
        if (st.starting_equipment)
                team->starting_equipment = st.starting_equipment;
        if (st.item_choices)
                team->item_choices = st.item_choices;
        if (st.weapon_choices)
                team->weapon_choices = st.weapon_choices;
        if (st.lives)
                team->start_lives = st.lives;
        if (st.minm)
                team->minm = st.minm;
        if (st.maxm)
                team->maxm = st.maxm;

        G_FreeEdict  (ent);
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

// GRIM - Multiple teams
/*QUAKED info_player_team3 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team3(edict_t *self)
{
}

/*QUAKED info_player_team4 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team4(edict_t *self)
{
}

/*QUAKED info_player_team5 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team5(edict_t *self)
{
}

/*QUAKED info_player_team6 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team6(edict_t *self)
{
}

/*QUAKED info_player_team7 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team7(edict_t *self)
{
}

/*QUAKED info_player_team8 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team8(edict_t *self)
{
}
