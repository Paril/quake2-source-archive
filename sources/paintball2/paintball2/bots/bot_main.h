/*
Copyright (c) 2015 Nathan "jitspoe" Wulf, Digital Paint

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

#pragma once

#ifndef _BOT_MAIN_H_
#define _BOT_MAIN_H_

#include "../game/q_shared.h"
#include "../game/game.h"
#include "bot_importexport.h"

extern cvar_t *skill;
extern cvar_t *bot_debug;

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level

#define nu_rand(x)	((float)((float)(x) * (float)rand() / ((float)RAND_MAX + 1.0f)))

// bot_main.c
void BotInitLibrary (void);
void BotShutdown (void);
void BotHandleGameEvent (game_event_t event, edict_t *ent, void *data1, void *data2);
void BotRunFrame (int msec, float level_time);
qboolean BotCommand (edict_t *ent, const char *cmd, const char *cmd2, const char *cmd3, const char *cmd4);
void BotExitLevel (void);
void BotSpawnEntities (void);
void AddBot (const char *name);

extern bot_import_t bi;

// bot_move.c
void BotUpdateMovement (int msec);
void PmoveOriginToWorldOrigin (const pmove_t *pm, vec_t *origin_out);

// bot_observe.c
void BotObservePlayerInput (unsigned int player_index, const edict_t *ent, const pmove_t *pm);

// bot_navmesh.c
void BotAddPotentialNavmeshFromPmove (const edict_t *ent, const pmove_t *pm);

// bot_waypoints.c
void BotReadWaypoints (const char *mapname);
void BotWriteWaypoints (const char *mapname);

// bot_goals.c
void BotUpdateGoals (int msec);

// bot_aim.c
void BotAimAndShoot (int botindex, int msec);
void BotSetDesiredAimAnglesFromPoint (int botindex, const vec3_t point);

#endif
