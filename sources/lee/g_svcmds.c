#include "g_local.h"
char *ClientTeam (edict_t *ent);

void	Svcmd_Test_f (void)
{
	gi.cprintf (NULL, PRINT_HIGH, "Svcmd_Test()\n");
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
	char	*cmd = gi.argv(1);
	edict_t *bot;
	int i,i2;

   if (Q_stricmp(cmd, "bot_num") == 0)   
		{			
		char *arg2 = gi.argv(2);
		number_bot = atoi(arg2);

		//teamplay
		if ((deathmatch->value) && ((int)(dmflags->value) & (DF_MODELTEAMS)))
		{
			edict_t *player;

			player = G_Find (NULL, FOFS(classname), "player");

			if (player != NULL)
			{
				if (strcmp(ClientTeam(player),"male") == 0)
					TP_player_team = 0;
				if (strcmp(ClientTeam(player),"female") == 0)
					TP_player_team = 1;
				if (strcmp(ClientTeam(player),"cyborg") == 0)
					TP_player_team = 2;

				TP_start_bot = 0;
			}
		}
		//end of tp

			if (number_bot > current_number_bot) 
			{
				// add bot to game
				number_bot=number_bot - current_number_bot;

				for (i=0;i<number_bot;i++)													
						Bot_Create();
					
			} 
			
			if (number_bot < current_number_bot) 
			{				
				// remove bots from game
				current_number_bot = current_number_bot - number_bot;
				//TP_start_bot = current_number_bot;
				for (i2=0;i2<current_number_bot;i2++)
					{		
					for (i = (maxclients->value-1-number_bot); i > 0; i--)
						{
						bot = g_edicts + i + 1;

						if (bot->client && (Q_stricmp("bot", bot->classname) == 0))
							{
							ClientDisconnect(bot);
							G_FreeEdict(bot);							
							i= -1;
							}
            
						} 

					}		
				current_number_bot = number_bot;
			}

		}
    else if (Q_stricmp (cmd, "test") == 0)
		Svcmd_Test_f ();
	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command: %s\n", cmd);
}

