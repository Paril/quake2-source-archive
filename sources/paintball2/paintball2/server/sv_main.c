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

#include "server.h"
#include "../qcommon/net_common.h"

netadr_t	master_adr[MAX_MASTERS];	// address of group servers

client_t	*sv_client;			// current client

cvar_t	*sv_paused;
cvar_t	*sv_timedemo;

cvar_t	*sv_enforcetime;
cvar_t	*sv_enforcetime2;		// jitspeedhackcheck
cvar_t  *sv_cullentities;

cvar_t	*timeout;				// seconds without any message
cvar_t	*zombietime;			// seconds to sink messages after disconnect

cvar_t	*rcon_password;			// password for remote server commands

cvar_t *allow_download;
cvar_t *allow_download_players;
cvar_t *allow_download_models;
cvar_t *allow_download_sounds;
cvar_t *allow_download_maps;
#ifdef USE_DOWNLOAD3
cvar_t *sv_fast_download; // jitdownload
cvar_t *sv_fast_download_max_rate;
#endif

cvar_t *sv_airaccelerate;

cvar_t	*sv_noreload;			// don't reload level state when reentering

cvar_t	*maxclients;			// FIXME: rename sv_maxclients
cvar_t	*sv_showclamp;

cvar_t	*hostname;
cvar_t	*public_server;			// should heartbeats be sent

cvar_t	*sv_reconnect_limit;	// minimum seconds between connect messages
cvar_t	*sv_noextascii = NULL;	// jit

cvar_t	*sv_locked;				// T3RR0R15T: Locked server. Prevent new players from connecting. (from R1Q2)
cvar_t	*sv_blockednames;		// T3RR0R15T: Blocked player names. Prevent players with bad names from connecting.

cvar_t	*sv_certificated;		// T3RR0R15T: certificated server info

cvar_t	*sv_reserved_slots;		// T3RR0R15T: Reserved slots (from R1Q2)
cvar_t	*sv_reserved_password;	// T3RR0R15T: Reserved slots (from R1Q2)

cvar_t	*sv_consolename;		// T3RR0R15T: Name for console say messages

void Master_Shutdown (void);


//============================================================================


#ifdef USE_DOWNLOAD3
void SV_FreeDownloadData (client_t *cl) // jitdownload
{
	if (cl->download)
	{
		FS_FreeFile(cl->download);
		cl->download = NULL;
	}

	if (cl->download3_chunks)
	{
		Z_Free(cl->download3_chunks);
		cl->download3_chunks = NULL;
	}

	if (cl->download3_window)
	{
		Z_Free(cl->download3_window);
		cl->download3_window = NULL;
	}

	cl->download3_active = 0;
}
#endif


/*
=====================
SV_DropClient

Called when the player is totally leaving the server, either willingly
or unwillingly.  This is NOT called if the entire server is quiting
or crashing.
=====================
*/
void SV_DropClient (client_t *drop)
{
	// add the disconnect
	MSG_WriteByte(&drop->netchan.message, svc_disconnect);

	if (drop->state == cs_spawned)
	{
		// call the prog function for removing a client
		// this will remove the body, among other things
		ge->ClientDisconnect(drop->edict);
	}

#ifdef USE_DOWNLOAD3
	SV_FreeDownloadData(drop); // jitdownload
#else
	if (drop->download)
	{
		FS_FreeFile(drop->download);
		drop->download = NULL;
	}
#endif

	drop->state = cs_zombie;		// become free in a few seconds
	drop->name[0] = 0;
}



/*
==============================================================================

CONNECTIONLESS COMMANDS

==============================================================================
*/

/*
===============
SV_StatusString

Builds the string that is sent as heartbeats and status replies
===============
*/
char	*SV_StatusString (void)
{
	char	*serverinfo;						// T3RR0R15T
	char	player[1024];
	static char	status[MAX_MSGLEN - 16];
	int		i;
	client_t	*cl;
	int		statusLength;
	int		playerLength;

	serverinfo = Cvar_Serverinfo();

	// T3RR0R15T: hide reserved slots
	Info_SetValueForKey(serverinfo, "maxclients", va("%d", (int)maxclients->value - (int)sv_reserved_slots->value));

	strcpy(status, serverinfo);
	strcat(status, "\n");
	statusLength = strlen(status);

	if (sv.attractloop) // jitdemo - fix crash from status command while playing a demo.
		return status;

	for (i = 0; i < maxclients->value; i++)
	{
		cl = &svs.clients[i];

		if (cl->state == cs_connected || cl->state == cs_spawned)
		{
			Com_sprintf(player, sizeof(player), "%i %i \"%s\"\n", 
				cl->edict->client->ps.stats[STAT_FRAGS], cl->ping, cl->name);
			playerLength = strlen(player);

			if (statusLength + playerLength >= sizeof(status))
				break;		// can't hold any more

			strcpy(status + statusLength, player);
			statusLength += playerLength;
		}
	}

	return status;
}

/*
================
SVC_Status

Responds with all the info that qplug or qspy can see
================
*/
void SVC_Status (void)
{
	Netchan_OutOfBandPrint(NS_SERVER, net_from, "print\n%s", SV_StatusString());
}

/*
================
SVC_Ack

================
*/
void SVC_Ack (void)
{
	Com_Printf("Ping acknowledge from %s.\n", NET_AdrToString(net_from));
}

void SVC_HeartbeatResponse (const char *sAddress) // jitheartbeat
{
	Com_Printf("Heartbeat acknowledged from %s.\n", sAddress);
}


/*
================
SVC_Info

Responds with short info for broadcast scans
The second parameter should be the current protocol version number.
================
*/
void SVC_Info (void)
{
	char	string[256]; // jit (was 64)
	int		i, count;
	int		version;

	if (maxclients->value == 1)
		return;		// ignore in single player

	version = atoi(Cmd_Argv(1));

	if (!version)
		return; // jitsecurity

	if (version != PROTOCOL_VERSION)
	{
		Com_sprintf(string, sizeof(string), "%s: Wrong version.\n", hostname->string, sizeof(string));
	}
	else
	{
		count = 0;

		for (i = 0; i < maxclients->value; i++)
			if (svs.clients[i].state >= cs_connected)
				count++;

		Com_sprintf(string, sizeof(string), "%16s %8s %2i/%2i\n", hostname->string, sv.name, count, (int)maxclients->value - (int)sv_reserved_slots->value); // T3RR0R15T: hide reserved slots
	}

	Netchan_OutOfBandPrint(NS_SERVER, net_from, "info\n%s", string);
}

/*
================
SVC_Ping

Just responds with an acknowledgement
================
*/
void SVC_Ping (void)
{
	Netchan_OutOfBandPrint (NS_SERVER, net_from, "ack");
}


/*
=================
SVC_GetChallenge

Returns a challenge number that can be used
in a subsequent client_connect command.
We do this to prevent denial of service attacks that
flood the server with invalid connection IPs.  With a
challenge, they must give a valid IP address.
=================
*/
void SVC_GetChallenge (void)
{
	int		i;
	int		oldest;
	int		oldestTime;

	oldest = 0;
	oldestTime = 0x7fffffff;

	// see if we already have a challenge for this ip
	for (i = 0 ; i < MAX_CHALLENGES ; i++)
	{
		if (NET_CompareBaseAdr (net_from, svs.challenges[i].adr))
			break;
		if (svs.challenges[i].time < oldestTime)
		{
			oldestTime = svs.challenges[i].time;
			oldest = i;
		}
	}

	if (i == MAX_CHALLENGES)
	{
		// overwrite the oldest
		svs.challenges[oldest].challenge = rand() & 0x7fff;
		svs.challenges[oldest].adr = net_from;
		svs.challenges[oldest].time = curtime;
		i = oldest;
	}

	// send it back
	Netchan_OutOfBandPrint (NS_SERVER, net_from, "challenge %i", svs.challenges[i].challenge);
}

/*
==================
SVC_DirectConnect

A connection request that did not come from the master
==================
*/
void SVC_DirectConnect (void)
{
	char		userinfo[MAX_INFO_STRING];
	netadr_t	adr;
	int			i;
	client_t	*cl, *newcl;
	client_t	temp;
	edict_t		*ent;
	int			edictnum;
	int			version;
	int			qport;
	int			challenge;
	char		blockednames_list[1024];	// T3RR0R15T: Blocked player names.
	char		lowercase_name[1024];		// T3RR0R15T: Blocked player names.
	char		*pname, *badname;			// T3RR0R15T: Blocked player names.
	char		*pass;			// T3RR0R15T: Reserved slots (from R1Q2)
	int			reserved;		// T3RR0R15T: Reserved slots (from R1Q2)

	adr = net_from;
	Com_DPrintf("SVC_DirectConnect ()\n");
	version = atoi(Cmd_Argv(1));

	if (version != PROTOCOL_VERSION)
	{
		Netchan_OutOfBandPrint(NS_SERVER, adr, "print\nServer is version %4.2f.\n", VERSION);
		Com_DPrintf("    rejected connect from version %i.\n", version);
		return;
	}

	qport = atoi(Cmd_Argv(2));
	challenge = atoi(Cmd_Argv(3));
	Q_strncpyz(userinfo, Cmd_Argv(4), sizeof(userinfo)-1);
	userinfo[sizeof(userinfo) - 1] = 0;

	// force the IP key/value pair so the game can filter based on ip
	Info_SetValueForKey(userinfo, "ip", NET_AdrToString(net_from));

	// attractloop servers are ONLY for local clients
	if (sv.attractloop)
	{
		if (!NET_IsLocalAddress(adr))
		{
			Com_Printf("Remote connect in attract loop. Ignored.\n");
			Netchan_OutOfBandPrint(NS_SERVER, adr, "print\nConnection refused.\n");
			return;
		}
	}

	// see if the challenge is valid
	if (!NET_IsLocalAddress(adr))
	{
		for (i = 0; i < MAX_CHALLENGES; i++)
		{
			if (NET_CompareBaseAdr(net_from, svs.challenges[i].adr))
			{
				if (challenge == svs.challenges[i].challenge)
					break;		// good

				Netchan_OutOfBandPrint (NS_SERVER, adr, "print\nBad challenge.\n");
				return;
			}
		}

		if (i == MAX_CHALLENGES)
		{
			Netchan_OutOfBandPrint (NS_SERVER, adr, "print\nNo challenge for address.\n");
			return;
		}
	}

	// T3RR0R15T: Locked server. Prevent new players from connecting. (from R1Q2)
	if (sv_locked->value)
	{
		Com_DPrintf ("    server locked.\n");
		Netchan_OutOfBandPrint (NS_SERVER, adr, "print\nServer is locked.\n");
		return;
	}

	// T3RR0R15T: Blocked player names. Prevent players with bad names from connecting.
	Q_strncpyz(blockednames_list, sv_blockednames->string, sizeof(blockednames_list));
	Q_strncpyz(lowercase_name, Info_ValueForKey(userinfo, "name"), sizeof(lowercase_name));
	strip_garbage(lowercase_name, lowercase_name, sizeof(lowercase_name)); // jit - strip out color codes and such.
	strtolower(lowercase_name);
	pname = strtok(blockednames_list, ",");

	while (pname != NULL)
	{
		badname = strstr(lowercase_name, pname);
		
		while(badname != NULL)
		{
			Com_DPrintf ("    bad name.\n");
			Netchan_OutOfBandPrint (NS_SERVER, adr, "print\nDisallowed characters in your name: %s\n", pname);
			return;
		}
			
		pname = strtok(NULL, ",");
	}

	// T3RR0R15T: Reserved slots (from R1Q2)
	pass = Info_ValueForKey(userinfo, "password");

	if (sv_reserved_password->string[0] && !strcmp(pass, sv_reserved_password->string))
	{
		reserved = 0;
		//r1: this prevents mod/admin dll from also checking password as some mods incorrectly
		//refuse if password cvar exists and no password is set on the server. by definition a
		//server with reserved slots should be public anyway.
		Info_RemoveKey (userinfo, "password");
	}
	else
	{
		reserved = (int)sv_reserved_slots->value;
	}

	newcl = &temp;
	memset(newcl, 0, sizeof(client_t));

	// if there is already a slot for this ip, reuse it
	for (i = 0, cl = svs.clients; i < maxclients->value; i++, cl++)
	{
		if (cl->state == cs_free)
			continue;

		if (NET_CompareBaseAdr (adr, cl->netchan.remote_address)
			&& (cl->netchan.qport == qport 
			|| adr.port == cl->netchan.remote_address.port))
		{
			if (!NET_IsLocalAddress(adr) && (svs.realtime - cl->lastconnect) < ((int)sv_reconnect_limit->value * 1000))
			{
				Com_DPrintf("%s:reconnect rejected : too soon\n", NET_AdrToString(adr));
				return;
			}
			Com_Printf("%s:reconnect\n", NET_AdrToString(adr));
			newcl = cl;
			goto gotnewcl;
		}
	}

	// find a client slot
	newcl = NULL;

	for (i = 0, cl = svs.clients; i < ((int)maxclients->value-reserved); i++, cl++)
	{
		if (cl->state == cs_free)
		{
			newcl = cl;
			break;
		}
	}

	// T3RR0R15T: Added reserved slots (from R1Q2)
	if (!newcl)
	{
		if ((int)sv_reserved_slots->value && !reserved)
		{
			Netchan_OutOfBandPrint(NS_SERVER, adr, "print\nServer and reserved slots are full.\n");
			Com_DPrintf("Reserved slots full\n");
		}
		else
		{
			Netchan_OutOfBandPrint(NS_SERVER, adr, "print\nServer is full.\n");
			Com_DPrintf("Server full\n");
		}
		return;
	}

gotnewcl:	
	// build a new connection
	// accept the new client
	// this is the only place a client_t is ever initialized
	*newcl = temp;
	sv_client = newcl;
	edictnum = (newcl - svs.clients) + 1;
	ent = EDICT_NUM(edictnum);
	ent->inuse = false; // jit - sometimes we run into strange situations where a client reconnects, replaces his own slot, and inuse is true, causing things to execute that shouldn't before the connection finishes.
	newcl->edict = ent;
	newcl->challenge = challenge; // save challenge for checksumming
#ifdef USE_DOWNLOAD3 // jitdownload
	newcl->download3_windowsize = DOWNLOAD3_STARTWINDOWSIZE;
#endif

	// get the game a chance to reject this connection or modify the userinfo
	if (!sv.attractloop) // jitdemo - don't call game functions for demo plays
	{
		if (!(ge->ClientConnect(ent, userinfo)))
		{
			if (*Info_ValueForKey(userinfo, "rejmsg")) 
				Netchan_OutOfBandPrint(NS_SERVER, adr, "print\n%s\nConnection refused.\n",  
					Info_ValueForKey(userinfo, "rejmsg"));
			else
				Netchan_OutOfBandPrint(NS_SERVER, adr, "print\nConnection refused.\n");

			Com_DPrintf("Game rejected a connection.\n");
			return;
		}

		// parse some info from the info strings
		Q_strncpyz(newcl->userinfo, userinfo, sizeof(newcl->userinfo) - 1);
		SV_UserinfoChanged(newcl);
	}

	// send the connect packet to the client
	Netchan_OutOfBandPrint(NS_SERVER, adr, "client_connect");
	Netchan_Setup(NS_SERVER, &newcl->netchan, adr, qport);
	newcl->state = cs_connected;
	SZ_Init(&newcl->datagram, newcl->datagram_buf, sizeof(newcl->datagram_buf));
	newcl->datagram.allowoverflow = true;
	newcl->lastmessage = svs.realtime;	// don't timeout
	newcl->lastconnect = svs.realtime;
}


/*
===============
SVC_RemoteCommand

A client issued an rcon command.
Shift down the remaining args
Redirect all printfs
===============
*/
void SVC_RemoteCommand (void)
{
	// === jit - revamped this function to be a little safer
	char		command_str[1024];
	int			command_len = 0;
	qboolean	rcon_set = false;
	qboolean	rcon_valid = false;

	if (strlen(rcon_password->string))
		rcon_set = true;

	if (rcon_set && Q_streq(Cmd_Argv(1), rcon_password->string))
		rcon_valid = true;

	if (net_message.cursize > 4)
		command_len = net_message.cursize - 4;

	if (command_len >= sizeof(command_str))
	{
		assert(0);
		Com_Printf("Large (malicious?) rcon from %s.  Ignoring.\n", NET_AdrToString(net_from));
		return;
	}

	memcpy(command_str, net_message.data + 4, command_len);
	command_str[command_len] = 0;

	if (!rcon_valid)
		Com_Printf("Bad rcon from %s:\n%s\n", NET_AdrToString(net_from), command_str);
	else
		Com_Printf("Rcon from %s:\n%s\n", NET_AdrToString(net_from), command_str);

	Com_BeginRedirect(RD_PACKET, sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect);

	if (!rcon_valid)
	{
		if (!rcon_set)
			Com_Printf("rcon_password not set.\n");
		else
			Com_Printf("Bad rcon_password.\n");
	}
	else
	{
		char *s = command_str;

		if (command_len > 5) // "rcon "
		{
			s += 5;

			// Skip past the password
			while (*s)
			{
				if (*s == ' ')
				{
					++s;
					break;
				}

				++s;
			}

			Cmd_ExecuteString(s);
		}
	}

	Com_EndRedirect();
	// jit ===
}

#ifdef USE_DOWNLOAD3 // jitdownload

// Reads qport from the packet and returns the client it came from,
// or NULL if there are no matches.
client_t *GetClientFromPacket (void)
{
	int i, max, qport;
	client_t *cl;

	qport = MSG_ReadShort(&net_message) & 0xFFFF;
	max = (int)maxclients->value;

	// Find the connected client this came from
	for (i = 0, cl = svs.clients; i < max; i++, cl++)
	{
		if (cl->state == cs_free)
			continue;

		if (!NET_CompareBaseAdr(net_from, cl->netchan.remote_address))
			continue;

		if (cl->netchan.qport != qport)
			continue;

		return cl;
	}

	return NULL;
}


int GetNextDownload3Chunk (client_t *cl)
{
	int i, num_chunks, chunk_to_send = -1, chunk_status, timediff, largest_timediff = 0;
	int realtime = Sys_Milliseconds();

	num_chunks = (cl->downloadsize + (DOWNLOAD3_CHUNKSIZE - 1)) / DOWNLOAD3_CHUNKSIZE;

	for (i = 0; i < num_chunks; ++i)
	{
		chunk_status = cl->download3_chunks[i];

		// Find either a chunk that hasn't been sent yet or the oldest chunk that hasn't been ack'd.
		if (chunk_status == 0)
		{
			chunk_to_send = i;
			break;
		}
		else if (chunk_status != -1)
		{
			timediff = realtime - chunk_status;

			if (timediff > cl->download3_rtt_est * 8.0f + DOWNLOAD3_MINRESENDWAIT)
			{
				// This chunk is REALLY old and will probably never be received, so re-send it.
				chunk_to_send = i;
#ifdef DOWNLOAD3_DEBUG
				Com_Printf("Chunk %d considered dropped after %d ms.  Re-sending.\n", i, timediff);
#endif
				break;
			}
			else if (timediff > largest_timediff)
			{
				largest_timediff = timediff;
				chunk_to_send = i;
			}
		}
	}

	return chunk_to_send;
}


static qboolean Download3RateExceeded (void)
{
	if (sv_fast_download_max_rate->value)
	{
		static int last_rate_check = 0;
		int current_time = Sys_Milliseconds();
		static float rateaverage = 0.0f;

		if (!last_rate_check)
			last_rate_check = current_time;

		if (current_time - last_rate_check > 0)
		{
			float rate; // rate in KB/s

			rate = (float)sv.download3_bytessent / 1024.0f / (float)(current_time - last_rate_check) * 1000.0f;
			rateaverage = rateaverage * 0.5f + rate * 0.5f;
			sv.download3_bytessent = 0;
			last_rate_check = current_time;
			sv.download3_rateexceeded = rateaverage > sv_fast_download_max_rate->value;
#ifdef DOWNLOAD3_DEBUG
			Com_Printf("Rate: %g\n", rateaverage);
#endif

			return sv.download3_rateexceeded;
		}
		else
		{
			return sv.download3_rateexceeded;
		}
	}
	else
	{
		return false; // no max rate
	}
}


void SV_SendDownload3Chunk (client_t *cl, int chunk_to_send)
{
	unsigned char msgbuf[MAX_MSGLEN];
	sizebuf_t message;
	unsigned int offset = chunk_to_send * DOWNLOAD3_CHUNKSIZE;
	int chunksize;
	unsigned int md5sum;
	unsigned short md5sum_short;
	int realtime = Sys_Milliseconds();

	if (!cl->download || offset > cl->downloadsize || chunk_to_send < 0)
		return;

	if (realtime == -1 || realtime == 0) // probably won't ever happen, but just to be safe...
		realtime = 1;

	if (Download3RateExceeded())
	{
		// Fake sending a packet that will later time out to throttle download rate
		cl->download3_chunks[chunk_to_send] = realtime;
		return;
	}

	chunksize = cl->downloadsize - offset;

	if (chunksize > DOWNLOAD3_CHUNKSIZE)
		chunksize = DOWNLOAD3_CHUNKSIZE;

	SZ_Init(&message, msgbuf, sizeof(msgbuf));
	MSG_WriteLong(&message, -1); // out of band packet
	md5sum = Com_MD5Checksum(cl->download + offset, chunksize);
	md5sum_short = (unsigned short)(md5sum & 0xFFFF);
	SZ_Write(&message, "dl3\n", 4);
	MSG_WriteByte(&message, cl->download3_fileid);
	MSG_WriteShort(&message, md5sum_short);
	MSG_WriteLong(&message, chunk_to_send);
	SZ_Write(&message, cl->download + offset, chunksize);
	NET_SendPacket(NS_SERVER, message.cursize, message.data, cl->netchan.remote_address);
	cl->download3_chunks[chunk_to_send] = realtime;
	sv.download3_bytessent += chunksize;
#ifdef DOWNLOAD3_DEBUG
	Com_Printf("DL3SEND %04d\n", chunk_to_send);
#endif
}


static void AddToDownload3Window (client_t *cl, int chunk)
{
	int i, windowsize;
	int num_chunks = (cl->downloadsize + (DOWNLOAD3_CHUNKSIZE - 1)) / DOWNLOAD3_CHUNKSIZE;
	int new_chunk;

	windowsize = cl->download3_windowsize;

	for (i = 0; i < windowsize; ++i)
	{
		if (cl->download3_window[i] == chunk)
		{
			new_chunk = GetNextDownload3Chunk(cl);
			SV_SendDownload3Chunk(cl, new_chunk);
			cl->download3_window[i] = new_chunk;

			if (windowsize < DOWNLOAD3_MAXWINDOWSIZE)
			{
				new_chunk = GetNextDownload3Chunk(cl);
				cl->download3_window[windowsize] = new_chunk;
				SV_SendDownload3Chunk(cl, new_chunk);
				cl->download3_windowsize++;
			}

			return;
		}
	}

#ifdef DOWNLOAD3_ALWAYS_INC_WINDOW
	if (windowsize < DOWNLOAD3_MAXWINDOWSIZE) // testing: always expand the window when a chunk is ack'd.
	{
		new_chunk = GetNextDownload3Chunk(cl);
		cl->download3_window[windowsize] = new_chunk;
		SV_SendDownload3Chunk(cl, new_chunk);
		cl->download3_windowsize++;
	}
#endif

#ifdef DOWNLOAD3_DEBUG
	Com_Printf("Yay! Chunk %d saved from dropped packet sequence!\n", chunk);
#endif
}


static void Download3AckChunk (client_t *cl, int chunk, int num_chunks)
{
	int chunk_status;

	if (chunk == -1)
		return;

	if (chunk >= num_chunks || chunk < 0)
	{
		Com_Printf("Download chunk out of range: %d (%d)\n", chunk, num_chunks);
		//assert(chunk < num_chunks && chunk >= 0);
		return;
	}

	if (cl->download3_active && cl->download3_chunks)
	{
		chunk_status = cl->download3_chunks[chunk];

		if (chunk_status != 0 && chunk_status != -1)
		{
			int realtime = Sys_Milliseconds();
			int timediff = realtime - chunk_status;
			float dev;

			if (cl->download3_rtt_est)
			{
				dev = fabs(timediff - cl->download3_rtt_est);
				cl->download3_rtt_est = (1 - DOWNLOAD3_RTT_ALPHA) * cl->download3_rtt_est + DOWNLOAD3_RTT_ALPHA * (float)timediff;
				
				if (cl->download3_rtt_dev)
				{
					cl->download3_rtt_dev = (1.0f - DOWNLOAD3_RTT_BETA) * cl->download3_rtt_dev + DOWNLOAD3_RTT_BETA * dev;
#ifdef DOWNLOAD3_DEBUG
					//Com_Printf("DL3RTT\t%g\t%g\n", cl->download3_rtt_est, cl->download3_rtt_dev);
#endif
				}
				else
				{
					cl->download3_rtt_dev = dev;
				}
			}
			else
			{
				cl->download3_rtt_est = (float)timediff;
			}

			AddToDownload3Window(cl, chunk);
			cl->download3_chunks[chunk] = -1; // Chunk acknowledged.
#ifdef DOWNLOAD3_DEBUG
			Com_Printf("DL3ACK  %04d\n", chunk);
#endif
		}
	}
}


// Client has sent an acknowledgement of a download3 packet.
void SVC_Download3Ack (void)
{
	int num_chunks, chunk;
	byte fileid;
	client_t *cl = GetClientFromPacket();
	int i;

	if (!cl)
		return;

	if (!cl->download3_active)
		return;

	num_chunks = (cl->downloadsize + (DOWNLOAD3_CHUNKSIZE - 1)) / DOWNLOAD3_CHUNKSIZE;
	fileid = MSG_ReadByte(&net_message);
	chunk = MSG_ReadLong(&net_message);

	if (fileid != cl->download3_fileid)
	{
#ifdef WIN32
		assert(fileid == cl->download3_fileid);
#endif
		Com_Printf("Download fileid mismatch: %d != %d\n", (int)fileid, (int)cl->download3_fileid);
		return;
	}

	Download3AckChunk(cl, chunk, num_chunks);

	for (i = 0; i < DOWNLOAD3_NUMBACKUPACKS; ++i)
	{
		chunk = MSG_ReadLong(&net_message);
		Download3AckChunk(cl, chunk, num_chunks);
	}
}

#endif

/*
=================
SV_ConnectionlessPacket

A connectionless packet has four leading 0xff
characters to distinguish it from a game channel.
Clients that are in the game can still send
connectionless packets.
=================
*/
void SV_ConnectionlessPacket (void)
{
	char *s, *c;

	//if (sv.attractloop) -- oops, guess these are needed to connect to the demo to watch it...
	//{
	//	Com_Printf("Ignored connectionless packet from %s while in attractloop.\n", NET_AdrToString(net_from));
	//	return;
	//}

	// jitsecurity -- fix from Echon.
	// 1024 is the absolute largest, but nothing should be over 600 unless it's malicious.
	if (net_message.cursize > 800)
	{
		Com_Printf("Connectionless packet > 800 bytes from %s.\n", NET_AdrToString(net_from));
		return;
	}

	//r1: make sure we never talk to ourselves
	//if (NET_IsLocalAddress(&net_from) && !NET_IsLocalHost(&net_from) && ShortSwap(net_from.port) == server_port)
	if (net_from.ip[0] == 127 && !(net_from.type == NA_LOOPBACK) && BigShort(net_from.port) == server_port) // jitsecurity
	{
		Com_Printf("Dropped %d byte connectionless packet from self! (spoofing attack?)\n", net_message.cursize);
		return;
	}

	MSG_BeginReading(&net_message);
	MSG_ReadLong(&net_message);		// skip the -1 marker
	s = MSG_ReadStringLine(&net_message);
	Cmd_TokenizeString(s, false);
	c = Cmd_Argv(0);
	Com_DPrintf("Packet %s : %s\n", NET_AdrToString(net_from), c);

	if (Q_streq(c, "ping"))
		SVC_Ping();
	else if (Q_streq(c, "ack"))
		SVC_Ack();
	else if (Q_streq(c, "status"))
		SVC_Status();
	else if (Q_streq(c, "info"))
		SVC_Info();
	else if (Q_streq(c, "getchallenge"))
		SVC_GetChallenge();
	else if (Q_streq(c, "connect"))
		SVC_DirectConnect();
	else if (Q_streq(c, "rcon"))
		SVC_RemoteCommand();
	else if (Q_streq(c, "svheartbeatresponse"))
		SVC_HeartbeatResponse(NET_AdrToString(net_from));
#ifdef USE_DOWNLOAD3 // jitdownload
	else if (Q_streq(c, "dl3ack"))
		SVC_Download3Ack();
#endif
	else
		Com_Printf("Bad connectionless packet from %s:\n%s\n", NET_AdrToString(net_from), s);
}


//============================================================================

/*
===================
SV_CalcPings

Updates the cl->ping variables
===================
*/
void SV_CalcPings (void)
{
	int			i, j;
	client_t	*cl;
	int			total, count;
	struct gclient_s *client; // jitdemo

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl = &svs.clients[i];
		if (cl->state != cs_spawned )
			continue;

#if 0
		if (cl->lastframe > 0)
			cl->frame_latency[sv.framenum&(LATENCY_COUNTS-1)] = sv.framenum - cl->lastframe + 1;
		else
			cl->frame_latency[sv.framenum&(LATENCY_COUNTS-1)] = 0;
#endif

		total = 0;
		count = 0;
		for (j=0 ; j<LATENCY_COUNTS ; j++)
		{
			if (cl->frame_latency[j] > 0)
			{
				count++;
				total += cl->frame_latency[j];
			}
		}
		if (!count)
			cl->ping = 0;
		else
#if 0
			cl->ping = total*100/count - 100;
#else
			cl->ping = total / count;
#endif

		// let the game dll know about the ping
		// === jitdemo - multi-map demo support
		client = cl->edict->client;

		if (client)
			client->ping = cl->ping;
		// jitdemo ===
	}
}


/*
===================
SV_GiveMsec

Every few frames, gives all clients an allotment of milliseconds
for their command moves.  If they exceed it, assume cheating.
===================
*/
void SV_GiveMsec (void)
{
	int			i;
	client_t	*cl;

	if (sv.framenum & 15)
		return;

	for (i = 0; i < maxclients->value; i++)
	{
		cl = &svs.clients[i];

		if (cl->state == cs_free)
			continue;
		
		cl->commandMsec = 1800;		// 1600 + some slop

		if (!(sv.framenum & 255)) // jitspeedhackcheck: about 25 seconds
			cl->commandMsec2 = 25600 + ((int)sv_enforcetime->value) * 10;
	}
}


/*
=================
SV_ReadPackets
=================
*/
void SV_ReadPackets (void)
{
	int			i;
	client_t	*cl;
	int			qport;

	while (NET_GetPacket(NS_SERVER, &net_from, &net_message))
	{
		// check for connectionless packet (0xffffffff) first
		if (*(int*)net_message.data == -1)
		{
			SV_ConnectionlessPacket();
			continue;
		}

		// read the qport out of the message so we can fix up
		// stupid address translating routers
		MSG_BeginReading(&net_message);
		MSG_ReadLong(&net_message);		// sequence number
		MSG_ReadLong(&net_message);		// sequence number
		qport = MSG_ReadShort(&net_message) & 0xffff;

		// check for packets from connected clients
		for (i = 0, cl = svs.clients; i < maxclients->value; i++, cl++)
		{
			if (cl->state == cs_free)
				continue;

			if (!NET_CompareBaseAdr(net_from, cl->netchan.remote_address))
				continue;

			if (cl->netchan.qport != qport)
				continue;

			if (cl->netchan.remote_address.port != net_from.port)
			{
				Com_Printf("SV_ReadPackets: Fixing up a translated port.\n");
				cl->netchan.remote_address.port = net_from.port;
			}

			if (Netchan_Process(&cl->netchan, &net_message))
			{	// this is a valid, sequenced packet, so process it
				if (cl->state != cs_zombie)
				{
					cl->lastmessage = svs.realtime;	// don't timeout
					SV_ExecuteClientMessage(cl);
				}
			}

			break;
		}
		
		if (i != maxclients->value)
			continue;
	}
}

#ifdef USE_DOWNLOAD3 // jitdownload

// Check status of downloads and whatnot
static void SV_HandleDownload3 (void)
{
	int i, max = (int)maxclients->value;
	client_t *cl;
	int realtime = Sys_Milliseconds();
	int timeout_interval;

	if (realtime == -1 || realtime == 0) // probably won't ever happen, but just to be safe...
		realtime = 1;

	for (i = 0; i < max; ++i)
	{
		cl = &svs.clients[i];

		if (cl->download3_active && cl->download3_chunks)
		{
			int window, window_size = cl->download3_windowsize, chunk, chunk_status;

			timeout_interval = (int)(cl->download3_rtt_est + 4.0f * cl->download3_rtt_dev + 1.0f);

			for (window = 0; window < window_size; ++window)
			{
				chunk = cl->download3_window[window];

				if (chunk >= 0)
				{
					chunk_status = cl->download3_chunks[chunk];

					if (chunk_status != 0 && chunk_status != -1)
					{
						if (realtime - chunk_status > timeout_interval)
						{
							// Packet probably dropped.  Halve the window size and send more data
							window_size = (window_size + 1) / 2;
							cl->download3_windowsize = window_size;
#ifdef DOWNLOAD3_DEBUG
							Com_Printf("Chunk %d timed out, halving window size to %d\n", chunk, window_size);
#endif

							for (window = 0; window < window_size; ++window)
							{
								chunk = GetNextDownload3Chunk(cl);
								SV_SendDownload3Chunk(cl, chunk);
								cl->download3_window[window] = chunk;
							}

							break; // break out of the window loop
						}
					}
				}
			}
		}
	}
}
#endif

/*
==================
SV_CheckTimeouts

If a packet has not been received from a client for timeout->value
seconds, drop the conneciton.  Server frames are used instead of
realtime to avoid dropping the local client while debugging.

When a client is normally dropped, the client_t goes into a zombie state
for a few seconds to make sure any final reliable message gets resent
if necessary
==================
*/
void SV_CheckTimeouts (void)
{
	int		i;
	client_t	*cl;
	int			droppoint;
	int			zombiepoint;

	droppoint = svs.realtime - 1000 * timeout->value;
	zombiepoint = svs.realtime - 1000 * zombietime->value;

	for (i = 0, cl = svs.clients; i < maxclients->value; i++, cl++)
	{
		// message times may be wrong across a changelevel
		if (cl->lastmessage > svs.realtime)
			cl->lastmessage = svs.realtime;

		if (cl->state == cs_zombie
			&& cl->lastmessage < zombiepoint)
		{
			cl->state = cs_free;	// can now be reused
			continue;
		}

		if ((cl->state == cs_connected || cl->state == cs_spawned) 
			&& cl->lastmessage < droppoint)
		{
			SV_BroadcastPrintf(PRINT_HIGH, "%s timed out.\n", cl->name);
			SV_DropClient(cl); 
			cl->state = cs_free;	// don't bother with zombie state
		}
	}
}

/*
================
SV_PrepWorldFrame

This has to be done before the world logic, because
player processing happens outside RunWorldFrame
================
*/
void SV_PrepWorldFrame (void)
{
	edict_t	*ent;
	int		i;

	for (i=0 ; i<ge->num_edicts ; i++, ent++)
	{
		ent = EDICT_NUM(i);
		// events only last for a single message
		ent->s.event = 0;
	}

}


void SV_ReplicatePhysicsSettings () // jitmovephysics
{
	pm_oldmovephysics = sv_oldmovephysics->value > 0.0f;
	pm_skyglide_maxvel = sv_skyglide_maxvel->value;
	pm_crouchslidefriction = sv_crouchslide->value ? 1.8f : 0.0f;
	Com_sprintf(sv.configstrings[CS_MOVEPHYSICS], sizeof(sv.configstrings[CS_MOVEPHYSICS]), "MovePhysVer: %d CrouchSlide: %g SkyGlide: %g", pm_oldmovephysics ? 0 : 1, pm_crouchslidefriction, pm_skyglide_maxvel);
	PF_Configstring(CS_MOVEPHYSICS, sv.configstrings[CS_MOVEPHYSICS]); 
	sv_oldmovephysics->modified = false;
	sv_skyglide_maxvel->modified = false;
	sv_crouchslide->modified = false;
}


/*
=================
SV_RunGameFrame
=================
*/
void SV_RunGameFrame (void)
{
	if (sv_oldmovephysics->modified || sv_skyglide_maxvel->modified || sv_crouchslide->modified) // jitmovephysics
		SV_ReplicatePhysicsSettings();

	if (host_speeds->value)
		time_before_game = Sys_Milliseconds();

	// we always need to bump framenum, even if we
	// don't run the world, otherwise the delta
	// compression can get confused when a client
	// has the "current" frame
	sv.framenum++;
	sv.time = sv.framenum * 100;

	// don't run if paused
	//if (!sv_paused->value || maxclients->value > 1)
	//if (!sv.attractloop && (!sv_paused->value || maxclients->value > 1)) // jitdemo - don't run server frames while playing a demo!
	if ((!sv_paused->value || maxclients->value > 1)) // jitdemo - jitest
	{
		ge->RunFrame();

		// never get more than one tic behind
		if (sv.time < svs.realtime)
		{
			if (sv_showclamp->value)
				Com_Printf("sv highclamp\n");

			svs.realtime = sv.time;
		}
	}

	if (host_speeds->value)
		time_after_game = Sys_Milliseconds();
}

/*
==================
SV_Frame

==================
*/
void SV_Frame (int msec)
{
	time_before_game = time_after_game = 0;

	// if server is not active, do nothing
	if (!svs.initialized)
		return;

    svs.realtime += msec;

	// keep the random time dependent
	rand();

	// check timeouts
	SV_CheckTimeouts();

	// jitspeedhackcheck - accumulate server msec on each client
	SV_AddServerMsecToClients(msec);

	// get packets from clients
	SV_ReadPackets();
#ifdef USE_DOWNLOAD3
	SV_HandleDownload3(); // jitdownload
#endif

	// move autonomous things around if enough time has passed
	if (!sv_timedemo->value && svs.realtime < sv.time)
	{
		// never let the time get too far off
		if (sv.time - svs.realtime > 100)
		{
			if (sv_showclamp->value)
				Com_Printf("sv lowclamp\n");

			svs.realtime = sv.time - 100;
		}

		NET_Sleep(sv.time - svs.realtime);
		return;
	}

	// update ping based on the last known frame from all clients
	SV_CalcPings();

	// give the clients some timeslices
	SV_GiveMsec();

	// let everything in the world think and move
	SV_RunGameFrame();

	// send messages back to the clients that had packets read this frame
	SV_SendClientMessages();

	// save the entire world state if recording a serverdemo
	SV_RecordDemoMessage();

	// send a heartbeat to the master if needed
	Master_Heartbeat();

	// clear teleport flags, etc for next frame
	SV_PrepWorldFrame();
}

//============================================================================

#define	HEARTBEAT_SECONDS	300

// Force a heartbeat to the master server in X seconds
void Force_Master_Heartbeat (int seconds)
{
	svs.last_heartbeat = svs.realtime - HEARTBEAT_SECONDS * 1000 + seconds * 1000;
}

/*
================
Master_Heartbeat

Send a message to the master every few minutes to
let it know we are alive, and log information
================
*/
void Master_Heartbeat (void)
{
	char		*string;
	int			i;

	if (!dedicated || !dedicated->value || !public_server || !public_server->value)
		return;		// only dedicated public servers send heartbeats

	// check for time wraparound
	if (svs.last_heartbeat > svs.realtime)
		svs.last_heartbeat = svs.realtime;

	if (svs.realtime - svs.last_heartbeat < HEARTBEAT_SECONDS*1000)
		return;		// not time to send yet

	svs.last_heartbeat = svs.realtime;

	// send the same string that we would give for a status OOB command
	string = SV_StatusString();

	// send to group master
	for (i = 0; i < MAX_MASTERS; i++)
	{
		if (master_adr[i].port)
		{
			Com_Printf("Sending heartbeat to %s.\n", NET_AdrToString(master_adr[i]));
			Netchan_OutOfBandPrint(NS_SERVER, master_adr[i], "heartbeat\n%s", string);
		}
	}
}

/*
=================
Master_Shutdown

Informs all masters that this server is going down
=================
*/
void Master_Shutdown (void)
{
	int			i;

	// pgm post3.19 change, cvar pointer not validated before dereferencing
	if (!dedicated || !dedicated->value)
		return;		// only dedicated servers send heartbeats

	// pgm post3.19 change, cvar pointer not validated before dereferencing
	if (!public_server || !public_server->value)
		return;		// a private dedicated game

	// send to group master
	for (i = 0; i < MAX_MASTERS; i++)
	{
		if (master_adr[i].port)
		{
			if (i > 0)
				Com_Printf("Sending heartbeat to %s.\n", NET_AdrToString(master_adr[i]));

			Netchan_OutOfBandPrint(NS_SERVER, master_adr[i], "shutdown");
		}
	}
}

//============================================================================


/*
=================
SV_UserinfoChanged

Pull specific info from a newly changed userinfo string
into a more C freindly form.
=================
*/
void SV_UserinfoChanged (client_t *cl)
{
	char	*val;
	int		i;

	// call game code to allow overrides
	ge->ClientUserinfoChanged(cl->edict, cl->userinfo);
	Q_strncpyzna(cl->name, Info_ValueForKey(cl->userinfo, "name"), sizeof(cl->name));

	// rate command
	val = Info_ValueForKey(cl->userinfo, "rate");

	if (strlen(val))
	{
		i = atoi(val);
		cl->rate = i;

		if (cl->rate < 3000) // jit, changed minimum from 100 to 3000
			cl->rate = 3000;

		if (cl->rate > 25000) // jit, changed max to 25000 from 15000
			cl->rate = 25000;
	}
	else
	{
		cl->rate = 15000; // jit - default to 15000 instead of 5000
	}

	// msg command
	val = Info_ValueForKey (cl->userinfo, "msg");

	if (strlen(val))
		cl->messagelevel = atoi(val);
}


//============================================================================

/*
===============
SV_Init

Only called at quake2.exe startup, not for each game
===============
*/
void SV_Init (void)
{
	SV_InitOperatorCommands();

	rcon_password = Cvar_Get("rcon_password", "", 0);
	Cvar_Get("skill", "1", 0);
	Cvar_Get("deathmatch", "1", CVAR_LATCH);
	Cvar_Get("coop", "0", CVAR_LATCH);
	Cvar_Get("dmflags", va("%i", DF_INSTANT_ITEMS), 0); // jit, removed serverinfo flag
	Cvar_Get("fraglimit", "50", CVAR_SERVERINFO); // jit, was 0
	Cvar_Get("timelimit", "10", CVAR_SERVERINFO); // jit, was 0
	Cvar_Get("cheats", "0", CVAR_LATCH); // jit, removed serverinfo flag
	Cvar_Get("protocol", va("%i", PROTOCOL_VERSION), CVAR_SERVERINFO|CVAR_NOSET);
	maxclients     = Cvar_Get("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH);
	hostname       = Cvar_Get("hostname", "Paintball 2.0 (build " BUILD_S ")", CVAR_SERVERINFO | CVAR_ARCHIVE);
	timeout        = Cvar_Get("timeout", "30", 0); // jittimeout - 30 secounds should be plenty
	zombietime     = Cvar_Get("zombietime", "2", 0);
	sv_showclamp   = Cvar_Get("showclamp", "0", 0);
	sv_paused      = Cvar_Get("paused", "0", 0);
	sv_timedemo    = Cvar_Get("timedemo", "0", 0);
	sv_enforcetime = Cvar_Get("sv_enforcetime", "0", 0); // 1.831 - disabled because of problems. "240", 0); // jitspeedhackcheck
	sv_enforcetime2= Cvar_Get("sv_enforcetime2", "1", 0); // jitspeedhackcheck - threshold, in seconds, before somebody is kicked for time discrepancy
	sv_cullentities= Cvar_Get("sv_cullentities", "0", 0);
	sv_noextascii  = Cvar_Get("sv_noextascii", "1", 0); // jit
	sv_locked	   = Cvar_Get("sv_locked", "0", 0);		// T3RR0R15T: Locked server. Prevent new players from connecting. (from R1Q2)
	sv_blockednames         = Cvar_Get("sv_blockednames", "rape,fuck,fuc k,fuq,phuck,fukc,shit,sh!t,sh1t,dick,d ick,bitch,whore,cock,fag,nigg,pussy,cunt,slut,asshole,prick,sieg heil,hitler,arsch,arschloch,fick,fotze,muschi,schwuchtel,schwutte,spast,spacko,scheise,scheisse,pisser,fehlgeburt,nazi,gay,bastard,hure,nutte", 0);			// T3RR0R15T: Blocked player names. Prevent players with bad names from connecting.
	sv_reserved_slots		= Cvar_Get("sv_reserved_slots", "0", CVAR_LATCH);	// T3RR0R15T: Number of reserved player slots. (from R1Q2)
	sv_reserved_password	= Cvar_Get("sv_reserved_password", "", 0);			// T3RR0R15T: Password required to access a reserved player slot. Clients should set their 'password' cvar to this. (from R1Q2)
	sv_consolename			= Cvar_Get("sv_consolename", "5", 0);				// T3RR0R15T: Name for console say messages (jit - 5 = "Admin")
	allow_download          = Cvar_Get("allow_download", "1", CVAR_ARCHIVE);
	allow_download_players  = Cvar_Get("allow_download_players", "1", CVAR_ARCHIVE); // jit, default to 1
	allow_download_models   = Cvar_Get("allow_download_models", "1", CVAR_ARCHIVE);
	allow_download_sounds   = Cvar_Get("allow_download_sounds", "1", CVAR_ARCHIVE);
	allow_download_maps	    = Cvar_Get("allow_download_maps", "1", CVAR_ARCHIVE);
#ifdef USE_DOWNLOAD3
	sv_fast_download		= Cvar_Get("sv_fast_download", "1", 0); // jitdownload
	sv_fast_download_max_rate = Cvar_Get("sv_fast_download_max_rate", "0", 0);
#endif

	sv_noreload = Cvar_Get("sv_noreload", "0", 0);
	sv_airaccelerate = Cvar_Get("sv_airaccelerate", "0", CVAR_LATCH);
	public_server = Cvar_Get("public", "1", 0);
	sv_reconnect_limit = Cvar_Get("sv_reconnect_limit", "3", CVAR_ARCHIVE);
	SZ_Init(&net_message, net_message_buffer, sizeof(net_message_buffer));
}

/*
==================
SV_FinalMessage

Used by SV_Shutdown to send a final message to all
connected clients before the server goes down.  The messages are sent immediately,
not just stuck on the outgoing message list, because the server is going
to totally exit after returning from this function.
==================
*/
void SV_FinalMessage (char *message, qboolean reconnect)
{
	int			i;
	client_t	*cl;
	
	SZ_Clear (&net_message);
	MSG_WriteByte (&net_message, svc_print);
	MSG_WriteByte (&net_message, PRINT_HIGH);
	MSG_WriteString (&net_message, message);

	if (reconnect)
		MSG_WriteByte (&net_message, svc_reconnect);
	else
		MSG_WriteByte (&net_message, svc_disconnect);

	// send it twice
	// stagger the packets to crutch operating system limited buffers

	for (i=0, cl = svs.clients ; i<maxclients->value ; i++, cl++)
		if (cl->state >= cs_connected)
			Netchan_Transmit (&cl->netchan, net_message.cursize
			, net_message.data);

	for (i=0, cl = svs.clients ; i<maxclients->value ; i++, cl++)
		if (cl->state >= cs_connected)
			Netchan_Transmit (&cl->netchan, net_message.cursize
			, net_message.data);
}



/*
================
SV_Shutdown

Called when each game quits,
before Sys_Quit or Sys_Error
================
*/
void SV_Shutdown (char *finalmsg, qboolean reconnect)
{
	if (svs.clients)
		SV_FinalMessage(finalmsg, reconnect);

	Master_Shutdown();
	SV_ShutdownGameProgs();

	// free current level
	if (sv.demofile)
		fclose(sv.demofile);

	memset(&sv, 0, sizeof(sv));
	Com_SetServerState (sv.state);

	// free server static data
	if (svs.clients)
		Z_Free(svs.clients);

	if (svs.client_entities)
		Z_Free(svs.client_entities);

	if (svs.demofile)
		fclose(svs.demofile);

	memset(&svs, 0, sizeof(svs));
}