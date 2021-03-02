
#include "g_local.h"


void	Svcmd_Test_f (void)
{
	gi.cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
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
	// DEREK - start of adding commands
	/*else if (Q_stricmp (cmd, "endmatch") == 0)
	{
		//check for conditions first
		EndMatch ();
	}	*/
	else if (Q_stricmp (cmd, "lockdown") == 0)
		Cmd_Lockdown_f (NULL);
	else if (Q_stricmp (cmd, "matchfragset") == 0)
		Cmd_MatchFragSet_f (NULL);
	else if (Q_stricmp (cmd, "matchtimeset") == 0)
		Cmd_MatchTimeSet_f (NULL);
	else if (Q_stricmp (cmd, "modstatus") == 0)
		Cmd_DisplayModMode_f (NULL);
	else if (Q_stricmp (cmd, "playerlist") == 0)
		Cmd_PlayerList_f (NULL);
	else if (Q_stricmp (cmd, "powerups") == 0)
		Cmd_Powerups_f (NULL);
	else if (Q_stricmp (cmd, "restart") == 0)
		Cmd_Restart_f (NULL);
	else if (Q_stricmp (cmd, "timerset") == 0)
		Cmd_TimerSet_f (NULL);
	// DEREK - end of adding commands
	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

