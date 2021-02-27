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
// wsrv.h -- primary header for WorldServer Remote Procedure Call

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

// *** CONSTANT ***

#define RPC_STATUS_READY        0
#define RPC_STATUS_IN_PROGRESS  1
#define RPC_STATUS_OK           2
#define RPC_STATUS_ERROR        3
#define RPC_STATUS_WAIT_ACK     4


#define	RPC_RESULT_SIZE				8192
#define RPC_XMLRULES_RESULT_SIZE	65536

// *** TYPE ***

typedef enum
{
	RPC_NONE=0,
	RPC_LOGIN,
	RPC_LOGOUT,
	RPC_PLAYER_CREATEGAME,

	WSRV_GAMELOOP,
	WSRV_PLAYERGETINFO,
	WSRV_PLAYERSSETINFO,
	WSRV_MAPGETXMLRULES,

} rpc_func_num_t;

typedef struct
{
	int             iData ;
	unsigned int    iFlags ;
	char            *pszRequest ;
	char            *pszResult ;
	unsigned int    iResultSize ;
	unsigned char   byRpcStatus ;
} rpc_data_t;

typedef struct
{
	int			iData;
} wsrv_data_client_t;

typedef struct
{
	char		szId[8];					// Id of the quake server for RPC
	char		szSession[64];				// Session handle for RPC
	int			iMapLock;					// Locking counter for prevent manual map changement
} wsrv_data_t;

// *** MACROS ***

#define SIZE_TAB( Tab )  ( sizeof(Tab) / sizeof(Tab[0]) )

// *** FUNCTION ***

void RPC_Log(char *fmt, ...) ;
void RPC_FlushLog(char *pszLog, int iSizeMax);
void RPC_LogFile(char *msg);

void RPC_Login(volatile rpc_data_t *pRpcData, char *szName, char *szPass);
qboolean RPC_Login_OK(volatile rpc_data_t *pRpcData, char *pszErrMsg, int iSizeErrMsg);

void RPC_Logout(volatile rpc_data_t *pRpcData);

void RPC_PlayerCreateGame(volatile rpc_data_t *pRpcData);
qboolean RPC_PlayerCreateGame_OK(volatile rpc_data_t *pRpcData, char *pszMsg, int iSizeMsg);

// ---
void HTTP_SendRpc(volatile rpc_data_t *pRpcData );
void RPC_Sleep(unsigned long dwlaps) ;
// ---
