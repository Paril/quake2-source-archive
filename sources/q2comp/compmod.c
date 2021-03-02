//
// compmod.c
//

/*
Q2Comp Issues
=============

Votes over countdowns
---------------------
What happens when a vote crosses the countdown boundary?
*/

// Includes
#include "g_local.h"

void Cmd_Admin_f (edict_t *ent)
{
	// if Admin already return
	if (ent->client->resp.adminflag)
		return;
	else if (ent->client->resp.adminflag == CM_ADMINFLAG_NOT)
	{
		if (Q_stricmp (gi.argv(1), "") == 0)
		{
			// tell user to use "admin <code>"
			gi.cprintf (ent, PRINT_HIGH, "\nYou are not an Admin.\n");
			gi.cprintf (ent, PRINT_HIGH, "To become an Admin, use the command:\n");
			gi.cprintf (ent, PRINT_HIGH, "'admin <code>' - where <code> is the Admin code.\n");
		}
		else if (Q_stricmp (gi.argv(1), admincode->string) == 0)
		{
			// you are now an Admin, tell everyone
			ent->client->resp.adminflag = CM_ADMINFLAG_CODED;
			ent->client->resp.adminerrorcount = 0;
			gi.bprintf (PRINT_HIGH, "\n%s is now an Admin.\n", ent->client->pers.netname);
		}
		else
		{
			// admin code incorrect
			gi.cprintf (ent, PRINT_HIGH, "The Admin code '%s' is incorrect.\n", gi.argv(1));
			ent->client->resp.adminerrorcount++;
			// too many invalids, then disconnect the person
			if (ent->client->resp.adminerrorcount >= adminkick->value)
			{
				gi.bprintf (PRINT_MEDIUM, "Too many invalid Admin tries, %s. You are gone.\n", ent->client->pers.netname);
				StuffCommand (ent, "disconnect\n");
			}
		}
	}
	else
		gi.dprintf ("Something is whacked - adminflag is %i\n", ent->client->resp.adminflag);
}

void Cmd_BecomePlayer_f (edict_t *ent)
{
	if (MatchInProgress(ent))
		return;

	if (ent->client->resp.teamnumber == CM_TEAM_SPECTATOR)
	{
		ent->client->resp.ready = CM_NOTREADY;
		ent->client->resp.teamnumber = CM_TEAM_NONE;
		ent->client->pers.spectator = 0;
		respawn (ent);
	}
}

void Cmd_BecomeSpectator_f (edict_t *ent)
{
	if (MatchInProgress(ent))
		return;
	if (ent->client->resp.teamnumber == CM_TEAM_SPECTATOR)
		return;

	// basic Quake 2 setting for the player
	ent->client->resp.score = 0;
	ent->client->pers.weapon = NULL;
	ChangeWeapon(ent);
	ent->solid = SOLID_NOT;
	ent->movetype = MOVETYPE_NOCLIP;
	ent->model = "";
	gi.setmodel (ent, ent->model);
	ent->client->pers.spectator = 1;

	// Q2Comp things
	// clear scores
	ent->client->resp.ready = CM_READY;
	ent->client->resp.teamnumber = CM_TEAM_SPECTATOR;	//spectator team number
	ent->client->resp.frags = 0;
	ent->client->resp.enemykills = 0;
	ent->client->resp.friendkills = 0;
	ent->client->resp.deaths = 0;
}

void Cmd_ClanList_f (edict_t *ent, int iListingType)
{
	edict_t	*clan = NULL;
	char	class_name[9] = "";

	gi.cprintf (ent, PRINT_HIGH, "\n");
	if (iListingType == 0)
		strcpy(class_name, "clan");
	else
	{
		strcpy(class_name, "usedclan");
		gi.cprintf (ent, PRINT_HIGH, "Used\n");
	}

	gi.cprintf (ent, PRINT_HIGH, "Team List\n");
	gi.cprintf (ent, PRINT_HIGH, "---------\n");
	while ((clan = G_Find (clan, FOFS(classname), class_name)) != NULL)
	{
		gi.cprintf (ent, PRINT_HIGH, "%s - %i\n", team[clan->flags].teamname, clan->mass);
	}
}

void Cmd_Contact_f (edict_t *ent)
{
	gi.cprintf (ent, PRINT_MEDIUM, "\nYour server admin is %s and can be contacted at %s\n", adminname->string, email->string);
}

void Cmd_Disable_f (edict_t *ent)
{
	gi.cprintf (ent, PRINT_MEDIUM, "'disable' not finished yet\n");
	return;
}

void Cmd_DisplayModMode_f (edict_t *ent)
{
	int	iTimeLimit;

	gi.cprintf (ent, PRINT_HIGH, "\nServer mode  :");
	if (compmod.serverstatus == CM_FFA)
		gi.cprintf (ent, PRINT_HIGH, "FFA\n");
	else if (compmod.serverstatus == CM_MATCHINPROGRESS)
		gi.cprintf (ent, PRINT_HIGH, "Match\n");
	else if (compmod.serverstatus == CM_MATCHRALLY)
		gi.cprintf (ent, PRINT_HIGH, "Rally\n");
	else if (compmod.serverstatus == CM_MATCHCOUNTDOWN)
		gi.cprintf (ent, PRINT_HIGH, "counting down to Match\n");

	// Powerups
	gi.cprintf (ent, PRINT_HIGH, "Powerups     :");
	if (compmod.powerups == CM_POWERUPS_ON)
		gi.cprintf (ent, PRINT_HIGH, "Enabled\n");
	else if (compmod.powerups == CM_POWERUPS_OFF)
		gi.cprintf (ent, PRINT_HIGH, "Disabled\n");
	else if (compmod.powerups == CM_INVULN_OFF)
		gi.cprintf (ent, PRINT_HIGH, "Inv off, Quad on\n");
	else if (compmod.powerups == CM_QUAD_OFF)
		gi.cprintf (ent, PRINT_HIGH, "Quad off, Inv on\n");

	// timelimit
	if (compmod.serverstatus == CM_FFA)
		iTimeLimit = timelimit->value;
	else
		iTimeLimit = compmod.matchtime;
	gi.cprintf (ent, PRINT_HIGH, "Time limit   :%i\n", iTimeLimit);

	// fraglimit
	if (compmod.serverstatus == CM_FFA)
		iTimeLimit = fraglimit->value;	// cheating and reusing the iTimeLimit var :)
	else
		iTimeLimit = compmod.matchfrags;
	gi.cprintf (ent, PRINT_HIGH, "Frag limit   :%i\n", iTimeLimit);

	// DropQuad
	gi.cprintf (ent, PRINT_HIGH, "Drop Quad    :");
	if ((int)(dmflags->value) & DF_QUAD_DROP)
		gi.cprintf (ent, PRINT_HIGH, "ON\n");
	else
		gi.cprintf (ent, PRINT_HIGH, "OFF\n");

	// Friendly Fire
	gi.cprintf (ent, PRINT_HIGH, "Friendly Fire:");
	if ((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE)
		gi.cprintf (ent, PRINT_HIGH, "OFF (can't hurt teammates)\n");
	else
		gi.cprintf (ent, PRINT_HIGH, "ON (can hurt teammates)\n");

	// FastWeap
	gi.cprintf (ent, PRINT_HIGH, "Fast Weap    :");
	if (fastweap->value)
		gi.cprintf (ent, PRINT_HIGH, "ON\n");
	else
		gi.cprintf (ent, PRINT_HIGH, "OFF\n");

	// Lockdown
	gi.cprintf (ent, PRINT_HIGH, "Lockdown     :");
	if (compmod.lockdown)
		gi.cprintf (ent, PRINT_HIGH, "ON\n");
	else
		gi.cprintf (ent, PRINT_HIGH, "OFF\n");

	// ScoreCasting
	gi.cprintf (ent, PRINT_HIGH, "ScoreCast    :");
	if (scorecast->value)
		gi.cprintf (ent, PRINT_HIGH, "ON\n");
	else
		gi.cprintf (ent, PRINT_HIGH, "OFF\n");

	// Shutup
	gi.cprintf (ent, PRINT_HIGH, "Shutup mode  :");
	if (compmod.shutup == CM_SHUTUP_OFF)
		gi.cprintf (ent, PRINT_HIGH, "Off - all can talk\n");
	else if (compmod.shutup == CM_SHUTUP_SPECS)
		gi.cprintf (ent, PRINT_HIGH, "Specs - Spectators are silenced\n");
	else if (compmod.shutup == CM_SHUTUP_ALL)
		gi.cprintf (ent, PRINT_HIGH, "All - only Admins can talk\n");

	// Respawn Protection
	iTimeLimit = protect->value;
	gi.cprintf (ent, PRINT_HIGH, "Respawn Prot.:%i secs.\n", iTimeLimit);

	// FullWeapRally
	gi.cprintf (ent, PRINT_HIGH, "FullWeapRally:");
	if (fullweaprally->value)
		gi.cprintf (ent, PRINT_HIGH, "ON\n");
	else
		gi.cprintf (ent, PRINT_HIGH, "OFF\n");

	// Spectator
	if (ent != NULL)
	{
		if (ent->client->resp.teamnumber == CM_TEAM_SPECTATOR)
		{
			gi.cprintf (ent, PRINT_HIGH, "\nYou are a Spectator.\n");
		}
	}

	// Admin
	if (ent != NULL)
	{
		gi.cprintf (ent, PRINT_HIGH, "\nYou are ");
		if (ent->client->resp.adminflag == CM_ADMINFLAG_NOT)
			gi.cprintf (ent, PRINT_HIGH, "NOT ");
		gi.cprintf (ent, PRINT_HIGH, "an Admin.\n");
	}
}

void DisplayMOTD (edict_t *ent)
{
	char	*sUserMessage1;
	char	*sUserMessage2;
	char	sMessage[600];

	// 4/6/98 - removed the INI crap and went with CVars!!!

	sUserMessage1 = motd1->string;
	sUserMessage2 = motd2->string;
	strcpy(sMessage, "Server running Quake 2\n"); // 3/30/98 - changed name
	strcat(sMessage, "Competition Mod (Q2Comp) 0.30 beta\n");
	strcat(sMessage, "Author: CrushBug\n");
	strcat(sMessage, "E-mail: crushbug@telefragged.com\n\n");
	if (sUserMessage1 != NULL)
		strcat(sMessage, sUserMessage1);
	if (sUserMessage2 != NULL)
	{
		strcat(sMessage, "\n");
		strcat(sMessage, sUserMessage2);
	}
	gi.centerprintf (ent, sMessage);
}

void Cmd_DropQuad_f (edict_t *ent)
{
	int	iDMFlags;

	if (MatchInProgress(ent))
		return;
	if (NotAnAdmin(ent))
		return;

	iDMFlags = dmflags->value;
	if (Q_stricmp (gi.argv(1), "on") == 0)
	{
		iDMFlags |= DF_QUAD_DROP;
		gi.cvar_set("dmflags", va("%i", iDMFlags));
	}
	else if (Q_stricmp (gi.argv(1), "off") == 0)
	{
		iDMFlags &= ~DF_QUAD_DROP;
		gi.cvar_set("dmflags", va("%i", iDMFlags));
	}
	else
	{
		gi.cprintf (ent, PRINT_MEDIUM,"\nTo change Drop Quad, use the command:\n");
		gi.cprintf (ent, PRINT_MEDIUM,"'dropquad <opt>'\n");
		gi.cprintf (ent, PRINT_MEDIUM,"<opt> = 'on'      - Drop Quad on\n");
		gi.cprintf (ent, PRINT_MEDIUM,"        'off'     - Drop Quad off\n");
		return;
	}

	gi.bprintf (PRINT_MEDIUM, "\nDrop Quad is ");
	if ((int)(dmflags->value) & DF_QUAD_DROP)
		gi.bprintf (PRINT_MEDIUM, "ON\n");
	else
		gi.bprintf (PRINT_MEDIUM, "OFF\n");
}

void Cmd_Elect_f (edict_t *ent)
{
	edict_t	*vote;

	// handle special situations
	// already an admin
	if (ent->client->resp.adminflag)
		return;
	// elections disabled
	if (noelect->value)
	{
		gi.cprintf (ent, PRINT_HIGH,"\nElections have been disabled by the server administrator.\n");
		return;
	}
	// only client, grant admin
	if (PlayerCount (PC_ALLCLIENTS) == 1)
	{
		ent->client->resp.adminflag = CM_ADMINFLAG_ELECTED;
		gi.bprintf (PRINT_HIGH, "%s is now an Admin.\n", ent->client->pers.netname);
		return;
	}

	// see if one is already happening
	if (FindVote ())
	{
		gi.cprintf (ent, PRINT_HIGH,"\nVote already in progress.\n");
		return;
	}

	// start by spawning a vote entity
	vote = G_Spawn();
	vote->classname = "vote";
	vote->flags = CM_VOTETYPE_ADMIN;
	vote->mass = 0;		// votes yes
	vote->health = 0;	// votes no
	vote->target = ent->client->pers.netname;	// requestor's name
	vote->nextthink = level.time + CM_VOTETHINKTIME;
	vote->think = VoteThink;

	// vote yes for this person
	Cmd_Vote_f (ent, CM_VOTE_YES);
}

void Cmd_Endmatch_f (edict_t *ent)
{
	edict_t	*vote;

	// must be a running match
	if (compmod.serverstatus != CM_MATCHINPROGRESS)
		return;

	if (ent->client->resp.adminflag)	// if Admin, kill the game NOW!
		EndMatch ();
	else
	{
		if (FindVote ())
		{
			gi.cprintf (ent, PRINT_HIGH,"\nVote already in progress.\n");
			return;
		}

		// start by spawning a vote entity
		vote = G_Spawn();
		vote->classname = "vote";
		vote->flags = CM_VOTETYPE_ENDMATCH;
		vote->mass = 0;		// votes yes
		vote->health = 0;	// votes no
		vote->target = ent->client->pers.netname;	// requestor's name
		vote->nextthink = level.time + CM_VOTETHINKTIME;
		vote->think = VoteThink;

		// vote yes for this person
		Cmd_Vote_f (ent, CM_VOTE_YES);
	}
}

void Cmd_FriendlyFire_f (edict_t *ent)
{
	int	iDMFlags;

	if (MatchInProgress(ent))
		return;
	if (NotAnAdmin(ent))
		return;

	iDMFlags = dmflags->value;
	if (Q_stricmp (gi.argv(1), "off") == 0)
	{
		iDMFlags |= DF_NO_FRIENDLY_FIRE;
		gi.cvar_set("dmflags", va("%i", iDMFlags));
	}
	else if (Q_stricmp (gi.argv(1), "on") == 0)
	{
		iDMFlags &= ~DF_NO_FRIENDLY_FIRE;
		gi.cvar_set("dmflags", va("%i", iDMFlags));
	}
	else
	{
		gi.cprintf (ent, PRINT_MEDIUM,"\nTo change Friendly Fire, use the command:\n");
		gi.cprintf (ent, PRINT_MEDIUM,"'ff <opt>'\n");
		gi.cprintf (ent, PRINT_MEDIUM,"<opt> = 'on'  - Friendly Fire on\n");
		gi.cprintf (ent, PRINT_MEDIUM,"        'off' - Friendly Fire off\n");
		return;
	}

	gi.bprintf (PRINT_MEDIUM, "\nFriendly Fire is ");
	if ((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE)
		gi.bprintf (PRINT_MEDIUM, "OFF (can't hurt teammates)\n");
	else
		gi.bprintf (PRINT_MEDIUM, "ON (can hurt teammates)\n");
}

void Cmd_Lockdown_f (edict_t *ent)
{
	int	iArgv = 1;

	if (MatchInProgress(ent))
		return;
	if (NotAnAdmin(ent))
		return;
	// lockdown disabled
	if (nolockdown->value)
	{
		gi.cprintf (ent, PRINT_HIGH,"\nLockdown has been disabled by the server administrator.\n");
		return;
	}

	if (ent == NULL)	// if ent is NULL, then this was "sv lockdown on" from the console
		iArgv++;

	if (Q_stricmp (gi.argv(iArgv), "on") == 0)
		compmod.lockdown = CM_LOCKDOWN_ON;
	else if (Q_stricmp (gi.argv(iArgv), "off") == 0)
		compmod.lockdown = CM_LOCKDOWN_OFF;
	else
	{
		gi.cprintf (ent, PRINT_HIGH,"\nTo change the server Lockdown, use the command:\n");
		gi.cprintf (ent, PRINT_HIGH,"'");
		if (ent == NULL)
			gi.cprintf (ent, PRINT_HIGH,"sv ");
		gi.cprintf (ent, PRINT_HIGH,"'lockdown <opt>'\n");
		gi.cprintf (ent, PRINT_HIGH,"<opt> = 'on'      - Lockdown ON\n");
		gi.cprintf (ent, PRINT_HIGH,"        'off'     - Lockdown OFF\n");
	}
	gi.cprintf (ent, PRINT_HIGH, "\nServer Lockdown is:");
	if (compmod.lockdown == CM_LOCKDOWN_OFF)
		gi.cprintf (ent, PRINT_HIGH, "OFF\n");
	else
		gi.cprintf (ent, PRINT_HIGH, "ON\n");
}

void Cmd_MatchFragSet_f (edict_t *ent)
{
	int	iArgv = 1;

	if (compmod.serverstatus == CM_FFA)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\nMatch fraglimit setting is only valid in Match mode.\n");
		return;
	}

	if (MatchInProgress(ent))
		return;
	if (NotAnAdmin(ent))
		return;

	if (ent == NULL)	// if ent is NULL, then this was "sv matchfragset <x>" from the console
		iArgv++;

	if (Q_stricmp (gi.argv(iArgv), "") == 0)
	{
		gi.cprintf (ent, PRINT_MEDIUM,"\nTo set the Match Fraglimit, use:\n");
		gi.cprintf (ent, PRINT_MEDIUM,"'");
		if (ent == NULL)
			gi.cprintf (ent, PRINT_MEDIUM,"sv ");
		gi.cprintf (ent, PRINT_MEDIUM,"matchfragset <n>' - where <n> is the Fraglimit.\n");
		gi.cprintf (ent, PRINT_MEDIUM,"0 means no Fraglimit.\n");
	}
	else
	{
		compmod.matchfrags = atoi(gi.argv(iArgv));
		if (compmod.matchfrags < 0)
		{
			compmod.matchfrags = 0;
			gi.cprintf (ent, PRINT_MEDIUM, "\nMatch lower Fraglimit is 0 - no Fraglimit.\n");
		}
		if (compmod.matchfrags > 200)
		{
			compmod.matchfrags = 200;
			gi.cprintf (ent, PRINT_MEDIUM, "\nMatch upper Fraglimit is 200.\n");
		}
	}
	gi.bprintf (PRINT_MEDIUM, "\nMatch Fraglimit is %i\n", compmod.matchfrags);
}

void Cmd_MatchTimeSet_f (edict_t *ent)
{
	int	iArgv = 1;

	if (compmod.serverstatus == CM_FFA)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\nMatch time setting is only valid in Match mode.\n");
		return;
	}

	if (MatchInProgress(ent))
		return;
	if (NotAnAdmin(ent))
		return;

	if (ent == NULL)	// if ent is NULL, then this was "sv lockdown on" from the console
		iArgv++;

	if (Q_stricmp (gi.argv(iArgv), "") == 0)
	{
		gi.cprintf (ent, PRINT_MEDIUM,"\nTo set the Match Timelimit, use:\n");
		gi.cprintf (ent, PRINT_MEDIUM,"'");
		if (ent == NULL)
			gi.cprintf (ent, PRINT_MEDIUM,"sv ");
		gi.cprintf (ent, PRINT_MEDIUM,"matchtime <time>' - where <time> is the time in minutes.\n");
	}
	else
	{
		compmod.matchtime = atoi(gi.argv(iArgv));
		if (compmod.matchtime < 2)	// this is so a test server can be forced to 2 minutes
		{
			compmod.matchtime = 2;
			gi.cprintf (ent, PRINT_MEDIUM, "\nMatch lower limit is 2 minutes.\n");
		}
		if (compmod.matchtime > 60)
		{
			compmod.matchtime = 60;
			gi.cprintf (ent, PRINT_MEDIUM, "\nMatch upper limit is 60 minutes.\n");
		}
	}
	gi.bprintf (PRINT_MEDIUM, "\nMatch Timelimit is %i minutes.\n", compmod.matchtime);
}

void Cmd_ModeSet_f (edict_t *ent)
{
	if (MatchInProgress(ent))
		return;
	if (NotAnAdmin(ent))
		return;
	
	if (compmod.serverstatus == CM_FFA)
	{
		compmod.serverstatus = CM_MATCHRALLY;
		RestartServer ();
	}
	else if (compmod.serverstatus == CM_MATCHRALLY)
	{
		compmod.serverstatus = CM_FFA;
		RestartServer ();
	}
	else
		gi.cprintf (ent, PRINT_HIGH,"\nSomething is wacked - Q2Comp Mode is not 0-3, its %i\n", compmod.serverstatus);
}

void Cmd_Normal_f (edict_t *ent)
{
	// if not an Admin return
	if (ent->client->resp.adminflag == CM_ADMINFLAG_NOT)
		return;
	// else turn off Admin
	else
	{
		// you are now no longer an Admin, tell everyone
		ent->client->resp.adminflag = CM_ADMINFLAG_NOT;
		ent->client->resp.adminerrorcount = 0;
		gi.bprintf (PRINT_MEDIUM, "\n%s is no longer an Admin.\n", ent->client->pers.netname);
	}
}

void Cmd_NotReady_f (edict_t *ent)
{
	edict_t *clan = NULL;

	if (compmod.serverstatus == CM_MATCHINPROGRESS)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\nToo late! Match in Progress.\n");
		return;
	}

	if (compmod.serverstatus == CM_FFA)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\n'notready' is only valid in Match mode.\n");
		return;
	}

	if (ent->client->resp.teamnumber == CM_TEAM_SPECTATOR)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\nYou are a Spectator, you cannot change your status.\n");
		return;
	}

	if (ent->client->resp.ready == CM_READY)
	{
		ent->client->resp.ready = CM_NOTREADY;
		gi.bprintf (PRINT_MEDIUM, "\n%s's status is Not Ready.\n", ent->client->pers.netname);
		// countdown already picks up this change
		// team is still valid, but could be changed
		clan = FindClan (ent->client->resp.teamnumber);
		if (clan)
			clan->mass--;
	}
}

void Cmd_PlayerList_f (edict_t *ent)
{
	edict_t	*player = NULL;
	static char	skin[512];

	if (compmod.serverstatus == CM_FFA)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\n'playerlist' command is only valid in Match mode.\n");
		return;
	}

	gi.cprintf (ent, PRINT_MEDIUM, "\nPlayer           Status    Team       Skin\n");
	gi.cprintf (ent, PRINT_MEDIUM, "=====================================================\n");
	while ((player = G_Find (player, FOFS(classname), "player")) != NULL)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "%-16s ", player->client->pers.netname);
		if (player->client->resp.ready)
			gi.cprintf (ent, PRINT_MEDIUM, "Ready     ");
		else
			gi.cprintf (ent, PRINT_MEDIUM, "Not Ready ");

		if (player->client->resp.teamnumber == CM_TEAM_SPECTATOR)
			gi.cprintf (ent, PRINT_MEDIUM, "SPECTATOR  ");
		else
			gi.cprintf (ent, PRINT_MEDIUM, "%-10s ", team[player->client->resp.teamnumber].teamname);
		strcpy(skin, Info_ValueForKey (player->client->pers.userinfo, "skin"));
		gi.cprintf (ent, PRINT_MEDIUM, "%-15s\n", skin);
	}
}

void Cmd_Powerups_f (edict_t *ent)
{
	int	iArgv = 1;

	if (MatchInProgress(ent))
		return;
	if (NotAnAdmin(ent))
		return;

	if (ent == NULL)	// if ent is NULL, then this was "sv lockdown on" from the console
		iArgv++;

	if (Q_stricmp (gi.argv(iArgv), "on") == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "Powerups are now enabled.\n");
		compmod.powerups = CM_POWERUPS_ON;
		DisableFlagSet (CM_ENABLE, CM_DIS_POWERUPS);
	}
	else if (Q_stricmp (gi.argv(iArgv), "off") == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "Powerups are now disabled.\n");
		compmod.powerups = CM_POWERUPS_OFF;
		DisableFlagSet (CM_DISABLE, CM_DIS_POWERUPS);
	}
	else if (Q_stricmp (gi.argv(iArgv), "invoff") == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "Powerups - Invunerability is now disabled.\n");
		compmod.powerups = CM_INVULN_OFF;
		DisableFlagSet (CM_DISABLE, CM_DIS_INV);
	}
	else if (Q_stricmp (gi.argv(iArgv), "quadoff") == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "Powerups - Quad is now disabled.\n");
		compmod.powerups = CM_QUAD_OFF;
		DisableFlagSet (CM_DISABLE, CM_DIS_QUAD);
	}
	else
	{
		// help message
		gi.cprintf (ent, PRINT_HIGH,"\nTo change the powerups, use the command:\n");
		gi.cprintf (ent, PRINT_HIGH,"'");
		if (ent == NULL)
			gi.cprintf (ent, PRINT_HIGH,"sv ");
		gi.cprintf (ent, PRINT_HIGH,"powerups <opt>'\n");
		gi.cprintf (ent, PRINT_HIGH,"<opt> = 'on'      - all on\n");
		gi.cprintf (ent, PRINT_HIGH,"        'off'     - all off\n");
		gi.cprintf (ent, PRINT_HIGH,"        'invoff'  - no Invulnerability\n");
		gi.cprintf (ent, PRINT_HIGH,"        'quadoff' - no Quad\n");
		return;
	}
	gi.cprintf (ent, PRINT_HIGH, "Restart level to take effect.\n");
}

void Cmd_Ready_f (edict_t *ent)
{
	edict_t *clan = NULL;

	if (MatchInProgress(ent))
		return;

	if (compmod.serverstatus == CM_FFA)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "'ready' is only valid in Match mode.\n");
		return;
	}

	if (ent->client->resp.teamnumber == CM_TEAM_SPECTATOR)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\nYou are a Spectator, you cannot change your status.\n");
		return;
	}

	// if already ready, state it and return
	if (ent->client->resp.ready == CM_READY)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\nYou are already 'ready'.\nType 'notready' at the console to change your status.\n");
		return;
	}
	else
	{
		if (ent->client->resp.teamnumber == 0)
		{
			gi.cprintf (ent, PRINT_MEDIUM, "\nYou have not selected a team.\nSelect a team using 'team <name>'\n");
			return;
		}
		else
		{
			ent->client->resp.ready = CM_READY;
			// check to see if a clan entity called 'team' exists, if not spawn it
			clan = FindClan (ent->client->resp.teamnumber);
			if (!clan)
			{
				clan = MakeClan (ent->client->resp.teamnumber);
				gi.bprintf (PRINT_HIGH, "\nClan '%s' created.\n", team[clan->flags].teamname);
			}
			clan->mass++;
			gi.bprintf (PRINT_HIGH, "\n%s set to Clan '%s'\n", ent->client->pers.netname, team[clan->flags].teamname);
		}
	}
}

void Cmd_Restart_f (edict_t *ent)
{
	if (MatchInProgress(ent))
		return;
	if (NotAnAdmin(ent))
		return;

	RestartServer ();
}

void Cmd_ShutUp_f (edict_t *ent)
{
	int	iArgv = 1;

	if (MatchInProgress(ent))
		return;
	if (NotAnAdmin(ent))
		return;

	if (ent == NULL)	// if ent is NULL, then this was "sv shutup <cmd>" from the console
		iArgv++;

	if (Q_stricmp (gi.argv(iArgv), "all") == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "All but Admins are silenced.\n");
		compmod.shutup = CM_SHUTUP_ALL;
	}
	else if (Q_stricmp (gi.argv(iArgv), "specs") == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "Spectators are silenced.\n");
		compmod.shutup = CM_SHUTUP_SPECS;
	}
	else if (Q_stricmp (gi.argv(iArgv), "off") == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "All are allowed to talk.\n");
		compmod.shutup = CM_SHUTUP_OFF;
	}
	else
	{
		// help message
		gi.cprintf (ent, PRINT_HIGH,"\nTo disable spectator or player talking, use the command:\n");
		gi.cprintf (ent, PRINT_HIGH,"'");
		if (ent == NULL)
			gi.cprintf (ent, PRINT_HIGH,"sv ");
		gi.cprintf (ent, PRINT_HIGH,"shutup <opt>'\n");
		gi.cprintf (ent, PRINT_HIGH,"<opt> = 'all'   - all but Admins are silenced\n");
		gi.cprintf (ent, PRINT_HIGH,"        'specs' - Spectators are silenced\n");
		gi.cprintf (ent, PRINT_HIGH,"        'off'   - all can talk\n");
		return;
	}
}

void Cmd_Team_f (edict_t *ent)
{
	char	name[256];

	if (compmod.serverstatus == CM_FFA)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\nTeam setting is only valid in Match mode.\n");
		return;
	}

	if (ent->client->resp.teamnumber == CM_TEAM_SPECTATOR)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\nYou are a Spectator, you cannot set a team.\n");
		return;
	}

	if (Q_stricmp (gi.argv(1), "") == 0)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\nYour Team is ");
		if (ent->client->resp.teamnumber == CM_TEAM_SPECTATOR)
			gi.cprintf (ent, PRINT_MEDIUM, "'<SPECTATOR>'\n");
		else
			gi.cprintf (ent, PRINT_MEDIUM, "'%s'\n", team[ent->client->resp.teamnumber].teamname);
		return;
	}
	else
	{
		if (MatchInProgress(ent))
			return;
		if (ent->client->resp.ready == CM_READY)
		{
			gi.cprintf (ent, PRINT_MEDIUM, "\nYour cannot change teams once you are 'ready'.\nUse 'notready' first, then set your team.\n");
			return;
		}
		strcpy (name, gi.argv(1));
		name[12] = '\0'; // cap off the team name to 12 characters
		ent->client->resp.teamnumber = AssignTeam (name);
		gi.cprintf (ent, PRINT_MEDIUM, "\nYour Team is ");
		if (ent->client->resp.teamnumber == CM_TEAM_SPECTATOR)
			gi.cprintf (ent, PRINT_MEDIUM, "'<SPECTATOR>'\n");
		else
			gi.cprintf (ent, PRINT_MEDIUM, "'%s'\n", team[ent->client->resp.teamnumber].teamname);
		return;
	}
}

void Cmd_TimerSet_f (edict_t *ent)
{
	edict_t	*timer;
	int	iArgv = 1;

	if (compmod.serverstatus == CM_FFA)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\nTimer setting is only valid in Match mode.\n");
		return;
	}

	if (MatchInProgress(ent))
		return;
	if (NotAnAdmin(ent))
		return;

	if (ent == NULL)	// if ent is NULL, then this was "sv timerset <x>" from the console
		iArgv++;

	if (Q_stricmp (gi.argv(iArgv), "") == 0)
	{
		gi.cprintf (ent, PRINT_MEDIUM,"\nTo set the Rally Timer, use:\n");
		gi.cprintf (ent, PRINT_MEDIUM,"'");
		if (ent == NULL)
			gi.cprintf (ent, PRINT_MEDIUM,"sv ");
		gi.cprintf (ent, PRINT_MEDIUM,"timerset <time>' - where <time> is the time in minutes.\n");
	}
	else
	{
		compmod.rallytime = atoi(gi.argv(iArgv));
		if (compmod.rallytime < 5)
		{
			compmod.rallytime = 5;
			gi.cprintf (ent, PRINT_MEDIUM, "\nRally Timer lower limit is 5 minutes.\n");
		}
		if (compmod.rallytime > 20)
		{
			compmod.rallytime = 20;
			gi.cprintf (ent, PRINT_MEDIUM, "\nRally Timer upper limit is 20 minutes.\n");
		}
		timer = FindTimer ();
		if (timer)
			timer->timestamp = level.time + (compmod.rallytime * 60);
	}
	gi.bprintf (PRINT_MEDIUM, "\nRally Timelimit is %i minutes.\n", compmod.rallytime);
}

void Cmd_Vote_f (edict_t *ent, int iVoteType)
{
	edict_t	*vote;

	vote = FindVote ();
	// if there is no vote going on, return
	if (!vote)
	{
		gi.cprintf (ent, PRINT_HIGH,"\nNo Vote in progress.\n");
		return;
	}
	// return if already voted
	if (ent->client->resp.voted)
	{
		gi.cprintf (ent, PRINT_HIGH,"\nYou have already voted.\n");
		return;
	}

	// do the vote
	if (iVoteType == CM_VOTE_YES)
	{
		ent->client->resp.voted = CM_VOTED_YES;
		vote->mass++;
	}
	else if (iVoteType == CM_VOTE_NO)
	{
		ent->client->resp.voted = CM_VOTED_NO;
		vote->health++;
	}
	else
	{
		gi.dprintf ("DEBUG - Cmd_Vote_f was passed %i\n.", iVoteType);
		return;
	}
	// evaluate the current stats
	EvaluateVote (vote);
}

void Cmd_Warp_f (edict_t *ent)
{
	char	command [256];
	char	map[256];
	edict_t	*vote;
	int		iTrust = 0;

	// handle special situations
	if (MatchInProgress(ent))
		return;
	// warp disabled
	if (nowarp->value && ent->client->resp.adminflag == CM_ADMINFLAG_NOT)
	{
		gi.cprintf (ent, PRINT_HIGH,"\nWarp has been disabled by the server administrator.\n");
		return;
	}

	// validate the target map - temporary, until a robust method is found
	if (Q_stricmp (gi.argv(1), "") == 0)
	{
		// display help on warp
		gi.cprintf (ent, PRINT_HIGH,"\nTo change the current map, use the command:\n");
		gi.cprintf (ent, PRINT_HIGH,"'warp <map> [trust]'\n");
		gi.cprintf (ent, PRINT_HIGH,"<map>   = the file name of the map to change to\n");
		gi.cprintf (ent, PRINT_HIGH,"[trust] = don't check for a valid map name\n");
		return;
	}

	strcpy(map, gi.argv(1));
	if ((Q_stricmp (gi.argv(2), "trust") == 0) && ent->client->resp.adminflag == CM_ADMINFLAG_CODED)
		iTrust = 1;	// trust works for coded admins only
	else
	{
		if(
			(stricmp(map, "base1") == 0)	||
			(stricmp(map, "base2") == 0)	||
			(stricmp(map, "base3") == 0)	||
			(stricmp(map, "biggun") == 0)	||
			(stricmp(map, "boss1") == 0)	||
			(stricmp(map, "boss2") == 0)	||
			(stricmp(map, "bunk1") == 0)	||
			(stricmp(map, "city1") == 0)	||
			(stricmp(map, "city2") == 0)	||
			(stricmp(map, "city3") == 0)	||
			(stricmp(map, "command") == 0)	||
			(stricmp(map, "cool1") == 0)	||
			(stricmp(map, "fact1") == 0)	||
			(stricmp(map, "fact2") == 0)	||
			(stricmp(map, "fact3") == 0)	||
			(stricmp(map, "hangar1") == 0)	||
			(stricmp(map, "hangar2") == 0)	||
			(stricmp(map, "jail1") == 0)	||
			(stricmp(map, "jail2") == 0)	||
			(stricmp(map, "jail3") == 0)	||
			(stricmp(map, "jail4") == 0)	||
			(stricmp(map, "jail5") == 0)	||
			(stricmp(map, "lab") == 0)		||
			(stricmp(map, "match1") == 0)	||
			(stricmp(map, "mine1") == 0)	||
			(stricmp(map, "mine2") == 0)	||
			(stricmp(map, "mine3") == 0)	||
			(stricmp(map, "mine4") == 0)	||
			(stricmp(map, "mintro") == 0)	||
			(stricmp(map, "power1") == 0)	||
			(stricmp(map, "power2") == 0)	||
			(stricmp(map, "q2dm1") == 0)	||
			(stricmp(map, "q2dm2") == 0)	||
			(stricmp(map, "q2dm3") == 0)	||
			(stricmp(map, "q2dm4") == 0)	||
			(stricmp(map, "q2dm5") == 0)	||
			(stricmp(map, "q2dm6") == 0)	||
			(stricmp(map, "q2dm7") == 0)	||
			(stricmp(map, "q2dm8") == 0)	||
			(stricmp(map, "security") == 0)	||
			(stricmp(map, "space") == 0)	||
			(stricmp(map, "strike") == 0)	||
			(stricmp(map, "train") == 0)	||
			(stricmp(map, "ware1") == 0)	||
			(stricmp(map, "ware2") == 0)	||
			(stricmp(map, "waste1") == 0)	||
			(stricmp(map, "waste2") == 0)	||
			(stricmp(map, "waste3") == 0)
			)
		{
			iTrust = 1;	//valid map
		}
		else
		{
			//no a valid map at this time
			gi.cprintf (ent, PRINT_HIGH,"\n'%s' is not a valid map to warp to.\n", map);
			return;
		}
	}

	if (ent->client->resp.adminflag)
	{	// if the person is an Admin, just do it
		Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", map);
		gi.AddCommandString (command);
		return;
	}

	// did is pass the conditions?
	if (iTrust ==0)
		return;

	// see if one is already happening
	if (FindVote ())
	{
		gi.cprintf (ent, PRINT_HIGH,"\nVote already in progress.\n");
		return;
	}

	// start by spawning a vote entity
	vote = G_Spawn();
	vote->classname = "vote";
	vote->flags = CM_VOTETYPE_WARP;
	vote->mass = 0;		// votes yes
	vote->health = 0;	// votes no
	vote->target = ent->client->pers.netname;	// requestor's name
	strcpy (compmod.nextmap, map);
	vote->nextthink = level.time + CM_VOTETHINKTIME;
	vote->think = VoteThink;

	// vote yes for this person
	Cmd_Vote_f (ent, CM_VOTE_YES);
}

void Cmd_Commands_f (edict_t *ent)
{
	// this will list all valid commands
	// care whether or not the person is an Admin?
	gi.cprintf (ent, PRINT_HIGH,"\nCommands\n");
	gi.cprintf (ent, PRINT_HIGH,"================\n");
	if (ent->client->resp.adminflag == CM_ADMINFLAG_NOT)
		gi.cprintf (ent, PRINT_HIGH,"admin <code>\n");
	gi.cprintf (ent, PRINT_HIGH,"commands\n");
	gi.cprintf (ent, PRINT_HIGH,"contact\n");
	gi.cprintf (ent, PRINT_HIGH,"credits\n");
	if (ent->client->resp.adminflag)
		gi.cprintf (ent, PRINT_HIGH,"dropquad <opt>\n");
	if (ent->client->resp.adminflag == CM_ADMINFLAG_NOT)
		gi.cprintf (ent, PRINT_HIGH,"elect\n");
	if (ent->client->resp.adminflag)
	{
		gi.cprintf (ent, PRINT_HIGH,"endmatch\n");
		gi.cprintf (ent, PRINT_HIGH,"ff <opt>\n");
		gi.cprintf (ent, PRINT_HIGH,"lockdown <opt>\n");
		gi.cprintf (ent, PRINT_HIGH,"matchfragset <n>\n");
		gi.cprintf (ent, PRINT_HIGH,"matchtimeset <n>\n");
		gi.cprintf (ent, PRINT_HIGH,"modeset\n");
	}
	gi.cprintf (ent, PRINT_HIGH,"modstatus\n");
	gi.cprintf (ent, PRINT_HIGH,"motd\n");
	gi.cprintf (ent, PRINT_HIGH,"myscore\n");
	gi.cprintf (ent, PRINT_HIGH,"no\n");
	if (ent->client->resp.adminflag)
		gi.cprintf (ent, PRINT_HIGH,"normal\n");
	gi.cprintf (ent, PRINT_HIGH,"notready\n");
	gi.cprintf (ent, PRINT_HIGH,"player\n");
	gi.cprintf (ent, PRINT_HIGH,"playerlist\n");
	if (ent->client->resp.adminflag)
		gi.cprintf (ent, PRINT_HIGH,"powerups <cmd>\n");
	gi.cprintf (ent, PRINT_HIGH,"ready\n");
	if (ent->client->resp.adminflag)
		gi.cprintf (ent, PRINT_HIGH,"restart\n");
	gi.cprintf (ent, PRINT_HIGH,"spectator\n");
	gi.cprintf (ent, PRINT_HIGH,"team <name>\n");
	gi.cprintf (ent, PRINT_HIGH,"teamlist\n");
	if (ent->client->resp.adminflag)
	{
		gi.cprintf (ent, PRINT_HIGH,"timerset <n>\n");
		gi.cprintf (ent, PRINT_HIGH,"warp <map>\n");
	}
	gi.cprintf (ent, PRINT_HIGH,"yes\n");
	gi.cprintf (ent, PRINT_HIGH,"================\n");
}

void Cmd_Credits_f (edict_t *ent)
{
	gi.cprintf (ent, PRINT_HIGH,"\nCredits (alphabetical)\n");
	gi.cprintf (ent, PRINT_HIGH,"=================\n");
	gi.cprintf (ent, PRINT_HIGH,"Avalon          - My wife, my life\n");
	gi.cprintf (ent, PRINT_HIGH,"Inside 3D       - for the site\n");
	gi.cprintf (ent, PRINT_HIGH,"Mike-D          - for the Linux version\n");
	gi.cprintf (ent, PRINT_HIGH,"QDeveLS         - for the site\n");
	gi.cprintf (ent, PRINT_HIGH,"StraT           - for the encouragement\n");
	gi.cprintf (ent, PRINT_HIGH,"TeleFragged     - THE Quake site\n");
	gi.cprintf (ent, PRINT_HIGH,"Zoran           - for the Solaris version\n");
	gi.cprintf (ent, PRINT_HIGH,"=================\n");
}

void StuffCommand (edict_t *ent, char *command)
{
	gi.WriteByte (CM_LOCALCOMMAND);
	gi.WriteString (command);
	gi.unicast (ent, true);
}

int MatchInProgress (edict_t *ent)
{
	if (compmod.serverstatus == CM_MATCHINPROGRESS || compmod.serverstatus == CM_MATCHCOUNTDOWN)
	{
		gi.cprintf (ent, PRINT_MEDIUM, "\nCannot change. Match in Progress.\n");
		return 1;
	}
	else
		return 0;
}

int NotAnAdmin (edict_t *ent)
{
	if (ent != NULL)
	{
		if (ent->client->resp.adminflag == CM_ADMINFLAG_NOT)
		{
			gi.cprintf (ent, PRINT_MEDIUM, "\nYou are not an Admin.\n");
			gi.cprintf (ent, PRINT_MEDIUM, "To become an Admin, use the command:\n");
			gi.cprintf (ent, PRINT_MEDIUM, "'admin <code>' - where <code> is the Admin code.\n");
			return 1;
		}
		else
			return 0;
	}
	else
		return 0;
}

edict_t *MakeClan (int iClanNumber)
{
	edict_t	*clan;

	clan = G_Spawn();
	clan->classname = "clan";
	clan->flags = iClanNumber;		// clan number, name is in team[clan->flags].teamname
	clan->mass = 0;					// number of ready players in the clan
	clan->health = 0;				// frags for a clan
	clan->max_health = 0;			// enemy kills for a clan
	clan->gib_health = 0;			// friendly kills for a clan
	clan->deadflag = 0;				// deaths for a clan
	return clan;
}

edict_t *FindClan (int iClanNumber)
{
	edict_t	*clan = NULL;

	while ((clan = G_Find (clan, FOFS(classname), "clan")) != NULL)
	{
		if (clan->flags == iClanNumber)
			return clan;
	}
	return NULL;
}

void SpawnTimer (void)
{
	edict_t	*timer;

	if (FindTimer ())
		return;

	timer = G_Spawn();
	timer->classname = "timer";
	timer->timestamp = level.time + 600;	// actual time in rally/match/etc.
	timer->nextthink = level.time + 1;		// this means think in a second
	timer->think = TimerThink;
}

edict_t *FindTimer (void)
{
	edict_t	*e;

	e = G_Find (NULL, FOFS(classname), "timer");
	if (e)
		return e;
	else
		return NULL;
}

void TimerThink (edict_t *ent)
{	// This will contain ALL of the handling for the matches.
	// For FFA, normal Q2 code should handle it.
	char	s = 's';
	edict_t	*clan = NULL;
	int		iTimeRemaining;
	int		iMinutesRemaining;

	ent->nextthink = level.time + 1; // this means every second
	iTimeRemaining = ent->timestamp - level.time;

	if (compmod.serverstatus == CM_FFA)
		return;
	else if (compmod.serverstatus == CM_MATCHRALLY)
	{
		// check to see if all are ready, if so, switch to countdown
		if (AllReady () && PlayerCount (PC_PLAYERSONLY))
		{
			gi.bprintf (PRINT_HIGH, "\nMatch begins in 30 seconds. Good Luck.\n");
			compmod.serverstatus = CM_MATCHCOUNTDOWN;
			ent->timestamp = level.time + 30;
			return;
		}
		if (iTimeRemaining <= 0)
		{
			// check to see if its needed to stay in match mode
			if (lockcurrmode->value)
				// just restart the server, and it will be stay in Match
				gi.bprintf (PRINT_HIGH, "\nServer is restarting.\n");
			else
			{
				// reset server to FFA because pre-DM rally time has lapsed
				gi.bprintf (PRINT_HIGH, "\nServer is switching to Free-For-All mode.\n");
				compmod.serverstatus = CM_FFA;
				ent->timestamp = 0;
			}
			RestartServer ();
		}
		else if (iTimeRemaining % 60 == 0)
		{
			iMinutesRemaining = iTimeRemaining / 60;
			if (iMinutesRemaining == 1)
				s = ' ';
			gi.bprintf (PRINT_MEDIUM,"Server will reset in %d minute%c\n", iMinutesRemaining, s);
		}
		else if (iTimeRemaining <= 15.0)
		{
			if (iTimeRemaining == 1)
				s = ' ';
			gi.bprintf (PRINT_MEDIUM,"Server will reset in %d second%c\n", iTimeRemaining, s);
		}
	}
	else if (compmod.serverstatus == CM_MATCHINPROGRESS)
	{
		// match is running
		if (iTimeRemaining <= 0)
		{
			// match over
			gi.bprintf (PRINT_HIGH, "\nThe match is over.\n");
			ApplyToAllPlayers (ATAP_GIB, 0, 0);			// gib everyone
			ApplyToAllPlayers (ATAP_DUMPSCORES, 0, 0);	// dump scores to each player
			DeclareWinner ();
			MakeClansUsed ();
			compmod.serverstatus = CM_MATCHRALLY;	// reset the timer
			ent->timestamp = level.time + (compmod.rallytime * 60);
			gi.bprintf (PRINT_HIGH, "\nServer will reset in %i minutes\n", compmod.rallytime);
		}
		else if (iTimeRemaining % 60 == 0)
		{
			iMinutesRemaining = iTimeRemaining / 60;
			if (iMinutesRemaining == 1)
				s = ' ';
			gi.bprintf (PRINT_MEDIUM,"\nMatch ends in %d minute%c\n", iMinutesRemaining, s);
			if (scorecast->value)
			{
				while ((clan = G_Find (clan, FOFS(classname), "clan")) != NULL)
				{
					if (clan->health == 1)
						s = ' ';
					else
						s = 's';
					gi.bprintf (PRINT_HIGH, "Clan %s: %i frag%c\n", team[clan->flags].teamname, clan->health, s);
				}
			}
		}
		else if (iTimeRemaining <= 15.0)
		{
			if (iTimeRemaining == 1)
				s = ' ';
			gi.bprintf (PRINT_MEDIUM,"Match ends in %d second%c\n", iTimeRemaining, s);
		}
		if (compmod.matchfrags && iTimeRemaining > 0)
		{
			while ((clan = G_Find (clan, FOFS(classname), "clan")) != NULL)
			{
				if (clan->health >= compmod.matchfrags)
				{
					// fraglimit hit
					gi.bprintf (PRINT_HIGH, "\nFraglimit hit.\n");
					ent->timestamp = level.time;
					ent->nextthink = level.time + 0.1;
				}
			}
		}
	}
	else if (compmod.serverstatus == CM_MATCHCOUNTDOWN)
	{
		// countdown to start of match
		if (!AllReady ())
		{
			gi.bprintf (PRINT_HIGH, "\nServer is now in Match mode.\n");
			compmod.serverstatus = CM_MATCHRALLY;
			ent->timestamp = level.time + (compmod.rallytime * 60);
			return;
		}
		if (iTimeRemaining <= 0)
		{
			// start the match
			gi.bprintf (PRINT_HIGH, "\nGame on!\n");
			ApplyToAllPlayers (ATAP_CLEARREADY, 0, 0);
			ApplyToAllPlayers (ATAP_CLEARSCORES, 0, 0);
			ApplyToAllPlayers (ATAP_GIB, 0, 0);
			ApplyToAllClans (ATAC_CLEARSCORES);
			ApplyToAllClans (ATAC_DESTROYZEROCLANS);
			ApplyToAllClans (ATAC_KILLUSEDCLANS);
			compmod.serverstatus = CM_MATCHINPROGRESS;
			// unlock the server
			compmod.lockdown = CM_LOCKDOWN_OFF;
			ent->timestamp = level.time + compmod.matchtime * 60;
			iTimeRemaining = ent->timestamp - level.time;
			iMinutesRemaining = iTimeRemaining / 60;
			gi.bprintf (PRINT_MEDIUM,"Match ends in %d minutes\n", iMinutesRemaining);
		}
		else if (iTimeRemaining <= 15.0)
		{
			if (iTimeRemaining == 1)
				s = ' ';
			gi.bprintf (PRINT_MEDIUM, "Match starts in %d second%c\n", iTimeRemaining, s);
		}
	}
}

int AllReady (void)
{
	edict_t	*player = NULL;
	int		iNotReadyCount = 0;

	while ((player = G_Find (player, FOFS(classname), "player")) != NULL)
	{
		if (player->client->resp.ready == CM_NOTREADY)
		{
			iNotReadyCount++;
			break; // break out of while because one not ready...
		}
	}
	if (iNotReadyCount || PlayerCount (PC_ALLCLIENTS) == 0)	// anyone not ready OR 0 players
		return 0;
	else
		return 1;
}

void ApplyToAllPlayers (int iApplyCode, int iTeamNumber, int iFrags)
{
	char	s = 's'; // for plurals
	edict_t	*player = NULL;

	while ((player = G_Find (player, FOFS(classname), "player")) != NULL)
	{
		if (iApplyCode == ATAP_CLEARREADY && player->client->resp.teamnumber != CM_TEAM_SPECTATOR)
			player->client->resp.ready = CM_NOTREADY;
		else if (iApplyCode == ATAP_GIB && player->client->resp.teamnumber != CM_TEAM_SPECTATOR)
			T_Damage (player, world, world, vec3_origin, player->s.origin, vec3_origin, 100000, 1, DAMAGE_NO_PROTECTION, MOD_MATCHSTARTEND);
		else if (iApplyCode == ATAP_CLEARSCORES && player->client->resp.teamnumber != CM_TEAM_SPECTATOR)
		{
			player->client->resp.score = 0;
			player->client->resp.frags = 0;
			player->client->resp.enemykills = 0;
			player->client->resp.friendkills = 0;
			player->client->resp.deaths = 0;
		}
		else if (iApplyCode == ATAP_DUMPSCORES && player->client->resp.teamnumber != CM_TEAM_SPECTATOR)
			DumpPlayerScore (player);
		else if (iApplyCode == ATAP_BROADCASTWINNER)
		{
			if (iFrags == 1)
				s=' ';
			gi.centerprintf (player, "Clan '%s' is the WINNER\nwith %i frag%c!", team[iTeamNumber].teamname, iFrags, s);
		}
		else if (iApplyCode == ATAP_CLEARVOTESTATUS && player->client->resp.teamnumber != CM_TEAM_SPECTATOR)
			player->client->resp.voted = CM_VOTED_NOT;
	}
}

void RestartServer (void)
{
	char	command [256];

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.mapname);
	gi.AddCommandString (command);
}

void UpdatePlayerStats (edict_t *player, int iFrags, int iEnemyKills, int iFriendlyKills, int iDeaths)
{
	edict_t	*clan = NULL;

	player->client->resp.frags += iFrags;
	player->client->resp.enemykills += iEnemyKills;
	player->client->resp.friendkills += iFriendlyKills;
	player->client->resp.deaths += iDeaths;
	if (compmod.serverstatus == CM_MATCHINPROGRESS)
	{
		// do the clan stats here
		clan = FindClan (player->client->resp.teamnumber);
		if (clan)
		{
			clan->health += iFrags;				// frags for a clan
			clan->max_health += iEnemyKills;	// enemy kills for a clan
			clan->gib_health += iFriendlyKills;	// friendly kills for a clan
			clan->deadflag += iDeaths;			// deaths for a clan
		}
		else	// shit a Miata, clan entity was not found!
			gi.cprintf (player, PRINT_HIGH, "\nYour Clan does not exist!\n");
	}
}

void DumpPlayerScore (edict_t *ent)
{
	if (compmod.serverstatus == CM_FFA)
		return;

	gi.cprintf (ent, PRINT_HIGH, "\nYour Score\n");
	gi.cprintf (ent, PRINT_HIGH, "Frags|Enemies|Friends|Deaths\n");
	gi.cprintf (ent, PRINT_HIGH, "%5i|%7i|%7i|%6i\n", ent->client->resp.frags, ent->client->resp.enemykills, ent->client->resp.friendkills, ent->client->resp.deaths);
}

void DumpClanScore (edict_t *ent)
{	// need to change this to display current score in a match and the last score in rally
	edict_t	*clan = NULL;

	if (compmod.serverstatus == CM_FFA)
		return;

	clan = FindClan (ent->client->resp.teamnumber);
	if (clan)
	{
		gi.cprintf (ent, PRINT_HIGH, "\n%12s Clan score\n", team[clan->flags].teamname);
		gi.cprintf (ent, PRINT_HIGH, "Frags|Enemies|Friends|Deaths\n");
		gi.cprintf (ent, PRINT_HIGH, "%5i|%7i|%7i|%6i\n", clan->health, clan->max_health, clan->gib_health, clan->deadflag);
	}
	else
		gi.cprintf (ent, PRINT_HIGH, "\nClan does not exist.\n");
}

void MakeClansUsed (void)
{
	edict_t	*clan = NULL;
	edict_t	*usedclan = NULL;

	while ((clan = G_Find (clan, FOFS(classname), "clan")) != NULL)
	{
		usedclan = G_Spawn();
		usedclan->classname = "usedclan";
		usedclan->flags = clan->flags;				// clan number
		usedclan->mass = clan->mass;				// number of ready players in the usedclan
		usedclan->health = clan->health;			// frags for a clan
		usedclan->max_health = clan->max_health;	// enemy kills for a clan
		usedclan->gib_health = clan->gib_health;	// friendly kills for a clan
		usedclan->deadflag = clan->deadflag;		// deaths for a clan
		clan->nextthink = level.time + 0.1;
		clan->think = G_FreeEdict;					// kill the clan
	}
}

void ApplyToAllClans (int iApplyCode)
{
	edict_t	*clan = NULL;
	char	class_name[9];

	if (iApplyCode == ATAC_KILLUSEDCLANS)
		strcpy(class_name, "usedclan");
	else
		strcpy(class_name, "clan");

	while ((clan = G_Find (clan, FOFS(classname), class_name)) != NULL)
	{
		if (iApplyCode == ATAC_CLEARSCORES)
		{
			clan->health = 0;		// frags for a clan
			clan->max_health = 0;	// enemy kills for a clan
			clan->gib_health = 0;	// friendly kills for a clan
			clan->deadflag = 0;		// deaths for a clan
		}
		else if (iApplyCode == ATAC_DESTROYZEROCLANS)
		{
			if (clan->mass <= 0) // mass is used for usecount
			{
				clan->nextthink = level.time + 0.1;
				clan->think = G_FreeEdict;
			}
		}
		else if (iApplyCode == ATAC_KILLUSEDCLANS)
		{
			clan->nextthink = level.time + 0.1;
			clan->think = G_FreeEdict;
		}
	}
}

void DeclareWinner (void)
{
	char	s = 's';
	edict_t	*clan = NULL;
	int		iHighestClanNumber = 0;
	int		iHighestCount = 0;
	int		iHighestScore = -10000;

	gi.bprintf (PRINT_HIGH, "\nFINAL SCORES\n");
	while ((clan = G_Find (clan, FOFS(classname), "clan")) != NULL)
	{
		if (clan->health == 1)
			s = ' ';
		gi.bprintf (PRINT_HIGH, "Clan %s: %i frag%c\n", team[clan->flags].teamname, clan->health, s);
		if (clan->health > iHighestScore)
		{
			iHighestScore = clan->health;
			iHighestClanNumber = clan->flags;
		}
	}

	s = 's';
	if (iHighestScore == -10000)
	{
		// huh? no clans found?
		gi.dprintf("DEBUG - No clans found in DeclareWinner");
	}
	else
	{
		// loop through all the clans again and look for ties
		clan = NULL;
		while ((clan = G_Find (clan, FOFS(classname), "clan")) != NULL)
		{
			if (clan->health == iHighestScore)
				iHighestCount++;
		}
		if (iHighestCount > 1)
		{
			if (iHighestScore == 1)
				s = ' ';
			gi.bprintf (PRINT_HIGH, "\nWe have a TIE with %i frag%c!\nHere are the Clans:\n", iHighestScore, s);
			clan = NULL;
			while ((clan = G_Find (clan, FOFS(classname), "clan")) != NULL)
			{
				if (clan->health == iHighestScore)
					gi.bprintf (PRINT_HIGH, "Clan %s\n", team[clan->flags].teamname);
			}
		}
		else
		{
			ApplyToAllPlayers (ATAP_BROADCASTWINNER, iHighestClanNumber, iHighestScore);	// broadcast the winning clan
		}
	}
}

int AssignTeam (char *szTeamName)
{
	int	iFound = 0;
	int	iNotInUse = 0;
	int	iLoop;

	for (iLoop = 1; iLoop < 17; iLoop++)	// starts at 1 because 0 means not selected
	{
		if (Q_stricmp (team[iLoop].teamname, szTeamName) == 0)
			return iLoop;
		if (iFound == 0 && Q_stricmp (team[iLoop].teamname, "") == 0)
		{
			iFound = 1;
			iNotInUse = iLoop;
		}
	}
	strcpy(team[iNotInUse].teamname, szTeamName);
	return iNotInUse;
}

void ApplyToAllAdmins (int iApplyCode)
{
	edict_t	*player = NULL;

	while ((player = G_Find (player, FOFS(classname), "player")) != NULL)
	{
		if (player->client->resp.adminflag)
		{
			if (iApplyCode == ATAA_REPORTREFUSEDCONNECT)
				gi.cprintf (player, PRINT_HIGH, "Attempted connection was refused.\n");
			else
				gi.dprintf ("DEBUG - ApplyToAllAdmins - invalid code %i", iApplyCode);
		}
	}
}

void EndMatch (void)
{
	edict_t *timer;

	if (PlayerCount (PC_PLAYERSONLY))
	{
		ApplyToAllPlayers (ATAP_GIB, 0, 0);			// gib everyone
		ApplyToAllPlayers (ATAP_DUMPSCORES, 0, 0);	// dump scores to each player
	}
	MakeClansUsed ();
	compmod.serverstatus = CM_MATCHRALLY;		// reset the timer
	timer = FindTimer();
	if (timer)
		timer->timestamp = level.time + 600;
	gi.bprintf (PRINT_HIGH, "\nServer will reset in 10 minutes.\n");
}

int PlayerCount (int iMode)
{	// there has to be a better way than this...
	edict_t *player = NULL;
	int		iCount = 0;

	while ((player = G_Find (player, FOFS(classname), "player")) != NULL)
	{
		if (player->client)
		{
			if (iMode == PC_PLAYERSONLY && player->client->resp.teamnumber != CM_TEAM_SPECTATOR)
				iCount++;
			else if (iMode == PC_SPECTATORSONLY && player->client->resp.teamnumber == CM_TEAM_SPECTATOR)
				iCount++;
			else if (iMode == PC_ALLCLIENTS)
				iCount++;
		}
	}

	return iCount;
}

int TestPlayerCount (void)
{
	int	iCount = 0;
	int	iLoop;

	for (iLoop = 1; iLoop <= maxclients->value; iLoop++)
		if (g_edicts[iLoop].inuse && g_edicts[iLoop].client)
			iCount++;

	return iCount;
}

edict_t *FindVote (void)
{
	edict_t	*e;

	e = G_Find (NULL, FOFS(classname), "vote");
	if (e)
		return e;
	else
		return NULL;
}

void VoteThink (edict_t *ent)
{
	ent->nextthink = level.time + CM_VOTETHINKTIME;
	// this has to happen in case someone disconnects during a vote
	EvaluateVote (ent);
}

void EvaluateVote (edict_t *vote)
{
	char	command [256];
	edict_t	*player = NULL;
	float	fPlayerCount;	// float so we are not doing integer divided by integer - duh
	int		iFlags;

	fPlayerCount = PlayerCount (PC_PLAYERSONLY);
	if ((vote->mass / fPlayerCount) > 0.5)
	{
		iFlags = vote->flags;
		KillVote (vote);
		// we have a majority, so do the Dew
		gi.bprintf (PRINT_HIGH, "\nThe vote has passed.\n");
		if (iFlags == CM_VOTETYPE_ADMIN)
		{
			while ((player = G_Find (player, FOFS(classname), "player")) != NULL)
			{
				if (Q_stricmp (player->client->pers.netname, vote->target) == 0)
				{
					player->client->resp.adminflag = CM_ADMINFLAG_ELECTED;
					player->client->resp.adminerrorcount = 0;
					gi.bprintf (PRINT_HIGH, "%s is now an Admin.\n", vote->target);
					break;
				}
			}
		}
		else if (iFlags == CM_VOTETYPE_WARP)
		{
			// do the warp
			Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", compmod.nextmap);
			gi.AddCommandString (command);
		}
		else if (iFlags == CM_VOTETYPE_ENDMATCH)
			EndMatch ();
		return;
	}
	else if ((vote->health / fPlayerCount) > 0.5)
	{
		KillVote (vote);
		gi.bprintf (PRINT_HIGH, "\nThe vote has been defeated.\n");
		// in future count rejected admin elects and ban the command from that loser ;)
		return;
	}
	else if ((vote->mass / fPlayerCount) == 0.5 && (vote->mass + vote->health == fPlayerCount))
	{
		KillVote (vote);
		gi.bprintf (PRINT_HIGH, "\nThe vote is a TIE, so it is cancelled.\n");
		return;
	}

	if (vote->flags == CM_VOTETYPE_NONE) // uhhhh, just in case?
	{
		KillVote (vote);
		return;
	}
	//display status
	gi.bprintf (PRINT_HIGH, "\n%s has requested ", vote->target);
	if (vote->flags == CM_VOTETYPE_ADMIN)
		gi.bprintf (PRINT_HIGH, "Admin status.\n");
	else if (vote->flags == CM_VOTETYPE_WARP)
		gi.bprintf (PRINT_HIGH, "a level change to '%s'.\n", compmod.nextmap);
	else if (vote->flags == CM_VOTETYPE_ENDMATCH)
		gi.bprintf (PRINT_HIGH, "to end the match.\n");
	gi.bprintf (PRINT_HIGH, "Votes YES:%i\n", vote->mass);
	gi.bprintf (PRINT_HIGH, "Votes  NO:%i\n", vote->health);
	gi.bprintf (PRINT_HIGH, "Enter 'yes' or 'no' at the console.\n");
}

void KillVote (edict_t *vote)
{
	vote->flags = CM_VOTETYPE_NONE;
	vote->nextthink = level.time + 0.1;
	vote->think = G_FreeEdict;
	ApplyToAllPlayers (ATAP_CLEARVOTESTATUS, 0, 0);
}

int CountClans (void)
{
	edict_t	*clan = NULL;
	int iClanCount = 0;

	while ((clan = G_Find (clan, FOFS(classname), "clan")) != NULL)
	{
		if (clan->mass)
			iClanCount++;
	}
	return iClanCount;
}

void DisableFlagSet (int iEnableDisable, int iDisableFlags)
{
	int iFlags;

	// read in the proper CVar
	if (compmod.serverstatus == CM_FFA)
		iFlags = ffadisable->value;
	else
		iFlags = matchdisable->value;

	if (iEnableDisable == CM_DISABLE)	// disable stuff
		iFlags |= iDisableFlags; 
	else	// enable stuff
		iFlags &= ~iDisableFlags; 

	if (compmod.serverstatus == CM_FFA)
		gi.cvar_set("ffadisable", va("%i", iFlags));
	else
		gi.cvar_set("matchdisable", va("%i", iFlags));
}

int IsItDisabled (edict_t *ent)
{
	int iFlags;

	// read in the proper CVar
	if (compmod.serverstatus == CM_FFA)
		iFlags = ffadisable->value;
	else
		iFlags = matchdisable->value;

	if (
		((Q_stricmp (ent->classname, "item_quad") == 0) && (iFlags & CM_DIS_QUAD))				||
		((Q_stricmp (ent->classname, "item_invulnerability") == 0) && (iFlags & CM_DIS_INV))	||
		((Q_stricmp (ent->classname, "weapon_bfg") == 0) && (iFlags & CM_DIS_BFG))				||
		((Q_stricmp (ent->classname, "weapon_railgun") == 0) && (iFlags & CM_DIS_RAIL))			||
		((Q_stricmp (ent->classname, "weapon_hyperblaster") == 0) && (iFlags & CM_DIS_HYPER))	||
		((Q_stricmp (ent->classname, "weapon_rocketlauncher") == 0) && (iFlags & CM_DIS_RL))	||
		((Q_stricmp (ent->classname, "weapon_grenadelauncher") == 0) && (iFlags & CM_DIS_GL))	||
		((Q_stricmp (ent->classname, "ammo_grenades") == 0) && (iFlags & CM_DIS_GREN))			||
		((Q_stricmp (ent->classname, "weapon_chaingun") == 0) && (iFlags & CM_DIS_CHAIN))		||
		((Q_stricmp (ent->classname, "weapon_machinegun") == 0) && (iFlags & CM_DIS_MG))		||
		((Q_stricmp (ent->classname, "weapon_supershotgun") == 0) && (iFlags & CM_DIS_SSG))		||
		((Q_stricmp (ent->classname, "weapon_shotgun") == 0) && (iFlags & CM_DIS_SG))
		)
	{
		return 1;
	}
	else
		return 0;
}