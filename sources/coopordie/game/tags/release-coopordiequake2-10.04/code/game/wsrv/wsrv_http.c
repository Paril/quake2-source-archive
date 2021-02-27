/*
Copyright (C) 2001-2009 Pat AfterMoon (www.aftermoon.net)
Thanks to Niko Defranoux for the initial source code of HTTP request

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

//-----------------------------------------------------------------------------
// wsrv_http.c
// low level function for HTTP request used by the WorldServer
//-----------------------------------------------------------------------------


#include "../g_local.h"
#include "wsrv.h"

#include <windows.h>
#include <string.h>
#include "stdio.h"
#include "conio.h"

#define TCP_SIZE_REQUEST 16384     // Max size of request
#define TCP_SIZE_REPLY   16384     // Max size of buffer reply
#define TCP_TIMEOUT      30000     // Socket timeout in millisecond

#define CR "\r"
#define LF "\n"

char *NET_ErrorString (void);

//-----------------------------------------------------------------------------
// Socket + Connect
//-----------------------------------------------------------------------------
int TCP_Connect(SOCKET Socket, DWORD dwIpAddress, WORD wPort)
{
	struct sockaddr_in	SockAddrIn;	

	SockAddrIn.sin_addr.S_un.S_addr = dwIpAddress;
	SockAddrIn.sin_family = AF_INET;
	SockAddrIn.sin_port = htons(wPort);

	return connect(Socket, (struct sockaddr*) &SockAddrIn, sizeof(SockAddrIn));
}

//-----------------------------------------------------------------------------
// Receive data until connection is closed of MaxSize is reached
//-----------------------------------------------------------------------------
int TCP_Recv(SOCKET Socket, char *pBuffer, int iMaxSize, const clock_t clockStart)
{
	struct      timeval timeout;
	fd_set      readfds, exceptfds ;
	int         iTotalSize, iResult;
	clock_t     clockLeft;

	iTotalSize = 0 ;
	do
	{
		FD_ZERO(&readfds) ;
		FD_SET(Socket, &readfds) ;

		FD_ZERO(&exceptfds) ;
		FD_SET(Socket, &exceptfds) ;

		clockLeft = ((clock()-clockStart)*1000) / CLOCKS_PER_SEC;
		if ( clockLeft > TCP_TIMEOUT )
		{
			RPC_Log("ERROR\tTCP_Recv\ttimeout before select\n");
			return SOCKET_ERROR;
		}
		else
			clockLeft = TCP_TIMEOUT - clockLeft;

		timeout.tv_sec  = clockLeft / 1000;
		timeout.tv_usec = (clockLeft % 1000) * 1000;

		iResult = select( Socket+1, &readfds, NULL, &exceptfds, &timeout);
		if (iResult == SOCKET_ERROR)
		{
			RPC_Log("ERROR\tTCP_Recv\tWSAGetLastError\t%s\n", NET_ErrorString());
			return SOCKET_ERROR;
		}
		else if ( !iResult )
		{
			RPC_Log("ERROR\tTCP_Recv\tselect timeout\n");
			return SOCKET_ERROR;
		}

		iResult = recv(Socket, (char*)pBuffer+iTotalSize, iMaxSize-iTotalSize, 0) ;
		if (iResult == SOCKET_ERROR)
		{
			RPC_Log("ERROR\tTCP_Recv\tWSAGetLastError\t%s\n", NET_ErrorString());
			return SOCKET_ERROR;
		}

		iTotalSize += iResult;

		if ( iTotalSize>= iMaxSize )
		{
			RPC_Log("ERROR\tTCP_Recv\ttoo much data received\tlimit is %d\n", iMaxSize);
			return -1;
		}
	}
	while ( iResult );

	pBuffer[iTotalSize] = '\0';

	return iTotalSize ;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
DWORD DNS_GetHostAddr(const char *pszHostName)
{
	struct hostent *l_pHost;

	if ( *pszHostName>='0' && *pszHostName<='9' )
		return inet_addr(pszHostName);

	l_pHost = gethostbyname(pszHostName);
	
	if (l_pHost == NULL)
		return 0 ;
	else
		return *(DWORD*) l_pHost->h_addr_list[0];
}


//-----------------------------------------------------------------------------
// ex : HTTP_SplitUrl( pszHost, pszAdr, "http://rpc.aftermoon.net/?Func=PlayerLogin&Player=Pat" )
//      pszHost = "rpc.aftermoon.net"
//      pszAdr  = "/?Func=PlayerLogin&Player=Pat"
//-----------------------------------------------------------------------------
void HTTP_SplitUrl( char **pszHost, char **pszAdr, char *pszUrl )
{
	char *pszSlash ;


	if (strncmp(pszUrl,"http://",7)==0)
		*pszHost = pszUrl + 7 ;
	else
		*pszHost = pszUrl ;

	pszSlash = strchr(*pszHost,'/');
	if ( pszSlash )
	{
		*pszSlash = '\0';
		*pszAdr   = pszSlash + 1;	
	}
	else
		*pszAdr   = *pszHost + strlen(*pszHost) ;	
}

//-----------------------------------------------------------------------------
// Get a page throught HTTP protocol
//-----------------------------------------------------------------------------
// IN:      pszUrl       = URL of the page
//          pszData      = Data to send
//          dwPageSize   = size of the pszPage buffer
// OUT:     pszPage      = Buffer for the page result
// RETURN:  TRUE if no error
//-----------------------------------------------------------------------------
BOOL HTTP_GetPage(char *pszHost, char *pszPath, char *pszData, 
				  char *pszPage, DWORD dwPageSize, const clock_t clockStart)
{
	SOCKET  Socket ;
	DWORD   l_dwSvrIpAddress ;
	static char    szSendStr[8192];
	static char    szLog[4096];
	BOOL    bError = FALSE ;

	l_dwSvrIpAddress = DNS_GetHostAddr( pszHost );
	if ( l_dwSvrIpAddress==0 )
	{
		RPC_Log ("ERROR\tHTTP_GetPage\thost not found\t%s\n", pszHost);
		return FALSE ;
	}
	else if ( (((clock()-clockStart)*1000)/CLOCKS_PER_SEC) >= TCP_TIMEOUT )
	{
		RPC_Log ("ERROR\tHTTP_GetPage\ttimeout\n");
		return FALSE ;
	}

	Socket = socket(AF_INET, SOCK_STREAM, 0);
	if ( Socket == INVALID_SOCKET )
	{
		RPC_Log ( "ERROR\tHTTP_GetPage\tunable to create socket\n");
		return FALSE ;
	}

	// Connect socket on port 80 (HTTP protocol)
	if ( TCP_Connect(Socket, l_dwSvrIpAddress, 80) != 0 )
	{
		RPC_Log ( "ERROR\tHTTP_GetPage\tunable to connect socket\t%d.%d.%d.%d\t%s\n", 
				  (l_dwSvrIpAddress & 0XFF000000)>>24,
				  (l_dwSvrIpAddress & 0X00FF0000)>>16,
				  (l_dwSvrIpAddress & 0X0000FF00)>>8,
				  (l_dwSvrIpAddress & 0X000000FF),
				  NET_ErrorString() );
		return FALSE ;
	}

	RPC_FlushLog(szLog, sizeof(szLog)) ;

	// Format the request for the page
	Com_sprintf( szSendStr, 
				 sizeof(szSendStr), 
				 "POST /%s HTTP/1.0" CR LF
				 "Content-Type: application/x-www-form-urlencoded" CR LF
				 "Host: %s" CR LF,
				 pszPath,
				 pszHost ) ;

	// Add the data
	if ( *pszData )
	{
		int iLen = strlen(szSendStr) ;
		Com_sprintf( szSendStr+iLen, 
					 sizeof(szSendStr)-iLen,
					 "Content-Length: %d" CR LF CR LF
					 "%s&log=%s",
					 strlen(pszData) + strlen(szLog) + 5,
					 pszData,
					 szLog ) ;
	}

	if ( strlen(szSendStr) >= sizeof(szSendStr)-1 )
	{
		RPC_Log ("ERROR\tHTTP_GetPage\tsend buffer too small\t%d\n", sizeof(szSendStr));
		return FALSE ;
	}

	if ( wsrv_debug->value )
		RPC_LogFile(szSendStr);

	// Send the request via HTTP
	send(Socket, szSendStr, strlen(szSendStr), 0);

	bError = TCP_Recv(Socket, pszPage, dwPageSize, clockStart) < 1 ;
	closesocket(Socket);

	if ( wsrv_debug->value )
		RPC_LogFile(pszPage);

	if ( bError )
	{
		RPC_Log( "ERROR\tHTTP_GetPage\t%s\t%d ms\n", 
				 pszData, ((clock()-clockStart)*1000)/CLOCKS_PER_SEC) ;
	}
	else
	{
		char	*pszStartData = strstr(pszPage, "<rpc>");

		if ( !pszStartData )
		{
			bError = true;
			RPC_Log("ERROR\tHTTP_GetPage\ttag <rpc></rpc> not found\n");
		}
		else
		{
			clock_t clockDelay;
			
			memmove(pszPage, pszStartData, strlen(pszStartData)+1);

			// if the request duration is more than 5000 ms, log it
			clockDelay = ((clock()-clockStart)*1000) / CLOCKS_PER_SEC;
			if ( clockDelay > 5000 )
			{
				strtok(pszData, "&") ;      // Truncate after the name of the remote function
				RPC_Log( "WARNING\tHTTP_GetPage\t%s\t%d ms\n", pszData, clockDelay) ;
			}
		}
	}

	return !bError ;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static DWORD  HTTP_ThreadSendRpc(void *param)
{
	volatile rpc_data_t *pRpcData = (rpc_data_t*)param;
	BOOL                bLocalMalloc = FALSE ;
	BOOL                bResult ;
	char                *pszUrl, *pszData;
	char                *pszHost, *pszPath;
	clock_t             clockStart;

	clockStart = clock();

	pszUrl  = strtok(pRpcData->pszRequest, "?");
	pszData = strtok(NULL, "?");
	HTTP_SplitUrl( &pszHost, &pszPath, pszUrl );

	bLocalMalloc = (pRpcData->pszResult == NULL) ;
	if ( bLocalMalloc )
	{
		pRpcData->iResultSize = TCP_SIZE_REPLY ;
		pRpcData->pszResult = gi.TagMalloc(pRpcData->iResultSize, 0);
	}

	bResult = HTTP_GetPage( pszHost, pszPath, pszData,
							pRpcData->pszResult,  pRpcData->iResultSize, clockStart);

	if ( !bResult )
	{
		RPC_Log("WARNING\tHTTP_ThreadSendRpc\tRetry HTTP_GetPage\t%.16s\n", pRpcData->pszRequest);
		bResult = HTTP_GetPage( pszHost, pszPath, pszData,
								pRpcData->pszResult,  pRpcData->iResultSize, clockStart);
	}

	if ( bResult )
		pRpcData->byRpcStatus = RPC_STATUS_OK;
	else
		pRpcData->byRpcStatus = RPC_STATUS_ERROR ;

	if ( bLocalMalloc )
	{
		gi.TagFree(pRpcData->pszResult);
		pRpcData->pszResult = NULL ;
	}
	
	return 0;
}

//-----------------------------------------------------------------------------
// Create an independant thread that send an HTTP request
// During process, the state could be know by reading the pRpcData->byRpcStatus
// INPUT :
//  pRpcData->pszRequest    = complete request to send with url and parameters
//                          (ex : "www.aftermoon.net/rpc/?func=Login&name=toto&pass=titi")
// OUTPUT :
//  nothing
//
// WHEN TERMINATED :
//      pRpcData->byRpcStatus   = RPC_STATUS_OK
//      pRpcData->pszResult     = is allocated and contain the reply of the server
//  OR
//      pRpcData->byRpcStatus   = RPC_STATUS_ERROR
//-----------------------------------------------------------------------------
void HTTP_SendRpc(volatile rpc_data_t *pRpcData )
{
	HANDLE hThread ;
	DWORD   id;

	pRpcData->byRpcStatus = RPC_STATUS_IN_PROGRESS ;
	hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HTTP_ThreadSendRpc, (void*)pRpcData, 0, &id);
}

//-----------------------------------------------------------------------------
// Sleep a number of Milliseconds
// (Defined here for helping platform portage)
//-----------------------------------------------------------------------------
void RPC_Sleep(DWORD dwMilliseconds)
{
	Sleep(dwMilliseconds) ;
}

//====================
// NET_ErrorString
//====================
char *NET_ErrorString (void)
{
	int		code;

	code = WSAGetLastError ();
	switch (code)
	{
	case WSAEINTR: return "WSAEINTR";
	case WSAEBADF: return "WSAEBADF";
	case WSAEACCES: return "WSAEACCES";
	case WSAEDISCON: return "WSAEDISCON";
	case WSAEFAULT: return "WSAEFAULT";
	case WSAEINVAL: return "WSAEINVAL";
	case WSAEMFILE: return "WSAEMFILE";
	case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
	case WSAEINPROGRESS: return "WSAEINPROGRESS";
	case WSAEALREADY: return "WSAEALREADY";
	case WSAENOTSOCK: return "WSAENOTSOCK";
	case WSAEDESTADDRREQ: return "WSAEDESTADDRREQ";
	case WSAEMSGSIZE: return "WSAEMSGSIZE";
	case WSAEPROTOTYPE: return "WSAEPROTOTYPE";
	case WSAENOPROTOOPT: return "WSAENOPROTOOPT";
	case WSAEPROTONOSUPPORT: return "WSAEPROTONOSUPPORT";
	case WSAESOCKTNOSUPPORT: return "WSAESOCKTNOSUPPORT";
	case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
	case WSAEPFNOSUPPORT: return "WSAEPFNOSUPPORT";
	case WSAEAFNOSUPPORT: return "WSAEAFNOSUPPORT";
	case WSAEADDRINUSE: return "WSAEADDRINUSE";
	case WSAEADDRNOTAVAIL: return "WSAEADDRNOTAVAIL";
	case WSAENETDOWN: return "WSAENETDOWN";
	case WSAENETUNREACH: return "WSAENETUNREACH";
	case WSAENETRESET: return "WSAENETRESET";
	case WSAECONNABORTED: return "WSWSAECONNABORTEDAEINTR";
	case WSAECONNRESET: return "WSAECONNRESET";
	case WSAENOBUFS: return "WSAENOBUFS";
	case WSAEISCONN: return "WSAEISCONN";
	case WSAENOTCONN: return "WSAENOTCONN";
	case WSAESHUTDOWN: return "WSAESHUTDOWN";
	case WSAETOOMANYREFS: return "WSAETOOMANYREFS";
	case WSAETIMEDOUT: return "WSAETIMEDOUT";
	case WSAECONNREFUSED: return "WSAECONNREFUSED";
	case WSAELOOP: return "WSAELOOP";
	case WSAENAMETOOLONG: return "WSAENAMETOOLONG";
	case WSAEHOSTDOWN: return "WSAEHOSTDOWN";
	case WSASYSNOTREADY: return "WSASYSNOTREADY";
	case WSAVERNOTSUPPORTED: return "WSAVERNOTSUPPORTED";
	case WSANOTINITIALISED: return "WSANOTINITIALISED";
	case WSAHOST_NOT_FOUND: return "WSAHOST_NOT_FOUND";
	case WSATRY_AGAIN: return "WSATRY_AGAIN";
	case WSANO_RECOVERY: return "WSANO_RECOVERY";
	case WSANO_DATA: return "WSANO_DATA";
	default: return "NO ERROR";
	}
}
