/*
Copyright (C) 1997-2001 Id Software, Inc.

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

// net_common.c - to reduce redundant os-specific code.

#include "qcommon.h"
#include "net_common.h"

int		ip_sockets[2];
int		server_port;

/*
====================
NET_OpenIP
====================
*/
void NET_OpenIP (void)
{
	cvar_t	*ip;
	int		port;
	int		dedicated;

	ip = Cvar_Get("ip", "localhost", CVAR_NOSET);
	dedicated = Cvar_VariableValue("dedicated");

	if (!ip_sockets[NS_SERVER])
	{
		port = Cvar_Get("ip_hostport", "0", CVAR_NOSET)->value;

		if (!port)
		{
			port = Cvar_Get("hostport", "0", CVAR_NOSET)->value;

			if (!port)
				port = Cvar_Get("port", va("%i", PORT_SERVER), CVAR_NOSET)->value;
		}

		server_port = port;
		ip_sockets[NS_SERVER] = NET_IPSocket(ip->string, port);

		if (!ip_sockets[NS_SERVER] && dedicated)
			Com_Error (ERR_FATAL, "Couldn't allocate dedicated server IP port.");
	}


	// dedicated servers don't need client ports
	if (dedicated)
		return;

	if (!ip_sockets[NS_CLIENT])
	{
		port = Cvar_Get("ip_clientport", "0", CVAR_NOSET)->value;

		if (!port)
		{
			srand(Sys_Milliseconds());
			port = Cvar_Get("clientport", va("%i", PORT_CLIENT), CVAR_NOSET)->value;

			if (!port)
				port = PORT_ANY;
		}

		ip_sockets[NS_CLIENT] = NET_IPSocket(ip->string, port);

		if (!ip_sockets[NS_CLIENT])
			ip_sockets[NS_CLIENT] = NET_IPSocket(ip->string, PORT_ANY);
	}
}

// ===
// jithttp
int GetHTTP (const char *url, char *received, int received_max)
{
	char szRequest[1024];
	char szDomain[1024];
	const char *s, *s2;
	int len, bytes_sent;
	int bytes_read, numread;
	int socket;

	// TODO: check for redirects.  Put this in the net/tcp file?
	received_max -= 1;
	Com_sprintf(szRequest, sizeof(szRequest), "GET %s HTTP/1.0\n\n", url);
	socket = NET_TCPSocket(0);	// Create the socket descriptor

	if (socket == 0)
	{
		Com_Printf("GetHTTP(): Unable to create socket.\n");
		return -1; // No socket created
	}

	s = strstr(url, "://");

	if (s)
		s += 3;
	else
		s = url;

	s2 = strchr(s, '/');
	
	if (s2)
		len = (sizeof(szDomain) - 1 < s2 - s) ? sizeof(szDomain) - 1 : s2 - s;
	else
		len = strlen(s);

	memcpy(szDomain, s, len);
	szDomain[len] = 0;

	if (!NET_TCPConnect(socket, szDomain, 80))
	{
		Com_Printf("GetHTTP(): Unable to connect to %s.\n", szDomain);
		closesocket(socket);
		return -1;
	}

	len = strlen(szRequest);
	bytes_sent = send(socket, szRequest, len, 0);

	if (bytes_sent < len)
	{
		Com_Printf("GetHTTP(): HTTP Server did not accept request, aborting.\n");
		closesocket(socket);
		return -1;
	}

	numread = 0;

	while (numread < received_max && 0 < (bytes_read = recv(socket, received + numread, received_max - numread, 0)))
		numread += bytes_read;

	received[numread] = 0; // make sure it's null terminated.

	// Check for a forward:
	if (memcmp(received + 9, "301", 3) == 0 || memcmp(received + 9, "302", 3) == 0)
	{
		char *newaddress, *s, *s1;

		if ((newaddress = strstr(received, "\nLocation: ")))
		{
			newaddress += sizeof("\nLocation: ") - 1;

			// terminate string at LF or CRLF
			s = strchr(newaddress, '\r');
			s1 = strchr(newaddress, '\n');

			if (s && s < s1)
				*s = '\0';
			else
				*s1 = '\0';

			Com_Printf("Redirect: %s to %s.\n", url, newaddress);
			return GetHTTP(newaddress, received, received_max);
		}
		else
		{
			// Should never happen
			Com_Printf("GetHTTP(): 301 redirect with no new location.\n");
			return -1;
		}
	}

	return numread;
}
// jit
// ===

/*
====================
NET_IPSocket
====================
*/
int NET_IPSocket (char *net_interface, int port)
{
	int newsocket;
	struct sockaddr_in address;
	u_long _true = 1;
	int     i = 1;

	if ((newsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
#ifdef WIN32
		int err;

		err = WSAGetLastError();

		if (err != WSAEAFNOSUPPORT)
			Com_Printf("WARNING: UDP_OpenSocket: socket: %s", NET_ErrorString());
#else
		Com_Printf ("ERROR: UDP_OpenSocket: socket: %s", NET_ErrorString());
#endif
		return 0;
	}

	// make it non-blocking
#ifdef WIN32
	if (ioctlsocket(newsocket, FIONBIO, &_true) == -1)
#else
	if (ioctl(newsocket, FIONBIO, &_true) == -1)
#endif
	{
		Com_Printf("WARNING: UDP_OpenSocket: ioctl FIONBIO: %s\n", NET_ErrorString());
		return 0;
	}

	// make it broadcast capable
	if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) == -1)
	{
		Com_Printf("WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString());
		return 0;
	}

	if (!net_interface || !net_interface[0] || !stricmp(net_interface, "localhost"))
		address.sin_addr.s_addr = INADDR_ANY;
	else
		NET_StringToSockaddr(net_interface, (struct sockaddr *)&address);

	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);

	address.sin_family = AF_INET;

	if (bind(newsocket, (void *)&address, sizeof(address)) == -1)
	{
		Com_Printf("WARNING: UDP_OpenSocket: bind (%d): %s\n", port, NET_ErrorString());
#ifdef WIN32
		closesocket(newsocket);
#else
		close(newsocket);
#endif
		return 0;
	}

	return newsocket;
}


/*
=============
NET_StringToAdr

localhost
idnewt
idnewt:28000
192.246.40.70
192.246.40.70:28000
=============
*/
#define DO(src,dest)	\
	copy[0] = s[src];	\
	copy[1] = s[src + 1];	\
	sscanf (copy, "%x", &val);	\
	((struct sockaddr_ipx *)sadr)->dest = val

qboolean NET_StringToSockaddr (const char *s, struct sockaddr *sadr)
{
	struct hostent *h;
	char *colon;
	int val;
	char copy[128];

	memset(sadr, 0, sizeof(*sadr));

#ifdef WIN32
	if ((strlen(s) >= 23) && (s[8] == ':') && (s[21] == ':'))	// check for an IPX address
	{
		((struct sockaddr_ipx *)sadr)->sa_family = AF_IPX;
		copy[2] = 0;
		DO(0, sa_netnum[0]);
		DO(2, sa_netnum[1]);
		DO(4, sa_netnum[2]);
		DO(6, sa_netnum[3]);
		DO(9, sa_nodenum[0]);
		DO(11, sa_nodenum[1]);
		DO(13, sa_nodenum[2]);
		DO(15, sa_nodenum[3]);
		DO(17, sa_nodenum[4]);
		DO(19, sa_nodenum[5]);
		sscanf(&s[22], "%u", &val);
		((struct sockaddr_ipx *)sadr)->sa_socket = htons((unsigned short)val);
	}
	else
#endif
	{
		((struct sockaddr_in *)sadr)->sin_family = AF_INET;
		((struct sockaddr_in *)sadr)->sin_port = 0;

		strcpy(copy, s);

		// strip off a trailing :port if present
		for (colon = copy; *colon; colon++)
		{
			if (*colon == ':')
			{
				*colon = 0;
				((struct sockaddr_in *)sadr)->sin_port = htons((short)atoi(colon+1));	
			}
		}
		
		if (copy[0] >= '0' && copy[0] <= '9')
		{
			*(int *)&((struct sockaddr_in *)sadr)->sin_addr = inet_addr(copy);
		}
		else
		{
			if (!(h = gethostbyname(copy)))
				return 0;

			*(int *)&((struct sockaddr_in *)sadr)->sin_addr = *(int *)h->h_addr_list[0];
		}
	}
	
	return true;
}

#undef DO


void SockadrToNetadr (struct sockaddr_in *si, netadr_t *a)
{
	struct sockaddr *s = (struct sockaddr *)si;

	if (s->sa_family == AF_INET)
	{
		a->type = NA_IP;
		*(int *)&a->ip = *(int *)&si->sin_addr;
		a->port = si->sin_port;
	}
#ifdef WIN32
	else if (s->sa_family == AF_IPX)
	{
		a->type = NA_IPX;
		memcpy(&a->ipx[0], ((struct sockaddr_ipx *)s)->sa_netnum, 4);
		memcpy(&a->ipx[4], ((struct sockaddr_ipx *)s)->sa_nodenum, 6);
		a->port = ((struct sockaddr_ipx *)s)->sa_socket;
	}
#endif
}


/*
=============
NET_StringToAdr

localhost
idnewt
idnewt:28000
192.246.40.70
192.246.40.70:28000
=============
*/
qboolean NET_StringToAdr (const char *s, netadr_t *a)
{
	struct sockaddr sadr;
	
	if (Q_streq(s, "localhost"))
	{
		memset(a, 0, sizeof(*a));
		a->type = NA_LOOPBACK;
		return true;
	}

	if (!NET_StringToSockaddr(s, &sadr))
		return false;

	SockadrToNetadr((struct sockaddr_in *)&sadr, a);
	return true;
}




