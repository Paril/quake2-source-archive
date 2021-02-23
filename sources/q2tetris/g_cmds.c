#include "g_local.h"
#include "m_player.h"

//=================================================================================


/*
=================
ClientCommand
=================
*/
void issueCommand(T_arena_t* arena, T_command_t command);
void startGame (edict_t* ent);

void ClientCommand (edict_t *ent)
{
	char	*cmd;
	int i;
	
	if (!ent->client)
		return;		// not fully in game yet

	i = 0;
	while ( (cmd = gi.argv(i++))[0] ) {
	//cmd = gi.argv(i++);
		if (i > 1) gi.dprintf("processing command #%d\n",i-1);
		if (Q_stricmp (cmd, "gameversion") == 0)
		{
			gi.cprintf (ent, PRINT_HIGH, "%s : %s\n", GAMEVERSION, __DATE__);
		}
		else if (Q_stricmp (cmd, "fov") == 0)
		{
			ent->client->ps.fov = atoi(gi.argv(1));
			if (ent->client->ps.fov < 1)
				ent->client->ps.fov = 90;
			else if (ent->client->ps.fov > 160)
				ent->client->ps.fov = 160;
		} else if (Q_stricmp (cmd, "helpscrn") == 0) {
			if (ent->client->ps.stats[8]) { 
				ent->client->ps.stats[8] = 0;
			}
			else
			{
				ent->client->ps.stats[8] = gi.imageindex("t_helpscrn");
			}
		} else if (Q_stricmp (cmd, "block_left") == 0) {
			issueCommand(ent->client->arena, BL_LEFT);
		} else if (Q_stricmp (cmd, "block_right") == 0) {
			issueCommand(ent->client->arena, BL_RIGHT);
		} else if (Q_stricmp (cmd, "block_rot") == 0) {
			issueCommand(ent->client->arena, BL_ROT);
		} else if (Q_stricmp (cmd, "block_drop") == 0) {
			issueCommand(ent->client->arena, BL_DROP);
		} else if (Q_stricmp (cmd, "putaway") == 0) {
			// lookie, id added a vaguely bizarre hack
			// I'm not quite sure what they did, but I don't like it
			gi.AddCommandString("menu_main");
		} else if (Q_stricmp (cmd, "new_game") == 0) {
			if ((ent->client->state == NO_GAME) || (ent->client->state == GAME_OVER))
				startGame(ent);
		} else
			gi.cprintf (ent, PRINT_HIGH, "Bad command: %s\n", cmd);
	}
}

