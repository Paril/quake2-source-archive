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
// g_wsrv.c -- WorldServer Remote Procedure Call for game dll

#include "../g_local.h"
#include "wsrv.h"
#include "wsrv_rpc.h"

//-----------------------------------------------------------------------------
// CONST
//-----------------------------------------------------------------------------

// !!! if this is changed, it must be changed in qcommon.h too !!!
#define PORT_SERVER	    27910

#define WSRV_SEND_DATA_SECONDS 30

//-----------------------------------------------------------------------------
// VARIABLES
//-----------------------------------------------------------------------------

float l_WSrv_time = 0 ;

// Variable for debug log file
static FILE    *l_RPCLOG_file ;
static char     l_RPCLOG_Text[4096] ;
static int      l_RPCLOG_Len = 0 ;
static int      l_RPCLOG_Count = 0 ;

// Internal data for WorldServer link management
static wsrv_data_t *l_pWsrvData;

//-----------------------------------------------------------------------------
// Return a pointer on a client entity with a given name (or NULL if no match)
//-----------------------------------------------------------------------------
edict_t *GetClientEntByName(char *szName)
{
	edict_t *cl_ent;
	int i;

	for ( i=0 ; i<maxclients->value ; i++ )
	{
		cl_ent = g_edicts + 1 + i;
		
		if ( cl_ent->client->pers.connected || cl_ent->inuse )
			if ( stricmp(cl_ent->client->pers.netname, szName)==0 )
				return cl_ent;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Flush the message loging buffer
//-----------------------------------------------------------------------------
void RPC_FlushLog(char *pszLog, int iSizeMax)
{
	l_RPCLOG_Count++ ;
	strncpy(pszLog, l_RPCLOG_Text, iSizeMax-1) ;
	pszLog[iSizeMax-1] = '\0' ;

	l_RPCLOG_Len = 0 ;
	l_RPCLOG_Text[0] = '\0' ;
}

//-----------------------------------------------------------------------------
// Message log for the WorldServer
// The last message is always lost for the worldserver (but not for the local 
// file) because each message send the log of the previous . At each dll reload 
// we lost a log message, it's not a bug, it's a feature ;-)
//-----------------------------------------------------------------------------
void RPC_Log(char *fmt, ...)
{
	va_list		argptr;
	char		*pszTmp ;
	char		msg[16384];
	int			iLen ;

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	RPC_LogFile(msg);

	pszTmp = strchr(msg, '&') ;
	while ( pszTmp )
	{
		*pszTmp = '#' ;
		pszTmp = strchr(pszTmp, '&') ;
	}
/*
	pszTmp = strchr(msg, '\n') ;
	while ( pszTmp )
	{
		*pszTmp = '\t' ;
		pszTmp = strchr(pszTmp, '\n') ;
	}
*/
	// Truncate the message if it is too long
	iLen = strlen(msg) ;
	if ( iLen>256 )
		iLen = 256 ;

	// Store the message for sending it to the WorldServer
	if ( (sizeof(l_RPCLOG_Text) - l_RPCLOG_Len) > (iLen+7) )
	{
		strncpy(l_RPCLOG_Text+l_RPCLOG_Len, msg, iLen) ;
		l_RPCLOG_Text[l_RPCLOG_Len+iLen] = '\0' ;
		l_RPCLOG_Len += iLen ;
	}
	else if ( (sizeof(l_RPCLOG_Text) - l_RPCLOG_Len) > 7 )
	{
		strcpy(l_RPCLOG_Text+l_RPCLOG_Len, "<full>") ;
		l_RPCLOG_Len += 6 ;
	}
}

//-----------------------------------------------------------------------------
// Store a message in the local log file. If the log file wasn't already opened,
// then, we open it.
//-----------------------------------------------------------------------------
void RPC_LogFile(char *msg)
{
	char szDate[32], szTime[32];

	if (!l_RPCLOG_file)
		l_RPCLOG_file = fopen ("wsrv.log", "a");

	if (l_RPCLOG_file)
	{
		fprintf (l_RPCLOG_file, "--------------------------------------------------------------------------------\n");
		fprintf (l_RPCLOG_file, "%s %s\n", _strdate(szDate), _strtime(szTime));
		fprintf (l_RPCLOG_file, msg);
		fprintf (l_RPCLOG_file, "\n\n");
		fflush (l_RPCLOG_file);          // force it to save every time
	}
}

//-----------------------------------------------------------------------------
// Light XML parser for simple tag
//-----------------------------------------------------------------------------
static qboolean XML_Parser(const char *pszData, char *pszTag, char *pszVar, int iSize)
{
	char	szBegin[64], szEnd[64] ;
	char	*pszBegin, *pszEnd ;
	int		iSizeData;

	// Default value is empty string
	*pszVar = '\0' ;

	Com_sprintf( szBegin, sizeof(szBegin), "<%s>", pszTag );
	pszBegin = strstr(pszData, szBegin) ;
	if ( pszBegin )
	{
		pszBegin += strlen(szBegin) ;

		Com_sprintf( szEnd, sizeof(szEnd), "</%s>", pszTag );
		pszEnd = strstr(pszBegin, szEnd) ;
		if ( pszEnd )
		{
			iSizeData = pszEnd - pszBegin;
			if ( iSize >= iSizeData )
			{
				iSize = pszEnd - pszBegin ;
				memcpy(pszVar, pszBegin, iSize) ;
				pszVar[iSize] = '\0';
				return true;
			}
			else
				RPC_Log("ERROR_XML_PARSER\tData too big\t%d bytes inside %s tag\n", iSizeData, pszTag);
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Treat a RPC error
//-----------------------------------------------------------------------------
void RPC_Error(volatile rpc_data_t *pRpcData, char *pszErrMsg, int iSizeErrMsg)
{
	char	szStatus[16];
	char	szErrCode[16];
	char	szErrMsg[64];

	if (    XML_Parser(pRpcData->pszResult, "status",  szStatus,  sizeof(szStatus))
		 && strcmp(szStatus, "ERR")==0
		 && XML_Parser(pRpcData->pszResult, "errcode", szErrCode, sizeof(szErrCode))
		 && XML_Parser(pRpcData->pszResult, "errmsg",  szErrMsg,  sizeof(szErrMsg)) )
	{
		Com_sprintf( pszErrMsg, iSizeErrMsg, "%s\0%s", szErrMsg, szErrCode );
	}
	else
		Com_sprintf( pszErrMsg, iSizeErrMsg, "Invalid WorldServer reply" );

	Com_Printf ("### RPC_Error : %s\n", pszErrMsg);
}

//-----------------------------------------------------------------------------
// Manage common tag that can be received with every worldserver answer
//
//	<cvar_set> string </cvar_set>
//
//	<cvar_forceset> string </cvar_forceset>
//
//	<bprintf> message </bprintf>
//
//	<serverlist>
//		<adr1> ip:port </adr1>
//		<adr2>
//		:
//		<adr8>
//	</serverlist>
//
//	<client_set>
//		<client1>
//			<name> string </name>
//			<health> integer </health>
//			<inventory> string <inventory>
//			<spectator> integer </spectator>
//		</client1>
//		<client2>
//		:
//		<client8>
//	</client_set>
//
//	<console>
//      <command>
//          map startmap
//      </command>
//  </console>
//
//-----------------------------------------------------------------------------
void WSrv_CommonReplyTag_OK(char *pszResult)
{
	char	szTmp[8192];
	char	*pszTag, *pszVal ;
	char	szData[1024], szSubData[1024];
	int		i;
	char	szName[16];
	char	*pszItem, *pszNb ;
	int		iNb,iItem;


	// A CVAR_SET is requested by the worldserver
	if ( XML_Parser(pszResult, "cvar_set",  szTmp,  sizeof(szTmp)) )
	{
		strtok(szTmp, "<") ;
		while ( pszTag = strtok(NULL, ">") )
		{
			pszVal = strtok(NULL, "<");
			gi.cvar_set( pszTag, pszVal );
			strtok(NULL, "<");      // next Cvar
		}
	}

	// A CVAR_FORCESET is requested by the worldserver
	if ( XML_Parser(pszResult, "cvar_forceset",  szTmp,  sizeof(szTmp)) )
	{
		strtok(szTmp, "<") ;
		while ( pszTag = strtok(NULL, ">") )
		{
			pszVal = strtok(NULL, "<");
			gi.cvar_forceset( pszTag, pszVal );
			
			// NOTE: hack to add a prefix to the hostname in case of challenge mode
			if ( strcmp(pszTag, "challengemode") == 0 && atoi(pszVal) != 0 )
			{
				cvar_t	*name = gi.cvar ("name", "", 0);
				if ( name )
				{
					gi.cvar_forceset( "hostname", va("[C] %s", name->string ) );
				}
			}

			strtok(NULL, "<");      // next Cvar
		}
	}

	// Console command is requested by the worldserver
	if ( XML_Parser(pszResult, "console",  szTmp,  sizeof(szTmp)) )
	{
		strtok(szTmp, "<") ;
		while ( pszTag = strtok(NULL, ">") )
		{
			if ( strcmp(pszTag, "command") == 0 )
			{
				pszVal = strtok(NULL, "<");		// the command text
				gi.AddCommandString(va("%s\n", pszVal));
				strtok(NULL, "<");				// next command
			}
		}
	}

	// Receive a broadcast message from the WorldServer
	if ( XML_Parser(pszResult, "bprintf", szData, sizeof(szData)) )
	{
		gi.bprintf(PRINT_MEDIUM, va("%s\n", szData)) ;
	}

	// Receive the server list from the WorldServer
	if ( XML_Parser(pszResult, "serverlist", szData, sizeof(szData)) )
	{
		for ( i=0 ; i<8 ; i++ ) 
		{
			gi.WriteByte (svc_stufftext);
			if ( XML_Parser(pszResult, va("adr%d", i), szSubData, sizeof(szSubData)) )
				Com_sprintf(szTmp, sizeof(szTmp), "adr%d %s\n", i, szSubData);
			else
				Com_sprintf(szTmp, sizeof(szTmp), "adr%d \"\"\n", i);

			gi.WriteString(szTmp);
			gi.multicast(NULL, MULTICAST_ALL);
		}
	}

	// Receive a playergetinfo answer from the WorldServer
	if ( XML_Parser(pszResult, "client_set", szData, sizeof(szData)) )
	{
		edict_t *ent;

		for ( i=0 ; i<maxclients->value ; i++ )
		{
			if (    XML_Parser(szData, va("client%d", i), szSubData, sizeof(szSubData))
				 && XML_Parser(szSubData, "name", szName, sizeof(szName))
				 && (ent = GetClientEntByName(szName)) != NULL )
			{
				if ( XML_Parser(szSubData, "health", szTmp,    sizeof(szTmp)) )
					ent->client->pers.health = atoi(szTmp);

				if ( XML_Parser(szSubData, "inventory", szTmp, sizeof(szTmp)) )
				{
					// Reset inventory, before filling it
					memset(ent->client->pers.inventory, 0, sizeof(game.clients[0].pers.inventory));

					pszItem = strtok(szTmp, ":") ;
					while ( pszItem )
					{
						pszNb = strtok(NULL, ",") ;
						if ( pszNb )
						{
							iNb   = atoi(pszNb) ;
							iItem = atoi(pszItem);
			
							ent->client->pers.inventory[iItem] = iNb ;
						}
						pszItem = strtok(NULL, ":") ;
					}
					
					// Check the current weapon or select the next one
					SelectPrevItem (ent, IT_WEAPON);
					SelectNextItem (ent, IT_WEAPON);

					if (   (ent->client->pers.selected_item != -1)
						&& (itemlist[ent->client->pers.selected_item].flags & IT_WEAPON) )
					{
						ent->client->pers.weapon = &itemlist[ent->client->pers.selected_item];
						ent->client->pers.lastweapon = &itemlist[ent->client->pers.selected_item];
					}
				}

				if ( XML_Parser(szSubData, "spectator", szTmp, sizeof(szTmp)) )
					ent->client->pers.spectator = atoi(szTmp) != 0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Called by the standard Killed function for reporting each kill to the WorldServer
//-----------------------------------------------------------------------------
#define MOD_SELF_FIRE   0x4000000
//-----------------------------------------------------------------------------
void WSrv_Killed(edict_t *targ, edict_t *inflictor, edict_t *attacker)
{
	int		iMeansOfDeath;
	char	*pszAttacker, *pszCommand, *pszTarg ;
	char	szData[128];

	if ( targ->deadflag == DEAD_DEAD )
		return;

	if ( attacker->deadflag == DEAD_DEAD )
		return;

	// Additionnal data : mapname and position of the target
	Com_sprintf(szData, sizeof(szData), "%s,%d,%d,%d,%0.1f",
		level.mapname,
		(int)targ->s.origin[0], 
		(int)targ->s.origin[1], 
		(int)targ->s.origin[2],
		level.time );

	if ( attacker->client && (targ->svflags & SVF_MONSTER) )
	{   // a player kill a monster : attacker KIll target
		pszAttacker   = attacker->client->pers.netname ;
		pszCommand    = "ki" ;
		pszTarg       = targ->classname ;
		iMeansOfDeath = meansOfDeath ;
	}
	else if ( targ->client )
	{
		// Force immediat send of data, because a player death is a priority data
		WSrv_BypassCheckWorldServer();

		pszTarg = targ->client->pers.netname ;

		iMeansOfDeath = meansOfDeath ;
	
		if (attacker == targ)
			iMeansOfDeath |= MOD_SELF_FIRE;

		if (attacker && attacker->client)
		{
			if (coop->value)
				iMeansOfDeath |= MOD_FRIENDLY_FIRE;
			pszCommand  = "pk" ;
			pszAttacker = attacker->client->pers.netname ;
		}
		else
		{
			pszCommand  = "kb" ; 
			if (attacker && (attacker->svflags & SVF_MONSTER))
				pszAttacker = attacker->classname ;
			else
				pszAttacker = "" ;
		}
	}
	else
		return ;
	
	WSrv_GameLoopAddData(va("%s,%s,%s,%X,%s/",pszAttacker, pszCommand, pszTarg,iMeansOfDeath,szData));
}

//-----------------------------------------------------------------------------
// Called by the standard use_target_goal or use_target_secret functions for 
// reporting events to the WorldServer.
// ex: "player,Pat,target_secret,base1,t77"
//-----------------------------------------------------------------------------
void WSRV_UseTarget(edict_t *ent, edict_t *other, edict_t *activator)
{
    WSrv_GameLoopAddData(va("%s,%s,%s,%s,%s/",
        activator->client ? activator->client->pers.netname : "",
        ent->classname,
        activator->classname, 
        level.mapname,
        ent->targetname) );
}

//-----------------------------------------------------------------------------
// Inform the WorldServer that a player quit the server
//-----------------------------------------------------------------------------
void WSrv_ClientDisconnect(const char *pszName)
{
	WSrv_GameLoopAddData(va("%s,quit/", pszName));
	// Force immediat send of data, because a player quit is a priority data
	// disabled for 0.5a version : need more test // WSrv_BypassCheckWorldServer();
}

//-----------------------------------------------------------------------------
// Inform the WorldServer that a player join the server
//-----------------------------------------------------------------------------
void WSrv_ClientConnect(edict_t *ent)
{
	WSrv_AddCmd(va("GetInventory,%s", ent->client->pers.netname));

	if ( ent != g_edicts+1 )    // Inform GameLoop only if it's not the server
	{
		WSrv_GameLoopAddData(va("%s,join/", ent->client->pers.netname));
		// Force immediat send of data, because a player join is a priority data
		// disabled for 0.5a version : need more test // WSrv_BypassCheckWorldServer();

	}
}

//-----------------------------------------------------------------------------
static char l_szGameLoopData[4096] = "" ;
//-----------------------------------------------------------------------------
// Add a message to the "GameLoop" message buffer.
//-----------------------------------------------------------------------------
void WSrv_GameLoopAddData(const char *szData)
{
	int iLen = strlen(l_szGameLoopData) ;

	if ( (iLen+strlen(szData)) > sizeof(l_szGameLoopData) )
		RPC_Log("ERROR\tGameLoopData: overflow of %i in %i (%s)", 
			iLen+strlen(szData), sizeof(l_szGameLoopData), szData);
	else
		Com_sprintf ( l_szGameLoopData+iLen, sizeof(l_szGameLoopData)-iLen, "%s", szData ) ;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void WSrv_GameLoop(volatile rpc_data_t *pRpcData)
{
	static char	buffer[4096];
	int			i,iCount;

	// FIXME: hack for correcting the "map doesn't save on worldserver bug"
/*$$
	for ( i=0 ; i<maxclients->value ; i++ )
	{
		if ( !game.clients[i].pers.connected && game.clients[i].pers.netname[0] && game.clients[i].ping )
		{
			edict_t *cl_ent = g_edicts + 1 + i;
			RPC_Log("ERROR_BUG_PERS_CONNECTED\tWSrv_GameLoop\tnetname=%s\tmapname=%s\tclassname=%s\tinuse=%s\n", 
				game.clients[i].pers.netname, level.mapname, cl_ent->classname, cl_ent->inuse ? "true" : "false");
			game.clients[i].pers.connected = true;
		}
	}
$$*/

	iCount = 0;
	for ( i=0 ; i<maxclients->value ; i++ )
	{
		edict_t *cl_ent = g_edicts + 1 + i;
		if ( game.clients[i].pers.connected || cl_ent->inuse )
		{
			WSrv_GameLoopAddData(va("info,name,%s,map,%s,xyz,%d,%d,%d,ping,%d,health,%03d,classname,%s/", 
				game.clients[i].pers.netname, 
				level.mapname,
				(int)cl_ent->s.origin[0], 
				(int)cl_ent->s.origin[1], 
				(int)cl_ent->s.origin[2],
				game.clients[i].ping,
				cl_ent->health,
				cl_ent->classname) );
			iCount++;
		}
	}

	// FIXME: don't report this, it's always the case when entering a map previously visited
	//if ( !iCount )
	//	RPC_Log("ERROR_BUG_NO_CLIENT\tWSrv_GameLoop\tmapname=%s\tlevel.time=%0.03f", 
	//		level.mapname, level.time);

	Com_sprintf ( buffer, sizeof(buffer), "%s?func=GameLoop&id=%s&session=%s&data=%s",
				  wsrv_url->string, 
				  l_pWsrvData->szId, 
				  l_pWsrvData->szSession,
				  l_szGameLoopData ) ;

	pRpcData->iData      = WSRV_GAMELOOP ;
	pRpcData->pszRequest = buffer ;
	pRpcData->iFlags     = 0 ;
	HTTP_SendRpc(pRpcData) ;

	// Reset the command buffer
	l_szGameLoopData[0] = '\0' ;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void WSrv_GameLoop_OK(char *pszResult)
{
	WSrv_CommonReplyTag_OK(pszResult);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern int FS_LoadFile (char *path, void **buffer);
extern void FS_FreeFile (void *buffer);
//----------------------------------------------------------------------------
void WSrv_MapGetXmlRules(volatile rpc_data_t *pRpcData)
{
	static char		buffer[320];
	int				i, iLen, iNbPlayer;
	cvar_t			*mapchecksum = gi.cvar ("mapchecksum", "", 0);

	Com_sprintf ( buffer, sizeof(buffer), 
				  "%s?func=MapGetXmlRules&id=%s&session=%s"
				  "&mapname=%s&mapchecksum=%s&player_list=",
				  wsrv_url->string,
				  l_pWsrvData->szId, 
				  l_pWsrvData->szSession,
				  level.mapname,
				  mapchecksum->string ) ;


	iLen = strlen(buffer) ;
	iNbPlayer = 0 ;
	for ( i=0 ; i<maxclients->value ; i++ )
	{
		edict_t *cl_ent = g_edicts + 1 + i;
		if ( game.clients[i].pers.connected || cl_ent->inuse )
		{
			Com_sprintf ( buffer+iLen, 
						  sizeof(buffer)-iLen, 
						  "%s,", game.clients[i].pers.netname ) ;
			iNbPlayer++ ;
			iLen += strlen(buffer+iLen) ;
		}
	}

	if ( !iNbPlayer )
	{
		cvar_t	*name = gi.cvar ("name", "", 0);
		if ( name )
			Com_sprintf ( buffer+iLen, sizeof(buffer)-iLen, "%s", name->string ) ;
	}
	else
		buffer[iLen-1] = '\0' ;     // Suppress the last ','

	pRpcData->iData      = WSRV_MAPGETXMLRULES ;
	pRpcData->pszRequest = buffer ;
	pRpcData->iFlags     = 0 ;
	HTTP_SendRpc(pRpcData) ;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
qboolean WSrv_MapGetXmlRules_OK(volatile rpc_data_t *pRpcData)
{
	char		szStatus[4];
	qboolean	bResult = false;

	if ( pRpcData->byRpcStatus == RPC_STATUS_OK )
	{
		if (    XML_Parser(pRpcData->pszResult, "status", szStatus, sizeof(szStatus))
			 && strcmp(szStatus, "OK")==0 )
		{
			WSrv_CommonReplyTag_OK(pRpcData->pszResult);
			bResult = true;
		}
	}
	return bResult;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void WSrv_GetInventory(volatile rpc_data_t *pRpcData, char *pszName)
{
	static char	buffer[1024];

	Com_sprintf ( buffer, sizeof(buffer), 
				  "%s?func=PlayerGetInfo&id=%s&session=%s"
				  "&player_info_name=%s&token=get_inventory",
				  wsrv_url->string, 
				  l_pWsrvData->szId, 
				  l_pWsrvData->szSession,
				  pszName ) ;

	if ( strlen(buffer) >= sizeof(buffer)-1 )
		RPC_Log("ERROR_BUFFER_INVENTORY\tWSrv_GetInventory\tbuffer too small (%d)\n", sizeof(buffer));

	pRpcData->iData      = WSRV_PLAYERGETINFO ;
	pRpcData->pszRequest = buffer ;
	pRpcData->iFlags     = 0 ;
	HTTP_SendRpc(pRpcData) ;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void WSrv_GetInventory_OK(char *pszResult)
{
	WSrv_CommonReplyTag_OK(pszResult);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void WSrv_SetInventory(volatile rpc_data_t *pRpcData, char *apszName[], int iNbPlayer)
{
	edict_t		*cl_ent;
	static char	buffer[4096];
	int			iPlayer, i, ii, iLen ;

	Com_sprintf ( buffer, sizeof(buffer), 
				  "%s?func=PlayersSetInfo&id=%s&session=%s",
				  wsrv_url->string, 
				  l_pWsrvData->szId, 
				  l_pWsrvData->szSession ) ;

	iLen = strlen(buffer) ;

	for ( iPlayer=0 ; iPlayer < iNbPlayer ; iPlayer++ )
	{
		// check for a name match
		for ( i=0 ; i<maxclients->value ; i++ )
		{
			cl_ent = g_edicts + 1 + i;
			// FIXME : hack for preventing ERROR_INVENTORY_PLAYER during exitlevel
			//$$if ( !game.clients[i].pers.connected && !cl_ent->inuse )
			//$$	    continue;

			if ( stricmp(game.clients[i].pers.netname, apszName[iPlayer])==0 )
			{
				// FIXME : trace the hack
				//if ( !game.clients[i].pers.connected && !cl_ent->inuse )
				//	RPC_Log("WARNING_INVENTORY_PLAYER\tWSrv_SetInventory\tPlayer=%s\tmapname=%s\tlevel.time=%0.03f\t"
				//		"not pers.connected and not inuse\n", 
				//		apszName[iPlayer], level.mapname, level.time);

				// Save inventory
				Com_sprintf ( buffer+iLen, sizeof(buffer)-iLen,
							  "&player_info_name%d=%s"
							  "&data_map%d=%s$%s"
							  "&data_health%d=%d"
							  "&data_inventory%d=",
							  iPlayer, apszName[iPlayer],
							  iPlayer, level.mapname, game.spawnpoint,
							  iPlayer, g_edicts[1+i].health>0 ? g_edicts[1+i].health : game.clients[i].pers.health,
							  iPlayer ) ;
				iLen += strlen(buffer+iLen) ;

				for ( ii=0 ; ii<game.num_items ; ii++ )
				{
					if ( game.clients[i].pers.inventory[ii] )
					{
						Com_sprintf ( buffer+iLen, sizeof(buffer)-iLen,
									  "%d:%d,", ii, game.clients[i].pers.inventory[ii] ) ;
						
						iLen += strlen(buffer+iLen) ;
					}
				}
				if ( buffer[iLen-1]==',' )
					buffer[--iLen] = '\0' ; // Suppress the last comma

				break;
			}
		}
		if ( i >= maxclients->value )
			RPC_Log("ERROR_INVENTORY_PLAYER\tWSrv_SetInventory\tPlayer %s not found\n", apszName[iPlayer]);
	}

	if ( iLen >= sizeof(buffer)-1 )
		RPC_Log("ERROR_INVENTORY_BUFFER\tWSrv_SetInventory\tbuffer too small (%d)\n", iLen);

	pRpcData->iData      = WSRV_PLAYERSSETINFO ;
	pRpcData->pszRequest = buffer ;
	pRpcData->iFlags     = 0 ;
	HTTP_SendRpc(pRpcData) ;
}

//-----------------------------------------------------------------------------
//    FIFO
//-----------------------------------------------------------------------------
static char	WSrv_FifoCmd[15][40] ;
static int	WSrv_FifoIn  = 0 ;
static int	WSrv_FifoOut = 0 ;

//-----------------------------------------------------------------------------
//    Add a command in the FIFO
//-----------------------------------------------------------------------------
void WSrv_AddCmd(char *pszCmd)
{
	if ( ((WSrv_FifoIn + 1) % SIZE_TAB(WSrv_FifoCmd)) == WSrv_FifoOut )
		return ;

	strncpy(WSrv_FifoCmd[WSrv_FifoIn], pszCmd, sizeof(WSrv_FifoCmd[0]));
	WSrv_FifoCmd[WSrv_FifoIn][sizeof(WSrv_FifoCmd[0])-1] = 0 ;
	WSrv_FifoIn++ ;
	WSrv_FifoIn %= SIZE_TAB(WSrv_FifoCmd) ;
}

//-----------------------------------------------------------------------------
//  Send a command to the WorldServer
//-----------------------------------------------------------------------------
void WSrv_SendCmd(volatile rpc_data_t *pRpcData)
{
	char	*pszFunc ;
	char	*pszName ;

	// Treat a command
	pszFunc = strtok(WSrv_FifoCmd[WSrv_FifoOut], ",");
	WSrv_FifoOut++ ;
	WSrv_FifoOut %= SIZE_TAB(WSrv_FifoCmd) ;

	if ( strcmp(pszFunc,"GetInventory")==0 )
	{
		pszName = strtok(NULL, ",");
		WSrv_GetInventory(pRpcData, pszName) ;
	}
	else if ( strcmp(pszFunc,"SetInventory")==0 )
	{
		int     iNbPlayer = 0 ;
		char    *apszNames[4] ;

		pszName = strtok(NULL, ",");
		apszNames[iNbPlayer++] = pszName ;

		// If multiple SetInventory in the stack : Concatenate them
		while (    WSrv_FifoIn != WSrv_FifoOut
				&& iNbPlayer < SIZE_TAB(apszNames)
				&& strncmp(WSrv_FifoCmd[WSrv_FifoOut], "SetInventory,", strlen("SetInventory,"))==0 )
		{
			pszFunc = strtok(WSrv_FifoCmd[WSrv_FifoOut], ",");
			pszName = strtok(NULL, ",");
			apszNames[iNbPlayer++] = pszName ;

			WSrv_FifoOut++ ;
			WSrv_FifoOut %= SIZE_TAB(WSrv_FifoCmd) ;
		}

		WSrv_SetInventory(pRpcData, apszNames, iNbPlayer) ;
	}
	else if ( strcmp(pszFunc,"Logout")==0 )
	{
		RPC_Logout(pRpcData) ;
	}
}

//-----------------------------------------------------------------------------
//  Send immediately a command to the WorldServer
//-----------------------------------------------------------------------------
void WSrv_SendCmdImmediate(char *pszCmd)
{
	static char RpcData_buffer[1024] ;
	static volatile rpc_data_t  RpcData = { RPC_NONE, 0, NULL, 
		RpcData_buffer, sizeof(RpcData_buffer), RPC_STATUS_READY } ;
	char	*pszFunc ;
	char	*pszName ;

	if ( RpcData.byRpcStatus != RPC_STATUS_IN_PROGRESS )
	{
		pszFunc = strtok(pszCmd, ",");

		if ( strcmp(pszFunc,"SetInventory")==0 )
		{
			pszName = strtok(NULL, ",");
			WSrv_SetInventory(&RpcData, &pszName, 1) ;
		}
	}
	else
		RPC_Log("ERROR_RPC_IN_PROGRESS\timpossible to send\t%s\n", pszCmd);
}

//-----------------------------------------------------------------------------
// Change the delay for the next send to the worldserver to force immediat send.
// It can be used to send immediatly some priority data.
//-----------------------------------------------------------------------------
void WSrv_BypassCheckWorldServer()
{
	// Force immediat send of data
	l_WSrv_time = -WSRV_SEND_DATA_SECONDS ;
}

//-----------------------------------------------------------------------------
static char l_RpcData_buffer[RPC_RESULT_SIZE] ;
static volatile rpc_data_t  l_RpcData = { RPC_NONE, 0, NULL, 
	l_RpcData_buffer, sizeof(l_RpcData_buffer), RPC_STATUS_READY } ;
//-----------------------------------------------------------------------------
// Check the WorldServer according to the current state of the RPC 
// communication : READY / IN_PROGRESS / OK / ERROR
// If bFlush is TRUE, the function will block until the answer of the 
// WorldServer ( OK or ERROR ).
//-----------------------------------------------------------------------------
void WSrv_CheckWorldServer(qboolean bFlush)
{
	char    szErrMsg[80] ;
	char    szStatus[4] ;

	// Force immediat send of data if the level has changed or if bFlush
	if ( bFlush || (l_WSrv_time > level.time) )
		l_WSrv_time = -WSRV_SEND_DATA_SECONDS ;

	do
	{
		if ( l_RpcData.byRpcStatus == RPC_STATUS_READY )
		{
			int   iLen = strlen(l_szGameLoopData) ;

			if (    iLen > (sizeof(l_szGameLoopData) * 0.75)
					  || ((l_WSrv_time + WSRV_SEND_DATA_SECONDS) < level.time) )
			{   // Then 
				l_WSrv_time = level.time ;
				WSrv_GameLoop(&l_RpcData) ;
			}
			else if ( WSrv_FifoIn != WSrv_FifoOut )
			{   // Priority to the WorldServer command stack
				WSrv_SendCmd(&l_RpcData) ;
			}
			else
				bFlush = false ;
		}
		else if ( l_RpcData.byRpcStatus == RPC_STATUS_OK )
		{
			if (    XML_Parser(l_RpcData.pszResult, "status",  szStatus,  sizeof(szStatus))
				 && strcmp(szStatus, "OK")==0 )
			{
				if ( l_RpcData.iData == WSRV_PLAYERGETINFO )
					WSrv_GetInventory_OK(l_RpcData.pszResult) ;
				else if ( l_RpcData.iData == WSRV_GAMELOOP )
					WSrv_GameLoop_OK(l_RpcData.pszResult) ;
				else if ( l_RpcData.iData == RPC_LOGOUT )
				{
					// It's not really a Logout, it can be a server change
				}
			}
			else
				RPC_Error(&l_RpcData, szErrMsg, sizeof(szErrMsg));
		
			l_RpcData.byRpcStatus = RPC_STATUS_READY ;
		}
		else if ( l_RpcData.byRpcStatus == RPC_STATUS_ERROR )
		{
			Com_Printf ("### RPC_Error : no reply\n");
			l_RpcData.byRpcStatus = RPC_STATUS_READY ;
			bFlush = false ;
		}
		else  if ( bFlush && (l_RpcData.byRpcStatus == RPC_STATUS_IN_PROGRESS) )
		{
			// If we are looping, we don't need to charge the CPU too much
			RPC_Sleep(50) ;
		}
	}
	while ( bFlush ) ;
}

#ifndef WSRV_VERSION
#define WSRV_VERSION "v10.04"
#endif

#ifndef WSRV_ID_EXE
#define WSRV_ID_EXE "coopordie_" WSRV_VERSION "_" __DATE__
#endif
//-----------------------------------------------------------------------------
// Send a "Login" request to the WorldServer.
//-----------------------------------------------------------------------------
void RPC_Login(volatile rpc_data_t *pRpcData, char *szName, char *szPass)
{
	static char	buffer[1024] ;
	int			i ;
	cvar_t		*version = gi.cvar ("version", "", 0);


	// Verify if the '/' is present, then add it if not (it's a current user mistake)
	i = strlen(wsrv_url->string) ;
	if ( i && wsrv_url->string[i-1]!='/' )
	{
		char    szBuffer[80] ;

		i = min(i, sizeof(szBuffer)-2) ;
		strncpy(szBuffer, wsrv_url->string, i) ;
		szBuffer[i]   = '/' ;
		szBuffer[i+1] = 0 ;
		gi.cvar_forceset("wsrv_url", szBuffer);
	}

	Com_sprintf ( buffer, sizeof(buffer), "%s?func=PlayerLogin&name=%s&pass=%s&version=%s&wsrv_version=%s&idexe=%s",
				  wsrv_url->string, 
				  szName,
				  szPass,
				  version->string,
				  WSRV_VERSION,
				  WSRV_ID_EXE );

	pRpcData->iData = RPC_LOGIN ;
	pRpcData->pszRequest = buffer ;
	pRpcData->iFlags = 0 ;
	HTTP_SendRpc(pRpcData) ;
}

//-----------------------------------------------------------------------------
// Manage the WorldServer answer to a "Login" request.
//-----------------------------------------------------------------------------
qboolean RPC_Login_OK(volatile rpc_data_t *pRpcData, char *pszErrMsg, int iSizeErrMsg)
{
	qboolean    bResult = false;
	char        szStatus[16];

	if (    XML_Parser(pRpcData->pszResult, "status",  szStatus,  sizeof(szStatus))
		 && strcmp(szStatus, "OK")==0
		 && XML_Parser(pRpcData->pszResult, "id",      l_pWsrvData->szId,      sizeof(l_pWsrvData->szId))
		 && XML_Parser(pRpcData->pszResult, "session", l_pWsrvData->szSession, sizeof(l_pWsrvData->szSession)) )
	{
		WSrv_CommonReplyTag_OK(pRpcData->pszResult);
		bResult = true;
	}
	else
		RPC_Error(pRpcData, pszErrMsg, iSizeErrMsg);

	return bResult;
}

//-----------------------------------------------------------------------------
// Send a "CreateGame" request to the WorldServer.
//-----------------------------------------------------------------------------
void RPC_PlayerCreateGame(volatile rpc_data_t *pRpcData)
{
	static char	buffer[1024];

	if ( port->value < 1 )
		gi.cvar_forceset("port", va("%d",PORT_SERVER));

	Com_sprintf ( buffer, sizeof(buffer), "%s?func=PlayerCreateGame&id=%s&session=%s&port=%s&dedicated=%d",
				  wsrv_url->string, 
				  l_pWsrvData->szId, 
				  l_pWsrvData->szSession,
				  port->string, 
				  dedicated->value);

	pRpcData->iData = RPC_PLAYER_CREATEGAME ;
	pRpcData->pszRequest = buffer ;
	pRpcData->iFlags = 0 ;
	HTTP_SendRpc(pRpcData) ;
}

//-----------------------------------------------------------------------------
// Manage the WorldServer answer to a "CreateGame" request.
//-----------------------------------------------------------------------------
qboolean RPC_PlayerCreateGame_OK(volatile rpc_data_t *pRpcData, char *pszMsg, int iSizeMsg)
{
	qboolean    bResult = false;
	char        szStatus[8];

	if (    XML_Parser(pRpcData->pszResult, "status",  szStatus,  sizeof(szStatus))
		 && strcmp(szStatus, "OK")==0 )
	{
		WSrv_CommonReplyTag_OK(pRpcData->pszResult);
		bResult = true;
	}
	else
		RPC_Error(pRpcData, pszMsg, iSizeMsg);

	return bResult;
}

//-----------------------------------------------------------------------------
// Send a "Logout" request to the WorldServer.
//-----------------------------------------------------------------------------
void RPC_Logout(volatile rpc_data_t *pRpcData)
{
	static char	buffer[1024];

	if ( l_pWsrvData )
	{
		Com_sprintf ( buffer, sizeof(buffer), "%s?func=PlayerLogout&id=%s&session=%s",
					  wsrv_url->string, 
					  l_pWsrvData->szId, 
					  l_pWsrvData->szSession);

		pRpcData->iData = RPC_LOGOUT ;
		pRpcData->pszRequest = buffer ;
		pRpcData->iFlags = 0 ;
		HTTP_SendRpc(pRpcData) ;
	}
	else
		gi.error("RPC_Logout : worldserver zero data\n");
}

#define CMD_MAP_STARTMAP    "map startmap\n"
//-----------------------------------------------------------------------------
// Console command : Login to a WorldServer
// The "ent" parameter could be NULL if it's a dedicated server
// If we are already connected to the worldserver, this command
// jump to the StartMap
//-----------------------------------------------------------------------------
void	Cmd_WSrv_Login_f (edict_t *ent)
{
	int     i;
	char    szMsg[64] ;
	// if dedicated there is one more argument (sv)
	int     iArg = dedicated->value ? 1 : 0;

	if ( worldserver->value )
	{
		if ( ent )
		{
			// a Login when already connected cause a restart to the STARTMAP
			// If a client type wsrv_Login, he quit the game and recreate his own server
			gi.WriteByte (svc_stufftext);
            gi.WriteString(CMD_MAP_STARTMAP);
			gi.unicast (ent, true);
		}
		else
            gi.AddCommandString (CMD_MAP_STARTMAP);
		return;
	}

	i = gi.argc() ;
	if ( gi.argc() != (3+iArg) ) 
	{
		Com_Printf("Usage: wsrv_login <name> <password>\n");
		return;
	}

	if ( strlen(wsrv_url->string) == 0 )
	{
		Com_Printf("wsrv_url must be defined\n");
		return;
	}

	RPC_Login(&l_RpcData, gi.argv(1+iArg), gi.argv(2+iArg)) ;

	while ( l_RpcData.byRpcStatus == RPC_STATUS_IN_PROGRESS )
	{
		RPC_Sleep(100) ;
		Com_Printf(".");
	}
	Com_Printf("\n");

	if ( l_RpcData.byRpcStatus == RPC_STATUS_OK )
	{
		l_pWsrvData = gi.TagMalloc(sizeof(*l_pWsrvData), TAG_GAME);

		if ( RPC_Login_OK(&l_RpcData, szMsg, sizeof(szMsg)) )
		{
			Com_Printf("Connected to WorldServer\n");
			gi.cvar_set("worldserver", "1");
			gi.cvar_forceset("name", gi.argv(1+iArg));
			gi.cvar_forceset("hostname", gi.argv(1+iArg));

			for ( i=0 ; i<8 ; i++ ) 
				gi.cvar_set(va("adr%d", i), "");

			// Now create the game
			RPC_PlayerCreateGame(&l_RpcData) ;
			while ( l_RpcData.byRpcStatus == RPC_STATUS_IN_PROGRESS )
			{
				RPC_Sleep(100) ;
				Com_Printf(".");
			}
			Com_Printf("\n");

			if ( l_RpcData.byRpcStatus == RPC_STATUS_OK )
			{
				if ( RPC_PlayerCreateGame_OK(&l_RpcData, szMsg, sizeof(szMsg)) )
				{
                    gi.AddCommandString (CMD_MAP_STARTMAP);
				}
			}
		}
		else
			Com_Printf("Unable to Login to World Server : %s\n", szMsg);
	}
	else if ( l_RpcData.byRpcStatus == RPC_STATUS_ERROR )
		Com_Printf("Unable to Login to World Server : No reply\n");

	l_RpcData.byRpcStatus = RPC_STATUS_READY;
}

//-----------------------------------------------------------------------------
// Console command : Logout from the WorldServer
// The "ent" parameter could be NULL if it's a dedicated server
//-----------------------------------------------------------------------------
void	Cmd_WSrv_Logout_f(edict_t *ent)
{
	// if dedicated there is one more argument (sv)
	int iArg = dedicated->value ? 1 : 0; ;

	if ( ent && ent != g_edicts+1 )    // Prevent a Logout from a client
		return;

	if ( gi.argc() != (1+iArg) ) 
	{
		Com_Printf("Usage: wsrv_logout\n");
		return;
	}

	if ( strlen(wsrv_url->string) == 0 )
	{
		Com_Printf("wsrv_url must be defined\n");
		return;
	}

	if ( !worldserver->value )
	{
		Com_Printf("not connected to WorldServer\n");
		return;
	}

	RPC_Logout(&l_RpcData) ;

	while ( l_RpcData.byRpcStatus == RPC_STATUS_IN_PROGRESS )
	{
		RPC_Sleep(100) ;
		Com_Printf(".");
	}
	Com_Printf("\n");

	if ( l_RpcData.byRpcStatus == RPC_STATUS_OK )
	{
		Com_Printf("Disconnected from WorldServer\n");
		gi.cvar_forceset("worldserver",  "0");
		l_pWsrvData = NULL;
		gi.cvar_set("tag_worldserver", "");
	}
	else if ( l_RpcData.byRpcStatus == RPC_STATUS_ERROR )
		Com_Printf("Unable to Logout from World Server : No reply\n");

	l_RpcData.byRpcStatus = RPC_STATUS_READY;

	gi.AddCommandString("disconnect");
}

//-----------------------------------------------------------------------------
// Access to the worldserver internal data
//-----------------------------------------------------------------------------
const int	WSrv_GetMapLock()				{ return l_pWsrvData->iMapLock; }
void		WSrv_SetMapLock(int i_iMapLock)	{ l_pWsrvData->iMapLock = i_iMapLock; }

//-----------------------------------------------------------------------------
// Deserialize WorldServer data from an ascii string (hexadecimal dump)
//-----------------------------------------------------------------------------
void WSrv_DeserializeWsrvData(const char *i_pWsrvDataSerialised)
{
	char szValue[3];
	int  iValue;
	char *pcValue;


	if ( strlen(i_pWsrvDataSerialised) != sizeof(wsrv_data_t)*2 )
		gi.error("worldserver data corrupted (invalid len = %d)\n", strlen(i_pWsrvDataSerialised));
	else
	{
		l_pWsrvData = gi.TagMalloc(sizeof(*l_pWsrvData), TAG_GAME);
		pcValue = (char *)l_pWsrvData;
		szValue[2] = '\0';
		while ( i_pWsrvDataSerialised[0] )
		{
			szValue[0] = i_pWsrvDataSerialised[0];
			szValue[1] = i_pWsrvDataSerialised[1];
			sscanf(szValue, "%X", &iValue);
			*pcValue = (char)iValue;

			pcValue++;
			i_pWsrvDataSerialised += 2;
		}
	}
}

//-----------------------------------------------------------------------------
// Serialize WorldServer data to a cvar in ascii (hexadecimal dump)
//-----------------------------------------------------------------------------
void WSrv_SerializeWsrvData(char *i_szVarName)
{
	char szTmp[(sizeof(wsrv_data_t)*2)+1];
	char *pcValue;
	int  i;

	pcValue = (char *)l_pWsrvData;

	for ( i=0 ; i<sizeof(wsrv_data_t) ; i++ )
	{
		sprintf(&szTmp[i*2], "%02X", (int)*pcValue);
		pcValue++;
	}
	gi.cvar_forceset( i_szVarName, szTmp );
}
