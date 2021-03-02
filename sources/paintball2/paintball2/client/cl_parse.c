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
// cl_parse.c  -- parse a message received from the server

#include "client.h"

char *Cmd_MacroExpandString (const char *text);

#ifndef WIN32
#define _strtime(a) sprintf(a, "TODO")
#endif

char *svc_strings[256] =
{
	"svc_bad",

	"svc_muzzleflash",
	"svc_muzzlflash2",
	"svc_temp_entity",
	"svc_layout",
	"svc_inventory",

	"svc_nop",
	"svc_disconnect",
	"svc_reconnect",
	"svc_sound",
	"svc_print",
	"svc_stufftext",
	"svc_serverdata",
	"svc_configstring",
	"svc_spawnbaseline",	
	"svc_centerprint",
	"svc_download",
	"svc_playerinfo",
	"svc_packetentities",
	"svc_deltapacketentities",
	"svc_frame"
};

//=============================================================================

void CL_DownloadFileName (char *dest, int destlen, char *fn)
{
	if (strncmp(fn, "players", 7) == 0)
		Com_sprintf(dest, destlen, "%s/%s", BASEDIRNAME, fn);
	else
		Com_sprintf(dest, destlen, "%s/%s", FS_Gamedir(), fn);
}

/*
===============
CL_CheckOrDownloadFile

Returns true if the file exists, otherwise it attempts
to start a download from the server.
===============
*/

// Knightmare- store the names of last downloads that failed, jitdownload
#define NUM_FAIL_DLDS 64
char lastfaileddownload[NUM_FAIL_DLDS][MAX_OSPATH];

void clearfaileddownloads() // jitdownload
{
	int i;

	for (i = 0; i < NUM_FAIL_DLDS; i++)
		*lastfaileddownload[i] = '\0';
}


// xrichardx: replaces all '\' with '/' to avoid issues
void filename_replace_backslashes(char * filename, size_t size)
{
	char *ptr;

	for (ptr = filename; ptr < (filename + size); ptr++)
	{
		if (*ptr == '\\')
		{
			*ptr = '/';
		}
	}
}

qboolean CL_CheckOrDownloadFile (const char *check_filename)
{
	FILE	*fp;
	char	name[MAX_OSPATH];
	char	filename[MAX_OSPATH];
	int		i;
	const char *file_ext;

	Q_strncpyz(filename, check_filename, sizeof(filename) - 4);

	// === jitdownload - make sure we have skp files to go with skm's
	if (Q_streq(COM_FileExtension(filename), "md2"))
	{
		COM_StripExtension(filename, name, sizeof(name));
		Q_strncatz(name, ".skm", sizeof(name));

		if (FS_LoadFile(name, NULL) != -1)
		{
			// make sure the .skp exists, too
			COM_StripExtension(filename, name, sizeof(name));
			Q_strncatz(name, ".skp", sizeof(name));

			if (FS_LoadFile(name, NULL) != -1)
				return true;
			else
				Q_strncpyz(filename, name, sizeof(filename)); // we need to download the .skp file.
		}
		else
		{
			// Check if .md2 exists
			if (FS_LoadFile(filename, NULL) != -1)
				return true;
            else if (cls.download3supported)
				Q_strncpyz(filename, name, sizeof(filename)); // request the .skm file
		}
	}
	// jitdownload ===

	if (FS_LoadFile(filename, NULL) != -1)
	{	// it exists, no need to download
		return true;
	}

	if (strstr(filename, "..")) // jitdownload, moved so this error doesn't come up with existing files.
	{
		Com_Printf("Refusing to download a path with ..\n");
		return true;
	}

	// (jitdownload) Knightmare- don't try again to download a file that just failed
	for (i = 0; i < NUM_FAIL_DLDS; i++)
	{
		if (lastfaileddownload[i] && *lastfaileddownload[i] &&
			Q_streq(filename, lastfaileddownload[i]))
		{	// we already tried downlaoding this, server didn't have it
			return true;
		}
	}

	// ===
	// jitdownload, check for pngs, jpgs and tga's if pcx's aren't there
	file_ext = COM_FileExtension(filename);

	if (Q_streq(file_ext, "pcx") || Q_streq(file_ext, "jpg") || Q_streq(file_ext, "png") ||
		Q_streq(file_ext, "tga") || Q_streq(file_ext, "wal"))
	{
		int retry;

		for (retry = 0; retry < 2; ++retry)
		{
			// look for png first:
			COM_StripExtension(filename, name, sizeof(name));
			Q_strncatz(name, ".png", sizeof(name));

			if (FS_LoadFile(name, NULL) != -1)
				return true; // png exists, don't try to download anything else

			// no png, look for jpg:
			COM_StripExtension(filename, name, sizeof(name));
			Q_strncatz(name, ".jpg", sizeof(name));

			if (FS_LoadFile(name, NULL) != -1)
				return true; // jpg exists, don't try to download anything else

			// couldn't find jpg, let's try tga;
			COM_StripExtension(filename, name, sizeof(name));
			Q_strncatz(name, ".tga", sizeof(name));

			if (FS_LoadFile(name, NULL) != -1)
				return true; // tga exists

			// no tga, try pcx:
			COM_StripExtension(filename, name, sizeof(name));
			Q_strncatz(name, ".pcx", sizeof(name));

			if (FS_LoadFile(name, NULL) != -1)
				return true;

			// no pcx, try wal:
			COM_StripExtension(filename, name, sizeof(name));
			Q_strncatz(name, ".wal", sizeof(name));

			if (FS_LoadFile(name, NULL) != -1)
				return true;

#ifdef USE_DOWNLOAD3
			// If high-resolution textures are enabled, check for them first.
			if (cl_fast_download->value && cls.download3supported && gl_highres_textures->value)
			{
				if (!strstr(filename, "/hr4/"))
				{
					char *lastdir, *s;

					lastdir = s = filename;

					while (*s)
					{
						if (*s == '/')
							lastdir = s + 1;

						s++;
					}

					memmove(lastdir + 4, lastdir, strlen(lastdir) + 1);
					memcpy(lastdir, "hr4/", 4);

					// don't try again to download a file that just failed
					for (i = 0; i < NUM_FAIL_DLDS; i++)
					{
						if (lastfaileddownload[i] && *lastfaileddownload[i] &&
							Q_streq(filename, lastfaileddownload[i]))
						{	// we already tried downlaoding this, server didn't have it
							return true;
						}
					}
				}
			}
#endif
		}
	}

	if (Q_streq(COM_FileExtension(filename), "md2"))
	{
		// .md2 file not required if we have a .skm
		COM_StripExtension(filename, name, sizeof(name));
		Q_strncatz(name, ".skm", sizeof(name));

		if (FS_LoadFile(name, NULL) != -1)
		{
			// make sure the .skp exists, too
			COM_StripExtension(filename, name, sizeof(name));
			Q_strncatz(name, ".skp", sizeof(name));

			if (FS_LoadFile(name, NULL) != -1)
				return true;
			else
				Q_strncpyz(filename, name, sizeof(filename)); // we need to download the .skp file.
		}
	}
	// jit
	// ===

	filename_replace_backslashes(filename, strlen(filename));

	Q_strncpyz(cls.downloadname, filename, sizeof(cls.downloadname));

	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	COM_StripExtension(cls.downloadname, cls.downloadtempname, sizeof(cls.downloadtempname));
	strcat(cls.downloadtempname, ".tmp");

//ZOID
	// check to see if we already have a tmp for this file, if so, try to resume
	// open the file if not opened yet
	CL_DownloadFileName(name, sizeof(name), cls.downloadtempname);

#ifdef USE_DOWNLOAD3
	cls.download3rate = 0.0f;

	if (cl_fast_download->value && cls.download3supported)
	{
		// Forward this on to the download3 console command to avoid redundant code
		Com_Printf("Requesting %s.\n", cls.downloadname);
		Cbuf_AddText(va("download3 %s\n", cls.downloadname));
	}
	else
#endif
	{
		fp = fopen(name, "r+b");

		if (fp)
		{ // it exists
			int len;
			fseek(fp, 0, SEEK_END);
			len = ftell(fp);

			cls.download = fp;

			// give the server an offset to start the download
			Com_Printf("Resuming %s.\n", cls.downloadname);
			MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
			MSG_WriteString(&cls.netchan.message,
				va("download %s %i", cls.downloadname, len));
		} else {
			Com_Printf("Downloading %s.\n", cls.downloadname);
			MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
			MSG_WriteString(&cls.netchan.message,
				va("download %s", cls.downloadname));
		}
	}

	cls.downloadnumber++;
	return false;
}


/*
===============
CL_Download_f

Request a download from the server
===============
*/
void CL_Download_f (void)
{
	char filename[MAX_OSPATH];

#ifdef USE_DOWNLOAD3
	if (cl_fast_download->value && cls.download3supported)
	{
		CL_Download3_f();
		return;
	}
#endif

	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: download <filename>\n");
		return;
	}

	Com_sprintf(filename, sizeof(filename), "%s", Cmd_Argv(1));

	if (strstr(filename, ".."))
	{
		Com_Printf ("Refusing to download a path with ..\n");
		return;
	}

	if (FS_LoadFile(filename, NULL) != -1)
	{	// it exists, no need to download
		Com_Printf("File already exists.\n");
		return;
	}

	strcpy(cls.downloadname, filename);
	Com_Printf("Downloading %s.\n", cls.downloadname);

	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	COM_StripExtension(cls.downloadname, cls.downloadtempname, sizeof(cls.downloadtempname));
	strcat(cls.downloadtempname, ".tmp");

	MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
	MSG_WriteString(&cls.netchan.message,
		va("download %s", cls.downloadname));

	cls.downloadnumber++;
#ifdef USE_DOWNLOAD3
	cls.download3rate = 0.0f;
#endif
}


#ifdef USE_DOWNLOAD3
void CL_Download3_f (void) // jitdownload
{
	char szFilename[MAX_OSPATH];

	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: download3 <filename>\n");
		return;
	}

	Q_strncpyz(szFilename, Cmd_Argv(1), sizeof(szFilename));
	
	if (FS_LoadFile(szFilename, NULL) != -1)
	{
		Com_Printf("File already exists.\n");
		return;
	}

	MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
	MSG_WriteString(&cls.netchan.message, va("download3 %s", szFilename));
	cls.download3requested = true;
	cls.downloadnumber++; // not sure if this is needed, copied from other download func.
}
#endif

/*
======================
CL_RegisterSounds
======================
*/
void CL_RegisterSounds (void)
{
	int i;

	S_BeginRegistration();
	CL_RegisterTEntSounds();

	for (i = 1; i < MAX_SOUNDS; ++i)
	{
		if (!cl.configstrings[CS_SOUNDS + i][0])
			break;

		cl.sound_precache[i] = S_RegisterSound(cl.configstrings[CS_SOUNDS + i]);
		Sys_SendKeyEvents();	// pump message loop
	}

	S_EndRegistration();
}


/*
=====================
CL_ParseDownload

A download message has been received from the server
=====================
*/
void CL_ParseDownload (void)
{
	int		size, percent;
	char	name[MAX_OSPATH];
	int		r, i;

	// read the data
	size = MSG_ReadShort(&net_message);
	percent = MSG_ReadByte(&net_message);

	// === jitdemo - multi-map support
	if (cl.attractloop)
	{
		net_message.readcount += size;
		return;
	}
	// jitdemo ===

#ifdef USE_DOWNLOAD3
	// If for some reason we have a download active at the same time as a download3,
	// ignore the old style download
	if (cls.download3chunks)
	{
		net_message.readcount += size;
		return;
	}
#endif

	if (size == -1)
	{
		Com_Printf("Server does not have this file or refused download.\n");

		if (cls.downloadname)	// (jitdownload) Knightmare- save name of failed download
		{
			qboolean found = false;
			qboolean added = false;

			// check if this name is already in the table
			for (i = 0; i < NUM_FAIL_DLDS; i++)
			{
				if (lastfaileddownload[i] && strlen(lastfaileddownload[i])
					&& Q_streq(cls.downloadname, lastfaileddownload[i]))
				{
					found = true;
					break;
				}
			}

			// if it isn't already in the table, then we need to add it
			if (!found)
			{
				char *file_ext;

				for (i = 0; i < NUM_FAIL_DLDS; i++)
				{
					if (!lastfaileddownload[i] || !strlen(lastfaileddownload[i]))
					{	// found an open spot, fill it
						sprintf(lastfaileddownload[i], "%s", cls.downloadname);
						added = true;
						break;
					}
				}

				// if there wasn't an open spot, we'll have to move one over to make room
				// and add it in the last spot
				if (!added)
				{
					for (i = 0; i < NUM_FAIL_DLDS-1; i++)
						sprintf(lastfaileddownload[i], "%s", lastfaileddownload[i+1]);

					sprintf(lastfaileddownload[NUM_FAIL_DLDS-1], "%s", cls.downloadname);
				}

#ifdef USE_DOWNLOAD3
				if (!cls.download3supported || !cl_fast_download->value) // dl3 system checks for alternates server-side
#endif
				{
					// jitdownload -- if png failed, try jpg, then tga, then pcx or wal
					if (file_ext = strstr(cls.downloadname, ".png"))
					{
						*file_ext = 0;
						strcat(cls.downloadname, ".jpg");
						CL_CheckOrDownloadFile(cls.downloadname);
						return;
					}
					else if (file_ext = strstr(cls.downloadname, ".jpg"))
					{
						*file_ext = 0;
						strcat(cls.downloadname, ".tga");
						CL_CheckOrDownloadFile(cls.downloadname);
						return;
					}
					else if (file_ext = strstr(cls.downloadname, ".tga"))
					{
						*file_ext = 0;

						if (strstr(cls.downloadname, "textures"))
							strcat(cls.downloadname, ".wal");
						else
							strcat(cls.downloadname, ".pcx");

						CL_CheckOrDownloadFile(cls.downloadname);
						return;
					}
				}
			}	
		}
		// end Knightmare

#ifdef USE_DOWNLOAD3
		CL_StopCurrentDownload();
#else
		if (cls.download)
		{
			// if here, we tried to resume a file but the server said no
			fclose(cls.download);
			cls.download = NULL;
		}
#endif

		CL_RequestNextDownload();
		return;
	}

	// open the file if not opened yet
	if (!cls.download)
	{
		CL_DownloadFileName(name, sizeof(name), cls.downloadtempname);
		FS_CreatePath(name);
		cls.download = fopen(name, "wb");

		if (!cls.download)
		{
			net_message.readcount += size;
			Com_Printf("Failed to open %s.\n", cls.downloadtempname);
			CL_RequestNextDownload();
			return;
		}
	}

	fwrite(net_message.data + net_message.readcount, 1, size, cls.download);
	net_message.readcount += size;

	if (percent != 100)
	{
		// request next block
		cls.downloadpercent = percent;
		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		SZ_Print(&cls.netchan.message, "nextdl");
	}
	else
	{
		char oldn[MAX_OSPATH];
		char newn[MAX_OSPATH];

#ifdef USE_DOWNLOAD3
		CL_StopCurrentDownload();
#else
		fclose(cls.download);
#endif
		// rename the temp file to it's final name
		CL_DownloadFileName(oldn, sizeof(oldn), cls.downloadtempname);
		CL_DownloadFileName(newn, sizeof(newn), cls.downloadname);
		r = rename(oldn, newn);

		if (r)
			Com_Printf ("Failed to rename.\n");

		cls.download = NULL;
		cls.downloadpercent = 0;
		CL_RequestNextDownload(); // get another file if needed
	}
}

#ifdef USE_DOWNLOAD3 // jitdownload
void CL_StopCurrentDownload (void)
{
	if (cls.download)
	{
		fclose(cls.download);
		cls.download = NULL;
	}

	if (cls.download3chunks)
	{
		Z_Free(cls.download3chunks);
		cls.download3chunks = NULL;
	}

	if (cls.download3data)
	{
		Z_Free(cls.download3data);
		cls.download3data = NULL;
	}

	cls.download3lastchunkwritten = 0;
	cls.download3size = 0;

	if (cls.download3fileid != -1)
		cls.download3lastfileid = cls.download3fileid;

	cls.download3fileid = -1;
	memset(cls.download3backacks, -1, sizeof(cls.download3backacks));
	cls.download3currentbackack = 0;
}


static void CL_StartDownload3 (void)
{
	const char *filename;
	int num_chunks, i;
	char name[MAX_OSPATH];

	CL_StopCurrentDownload();
	cls.download3rate = 0.0f;
	cls.download3completechunks = 0;
	cls.download3bytesreceived = 0;
	cls.download3bytessincelastratecheck = 0;
	cls.download3fileid = MSG_ReadByte(&net_message);
	cls.download3size = MSG_ReadLong(&net_message); // how big is the file?
	cls.download3compression = (int)MSG_ReadByte(&net_message); // compression mode / reserved.  Unused so far.  0 == none.
	cls.download3starttime = Sys_Milliseconds();
	cls.download3lastratecheck = cls.download3starttime;

	if (cls.download3compression != 0)
		Com_Printf("Compression mode/version not supported: %d\n", cls.download3compression);

	filename = MSG_ReadString(&net_message); // get the filename
	cls.download3md5sum = MSG_ReadLong(&net_message);

	if (!cls.download3requested)
	{
		// We didn't request a download!
		Com_Printf("Server initiated download when none was requested!\n");
		return;
	}

	if (strstr(filename, "..") || filename[0] == '/' || filename[0] == '\\')
	{
		Com_Printf("Refusing to download a path with .. or starting with /.\n");
		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		SZ_Print(&cls.netchan.message, va("dl3confirm -1 %d\n", cls.download3fileid));
		cls.download3requested = false;
		CL_RequestNextDownload(); // get another file if needed
		return;
	}

	if (PathContainsInvalidCharacters(filename))
	{
		Com_Printf("Filename contains invalid characters: %s\n", filename);
		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		SZ_Print(&cls.netchan.message, va("dl3confirm -1 %d\n", cls.download3fileid));
		cls.download3requested = false;
		CL_RequestNextDownload(); // get another file if needed
		return;
	}

	if (FS_LoadFile(filename, NULL) != -1)
	{
		Com_Printf("File already exists.\n");
		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		SZ_Print(&cls.netchan.message, va("dl3confirm -1 %d\n", cls.download3fileid));
		cls.download3requested = false;
		CL_RequestNextDownload(); // get another file if needed
		return;
	}

	strcpy(cls.downloadname, filename);
	num_chunks = (cls.download3size + DOWNLOAD3_CHUNKSIZE - 1) / DOWNLOAD3_CHUNKSIZE;

	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	COM_StripExtension(cls.downloadname, cls.downloadtempname, sizeof(cls.downloadtempname));
	strcat(cls.downloadtempname, ".tmp");
	CL_DownloadFileName(name, sizeof(name), cls.downloadtempname);
	FS_CreatePath(name);
	cls.download = fopen(name, "r+b");

	// Allocate data
	cls.download3chunks = Z_Malloc(num_chunks * sizeof(qboolean));
	cls.download3data = Z_Malloc(cls.download3size);

	if (!cls.download3data || !cls.download3chunks)
	{
		Com_Printf("Failed to allocate %d bytes for file download.\n", cls.download3size);
		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		SZ_Print(&cls.netchan.message, va("dl3confirm -1 %d\n", cls.download3fileid));
		cls.download3requested = false;
		CL_RequestNextDownload(); // get another file if needed
		return;
	}

	memset(cls.download3chunks, DOWNLOAD3_CHUNKWAITING, num_chunks * sizeof(qboolean));

	if (cls.download) // file already exists, so resume
	{
		int offset, chunk_offset;

		fseek(cls.download, 0, SEEK_END);
		offset = ftell(cls.download);
		chunk_offset = offset / DOWNLOAD3_CHUNKSIZE;

		if (chunk_offset >= num_chunks)
			chunk_offset = 0; // Something is screwball.  We have more data than there is in the file.  Just start over.

		for (i = 0; i < chunk_offset; ++i)
			cls.download3chunks[i] = DOWNLOAD3_CHUNKWRITTEN;

		fseek(cls.download, chunk_offset * DOWNLOAD3_CHUNKSIZE, SEEK_SET);
		Com_Printf("Resuming %s.\n", cls.downloadname);
		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		SZ_Print(&cls.netchan.message, va("dl3confirm %d %d\n", chunk_offset, cls.download3fileid));
		cls.download3completechunks = chunk_offset;
	}
	else
	{
		cls.download = fopen(name, "wb");

		if (!cls.download)
		{
			Com_Printf("Failed to open %s.\n", cls.downloadtempname);
			MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
			SZ_Print(&cls.netchan.message, va("dl3confirm -1 %d\n", cls.download3fileid));
			cls.download3requested = false;
			CL_RequestNextDownload(); // get another file if needed
			return;
		}

		Com_Printf("Downloading %s.\n", cls.downloadname);
		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		SZ_Print(&cls.netchan.message, va("dl3confirm 0 %d\n", cls.download3fileid));
	}
}

#endif


/*
=====================================================================
  SERVER CONNECTING MESSAGES
=====================================================================
*/

/*
==================
CL_ParseServerData
==================
*/
void CL_ParseServerData (void)
{
	extern cvar_t	*fs_gamedirvar;
	char	*str;
	int		i;
	qboolean attractloop = cl.attractloop; // jitdemo - multi-map demo support
	
	Com_DPrintf ("Serverdata packet received.\n");
	// wipe the client_state_t struct
	CL_ClearState();
	cls.state = ca_connected;
	// parse protocol version number
	i = MSG_ReadLong(&net_message);
	cls.serverProtocol = i;

	// BIG HACK to let demos from release work with the 3.0x patch!!!
	if (Com_ServerState() && PROTOCOL_VERSION == 34)
	{
	}
	else if (i != PROTOCOL_VERSION)
	{
		Com_Error(ERR_DROP, "Server returned version %i, not %i.", i, PROTOCOL_VERSION);
	}

	cl.servercount = MSG_ReadLong(&net_message);
	cl.attractloop = MSG_ReadByte(&net_message);

	// === jitdemo - multi-map demo support (not sure if this is the correct way to go about this - may not still be on the demo at this point)
	if (attractloop)
		cl.attractloop = true;
	// jitdemo ===

	// game directory
	str = MSG_ReadString(&net_message);
	Q_strncpyz(cl.gamedir, str, sizeof(cl.gamedir) - 1);

	// set gamedir
	if ((*str && (!fs_gamedirvar->string || !*fs_gamedirvar->string || !Q_streq(fs_gamedirvar->string, str))) || (!*str && (fs_gamedirvar->string || *fs_gamedirvar->string)))
		Cvar_Set("game", str); // jitodo -- don't set game on demos (cl.attractloop)

	// parse player entity number
	cl.playernum = MSG_ReadShort(&net_message);

	// get the full level name (CS_NAME)
	str = MSG_ReadString(&net_message);

	if (cl.playernum == -1)
	{
		// playing a cinematic or showing a pic, not a level
		SCR_PlayCinematic(str);
	}
	else
	{
		// seperate the printfs so the server message can have a color
		Com_Printf("\n\n %c%c\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n", SCHAR_COLOR, ';'); // jittext
		Com_Printf("%c%c%s\n\n", SCHAR_COLOR, COLOR_MAPNAME, str); // jittext

		// need to prep refresh at next oportunity
		cl.refresh_prepped = false;
	}

	Cvar_Set("menu_fullmapname", str);
}

/*
==================
CL_ParseBaseline
==================
*/
void CL_ParseBaseline (void)
{
	entity_state_t	*es;
	int				bits;
	int				newnum;
	entity_state_t	nullstate;

	memset (&nullstate, 0, sizeof(nullstate));

	newnum = CL_ParseEntityBits (&bits);
	es = &cl_entities[newnum].baseline;
	CL_ParseDelta (&nullstate, es, newnum, bits);
}


/*
================
CL_LoadClientinfo

================
*/
void CL_LoadClientinfo (clientinfo_t *ci, char *s)
{
	int			i;
	char		*t;
	char		model_name[MAX_QPATH];
	char		skin_name[MAX_QPATH];
	char		model_filename[MAX_QPATH];
	char		skin_filename[MAX_QPATH];
	char		weapon_filename[MAX_QPATH];
	char		oldname[MAX_QPATH];

	Q_strncpyz(ci->cinfo, s, sizeof(ci->cinfo));
	Q_strncpyz(oldname, ci->name, sizeof(oldname)); // jit - save previous name for name change comparison

	// isolate the player's name
	Q_strncpyz(ci->name, s, sizeof(ci->name));
	t = strstr(s, "\\");

	if (t)
	{
		ci->name[t - s] = 0;
		s = t + 1;
	}

	if (cl_shownamechange->value) // jit - show name changes on the client
	{
		char oldname_noformat[MAX_QPATH];
		char newname_noformat[MAX_QPATH];

		strip_garbage(oldname_noformat, oldname, sizeof(oldname_noformat));
		strip_garbage(newname_noformat, ci->name, sizeof(newname_noformat));

		if (*oldname_noformat && *newname_noformat && Q_strcasecmp(newname_noformat, oldname_noformat))
		{
			Com_Printf("%s changed name to %s.\n", oldname, ci->name);
		}
	}

	if (cl_noskins->value || *s == 0)
	{
		Com_sprintf(model_filename, sizeof(model_filename), "players/male/tris.md2");
		Com_sprintf(weapon_filename, sizeof(weapon_filename), "players/male/weapon.md2");

#ifdef QUAKE2
#else
		//===
		//jit
		switch(s ? s[strlen(s) - 1] : 0)
		{
		case 'r':
			Com_sprintf(skin_filename, sizeof(skin_filename), "players/male/pb2r.pcx");
			break;
		case 'b':
			Com_sprintf(skin_filename, sizeof(skin_filename), "players/male/pb2b.pcx");
			break;
		case 'p':
			Com_sprintf(skin_filename, sizeof(skin_filename), "players/male/pb2p.pcx");
			break;
		default:
			Com_sprintf(skin_filename, sizeof(skin_filename), "players/male/pb2y.pcx");
		}
		
		Com_sprintf(ci->iconname, sizeof(ci->iconname), "/players/male/pb2b_i.pcx");
		//jit
		//===
#endif

		ci->model = re.RegisterModel(model_filename);
		memset(ci->weaponmodel, 0, sizeof(ci->weaponmodel));
		ci->weaponmodel[0] = re.RegisterModel(weapon_filename);
		//ci->skin = re.RegisterSkin(skin_filename);
		re.RegisterSkin(skin_filename, ci->model, ci->skins); // jitskm
		ci->icon = re.RegisterPic(ci->iconname);
	}
	else
	{
		// isolate the model name
		strcpy(model_name, s);
		t = strstr(model_name, "/");

		if (!t)
			t = strstr(model_name, "\\");

		if (!t)
			t = model_name;

		*t = 0;

		// isolate the skin name
		strcpy(skin_name, s + strlen(model_name) + 1);

		// model file
		Com_sprintf(model_filename, sizeof(model_filename), "players/%s/tris.md2", model_name);
		ci->model = re.RegisterModel(model_filename);

		if (!ci->model)
		{
			strcpy(model_name, "male");
			Com_sprintf(model_filename, sizeof(model_filename), "players/male/tris.md2");
			ci->model = re.RegisterModel(model_filename);
		}

		// skin file
		Com_sprintf(skin_filename, sizeof(skin_filename), "players/%s/%s.pcx", model_name, skin_name);
		//ci->skin = re.RegisterSkin(skin_filename);
		re.RegisterSkin(skin_filename, ci->model, ci->skins); // jitskm

		// if we don't have the skin and the model wasn't male,
		// see if the male has it (this is for CTF's skins)
 		if (!ci->skins[0] && Q_strcasecmp(model_name, "male"))
		{
			// change model to male
			strcpy(model_name, "male");
			Com_sprintf(model_filename, sizeof(model_filename), "players/male/tris.md2");
			ci->model = re.RegisterModel(model_filename);

			// see if the skin exists for the male model
			Com_sprintf(skin_filename, sizeof(skin_filename), "players/%s/%s.pcx", model_name, skin_name);
			//ci->skin = re.RegisterSkin(skin_filename);
			re.RegisterSkin(skin_filename, ci->model, ci->skins); // jitskm
		}

		// if we still don't have a skin, it means that the male model didn't have
		// it, so default to grunt
		if (!ci->skins[0])
		{
			// see if the skin exists for the male model
			Com_sprintf(skin_filename, sizeof(skin_filename), "players/%s/grunt.pcx", model_name, skin_name); // jitodo -- check for appropriate pball skin
			//ci->skin = re.RegisterSkin(skin_filename);
			re.RegisterSkin(skin_filename, ci->model, ci->skins); // jitskm
		}

		// weapon file
		for (i = 0; i < num_cl_weaponmodels; i++)
		{
			Com_sprintf (weapon_filename, sizeof(weapon_filename), "players/%s/%s", model_name, cl_weaponmodels[i]);
			ci->weaponmodel[i] = re.RegisterModel(weapon_filename);

			if (!ci->weaponmodel[i] && Q_streq(model_name, "cyborg"))
			{
				// try male
				Com_sprintf(weapon_filename, sizeof(weapon_filename), "players/male/%s", cl_weaponmodels[i]);
				ci->weaponmodel[i] = re.RegisterModel(weapon_filename);
			}

			if (!cl_vwep->value)
				break; // only one when vwep is off
		}

		// icon file
		Com_sprintf(ci->iconname, sizeof(ci->iconname), "/players/%s/%s_i.pcx", model_name, skin_name);
		ci->icon = re.RegisterPic(ci->iconname);
	}

	// must have loaded all data types to be valud
	if (!ci->skins[0] || !ci->icon || !ci->model || !ci->weaponmodel[0])
	{
		//ci->skins[0] = NULL;
		memset(ci->skins, 0, sizeof(ci->skins));
		ci->icon = NULL;
		ci->model = NULL;
		ci->weaponmodel[0] = NULL;
		return;
	}
}

/*
================
CL_ParseClientinfo

Load the skin, icon, and model for a client
================
*/
void CL_ParseClientinfo (int player)
{
	char			*s;
	clientinfo_t	*ci;

	s = cl.configstrings[player + CS_PLAYERSKINS];
	ci = &cl.clientinfo[player];
	CL_LoadClientinfo(ci, s);
	cl_scores_refresh(); // jitmenu - refresh scoreboard on name change.
}


void translate_string (char *out, size_t size, const char *in) // jittrans
{
	extern cvar_t			*cl_language;
	static qboolean			loaded = false;
	static hash_table_t		string_trans_hash;
	char					*s;

	// Store the translation into a hash table for quick reference
	if (!loaded || cl_language->modified)
	{
		char lang_file[MAX_QPATH];
		char *buff;

		if (loaded)
			hash_table_free(&string_trans_hash); // wipe out the old hash table
        
		hash_table_init(&string_trans_hash, 0x100, Z_Free);
		loaded = true;
		cl_language->modified = false;
		Com_sprintf(lang_file, sizeof(lang_file), "configs/%s.lang", cl_language->string);
		
		if (FS_LoadFileZ(lang_file, (void **)&buff) > 0)
		{
			char *key, *val;
			
			key = strtok(buff, "\r\n");
			val = strtok(NULL, "\r\n");

			while (key && val)
			{
				char *key_newline = CopyStringAddNewline(key);
				hash_add(&string_trans_hash, key, CopyString(val));
				hash_add(&string_trans_hash, key_newline, CopyStringAddNewline(val)); // Not a great solution, but most prints include a \n at the end, so add this to the table as well.
				Z_Free(key_newline);
				key = strtok(NULL, "\r\n");
				val = strtok(NULL, "\r\n");
			}

			FS_FreeFile(buff);
		}
	}

	// If the string is in the translation table, copy it over the original
	if (s = hash_get(&string_trans_hash, in))
		Q_strncpyz(out, s, size);
	else
		Q_strncpyz(out, in, size);
}

/*
================
CL_ParseConfigString
================
*/
void CL_ParseConfigString (void)
{
	int		i;
	char	*s;
	char	olds[1024]; // jit

	i = MSG_ReadShort(&net_message);

	if (i < 0 || i >= MAX_CONFIGSTRINGS)
		Com_Error(ERR_DROP, "configstring > MAX_CONFIGSTRINGS");

	s = MSG_ReadString(&net_message);
	Q_strncpyz(olds, cl.configstrings[i], sizeof(olds)); // jit
	Q_strncpyz(cl.configstrings[i], s, (MAX_CONFIGSTRINGS - i) * MAX_QPATH); // jitsecurity

	// do something apropriate 
	if (i >= CS_LIGHTS && i < CS_LIGHTS+MAX_LIGHTSTYLES)
	{
		CL_SetLightstyle (i - CS_LIGHTS);
	}
	else if (i == CS_CDTRACK)
	{
		if (cl.refresh_prepped)
			CDAudio_Play (atoi(cl.configstrings[CS_CDTRACK]), true);
	}
	else if (i >= CS_MODELS && i < CS_MODELS + MAX_MODELS)
	{
		// Update the map name on the loading screen as soon as we have it
		if (i == CS_MODELS + 1)
		{
			char mapname[64];
			COM_StripExtension(cl.configstrings[CS_MODELS + 1] + 5, mapname, sizeof(mapname));
			Cvar_Set("menu_mapname", mapname);
		}

		if (cl.refresh_prepped)
		{
			cl.model_draw[i-CS_MODELS] = re.RegisterModel(cl.configstrings[i]);

			if (cl.configstrings[i][0] == '*')
				cl.model_clip[i - CS_MODELS] = CM_InlineModel(cl.configstrings[i]);
			else
				cl.model_clip[i - CS_MODELS] = NULL;
		}
	}
	else if (i >= CS_SOUNDS && i < CS_SOUNDS + MAX_MODELS)
	{
		if (cl.refresh_prepped)
			cl.sound_precache[i - CS_SOUNDS] = S_RegisterSound(cl.configstrings[i]);
	}
	else if (i >= CS_IMAGES && i < CS_IMAGES + MAX_MODELS)
	{
		if (cl.refresh_prepped)
			cl.image_precache[i - CS_IMAGES] = re.RegisterPic(cl.configstrings[i]);
	}
	else if (i >= CS_PLAYERSKINS && i < CS_PLAYERSKINS + MAX_CLIENTS)
	{
		if (cl.refresh_prepped && !Q_streq(olds, s))
			CL_ParseClientinfo(i - CS_PLAYERSKINS);
	}
	else if (i == CS_SERVERGVERSION) // jitversion
	{
		s = strstr(cl.configstrings[CS_SERVERGVERSION], "Gamebuild:");

		if (s)
			cls.server_gamebuild = atoi(s + 10);
		else
			cls.server_gamebuild = 0;
	}
	else if (i >= CS_EVENTS)
	{
		translate_string(cl.configstrings[i], MAX_QPATH, cl.configstrings[i]); // jittrans
	}
}


/*
=====================================================================

ACTION MESSAGES

=====================================================================
*/

/*
==================
CL_ParseStartSoundPacket
==================
*/
void CL_ParseStartSoundPacket (void)
{
    vec3_t  pos_v;
	float	*pos;
    int 	channel, ent;
    int 	sound_num;
    float 	volume;
    float 	attenuation;  
	int		flags;
	float	ofs;

	flags = MSG_ReadByte(&net_message);
	sound_num = MSG_ReadByte(&net_message);

    if (flags & SND_VOLUME)
		volume = MSG_ReadByte(&net_message) / 255.0;
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;
	
    if (flags & SND_ATTENUATION)
		attenuation = MSG_ReadByte(&net_message) / 64.0;
	else
		attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;	

    if (flags & SND_OFFSET)
		ofs = MSG_ReadByte(&net_message) / 1000.0;
	else
		ofs = 0;

	if (flags & SND_ENT)
	{	// entity reletive
		channel = MSG_ReadShort(&net_message); 
		ent = channel >> 3;

		if (ent > MAX_EDICTS)
			Com_Error(ERR_DROP, "CL_ParseStartSoundPacket: ent = %i", ent);

		channel &= 7;
	}
	else
	{
		ent = 0;
		channel = 0;
	}

	if (flags & SND_POS)
	{	// positioned in space
		MSG_ReadPos(&net_message, pos_v);
 
		pos = pos_v;
	}
	else	// use entity number
		pos = NULL;

	if (!cl.sound_precache[sound_num])
		return;

	S_StartSound(pos, ent, channel, cl.sound_precache[sound_num], volume, attenuation, ofs);
}       


void SHOWNET (char *s)
{
	if (cl_shownet->value>=2)
		Com_Printf ("%3i:%s\n", net_message.readcount-1, s);
}

extern cvar_t *cl_timestamp; // jit
char timestamp[24];

static void CL_ParseChat (int level, const char *s) // jitchat / jitenc
{
	int idx;
	qboolean isteam, isprivate;
	char *pch, *rep;
	extern cvar_t *cl_swearfilter, *cl_blockedwords;
	int len;
	qboolean darken_text = false;

	isteam = (level == PRINT_CHATN_TEAM);
	isprivate = (level == PRINT_CHATN_PRIVATE);

	// first character(s) used for decoding client index,
	// need to decode then offset properly to get our actual chat text
	decode_unsigned(s, &idx, 1);

	if (idx == 255)
		s += 5;
	else if (idx == 254)
		s += 2;
	else
		s ++;

	// jit - don't filter own text.  Let the children blabber on and think they're just being ignored, otherwise they'll try to evade the filter.
	if (idx != cl.playernum)
	{
		int filter_value = (int)(cl_swearfilter->value + 0.5f); // round to nearest integer

		// viciouz - swear filter
		if (filter_value)
		{
			char wordlist[1024];
			char lowercase_text[1024];


			Q_strncpyz(wordlist, cl_blockedwords->string, sizeof(wordlist));
			Q_strncpyz(lowercase_text, s, sizeof(lowercase_text));
			strtolower(lowercase_text);
			pch = strtok(wordlist, ","); // jit - may want to use spaces in the filters.  Regex would be the ultimate way to do it, but not very user friendly.

			while (pch != NULL)
			{
				rep = strstr(lowercase_text, pch);

				while(rep != NULL)
				{
					if (filter_value == 1) // jit - tweaked default behavior (eg: checkbox) to just drop the lines.  Set to 2 or other value to show **'s
						return;

					if (filter_value == 3) // jit - a little debug feature so we can see which lines are getting dropped.
					{
						darken_text = true;
						break;
					}

					len = strlen(pch);
					memset(rep, '*', len); // jit - on the off chance somebody puts a huge word in there.
					s = lowercase_text; // todo: this ends up making everything lowercase if a filtered word is in there
					rep = strstr(lowercase_text, pch);
				}
				
				pch = strtok(NULL, ",");
			}
		}
	}

	// todo - create a separate chat console.
	if (isteam)
		S_StartLocalSound("misc/talk_team.wav");
	else
		S_StartLocalSound("misc/talk.wav");

	if (cl_timestamp->value)
	{
		Com_Printf("%c%c[%s] %c%s%s%s%c%c%s%s %s%s",
			SCHAR_COLOR, isteam ? cl_scores_get_team_textcolor(idx) : COLOR_CHAT, 
			timestamp, cl_scores_get_team_splat(idx), 
			cl_scores_get_isalive(idx) ? "" : "[ELIM] ",
			(isteam || isprivate) ? "(" : "", name_from_index(idx),
			SCHAR_COLOR, darken_text ? '0' : (isteam ? cl_scores_get_team_textcolor(idx) : COLOR_CHAT), 
			(isteam || isprivate) ? ")" : "", 
			level == PRINT_CHATN_ACTION ? "" : ":",
			s, (s[strlen(s)-1] == '\n') ? "" : "\n");
	}
	else
	{
		Com_Printf("%c%c%c%s%s%s%c%c%s%s %s%s", cl_scores_get_team_splat(idx),
			SCHAR_COLOR, isteam ? cl_scores_get_team_textcolor(idx) : COLOR_CHAT, 
			cl_scores_get_isalive(idx) ? "" : "[ELIM] ", // jitodo - [OBS]
			(isteam || isprivate) ? "(" : "", name_from_index(idx),
			SCHAR_COLOR, darken_text ? '0' : (isteam ? cl_scores_get_team_textcolor(idx) : COLOR_CHAT), 
			(isteam || isprivate) ? ")" : "", 
			level == PRINT_CHATN_ACTION ? "" : ":",
			s, (s[strlen(s)-1] == '\n') ? "" : "\n");
	}
}


static void CL_PrintDefault (const char *text) // jit
{
	if (cl_timestamp->value)
		Com_Printf("[%s] %s", timestamp, text);
	else
		Com_Printf("%s", text);
}


static void CL_ParsePrintDefault (const char *text) // jit
{
	char translated[2048];
	translate_string(translated, sizeof(translated), text);
	CL_PrintDefault(translated);
}

extern cvar_t *cl_dialogprint;

static void CL_ParsePrintPopup (const char *text, qboolean printconsole) // jit
{
	char translated[2048];
	char *expanded;
	
	translate_string(translated, sizeof(translated), text);
	expanded = Cmd_MacroExpandString(translated);

	if (!printconsole || (cls.key_dest != key_console && cl_dialogprint->value)) // Only pop up the dialog when out of the console.
	{
		SCR_PrintPopup(expanded, !printconsole);
	}
	else
	{
		CL_PrintDefault(expanded);
	}
}

/*
=====================
CL_ParseServerMessage
=====================
*/
void CL_ParseServerMessage (void)
{
	int			cmd;
	char		*s;
	int			i;

	// ECHON / jit:
	_strtime(timestamp);

	// if recording demos, copy the message out
	if (cl_shownet->value == 1)
		Com_Printf("%i ", net_message.cursize);
	else if (cl_shownet->value >= 2)
		Com_Printf("------------------\n");

	// parse the message
	while (1)
	{
		if (net_message.readcount > net_message.cursize)
		{
			Com_Error (ERR_DROP,"CL_ParseServerMessage: Bad server message");
			break;
		}

		cmd = MSG_ReadByte(&net_message);

		if (cmd == -1)
		{
			SHOWNET("END OF MESSAGE");
			break;
		}

		if (cl_shownet->value>=2)
		{
			if (!svc_strings[cmd])
				Com_Printf ("%3i:BAD CMD %i\n", net_message.readcount-1,cmd);
			else
				SHOWNET(svc_strings[cmd]);
		}
	
		// other commands
		switch (cmd)
		{
		default:
#ifdef USE_DOWNLOAD3 // jitdownload
			if (cls.download3supported && cmd == cls.download3startcmd)
			{
				CL_StartDownload3();
				break;
			}
#endif

			Com_Error(ERR_DROP, "CL_ParseServerMessage: Illegible server message (%d)\n", cmd); // jit
			break;
			
		case svc_nop:
			break;
			
		case svc_disconnect:
			Com_Error(ERR_DISCONNECT, "Server disconnected.\n");
			break;

		case svc_reconnect:
			Com_Printf("Server disconnected, reconnecting.\n");

#ifdef USE_DOWNLOAD3
			CL_StopCurrentDownload();
#else
			if (cls.download)
			{
				//ZOID, close download
				fclose(cls.download);
				cls.download = NULL;
			}
#endif

			cls.state = ca_connecting;
			cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
			break;

		case svc_print:
			i = MSG_ReadByte(&net_message);

			switch (i) // jit
			{
			case PRINT_CHAT:
				S_StartLocalSound("misc/talk.wav");

				if (cl_timestamp->value) // jittext / jitcolor
					Com_Printf("%c%c[%s] %s", SCHAR_COLOR, COLOR_CHAT, timestamp, MSG_ReadString(&net_message));
				else
					Com_Printf("%c%c%s", SCHAR_COLOR, COLOR_CHAT, MSG_ReadString(&net_message));

				break;
			case PRINT_ITEM: // jit
				CL_ParsePrintItem(MSG_ReadString(&net_message));
				break;
			case PRINT_EVENT: // jit
				CL_ParsePrintEvent(MSG_ReadString(&net_message));
				break;
			case PRINT_SCOREDATA: // jitscores
				CL_ParseScoreData(MSG_ReadString(&net_message));
				break;
			case PRINT_PINGDATA:
				CL_ParsePingData(MSG_ReadString(&net_message));
				break;
			case PRINT_MAPLISTDATA:
				CL_ParseMaplistData(MSG_ReadString(&net_message));
				break;
			case PRINT_CHATN:
			case PRINT_CHATN_TEAM:
			case PRINT_CHATN_PRIVATE:
			case PRINT_CHATN_RESERVED:
			case PRINT_CHATN_RESERVED2:
			case PRINT_CHATN_ACTION:
				CL_ParseChat(i, MSG_ReadString(&net_message));
				break;
			case PRINT_POPUP:
				CL_ParsePrintPopup(MSG_ReadString(&net_message), true);
				break;
			case PRINT_POPUP_NOCON:
				CL_ParsePrintPopup(MSG_ReadString(&net_message), false);
				break;
			default:
				CL_ParsePrintDefault(MSG_ReadString(&net_message));
				break;
			}

			con.ormask = 0;
			break;
			
		case svc_centerprint:
			SCR_CenterPrint(MSG_ReadString(&net_message));
			break;
			
		case svc_stufftext:
			s = MSG_ReadString(&net_message);
			Com_DPrintf("Stufftext: %s\n", s);
			Cbuf_AddStuffText(s);
			break;
			
		case svc_serverdata:
			Cbuf_Execute();		// make sure any stuffed commands are done
			CL_ParseServerData();
			break;
			
		case svc_configstring:
			CL_ParseConfigString();
			break;
			
		case svc_sound:
			CL_ParseStartSoundPacket();
			break;
			
		case svc_spawnbaseline:
			CL_ParseBaseline();
			break;

		case svc_temp_entity:
			CL_ParseTEnt();
			break;

		case svc_muzzleflash:
			CL_ParseMuzzleFlash();
			break;

		case svc_muzzleflash2:
			CL_ParseMuzzleFlash2();
			break;

		case svc_download:
			CL_ParseDownload();
			break;
/*#ifdef USE_DOWNLOAD3 // jitdownload
		case svc_download3start:
			CL_StartDownload3();
			break;
#endif*/
		case svc_frame:
			CL_ParseFrame();
			break;

		case svc_inventory:
			CL_ParseInventory();
			break;

		case svc_layout:
			s = MSG_ReadString(&net_message);
			Q_strncpyz(cl.layout, s, sizeof(cl.layout)-1);
			break;

		case svc_playerinfo:
		case svc_packetentities:
		case svc_deltapacketentities:
			Com_Error(ERR_DROP, "Out of place frame data.");
			break;
		}
	}

	CL_AddNetgraph();

	// we don't know if it is ok to save a demo message until
	// after we have parsed the frame
	if (cls.demorecording && !cls.demowaiting)
		CL_WriteDemoMessage();
}


