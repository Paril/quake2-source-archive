#include "menu.h"
#include "../qcommon/net_common.h"
#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#define INITIAL_SERVERLIST_SIZE 32
#define UDP_SERVERLIST_PORT 27900
#define SERVERLIST_LISTVIEW_COLUMN_COUNT 4

// Local globals
static pthread_mutex_t m_mut_serverlist;
static pthread_t updatethread;
static pthread_t pingthread;
static qboolean refreshing = false;
static int m_serverPingSartTime;
char g_szRandomServerlistString[32];

// Project-wide Globals
m_serverlist_t m_serverlist;

void free_listview_info_entry(char**, int);

static void ping_broadcast (void)
{
	netadr_t	adr;
	char		buff[256];
	cvar_t		*noudp;
	cvar_t		*noipx;

	NET_Config(true);		// allow remote
	Com_DPrintf("Pinging broadcast...\n");
	noudp = Cvar_Get("noudp", "0", CVAR_NOSET);
	noipx = Cvar_Get("noipx", "1", CVAR_NOSET);

	m_serverPingSartTime = Sys_Milliseconds();

	if (!noudp->value)
	{
		int i;

		// Scan the LAN for multiple IP's
		for (i = PORT_SERVER; i <= PORT_SERVER_MAX; i++) // jitLAN
		{
			adr.type = NA_BROADCAST;
			adr.port = BigShort(i);
			sprintf(buff, "info %i", PROTOCOL_VERSION);
			Netchan_OutOfBandPrint(NS_CLIENT, adr, buff);
		}
	}

	if (!noipx->value)
	{
		adr.type = NA_BROADCAST_IPX;
		adr.port = BigShort(PORT_SERVER);
		sprintf(buff, "info %i", PROTOCOL_VERSION);
		Netchan_OutOfBandPrint(NS_CLIENT, adr, buff);
	}
}

/*
=================
CL_PingServers_f
=================
*/
void *CL_PingServers_multithreaded (void *ptr) // jitmultithreading
{
	register int i;
	char buff[32];

	ping_broadcast();
	
	for (i = 0; i < m_serverlist.numservers; i++)
	{
		sprintf(buff, "info %i", PROTOCOL_VERSION);
		Netchan_OutOfBandPrint(NS_CLIENT, m_serverlist.server[i].adr, buff);
		M_AddToServerList(m_serverlist.server[i].adr, "", true);
		Sleep(32); // jitodo - cvar for delay between pings
	}

	refreshing = false;
	pthread_exit(0);
	return NULL;
}

void CL_PingServers_f (void) // jitmultithreading
{
	if (!refreshing)
	{
		refreshing = true;
		pthread_create(&pingthread, NULL, CL_PingServers_multithreaded, NULL);
	}
}

// Print server list to console
void M_ServerlistPrint_f (void)
{
	int i;

	for(i = 0; i < m_serverlist.nummapped; i++)
		Com_Printf("%2d) %s\n    %s\n", i+1, m_serverlist.ips[i],
			m_serverlist.info[i]);
}

// Queue up a sort
static void Serverlist_Sort (void);
void M_ServerlistSort_f (void)
{
	if(!refreshing)
		Serverlist_Sort();
}

#if 0
static void free_menu_serverlist (void) // jitodo -- eh, something should call this?
{
	if (m_serverlist.info)
		free_string_array(m_serverlist.info, m_serverlist.nummapped);

	if (m_serverlist.ips)
		free_string_array(m_serverlist.ips, m_serverlist.nummapped);

	if (m_serverlist.server)
		Z_Free(m_serverlist.server);

	memset(&m_serverlist, 0, sizeof(m_serverlist_t));
}
#endif
void Serverlist_Clear_f (void)
{
	register int i, j;

	pthread_mutex_lock(&m_mut_serverlist); // jitmultithreading
	pthread_mutex_lock(&m_mut_widgets);

	for (i=0; i<m_serverlist.numservers; i++)
	{
		Z_Free(m_serverlist.server[i].mapname);
		Z_Free(m_serverlist.server[i].servername);
		memset(&m_serverlist.server[i], 0, sizeof(m_serverlist_server_t));
		m_serverlist.server[i].remap = -1;
		j = m_serverlist.server[i].remap;

		if (j >= 0)
		{
			Z_Free(m_serverlist.info[j]);
			m_serverlist.info[j] = NULL;
			Z_Free(m_serverlist.ips[j]);
			m_serverlist.ips[j] = NULL;
		}
	}

	pthread_mutex_unlock(&m_mut_widgets);
	M_RefreshWidget("serverlist", false);
	pthread_mutex_unlock(&m_mut_serverlist);
	m_serverlist.nummapped = m_serverlist.numservers = 0;
}


// Parse info string into server name, players, maxplayers
// Warning! Modifies string!
static void update_serverlist_server (m_serverlist_server_t *server, char *info, int ping)
{
	char *s;
	const char *servername;
	int servernamelen = 0;

	// start at end of string:
	s = strlen(info) + info; 

	// find max players
	while(s > info && *s != '/')
		s--;
	server->maxplayers = atoi(s+1);

	// find current number of players:
	*s = 0;
	s--;
	while(s > info && *s >= '0' && *s <= '9')
		s--;
	server->players = atoi(s+1);
	
	// find map name:
	while(s > info && *s == 32) // clear whitespace;
		s--;

	*(s + 1) = 0;

	while(s > info && *s > 32)
		s--;

	if (!server->mapname)
	{
		server->mapname = text_copy(s + 1);
	}
	else if (!Q_streq(server->mapname, s + 1))
	{
		char buf[MAX_QPATH];

		Z_Free(server->mapname);
		sprintf(buf, "%s/maps/%s.bsp", FS_Gamedir(), s + 1);

		if (!FileExists(buf))
		{
			// put maps clients don't have in italics
			sprintf(buf, "%c%s%c", SCHAR_ITALICS, s + 1, SCHAR_ITALICS);
			server->mapname = text_copy(buf);
		}
		else
		{
			server->mapname = text_copy(s+1);
		}
	}

	// servername is what's left over:
	*s = 0;
	servername = info;

	while (*servername == ' ') // skip leading spaces
		++servername;

	servernamelen = strlen(servername);

	if (!server->servername || strlen(server->servername) < servernamelen)
	{
		if (server->servername)
			Z_Free(server->servername);

		server->servername = text_copy(servername);
	}

	strip_garbage(server->servername, servername, servernamelen + 1);

	// and the ping
	server->ping = ping;
}

static void set_serverlist_server_pingtime (m_serverlist_server_t *server, int pingtime)
{
	server->ping_request_time = pingtime;
}

#define SERVER_NAME_MAXLENGTH 19
#define MAP_NAME_MAXLENGTH 8
static char *format_info_from_serverlist_server(m_serverlist_server_t *server)
{
	static char info[131];
	//char stemp=0, mtemp=0;
	int ping = 999;
	char pingcolor;
	int i, len;

	if (server->ping < 999)
		ping = server->ping;

	// truncate name if too long:
	//if (strlen_noformat(server->servername) > SERVER_NAME_MAXLENGTH)
	//{
	//	stemp = server->servername[SERVER_NAME_MAXLENGTH];
	//	server->servername[SERVER_NAME_MAXLENGTH] = 0;
	//}

	//if (strlen(server->mapname) > MAP_NAME_MAXLENGTH)
	//{
	//	mtemp = server->servername[MAP_NAME_MAXLENGTH];
	//	server->mapname[MAP_NAME_MAXLENGTH] = 0;
	//}

	// assumes SERVER_NAME_MAXLENGTH is 19 vv
	//if(ping < 999)
	//	Com_sprintf(info, sizeof(info), "%-19s %-3d %-8s %d/%d", 
	//		server->servername, ping, server->mapname,
	//		server->players, server->maxplayers);
	//else
	//	Com_sprintf(info, sizeof(info), "%c4%-19s %-3d %-8s %d/%d", 
	//		CHAR_COLOR, server->servername, ping, server->mapname,
	//		server->players, server->maxplayers);

	if (ping < 999)
		info[0] = 0;
	else
		sprintf(info, "%c4", SCHAR_COLOR);

	if ((len = strlen_noformat(server->servername)) <= SERVER_NAME_MAXLENGTH)
	{
		strcat(info, server->servername);

		for (i = 0; i < (SERVER_NAME_MAXLENGTH - len); i++)
			strcat(info, " ");
	}
	else // (len > SERVER_NAME_MAXLENGTH)
	{
		// just in case they put funky color characters in the server name.
		int pos = strpos_noformat(server->servername, SERVER_NAME_MAXLENGTH+1);
		strncat(info, server->servername, pos);
	}

	// Fade ping color from white to orange (like co2 bar colors)
	{
		const int pingcolormin = 222; // white
		const int pingcolormax = 208; // orange
		const int pingmin = 40;
		const int pingmax = 250;

		if (ping < pingmin)
			pingcolor = pingcolormin;
		else if (ping > pingmax)
			pingcolor = pingcolormax;
		else
			pingcolor = pingcolormin - (pingcolormin - pingcolormax) * (ping - pingmin) / (pingmax - pingmin);
	}

	Com_sprintf(info, sizeof(info), "%s %c%c%-3d%c ", info, SCHAR_COLOR, pingcolor, ping, SCHAR_ENDFORMAT);

	if ((len = strlen_noformat(server->mapname)) <= MAP_NAME_MAXLENGTH)
	{
		strcat(info, server->mapname);

		for (i = 0; i < (MAP_NAME_MAXLENGTH - len); i++)
			strcat(info, " ");
	}
	else
	{
		int pos = strpos_noformat(server->mapname, MAP_NAME_MAXLENGTH+1);
		strncat(info, server->mapname, pos);
		
		// handle italics (map player doesn't have)
		if ((unsigned char)server->mapname[0] == SCHAR_ITALICS)
		{
			char buf[2];
			sprintf(buf, "%c", SCHAR_ITALICS);
			strcat(info, buf);
		}
	}

	Com_sprintf(info, sizeof(info), "%s %d/%d", info, server->players, server->maxplayers);

	//if (stemp)
	//	server->servername[SERVER_NAME_MAXLENGTH] = stemp;

	//if (mtemp)
	//	server->mapname[MAP_NAME_MAXLENGTH] = mtemp;

	return info;
}

static char **create_listview_info(m_serverlist_server_t *server)
{
	char **info;
	char pingcolor;
	char buffer[32]; //for creating the player count ("12/16")

	info = Z_Malloc(sizeof(char*) * m_serverlist.listview_info_column_count); 

	info[0] = CopyString(server->servername ? server->servername : "NULL");

	// Fade ping color from white to orange (like co2 bar colors)
	{
		const int pingcolormin = 222; // white
		const int pingcolormax = 208; // orange
		const int pingmin = 40;
		const int pingmax = 250;
		int ping = server->ping;

		if (ping < pingmin)
			pingcolor = pingcolormin;
		else if (ping > pingmax)
			pingcolor = pingcolormax;
		else
			pingcolor = pingcolormin - (pingcolormin - pingcolormax) * (ping - pingmin) / (pingmax - pingmin);

		Com_sprintf(buffer, sizeof(buffer), "%c%c%-3d", SCHAR_COLOR, pingcolor, ping);
	}
	info[1] = CopyString(buffer);

	info[2] = CopyString(server->mapname ? server->mapname : "NULL");

	Com_sprintf(buffer, sizeof(buffer), "%d/%d", server->players, server->maxplayers);
	info[3] = CopyString(buffer);

	return info;
}

static qboolean adrs_equal (netadr_t adr1, netadr_t adr2)
{
	if (adr1.type != adr2.type || adr1.port != adr2.port)
		return false;

	if (adr1.type == NA_IP)
		return adr1.ip[0] == adr2.ip[0] && adr1.ip[1] == adr2.ip[1] &&
			adr1.ip[2] == adr2.ip[2] && adr1.ip[3] == adr2.ip[3];

	if (adr1.type == NA_IPX)
	{
		int i;

		for (i = 0; i < 10; i++)
			if (adr1.ipx[i] != adr2.ipx[i])
				return false;
	}

	return true;
}


static void NetAdrToString (netadr_t adr, char *strout, int sizeout)
{
	if (adr.type == NA_IP)
	{
		Com_sprintf(strout, sizeout,
			"%d.%d.%d.%d:%d", adr.ip[0], adr.ip[1], adr.ip[2], adr.ip[3], ntohs(adr.port));
	}
	else if (adr.type == NA_IPX)
	{
		Com_sprintf(strout, sizeout,
			"%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%i", adr.ipx[0], adr.ipx[1],
			adr.ipx[2], adr.ipx[3], adr.ipx[4], adr.ipx[5], adr.ipx[6], adr.ipx[7],
			adr.ipx[8], adr.ipx[9], ntohs(adr.port));
	}
	else if (adr.type == NA_LOOPBACK)
	{
		Q_strncpyz(strout, "loopback", sizeout);
	}
	else
	{
		return;
	}
}


void M_AddToServerList (netadr_t adr, char *info, qboolean pinging)
{
	int i;
	char addrip[32];
	int ping;
	qboolean added = false;

	NetAdrToString(adr, addrip, sizeof(addrip));

	pthread_mutex_lock(&m_mut_serverlist); // jitmultithreading -- todo, deadlock
	pthread_mutex_lock(&m_mut_widgets);

	// check if server exists in current serverlist:
	for (i = 0; i < m_serverlist.numservers; i++)
	{
		if (adrs_equal(adr, m_serverlist.server[i].adr))
		{
			// update info from server:
			if (pinging)
			{
				set_serverlist_server_pingtime(&m_serverlist.server[i], Sys_Milliseconds());
			}
			else
			{
				ping = Sys_Milliseconds() - m_serverlist.server[i].ping_request_time;
				update_serverlist_server(&m_serverlist.server[i], info, ping);

				if (m_serverlist.server[i].remap < 0)
				{
					m_serverlist.server[i].remap = m_serverlist.nummapped++;
					m_serverlist.ips[m_serverlist.server[i].remap] = text_copy(addrip);
				}
				else
				{
					Z_Free(m_serverlist.info[m_serverlist.server[i].remap]);
					free_listview_info_entry(m_serverlist.listview_info[m_serverlist.server[i].remap], m_serverlist.listview_info_column_count);
				}

				m_serverlist.info[m_serverlist.server[i].remap] =
					text_copy(format_info_from_serverlist_server(&m_serverlist.server[i]));

				m_serverlist.listview_info[m_serverlist.server[i].remap] =
					create_listview_info(&m_serverlist.server[i]);
			}

			added = true;
			break;
		}
	}

	if (!added) // doesn't exist.  Add it.
	{
		i++;

		// List too big?  Alloc more memory:
		// STL would be useful about now
		if (i > m_serverlist.actualsize) 
		{
			char ***templistview_info;
			char **tempinfo;
			char **tempips;
			m_serverlist_server_t *tempserver;

			// Double the size:
			templistview_info = Z_Malloc(sizeof(char**) * m_serverlist.actualsize * 2);
			tempinfo = Z_Malloc(sizeof(char*) * m_serverlist.actualsize * 2);
			tempips = Z_Malloc(sizeof(char*) * m_serverlist.actualsize * 2);
			tempserver = Z_Malloc(sizeof(m_serverlist_server_t) * m_serverlist.actualsize * 2);

			for(i = 0; i < m_serverlist.actualsize; i++)
			{
				templistview_info[i] = m_serverlist.listview_info[i];
				tempinfo[i] = m_serverlist.info[i];
				tempips[i] = m_serverlist.ips[i];
				tempserver[i] = m_serverlist.server[i];
			}

			for (i = m_serverlist.actualsize; i < m_serverlist.actualsize * 2; i++)
			{
				memset(&tempserver[i], 0, sizeof(m_serverlist_server_t));
				tempserver[i].remap = -1;
			}

			Z_Free(m_serverlist.info);
			Z_Free(m_serverlist.ips);
			Z_Free(m_serverlist.server);
			Z_Free(m_serverlist.listview_info);

			m_serverlist.info = tempinfo;
			m_serverlist.ips = tempips;
			m_serverlist.server = tempserver;
			m_serverlist.listview_info = templistview_info;

			m_serverlist.actualsize *= 2;
		}

		m_serverlist.server[m_serverlist.numservers].adr = adr; // jitodo - test

		// add data to serverlist:
		if (pinging)
		{
			set_serverlist_server_pingtime(&m_serverlist.server[m_serverlist.numservers], Sys_Milliseconds());
			ping = 999;
			update_serverlist_server(&m_serverlist.server[m_serverlist.numservers], info, ping);
		}
		else
		{
			// this will probably only happen with LAN servers.
			ping = Sys_Milliseconds() - m_serverPingSartTime;
			m_serverlist.ips[m_serverlist.nummapped] = text_copy(addrip);
			update_serverlist_server(&m_serverlist.server[m_serverlist.numservers], info, ping);

			m_serverlist.info[m_serverlist.nummapped] =
				text_copy(format_info_from_serverlist_server(&m_serverlist.server[m_serverlist.numservers]));

			m_serverlist.listview_info[m_serverlist.nummapped] =
					create_listview_info(&m_serverlist.server[i]);

			m_serverlist.server[m_serverlist.numservers].remap = m_serverlist.nummapped;
			m_serverlist.nummapped++;
		}

		m_serverlist.numservers++;
	}

	m_serverlist.sortthisframe = true;
	// Tell the widget the serverlist has updated:
	pthread_mutex_unlock(&m_mut_widgets);
	//M_RefreshActiveMenu(); // jitodo - target serverlist window specifically.
	M_RefreshWidget("serverlist", false);
	pthread_mutex_unlock(&m_mut_serverlist); // jitmultithreading
}

// Color all the items grey:
static void grey_serverlist (void)
{
	char *str;
	int i;

	for(i=0; i<m_serverlist.nummapped; i++)
	{
		if ((unsigned char)m_serverlist.info[i][0] != SCHAR_COLOR)
		{
			str = text_copy(va("%c4%s", SCHAR_COLOR, m_serverlist.info[i]));
			Z_Free(m_serverlist.info[i]);
			m_serverlist.info[i] = str;
		}
	}
}

// color servers grey and re-ping them
void M_ServerlistRefresh_f (void)
{
	grey_serverlist();
	CL_PingServers_f();
}


static void CL_AddServerlistServer (const char *address)
{
	netadr_t adr;
	char buff[64];
	
	if (!NET_StringToAdr(address, &adr))
	{
		Com_Printf("Bad address: %s\n", address);
		return;
	}

	Com_sprintf(buff, sizeof(buff), "%s --- 0/0", address);
	// Add to list as being pinged
	M_AddToServerList(adr, buff, true);
}

static void CL_PingServerlistServer (const char *pServerAddress)
{
	char buff[64];
	netadr_t adr;

	Com_Printf("Pinging %s...\n", pServerAddress);

	if (!NET_StringToAdr(pServerAddress, &adr))
	{
		Com_Printf("Bad address: %s\n", pServerAddress);
		return;
	}

	if (!adr.port)
		adr.port = BigShort(PORT_SERVER);

	Com_sprintf(buff, sizeof(buff), "info %i", PROTOCOL_VERSION);
	Netchan_OutOfBandPrint(NS_CLIENT, adr, buff);
	Com_sprintf(buff, sizeof(buff), "%s --- 0/0", pServerAddress);
	// Add to list as being pinged
	M_AddToServerList(adr, buff, true);
}

extern qboolean g_notified_of_new_version;

static void M_ServerlistUpdateUDP (int nStart)
{
	netadr_t adr;
	static qboolean didUpdateCheck = false;

	adr.port = htons(UDP_SERVERLIST_PORT);
	NET_StringToAdr(serverlist_udp_source1->string, &adr);
	//Netchan_OutOfBandPrint(NS_CLIENT, adr, "serverlist1 %d %s\n", nStart, g_szRandomServerlistString);
	Netchan_OutOfBandPrint(NS_CLIENT, adr, "serverlist2\n");

	if (!didUpdateCheck && !g_notified_of_new_version)
	{
		 // Not currently implemented on dplogin server, but it may work eventually:
		Netchan_OutOfBandPrint(NS_CLIENT, adr, "updatecheck1 " BUILD_S "\n"); // versioncheck / checkversion / buildcheck
		didUpdateCheck = true;
	}
}


void CL_Serverlist2Packet (netadr_t net_from, sizebuf_t *net_message)
{
	static qboolean firstupdate = true;

	if (Q_streq(MSG_ReadString(net_message), "serverlist2"))
	{
		int num_servers, i, j;
		int ip[4], port;
		char szServer[256];

		// Wait until we get a server list response to clear the list.  We don't want to wipe somebody's list
		// if the server list server isn't responding.  Only clear the list once per run, as well (in case server sends multiple packets).
		if (firstupdate)
		{
			firstupdate = false;
			Serverlist_Clear_f();
		}

		num_servers = ntohl(MSG_ReadLong(net_message));

		for (i = 0; i < num_servers; ++i)
		{
			for (j = 0; j < 4; ++j)
			{
				ip[j] = MSG_ReadByte(net_message);

				if (ip[j] < 0)
				{
					assert(ip[j] >= 0);
					return; // end of packet -- corrupt or truncated, just bail out.  todo: handle > 164 servers.
				}
			}

			port = ntohs(MSG_ReadShort(net_message));
			Com_sprintf(szServer, sizeof(szServer), "%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], port);
			CL_AddServerlistServer(szServer);
		}

		CL_PingServers_f();
	}
}


// unused function
void CL_ServerlistPacket (netadr_t net_from, const char *sRandStr, sizebuf_t *net_message)
{
	char *sServerIP;
	int i = 1;

	if (!Q_streq(sRandStr, g_szRandomServerlistString))
		return; // malicious packet?  Whatever, just ignore it.

	while ((sServerIP = MSG_ReadStringLine(net_message)) && *sServerIP) // not threadsafe!
	{
		if (*sServerIP == '*')
		{
			if (memcmp(sServerIP, "*continue:", sizeof("*continue")) == 0)
			{
				int nStart;

				// serverlist did not fit in one packet, we need another to resume it.
				// actually, we'll probably never use this -- just have the server send all packets.
				nStart = atoi(sServerIP + sizeof("*continue:"));
				M_ServerlistUpdateUDP(nStart);
				break;
			}

			continue; // not a serverlist ip, special line.
		}

		CL_PingServerlistServer(sServerIP);
	}
}


// Download list of ip's from a remote location and
// add them to the local serverlist
#define BUFFER_SIZE 32767
// unused function
static void M_ServerlistUpdate (char *sServerSource)
{
	char *buffer = NULL;
	int numread = -1;
	qboolean file = false;
	char *current;
	char *found = NULL;
	int bytes_read = 0;

	if (!*sServerSource)
		return;

	Com_Printf("Retrieving serverlist from %s.\n", sServerSource);

	if (strstr(sServerSource, "://"))
	{
		buffer = Z_Malloc(BUFFER_SIZE);
		numread = GetHTTP(sServerSource, buffer, BUFFER_SIZE);
		current = buffer;

		if (numread < 0)
			return;

		current = buffer;

		// find \n\n, thats the end of header/beginning of the data
		while (*current != '\n' || *(current + 2) != '\n')
		{
			if (current > buffer + numread)
			{
				FILE *fp;

				Com_Printf("WARNING: Invalid serverlist %s (no header).\n", sServerSource);

				if (fp = fopen("invalid_serverlist.txt", "ab"))
				{
					fwrite(buffer, numread, 1, fp);
					fclose(fp);
				}

				free(buffer);
				return; 
			}

			current++;
		};

		current = current + 3; // skip the trailing \n.  We're at the beginning of the data now

		if (strchr(current, '<'))
		{
			// If it has any HTML codes in it, we know it's not valid (probably a 404 page).
			if (strstr(current, "Not Found") || strstr(current, "not found"))
			{
				Com_Printf("WARNING: %s returned 404 Not Found.\n", sServerSource);
			}
			else
			{
				FILE *fp;

				Com_Printf("WARNING: Invalid serverlist: %s\n", sServerSource);

				if (fp = fopen("invalid_serverlist.txt", "ab"))
				{
					fwrite(buffer, numread, 1, fp);
					fclose(fp);
				}
			}

			Z_Free(buffer);
			return;
		}
	}
	else // Local file?
	{
		numread = FS_LoadFileZ(sServerSource, (void **)&buffer);
		file = true;
		current = buffer;

		if (numread < 0)
		{
			Com_Printf("WARNING: Bad serverlist address: %s\n", sServerSource);
			Com_Printf("Serverlist must be a file in the pball directory or http site.\n");
			return;
		}
	}

	// Ping all of the servers on the list
	while (current < buffer + numread)
	{
		found = current;						// Mark the beginning of the line

		while (*current && *current != '\r' && *current != '\n')
		{										// Find the end of the line
			current++; 
			
			if (current > buffer + numread)
			{
				if (file)
					FS_FreeFile(buffer);
				else
					Z_Free(buffer);

				return;	// Exit if we run out of room
			}
		}

		*current = 0; // NULL terminate the string

		if (!*found || Q_streq(found, "X"))
			goto done;

		CL_PingServerlistServer(found);
		Sleep(16); // jitodo -- make a cvar for the time between pings
		// Start at the next line:
		current++;

		while ((*current == '\r' || *current == '\n') && (current < buffer + numread))
			current++;
	};

done:
	if (current + 1 < buffer + numread)
	{
		// Check to make sure they're updated with the latest version.
		if (found = strstr(current + 1, "LatestClientBuild:"))
		{
			int latest_build;

			found += sizeof("LatestClientBuild:")-1;
			latest_build = atoi(found);

			if (latest_build > BUILD)
			{
				if (!g_notified_of_new_version) // only pop it up once.
				{
					Cbuf_AddTextThreadsafe("menu newversion\n");
					g_notified_of_new_version = true;
				}
			}
		}
	}

	if (file)
		FS_FreeFile(buffer);
	else
		Z_Free(buffer);

	return;
}

void ServerlistBlacklistUpdate (const char *sURL)
{
	if (!sURL || !*sURL)
		return;

//	todo;
}

// Note: This is no longer used (we use UDP instead)
void *M_ServerlistUpdate_multithreaded (void *ptr)
{
	grey_serverlist();
	ping_broadcast();
	ServerlistBlacklistUpdate(serverlist_blacklist->string);
	M_ServerlistUpdate(serverlist_source->string);
	M_ServerlistUpdate(serverlist_source2->string);
	M_ServerlistUpdate(serverlist_source3->string);
	refreshing = false;
	pthread_exit(0);

	return NULL;
}


static void GenerateRandomServerlistString (void)
{
	int i;

	for (i = 0; i < 12; ++i)
	{
		g_szRandomServerlistString[i] = (int)(frand() * 26) + 'a';
	}

	g_szRandomServerlistString[i] = 0;
}


void M_ServerlistUpdate_f (void)
{
	NET_Config(true); // Open up ports

#if 1 // todo - cvar?
	M_ServerlistUpdateUDP(0);
#else
	if (!refreshing)
	{
		refreshing = true;
		pthread_create(&updatethread, NULL, M_ServerlistUpdate_multithreaded, NULL);
	}
#endif
}


static void create_serverlist (int size)
{
	register int i;

	memset(&m_serverlist, 0, sizeof(m_serverlist_t));
	m_serverlist.actualsize = size;
	m_serverlist.listview_info = Z_Malloc(sizeof(char**)*size);
	m_serverlist.listview_info_column_count = SERVERLIST_LISTVIEW_COLUMN_COUNT;
	m_serverlist.info = Z_Malloc(sizeof(char*)*size); 
	m_serverlist.ips = Z_Malloc(sizeof(char*)*size);
	m_serverlist.server = Z_Malloc(sizeof(m_serverlist_server_t)*size);

	for (i=0; i<size; i++)
	{
		memset(&m_serverlist.server[i], 0, sizeof(m_serverlist_server_t));
		m_serverlist.server[i].remap = -1;
	}
}

static __inline char *skip_string (char *s)
{
	while (*s)
		s++;
	s++;
	return s;
}

static qboolean serverlist_load (void)
{
	int size;
	char *data;
	char *ptr;
	char buffer[MAX_TOKEN_CHARS];

	size = FS_LoadFile("serverlist.dat", (void**)&data);

	if (size > -1)
	{
		int endiantest;
		int actualsize;
		register int i;
		int j;

		// Check header to make sure it's a valid file:
		if (memcmp(data, "PB2Serverlist1.00", sizeof("PB2Serverlist1.00")-1) != 0)
		{
			FS_FreeFile(data);
			return false;
		}

		ptr = data + sizeof("PB2Serverlist1.00")-1;
		memcpy(&endiantest, ptr, sizeof(int));

		if (endiantest != 123123123)
		{
			FS_FreeFile(data);
			return false;
		}

		// Read our data:
		ptr += sizeof(int);
		memcpy(&actualsize, ptr, sizeof(int));

		if (actualsize < INITIAL_SERVERLIST_SIZE)
		{
			FS_FreeFile(data);
			return false;
		}

		ptr += sizeof(int);
		create_serverlist(actualsize);
		memcpy(&m_serverlist.numservers, ptr, sizeof(int));
		ptr += sizeof(int);
		memcpy(&m_serverlist.nummapped, ptr, sizeof(int));
		ptr += sizeof(int);

		for (i=0; i<m_serverlist.numservers; i++)
		{
			memcpy(&m_serverlist.server[i].adr, ptr, sizeof(netadr_t));
			ptr += sizeof(netadr_t);
			m_serverlist.server[i].remap = *(short*)ptr;
			ptr += sizeof(short);
			m_serverlist.server[i].servername = text_copy(ptr);
			ptr = skip_string(ptr);
			m_serverlist.server[i].mapname = text_copy(ptr);
			ptr = skip_string(ptr);
			m_serverlist.server[i].ping = *(unsigned short*)ptr;
			ptr += sizeof(unsigned short);
			m_serverlist.server[i].players = *(unsigned char*)ptr;
			ptr += sizeof(unsigned char);
			m_serverlist.server[i].maxplayers = *(unsigned char*)ptr;
			ptr += sizeof(unsigned char);
		}

		for (i=0; i<m_serverlist.nummapped; i++)
		{
			m_serverlist.ips[i] = text_copy(ptr);
			ptr = skip_string(ptr);
			m_serverlist.info[i] = text_copy(ptr);
			ptr = skip_string(ptr);

			//xrichardx todo: I think this should be done in another way. Maybe the way the data is stored should be changed
			m_serverlist.listview_info[i] = (char**)(Z_Malloc(sizeof(char*)*m_serverlist.listview_info_column_count));
			for (j = 0; j <m_serverlist.numservers; j++) //make sure we assign the information correctly
			{
				NetAdrToString(m_serverlist.server[j].adr, buffer, sizeof(buffer));
				if ( 0 == strcmp(buffer, m_serverlist.ips[i]) )
				{
					m_serverlist.listview_info[i][0] = CopyString(m_serverlist.server[j].servername);
					Com_sprintf(buffer, sizeof(buffer), "%d", m_serverlist.server[j].ping);
					m_serverlist.listview_info[i][1] = CopyString(buffer);
					m_serverlist.listview_info[i][2] = CopyString(m_serverlist.server[j].mapname);
					Com_sprintf(buffer, sizeof(buffer), "%d/%d", m_serverlist.server[j].players, m_serverlist.server[j].maxplayers);
					m_serverlist.listview_info[i][3] = CopyString(buffer);
					break;
				}
			}
		}

		FS_FreeFile(data);
		return true;
	}

	return false;
}


void Serverlist_Init (void)
{
	GenerateRandomServerlistString();

	// Init mutex:
	pthread_mutex_init(&m_mut_serverlist, NULL);

	// Init server list:
	if (!serverlist_load())
	{
		create_serverlist(INITIAL_SERVERLIST_SIZE);
	}

	// Add scommands:
	Cmd_AddCommand("serverlist_update", M_ServerlistUpdate_f);
	Cmd_AddCommand("serverlist_refresh", M_ServerlistRefresh_f);
	Cmd_AddCommand("serverlist_print", M_ServerlistPrint_f);
	Cmd_AddCommand("serverlist_clear", Serverlist_Clear_f);
	Cmd_AddCommand("serverlist_sort", M_ServerlistSort_f);
}


static void serverlist_save (void)
{
	FILE *fp;
	char szFilename[MAX_QPATH];

	if (!m_serverlist.actualsize)
		return; // don't write if there's no data.

	sprintf(szFilename, "%s/serverlist.dat", FS_Gamedir());

	if ((fp = fopen(szFilename, "wb")))
	{
		int endiantest = 123123123;
		register int i;
		short stemp;
		unsigned char ctemp;
		char *s;

		// Write Header:
		fwrite("PB2Serverlist1.00", sizeof("PB2Serverlist1.00")-1, 1, fp);
		fwrite(&endiantest, sizeof(int), 1, fp);
		
		// Write our data:
		pthread_mutex_lock(&m_mut_serverlist); // make sure no other threads are using it
		fwrite(&m_serverlist.actualsize, sizeof(int), 1, fp);
		fwrite(&m_serverlist.numservers, sizeof(int), 1, fp);
		fwrite(&m_serverlist.nummapped, sizeof(int), 1, fp);
		
		for (i=0; i<m_serverlist.numservers; i++)
		{
			fwrite(&m_serverlist.server[i].adr, sizeof(netadr_t), 1, fp);
			stemp = m_serverlist.server[i].remap;
			fwrite(&stemp, sizeof(short), 1, fp);
			s = m_serverlist.server[i].servername;
			fwrite(s, strlen(s)+1, 1, fp);
			s = m_serverlist.server[i].mapname;
			fwrite(s, strlen(s)+1, 1, fp);
			stemp = (short)m_serverlist.server[i].ping;
			fwrite(&stemp, sizeof(short), 1, fp);
			ctemp = (unsigned char)m_serverlist.server[i].players;
			fwrite(&ctemp, sizeof(unsigned char), 1, fp);
			ctemp = (unsigned char)m_serverlist.server[i].maxplayers;
			fwrite(&ctemp, sizeof(unsigned char), 1, fp);
		}

		for (i=0; i<m_serverlist.nummapped; i++)
		{
			s = m_serverlist.ips[i];
			fwrite(s, strlen(s)+1, 1, fp);
			s = m_serverlist.info[i];
			fwrite(s, strlen(s)+1, 1, fp);
		}
		
		pthread_mutex_unlock(&m_mut_serverlist); // tell other threads the serverlist is safe
		fclose(fp);
	}
}


void Serverlist_Shutdown (void)
{
	serverlist_save();
}

// Weighted by ping & players
static int Serverlist_SortCompare (const void *a, const void *b)
{
	const m_serverlist_server_t *serverA = a;
	const m_serverlist_server_t *serverB = b;

	if (serverA && serverB)
	{
		int ret = ((serverB->players + 1) * 10000 / (serverB->ping + 1)) - ((serverA->players + 1) * 10000 / (serverA->ping + 1));
		return ret;
	}

	assert(0);
	return 0;
}

// Weighted solely by ping
static int Serverlist_SortCompare_Ping (const void *a, const void *b)
{
	const m_serverlist_server_t *serverA = a;
	const m_serverlist_server_t *serverB = b;
	if (serverA && serverB)
		return (serverA->ping - serverB->ping);
	assert(0);
	return 0;
}

// Weighted solely by players
static int Serverlist_SortCompare_Players (const void *a, const void *b)
{
	const m_serverlist_server_t *serverA = a;
	const m_serverlist_server_t *serverB = b;
	if (serverA && serverB)
		return (serverB->players - serverA->players);
	assert(0);
	return 0;
}

// Sort by alphabetical name
static int Serverlist_SortCompare_Name (const void *a, const void *b)
{
	const m_serverlist_server_t *serverA = a;
	const m_serverlist_server_t *serverB = b;
	if (serverA && serverB)
		return stricmp(serverA->servername, serverB->servername);
	assert(0);
	return 0;
}

// Sort by alphabetical mapname
static int Serverlist_SortCompare_Mapname (const void *a, const void *b)
{
	const m_serverlist_server_t *serverA = a;
	const m_serverlist_server_t *serverB = b;
	if (serverA && serverB)
	{
		// cope with unknown maps
		char* aname = ((unsigned char)serverA->mapname[0] == SCHAR_ITALICS) ? &serverA->mapname[1] : serverA->mapname;
		char* bname = ((unsigned char)serverB->mapname[0] == SCHAR_ITALICS) ? &serverB->mapname[1] : serverB->mapname;
		int result = stricmp(aname, bname);
		if(Cvar_Get("serverlist_order_invert","0",0)->value)
			result = -result;
		return result;
	}
	assert(0);
	return 0;
}

static void Serverlist_Sort (void)
{
	int i;
	int remap = 0;
	char addr[32];

	int (*comparefunc)(const void *, const void *) = NULL;

	switch((int)Cvar_Get("serverlist_order","0",0)->value)
	{
	case 1:
		comparefunc = &Serverlist_SortCompare_Name;
		break;
	case 2:
		comparefunc = &Serverlist_SortCompare_Ping;
		break;
	case 3:
		comparefunc = &Serverlist_SortCompare_Mapname;
		break;
	case 4:
		comparefunc = &Serverlist_SortCompare_Players;
		break;
	default:
		comparefunc = &Serverlist_SortCompare;
	}

	qsort(m_serverlist.server, m_serverlist.numservers, sizeof(m_serverlist.server[0]), comparefunc);

	// viciouz - apparently the following doesn't work for alphabetical sorting. how odd.
	// anyway, as it's only running one frame the small efficiency saving isn't really necessary.
	// for (i = 0; i < m_serverlist.nummapped; ++i)
	for (i = 0; i < m_serverlist.numservers; ++i)
	{
		if (m_serverlist.server[i].remap >= 0)
		{
			if (remap < m_serverlist.nummapped) // probably unnecessary to check, but just in case
			{
				NetAdrToString(m_serverlist.server[i].adr, addr, sizeof(addr));
				m_serverlist.server[i].remap = remap;

				Z_Free(m_serverlist.ips[remap]);
				m_serverlist.ips[remap] = text_copy(addr);

				Z_Free(m_serverlist.info[remap]);
				m_serverlist.info[remap] = text_copy(format_info_from_serverlist_server(&m_serverlist.server[i]));

				free_listview_info_entry(m_serverlist.listview_info[remap], m_serverlist.listview_info_column_count);
				m_serverlist.listview_info[remap] = create_listview_info(&m_serverlist.server[i]);

				++remap;
			}
		}
	}
}


void CL_Serverlist_RunFrame (void)
{
	// Don't sort while refreshing as refresh runs in a different thread and could mess things up
	// (might be ok to remove this check later if determined to be thread safe, or add a lock)
	if (m_serverlist.sortthisframe && !refreshing)
	{
		m_serverlist.sortthisframe = false;
		Serverlist_Sort();
	}
}

