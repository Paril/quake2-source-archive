
#include "g_local.h"

#ifdef _WIN32
#ifdef GEN_SOCK

#include <winsock.h>
//#endif

#include "g_gensock.h"
#include "g_genmidi.h"


//Message codes
//SERVER OUTBOUND MESSAGE IDS
#define MSG_PLAY		'a'		
#define	MSG_PAUSE		'b'
#define MSG_RESUME		'c'
#define MSG_STOP		'd'
#define MSG_ERROR		'j'
#define MSG_DPLAYINTERMISSION 'i'

//incoming and outgoing buffers
#define BUF_INSIZE		2
#define BUF_OUTSIZE		10

//Client states
#define CLISTEN		0
#define CPLAY		1
#define CPAUSE		2
#define CSTOP		3
#define CWLISTEN	4
#define CWPLAY		5
#define CWPAUSE		6
#define CWSTOP		7
#define CERROR		8



//SOCKADDR_IN saLocal;			/* local socket name (address & port) */
char bufout[BUF_OUTSIZE];
char bufin[BUF_INSIZE];

TIMEVAL		stime;

fd_set		readfds;			//Socket Descriptor sets for Select calls
fd_set		writefds;
			//exceptfds;


/*
======================================
Util Function to get clients ip
======================================
*/

qboolean ParseIP (char *ip, char *newip)
{
	char	*s;
	s= strchr(ip,':');
	
	if(s)
	{
		int len=0;
		len = s - ip;
/*		if(len >= 7 && len <= 16)		//IP addy qualifications
		{
			return true;
		}
*/
		memset(newip,'\0',16);
		memcpy(newip,ip,len);
		//gi.dprintf("parsed IP for %s is %s\n",ip,newip); 
		return true;
	}

	if(strcmp(ip,"localhost")==0)
	{
		memset(newip,'\0',16);
		strcpy(newip,"127.0.0.1");
		return true;
	}
	return false;
}


/*
======================================
Util function - prints out Winsock errors
======================================
*/

static void PrintSockError()
{
	char error[128];
	strcpy(error,"Socket Error :");

	switch(WSAGetLastError())
	{
	case 10013:strcat(error,"WSAEACCES - error in accessing socket"); break;
	case 10048:strcat(error,"WSAEADDRINUSE - address is in use"); break;
	case 10049:strcat(error,"WSAEADDRNOTAVAIL - address is not valid in context"); break;
	case 10047:strcat(error,"WSAEAFNOSUPPORT - address family not supported by protocol"); break;
	case 10037:strcat(error,"WSAEALREADY - operation already in progress"); break;
	case 10053:strcat(error,"WSACONNABORTED - software caused connection aborted"); break;
	case 10061:strcat(error,"WSAECONNREFUSED - connection refused"); break;
	case 10054:strcat(error,"WSAECONNRESET - connection reset by peer"); break;
	case 10039:strcat(error,"WSAEDESTADDRREQ - destination address required"); break;
	case 10014:strcat(error,"WSAEFAULT - bad address"); break;
	case 10064:strcat(error,"WSAEHOSTDOWN - host is down"); break;
	case 10065:strcat(error,"WSAEHOSTUNREACH - no route to host"); break;
	case 10036:strcat(error,"WSAEINPROGRESS - operation now in progress"); break;
	case 10004:strcat(error,"WSAEINTR - interrupted function call"); break;
	case 10022:strcat(error,"WSAEINVAL - invalid argument"); break;
	case 10056:strcat(error,"WSAEISCONN - socket is already connected"); break;
	case 10024:strcat(error,"WSAEMFILE - too many open files"); break;
	case 10040:strcat(error,"WSAEMSGSIZE - message to long"); break;
	case 10050:strcat(error,"WSAENETDOWN - network is down"); break;
	case 10052:strcat(error,"WSAENETRESET - network dropped connection on reset"); break;
	case 10051:strcat(error,"WSAENETUNREACH - network is unreachable"); break;
	case 10055:strcat(error,"WSAENOBUFS - no buffer space available"); break;
	case 10042:strcat(error,"WSAENOPROTOOPT - bad protocol option"); break;
	case 10057:strcat(error,"WSAENOTCONN - socket is not connected"); break;
	case 10038:strcat(error,"WSAENOTSOCK - socket operation on non-socket"); break;
	case 10045:strcat(error,"WSAEOPNOTSUPP - operation not supported"); break;
	case 10046:strcat(error,"WSAEPFNOSUPPORT - protocol family not supported"); break;
	case 10067:strcat(error,"WSAEPROCLIM - too many processes"); break;
	case 10043:strcat(error,"WSAEPROTONOSUPPORT - protocol not supported"); break;
	case 10041:strcat(error,"WSAEPROTOTYPE - protocol wrong type for socket"); break;
	case 10058:strcat(error,"WSAESHUTDOWN - cannot send after socket shutdown"); break;
	case 10044:strcat(error,"WSAESOCKTNOSUPPORT - socket type not supported"); break;
	case 10060:strcat(error,"WSAETIMEDOUT - connection timed out"); break;
	case 10035:strcat(error,"WSAEWOULDBLOCK - resource temporarily unavailable"); break;
	case 11001:strcat(error,"WSAHOST_NOT_FOUND - host not found"); break;
	case 10093:strcat(error,"WSANOTINITIALISED - WSAStartup not yet performed"); break;
	case 11004:strcat(error,"WSANO_DATA - valid name, no data record of requested type"); break;
	case 11003:strcat(error,"WSANO_RECOVERY - non-recoverable error"); break;
	case 10091:strcat(error,"WSASYSNOTREADY - network subsystem is unavailable"); break;
	case 11002:strcat(error,"WSATRY_AGAIN - non-authoritative host not found"); break;
	case 10092:strcat(error,"WSAVERNOTSUPPORTED - winsock.dll verison out of range"); break;
	case 10094:strcat(error,"WSAEDISCON - graceful shutdown in progress"); break;
	}
	gi.dprintf("%s\n",error);
}

static void PrintErrorNum(int err)
{
	char error[128];
	strcpy(error,"Socket Error :");

	switch(err)
	{
	case 10013:strcat(error,"WSAEACCES - error in accessing socket"); break;
	case 10048:strcat(error,"WSAEADDRINUSE - address is in use"); break;
	case 10049:strcat(error,"WSAEADDRNOTAVAIL - address is not valid in context"); break;
	case 10047:strcat(error,"WSAEAFNOSUPPORT - address family not supported by protocol"); break;
	case 10037:strcat(error,"WSAEALREADY - operation already in progress"); break;
	case 10053:strcat(error,"WSACONNABORTED - software caused connection aborted"); break;
	case 10061:strcat(error,"WSAECONNREFUSED - connection refused"); break;
	case 10054:strcat(error,"WSAECONNRESET - connection reset by peer"); break;
	case 10039:strcat(error,"WSAEDESTADDRREQ - destination address required"); break;
	case 10014:strcat(error,"WSAEFAULT - bad address"); break;
	case 10064:strcat(error,"WSAEHOSTDOWN - host is down"); break;
	case 10065:strcat(error,"WSAEHOSTUNREACH - no route to host"); break;
	case 10036:strcat(error,"WSAEINPROGRESS - operation now in progress"); break;
	case 10004:strcat(error,"WSAEINTR - interrupted function call"); break;
	case 10022:strcat(error,"WSAEINVAL - invalid argument"); break;
	case 10056:strcat(error,"WSAEISCONN - socket is already connected"); break;
	case 10024:strcat(error,"WSAEMFILE - too many open files"); break;
	case 10040:strcat(error,"WSAEMSGSIZE - message to long"); break;
	case 10050:strcat(error,"WSAENETDOWN - network is down"); break;
	case 10052:strcat(error,"WSAENETRESET - network dropped connection on reset"); break;
	case 10051:strcat(error,"WSAENETUNREACH - network is unreachable"); break;
	case 10055:strcat(error,"WSAENOBUFS - no buffer space available"); break;
	case 10042:strcat(error,"WSAENOPROTOOPT - bad protocol option"); break;
	case 10057:strcat(error,"WSAENOTCONN - socket is not connected"); break;
	case 10038:strcat(error,"WSAENOTSOCK - socket operation on non-socket"); break;
	case 10045:strcat(error,"WSAEOPNOTSUPP - operation not supported"); break;
	case 10046:strcat(error,"WSAEPFNOSUPPORT - protocol family not supported"); break;
	case 10067:strcat(error,"WSAEPROCLIM - too many processes"); break;
	case 10043:strcat(error,"WSAEPROTONOSUPPORT - protocol not supported"); break;
	case 10041:strcat(error,"WSAEPROTOTYPE - protocol wrong type for socket"); break;
	case 10058:strcat(error,"WSAESHUTDOWN - cannot send after socket shutdown"); break;
	case 10044:strcat(error,"WSAESOCKTNOSUPPORT - socket type not supported"); break;
	case 10060:strcat(error,"WSAETIMEDOUT - connection timed out"); break;
	case 10035:strcat(error,"WSAEWOULDBLOCK - resource temporarily unavailable"); break;
	case 11001:strcat(error,"WSAHOST_NOT_FOUND - host not found"); break;
	case 10093:strcat(error,"WSANOTINITIALISED - WSAStartup not yet performed"); break;
	case 11004:strcat(error,"WSANO_DATA - valid name, no data record of requested type"); break;
	case 11003:strcat(error,"WSANO_RECOVERY - non-recoverable error"); break;
	case 10091:strcat(error,"WSASYSNOTREADY - network subsystem is unavailable"); break;
	case 11002:strcat(error,"WSATRY_AGAIN - non-authoritative host not found"); break;
	case 10092:strcat(error,"WSAVERNOTSUPPORTED - winsock.dll verison out of range"); break;
	case 10094:strcat(error,"WSAEDISCON - graceful shutdown in progress"); break;
	}
	gi.dprintf("%s\n",error);
}


/*
======================================
Init winsock, socket descriptors etc
======================================
*/

qboolean GenInitSockets(void)
{
	WORD wVersionRequested;
	WSADATA wsaData;
#if 0
	char   myname[256];
	struct hostent *hp;
#endif
	int err;
	
	//Init Winsock
	wVersionRequested = MAKEWORD( 1, 1 ); 
	err = WSAStartup( wVersionRequested, &wsaData );
	
	if ( err != 0 ) 
	{
		/* Tell the user that we couldn't find a usable 
		   WinSock DLL.                                 */    
		gi.dprintf("GenInitSocks::Could not find a usable Winsock dll\n");
		return false;
	} 
	
		/* Confirm that the WinSock DLL supports 2.0.		 */
		/* Note that if the DLL supports versions greater    */
		/* than 2.0 in addition to 2.0, it will still return */
		/* 2.0 in wVersion since that is the version we      */
		/* requested.                                        */ 

	if ( LOBYTE( wsaData.wVersion ) != 1 ||
         HIBYTE( wsaData.wVersion ) != 1) 
	{
		/* Tell the user that we couldn't find a usable 
		   WinSock DLL.                                 */    
		WSACleanup();
		gi.dprintf("GenInitSocks::Could not find Winsock 2.0\n");
		return false;
	}  
	// The WinSock DLL is acceptable. Proceed. 

	stime.tv_sec = 0;
	stime.tv_usec = 0;

	//Set all Socket Descriptor sets to 0
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	//FD_ZERO(&exceptfds);

#if 0
	//Init Local Address Info
	memset(&saLocal, 0, sizeof(struct sockaddr_in));	
	gethostname(myname, sizeof(myname));				
	hp = gethostbyname(myname);							
  
	if (hp == NULL)										
	{
		gi.dprintf("GenInitSocks::Bad Host Address\n");
		return false;
	}

	saLocal.sin_family = hp->h_addrtype; 
	saLocal.sin_port = htons(LISTENPORT);				

	//Init Socket
	slisten = socket(AF_INET, SOCK_DGRAM, 0);		
	if (slisten == INVALID_SOCKET)
	{
		gi.dprintf("Invalid Listener Socket Created\n");
		return false;
	}

	//Setup the Listener Socket
	if(!setsockopt(slisten,SOL_SOCKET,SO_REUSEADDR,"1",sizeof(int)))
		gi.dprintf("GenInitSocks::Socket Options set\n");
	else
		gi.dprintf("GenInitSocks::Error setting Socket Options:%d\n",WSAGetLastError());

	// bind the socket to the internet address 
	if (bind(slisten, (struct sockaddr *)&saLclName, 
					   sizeof(struct sockaddr_in)) == SOCKET_ERROR) 
	{
		closesocket(slisten);
		gi.dprintf("GenInitSocks::Could not bind socket\n");
		return false;
	}
	gi.dprintf("GenInitSocks::socket bound and ready\n");


	sout = socket(AF_INET, SOCK_DGRAM, 0);				
	if (sout == INVALID_SOCKET)
	{
		gi.dprintf("Invalid Outgoing Socket Created\n");
		return false;
	}
#endif
	return true;
}

/*
======================================
called from the Game Init function
======================================
*/

qboolean GenInitSocks(void)
{
	if(!dedicated->value)
		return false;
	
	if(GenInitSockets())
		return true;
	return false;
}


/*
======================================
Go through all the active clients
and reset all socket info
called from Shutdown()
======================================
*/

void GenShutdownSocks(void)
{
}



/*
======================================
Get data waiting on a client socket
======================================
*/

qboolean GenReadMsg(gclient_t *client, char *out)
{
	int rc;

	if(!client->sock.sconnected)
	{
		gi.dprintf("GenReadMsg:Error receiving - %s is not connected\n",client->pers.netname);
		return false;
	}


	memset(out,0,sizeof(out));
	rc = recv(client->sock.sock,out,sizeof(out),0);

	if (rc == SOCKET_ERROR) 
	{
		printf("GenReadMsg: %s:recv failed\n",client->pers.netname);
		PrintSockError();
		return false;
	}
	
	if (rc ==0) 
	{
		printf("GenReadMsg: %s:Connection closed by client\n",client->pers.netname);
		client->sock.sconnected = false;

		//Reset Socket
		closesocket(client->sock.sock);
		client->sock.sock = socket(AF_INET, SOCK_STREAM, 0);
		if(client->sock.sock !=INVALID_SOCKET)
		{
			client->sock.sactive = true;
		}
		return false;
	}
//	printf("Received [%s] from %s\n",out,client->pers.netname);
	return true;
}



/*
======================================
Send something to the client
======================================
*/

qboolean GenSendMsg(gclient_t *client, char msgid, char *midi)
{
	int nSent;

	if(!client->sock.sconnected)
	{
		gi.dprintf("GenSendMsg:Error - %s is not connected\n",client->pers.netname);
		return false;
	}
	
	memset(bufout, 0,sizeof(bufout));					//clear buffer

	//make buffer
	bufout[0] = msgid;
	if(midi != 0)
	{
		memcpy(bufout+1,midi,8);
		bufout[9] ='\0';
	}
	else
		bufout[1] ='\0';
 
	//send
	nSent = send(client->sock.sock, (char FAR *)bufout, sizeof(bufout),0);

	if(nSent == SOCKET_ERROR)
	{
		gi.dprintf("GenSendMsg:Error sending midi info to %s\n",client->pers.netname);
		PrintSockError();
		return false;
	}

	if(nSent == sizeof(bufout))
	{
//		gi.dprintf("Sent:%s to %s\n",bufout,client->sock.ipaddr);
#if 0		
		switch(msgid)
		{
			case MSG_PLAY:
				client->sock.midstate = CWPLAY;
				break;
			case MSG_PAUSE:
				client->sock.midstate = CWPAUSE;
				break;
			case MSG_RESUME:
				client->sock.midstate = CWPLAY;
				break;
			case MSG_STOP:
				client->sock.midstate = CWSTOP;
				break;
/*			default:
			case MSG_CERROR:
				client->sock.midstate = CERROR;
				break;
*/
		}
#endif
	}
	return true;
}



/*
======================================
called from the main game loop
======================================
*/


//Called in the Game loop
void GenChkSockState(void)
{
	int i,j;
	gclient_t * cl;
	
	i = select(0,&readfds,&writefds,NULL,&stime);
	
	if(i==SOCKET_ERROR)
	{
//		gi.dprintf("Select failed\n");
//		PrintSockError();
//		return;
	}
	if(i==0)
	{
		//printf("Select returned no fds ready\n");
		//continue;
	}

	for (j=0 ; j<maxclients->value ; j++)
	{
			cl = game.clients + j;
			if (!g_edicts[j+1].inuse)
				continue;

			if(cl->sock.sactive)
			{
				if(FD_ISSET(cl->sock.sock, &writefds))
				{
					printf("connected to %s's midi client\n",cl->pers.netname);
					cl->sock.sconnected = true;
					FD_SET(cl->sock.sock,&readfds);
					//FD_CLR(cl->sock.sock,&readfds);
					FD_CLR(cl->sock.sock,&writefds);
					//continue;
				}
				
				if(cl->sock.sconnected)
				{
					if(FD_ISSET(cl->sock.sock,&readfds))
					{
						FD_CLR(cl->sock.sock,&readfds);
						FD_CLR(cl->sock.sock,&writefds);
						//continue;
					}
					FD_SET(cl->sock.sock,&readfds);
				}
				//FD_SET(cl->pers.sock,&writefds);
			}
	}
}




/*
======================================
Try to connect to client
called every level load, if client is not connected
======================================
*/

qboolean GenConnect(gclient_t *client)
{
	if(!client->sock.sactive)
	{
		gi.dprintf("%s:tried to connect with invalid socket\n",client->pers.netname);
		return false;
	}

	if(connect(client->sock.sock,(LPSOCKADDR)&client->sock.saddr,sizeof(SOCKADDR)) != SOCKET_ERROR)
	{
		printf("connected to %s's midi client\n",client->pers.netname);
		client->sock.sconnected = true;
		FD_CLR(client->sock.sock,&writefds);
		FD_SET(client->sock.sock,&readfds);
		return true;
	}
	else
	{
		int i = WSAGetLastError();
		if(i==WSAEISCONN)
		{
			client->sock.sconnected = true;
			return true;
		}
		if(i == WSAEWOULDBLOCK)
		{
			FD_SET(client->sock.sock,&writefds);
			//PrintSockError();
			return true;
		}
		printf("Failed to connect to %s\n",client->pers.netname);
		PrintErrorNum(i);
		return false;
	}
}



/*
======================================
Set up the socket info in the client struct
only called once, when the client connects
======================================
*/

qboolean GenInitClientSock(gclient_t *client)
{
	unsigned long ioctl_opt =1;

	if(!client)
		return false;

	memset(&client->sock.saddr,0,sizeof(SOCKADDR_IN));
	
	client->sock.saddr.sin_family = AF_INET;              
	client->sock.saddr.sin_port = htons((short)client->sock.port); 
	client->sock.saddr.sin_addr.s_addr = inet_addr(client->sock.ipaddr); 

	if(client->sock.sactive==true)
	{
		gi.dprintf("socket is already open, closing, reopening\n");
		closesocket(client->sock.sock);
	}
	client->sock.sock = socket(AF_INET,SOCK_STREAM,0);

	if (client->sock.sock == INVALID_SOCKET)
	{
		gi.dprintf("%s: Couldnt create socket\n",client->pers.netname);
		return false;
	}

	//Setup the Listener Socket
	if(!strcmp(client->sock.ipaddr,"127.0.0.1"))
	{
		if(setsockopt(client->sock.sock,SOL_SOCKET,SO_REUSEADDR,"1",sizeof(int)))
//			gi.dprintf("%s:Socket Options set\n",client->sock.netname);
//		else
		{
			gi.dprintf("%s:Error setting Socket Options:%d\n",client->pers.netname,WSAGetLastError());
			closesocket(client->sock.sock);
			return false;
		}
	}
	
	//set to nonblocking mode
	if (ioctlsocket(client->sock.sock,FIONBIO,&ioctl_opt) == SOCKET_ERROR)
	{
		closesocket(client->sock.sock);
		printf("%s:ioctlsocket failed %d\n",client->pers.netname,WSAGetLastError());
		return false;
	}
	
	client->sock.sactive = true;
	return true;
}

/*
======================================
close the socket in the client struct
======================================
*/

qboolean GenCloseClientSocket(gclient_t *client)
{
//	shutdown(client->sock.sock,SD_SEND);
	FD_CLR(client->sock.sock,&readfds);
	FD_CLR(client->sock.sock,&writefds);

	if(client->sock.sock != INVALID_SOCKET)
		closesocket(client->sock.sock);

	memset(&client->sock.saddr,0,sizeof(SOCKADDR_IN));
	client->sock.sactive = false;
	client->sock.sconnected = false;
	memset(client->sock.ipaddr,0,sizeof(client->sock.ipaddr));
}

#endif
#endif



#if 0
	int error;
	LPWSANETWORKEVENTS lpNetworkEvents;
	int i;
	gclient_t *cl;
	
	if(!dedicated->value) 
		return;


	if(sockstate !=SOCK_LISTENING)
		return;
	
	error = WSAEnumNetworkEvents(slisten,gEvent,lpNetworkEvents);
	if(!error)
	{
		//for(lNetworkEvents
		if(lpNetworkEvents->lNetworkEvents & FD_READ)
		{
			int bytes_in;
			int nAddrSize = sizeof(SOCKADDR);

			memset(bufin,0,BUF_INSIZE);
			bytes_in = recvfrom (slisten, (char FAR *)bufin, BUF_INSIZE, 0,
						(struct sockaddr *) &saRmtName, &nAddrSize);

			if(bytes_in)
			{
				char *ip;
				ip = inet_ntoa(saRmtName.sin_addr);

//Process this - update client state
				gi.dprintf("Recvfrom:%s\nBufIn:%s\nBytes:%d\n",
							ip,bufin,bytes_in);

				for (i=0 ; i<maxclients->value ; i++)
				{
					cl = game.clients + i;
					if (!g_edicts[i+1].inuse)
						continue;

					if(!memcmp(cl->pers.ipaddr,ip,sizeof(ip)))
//					if(!memcmp(cl->ipaddr,ip,sizeof(ip)))
					{
						gi.dprintf ("Msg is from:%s\n",cl->pers.netname);
						HandleMessage(cl,bufin[0]);
						return;
					}
				}
			}
		}
	}
	else
		gi.dprintf("GenChkSockState::Error:%d\n",WSAGetLastError());

	//Resend packets to peeps we havent received a response from
	for(i=0;i<maxclients->value;i++)
	{
		cl = game.clients +i;
		if(!g_edicts[i+1].inuse)
			continue;

		if(cl->pers.mstate > 4)
		{
			//sent it more than 2 seconds ago and havent tried it more than 3 times
			if(((level.time  - cl->lastsendtime) > 3.0) &&
				(cl->pers.mtry < 4))
			{
				//resend
				switch(cl->pers.mstate)
				{
				case CWPLAY:
					GenSendMsg(cl,MSG_PLAY,level.midiname);
					break;
				case CWPAUSE:
					GenSendMsg(cl,MSG_PAUSE,NULL);
					break;
				case CWRESUME:
					GenSendMsg(cl,MSG_RESUME,NULL);
					break;
				case CWSTOP:
					GenSendMsg(cl,MSG_STOP,NULL);
					break;
				case CWERROR:
					GenSendMsg(cl,MSG_ERROR,NULL);
					break;
				}
			}
		}
	}
#endif


	void HandleMessage(gclient_t *client,char msgid)
{
#if 0	
	client->lastacktime = level.time;
	switch(msgid)
	{
	case MSG_CACKPLAY:
		{
			if(client->sock.mstate == CWPLAY)
			{
				client->sock.mstate = CPLAY;
				client->sock.mtry = 0;
			}
			break;
		}
	case MSG_CACKPAUSE:
		{
			if(client->sock.mstate == CWPAUSE)
			{
				client->sock.mstate = CPAUSE;
				client->sock.mtry = 0;
			}
			break;
		}
	case MSG_CACKRESUME:
		{
			if(client->sock.mstate == CWRESUME)
			{
				client->sock.mstate = CPLAY;
				client->sock.mtry = 0;
			}
			break;
		}
	case MSG_CACKSTOP:
		{
			if(client->sock.mstate == CWSTOP)
			{
				client->sock.mstate = CSTOP;
				client->sock.mtry = 0;
			}
			break;
		}
	default:
	case MSG_CERROR:
		{
			client->sock.mstate = CSTOP;
			client->sock.mtry = -1;
			break;
		}
	}
#endif
}