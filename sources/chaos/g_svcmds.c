#include "g_local.h"
#include "c_botai.h"

void LoadMaplist();

void	Svcmd_Test_f (void)
{
	gi.cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}


/* FWP Move to next map in the current rotation */

void Svcmd_nextmap_f()

{
  bprintf2 (PRINT_HIGH, "Advancing to next level.\n");  
  EndDMLevel();
 }

/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/

void ClearMaplist();

void	ServerCommand (void)
{
	char	*cmd;

	cmd = gi.argv(1);
	if (Q_stricmp (cmd, "test") == 0)
		Svcmd_Test_f ();
	else if (Q_stricmp(cmd, "addbots") == 0)
		Svcmd_addbots_f();
	else if (Q_stricmp(cmd, "killbot") == 0)
		Svcmd_killbot_f(gi.argv(2));
	else if (Q_stricmp(cmd, "nextmap") == 0)
		Svcmd_nextmap_f();
	else if (Q_stricmp(cmd, "ml") == 0)
	{
		if (Q_stricmp(gi.argv(2), "0") == 0)	//maprotation off
		{
			ClearMaplist();
			maplist.mlflag = 0; //off
			gi.cprintf (NULL, PRINT_HIGH, "Map rotation OFF!\n\n");
		}
		else if (Q_stricmp(gi.argv(2), "1") == 0)	//start sequential rotation
		{
			if (maplist.nummaps > 0)  // does a maplist exist? 
			{
				maplist.mlflag = 1; //sequential
				maplist.currentmap = -1;
				gi.cprintf (NULL, PRINT_HIGH, "Sequential map rotation ON!\n\n");
                EndDMLevel();
			}
            else 
                 gi.cprintf (NULL, PRINT_HIGH, "You have to load a maplist first!\n\n");
			
		}
		else if (Q_stricmp(gi.argv(2), "2") == 0)	//start random rotation
		{
			if (maplist.nummaps > 0)  // does a maplist exist? 
			{
				maplist.mlflag = 2; //random
				maplist.currentmap = -1;
				gi.cprintf (NULL, PRINT_HIGH, "Random map rotation ON!\n\n");
                EndDMLevel();
			}
            else 
                 gi.cprintf (NULL, PRINT_HIGH, "You have to load a maplist first!\n\n");
		}
		/*else if (Q_stricmp(gi.argv(2), "goto") == 0)	//jump to map X in list
		{
			if (maplist.nummaps > 0)  // does a maplist exist? 
			{
				if (maplist.mlflag > 0)
				{
					int num = atoi(gi.argv(3));

					if (num < maplist.nummaps)
					{
						maplist.currentmap = num;
						gi.cprintf (NULL, PRINT_HIGH, "Current map is %d!\n\n", num);
						EndDMLevel();
					}
					else
						gi.cprintf (NULL, PRINT_HIGH, "Map number %d not found!\n\n", num);
				}
				else
					gi.cprintf (NULL, PRINT_HIGH, "You have to start the maprotation with <sv ml 1 or 2> first!\n\n");
			}
            else 
                 gi.cprintf (NULL, PRINT_HIGH, "You have to load a maplist first!\n\n");
			
		}*/
		else if (Q_stricmp(gi.argv(2), "") == 0)	//print maplist
		{
			if (maplist.nummaps > 0)  // does a maplist exist? 
			{
				int i;

				gi.cprintf(NULL, PRINT_HIGH, "Current maplist:\n");
				
				for (i = 0;i < maplist.nummaps;i++)
				{ 
					gi.cprintf(NULL, PRINT_HIGH, "...%s,ctf=%c,lightsoff=%c\n", maplist.mapnames[i], maplist.ctf[i], maplist.lightsoff[i]);
				}
				gi.cprintf(NULL, PRINT_HIGH, "\n");
			}
            else 
                 gi.cprintf (NULL, PRINT_HIGH, "Load a maplist with <sv ml maplistname>!\n\n");
			
		}
		else	//load maplist
		{
			ClearMaplist();
			LoadMaplist(gi.argv(2));
		}
	}
	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}





