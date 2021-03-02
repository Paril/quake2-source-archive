
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
	else if (process_bop_cmds(cmd, gi.argv(2)) == true)	// MATT
		return;
	else
		gi.cprintf(0, PRINT_HIGH, "Unknown server command!\n");
}

