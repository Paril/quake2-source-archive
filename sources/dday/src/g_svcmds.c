/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_svcmds.c,v $
 *   $Revision: 1.6 $
 *   $Date: 2002/06/04 19:49:47 $
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

int PlayerCountForTeam (int team_number);//faf

void	Svcmd_Test_f (void)
{
	gi.cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

void Svcmd_Mapinfo_f (void)
{
	gi.dprintf("%s (%i) ---\n",		team_list[0]->teamname, PlayerCountForTeam(0));//faf team_list[0]->total);
	gi.dprintf("  Score: %i/%i\n",	team_list[0]->score, team_list[0]->need_points);
	gi.dprintf("  Kills: %i/%i\n",	team_list[0]->kills, team_list[0]->need_kills);
	gi.dprintf("%s (%i) -----\n", team_list[1]->teamname, PlayerCountForTeam(1));//faf team_list[1]->total);
	gi.dprintf("  Score: %i/%i\n",	team_list[1]->score, team_list[1]->need_points);
	gi.dprintf("  Kills: %i/%i\n",	team_list[1]->kills, team_list[1]->need_kills);
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
	else if (Q_stricmp (cmd, "maplist") == 0) 
		Svcmd_Maplist_f (); 
	else if (Q_stricmp (cmd, "mapinfo") == 0)
		Svcmd_Mapinfo_f ();
	

	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

