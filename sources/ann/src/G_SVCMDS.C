#include "g_local.h"
#include "g_brrr.h"

/* *** SV PLAYERS *** */
void Svcmd_Players_f (void);
int PlayerSort (void const *a, void const *b);
/* *** */

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
	else if ((Q_stricmp (cmd, "players") == 0) || (Q_stricmp (cmd, "who") == 0))
		Svcmd_Players_f ();
	else if (Q_stricmp(cmd,"firstmap")==0)
		firstmap();
	else if (Q_stricmp(cmd,"nextmap")==0)
		nextmap();
	else if (Q_stricmp(cmd,"listmaps")==0)
		listmaps();
	else if (Q_stricmp(cmd,"reloadmaplist")==0)
		reloadmaplist();

	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

void Svcmd_Players_f (void)
{
	int     i;
	int     count;
	char    small[64];
	char    large[1280];
	int     index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	gi.cprintf (NULL, PRINT_HIGH, "ID, Frags, NetName, IP\n");
	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %3i %c%s %s\n",
			index[i],
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			(game.clients[index[i]].resp.allow_devmode ? '$' : ' '),
			game.clients[index[i]].pers.netname,
			Info_ValueForKey(game.clients[index[i]].pers.userinfo, "ip"));
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{   // can't print all of them in one packet
			strcat (large, "...\n");
            break;
        }
        strcat (large, small);
    }

    gi.cprintf (NULL, PRINT_HIGH, "%s\n%i players\n", large, count);
}

