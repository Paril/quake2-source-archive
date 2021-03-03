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


static void BotHandleDisconnect (const edict_t *ent)
{
	int i;
	int count = bots.count;
	qboolean removed = false;

	// Check to see if the entity that disconnected was one of the bots.  If so, remove it from the manager.
	for (i = 0; i < count; ++i)
	{
		// Shift all entries over 1 since a previous entry was removed.
		if (removed)
		{
			bots.ents[i - 1] = bots.ents[i];
		}
		else if (bots.ents[i] == ent)
		{
			--bots.count;
			removed = true;
		}
	}

	// todo: BotUpdateGoals(); // Index may be offset for some bots, so we need to update all bots
}


void BotHandleGameEvent (game_event_t event, edict_t *ent, void *data1, void *data2)
{
	switch (event)
	{
	case EVENT_DISCONNECT:
		BotHandleDisconnect(ent);
		break;
	}
	// Events like round starts, flag grabs, eliminations, etc. will be passed in here
}


// todo: check if a bot has been removed or replaced.  Update bot manager

