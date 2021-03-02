/*
Copyright (C) 1997-2001 Id Software, Inc.

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
#include "g_tdm.h"
#include "g_svcmds.h"

/*
==============
TDM_RateLimited
==============
Return true if the client has used a command that prints global info or similar recently.
*/
qboolean TDM_RateLimited (edict_t *ent, int penalty)
{
	if (level.realframenum < ent->client->resp.last_command_frame)
	{
		gi.cprintf (ent, PRINT_HIGH, "Command ignored due to rate limiting, wait a little longer.\n");
		return true;
	}

	ent->client->resp.last_command_frame = level.realframenum + penalty;
	return false;
}

/*
==============
TDM_ForceReady_f
==============
Force everyone to be ready/notready, admin command.
*/
static void TDM_ForceReady_f (qboolean status)
{
	edict_t	*ent;

	for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		if (!ent->inuse)
			continue;

		if (!ent->client->pers.team)
			continue;

		ent->client->resp.ready = status;
	}

	TDM_CheckMatchStart ();
}

/*
==============
TDM_StartMatch_f
==============
Start the match right away.
*/
/*void TDM_StartMatch_f (edict_t *ent)
{
	if (!teaminfo[TEAM_A].players || !teaminfo[TEAM_B].players)
	{
		gi.cprintf (ent, PRINT_HIGH, "Not enough players to start the match.\n");
		return;
	}

	TDM_BeginMatch ();
}*/

/*
==============
TDM_Commands_f
==============
Show brief help on all commands
*/
void TDM_Commands_f (edict_t *ent)
{
	gi.cprintf (ent, PRINT_HIGH,
		"General\n"
		"-------\n"
		"menu           Show OpenTDM menu\n"
		"join           Join a team\n"
		"vote           Propose new settings\n"
		"accept         Accept a team invite\n"
		"captain        Show / become / set captain\n"
		"settings       Show match settings\n"
		"ready          Set ready\n"
		"notready       Set not ready\n"
		"time           Call a time out\n"
		"chase          Enter chasecam mode\n"
		"overtime       Show overtime\n"
		"timelimit      Show timelimit\n"
		"bfg            Show bfg settings\n"
		"powerups       Show powerups settings\n"
		"obsmode        Show obsmode settins\n"
		"\n"
		"Team Captains\n"
		"-------------\n"
		"time           Call timeout\n"
		"teamname       Change team name\n"
		"teamskin       Change team skin\n"
		"invite         Invite a player\n"
		"pickplayer     Pick a player\n"
		"lockteam       Lock team\n"
		"unlockteam     Unlock team\n"
		"teamready      Force team ready\n"
		"teamnotready   Force team not ready\n"
		"kickplayer     Remove a player from team\n"
		);
}

/*
==============
TDM_Acommands_f
==============
Show brief help on all commands
*/
void TDM_Acommands_f (edict_t *ent)
{
	gi.cprintf (ent, PRINT_HIGH,
		"Admin commands\n"
		"-------\n"
		"acommands          Show OpenTDM admin commands\n"
		"tl <mins>          Change match timelimit\n"
		"powerups 0/1       Enable powerups\n"
		"bfg 0/1            Enable BFG10k\n"
		"obsmode 0/1/2      Enable observer talk during the match\n"
		"break              End current match\n"
		"hold               Pause current match\n"
		"changemap <map>    Change current map\n"
		"overtime 0/1/2     Set overtime\n"
		"kick / boot <id>   Kick a player from server\n"
		"kickban <id>       kickban a player from server\n"
		"ban <id> [mins]    Ban ip on the server, default for 1 hour\n"
		"unban <ip>         Unban ip on the server\n"
		"bans               Show current bans\n"
		"readyall           Force all players to be ready\n"
//		"forceteam          Force team\n"
		"notreadyall        Force all players not to be ready\n"
		);
}

/*
==============
TDM_Timelimit_f
==============
Set the timelimit.
*/
void TDM_Timelimit_f (edict_t *ent)
{
	unsigned	limit;
	const char	*input;
	char 		seconds[16];

	input = gi.argv(1);

	if (!input[0] || !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Timelimit is %d minute%s.\n", (int)g_match_time->value/60, (int)g_match_time->value/60 == 1 ? "" : "s");
		return;
	}

	limit = strtoul (input, NULL, 10);
	if (limit < 1)
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid timelimit value '%s'.\n", input);
		return;
	}

	tdm_settings_not_default = true;

	sprintf (seconds, "%d", limit * 60);
	g_match_time = gi.cvar_set ("g_match_time", seconds);

	if (tdm_match_status >= MM_PLAYING && tdm_match_status < MM_SCOREBOARD)
	{
		current_matchinfo.timelimit = g_match_time->value / 60;
		level.match_end_framenum = level.match_start_framenum + (int)(g_match_time->value * SERVER_FPS);
	}

	//wision: FIXME: remove _only_ timelimit vote (fix other commands too)
	if (vote.flags & VOTE_TIMELIMIT)
		TDM_RemoveVote ();

	gi.bprintf (PRINT_HIGH, "Timelimit set to %s.\n", input);
}

/*
==============
TDM_Powerups_f
==============
Enable / disable all powerups. For better weapon settings, use vote.
*/
void TDM_Powerups_f (edict_t *ent)
{
	unsigned	i;
	unsigned	flags;
	const char	*input;
	char		value[16];
	static char	settings[100];

	settings[0] = 0;

	input = gi.argv(1);

	if (!input[0] || !ent->client->pers.admin)
	{
		for (i = 0; i < sizeof(powerupvotes) / sizeof(powerupvotes[0]); i++)
		{
			if ((int)g_powerupflags->value & powerupvotes[i].value)
			{
				strcat (settings, TDM_SetColorText (va ("%s", powerupvotes[i].names[0])));
				strcat (settings, " ");
			}
		}
		gi.cprintf (ent, PRINT_HIGH, "Removed powerups: %s\n", settings[0] == '\0' ? "none" : settings);
		return;
	}

	if (tdm_match_status > MM_COUNTDOWN && tdm_match_status < MM_SCOREBOARD)
	{
		gi.cprintf (ent, PRINT_HIGH, "You cannot change powerups during the match.\n");
		return;
	}

	if (!Q_stricmp (input, "0"))
		flags = 0xFFFFFFFFU;
	else if (!Q_stricmp (input, "1"))
		flags = 0;
	else
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: powerups 1/0.\n");
		return;
	}


	if ((unsigned)g_powerupflags->value == flags)
	{
		gi.cprintf (ent, PRINT_HIGH, "Powerups are already set to %s.\n", input);
		return;
	}

	tdm_settings_not_default = true;

	sprintf (value, "%d", flags);
	g_powerupflags = gi.cvar_set ("g_powerupflags", value);
	TDM_ResetLevel ();

	if (vote.flags & VOTE_POWERUPS)
		TDM_RemoveVote ();

	gi.bprintf (PRINT_HIGH, "Powerups set to %s.\n", input);
}

/*
==============
TDM_Bfg_f
==============
Enable / disable bfg.
*/
void TDM_Bfg_f (edict_t *ent)
{
	unsigned	flags;
	const char	*input;
	char		value[16];

	input = gi.argv(1);
	flags = (unsigned)g_itemflags->value;

	if (!input[0] || !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Bfg is set to %d.\n", (unsigned)g_itemflags->value & WEAPON_BFG10K ? 0 : 1);
		return;
	}

	if (tdm_match_status > MM_COUNTDOWN && tdm_match_status < MM_SCOREBOARD)
	{
		gi.cprintf (ent, PRINT_HIGH, "You cannot change bfg settings during the match.\n");
		return;
	}

	if (!Q_stricmp (input, "0"))
		flags |= WEAPON_BFG10K;
	else if (!Q_stricmp (input, "1"))
		flags &= ~WEAPON_BFG10K;
	else
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: bfg 1/0.\n");
		return;
	}

	if ((unsigned)g_itemflags->value == flags)
	{
		gi.cprintf (ent, PRINT_HIGH, "Bfg is already set to %s.\n", input);
		return;
	}

	tdm_settings_not_default = true;

	sprintf (value, "%d", flags);
	g_itemflags = gi.cvar_set ("g_itemflags", value);
	TDM_ResetLevel ();

	if (vote.flags & VOTE_WEAPONS)
		TDM_RemoveVote ();

	gi.bprintf (PRINT_HIGH, "Bfg set to %s.\n", input);
}

/*
==============
TDM_Changemap_f
==============
Change current map.
*/
void TDM_Changemap_f (edict_t *ent)
{
	const char	*mapname;

	mapname = gi.argv(1);
	if (!mapname[0])
	{
		if (tdm_maplist != NULL)
			TDM_WriteMaplist (ent);

		gi.cprintf (ent, PRINT_HIGH, "Usage: changemap <mapname>\n");
		return;
	}

	if (!TDM_Checkmap(ent, mapname))
		return;

	// safe, checkmap checks length
	strcpy (level.nextmap, mapname);

	gi.bprintf (PRINT_HIGH, "New map: %s\n", mapname);
	EndDMLevel();
}

/*
==============
TDM_Overtime_f
==============
Set overtime.
*/
void TDM_Overtime_f (edict_t *ent)
{
	int		tiemode;
	unsigned	overtimemins = 0;
	const char	*input;
	const char	*time;
	char		value[16];
	char		time_value[16];

	input = gi.argv(1);

	if (!input[0] || !ent->client->pers.admin)
	{
		if (g_tie_mode->value == 1)
			gi.cprintf (ent, PRINT_HIGH, "Overtime is set to %d minute%s.\n", (int)g_overtime->value/60, (int)g_overtime->value/60 == 1 ? "" : "s");
		else
			gi.cprintf (ent, PRINT_HIGH, "Game is set to %s.\n", (int)g_tie_mode->value == 0 ? "tie mode" : "sudden death");
		return;
	}
	else if (!Q_stricmp (input, "0"))
	{
		tiemode = 0;
	}
	else if (!Q_stricmp (input, "1"))
	{
		time = gi.argv(2);
		if (!time[0])
		{
			gi.cprintf (ent, PRINT_HIGH, "Usage: overtime 0/1/2 <minutes> (tie/overtime/sudden death)\n");
			return;
		}
		overtimemins = strtoul (time, NULL, 10);
		tiemode = 1;
	}
	else if (!Q_stricmp (input, "2"))
	{
		tiemode = 2;
	}
	else
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: overtime 0/1/2 <minutes> (tie/overtime/sudden death)\n");
		return;
	}

	if (g_tie_mode->value == tiemode && tiemode == 1)
	{
		if (g_overtime->value == overtimemins)
		{
			gi.cprintf (ent, PRINT_HIGH, "That overtime is already set!\n");
			return;
		}
	}
	else if (g_tie_mode->value == tiemode)
	{
		gi.cprintf (ent, PRINT_HIGH, "That tie mode is already set!\n");
		return;
	}

	tdm_settings_not_default = true;

	sprintf (value, "%d", tiemode);
	g_tie_mode = gi.cvar_set ("g_tie_mode", value);

	if (vote.flags & VOTE_OVERTIME)
		TDM_RemoveVote ();

	if (tiemode == 1)
	{
		sprintf (time_value, "%d", overtimemins * 60);
		g_overtime = gi.cvar_set ("g_overtime", time_value);
		gi.bprintf (PRINT_HIGH, "New overtime: %d minute%s\n", (int)overtimemins, overtimemins == 1 ? "" : "s");
	}
	else
		gi.bprintf (PRINT_HIGH, "New tie mode: %s\n", tiemode == 0 ? "no overtime" : "sudden death");
}

/*
==============
TDM_Break_f
==============
Show scoreboard and end the match.
*/
void TDM_Break_f (edict_t *ent)
{
	if (tdm_match_status < MM_PLAYING || tdm_match_status == MM_SCOREBOARD)
		return;

	gi.bprintf (PRINT_HIGH, "%s has ended the match.\n", ent->client->pers.netname);
	TDM_EndMatch();
}

/*
==============
TDM_Bans_f
==============
Show current bans on the server.
*/
void TDM_Bans_f (edict_t *ent)
{
	SVCmd_ListIP_f (ent);
}

/*
==============
TDM_Unban_f
==============
Remove the ban.
*/
void TDM_Unban_f (edict_t *ent)
{
	SVCmd_RemoveIP_f (ent, gi.argv(1));
}

/*
==============
TDM_Ban_f
==============
Ban an ip from the server. Default 1 hour.
*/
void TDM_Ban_f (edict_t *ent)
{
	int			time = 60;
	const char	*input;
	
	if (gi.argc() > 2)
	{
		input = gi.argv(2);
		time = strtoul (input, NULL, 10);

		// don't allow admins more than 12 hours
		// only real admin should be able to put permban
		if (time > 720)
			time = 720;
		else if (time <= 0)
		{
			gi.cprintf (ent, PRINT_HIGH, "You may not set permanent bans.\n");
			return;
		}
	}

	SVCmd_AddIP_f (ent, gi.argv(1), time);
}

/*
==============
TDM_Kickban_f
==============
Kickban a player from the server.
*/
void TDM_Kickban_f (edict_t *ent)
{
	edict_t	*victim;

	if (gi.argc() < 2)
	{
		gi.cprintf(ent, PRINT_HIGH, "Usage: kickban <id>\n");
		return;
	}

	if (LookupPlayer (gi.args(), &victim, ent))
	{
		if (victim->client->pers.admin)
		{
			gi.cprintf (ent, PRINT_HIGH, "You cannot kickban an admin!\n");
			return;
		}

		if (vote.flags & VOTE_KICK && vote.victim == victim)
			TDM_RemoveVote ();

		SVCmd_AddIP_f (ent, victim->client->pers.ip, 60);

		gi.AddCommandString (va ("kick %d\n", (int)(victim - g_edicts - 1)));
	}
}

/*
==============
TDM_Obsmode_f
==============
Enable/disable observer talk during the match. values: speak, whisper, shutup
*/
void TDM_Obsmode_f (edict_t *ent)
{
	const char	*input;
	int			value;
	char		*mode;

	input = gi.argv(1);

	if (!input[0] || !ent->client->pers.admin)
	{
		if (g_chat_mode->value == 0)
			mode = "speak";
		else if (g_chat_mode->value == 1)
			mode = "whisper";
		else
			mode = "shutup";
		gi.cprintf (ent, PRINT_HIGH, "Obsmode is %s.\n", mode);
		return;
	}
	else if (!Q_stricmp (input, "speak") || !Q_stricmp (input, "0"))
		value = 0;
	else if (!Q_stricmp (input, "whisper") || !Q_stricmp (input, "1"))
		value = 1;
	else if (!Q_stricmp (input, "shutup") || !Q_stricmp (input, "2"))
		value = 2;
	else
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: obsmode speak/whisper/shutup\n");
		return;
	}

	tdm_settings_not_default = true;

	g_chat_mode = gi.cvar_set ("g_chat_mode", va("%d", value));
	gi.bprintf (PRINT_HIGH, "Obsmode is set to %s.\n", input);
}


/*
==============
TDM_Team_f
==============
Show team status / join a team
*/
void TDM_Team_f (edict_t *ent)
{
	int			team;

	if (gi.argc () < 2)
	{
		if (!ent->client->pers.team)
		{
			gi.cprintf (ent, PRINT_HIGH, "You are not on a team. Use %s 1 or %s 2 to join a team.\n", gi.argv(0), gi.argv(0));
			return;
		}

		gi.cprintf (ent, PRINT_HIGH, "You are on the '%s' team.\n", teaminfo[ent->client->pers.team].name);
		return;
	}

	if (tdm_match_status != MM_WARMUP)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can't change team in the middle of a match.\n");
		return;
	}

	team = TDM_GetTeamFromArg (ent, gi.args());
	if (team == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "Unknown team: %s\n", gi.args());
		return;
	}

	if ((int)ent->client->pers.team == team)
	{
		gi.cprintf (ent, PRINT_HIGH, "You're already on that team!\n");
		return;
	}

	if (TDM_RateLimited (ent, SECS_TO_FRAMES(2)))
		return;

	if (team == 1)
		JoinTeam1 (ent);
	else if (team == 2)
		JoinTeam2 (ent);
	else
		ToggleChaseCam (ent);
}

/*
==============
TDM_Settings_f
==============
Shows current match settings.
*/
void TDM_Settings_f (edict_t *ent)
{
	gi.cprintf (ent, PRINT_HIGH, "%s", TDM_SettingsString ());
}

/*
==============
TDM_Timeout_f
==============
Call a timeout.
*/
void TDM_Timeout_f (edict_t *ent)
{
	int	time_len;

	if (!ent->client->pers.team && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only players in the match may call a time out.\n");
		return;
	}

	if (tdm_match_status == MM_TIMEOUT)
	{
		if (level.match_resume_framenum)
		{
			if (level.tdm_timeout_caller && level.tdm_timeout_caller->client == ent)
			{
				gi.cprintf (ent, PRINT_HIGH, "You may not call another time out.\n");
				return;
			}

			//someone called another timeout during the game restart countdown.
			G_StuffCmd (NULL, "stopsound\n");
			level.match_resume_framenum = 0;
		}
		else
		{
			//someone tried to resume a time out
			if (!level.tdm_timeout_caller)
			{
				//ugly fix for 0000096
				if (!ent->client->pers.admin)
				{
					gi.cprintf (ent, PRINT_HIGH, "Only an admin can resume play.\n");
					return;
				}
			}
			else if (level.tdm_timeout_caller->client == NULL)
			{
				gi.cprintf (ent, PRINT_HIGH, "Match will automatically resume once %s reconnects.\n", level.tdm_timeout_caller->name);
				return;
			}
			else if (level.tdm_timeout_caller->client != ent && !ent->client->pers.admin)
			{
				gi.cprintf (ent, PRINT_HIGH, "Only %s or an admin can resume play.\n", level.tdm_timeout_caller->name);
				return;
			}

			TDM_ResumeGame ();
			return;
		}
	}

	if (tdm_match_status < MM_PLAYING || tdm_match_status == MM_SCOREBOARD)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only call a time out during a match.\n");
		return;
	}

	if (g_max_timeout->value == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "Time out is disabled on this server.\n");
		return;
	}

	// wision: if i'm admin, i want unlimited timeout!
	// wision: check what happens if admin is just a spectator and he calls timeout
	// r1: your method didn't work, and the code isn't really designed to allow such. is an hour enough to organize the game? :)
	if (ent->client->pers.admin)
		time_len = 3600;
	else
		time_len = g_max_timeout->value;

	level.timeout_end_framenum = level.realframenum + SECS_TO_FRAMES (time_len);
	level.tdm_timeout_caller = ent->client->resp.teamplayerinfo;

	// r1: crash fix, never reset the match status to a timeout (calling time during resume timer)
	if (tdm_match_status != MM_TIMEOUT)
		level.last_tdm_match_status = tdm_match_status;

	tdm_match_status = MM_TIMEOUT;
	
	if (TDM_Is1V1 ())
		gi.bprintf (PRINT_CHAT, "%s called a time out. Match will resume automatically in %s.\n", ent->client->pers.netname, TDM_SecsToString (time_len));
	else
		gi.bprintf (PRINT_CHAT, "%s (%s) called a time out. Match will resume automatically in %s.\n", ent->client->pers.netname, teaminfo[ent->client->pers.team].name, TDM_SecsToString (time_len));

	gi.cprintf (ent, PRINT_HIGH, "Match paused. Use '%s' again to resume play.\n", gi.argv(0));
}

/*
==============
TDM_Win_f
==============
In 1v1 games, causes player who issues the command to win by forfeit. Only used when opponent
disconnects mid-match and this player doesn't want to wait for them to reconnect.
*/
void TDM_Win_f (edict_t *ent)
{
	if (!TDM_Is1V1())
	{
		gi.cprintf (ent, PRINT_HIGH, "This command may only be used in 1v1 mode.\n");
		return;
	}

	if (tdm_match_status != MM_TIMEOUT || !level.tdm_timeout_caller || level.tdm_timeout_caller->client)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only cause a forfeit during a time out caused by your opponent disconnecting.\n");
		return;
	}

	level.timeout_end_framenum = 0;
	level.match_resume_framenum = level.realframenum + 1;
}

/*
==============
TDM_Settings_f
==============
A horrible glob of settings text.
*/
char *TDM_SettingsString (void)
{
	static char	settings[1400];
	int			i;

	static const char *gamemode_text[] = {"Team Deathmatch", "Instagib Team Deathmatch", "1 vs 1 duel"};
	static const char *switchmode_text[] = {"normal", "faster", "instant", "insane", "extreme"};
	static const char *telemode_text[] = {"normal", "no freeze"};
	static const char *bugs_text[] = {"all gameplay bugs fixed", "serious gameplay bugs fixed", "no gameplay bugs fixed"};

	settings[0] = 0;

	strcat (settings, "Game mode: ");
	strcat (settings, TDM_SetColorText(va("%s\n", gamemode_text[(int)g_gamemode->value])));

	strcat (settings, "Timelimit: ");
	strcat (settings, TDM_SetColorText(va("%g minute%s\n", g_match_time->value / 60, g_match_time->value / 60 == 1 ? "" : "s")));

	strcat (settings, "Overtime: ");
	switch ((int)g_tie_mode->value)
	{
		case 0:
			strcat (settings, TDM_SetColorText(va ("disabled")));
			break;
		case 1:
			strcat (settings, TDM_SetColorText(va ("%g minute%s\n", g_overtime->value / 60, g_overtime->value / 60 == 1 ? "" : "s")));
			break;
		case 2:
			strcat (settings, TDM_SetColorText(va ("Sudden death\n")));
			break;
	}

	strcat (settings, "\n");

	strcat (settings, "Removed weapons: ");
	for (i = 0; i < sizeof(weaponvotes) / sizeof(weaponvotes[0]); i++)
	{
		if ((int)g_itemflags->value & weaponvotes[i].value)
		{
			strcat (settings, TDM_SetColorText (va ("%s", weaponvotes[i].names[1])));
			strcat (settings, " ");
		}
	}
	strcat (settings, "\n");
	
	strcat (settings, "Removed powerups: ");
	for (i = 0; i < sizeof(powerupvotes) / sizeof(powerupvotes[0]); i++)
	{
		if ((int)g_powerupflags->value & powerupvotes[i].value)
		{
			strcat (settings, TDM_SetColorText (va ("%s", powerupvotes[i].names[0])));
			strcat (settings, " ");
		}
	}
	strcat (settings, "\n");
	
	strcat (settings, "\n");


	strcat (settings, va("'%s' skin: ", teaminfo[TEAM_A].name));
	strcat (settings, TDM_SetColorText(va("%s\n", teaminfo[TEAM_A].skin)));

	strcat (settings, va("'%s' skin: ", teaminfo[TEAM_B].name));
	strcat (settings, TDM_SetColorText(va("%s\n", teaminfo[TEAM_B].skin)));

	strcat (settings, "\n");

	strcat (settings, "Weapon switch: ");
	strcat (settings, TDM_SetColorText(va("%s\n", switchmode_text[(int)g_fast_weap_switch->value])));

	strcat (settings, "Teleporter mode: ");
	strcat (settings, TDM_SetColorText(va("%s\n", telemode_text[(int)g_teleporter_nofreeze->value])));

	strcat (settings, "Gameplay bugs: ");
	strcat (settings, TDM_SetColorText(va("%s\n", bugs_text[(int)g_bugs->value])));

	if (TDM_Is1V1())
	{
		int		mode;
		strcat (settings, "1v1 Respawn mode: ");

		mode = (int)g_1v1_spawn_mode->value & ~SPAWN_RANDOM_ON_SMALL_MAPS;

		if (mode == 0)
			strcat (settings, TDM_SetColorText(va ("%s", "avoid closest (bugged)")));
		else if (mode == 1)
			strcat (settings, TDM_SetColorText(va ("%s", "avoid closest")));
		else
			strcat (settings, TDM_SetColorText(va ("%s", "random")));

		if ((int)g_1v1_spawn_mode->value & SPAWN_RANDOM_ON_SMALL_MAPS)
			strcat (settings, TDM_SetColorText(va ("%s", " (random on small maps)")));
	}
	else
	{
		strcat (settings, "TDM Respawn mode: ");
		if (g_tdm_spawn_mode->value == 0)
			strcat (settings, TDM_SetColorText(va ("%s", "avoid closest (bugged)")));
		else if (g_tdm_spawn_mode->value == 1)
			strcat (settings, TDM_SetColorText(va ("%s", "avoid closest")));
		else
			strcat (settings, TDM_SetColorText(va ("%s", "random")));
	}
	strcat (settings, "\n");

	return settings;
}

/*
==============
TDM_SV_ApplySettings_f
==============
A config was execed, update our internal state.
*/
void TDM_SV_ApplySettings_f (void)
{
	if (g_gamemode->latched_string) //can't use latched_string directly, cvar_forceset frees it
		gi.cvar_forceset ("g_gamemode", va ("%d", atoi(g_gamemode->latched_string)));

	if (g_gamemode->value == GAMEMODE_ITDM)
		dmflags = gi.cvar_set ("dmflags", g_itdmflags->string);
	else if (g_gamemode->value == GAMEMODE_TDM)
		dmflags = gi.cvar_set ("dmflags", g_tdmflags->string);
	else if (g_gamemode->value == GAMEMODE_1V1)
		dmflags = gi.cvar_set ("dmflags", g_1v1flags->string);

	TDM_ResetGameState ();
	TDM_UpdateConfigStrings (true);
}

/*
==============
TDM_SV_Settings_f
==============
Server admin wants to see current settings.
*/
void TDM_SV_Settings_f (void)
{
	char	*settings;
	size_t	len;
	int		i;

	settings = TDM_SettingsString ();

	//no colortext for the console
	len = strlen (settings);
	for (i = 0; i < len; i++)
		settings[i] &= ~0x80;

	gi.dprintf ("%s", settings);
}

/*
==============
TDM_SV_SaveDefaults_f
==============
Server admin changed cvars and wants the new ones to be default.
*/
void TDM_SV_SaveDefaults_f (void)
{
	TDM_SaveDefaultCvars ();
	gi.dprintf ("Default cvars saved.\n");
}

/*
==============
TDM_ServerCommand
==============
Handle a server sv console command.
*/
qboolean TDM_ServerCommand (const char *cmd)
{
	if (!Q_stricmp (cmd, "settings"))
		TDM_SV_Settings_f ();
	else if (!Q_stricmp (cmd, "savedefaults"))
		TDM_SV_SaveDefaults_f ();
	else if (!Q_stricmp (cmd, "applysettings"))
		TDM_SV_ApplySettings_f ();
	else
		return false;

	return true;
}

/*
==============
TDM_Teamname_f
==============
Set teamname (captain/admin only).
*/
void TDM_Teamname_f (edict_t *ent)
{
	char		*value;
	int			team;
	unsigned	i;

	if (g_locked_names->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "Teamnames are locked.\n");
		return;
	}

	if (gi.argc() < 3 && ent->client->pers.admin && !ent->client->pers.team)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: teamname <team> <name>\n");
		return;
	}

	if (gi.argc() < 2)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: teamname <name>\n");
		return;
	}

	if (g_gamemode->value == GAMEMODE_1V1)
	{
		gi.cprintf (ent, PRINT_HIGH, "This command is unavailable in 1v1 mode.\n");
		return;
	}

	if (teaminfo[ent->client->pers.team].captain != ent && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only team captains or admins can change teamname.\n");
		return;
	}

	if (tdm_match_status != MM_WARMUP && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only change team name during warmup.\n");
		return;
	}

	if (ent->client->pers.admin && gi.argc() > 2)
		team = TDM_GetTeamFromArg (ent, gi.argv(1));
	else
		team = ent->client->pers.team;

	if (team != TEAM_A && team != TEAM_B)
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid team.\n");
		return;
	}

	value = gi.args ();

	// skip original team name in the string
	if (gi.argc() == 3)
	{
		while (*value != ' ')
			value++;

		while (*value == ' ')
			value++;
	}

	//chop off quotes if the user specified them
	value = G_StripQuotes (value);

	//max however many characters
	value[sizeof(teaminfo[TEAM_SPEC].name)-1] = '\0';

	//validate teamname in the most convuluted way possible: disallow high ascii,
	//quotes and escape char as they can mess up the scoreboard program.
	i = 0;
	do
	{
		if (value[i] < 32 || value[i] == '"')
		{
			gi.cprintf (ent, PRINT_HIGH, "Invalid team name, must not contain color text or quotes.\n");
			return;
		}
		i++;
	} while (value[i]);

	if (team == TEAM_A)
	{
		if (!strcmp (teaminfo[TEAM_A].name, value))
			return;

		g_team_a_name = gi.cvar_set ("g_team_a_name", value);
		g_team_a_name->modified = true;
	}
	else if (team == TEAM_B)
	{
		if (!strcmp (teaminfo[TEAM_B].name, value))
			return;

		g_team_b_name = gi.cvar_set ("g_team_b_name", value);
		g_team_b_name->modified = true;
	}

	tdm_settings_not_default = true;

	gi.bprintf (PRINT_HIGH, "Team '%s' renamed to '%s'.\n", teaminfo[team].name, value);

	TDM_UpdateTeamNames ();
	
	UpdateTeamMenu ();

	TDM_UpdateConfigStrings(false);
}

/*
==============
TDM_Lockteam_f
==============
Locks / unlocks team (captain/admin only).
*/
void TDM_Lockteam_f (edict_t *ent, qboolean lock)
{
	int 	team;

	if (g_gamemode->value == GAMEMODE_1V1)
	{
		gi.cprintf (ent, PRINT_HIGH, "This command is unavailable in 1v1 mode.\n");
		return;
	}

	if (teaminfo[ent->client->pers.team].captain != ent && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only team captains or admins can lock/unlock team.\n");
		return;
	}

	if (gi.argc() < 2 && ent->client->pers.admin && !ent->client->pers.team)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: %s <team>\n", gi.argv(0));
		return;
	}

	if (ent->client->pers.admin && gi.argc() > 1)
		team = TDM_GetTeamFromArg (ent, gi.args());
	else
		team = ent->client->pers.team;

	if (team != TEAM_A && team != TEAM_B)
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid team.\n");
		return;
	}

	if (teaminfo[team].players == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can't lock empty team!\n");
		return;
	}

	teaminfo[team].locked = lock;
	gi.cprintf (ent, PRINT_HIGH, "Team '%s' is %slocked.\n", teaminfo[team].name, (lock ? "" : "un"));
}

void TDM_Forceteam_f (edict_t *ent)
{
	//TODO: Force team
}

/*
==============
TDM_Invite_f
==============
Invite a player to your team.
*/
void TDM_Invite_f (edict_t *ent)
{
	edict_t	*victim;

	if (gi.argc() < 2)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: invite <name/id>\n");
		return;
	}

	if (teaminfo[ent->client->pers.team].captain != ent && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only team captains or admins can invite players.\n");
		return;
	}

	if (TDM_Is1V1())
	{
		gi.cprintf (ent, PRINT_HIGH, "This command is unavailable in 1v1 mode.\n");
		return;
	}

	if (tdm_match_status != MM_WARMUP && tdm_match_status != MM_PLAYING && tdm_match_status != MM_TIMEOUT)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only invite players during warmup or during the match.\n");
		return;
	}

	if (LookupPlayer (gi.args(), &victim, ent))
	{
		if (ent == victim)
		{
			gi.cprintf (ent, PRINT_HIGH, "You can't invite yourself!\n");
			return;
		}

		if (TDM_RateLimited (ent, SECS_TO_FRAMES(2)))
			return;

		if (victim->client->pers.team)
		{
			gi.cprintf (ent, PRINT_HIGH, "%s is already on a team.\n", victim->client->pers.netname);
			return;
		}

		victim->client->resp.last_invited_by = ent;
		gi.centerprintf (victim, "You are invited to '%s'\nby %s. Type ACCEPT in\nthe console to accept.\n", teaminfo[ent->client->pers.team].name, ent->client->pers.netname);
		gi.cprintf (ent, PRINT_HIGH, "%s was invited to join your team.\n", victim->client->pers.netname);
	}
}

/*
==============
TDM_PickPlayer_f
==============
Pick a player
//TODO: invite instead of direct picking
*/
void TDM_PickPlayer_f (edict_t *ent)
{
	int		team;
	char	*name;
	edict_t	*victim;

	//this could be abused by some captain to make server unplayable by constantly picking
	if (!g_tdm_allow_pick->value)
	{
		TDM_Invite_f (ent);
		//gi.cprintf (ent, PRINT_HIGH, "Player picking is disabled by the server administrator. Try using invite instead.\n");
		return;
	}

	if (gi.argc() < 3 && ent->client->pers.admin && !ent->client->pers.team)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: %s <team> <name/id>\n", gi.argv(0));
		return;
	}

	if (gi.argc() < 2)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: %s <name/id>\n", gi.argv(0));
		return;
	}

	if (TDM_Is1V1())
	{
		gi.cprintf (ent, PRINT_HIGH, "This command is unavailable in 1v1 mode.\n");
		return;
	}

	if (teaminfo[ent->client->pers.team].captain != ent && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only team captains or admins can pick players.\n");
		return;
	}

	if (tdm_match_status != MM_WARMUP && tdm_match_status != MM_PLAYING && tdm_match_status != MM_TIMEOUT)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only pick players during warmup or during the match.\n");
		return;
	}

	if (TDM_RateLimited (ent, SECS_TO_FRAMES(1)))
		return;

	name = gi.args();

	// skip the name in the string
	if (gi.argc() > 2)
	{
		while (*name != ' ')
			name++;

		while (*name == ' ')
			name++;
	}

	if (LookupPlayer (name, &victim, ent))
	{
		if (ent == victim)
		{
			gi.cprintf (ent, PRINT_HIGH, "You can't pick yourself!\n");
			return;
		}

		if (victim->client->pers.team)
		{
			gi.cprintf (ent, PRINT_HIGH, "%s is already on a team.\n", victim->client->pers.netname);
			return;
		}

		if (ent->client->pers.admin && gi.argc() > 2)
			team = TDM_GetTeamFromArg (ent, gi.argv(1));
		else
			team = ent->client->pers.team;

		if (team != TEAM_A && team != TEAM_B)
		{
			gi.cprintf (ent, PRINT_HIGH, "Invalid team.\n");
			return;
		}

		gi.bprintf (PRINT_CHAT, "%s picked %s for team '%s'.\n", ent->client->pers.netname, victim->client->pers.netname, teaminfo[team].name);

		if (victim->client->pers.team)
			TDM_LeftTeam (victim, false);

		victim->client->pers.team = team;
		JoinedTeam (victim, false, false);
	}
}

/*
==============
TDM_Accept_f
==============
Accept an invite.
*/
void TDM_Accept_f (edict_t *ent)
{
	if (!ent->client->resp.last_invited_by)
	{
		gi.cprintf (ent, PRINT_HIGH, "No invite to accept!\n");
		return;
	}

	if (tdm_match_status != MM_WARMUP && tdm_match_status != MM_PLAYING && tdm_match_status != MM_TIMEOUT)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only accept an invite during the match or warmup!\n");
		ent->client->resp.last_invited_by = NULL;
		return;
	}

	//holy dereference batman
	if (!ent->client->resp.last_invited_by->inuse ||
		teaminfo[ent->client->resp.last_invited_by->client->pers.team].captain != ent->client->resp.last_invited_by)
	{
		gi.cprintf (ent, PRINT_HIGH, "The invite is no longer valid.\n");
		ent->client->resp.last_invited_by = NULL;
		return;
	}

	//prevent invite going over max allowed on a team
	if ((g_max_players_per_team->value && teaminfo[ent->client->resp.last_invited_by->client->pers.team].players >= g_max_players_per_team->value) ||
		(tdm_match_status >= MM_PLAYING && teaminfo[ent->client->resp.last_invited_by->client->pers.team].players >= current_matchinfo.max_players_per_team))
	{
		gi.cprintf (ent, PRINT_HIGH, "The team is already full.\n");
		ent->client->resp.last_invited_by = NULL;
		return;
	}

	if (ent->client->pers.team)
		TDM_LeftTeam (ent, true);

	ent->client->pers.team = ent->client->resp.last_invited_by->client->pers.team;

	JoinedTeam (ent, false, true);
}

/*
==============
TDM_PrintPlayers
==============
Prints the list of current players and their ID.
*/
void TDM_PrintPlayers (edict_t *ent)
{
	char	st[128];
	char	text[1024];
	edict_t	*e2;

	strcpy (text, "   id  name\n");
	strcat (text, "  ---------\n");

	for (e2 = g_edicts + 1; e2 <= g_edicts + game.maxclients; e2++)
	{
		if (!e2->inuse)
			continue;

		Com_sprintf (st, sizeof(st), "  %3d  %s\n", (int)(e2 - g_edicts - 1), e2->client->pers.netname);

		if (strlen(text) > 900)
		{
			gi.cprintf (ent, PRINT_HIGH, "%s", text);
			text[0] = 0;
		}

		strcat (text, st);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s", text);
}

/*
==============
TDM_Talk_f
==============
Talk to a player.
*/
void TDM_Talk_f (edict_t *ent)
{
	char	*p;
	char	text[256];
	edict_t	*victim;
	int		i;
	size_t	len, total;

	if (gi.argc() < 3)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: %s <name/id> message\n", gi.argv(0));
		TDM_PrintPlayers (ent);
		return;
	}

	if (LookupPlayer (gi.argv(1), &victim, ent))
	{
		if (victim == ent)
		{
			gi.cprintf (ent, PRINT_HIGH, "You cannot talk to yourself.\n");
			return;
		}

		if (tdm_match_status >= MM_PLAYING)
		{
			if (ent->client->pers.team == TEAM_SPEC && victim->client->pers.team != TEAM_SPEC)
			{
				gi.cprintf (ent, PRINT_HIGH, "Spectators cannot talk to players during the match.\n");
				return;
			}
		}

		if (g_chat_mode->value == 2 && ent->client->pers.team == TEAM_SPEC && !victim->client->pers.admin)
		{
			gi.cprintf (ent, PRINT_HIGH, "Spectators cannot talk during shutup mode.\n");
			return;
		}

		// skip first argument (victims name/id)
		total = 0;
		for (i = 2; i < gi.argc(); i++)
		{
			p = gi.argv (i);
			len = strlen (p);
			if (!len)
				continue;
			if (total + len + 1 >= sizeof (text))
				break;
			memcpy (text + total, p, len);
			text[total + len] = ' ';
			total += len + 1;
		}
		text[total] = 0;

		if (!total)
			return;

		gi.cprintf (ent, PRINT_CHAT, "{%s}: %s\n", ent->client->pers.netname, text);
		gi.cprintf (victim, PRINT_CHAT, "{%s}: %s\n", ent->client->pers.netname, text);
	}
}

/*
==============
TDM_KickPlayer_f
==============
Kick a player from a team
*/
void TDM_KickPlayer_f (edict_t *ent)
{
	unsigned	team;
	char		*name;
	edict_t		*victim;

	if (gi.argc() < 3 && ent->client->pers.admin && !ent->client->pers.team)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: %s <team> <name>\n", gi.argv(0));
		return;
	}

	if (gi.argc() < 2)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: %s <name/id>\n", gi.argv(0));
		return;
	}

	if (g_gamemode->value == GAMEMODE_1V1)
	{
		gi.cprintf (ent, PRINT_HIGH, "This command is unavailable in 1v1 mode.\n");
		return;
	}

	if (teaminfo[ent->client->pers.team].captain != ent && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only team captains or admins can kick players.\n");
		return;
	}

	if (ent->client->pers.admin && gi.argc() > 2)
		team = TDM_GetTeamFromArg (ent, gi.argv(1));
	else
		team = ent->client->pers.team;

	if (team != TEAM_A && team != TEAM_B)
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid team.\n");
		return;
	}

	name = gi.args();

	// skip the name in the string
	if (gi.argc() > 2)
	{
		while (*name != ' ')
			name++;

		while (*name == ' ')
			name++;
	}

	if (LookupPlayer (name, &victim, ent))
	{
		if (!victim->client->pers.team)
		{
			gi.cprintf (ent, PRINT_HIGH, "%s is not on a team.\n", victim->client->pers.netname);
			return;
		}

		if (victim->client->pers.team != team && !ent->client->pers.admin && gi.argc() < 3)
		{
			gi.cprintf (ent, PRINT_HIGH, "%s is not on your team.\n", victim->client->pers.netname);
			return;
		}

		if (victim->client->pers.admin)
		{
			gi.cprintf (ent, PRINT_HIGH, "You can't kick an admin!\n");
			return;
		}

		if (victim == ent)
		{
			gi.cprintf (ent, PRINT_HIGH, "You can't kick yourself!\n");
			return;
		}

		//maybe this should broadcast?
		gi.cprintf (victim, PRINT_HIGH, "You were removed from team '%s' by %s.\n", teaminfo[victim->client->pers.team].name, ent->client->pers.netname);
		ToggleChaseCam (victim);
	}
}

/*
==============
TDM_Admin_f
==============
Become an admin
*/
void TDM_Admin_f (edict_t *ent)
{
	if (ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "You are no longer registered as an admin.\n");
		ent->client->pers.admin = false;
		return;
	}

	if (!g_admin_password->string[0])
	{
		gi.cprintf (ent, PRINT_HIGH, "Admin is disabled on this server.\n");
		return;
	}

	if (TDM_RateLimited (ent, SECS_TO_FRAMES(1)))
		return;

	if (!strcmp (gi.argv(1), g_admin_password->string))
	{
		gi.bprintf (PRINT_HIGH, "%s became an admin.\n", ent->client->pers.netname);
		ent->client->pers.admin = true;
	}
	else
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid password.\n");
		gi.dprintf ("%s[%s] failed to login as admin.\n", ent->client->pers.netname, ent->client->pers.ip);
	}
}

/*
==============
TDM_Captain_f
==============
Print / set captain
*/
void TDM_Captain_f (edict_t *ent)
{
	if (g_gamemode->value == GAMEMODE_1V1)
	{
		gi.cprintf (ent, PRINT_HIGH, "This command is unavailable in 1v1 mode.\n");
		return;
	}

	if (gi.argc() < 2)
	{
		//checking captain status or assigning from NULL captain
		if (ent->client->pers.team == TEAM_SPEC)
		{
			gi.cprintf (ent, PRINT_HIGH, "You must join a team to set or become captain.\n");
			return;
		}

		if (teaminfo[ent->client->pers.team].captain == ent)
		{
			gi.cprintf (ent, PRINT_HIGH, "You are the captain of team '%s'\n", teaminfo[ent->client->pers.team].name);
		}
		else if (teaminfo[ent->client->pers.team].captain)
		{
			gi.cprintf (ent, PRINT_HIGH, "%s is the captain of team '%s'\n",
			(teaminfo[ent->client->pers.team].captain)->client->pers.netname, teaminfo[ent->client->pers.team].name);
		}
		else
		{
			TDM_SetCaptain (ent->client->pers.team, ent);
		}

	}
	//transferring captain to another player
	else if (gi.argc() == 2)
	{
		edict_t	*victim;

		if (teaminfo[ent->client->pers.team].captain != ent)
		{
			gi.cprintf (ent, PRINT_HIGH, "You must be captain to transfer it to another player!\n");
			return;
		}

		if (LookupPlayer (gi.argv(1), &victim, ent))
		{
			if (victim == ent)
			{
				gi.cprintf (ent, PRINT_HIGH, "You can't transfer captain to yourself!\n");
				return;
			}

			if (victim->client->pers.team != ent->client->pers.team)
			{
				gi.cprintf (ent, PRINT_HIGH, "%s is not on your team.\n", victim->client->pers.netname);
				return;
			}

			//so they don't wonder wtf just happened...
			gi.cprintf (victim, PRINT_HIGH, "%s transferred captain status to you.\n", ent->client->pers.netname);
			TDM_SetCaptain (victim->client->pers.team, victim);
		}
	}
	else
	{
		unsigned		team;
		edict_t	*victim;

		if (!ent->client->pers.admin)
		{
			gi.cprintf (ent, PRINT_HIGH, "You must be admin to change another teams captain!\n");
			return;
		}
		team = TDM_GetTeamFromArg (ent, gi.argv(1));

		if (team != TEAM_A && team != TEAM_B)
		{
			gi.cprintf (ent, PRINT_HIGH, "Invalid team.\n");
			return;
		}

		if (LookupPlayer (gi.argv(2), &victim, ent))
		{
			if (victim->client->pers.team != team)
			{
				gi.cprintf (ent, PRINT_HIGH, "%s is not on the team.\n", victim->client->pers.netname);
				return;
			}

			//so they don't wonder wtf just happened...
			gi.cprintf (victim, PRINT_HIGH, "%s transferred captain status to you.\n", ent->client->pers.netname);
			TDM_SetCaptain (victim->client->pers.team, victim);
		}
	}
}

/*
==============
TDM_Captains_f
==============
Show who the captains are for each team.
*/
void TDM_Captains_f (edict_t *ent)
{
	gi.cprintf (ent, PRINT_HIGH, "Team '%s' captain: %s\nTeam '%s' captain: %s\n",
		teaminfo[TEAM_A].name,
		teaminfo[TEAM_A].captain ? teaminfo[TEAM_A].captain->client->pers.netname : "(none)",
		teaminfo[TEAM_B].name,
		teaminfo[TEAM_B].captain ? teaminfo[TEAM_B].captain->client->pers.netname : "(none)");
}

/*
==============
TDM_Kick_f
==============
Kick a player from the server (admin only)
*/
void TDM_Kick_f (edict_t *ent)
{
	edict_t	*victim;

	if (gi.argc() < 2)
	{
		gi.cprintf(ent, PRINT_HIGH, "Usage: kick <id>\n");
		return;
	}

	if (LookupPlayer (gi.args(), &victim, ent))
	{
		if (victim->client->pers.admin)
		{
			gi.cprintf (ent, PRINT_HIGH, "You cannot kick an admin!\n");
			return;
		}

		if (vote.flags & VOTE_KICK && vote.victim == victim)
			TDM_RemoveVote ();

		gi.AddCommandString (va ("kick %d\n", (int)(victim - g_edicts - 1)));
	}
}

qboolean TDM_ValidateModelSkin (const char *value)
{
	char		*skin;
	char		*model;
	size_t		len;
	unsigned	i;

	model = G_CopyString (value);

	skin = strchr (model, '/');
	if (!skin)
	{
		gi.TagFree (model);
		return false;
	}

	skin[0] = 0;
	skin++;

	if (!skin[0])
	{
		gi.TagFree (model);
		return false;
	}

	if (!Q_stricmp (model, "opentdm"))
	{
		gi.TagFree (model);
		return false;
	}

	len = strlen (model);
	for (i = 0; i < len; i++)
	{
		if (!isalnum (model[i]) && model[i] != '_' && model[i] != '-')
		{
			gi.TagFree (model);
			return false;
		}
	}
	
	len = strlen (skin);
	for (i = 0; i < len; i++)
	{
		if (!isalnum (skin[i]) && skin[i] != '_' && skin[i] != '-')
		{
			gi.TagFree (model);
			return false;
		}
	}

	gi.TagFree (model);
	return true;
}

/*
==============
TDM_Teamskin_f
==============
Set teamskin (captain/admin only).
*/
void TDM_Teamskin_f (edict_t *ent)
{
	const char *value;
	int			team;

	if (g_locked_skins->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "Teamskins are locked.\n");
		return;
	}

	if (gi.argc() < 3 && ent->client->pers.admin && !ent->client->pers.team)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: teamskin <team> <model/skin>\n");
		return;
	}

	if (gi.argc() < 2)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: teamskin <model/skin>\n");
		return;
	}

	if (teaminfo[ent->client->pers.team].captain != ent && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only team captains or admins can change teamskin.\n");
		return;
	}

	if (gi.argc() == 3 && ent->client->pers.admin)
	{
		team = TDM_GetTeamFromArg (ent, gi.argv(1));
		value = gi.argv(2);
	}
	else
	{
		team = ent->client->pers.team;
		value = gi.argv(1);
	}

	if (team != TEAM_A && team != TEAM_B)
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid team.\n");
		return;
	}

	if (!value[0] || strlen (value) >= sizeof(teaminfo[TEAM_SPEC].skin) - 1)
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid model/skin name.\n");
		return;
	}

	/*if (g_allowed_skins->string[0])
	{
		char	*p = g_allowed_skins->string;
		while (*/

	if (!TDM_ValidateModelSkin (value))
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid model/skin.\n");
		return;
	}

	if (tdm_match_status != MM_WARMUP && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only change team skin during warmup.\n");
		return;
	}

	//TODO: some check model/skin name, force only female/male models

	if (team == TEAM_A)
	{
		if (!strcmp (teaminfo[TEAM_A].skin, value))
			return;
		g_team_a_skin = gi.cvar_set ("g_team_a_skin", value);
		g_team_a_skin->modified = true;
	}
	else if (team == TEAM_B)
	{
		if (!strcmp (teaminfo[TEAM_B].skin, value))
			return;
		g_team_b_skin = gi.cvar_set ("g_team_b_skin", value);
		g_team_b_skin->modified = true;
	}

	tdm_settings_not_default = true;

	TDM_UpdateConfigStrings (false);
}

/*
==============
TDM_NotReady_f
==============
Set notready status
wision: some ppl actually use this
*/
void TDM_NotReady_f (edict_t *ent)
{
	if (!ent->client->pers.team)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must be on a team to be NOT ready.\n");
		return;
	}

	if (tdm_match_status >= MM_PLAYING)
		return;

	if (!ent->client->resp.ready)
		return;

	if (TDM_RateLimited (ent, SECS_TO_FRAMES(1)))
		return;

	ent->client->resp.ready = false;

	gi.bprintf (PRINT_HIGH, "%s is not ready!\n", ent->client->pers.netname);

	TDM_CheckMatchStart ();
}

/*
==============
TDM_Ready_f
==============
Toggle ready status
*/
void TDM_Ready_f (edict_t *ent)
{
	if (!ent->client->pers.team)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must be on a team to be ready.\n");
		return;
	}

	if (tdm_match_status >= MM_PLAYING)
		return;

	//apparently this should not toggle
	if (ent->client->resp.ready)
		return;

	if (TDM_RateLimited (ent, SECS_TO_FRAMES(1)))
		return;

	ent->client->resp.ready = true;

	gi.bprintf (PRINT_HIGH, "%s is ready!\n", ent->client->pers.netname);

	TDM_CheckMatchStart ();
}

/*
==============
TDM_Motd_f
==============
Show/hide motd message defined in g_motd_message.
*/
void TDM_Motd_f (edict_t *ent)
{
	static char	string[1300];
	char		message[512];
	int			msg_offset = 0;
	int			offset = 0;
	int			len;
	int			current_length;
	int			i;

	if (!g_motd_message || !g_motd_message->string[0] || ent->client->showmotd)
	{
		ent->client->showmotd = false;
		return;
	}

	if (ent->client->pers.menu.active)
		PMenu_Close (ent);

	ent->client->showscores = false;
	ent->client->showoldscores = false;

	*string = 0;

	Q_strncpy (message, g_motd_message->string, sizeof(message)-1);

	len = strlen(message);

	for (i = 0; i <= len; i++)
	{
		if (i == len || (message[i] == '\\' && message[i+1] == 'n'))
		{
			// don't cut last letter if it's last line
			if (i != len)
				message[i] = '\0';

			current_length = strlen(string);

			if (current_length + strlen(message + msg_offset) + 23 > sizeof(string)-1)
				break;

			sprintf (string + current_length, "xl 8 yb %d string \"%s\" ", offset - 160, message + msg_offset);
			offset += 8;
			msg_offset = i + 2;

			// don't allow text to go off screen
			if (offset > 160)
				break;
		}
	}

	ent->client->showmotd = true;

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}

/*
==============
TDM_Changeteamstatus_f
==============
Set ready/notready status for whole team
*/
void TDM_Changeteamstatus_f (edict_t *ent, qboolean ready)
{
	edict_t *ent2;

	if (ent->client->pers.team == TEAM_SPEC)
		return;

	if (teaminfo[ent->client->pers.team].captain != ent && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only team captains or admins can use teamready/teamnotready.\n");
		return;
	}

	for (ent2 = g_edicts + 1; ent2 <= g_edicts + game.maxclients; ent2++)
	{
		if (!ent2->inuse)
			continue;

		if (ent->client->pers.team != ent2->client->pers.team)
			continue;

		if (ready && ent2->client->resp.ready)
			continue;

		if (!ready && !ent2->client->resp.ready)
			continue;

		if (ent2 != ent)
			gi.cprintf (ent2, PRINT_HIGH, "You were forced %sready by team captain %s!\n", ready ? "" : "not ", ent->client->pers.netname);

		TDM_Ready_f (ent2);
	}
}

/*
==============
TDM_OldScores_f
==============
Show the scoreboard from the last match. I hope I got that right :).
*/
void TDM_OldScores_f (edict_t *ent)
{
	if (!old_matchinfo.scoreboard_string[0])
	{
		gi.cprintf (ent, PRINT_HIGH, "No old scores to show yet.\n");
		return;
	}

	if (ent->client->showoldscores)
	{
		ent->client->showoldscores = false;
		return;
	}

	PMenu_Close (ent);

	ent->client->showmotd = false;
	ent->client->showscores = false;
	ent->client->showoldscores = true;

	gi.WriteByte (svc_layout);
	gi.WriteString (old_matchinfo.scoreboard_string);
	gi.unicast (ent, true);
}

/*
==============
TDM_Ghost_f
==============
Manual recovery of saved client info via join code
*/
void TDM_Ghost_f (edict_t *ent)
{
	unsigned code;

	if (gi.argc() < 2)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: %s <code>\n", gi.argv(0));
		return;
	}

	if (tdm_match_status < MM_PLAYING || tdm_match_status == MM_SCOREBOARD)
	{
		gi.cprintf (ent, PRINT_HIGH, "No match in progress.\n");
		return;
	}

	if (ent->client->pers.team)
	{
		gi.cprintf (ent, PRINT_HIGH, "%s can only be used from spectator mode.\n", gi.argv(0));
		return;
	}

	code = strtoul (gi.args(), NULL, 0);

	//to prevent brute forcing :)
	if (TDM_RateLimited (ent, 2))
		return;

	if (!TDM_ProcessJoinCode (ent, code))
	{
		gi.cprintf (ent, PRINT_HIGH, "No client information found for code %u.\n", code);
		return;
	}
}

/*
==============
TDM_Id_f
==============
Toggle player id display
*/
void TDM_Id_f (edict_t *ent)
{
	ent->client->pers.disable_id_view = !ent->client->pers.disable_id_view;
	gi.cprintf (ent, PRINT_HIGH, "Player identification display is now %sabled.\n", ent->client->pers.disable_id_view ? "dis" : "en");
}

/*
==============
TDM_Mute_f
==============
Mute a player. Default for five minutes, maximum 1 hour
should we block everything like 'say', 'say_team', 'talk'? or just 'say' ?
*/
void TDM_Mute_f (edict_t *ent)
{
	unsigned	time;
	const char	*input;
	edict_t	*victim;

	if (!ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only admin can mute other players\n");
		return;
	}

	if (gi.argc() < 2)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: mute <name/id> [minutes]\n");
		TDM_PrintPlayers (ent);
		return;
	}

	if (LookupPlayer (gi.argv(1), &victim, ent))
	{
		if (victim == ent)
		{
			gi.cprintf (ent, PRINT_HIGH, "You cannot mute yourself.\n");
			return;
		}

		if (victim->client->pers.admin)
		{
			gi.cprintf (ent, PRINT_HIGH, "You cannot mute an admin!\n");
			return;
		}

		// default mute for five minutes
		if (gi.argc() < 3)
			time = 5;
		else
		{
			input = gi.argv(2);
			time = strtoul (input, NULL, 10);
		}

		if (time > 0)
		{
			if (time > 60)
				time = 60;

			victim->client->pers.mute_frame = level.framenum + SECS_TO_FRAMES (time * 60);
			gi.cprintf (ent, PRINT_HIGH, "%s is muted for %d minute%s\n", victim->client->pers.netname, time, time == 1 ? "" : "s");
			gi.cprintf (victim, PRINT_HIGH, "You are muted for %d minute%s by admin\n", time, time == 1 ? "" : "s");
		}
	}
}

/*
==============
TDM_Unmute_f
==============
Unmute a player.
*/
void TDM_Unmute_f (edict_t *ent)
{
	edict_t	*victim;

	if (!ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only admin can unmute other players\n");
		return;
	}

	if (gi.argc() < 2)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: unmute <name/id>\n");
		TDM_PrintPlayers (ent);
		return;
	}

	if (LookupPlayer (gi.argv(1), &victim, ent))
	{
		if (victim->client->pers.mute_frame > level.framenum)
		{
			victim->client->pers.mute_frame = 0;
			gi.cprintf (ent, PRINT_HIGH, "%s is not muted anymore\n", victim->client->pers.netname);
			gi.cprintf (victim, PRINT_HIGH, "You are not muted anymore\n");
		}
	}
}

/*
==============
TDM_Speclock_f
==============
Lock the team against spectators.
*/
void TDM_Speclock_f (edict_t *ent)
{
	int		team;

	if (!((int)g_command_mask->value & COMMAND_SPECLOCK) && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Command '%s' is not allowed on this server.\n", gi.argv(0));
		return;
	}

	if (gi.argc() < 2 && ent->client->pers.admin && !ent->client->pers.team)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: speclock <team>\n");
		return;
	}

	if (teaminfo[ent->client->pers.team].captain != ent && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only team captains or admins can speclock team.\n");
		return;
	}

	if (gi.argc() == 2 && ent->client->pers.admin)
		team = TDM_GetTeamFromArg (ent, gi.argv(1));
	else
		team = ent->client->pers.team;

	if (team != TEAM_A && team != TEAM_B)
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid team.\n");
		return;
	}

	if (teaminfo[team].speclocked)
	{
		teaminfo[team].speclocked = false;
		gi.cprintf (ent, PRINT_HIGH, "Team %sis unlocked against spectators.\n", ((ent->client->pers.admin) ? va("'%s' ", teaminfo[team].name) : ""));
	}
	else
	{
		edict_t		*e;
		// reset all invites
		for (e = g_edicts + 1; e <= g_edicts + game.maxclients; e++)
			if (e->inuse)
				e->client->pers.specinvite[team] = false;

		teaminfo[team].speclocked = true;
		gi.cprintf (ent, PRINT_HIGH, "Team %sis locked against spectators.\n", ((ent->client->pers.admin) ? va("'%s' ", teaminfo[team].name) : ""));
	}
}

/*
==============
TDM_Specinvite_f
==============
Invite a spectator for speclocked team.
*/
void TDM_Specinvite_f (edict_t *ent)
{
	int			team;
	edict_t		*victim;
	const char	*value;

	if (!((int)g_command_mask->value & COMMAND_SPECLOCK) && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Command '%s' is not allowed on this server.\n", gi.argv(0));
		return;
	}

	if (gi.argc() < 3 && ent->client->pers.admin && !ent->client->pers.team)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: specinvite <team> <name/id>\n");
		TDM_PrintPlayers (ent);
		return;
	}

	if (gi.argc() < 2)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: specinvite <name/id>\n");
		TDM_PrintPlayers (ent);
		return;
	}

	if (gi.argc() == 3 && ent->client->pers.admin)
	{
		team = TDM_GetTeamFromArg (ent, gi.argv(1));
		value = gi.argv(2);
	}
	else
	{
		team = ent->client->pers.team;
		value = gi.argv(1);
	}

	if (team != TEAM_A && team != TEAM_B)
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid team.\n");
		return;
	}

	if (!teaminfo[team].speclocked)
	{
		gi.cprintf (ent, PRINT_HIGH, "Team is not locked against spectators.\n");
		return;
	}
	
	if (teaminfo[team].captain != ent && !ent->client->pers.admin)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only team captains or admins can specinvite players.\n");
		return;
	}

	if (LookupPlayer (value, &victim, ent))
	{
		if (victim->client->pers.team)
		{
			gi.cprintf (ent, PRINT_HIGH, "%s is not a spectator.\n", victim->client->pers.netname);
			return;
		}

		if (victim->client->pers.admin)
		{
			gi.cprintf (ent, PRINT_HIGH, "Admins can spectate anyone.\n");
			return;
		}

		if (victim->client->pers.specinvite[team])
		{
			victim->client->pers.specinvite[team] = false;
			gi.cprintf (ent, PRINT_HIGH, "%s was forbidden to spectate team '%s'.\n", victim->client->pers.netname, teaminfo[team].name);
			gi.cprintf (victim, PRINT_HIGH, "You were forbidden by %s to spectate team '%s'.\n", ent->client->pers.netname, teaminfo[team].name);
		}
		else
		{
			victim->client->pers.specinvite[team] = true;
			gi.cprintf (ent, PRINT_HIGH, "%s was invited to spectate team '%s'.\n", victim->client->pers.netname, teaminfo[team].name);
			gi.cprintf (victim, PRINT_HIGH, "You were invited by %s to spectate team '%s'.\n", ent->client->pers.netname, teaminfo[team].name);
		}
	}
}

/*
==============
TDM_Spectate_f
==============
Move to spectator or change spetating mode.
*/
void TDM_Spectate_f (edict_t *ent)
{
	if (tdm_match_status == MM_TIMEOUT && !ent->client->pers.team)
		return;

	if (gi.argc() < 2 || !Q_stricmp (gi.argv(1), "none"))
	{
		ent->client->resp.spec_mode = SPEC_NONE;
		ToggleChaseCam (ent);
		return;
	}

	if (!Q_stricmp (gi.argv(1), "quad"))
		ent->client->resp.spec_mode = SPEC_QUAD;
	else if (!Q_stricmp (gi.argv(1), "invul") || !Q_stricmp (gi.argv(1), "pent") || !Q_stricmp (gi.argv(1), "666"))
		ent->client->resp.spec_mode = SPEC_INVUL;
	else if (!Q_stricmp (gi.argv(1), "killer"))
		ent->client->resp.spec_mode = SPEC_KILLER;
	else if (!Q_stricmp (gi.argv(1), "leader") || !Q_stricmp (gi.argv(1), "topfragger"))
		ent->client->resp.spec_mode = SPEC_LEADER;
	else
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: %s [quad/invul/killer/leader/none]\n", gi.argv(0));
		return;
	}

	if (ent->client->pers.team)
		ToggleChaseCam (ent);

	TDM_UpdateSpectator (ent);
}

/*
==============
TDM_TeamEnemySkin_f
==============
Set team or enemy skin.
*/
void TDM_TeamEnemySkin_f (edict_t *ent, qboolean team)
{
	if (gi.argc() < 2)
	{
		gi.cprintf (ent, PRINT_HIGH, "Usage: %s model/skin\n", gi.argv(0));
		return;
	}

	if (!TDM_ValidateModelSkin (gi.argv(1)))
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid model/skin.\n");
		return;
	}

	if (team)
		strncpy (ent->client->pers.config.teamskin, gi.argv(1), sizeof(ent->client->pers.config.teamskin)-1);
	else
		strncpy (ent->client->pers.config.enemyskin, gi.argv(1), sizeof(ent->client->pers.config.enemyskin)-1);

	TDM_SetTeamSkins (ent, NULL);
}

/*
==============
TDM_Command
==============
Process TDM commands (from ClientCommand)
*/
qboolean TDM_Command (const char *cmd, edict_t *ent)
{
	if (ent->client->pers.admin)
	{
		if (!Q_stricmp (cmd, "forceready") || !Q_stricmp (cmd, "readyall") || !Q_stricmp (cmd, "allready") || !Q_stricmp (cmd, "startcountdown"))
		{
			TDM_ForceReady_f (true);
			return true;
		}
		/*else if (!Q_stricmp (cmd, "startmatch"))
		{
			TDM_StartMatch_f (ent);
			return true;
		}*/
		else if (!Q_stricmp (cmd, "kickplayer"))
		{
			TDM_KickPlayer_f (ent);
			return true;
		}
		else if (!Q_stricmp (cmd, "kick") || !Q_stricmp (cmd, "boot"))
		{
			TDM_Kick_f (ent);
			return true;
		}
		else if (!Q_stricmp (cmd, "ban"))
		{
			TDM_Ban_f (ent);
			return true;
		}
		else if (!Q_stricmp (cmd, "forceteam"))
		{
			TDM_Forceteam_f (ent);
			return true;
		}
		// wision: some more acommands
		else if (!Q_stricmp (cmd, "kickban"))
		{
			TDM_Kickban_f (ent);
			return true;
		}
		else if (!Q_stricmp (cmd, "acommands"))
		{
			TDM_Acommands_f (ent);
			return true;
		}
		else if (!Q_stricmp (cmd, "unreadyall") || !Q_stricmp (cmd, "notreadyall"))
		{
			TDM_ForceReady_f (false);
			return true;
		}
		else if (!Q_stricmp (cmd, "unban"))
		{
			TDM_Unban_f (ent);
			return true;
		}
		else if (!Q_stricmp (cmd, "bans"))
		{
			TDM_Bans_f (ent);
			return true;
		}
		else if (!Q_stricmp (cmd, "break"))
		{
			TDM_Break_f (ent);
			return true;
		}
		else if (!Q_stricmp (cmd, "changemap"))
		{
			TDM_Changemap_f (ent);
			return true;
		}
	}

	// let's allow voting during the timeout
	if (tdm_match_status == MM_TIMEOUT)
	{
		if (!Q_stricmp (cmd, "vote"))
			TDM_Vote_f (ent);
		else if (!Q_stricmp (cmd, "yes") || !Q_stricmp (cmd, "no"))
			TDM_Vote_f (ent);
	}
	//only a few commands work in time out mode or intermission
	if (tdm_match_status == MM_TIMEOUT || tdm_match_status == MM_SCOREBOARD)
	{
		if (!Q_stricmp (cmd, "commands"))
			TDM_Commands_f (ent);
		else if (!Q_stricmp (cmd, "settings") || !Q_stricmp (cmd, "matchinfo"))
			TDM_Settings_f (ent);
		else if (!Q_stricmp (cmd, "calltime") | !Q_stricmp (cmd, "pause") || !Q_stricmp (cmd, "ctime") ||
				!Q_stricmp (cmd, "time") || !Q_stricmp (cmd, "hold"))
			TDM_Timeout_f (ent);
		else if (!Q_stricmp (cmd, "ghost") || !Q_stricmp (cmd, "restore") || !Q_stricmp (cmd, "recover") | !Q_stricmp (cmd, "rejoin"))
			TDM_Ghost_f (ent);
		else if (!Q_stricmp (cmd, "win"))
			TDM_Win_f (ent);
		else if (!Q_stricmp (cmd, "observer") || !Q_stricmp (cmd, "spectate") || !Q_stricmp (cmd, "chase") ||
				!Q_stricmp (cmd, "spec") || !Q_stricmp (cmd, "obs"))
			TDM_Spectate_f (ent);
		else if (!Q_stricmp (cmd, "pickplayer") || !Q_stricmp (cmd, "pick"))
			TDM_PickPlayer_f (ent);
		else if (!Q_stricmp (cmd, "invite"))
			TDM_Invite_f (ent);
		else if (!Q_stricmp (cmd, "accept"))
			TDM_Accept_f (ent);
		else if (!Q_stricmp (cmd, "admin") || !Q_stricmp (cmd, "referee"))
			TDM_Admin_f (ent);
		else if (!Q_stricmp (cmd, "stopsound"))
			return true;	//prevent chat from our stuffcmds on people who have no sound
		else
			return true;	//don't print everything else as a text

		return true;
	}
	else
	{
		if (!Q_stricmp (cmd, "ready"))
			TDM_Ready_f (ent);
		else if (!Q_stricmp (cmd, "notready") || !Q_stricmp (cmd, "unready") || !Q_stricmp (cmd, "noready"))
			TDM_NotReady_f (ent);
		else if (!Q_stricmp (cmd, "kickplayer") || !Q_stricmp (cmd, "removeplayer") || !Q_stricmp (cmd, "remove"))
			TDM_KickPlayer_f (ent);
		else if (!Q_stricmp (cmd, "admin") || !Q_stricmp (cmd, "referee"))
			TDM_Admin_f (ent);
		else if (!Q_stricmp (cmd, "captain"))
			TDM_Captain_f (ent);
		else if (!Q_stricmp (cmd, "captains"))
			TDM_Captains_f (ent);
		else if (!Q_stricmp (cmd, "vote"))
			TDM_Vote_f (ent);
		else if (!Q_stricmp (cmd, "yes") || !Q_stricmp (cmd, "no"))
			TDM_Vote_f (ent);
		else if (!Q_stricmp (cmd, "lockteam") || !Q_stricmp (cmd, "lock"))
			TDM_Lockteam_f (ent, true);
		else if (!Q_stricmp (cmd, "unlockteam") || !Q_stricmp (cmd, "unlock"))
			TDM_Lockteam_f (ent, false);
		else if (!Q_stricmp (cmd, "pickplayer") || !Q_stricmp (cmd, "pick"))
			TDM_PickPlayer_f (ent);
		else if (!Q_stricmp (cmd, "invite"))
			TDM_Invite_f (ent);
		else if (!Q_stricmp (cmd, "accept"))
			TDM_Accept_f (ent);
		else if (!Q_stricmp (cmd, "teamskin"))
			TDM_Teamskin_f (ent);
		else if (!Q_stricmp (cmd, "teamname"))
			TDM_Teamname_f (ent);
		else if (!Q_stricmp (cmd, "teamready") || !Q_stricmp (cmd, "readyteam"))
			TDM_Changeteamstatus_f (ent, true);
		else if (!Q_stricmp (cmd, "teamnotready") || !Q_stricmp (cmd, "notreadyteam"))
			TDM_Changeteamstatus_f (ent, false);
		else if (!Q_stricmp (cmd, "menu") || !Q_stricmp (cmd, "ctfmenu") || !Q_stricmp (cmd, "inven"))
			TDM_ShowTeamMenu (ent);
		else if (!Q_stricmp (cmd, "commands"))
			TDM_Commands_f (ent);
		else if (!Q_stricmp (cmd, "join") || !Q_stricmp (cmd, "team"))
			TDM_Team_f (ent);
		else if (!Q_stricmp (cmd, "settings") || !Q_stricmp (cmd, "matchinfo"))
			TDM_Settings_f (ent);
		else if (!Q_stricmp (cmd, "observer") || !Q_stricmp (cmd, "spectate") || !Q_stricmp (cmd, "chase") ||
				!Q_stricmp (cmd, "spec") || !Q_stricmp (cmd, "obs"))
			TDM_Spectate_f (ent);
		else if (!Q_stricmp (cmd, "calltime") | !Q_stricmp (cmd, "pause") || !Q_stricmp (cmd, "ctime") ||
				!Q_stricmp (cmd, "time") || !Q_stricmp (cmd, "hold"))
			TDM_Timeout_f (ent);
		// stats.. this could use some cleanup
		else if (!Q_stricmp (cmd, "stats") || !Q_stricmp (cmd, "kills") || !Q_stricmp (cmd, "accuracy") ||
				!Q_stricmp (cmd, "damage") || !Q_stricmp (cmd, "weapons") || !Q_stricmp (cmd, "items") ||
				!Q_stricmp (cmd, "killstats") || !Q_stricmp (cmd, "deathstats"))
			TDM_Stats_f (ent, &current_matchinfo);
		else if (!Q_stricmp (cmd, "oldstats") || !Q_stricmp (cmd, "oldkills") || !Q_stricmp (cmd, "laststats") ||
				!Q_stricmp (cmd, "lastkills") || !Q_stricmp (cmd, "oldaccuracy") || !Q_stricmp (cmd, "lastaccuracy") ||
				!Q_stricmp (cmd, "olddamage") || !Q_stricmp (cmd, "lastdamage") || !Q_stricmp (cmd, "oldweapons") ||
				!Q_stricmp (cmd, "lastweapons") || !Q_stricmp (cmd, "olditems") || !Q_stricmp (cmd, "lastitems") ||
				!Q_stricmp (cmd, "oldkillstats") || !Q_stricmp (cmd, "lastkillstats") || !Q_stricmp (cmd, "lastdeathstats") ||
				!Q_stricmp (cmd, "olddeathstats"))
			TDM_Stats_f (ent, &old_matchinfo);
		else if (!Q_stricmp (cmd, "teamstats") || !Q_stricmp (cmd, "teamkills") || !Q_stricmp (cmd, "teamaccuracy") ||
				!Q_stricmp (cmd, "teamdamage") || !Q_stricmp (cmd, "teamweapons") || !Q_stricmp (cmd, "teamitems") ||
				!Q_stricmp (cmd, "teamkillstats") || !Q_stricmp (cmd, "teamdeathstats"))
			TDM_TeamStats_f (ent, &current_matchinfo);
		else if (!Q_stricmp (cmd, "oldteamstats") || !Q_stricmp (cmd, "oldteamkills") || !Q_stricmp (cmd, "lastteamstats") ||
				!Q_stricmp (cmd, "lastteamkills") || !Q_stricmp (cmd, "oldteamaccuracy") || !Q_stricmp (cmd, "lastteamaccuracy") ||
				!Q_stricmp (cmd, "oldteamdamage") || !Q_stricmp (cmd, "lastteamdamage") || !Q_stricmp (cmd, "oldteamweapons") ||
				!Q_stricmp (cmd, "lastteamweapons") || !Q_stricmp (cmd, "oldteamitems") || !Q_stricmp (cmd, "lastteamitems") ||
				!Q_stricmp (cmd, "oldteamkillstats") || !Q_stricmp (cmd, "lastteamkillstats") || !Q_stricmp (cmd, "lastteamdeathstats") ||
				!Q_stricmp (cmd, "oldteamdeathstats"))
			TDM_TeamStats_f (ent, &old_matchinfo);
 		else if (!Q_stricmp (cmd, "topshots"))
 			TDM_TopBottomShots_f (ent, false, true);
 		else if (!Q_stricmp (cmd, "teamtopshots"))
 			TDM_TopBottomShots_f (ent, true, true);
 		else if (!Q_stricmp (cmd, "bottomshots"))
 			TDM_TopBottomShots_f (ent, false, false);
 		else if (!Q_stricmp (cmd, "teambottomshots"))
 			TDM_TopBottomShots_f (ent, true, false);
		else if (!Q_stricmp (cmd, "oldscores") || !Q_stricmp (cmd, "oldscore") || !Q_stricmp (cmd, "lastscores") || !Q_stricmp (cmd, "lastscore"))
			TDM_OldScores_f (ent);
		else if (!Q_stricmp (cmd, "ghost") || !Q_stricmp (cmd, "restore") || !Q_stricmp (cmd, "recover") | !Q_stricmp (cmd, "rejoin"))
			TDM_Ghost_f (ent);
		else if (!Q_stricmp (cmd, "talk"))
			TDM_Talk_f (ent);
		else if (!Q_stricmp (cmd, "id") || !Q_stricmp (cmd, "ident"))
			TDM_Id_f (ent);
		//wision: some compatibility with old mods (ppl are lazy to learn new commands)
		else if (!Q_stricmp (cmd, "powerups"))
			TDM_Powerups_f (ent);
		else if (!Q_stricmp (cmd, "tl"))
			TDM_Timelimit_f (ent);
		else if (!Q_stricmp (cmd, "bfg"))
			TDM_Bfg_f (ent);
		else if (!Q_stricmp (cmd, "overtime") || !Q_stricmp (cmd, "ot") || !Q_stricmp (cmd, "tiemode"))
			TDM_Overtime_f (ent);
		else if (!Q_stricmp (cmd, "obsmode") || !Q_stricmp (cmd, "chat"))
			TDM_Obsmode_f (ent);
		else if (!Q_stricmp (cmd, "motd"))
			TDM_Motd_f (ent);
		else if (!Q_stricmp (cmd, "mute"))
			TDM_Mute_f (ent);
		else if (!Q_stricmp (cmd, "unmute"))
			TDM_Unmute_f (ent);
		else if (!Q_stricmp (cmd, "speclock"))
			TDM_Speclock_f (ent);
		else if (!Q_stricmp (cmd, "specinvite"))
			TDM_Specinvite_f (ent);
		else if (!Q_stricmp (cmd, "tskin"))
			TDM_TeamEnemySkin_f (ent, true);
		else if (!Q_stricmp (cmd, "eskin"))
			TDM_TeamEnemySkin_f (ent, false);
		else if (!Q_stricmp (cmd, "stopsound"))
			return true;	//prevent chat from our stuffcmds on people who have no sound
		else
			return false;
	}

	return true;
}
