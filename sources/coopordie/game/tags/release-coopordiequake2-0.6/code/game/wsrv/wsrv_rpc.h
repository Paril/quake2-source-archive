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
// wsrv_rpc.h -- primary header for WorldServer Remote Procedure Call

// *** FUNCTION ***

void WSrv_ClientConnect(edict_t *ent);
void WSrv_ClientDisconnect(const char *pszName);
void WSrv_Killed(edict_t *targ, edict_t *inflictor, edict_t *attacker);
void WSrv_GameLoopAddData(const char *szData);
void WSrv_GameLoop(volatile rpc_data_t *pRpcData);
void WSrv_AddCmd(char *pszCmd);
void WSrv_SendCmdImmediate(char *pszCmd);
void WSrv_CheckWorldServer(qboolean bFlush);
void WSrv_BypassCheckWorldServer();
void WSrv_MapGetXmlRules(volatile rpc_data_t *pRpcData);
qboolean WSrv_MapGetXmlRules_OK(volatile rpc_data_t *pRpcData);

void Cmd_WSrv_Login_f (edict_t *ent);
void Cmd_WSrv_Logout_f(edict_t *ent);

const int	WSrv_GetMapLock();
void		WSrv_SetMapLock(int i_iMapLock);
//$$void		WSrv_SetWsrvData(wsrv_data_t *i_pWsrvData);
void WSrv_DeserializeWsrvData(const char *i_pWsrvDataSerialised);
void WSrv_SerializeWsrvData(char *i_szVarName);
