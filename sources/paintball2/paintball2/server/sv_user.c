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
// sv_user.c -- server code for moving users

#include "server.h"

edict_t	*sv_player;

/*
============================================================

USER STRINGCMD EXECUTION

sv_client and sv_player will be valid.
============================================================
*/

/*
==================
SV_BeginDemoServer
==================
*/
void SV_BeginDemoserver (void)
{
	char		name[MAX_OSPATH];

	Com_sprintf(name, sizeof(name), "demos/%s", sv.name);
	FS_FOpenFile(name, &sv.demofile);

	if (!sv.demofile)
		Com_Error(ERR_DROP, "Couldn't open %s.\n", name);
}


#ifdef USE_DOWNLOAD3
// Hopefully this can be standardized somewhat - send server extensions to the client
void SV_WriteExtensions (sizebuf_t *message)
{
	char extensions[MAX_INFO_STRING] = "";
	char cmd[1024];

	if (sv_fast_download->value)
		Info_SetValueForKey(extensions, "download3", va("%d", (int)svc_download3start));

	Com_sprintf(cmd, sizeof(cmd), "svextensions \"%s\"\n", extensions);
	MSG_WriteByte(message, svc_stufftext);
	MSG_WriteString(message, cmd);
}
#endif


/*
================
SV_New_f

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each server load.
================
*/
void SV_New_f (void)
{
	char		*gamedir;
	int			playernum;
	edict_t		*ent;

	Com_DPrintf("New() from %s.\n", sv_client->name);

	if (sv_client->state != cs_connected)
	{
		Com_Printf("New not valid -- already spawned.\n");
		return;
	}

	// demo servers just dump the file message
	if (sv.state == ss_demo)
	{
		// === jitdemo - multi-map demo support (not sure if this is correct)
		if (sv.demofile) // already playing a demo
			return;
		// jitdemo ===

		SV_BeginDemoserver();
		return;
	}

	// serverdata needs to go over for all types of servers
	// to make sure the protocol is right, and to set the gamedir
	gamedir = Cvar_VariableString("gamedir");

	// send the serverdata
	MSG_WriteByte(&sv_client->netchan.message, svc_serverdata);
	MSG_WriteLong(&sv_client->netchan.message, PROTOCOL_VERSION);
	MSG_WriteLong(&sv_client->netchan.message, svs.spawncount);
	MSG_WriteByte(&sv_client->netchan.message, sv.attractloop);
	MSG_WriteString(&sv_client->netchan.message, gamedir);

	if (sv.state == ss_cinematic || sv.state == ss_pic)
		playernum = -1;
	else
		playernum = sv_client - svs.clients;

	MSG_WriteShort(&sv_client->netchan.message, playernum);

	// send full levelname
	MSG_WriteString(&sv_client->netchan.message, sv.configstrings[CS_NAME]);

	// game server
	if (sv.state == ss_game)
	{
		// set up the entity for the client
		ent = EDICT_NUM(playernum+1);
		ent->s.number = playernum+1;
		sv_client->edict = ent;
		memset(&sv_client->lastcmd, 0, sizeof(sv_client->lastcmd));

#ifdef USE_DOWNLOAD3
		SV_WriteExtensions(&sv_client->netchan.message);
#endif

		// begin fetching configstrings
		MSG_WriteByte(&sv_client->netchan.message, svc_stufftext);
		MSG_WriteString(&sv_client->netchan.message, va("cmd configstrings %i 0\n", svs.spawncount));
	}
}


/*
==================
SV_Configstrings_f
==================
*/
void SV_Configstrings_f (void)
{
	int start;

	Com_DPrintf("Configstrings() from %s.\n", sv_client->name);

	if (sv_client->state != cs_connected)
	{
		Com_Printf("Configstrings not valid -- already spawned.\n");
		return;
	}

	// handle the case of a level changing while a client was connecting
	if (atoi(Cmd_Argv(1)) != svs.spawncount && !sv.attractloop) // jitdemo - multi-map demo support
	{
		Com_Printf("SV_Configstrings_f from different level.\n");
		SV_New_f();
		return;
	}

	start = atoi(Cmd_Argv(2));

	if (start < 0) // jitsecurity, fix by [SkulleR]
		start = 0;

	// write a packet full of data
	while (sv_client->netchan.message.cursize < MAX_MSGLEN*0.5 
		&& start < MAX_CONFIGSTRINGS)
	{
		if (sv.configstrings[start][0])
		{
			MSG_WriteByte(&sv_client->netchan.message, svc_configstring);
			MSG_WriteShort(&sv_client->netchan.message, start);
			MSG_WriteString(&sv_client->netchan.message, sv.configstrings[start]);
		}

		start++;
	}

	// send next command
	if (start == MAX_CONFIGSTRINGS)
	{
		MSG_WriteByte(&sv_client->netchan.message, svc_stufftext);
		MSG_WriteString(&sv_client->netchan.message, va("cmd baselines %i 0\n", svs.spawncount));
	}
	else
	{
		MSG_WriteByte(&sv_client->netchan.message, svc_stufftext);
		MSG_WriteString(&sv_client->netchan.message, va("cmd configstrings %i %i\n", svs.spawncount, start));
	}
}

/*
==================
SV_Baselines_f
==================
*/
void SV_Baselines_f (void)
{
	int		start;
	entity_state_t	nullstate;
	entity_state_t	*base;

	Com_DPrintf("Baselines() from %s.\n", sv_client->name);

	if (sv_client->state != cs_connected)
	{
		Com_Printf("Baselines not valid -- already spawned.\n");
		return;
	}
	
	// handle the case of a level changing while a client was connecting
	if (atoi(Cmd_Argv(1)) != svs.spawncount && !sv.attractloop) // jitdemo - multi-map support
	{
		Com_Printf("SV_Baselines_f from different level.\n");
		SV_New_f();
		return;
	}
	
	start = atoi(Cmd_Argv(2));

	if (start < 0) // jitsecurity, fix by [SkulleR]
		start = 0;

	memset(&nullstate, 0, sizeof(nullstate));

	// write a packet full of data
	while ((sv_client->netchan.message.cursize <  MAX_MSGLEN*0.5) && (start < MAX_EDICTS))
	{
		base = &sv.baselines[start];

		if (base->modelindex || base->sound || base->effects)
		{
			MSG_WriteByte(&sv_client->netchan.message, svc_spawnbaseline);
			MSG_WriteDeltaEntity(&nullstate, base, &sv_client->netchan.message, true, true);
		}

		start++;
	}

	// send next command

	if (start == MAX_EDICTS)
	{
		MSG_WriteByte(&sv_client->netchan.message, svc_stufftext);
		MSG_WriteString(&sv_client->netchan.message, va("precache %i\n", svs.spawncount));
	}
	else
	{
		MSG_WriteByte(&sv_client->netchan.message, svc_stufftext);
		MSG_WriteString(&sv_client->netchan.message, va("cmd baselines %i %i\n", svs.spawncount, start));
	}
}

/*
==================
SV_Begin_f
==================
*/
void SV_Begin_f (void)
{
	Com_DPrintf("Begin() from %s.\n", sv_client->name);

	// handle the case of a level changing while a client was connecting
	if (atoi(Cmd_Argv(1)) != svs.spawncount)
	{
		Com_Printf("SV_Begin_f from different level.\n");
		SV_New_f();
		return;
	}

	sv_client->state = cs_spawned;
	
	// call the game begin function
	if (!sv.attractloop) // jitdemo - multi-map demo support
		ge->ClientBegin(sv_player);

	Cbuf_InsertFromDefer();
}

//=============================================================================

/*
==================
SV_NextDownload_f
==================
*/
void SV_NextDownload_f (void)
{
	int		r;
	int		percent;
	int		size;

	if (!sv_client->download)
		return;

	r = sv_client->downloadsize - sv_client->downloadcount;

	if (r > 1024)
		r = 1024;

	MSG_WriteByte(&sv_client->netchan.message, svc_download);
	MSG_WriteShort(&sv_client->netchan.message, r);
	sv_client->downloadcount += r;
	size = sv_client->downloadsize;

	if (!size)
		size = 1;

	percent = sv_client->downloadcount * 100 / size;
	MSG_WriteByte(&sv_client->netchan.message, percent);
	SZ_Write(&sv_client->netchan.message, sv_client->download + sv_client->downloadcount - r, r);

	if (sv_client->downloadcount != sv_client->downloadsize)
		return;

	FS_FreeFile(sv_client->download);
	sv_client->download = NULL;
}


static qboolean CheckDownloadFilename (const char *name) // jitsecurity
{
	extern	cvar_t *allow_download;
	extern	cvar_t *allow_download_players;
	extern	cvar_t *allow_download_models;
	extern	cvar_t *allow_download_sounds;
	extern	cvar_t *allow_download_maps;

	if (strstr(name, "..")
		|| !allow_download->value
		// leading dot is no good
		|| *name == '.' 
		// leading slash bad as well, must be in subdir
		|| *name == '/'
		// jitsecurity, leading backslash is bad on WIN32
		|| *name == '\\'
		// next up, skin check
		|| (strncmp(name, "players/", 8) == 0 && !allow_download_players->value)
		// now models
		|| (strncmp(name, "models/", 7) == 0 && !allow_download_models->value)
		// now sounds
		|| (strncmp(name, "sound/", 6) == 0 && !allow_download_sounds->value)
		// now maps (note special case for maps, must not be in pak)
		|| (strncmp(name, "maps/", 5) == 0 && !allow_download_maps->value)
		// MUST be in a subdirectory	
		|| !strstr(name, "/")
		// **** NiceAss: Ends in a backslash. Linux server crash protection--Start ****
		|| name[strlen(name) - 1] == '/'
		// **** NiceAss: Ends in a backslash. Linux server crash protection--End ****
#ifdef WIN32
		|| (strnicmp(name, "config", 6) == 0) // jitsecurity - don't allow downloads from configs dir.
#else
		|| (strncasecmp(name, "config", 6) == 0) // jitsecurity - don't allow downloads from configs dir.
#endif
		|| PathContainsInvalidCharacters(name) // jitdownload
		)
	{
		return false;
	}
	else
	{
		return true;
	}
}

/*
==================
SV_BeginDownload_f
==================
*/
void SV_BeginDownload_f (void)
{
	char	*name;
	int offset = 0;

	name = Cmd_Argv(1);

	if (Cmd_Argc() > 2)
		offset = atoi(Cmd_Argv(2)); // downloaded offset

	// don't allow anything with .. path
	if (!CheckDownloadFilename(name))
	{
		MSG_WriteByte(&sv_client->netchan.message, svc_download);
		MSG_WriteShort(&sv_client->netchan.message, -1);
		MSG_WriteByte(&sv_client->netchan.message, 0);
		return;
	}

	if (sv_client->download)
		FS_FreeFile (sv_client->download);

	sv_client->downloadsize = FS_LoadFile(name, (void **)&sv_client->download);
	sv_client->downloadcount = offset;

	if (offset > sv_client->downloadsize)
		sv_client->downloadcount = sv_client->downloadsize;

	if (!sv_client->download)
	{
		Com_DPrintf("Couldn't download %s to %s.\n", name, sv_client->name);
		MSG_WriteByte(&sv_client->netchan.message, svc_download);
		MSG_WriteShort(&sv_client->netchan.message, -1);
		MSG_WriteByte(&sv_client->netchan.message, 0);
		return;
	}

	SV_NextDownload_f();
	Com_DPrintf("Downloading %s to %s.\n", name, sv_client->name);
}


#ifdef USE_DOWNLOAD3
static long LoadFileOrAlternate (const char *filename, char *filename_alt, size_t filename_alt_len, void **data_ptr)
{
	int size;
	const char *sExt;
	char filename_temp[MAX_QPATH];
	char filename_noext[MAX_QPATH];

	Q_strncpyz(filename_temp, filename, sizeof(filename_temp));
	COM_StripExtension(filename_temp, filename_noext, sizeof(filename_noext));

	// Check for files in this order: png, jpg, tga, pcx, wal
	while ((size = FS_LoadFile(filename_temp, data_ptr)) < 0)
	{
		sExt = COM_FileExtension(filename_temp);

		if (Q_strcasecmp(sExt, "png") == 0)
		{
			Com_sprintf(filename_temp, sizeof(filename_temp), "%s.jpg", filename_noext);
		}
		else if (Q_strcasecmp(sExt, "jpg") == 0)
		{
			Com_sprintf(filename_temp, sizeof(filename_temp), "%s.tga", filename_noext);
		}
		else if (Q_strcasecmp(sExt, "tga") == 0)
		{
			Com_sprintf(filename_temp, sizeof(filename_temp), "%s.pcx", filename_noext);
		}
		else if (Q_strcasecmp(sExt, "pcx") == 0)
		{
			Com_sprintf(filename_temp, sizeof(filename_temp), "%s.wal", filename_noext);
		}
		else if (Q_strcasecmp(sExt, "skm") == 0)
		{
			Com_sprintf(filename_temp, sizeof(filename_temp), "%s.md2", filename_noext);
		}
		else if (Q_strcasecmp(sExt, "md3") == 0)
		{
			Com_sprintf(filename_temp, sizeof(filename_temp), "%s.md2", filename_noext);
		}
		else
		{
			char *s;
			
			Q_strncpyz(filename_temp, filename, sizeof(filename_temp));

			// if we don't have the high-res version, check for the regular version.
			if ((s = strstr(filename_temp, "/hr4/")))
			{
				memmove(s, s + 4, strlen(s + 3));
				return LoadFileOrAlternate(filename_temp, filename_alt, filename_alt_len, data_ptr);
			}

			return -1;
		}
	}

	Q_strncpyz(filename_alt, filename_temp, filename_alt_len);
	return size;
}


void SV_BeginDownload3_f (void) // jitdownload
{
	char *name;
	int num_chunks;
	char download_filename[MAX_QPATH];
	unsigned int md5sum;

	name = Cmd_Argv(1);
	SV_FreeDownloadData(sv_client); // make sure we stop any current downloads

	// don't allow anything with .. path
	// don't download if sv_fast_download is disabled
	if (!CheckDownloadFilename(name) || !sv_fast_download->value)
	{
		MSG_WriteByte(&sv_client->netchan.message, svc_download);
		MSG_WriteShort(&sv_client->netchan.message, -1);
		MSG_WriteByte(&sv_client->netchan.message, 0);
		return;
	}

	sv_client->downloadsize = LoadFileOrAlternate(name, download_filename, sizeof(download_filename), (void **)&sv_client->download);

	if (!sv_client->download)
	{
		Com_DPrintf("Couldn't download %s to %s.\n", download_filename, sv_client->name);
		MSG_WriteByte(&sv_client->netchan.message, svc_download);
		MSG_WriteShort(&sv_client->netchan.message, -1);
		MSG_WriteByte(&sv_client->netchan.message, 0);
		return;
	}

	if (sv_client->downloadsize == -1) // file failed to open
	{
		MSG_WriteByte(&sv_client->netchan.message, svc_download);
		MSG_WriteShort(&sv_client->netchan.message, -1);
		MSG_WriteByte(&sv_client->netchan.message, 0);
		return;
	}

	num_chunks = (sv_client->downloadsize + (DOWNLOAD3_CHUNKSIZE - 1)) / DOWNLOAD3_CHUNKSIZE;
	sv_client->download3_chunks = Z_Malloc(num_chunks * sizeof(int));
	assert(sv_client->download3_chunks);
	memset(sv_client->download3_chunks, 0, num_chunks * sizeof(int));
	sv_client->download3_window = Z_Malloc(DOWNLOAD3_MAXWINDOWSIZE * sizeof(int));
	assert(sv_client->download3_window);
	sv_client->downloadcount = 0;
	md5sum = Com_MD5Checksum(sv_client->download, sv_client->downloadsize);
	sv_client->download3_fileid = sv.download3_nextfileid;
	sv.download3_nextfileid++;
	MSG_WriteByte(&sv_client->netchan.message, svc_download3start); // acknowledge download request
	MSG_WriteByte(&sv_client->netchan.message, sv_client->download3_fileid);
	MSG_WriteLong(&sv_client->netchan.message, sv_client->downloadsize); // tell client filesize
	MSG_WriteByte(&sv_client->netchan.message, 0); // tell client which compression algorithm to use (0 = none)
	MSG_WriteString(&sv_client->netchan.message, download_filename); // tell client what filename should be.
	MSG_WriteLong(&sv_client->netchan.message, md5sum); // md5 checksum of the file, for validation.
	Com_DPrintf("Downloading %s to %s.\n", download_filename, sv_client->name);
}


void SV_CompleteDownload3_f (void)
{
	int fileid;

	if (!sv_client->download)
		return;

	fileid = atoi(Cmd_Argv(1));

	if (fileid != sv_client->download3_fileid)
	{
#ifdef WIN32
		assert(fileid == sv_client->download3_fileid);
#endif
		Com_Printf("%s: dl3complete fileid %d != %d\n", sv_client->name, fileid, sv_client->download3_fileid);
		return;
	}

	SV_FreeDownloadData(sv_client);
}


void SV_ConfirmDownload3_f (void)
{
	int i, start_chunk, chunk;
	int num_chunks = (sv_client->downloadsize + (DOWNLOAD3_CHUNKSIZE - 1)) / DOWNLOAD3_CHUNKSIZE;
	int client_fileid;

	start_chunk = atoi(Cmd_Argv(1));
	client_fileid = atoi(Cmd_Argv(2));

	if (client_fileid != sv_client->download3_fileid)
	{
#ifdef WIN32
		assert(client_fileid == sv_client->download3_fileid);
#endif
		Com_Printf("Client sent dl3ack for fileid %d when server was on fileid %d.\n", client_fileid, sv_client->download3_fileid);
		return;
	}

	if (start_chunk < 0) // Client refused the download
	{
		SV_CompleteDownload3_f();
		return;
	}

	if (start_chunk >= num_chunks)
		start_chunk = num_chunks - 1;

	for (i = 0; i < start_chunk; ++i)
		sv_client->download3_chunks[i] = -1;

	if (sv_client->download3_windowsize > num_chunks)
		sv_client->download3_windowsize = num_chunks; // more than this and we're just wasting packets

	if (sv_client->download3_windowsize < 1)
		sv_client->download3_windowsize = 1;

	sv_client->download3_active = true;

	for (i = 0; i < sv_client->download3_windowsize; ++i)
	{
		chunk = GetNextDownload3Chunk(sv_client);
		SV_SendDownload3Chunk(sv_client, chunk);
		sv_client->download3_window[i] = chunk;
	}
}

void SV_ExtensionsDummy_f (void)
{
	// the svextensions command may get passed back to the server if the client doesn't support it.  Just ignore it.
}

void SV_ClientExtensions_f (void)
{
	// Nothing here, yet...
}
#endif

//============================================================================


/*
=================
SV_Disconnect_f

The client is going to disconnect, so remove the connection immediately
=================
*/
void SV_Disconnect_f (void)
{
	SV_DropClient(sv_client);	
}

/*
==================
SV_ShowServerinfo_f

Dumps the serverinfo info string
==================
*/
/*void SV_ShowServerinfo_f (void)
{
	Info_Print(Cvar_Serverinfo());
}*/

static void SV_ShowServerinfo_f (void) // jit - from R1CH
{
	char	*s;
	char	*p;
	int		flip;

	s = Cvar_Serverinfo();

	//skip beginning \\ char
	s++;
	flip = 0;
	p = s;

	//make it more readable
	while (*p)
	{
		if (*p == '\\')
		{
			if (flip)
				*p = '\n';
			else
				*p = '=';

			flip ^= 1;
		}

		p++;
	}

	SV_ClientPrintf(sv_client, PRINT_HIGH, "%s\n", s);
}

void SV_Nextserver (void)
{
	char	*v;

	//ZOID, ss_pic can be nextserver'd in coop mode
	if (sv.state == ss_game || (sv.state == ss_pic && !Cvar_VariableValue("coop")))
		return;		// can't nextserver while playing a normal game

	svs.spawncount++;	// make sure another doesn't sneak in
	v = Cvar_VariableString("nextserver");

	if (!v[0])
	{
		if (!sv.demofile) // jitdemo - multi-map demo support not playing a demo
			Cbuf_AddText("killserver\n");
	}
	else
	{
		Cbuf_AddText(v);
		Cbuf_AddText("\n");
	}

	Cvar_Set("nextserver", "");
}

/*
==================
SV_Nextserver_f

A cinematic has completed or been aborted by a client, so move
to the next server,
==================
*/
void SV_Nextserver_f (void)
{
	if (atoi(Cmd_Argv(1)) != svs.spawncount)
	{
		Com_DPrintf("Nextserver() from wrong level, from %s.\n", sv_client->name);
		return;		// leftover from last server
	}

	Com_DPrintf("Nextserver() from %s.\n", sv_client->name);
	SV_Nextserver();
}

typedef struct
{
	char	*name;
	void	(*func) (void);
} ucmd_t;

ucmd_t ucmds[] =
{
	// auto issued
	{"new", SV_New_f},
	{"configstrings", SV_Configstrings_f},
	{"baselines", SV_Baselines_f},
	{"begin", SV_Begin_f},

	{"nextserver", SV_Nextserver_f},

	{"disconnect", SV_Disconnect_f},

	// issued by hand at client consoles	
	{"info", SV_ShowServerinfo_f},

	{"download", SV_BeginDownload_f},
	{"nextdl", SV_NextDownload_f},
#ifdef USE_DOWNLOAD3 // jitdownload
	{"download3", SV_BeginDownload3_f},
	{"dl3confirm", SV_ConfirmDownload3_f},
	{"dl3complete", SV_CompleteDownload3_f},
	{"svextensions", SV_ExtensionsDummy_f}, // for clients that don't support extensions
	{"clextensions", SV_ClientExtensions_f},
#endif

	{NULL, NULL}
};

/*
==================
SV_ExecuteUserCommand
==================
*/
void SV_ExecuteUserCommand (char *s)
{
	ucmd_t	*u;
	
	Cmd_TokenizeString((unsigned char *)s, false); // jitspoe -- bug fix from Redix
	sv_player = sv_client->edict;

	for (u = ucmds; u->name; u++)
	{
		if (Q_streq(Cmd_Argv(0), u->name))
		{
			u->func();
			break;
		}
	}

	if (!u->name && sv.state == ss_game)
		ge->ClientCommand(sv_player);
}

/*
===========================================================================

USER CMD EXECUTION

===========================================================================
*/

void SV_ClientThink (client_t *cl, usercmd_t *cmd)
{
	cl->commandMsec -= cmd->msec;

	// === jitspeedhackcheck
	if (cl->state == cs_spawned)
		cl->commandMsecTotal += cmd->msec;

	if (sv_enforcetime2->value)
	{
		// If the accumulated msecs from the client is greater than the accumulated msecs from the server (+ some configurable leeway),
		// he's probably using some sort of speed cheat.
		if (cl->commandMsecTotal > cl->commandMsecServerTotal + (int)(sv_enforcetime2->value * 1000.0f))
		{
			SV_BroadcastPrintf(PRINT_HIGH, "%s kicked for time discrepancy 2.\n", cl->name);
			SV_DropClient(cl);
		}
	}

	if (cl->commandMsec2) // wait 'til this value gets initialized before subtracting
	{
		cl->commandMsec2 -= cmd->msec;

		if (!cl->commandMsec2) // rare case where it's exactly 0 after subtraction.
			cl->commandMsec2--; // they're already below the threshold, so go ahead and kick 'em.
	}

	if (sv_enforcetime->value > 1.0f && cl->commandMsec2 < 0)
	{
		SV_BroadcastPrintf(PRINT_HIGH, "%s kicked for time discrepancy.\n", cl->name);
		SV_DropClient(cl);
	}
	// jitspeedhackcheck ===

	//SV_BroadcastPrintf(PRINT_HIGH, "%s discrepancy: %d\n", cl->name, cl->commandMsec2); // testing

	if (cl->commandMsec < 0)
	{
		Com_DPrintf("commandMsec underflow from %s.\n", cl->name);

		if (sv_enforcetime->value == 1.0f)
			return; // old-style sv_enforcetime
	}

	ge->ClientThink(cl->edict, cmd);
}


#define	MAX_STRINGCMDS	8
/*
===================
SV_ExecuteClientMessage

The current net_message is parsed for the given client
===================
*/
void SV_ExecuteClientMessage (client_t *cl)
{
	int		c;
	char	*s;

	usercmd_t	nullcmd;
	usercmd_t	oldest, oldcmd, newcmd;
	int		net_drop;
	int		stringCmdCount;
	int		checksum, calculatedChecksum;
	int		checksumIndex;
	qboolean	move_issued;
	int		lastframe;

	// === jitdemo - multi-map demo support (not sure if this is correct)
	//if (sv.attractloop)
	//	return;
	// jitdemo ===

	sv_client = cl;
	sv_player = sv_client->edict;

	// only allow one move command
	move_issued = false;
	stringCmdCount = 0;

	// allow game dll to access client packets directly (could come in handy)
	if (geClientPacket && !sv.attractloop) // jitclpacket
		geClientPacket(cl->edict, &net_message);

	while (1)
	{
		if (net_message.readcount > net_message.cursize)
		{
			Com_Printf("SV_ReadClientMessage: badread\n");
			SV_DropClient (cl);
			return;
		}	

		c = MSG_ReadByte(&net_message);

		if (c == -1)
			break;
				
		switch (c)
		{
		default:
			Com_Printf("SV_ReadClientMessage: unknown command char\n");
			SV_DropClient(cl);
			return;
						
		case clc_nop:
			break;

		case clc_userinfo:
			Q_strncpyz(cl->userinfo, MSG_ReadString(&net_message), sizeof(cl->userinfo)); // jit
			SV_UserinfoChanged(cl);
			break;

		case clc_move:
			if (move_issued)
				return;		// someone is trying to cheat...

			move_issued = true;
			checksumIndex = net_message.readcount;
			checksum = MSG_ReadByte(&net_message);
			lastframe = MSG_ReadLong(&net_message);

			if (lastframe != cl->lastframe)
			{
				cl->lastframe = lastframe;

				if (cl->lastframe > 0)
				{
					cl->frame_latency[cl->lastframe&(LATENCY_COUNTS-1)] = 
						svs.realtime - cl->frames[cl->lastframe & UPDATE_MASK].senttime;
				}
			}

			memset(&nullcmd, 0, sizeof(nullcmd));
			MSG_ReadDeltaUsercmd(&net_message, &nullcmd, &oldest);
			MSG_ReadDeltaUsercmd(&net_message, &oldest, &oldcmd);
			MSG_ReadDeltaUsercmd(&net_message, &oldcmd, &newcmd);

			if (cl->state != cs_spawned)
			{
				cl->lastframe = -1;
				break;
			}

			// if the checksum fails, ignore the rest of the packet
			calculatedChecksum = COM_BlockSequenceCRCByte(
				net_message.data + checksumIndex + 1,
				net_message.readcount - checksumIndex - 1,
				cl->netchan.incoming_sequence);

			if (calculatedChecksum != checksum)
			{
				Com_DPrintf("Failed command checksum for %s (%d != %d)/%d.\n", 
					cl->name, calculatedChecksum, checksum, 
					cl->netchan.incoming_sequence);
				return;
			}

			if (!sv_paused->value)
			{
				net_drop = cl->netchan.dropped;

				if (net_drop < 20)
				{
					while (net_drop > 2)
					{
						SV_ClientThink(cl, &cl->lastcmd);
						net_drop--;
					}

					if (net_drop > 1)
						SV_ClientThink(cl, &oldest);

					if (net_drop > 0)
						SV_ClientThink(cl, &oldcmd);
				}

				SV_ClientThink(cl, &newcmd);
			}

			cl->lastcmd = newcmd;
			break;

		case clc_stringcmd:	
			s = MSG_ReadString(&net_message);

			// malicious users may try using too many string commands
			if (++stringCmdCount < MAX_STRINGCMDS)
				SV_ExecuteUserCommand(s);

			if (cl->state == cs_zombie)
				return;	// disconnect command
			break;
		}
	}
}

// === jitspeedhackcheck
void SV_AddServerMsecToClients (int msec)
{
	int			i;
	client_t	*cl;
	int			max = maxclients->value; // todo: better way to do this?

	for (i = 0; i < max; ++i)
	{
		cl = svs.clients + i;

		if (cl->state != cs_spawned)
			continue;

		cl->commandMsecServerTotal += msec;
	}
}
// jitspeedhackcheck ===
