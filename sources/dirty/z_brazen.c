#include "g_local.h"

/*
=================

BraZen generic/often used functions and shit.
You'll get the idea.

=================
*/
void BraZenIniInit(void)
{
        teams_t *team;
        FILE    *ini_file;
        char    line[8][32];
        char    current_team[8][8];
        int     i, n;

        // Dirty
        if ((ini_file = fopen("dirty/dirty.ini", "r")))
        {       // If we open the file "dirty.ini"
        // Dirty
                i = 0;
                while (i < 8)
                {
                        i++;
                        team = GetGlobalTeamByIndex(i);
                        
                        strcpy(current_team[i], va("[team%i]", i));

                        //gi.dprintf("TeamInit - Team Section = %s\n", current_team);
                        do
                        {
                                fscanf(ini_file, "%s", line[i]);
                        } while (!feof(ini_file) && (Q_stricmp(line[i], current_team[i]) != 0));
                
                        if (feof(ini_file))
                        {
                                continue;
                                //gi.dprintf("============================================\n");
                                //gi.dprintf("= Error - No %s section in BraZen.ini =\n", current_team);
                                //gi.dprintf("============================================\n");
                        }
                        else
                        {       // Found Team heading.
                                n = 1;
                                while ((!feof(ini_file)) && (n < 4))
                                {
                                        fscanf(ini_file, "%s", line[i]);

                                        if (Q_stricmp(line[i], "###") == 0)
                                                break;

                                        if (n == 1)
                                        {
                                                strcpy(team->name, line[i]);
                                                //gi.dprintf("TeamInit - team->name = %s\n", line);
                                        }
                                        else if (n == 2)
                                        {
                                                strcpy(team->model, line[i]);
                                                //gi.dprintf("TeamInit - team->model = %s\n", line);
                                        }
                                        else if (n == 3)
                                        {
                                                strcpy(team->skin, line[i]);
                                                //gi.dprintf("TeamInit - team->skin = %s\n", line);
                                        }
                                        n++;
                                }
                        }
                }
                //close the file
                fclose(ini_file);
        }
}

/*=====================================================================*/

void BraZenInit(void)
{
        /* FIX ME - Add back in
        if (bhole->value > 64)
                gi.cvar_set("bhole", "64");
        else if (bhole->value < 0)
                gi.cvar_set("bhole", "0");

        if (blood->value > 64)
                gi.cvar_set("blood", "64");
        else if (blood->value < 0)
                gi.cvar_set("blood", "0");

        // Total bullet hole and blood limit...
        // This should hopefully stop overflows.
        if ((blood->value + bhole->value) > 80)
        {
                gi.cvar_set("blood", "40");
                gi.cvar_set("bhole", "40");
        }
        */

        // Auto force neccessaries
        //gi.dprintf("------Forcing deathmatch------\n");
        gi.cvar_set("deathmatch", "1");

        if (ctf->value || teams->value)
        {
                // Limit to 7 coz more don't fit menu right...
                if ((int)teams->value > 7)
                        gi.cvar_set("teams", "7");

                // Even CTF requires at least 2 teams
                if ((int)teams->value < 2)
                        gi.cvar_set("teams", "2");
        }

        // Dirty
        if (((int)teamkill->value > 1) && ((int)teamkill->value < 6))
                gi.cvar_set("teams", "6");
        // Dirty

        BraZenIniInit();

        memset(&teamgame, 0, sizeof(teamgame));
}

/*====================================================================*/

/*
=================
CheckBox - Just like killbox, only it doesn't.
=================
*/
qboolean CheckBox (edict_t *ent)
{
        edict_t *player;
        int     solids = 0;
        int     i = 0;

	for (i = 1; i <= maxclients->value; i++)
        {
		player = &g_edicts[i];
                if (!player->inuse)
			continue;
                if (player == ent)
			continue;
                if (InRange (ent, player, 64))
                {
                        solids++;
                        break;
                }
	}

        if (solids > 0)
                return false;
        else
                return true;
}


/*
=================
InRange - Checks if the two entities are within a certain distance
=================
*/
qboolean InRange (edict_t *ent1, edict_t *ent2, float range)
{
        vec3_t  dir;
        float   dist;

        VectorSubtract(ent1->s.origin, ent2->s.origin, dir);
        dist = VectorLength(dir);
        if (dist < range)
                return true;

        return false;
}


/*
=================
AdjustTeamScore - Called everytime a team gets a point/goal completed
=================
*/
void AdjustTeamScore(teams_t *team, int change, qboolean force)
{
        if (change == 0)
                return;

        if ((level.match_state != MATCH_DURING) && (!force))
                return;

        //if (!GameOn())
        //        return;

        team->total_score += change;
        teamgame.last_score_team = team;

        if ((goal_limit->value) && max_teams)
        {
                if (team->total_score >= goal_limit->value)
                {
                        gi.bprintf (PRINT_HIGH, "Goal limit hit.\n");
                        EndDMLevel ();
                }
        }
}

/*
=================
AdjustScore - Called everytime a player gets a frag
=================
*/
void AdjustScore(edict_t *player, int change, qboolean force)
{
        if (change == 0)
                return;

        if ((level.match_state != MATCH_DURING) && (!force))
                return;

        if (turns_on)
                player->kills_in_a_row += change;

        player->client->resp.score += change;

        // Paranoid (!ctf->value)
        if ((player->client->resp.team) && (change > 0) && (!ctf->value))
        // Paranoid
                player->client->resp.team->score += change;

        if (fraglimit->value && deathmatch->value)
	{
                if (player->client->resp.score >= (int)fraglimit->value)
                {
                        gi.bprintf (PRINT_HIGH, "Fraglimit hit.\n");
                        EndDMLevel ();
                }
	}
}

/*
=================
stuffcmd - command to client console
=================
*/
void stuffcmd(edict_t *ent, char *s) 	
{
        gi.WriteByte (svc_stufftext);
	gi.WriteString (s);
        gi.unicast(ent, true);
}

/*
=================
MakeObserver - Make this guy invisible and not solid.
=================
*/
void MakeObserver (edict_t *ent)
{
        ent->deadflag = DEAD_NO;
        ent->movetype = MOVETYPE_NOCLIP;
        ent->solid = SOLID_NOT;
        ent->svflags |= SVF_NOCLIENT;
        ent->client->ps.gunindex = 0;
        ent->client->weaponstate = WEAPON_ACTIVATING; // Stops actions...
        gi.linkentity (ent);
}

/*
=================
ThrowEmIn - Make this guy visible, solid and then start them
=================
*/
void ThrowEmIn (edict_t *ent)
{
	ent->svflags &= ~SVF_NOCLIENT;
	PutClientInServer (ent);
        // Dirty - More is less
        //ent->s.event = EV_PLAYER_TELEPORT;
        // Dirty
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
}

void HideSpot(edict_t *ent)
{
        ent->solid = SOLID_NOT;
        ent->svflags |= SVF_NOCLIENT;
        gi.linkentity (ent);
}

void UnHideSpot(edict_t *ent)
{
	ent->svflags &= ~SVF_NOCLIENT;
        ent->solid = SOLID_BBOX;
        gi.linkentity (ent);
}

void HideAllSpots(void)
{
        edict_t *spot;
	spot = NULL;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
                if (spot->solid == SOLID_BBOX)
                        HideSpot(spot);
	}
}

void UnHideAllSpots(void)
{
        edict_t *spot;
	spot = NULL;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
                if (spot->solid == SOLID_NOT)
                        UnHideSpot(spot);
	}
}

edict_t *SelectRandomDMSpot (void)
{
        edict_t *spot;
        int     selection;
        int     count = 0;

        spot = NULL;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
                count++;

        if (count < 1)
        {
                spot = G_Find (spot, FOFS(classname), "info_player_start");
                if (!spot)
                {
                        //gi.dprintf ("SelectRandomDMSpot - Someone fucked up...no spots found\n");
                        return NULL;
                }
        }

        selection = rand() % count;

	spot = NULL;
        do {
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	} while(selection--);

	return spot;
}


/*=====================================================================*/

void BrazenCmdsInit(edict_t *ent)
{
        // Set up Alias' keep to ONE HUGE line???
        stuffcmd(ent, "alias +action \"action_on; +speed\"; alias -action \"action_off; -speed\"; alias +attack2 +use; alias -attack2 -use; set old_crosshair_dude $crosshair;set b4_zoom_dude $sensitivity\n");
}

/*=====================================================================*/

pmenu_t stage3_menu[] = {
        // Dirty
        { "*Dirty",                     PMENU_ALIGN_CENTER, NULL, NULL },
        // Dirty
        { NULL,                         PMENU_ALIGN_CENTER, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL }
};

void UpdateStageThreeMenu(edict_t *ent)
{
        stage3_menu[3].text = "Currently, well, nothing";
        stage3_menu[4].text = "*GO BACK FUCK HEAD!";
        stage3_menu[4].SelectFunc = OpenChoosingMenu;
}

void OpenStageThreeMenu(edict_t *ent)
{
        if (ent->client->menu)
                PMenu_Close(ent);

        UpdateStageThreeMenu(ent);
        PMenu_Open(ent, stage3_menu, -1, sizeof(stage3_menu) / sizeof(pmenu_t));
}

void StageThree(edict_t *ent)
{
        if (turns_on)
        {
                ent->client->resp.state = CS_READY;
                if (!MatchInProgress())
                        ThrowEmIn(ent);
                else
                        gi.bprintf(PRINT_HIGH, "%s is ready...\n", ent->client->pers.netname);
        }
        else
        {
                gi.bprintf(PRINT_HIGH, "%s has joined in\n", ent->client->pers.netname);
                ent->client->resp.state = CS_PLAYING;
                ThrowEmIn(ent);
        }

        if (ent->client->menu)
                PMenu_Close(ent);

        // DTEMP - Display goal for this team?
        if (goals_exist)
                Cmd_MeGoal_f (ent);
        // DTEMP
}

/*=====================================================================*/
void StageTwo(edict_t *ent, pmenu_t *p)
{
        // Dirty - Allow weapon choice in DM now too...
        if ((int)bflags->value & BF_CHOOSE_STUFF)
        {
                OpenChooseWeaponMenu(ent);
                return;
        }
        // Dirty
        else if (turns_on)
        {
                ent->client->resp.state = CS_READY;
                if (!MatchInProgress())
                        ThrowEmIn(ent);
                else
                        gi.bprintf(PRINT_HIGH, "%s is ready...\n", ent->client->pers.netname);
        }
        else
        {
                gi.bprintf(PRINT_HIGH, "%s has joined in\n", ent->client->pers.netname);
                ent->client->resp.state = CS_PLAYING;
                ThrowEmIn(ent);
        }
        if (ent->client->menu)
                PMenu_Close(ent);
}

/*=====================================================================*/

pmenu_t choosing_menu[] = {
        // Dirty
        { "*Dirty",                     PMENU_ALIGN_CENTER, NULL, NULL },
        // Dirty
        { NULL,                         PMENU_ALIGN_CENTER, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { "BraZen Date:" BRAZEN_DATE,   PMENU_ALIGN_RIGHT, NULL, NULL }
};

void ChosenTeam(edict_t *ent, pmenu_t *p)
{
        teams_t *team;
        int     i;

        i = (((ent->client->menu->cur - 3) / 2) + 1);
        team = GetTeamByIndex(i);
        if (team == NULL)
        {
                gi.dprintf ("ChosenTeam & No team at index %i.\n", i);
                return;
        }
        JoinTeam(ent, team);
}

void UpdateChoosingMenu_WithTeams(edict_t *ent)
{
        static char teamplayers[8][32];
        teams_t *team;
        int     i, s;

        if (max_teams < 2)
        {
                gi.dprintf ("Errort - No enough teams requested\n");
                return;
        }

        // Join Team Names
        i = 1;
        s = 3;
        while (i <= max_teams)
        {
                team = GetTeamByIndex(i);
                if (team == NULL)
                {
                        gi.dprintf ("No team at index %i.\n", i);
                        break;
                }
                choosing_menu[s].text = team->name;
                choosing_menu[s].SelectFunc = ChosenTeam;
                s++;

                if (team->minm && team->maxm)
                {
                        if (team->maxm >= 1)
                                Com_sprintf (teamplayers[i], sizeof(teamplayers[i]), "  (%i players, %i min, %i max) ", team->members, team->minm, (int)team->maxm);
                        else
                                Com_sprintf (teamplayers[i], sizeof(teamplayers[i]), "  (%i players, %i min, max fraction) ", team->members, team->minm);
                }
                else if (team->maxm >= 1)
                        Com_sprintf (teamplayers[i], sizeof(teamplayers[i]), "  (%i players, %i max) ", team->members, (int)team->maxm);
                else if (team->maxm > 0)
                        Com_sprintf (teamplayers[i], sizeof(teamplayers[i]), "  (%i players, max fraction) ", team->members);
                else if (team->minm)
                        Com_sprintf (teamplayers[i], sizeof(teamplayers[i]), "  (%i players, %i min) ", team->members, team->minm);
                else
                        Com_sprintf (teamplayers[i], sizeof(teamplayers[i]), "  (%i players)", team->members);
                //gi.dprintf ("%s\n", teamplayers[i]);
                choosing_menu[s].text = teamplayers[i];
                s++;
                i++;
        }
}


void UpdateChoosingMenu(edict_t *ent)
{
        static  char levelname[32];
        static  char num[4][16];
        char    *message;
        //int     i, num_playing, num_watching, num_ready;

        // Map Name
	levelname[0] = '*';
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	levelname[sizeof(levelname) - 1] = 0;

        choosing_menu[1].text = levelname;

        if (max_teams) //&& (ent->client->resp.team == NULL))
                UpdateChoosingMenu_WithTeams(ent);
        else
        {
                if (turns_on)
                        message = "Declare readiness";
                else
                        message = "Enter Deathmatch";
                
                choosing_menu[3].text = message;
                choosing_menu[3].SelectFunc = StageTwo;

                /*
                num_playing = num_watching = 0;
                for (i = 0; i < maxclients->value; i++)
                {               
                        if (!g_edicts[i+1].inuse)
                                continue;
                        if (game.clients[i].resp.state == CS_PLAYING)
                                num_playing++;
                        else if (game.clients[i].resp.state == CS_READY)
                                num_ready++;
                        else
                                num_watching++;
                }

                sprintf(num[1], "%d playing", num_playing);
                choosing_menu[4].text = num[1];

                sprintf(num[2], "%d ready", num_ready);
                choosing_menu[5].text = num[2];

                sprintf(num[3], "%d not ready", num_watching);
                choosing_menu[6].text = num[3];
                */
        }

}

void OpenChoosingMenu (edict_t *ent, pmenu_t *p)
{
        int i, m;

        if (ent->client->menu)
                PMenu_Close(ent);

        /*
        if (((int)bflags->value & PF_CHOOSE_STUFF) && (!max_teams))
        {
                OpenChooseWeaponMenu(ent);
                return;
        }
        */

        i = GetTeamBySize (false, false, false);
        if (i > 0)
                m = (((i - 1) * 2) + 3);
        else
                m = -1;

        UpdateChoosingMenu(ent);
        PMenu_Open(ent, choosing_menu, m, sizeof(choosing_menu) / sizeof(pmenu_t));
}

// Dirty
void ChooseNextStep (edict_t *ent, pmenu_t *p)
{
        // Force Assign...
        // Dirty - Choose stuff in DM now
        if (max_teams && (!ent->client->resp.team))
                OpenChoosingMenu(ent, NULL);
        else if ((int)bflags->value & BF_CHOOSE_STUFF)
                StageTwo(ent, NULL);
        // Dirty
        else
                OpenChoosingMenu(ent, NULL);
}

void ChaseCamFromMenu(edict_t *ent, pmenu_t *p)
{
        PMenu_Close (ent);
	if (ent->client->chase_target)
        {
                NoLongerChase (ent);
                //ent->client->chase_target = NULL;
		return;
	}

        ent->client->chase_mode = CHASE_ROTATE;
        GetChaseTarget (ent);
}

pmenu_t credits_menu[] = {
        { "*Dirty",                     PMENU_ALIGN_CENTER, NULL, NULL },
        { NULL,                         PMENU_ALIGN_CENTER, NULL, NULL },
        { "*Models/Pics/Sounds",        PMENU_ALIGN_CENTER, NULL, NULL },
        { "The A-Team",                         PMENU_ALIGN_CENTER, NULL, NULL },
        { "*Programming/Modified Models",       PMENU_ALIGN_CENTER, NULL, NULL }, 
        { "Mr_Grim",                            PMENU_ALIGN_CENTER, NULL, NULL },
        { "*Special thanks to wyrdR!",          PMENU_ALIGN_CENTER, NULL, NULL }, 
        { "(logic and vector help)",            PMENU_ALIGN_CENTER, NULL, NULL },
        { "*Special thanks to Hellbent!",       PMENU_ALIGN_CENTER, NULL, NULL }, 
        { "(Server, beta testing & input)",     PMENU_ALIGN_CENTER, NULL, NULL },
        { "*Special thanks to Lacutis!",        PMENU_ALIGN_CENTER, NULL, NULL }, 
        { "(Servers, beta testing & input)",    PMENU_ALIGN_CENTER, NULL, NULL },
        { "*Testing/Input :",                   PMENU_ALIGN_CENTER, NULL, NULL }, 
        { "BracK, Bink, FuNBoY",                PMENU_ALIGN_CENTER, NULL, NULL },
        { "Draken, mim@ & ",                    PMENU_ALIGN_CENTER, NULL, NULL },
        { "everyone at the AQ MBOARD!",         PMENU_ALIGN_CENTER, NULL, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, NULL, NULL },
        { "Return to Main Menu",        PMENU_ALIGN_LEFT, NULL, OpenInitialMenu }
};

void CreditsMenu(edict_t *ent, pmenu_t *p)
{
        if (ent->client->menu)
                PMenu_Close(ent);

        PMenu_Open(ent, credits_menu, -1, sizeof(credits_menu) / sizeof(pmenu_t));
}

pmenu_t commands_menu[] = {
        { "*HELP!!!",                           PMENU_ALIGN_CENTER, NULL, NULL },
        { "*Commands",                          PMENU_ALIGN_LEFT, NULL, NULL },
        { "+attack = fire weapon",              PMENU_ALIGN_LEFT, NULL, NULL },
        { "+attack2 = fire in secondary mode",  PMENU_ALIGN_LEFT, NULL, NULL },
        { "     or change weapons mode",        PMENU_ALIGN_LEFT, NULL, NULL },
        { "+action = pickup, search bodies,",   PMENU_ALIGN_LEFT, NULL, NULL },
        { "     climb up, grab ledges, kick ",  PMENU_ALIGN_LEFT, NULL, NULL },
        { "     enemies and heal teammates ",   PMENU_ALIGN_LEFT, NULL, NULL },
        { "drop_weapon = free hands quickly",   PMENU_ALIGN_LEFT, NULL, NULL },
        { "     or select weapon quickly",      PMENU_ALIGN_LEFT, NULL, NULL },
        { "hands = put away weapon(s) ",        PMENU_ALIGN_LEFT, NULL, NULL },
        { "     or select weapon quickly",      PMENU_ALIGN_LEFT, NULL, NULL },
        { "reload = reload weapon(s) prior",    PMENU_ALIGN_LEFT, NULL, NULL },
        { "      to them running out of ammo",  PMENU_ALIGN_LEFT, NULL, NULL },
        { "team <teamname> = change team ",     PMENU_ALIGN_LEFT, NULL, NULL },
        { "      to <teamname>",                PMENU_ALIGN_LEFT, NULL, NULL },
        { "change = reselect equipment ",       PMENU_ALIGN_LEFT, NULL, NULL },
        { "reply = quick communicate status",   PMENU_ALIGN_LEFT, NULL, NULL },
        { "request = quick communicate status", PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, NULL, NULL },
        { "Return to Main Menu",                PMENU_ALIGN_LEFT, NULL, HelpMenu }
};

void CommandsMenu(edict_t *ent, pmenu_t *p)
{
        if (ent->client->menu)
                PMenu_Close(ent);

        PMenu_Open(ent, commands_menu, -1, sizeof(commands_menu) / sizeof(pmenu_t));
}

pmenu_t howto_menu[] = {
        { "*HELP!!!",                           PMENU_ALIGN_CENTER, NULL, NULL },
        { "*How to...",                         PMENU_ALIGN_LEFT, NULL, NULL },
        { "bandage = free your hands and",      PMENU_ALIGN_LEFT, NULL, NULL },
        { "     hold still for a second",       PMENU_ALIGN_LEFT, NULL, NULL },
        { "change equipment = you can type",    PMENU_ALIGN_LEFT, NULL, NULL },
        { "     'change' at the console ",      PMENU_ALIGN_LEFT, NULL, NULL },
        { "     or hit TAB when dead",          PMENU_ALIGN_LEFT, NULL, NULL },
        { "change team = you can type ",        PMENU_ALIGN_LEFT, NULL, NULL },
        { "     'team <teamname>'",             PMENU_ALIGN_LEFT, NULL, NULL },
        { "     or hit TAB when dead",          PMENU_ALIGN_LEFT, NULL, NULL },
        { "climb up = jump then hit +action",   PMENU_ALIGN_LEFT, NULL, NULL },
        { "do flips = jump while next to a",    PMENU_ALIGN_LEFT, NULL, NULL },
        { "     wall or moving through midair", PMENU_ALIGN_LEFT, NULL, NULL },
        { "grab a ledge = hit +action near",    PMENU_ALIGN_LEFT, NULL, NULL },
        { "     ledge, then press forward/up",  PMENU_ALIGN_LEFT, NULL, NULL },
        { "pickup an item = hold down ",        PMENU_ALIGN_LEFT, NULL, NULL },
        { "     +action, walk over item",       PMENU_ALIGN_LEFT, NULL, NULL },
        { "search a corpse = walk on corpse",   PMENU_ALIGN_LEFT, NULL, NULL },
        { "     then press +action",            PMENU_ALIGN_LEFT, NULL, NULL },
        { "getting more bandages = search a corpse,",   PMENU_ALIGN_LEFT, NULL, NULL },
        { "     then press +action",            PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, NULL, NULL },
        { "Return to Main Menu",                PMENU_ALIGN_LEFT, NULL, HelpMenu }
};

void HowToMenu(edict_t *ent, pmenu_t *p)
{
        if (ent->client->menu)
                PMenu_Close(ent);

        PMenu_Open(ent, howto_menu, -1, sizeof(howto_menu) / sizeof(pmenu_t));
}


pmenu_t wephelp_menu[] = {
        { "*HELP!!!",                                   PMENU_ALIGN_CENTER, NULL, NULL },
        { "*Weapon Help",                               PMENU_ALIGN_LEFT, NULL, NULL },
        { "*How do I use two weapons?",                 PMENU_ALIGN_LEFT, NULL, NULL },
        { "If you have two of them, just",              PMENU_ALIGN_LEFT, NULL, NULL },
        { "use the weapon twice.",                      PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                                         PMENU_ALIGN_CENTER, NULL, NULL },
        { "*How do I strip a weapons ammo?",            PMENU_ALIGN_LEFT, NULL, NULL },
        { "While your hands are free, drop the",        PMENU_ALIGN_LEFT, NULL, NULL },
        { "weapon you wish to take ammo from.",         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                                         PMENU_ALIGN_CENTER, NULL, NULL },
        { "Return to Main Menu",                PMENU_ALIGN_LEFT, NULL, HelpMenu }
};

void WepHelpMenu (edict_t *ent, pmenu_t *p)
{
        if (ent->client->menu)
                PMenu_Close(ent);

        PMenu_Open(ent, wephelp_menu, -1, sizeof(wephelp_menu) / sizeof(pmenu_t));
}

pmenu_t help_menu[] = {
        { "*HELP!!!",                           PMENU_ALIGN_CENTER, NULL, NULL },
        { "How to...",                          PMENU_ALIGN_LEFT, NULL, HowToMenu },
        { NULL,                                 PMENU_ALIGN_CENTER, NULL, NULL },
        { "Commands",                           PMENU_ALIGN_LEFT, NULL, CommandsMenu },
        { NULL,                                 PMENU_ALIGN_CENTER, NULL, NULL },
        { "Weapon Help",                       PMENU_ALIGN_LEFT, NULL, WepHelpMenu },
        //{ "???",                           PMENU_ALIGN_LEFT, NULL, CommandsMenu },
        { NULL,                                 PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, NULL, NULL },
        { "Return to Main Menu",                PMENU_ALIGN_LEFT, NULL, OpenInitialMenu }
};

void UpdateHelpMenu (edict_t *ent)
{
        if (ent->client->resp.state > CS_NOT_READY)
        {
                help_menu[10].text = "Use [, ]";
                help_menu[11].text = "ENTER to select";
                help_menu[12].text = "ESC, TAB or FIRE to Exit Menu";
                help_menu[12].SelectFunc = NULL;
        }
}

void HelpMenu (edict_t *ent, pmenu_t *p)
{
        if (ent->client->menu)
                PMenu_Close(ent);

        UpdateHelpMenu (ent);

        PMenu_Open(ent, help_menu, -1, sizeof(help_menu) / sizeof(pmenu_t));
}

pmenu_t initial_menu[] = {
        { "*Dirty",                     PMENU_ALIGN_CENTER, NULL, NULL },
        { NULL,                         PMENU_ALIGN_CENTER, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { "Next Step",                  PMENU_ALIGN_LEFT, NULL, ChooseNextStep },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { "Chase Camera",               PMENU_ALIGN_LEFT, NULL, ChaseCamFromMenu },
        { "Credits",                    PMENU_ALIGN_LEFT, NULL, CreditsMenu },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { "Help!!",                     PMENU_ALIGN_LEFT, NULL, HelpMenu },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { "Use [, ] or movement keys",  PMENU_ALIGN_LEFT, NULL, NULL },
        { "ENTER or FIRE to select",    PMENU_ALIGN_LEFT, NULL, NULL },
        { "ESC or ALTFIRE to Exit Menu", PMENU_ALIGN_LEFT, NULL, NULL },
        { "(TAB or ALTFIRE to Return)", PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { "BraZen Date:" BRAZEN_DATE,   PMENU_ALIGN_RIGHT, NULL, NULL }
};

void UpdateInitialMenu(edict_t *ent)
{
        static  char levelname[32];
        static  char num[4][16];
        char    *message;

        // Map Name
	levelname[0] = '*';
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	levelname[sizeof(levelname) - 1] = 0;

        initial_menu[1].text = levelname;

        if (max_teams)
        {
                if (ent->client->resp.team)
                        message = "Select Equipment";
                else
                        message = "Choose Team";
        }
        else if ((int)bflags->value & BF_CHOOSE_STUFF)
                message = "Select Equipment";
        else
        {
                if (turns_on)
                        message = "Declare readiness";
                else
                        message = "Enter Deathmatch";
                initial_menu[3].SelectFunc = StageTwo;
        }

        initial_menu[3].text = message;

        /*
	if (ent->client->chase_target)
                initial_menu[5].text = "Leave Chase Camera";
	else
                initial_menu[5].text = "Chase Camera";
        */
}

void OpenInitialMenu(edict_t *ent, pmenu_t *p)
{
        if (ent->client->menu)
                PMenu_Close(ent);

        UpdateInitialMenu(ent);
        PMenu_Open(ent, initial_menu, -1, sizeof(initial_menu) / sizeof(pmenu_t));
}

// Dirty


qboolean StartClient(edict_t *ent)
{
        if (!deathmatch->value)
        {
                ent->client->resp.state = CS_PLAYING;
                return true;
        }

        if (ent->client->pers.spectator)
        {
                gi.cprintf (ent, PRINT_HIGH, "Spectators can't join in\n");
                return false;
        }

        if (ent->client->resp.state > CS_NOT_READY)
        {
                if (!turns_on)
                        return true;
                if (ent->client->resp.state == CS_PLAYING)
                        return true;
                if ((ent->client->resp.state == CS_READY) && (!MatchInProgress()) && (level.match_state != MATCH_FINISHED))
                        return true;
        }

        // Start as 'observer'
        MakeObserver(ent);

        // If force join and teamplay, auto-open stage two
        // been_in is to stop overflows and Netchan_Transit shit...
        // must hit fire or something to bring up menu once in....
        if ((ent->client->resp.state == CS_NOT_READY) && ent->client->been_in)
                OpenInitialMenu (ent, NULL);

        if (!ent->client->been_in)
                BrazenCmdsInit (ent);

        ent->client->been_in = true;

        return false;
}

qboolean FloodCheck (edict_t *ent)
{
        gclient_t       *cl;
        int             i;

        // Flood protection for those fuck heads we know are out there...
	if (flood_msgs->value)
        {
		cl = ent->client;

                if (level.time < cl->flood_locktill)
                {
                        gi.cprintf(ent, PRINT_HIGH, "Wait %d more seconds\n", (int)(cl->flood_locktill - level.time));
                        return false;
                }

                i = cl->flood_whenhead - flood_msgs->value + 1;
                if (i < 0)
                        i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;

                if (cl->flood_when[i] && level.time - cl->flood_when[i] < flood_persecond->value)
                {
                        cl->flood_locktill = level.time + flood_waitdelay->value;
                        gi.cprintf(ent, PRINT_CHAT, "Flood protection:  Wait %d seconds.\n", (int)flood_waitdelay->value);
                        return false;
                }

                cl->flood_whenhead = (cl->flood_whenhead + 1) % (sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

        return true;
}
