
#include "g_local.h"
#include "bot_procs.h"

void	Svcmd_Test_f (void)
{
	gi.cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

void	Svcmd_Bots_f (void)
{
	int i=2, j, len;
	char	name[128];

	while (i < gi.argc())
	{
		strcpy(name, gi.argv(i));
		len = strlen(name);

		// convert '~' to ' '
		for (j=0; j<len; j++)
			if (name[j] == '~')
				name[j] = ' ';

		spawn_bot(name);

		i++;
	}
}

void	Svcmd_Teams_f (void)
{
	int arg=2, i;
	char	team[128];

	while (arg < gi.argc())
	{
		strcpy(team, gi.argv(arg));

		i=0;
		while (i<MAX_TEAMS)
		{
			if (!bot_teams[i])
				break;

#ifdef _WIN32
			if (!_stricmp(bot_teams[i]->teamname, team) || !_stricmp(bot_teams[i]->abbrev, team))
#else
			if (!strcasecmp(bot_teams[i]->teamname, team) || !strcasecmp(bot_teams[i]->abbrev, team))
#endif
			{	// found the team, so add the bots
				bot_teams[i]->ingame = true;	// bots will be added automatically (below)
				break;
			}

			i++;
		}

		arg++;
	}
}

int	force_team = CTF_NOTEAM;

void	Svcmd_Blueteam_f (void)
{
	int i=2;

	force_team = CTF_TEAM2;
	while (i < gi.argc())
	{
//gi.dprintf("Spawning: \"%s\"\n", gi.argv(i));
		spawn_bot(gi.argv(i));

		i++;
	}

	force_team = CTF_NOTEAM;
}

void	Svcmd_Redteam_f (void)
{
	int i=2;

	force_team = CTF_TEAM1;
	while (i < gi.argc())
	{
		spawn_bot(gi.argv(i));

		i++;
	}

	force_team = CTF_NOTEAM;
}

/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void	ServerCommand (void)
{
	char	*cmd;

	cmd = gi.argv(1);
	if (Q_stricmp (cmd, "test") == 0)
		Svcmd_Test_f ();
	else if (Q_stricmp (cmd, "bots") == 0)
		Svcmd_Bots_f ();
	else if (Q_stricmp (cmd, "teams") == 0)
		Svcmd_Teams_f ();
	else if (Q_stricmp (cmd, "bluebots") == 0)
		Svcmd_Blueteam_f ();
	else if (Q_stricmp (cmd, "redbots") == 0)
		Svcmd_Redteam_f ();
	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

