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

#ifdef WIN32
#include <winsock.h>
#include <wsipx.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <errno.h>
#include <arpa/inet.h>
#endif

extern int ip_sockets[2];
extern int server_port;

void NET_OpenIP (void);
int NET_IPSocket (char *net_interface, int port);
int GetHTTP (const char *url, char *received, int received_max);
qboolean NET_StringToSockaddr (const char *s, struct sockaddr *sadr);
void SockadrToNetadr (struct sockaddr_in *s, netadr_t *a);
