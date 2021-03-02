/*
Copyright (C) 1998-2005 Digital Paint

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
#include "../qcommon/qcommon.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <errno.h>

qboolean	NET_StringToSockaddr (char *s, struct sockaddr *sadr);
//=============================================================================
///* ACT
/*
====================
NET_TCPSocket

  This socket WILL BLOCK! Use it quickly, and DUMP it.
====================
*/
int NET_TCPSocket (int port)
{
	int		newsocket;
	int		err;

	if ((newsocket = socket(PF_INET, SOCK_STREAM , IPPROTO_TCP)) == -1)
	{
		err = errno;
		Com_Printf("WARNING: TCP_OpenSocket: socket: %s", NET_ErrorString());
		return 0;
	}

	return newsocket;
}

int NET_TCPConnect (SOCKET sockfd, char *net_remote_address, int port)
{
	struct sockaddr_in	address;	// Internet socket address

	if (port < 1 || port > 32767)
	{
		Com_Printf("WARNING: TCP_Connect: Invalid Port: %i\n", port);
		closesocket(sockfd);
		return 0;
	}

	if (!net_remote_address || !net_remote_address[0])
	{
		Com_Printf("WARNING: TCP_Connect: No host specified\n");
		closesocket(sockfd);
		return 0;
	}
	else
	{
		NET_StringToSockaddr(net_remote_address, (struct sockaddr *)&address);
	}

        address.sin_family = AF_INET;			// host byte order
        address.sin_port = htons((short)port);	// short, network byte order
        memset(&(address.sin_zero), '\0', 8);	// zero the rest of the struct

	if (-1 == connect(sockfd, (struct sockaddr*)&address, sizeof(struct sockaddr)))
	{
		Com_Printf ("WARNING: TCP_Connect failed: connect(%s:%i): %s\n", net_remote_address,port,NET_ErrorString());
		closesocket (sockfd);
		return 0;
	}

	return 1;
}
// ACT */
