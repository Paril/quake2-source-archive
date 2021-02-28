/*
Copyright (C) 2001-2009 Pat AfterMoon (www.aftermoon.net)

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
// wsrv_trap.h -- WorldServer function trap prototype

#include "wsrv.h"
#include "wsrv_rpc.h"

qboolean trap_WSRV_ClientCommand(edict_t *ent, char *cmd, char *parm);
void trap_WSRV_Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void trap_WSRV_UseTargetGoal(edict_t *ent, edict_t *other, edict_t *activator);
void trap_WSRV_UseTargetSecret(edict_t *ent, edict_t *other, edict_t *activator);

void trap_WSRV_ShutdownGame();
void trap_WSRV_ExitLevel();
void trap_WSRV_RunFrame();
void trap_WSRV_InitGame();
qboolean trap_WSRV_ServerCommand(char *cmd);
void BeginNewIntermission (char *changemap, vec3_t origin, vec3_t angles);
qboolean trap_WSRV_player_die1(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void trap_WSRV_player_die2(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
qboolean trap_WSRV_ClientConnect(edict_t *ent, char *userinfo);
void trap_WSRV_ClientDisconnect(edict_t *ent);
