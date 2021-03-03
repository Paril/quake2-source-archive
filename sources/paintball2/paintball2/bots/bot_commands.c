/*
Copyright (c) 2014 Nathan "jitspoe" Wulf, Digital Paint

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

#include "bot_main.h"
#include "bot_manager.h"
#include "../game/game.h"


static void AddBotCommand (const char *name)
{
	AddBot(name);
}


// The bot will get removed rom the manager in BotHandleDisconnect
static void RemoveBot (edict_t *ent)
{
	bi.DisconnectBot(ent);
}


static void RemoveAllBots (edict_t *ent_cmd)
{
	int bot_count = bots.count;

	while (bots.count)
	{
		RemoveBot(bots.ents[0]);
	}

	bi.cprintf(ent_cmd, PRINT_POPUP, "Removed %d bots.\n", bot_count);
}


static edict_t *GetBotEntByName (const char *name_to_find)
{
	int i;
	
	// search for exact name:
	for (i = 0; i < bots.count; ++i)
	{
		edict_t *ent = bots.ents[i];
		const char *name = bi.GetClientName(ent);

		if (Q_strcaseeq(name, name_to_find))
		{
			return ent;
		}
	}

	// search for partial match
	for (i = 0; i < bots.count; ++i)
	{
		edict_t *ent = bots.ents[i];
		const char *name = bi.GetClientName(ent);

		if (strstr(name, name_to_find)) // todo: make case insensitive
		{
			return ent;
		}
	}

	return NULL;
}


static void RemoveBotCommand (const char *nameToRemove, edict_t *ent_cmd)
{
	edict_t *ent;

	if (Q_strcaseeq(nameToRemove, "all"))
	{
		RemoveAllBots(ent_cmd);
		return;
	}

	ent = GetBotEntByName(nameToRemove);

	if (ent)
	{
		const char *name = bi.GetClientName(ent);

		bi.cprintf(ent_cmd, PRINT_POPUP, "Removed bot: %s\n", name);
		RemoveBot(ent);
		return;
	}

	bi.cprintf(ent_cmd, PRINT_POPUP, "No bots found with a name containing \"%s\".\n", nameToRemove);
}


// Debugging function to make bots move to player's location
void BotHere (edict_t *ent)
{
	int bot_index;

	for (bot_index = 0; bot_index < bots.count; ++bot_index)
	{
		BotSetGoal(bot_index, BOT_GOAL_REACH_POSITION, ent->s.origin);
	}
}


/*void BotCmd (edict_t *ent_cmd, const char *name_to_command, const char *cmd1, const char *cmd2)
{
	edict_t *bot_ent;

	if (Q_strcaseeq(name_to_command, "all"))
	{
		int bot_index;

		for (bot_index = 0; bot_index < bots.count; ++bot_index)
		{
			bot_ent = bots.ents[bot_index];
			BotCmdExec(bot_ent, cmd1, cmd2);
		}

		return;
	}

	bot_ent = GetBotEntByName(name_to_command);

	if (bot_ent)
	{
		BotCmdExec(bot_ent, cmd1, cmd2);
		return;
	}

	bi.cprintf(ent_cmd, PRINT_POPUP, "No bots found with a name containing \"%s\".\n", name_to_command);
}*/


qboolean BotCommand (edict_t *ent, const char *cmd, const char *cmd2, const char *cmd3, const char *cmd4)
{
	if (Q_strcaseeq(cmd, "addbot"))
	{
		AddBotCommand(cmd2);
		return true;
	}
	else if (Q_strcaseeq(cmd, "removebot"))
	{
		RemoveBotCommand(cmd2, ent);
		return true;
	}
	else if (Q_strcaseeq(cmd, "removeallbots"))
	{
		RemoveAllBots(ent);
		return true;
	}
	else if (bot_debug->value)
	{
		if (Q_strcaseeq(cmd, "bot_here"))
		{
			BotHere(ent);
			return true;
		}
		else if (Q_strcaseeq(cmd, "bot_debug_astar_start"))
		{
			AStarDebugStartPoint(ent->s.origin);
			return true;
		}
		else if (Q_strcaseeq(cmd, "bot_debug_astar_end"))
		{
			AStarDebugEndPoint(ent->s.origin);
			return true;
		}/*
		else if (Q_strcaseeq(cmd, "botcmd") || Q_strcaseeq(cmd, "botcommand"))
		{
			BotCmd(ent, cmd2, cmd3, cmd4);
			return true;
		}*/
	}

	return false;
}
