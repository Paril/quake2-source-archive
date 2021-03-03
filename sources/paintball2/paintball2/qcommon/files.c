/*
Copyright(C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "qcommon.h"
#ifdef WIN32
#include <windows.h>
#include <direct.h> // jit - for getcwd
#else
#include <sys/types.h> // jitsecurity
#include <sys/stat.h> // jitsecurity
#include <unistd.h>
#endif


/*
=============================================================================

QUAKE FILESYSTEM

=============================================================================
*/


//
// in memory
//

typedef struct
{
	char	name[MAX_QPATH];
	int		filepos, filelen;
} packfile_t;

typedef struct pack_s
{
	char	filename[MAX_OSPATH];
	FILE	*handle;
	int		numfiles;
	packfile_t	*files;
} pack_t;

char	fs_gamedir[MAX_OSPATH];
cvar_t	*fs_basedir;
cvar_t	*fs_cddir;
cvar_t	*fs_gamedirvar;

typedef struct filelink_s
{
	struct filelink_s	*next;
	char	*from;
	int		fromlength;
	char	*to;
} filelink_t;

filelink_t	*fs_links;

typedef struct searchpath_s
{
	char	filename[MAX_OSPATH];
	pack_t	*pack;		// only one of filename / pack will be used
	struct searchpath_s *next;
} searchpath_t;

searchpath_t	*fs_searchpaths;
searchpath_t	*fs_base_searchpaths;	// without gamedirs


/*

All of Quake's data access is through a hierchal file system, but the contents of the file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the quake.exe and all game directories.  The sys_* files pass this to host_init in quakeparms_t->basedir.  This can be overridden with the "-basedir" command line parm to allow code debugging in a different directory.  The base directory is
only used during filesystem initialization.

The "game directory" is the first tree on the search path and directory that all generated files (savegames, screenshots, demos, config files) will be saved to.  This can be overridden with the "-game" command line parameter.  The game directory can never be changed while quake is executing.  This is a precacution against having a malicious server instruct clients to write files over areas they shouldn't.

*/


/*
================
FS_filelength
================
*/
int FS_filelength (FILE *f)
{
	int		pos;
	int		end;

	pos = ftell(f);
	fseek(f, 0, SEEK_END);
	end = ftell(f);
	fseek(f, pos, SEEK_SET);

	return end;
}


/*
============
FS_CreatePath

Creates any directories needed to store the given filename
============
*/
void FS_CreatePath (char *path)
{
	char	*ofs;
	
	for (ofs = path + 1; *ofs; ofs++)
	{
		if (*ofs == '/')
		{	// create the directory
			*ofs = 0;
			Sys_Mkdir(path);
			*ofs = '/';
		}
	}
}


/*
==============
FS_FCloseFile

For some reason, other dll's can't just cal fclose ()
on files returned by FS_FOpenFile...
==============
*/
void FS_FCloseFile (FILE *f)
{
	fclose(f);
}


// RAFAEL
/*
	Developer_searchpath
*/
int	Developer_searchpath (int who)
{
	
	int		ch;
	// PMM - warning removal
//	char	*start;
	searchpath_t	*search;
	
	if (who == 1) // xatrix
		ch = 'x';
	else if (who == 2)
		ch = 'r';

	for (search = fs_searchpaths; search; search = search->next)
	{
		if (strstr(search->filename, "xatrix"))
			return 1;

		if (strstr(search->filename, "rogue"))
			return 2;
/*
		start = strchr(search->filename, ch);

		if (start == NULL)
			continue;

		if (strcmp(start ,"xatrix") == 0)
			return(1);
*/
	}
	return(0);

}


int FS_FOpenFileLCase (const char *filename, FILE **file) // jitlinux
{
	qboolean bRetry = false;
	char filename_lcase[MAX_QPATH];
	char *s;

	Q_strncpyz(filename_lcase, filename, sizeof(filename_lcase));

	// Replace backslashes with forward slashes for linux compatibility
	while (s = strchr(filename_lcase, '\\'))
	{
		*s = '/';
		bRetry = true;
	}

	// Try again with fixed slashes
	if (bRetry)
		return FS_FOpenFile(filename_lcase, file);

	// Try again with an all-lowercase filename
	if (strtolower(filename_lcase))
		return FS_FOpenFile(filename_lcase, file);

	// Give up.  It really doesn't exist.
	Com_DPrintf("FindFile: Can't find %s.\n", filename);
	*file = NULL;
	return -1;
}


/*
===========
FS_FOpenFile

Finds the file in the search path.
returns filesize and an open FILE *
Used for streaming data out of either a pak file or
a seperate file.
===========
*/
int file_from_pak = 0;

int FS_FOpenFile (const char *filename, FILE **file)
{
	searchpath_t	*search;
	char			netpath[MAX_OSPATH];
	pack_t			*pak;
	int				i;
	filelink_t		*link;

	file_from_pak = 0;

	// check for links first
	for (link = fs_links; link; link=link->next)
	{
		if (!strncmp(filename, link->from, link->fromlength))
		{
			Com_sprintf(netpath, sizeof(netpath), "%s%s", link->to, filename + link->fromlength);
			*file = fopen(netpath, "rb");

			if (*file)
			{		
				Com_DPrintf("Link file: %s\n",netpath);
				return FS_filelength (*file);
			}

			return -1;
		}
	}

	// search through the path, one element at a time
	for (search = fs_searchpaths; search; search = search->next)
	{
		// is the element a pak file?
		if (search->pack)
		{
			// look through all the pak file elements
			pak = search->pack;

			for (i = 0; i < pak->numfiles; i++)
			{
				if (Q_strcaseeq(pak->files[i].name, filename))
				{	// found it!
					file_from_pak = 1;
					Com_DPrintf("PackFile: %s : %s\n", pak->filename, filename);
					// open a new file on the pakfile
					*file = fopen(pak->filename, "rb");

					if (!*file)
						Com_Error(ERR_FATAL, "Couldn't reopen %s.", pak->filename);	

					fseek(*file, pak->files[i].filepos, SEEK_SET);
					return pak->files[i].filelen;
				}
			}
		}
		else
		{
			// check a file in the directory tree
			Com_sprintf(netpath, sizeof(netpath), "%s/%s", search->filename, filename);
#ifndef WIN32 // jitsecurity - b42 - make sure linux doesn't try to open directories as files, as those don't read.
			{
				struct stat statbuf;
				if (stat(netpath, &statbuf) < 0)
				{
					continue; // file doesn't exist?
				}
				
				if (!S_ISREG(statbuf.st_mode))
				{
					continue; // not a regular file.  Don't try to open it (could be a directory or something else that would cause problems).
				}
			}
#endif
			*file = fopen(netpath, "rb");

			if (!*file)
				continue;
			
			Com_DPrintf("FindFile: %s\n",netpath);
			return FS_filelength(*file);
		}
	}

	return FS_FOpenFileLCase(filename, file); // jitlinux
}


/*
=================
FS_ReadFile

Properly handles partial reads
=================
*/
void CDAudio_Stop (void);
#define	MAX_READ	0x10000		// read in blocks of 64k
void FS_Read (void *buffer, int len, FILE *f)
{
#if 0
	int read;

	read = fread(buffer, 1, len, f); // jit - optimized code a little.

	if (read < len)
		Com_Error(ERR_FATAL, "FS_Read: %d bytes read, should have been %d.", read, len);

#else // old code -- seems this is necessary on Linux for some dumb reason?
	int		block, remaining;
	int		read;
	byte	*buf;
	int		tries;

	buf = (byte *)buffer;

	// read in chunks for progress bar
	remaining = len;
	tries = 0;

	while (remaining)
	{
		block = remaining;

		if (block > MAX_READ)
			block = MAX_READ;

		read = fread(buf, 1, block, f);

		if (read == 0)
		{
			// we might have been trying to read from a CD
			if (!tries)
			{
				tries = 1;
				CDAudio_Stop();
			}
			else
			{
				assert(0);
				Com_Error(ERR_FATAL, "FS_Read: 0 bytes read");
			}
		}

		if (read == -1)
		{
			assert(0);
			Com_Error(ERR_FATAL, "FS_Read: -1 bytes read");
		}

		// do some progress bar thing here...
		remaining -= read;
		buf += read;
	}
#endif
}

/*
============
FS_LoadFile

Filename are reletive to the quake search path
a null buffer will just return the file length without loading
============
*/

int FS_LoadFile (const char *path, void **buffer)
{
// ===
// jitfilemap
#if USEFILEMAP
	HANDLE hFileMap;
	HANDLE hFile;
	int len;
	char fullpath[_MAX_PATH];

	_makepath(fullpath, NULL, FS_Gamedir(), path, NULL);
	hFile = CreateFile(fullpath, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (!hFile || hFile == INVALID_HANDLE_VALUE)
	{
		if (buffer)
			*buffer = NULL;

		return -1;
	}

	len = GetFileSize(hFile, NULL);

	if (!buffer)
	{
		CloseHandle(hFile);
		return len;
	}

	hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

	if (!hFileMap || hFileMap == INVALID_HANDLE_VALUE)
	{
		// probably won't ever happen, but just in case.
		*buffer = NULL;
		CloseHandle(hFile);
		return -1;
	}

	*buffer = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
	CloseHandle(hFileMap);
	CloseHandle(hFile);

	if (e.i)
		e.i(path, *buffer, len);

	return len;

#else
	FILE	*h;
	byte	*buf;
	int		len;

	buf = NULL;	// quiet compiler warning
// look for it in the filesystem or pack files
	len = FS_FOpenFile(path, &h);

	if (!h)
	{
		if (buffer)
			*buffer = NULL;
		return -1;
	}
	
	if (!buffer)
	{
		fclose(h);
		return len;
	}

	buf = Z_Malloc(len);
	*buffer = buf;
	FS_Read(buf, len, h);
	fclose(h);

#ifndef DEDICATED_ONLY
	if (e.i)
		e.i(path, buf, len);
#endif

	return len;
#endif
// jitfilemap
// ===
}

int FS_LoadFileZ (const char *path, void **buffer) // jit - null-terminated FS_LoadFile
{
	FILE	*h;
	byte	*buf;
	int		len;

	buf = NULL;
	len = FS_FOpenFile(path, &h);

	if (!h)
	{
		if (buffer)
			*buffer = NULL;

		return -1;
	}
	
	if (!buffer)
	{
		fclose(h);
		return len;
	}

	buf = Z_Malloc(len + 1);
	*buffer = buf;
	FS_Read(buf, len, h);
	buf[len] = 0;
	fclose(h);

#ifndef DEDICATED_ONLY
	if (e.i)
		e.i(path, buf, len);
#endif

	return len;
}


/*
=============
FS_FreeFile
=============
*/
void FS_FreeFile (void *buffer)
{
#if(defined(WIN32) && USEFILEMAP) // jitest
	UnmapViewOfFile(buffer);
#else
	Z_Free(buffer);
#endif
}

/*
=================
FS_LoadPackFile

Takes an explicit (not game tree related) path to a pak file.

Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/
pack_t *FS_LoadPackFile (char *packfile)
{
	dpackheader_t	header;
	int				i;
	packfile_t		*newfiles;
	int				numpackfiles;
	pack_t			*pack;
	FILE			*packhandle;
	dpackfile_t		info[MAX_FILES_IN_PACK];
	unsigned		checksum;

	packhandle = fopen(packfile, "rb");
	if (!packhandle)
		return NULL;

	fread(&header, 1, sizeof(header), packhandle);
	if (LittleLong(header.ident) != IDPAKHEADER)
		Com_Error(ERR_FATAL, "%s is not a packfile.", packfile);
	header.dirofs = LittleLong(header.dirofs);
	header.dirlen = LittleLong(header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Com_Error(ERR_FATAL, "%s has %i files.", packfile, numpackfiles);

	newfiles = Z_Malloc(numpackfiles * sizeof (packfile_t));
	fseek(packhandle, header.dirofs, SEEK_SET);
	fread(info, 1, header.dirlen, packhandle);
	// crc the directory to check for modifications
	checksum = Com_BlockChecksum((void *)info, header.dirlen);

	// parse the directory
	for (i = 0; i < numpackfiles; i++)
	{
		Q_strncpyz(newfiles[i].name, info[i].name, sizeof(newfiles[i].name));
		newfiles[i].filepos = LittleLong(info[i].filepos);
		newfiles[i].filelen = LittleLong(info[i].filelen);
	}

	pack = Z_Malloc(sizeof(pack_t));
	strcpy(pack->filename, packfile);
	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;
	Com_Printf("Added packfile %s(%i files).\n", packfile, numpackfiles);

	return pack;
}


int SortList (const void *data1, const void *data2)
{
	const char **a = (const char **)data1;
    const char **b = (const char **)data2;

	return Q_strcasecmp(*a, *b);
}

/*
================
FS_AddGameDirectory

Sets fs_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ... 
================
*/
void FS_AddGameDirectory (char *dir)
{
	searchpath_t	*search;
	pack_t			*pak;
	char			dirstring[MAX_QPATH];
	int				pakfile_count, i, j;
	char			**pakfile_list;
	
#if defined (__unix__)
	// Create directory if it does not exist
	Sys_Mkdir(fs_gamedir);
	strcpy(fs_gamedir, dir);
#endif

	// add the directory to the search path
	search = Z_Malloc(sizeof(searchpath_t));
	strcpy(search->filename, dir);
	search->next = fs_searchpaths;
	fs_searchpaths = search;

	// Build up a list of pak files to add to our search path
	// jitpak - modified to find all pak files in windows an linux
	Com_sprintf(dirstring, sizeof(dirstring), "%s/*.pak", dir);

	for (j = 0; j < 1; j++)
	{
		if (!(pakfile_list = FS_ListFiles(dirstring, &pakfile_count, 0, 0, true)))
			return;

		// Add each pak file from our list to the search path
		for (i = 0; i < pakfile_count-1; i++)
		{
			pak = FS_LoadPackFile(pakfile_list[i]);
			free(pakfile_list[i]);  // jitpak

			if (!pak)
				continue;

			search = Z_Malloc(sizeof(searchpath_t));
			search->pack = pak;
			search->next = fs_searchpaths;
			fs_searchpaths = search;
		}

		free(pakfile_list); // jitpak
		Com_sprintf(dirstring, sizeof(dirstring), "%s/pakfiles/*.pak", dir); // jitpak
	}
}

#if defined (__unix__)
/*
================
FS_AddHomeAsGameDirectory

Use ~/.paintball2/dir as fs_gamedir.
================
*/
void FS_AddHomeAsGameDirectory(char *dir)
{
	char	gdir[MAX_OSPATH];
	char   *homedir = getenv("HOME");

	if (homedir) {
		int len = snprintf(gdir, sizeof(gdir), "%s/.paintball2/%s/", homedir, dir);

		Com_Printf("Using %s for writing.\n",gdir);
		FS_CreatePath (gdir);

		if ((len > 0) && (len < sizeof(gdir)) && (gdir[len-1] == '/'))
	    	gdir[len-1] = 0;

		strncpy(fs_gamedir,gdir,sizeof(fs_gamedir)-1);
		fs_gamedir[sizeof(fs_gamedir)-1] = 0;

		FS_AddGameDirectory (gdir);
	}
}
#endif

/*
============
FS_Gamedir

Called to find where to write a file (demos, savegames, etc)
============
*/
char *FS_Gamedir (void)
{
	if (*fs_gamedir && !Q_streq(fs_gamedir, "./")) // jit
		return fs_gamedir;
	else
		return BASEDIRNAME;
}

/*
=============
FS_ExecAutoexec
=============
*/
void FS_ExecAutoexec (void)
{
	char *dir;
	char name [MAX_QPATH];

	dir = Cvar_VariableString("gamedir");

	if (*dir)
		Com_sprintf(name, sizeof(name), "%s/%s/configs/autoexec.cfg", fs_basedir->string, dir); // jit
	else
		Com_sprintf(name, sizeof(name), "%s/%s/configs/autoexec.cfg", fs_basedir->string, BASEDIRNAME); // jit

	if (Sys_FindFirst(name, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM))
		Cbuf_AddText("exec autoexec.cfg\n"); // jit

	Sys_FindClose();
}

// ===
// jitconfig -- only exec default.cfg if config.cfg is not found!
void FS_ExecConfig (void)
{
	char *dir;
	char name[MAX_QPATH];

	dir = Cvar_VariableString("gamedir");

	if (*dir)
		Com_sprintf(name, sizeof(name), "%s/%s/configs/config.cfg", 
					fs_basedir->string, dir);
	else
		Com_sprintf(name, sizeof(name), "%s/%s/configs/config.cfg", 
					fs_basedir->string, BASEDIRNAME);

	if (Sys_FindFirst(name, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM))
	{
		Cbuf_AddText("exec config.cfg\n");
	}
	else
	{
		Cbuf_AddText("exec default.cfg\n");
	}

	Sys_FindClose();
}
// ===

/*
================
FS_SetGamedir

Sets the gamedir and path to a different directory.
================
*/
void FS_SetGamedir (char *dir)
{
	searchpath_t	*next;

	if (strstr(dir, "..") || strstr(dir, "/")
		|| strstr(dir, "\\") || strstr(dir, ":"))
	{
		Com_Printf("Gamedir should be a single filename, not a path.\n");
		return;
	}

	//
	// free up any current game dir info
	//
	while(fs_searchpaths != fs_base_searchpaths)
	{
		if (fs_searchpaths->pack)
		{
			fclose(fs_searchpaths->pack->handle);
			Z_Free(fs_searchpaths->pack->files);
			Z_Free(fs_searchpaths->pack);
		}
		next = fs_searchpaths->next;
		Z_Free(fs_searchpaths);
		fs_searchpaths = next;
	}

	//
	// flush all data, so it will be forced to reload
	//
	if (dedicated && !dedicated->value)
		Cbuf_AddText("vid_restart\nsnd_restart\n");

	Com_sprintf(fs_gamedir, sizeof(fs_gamedir), "%s/%s", fs_basedir->string, dir);

	if (Q_streq(dir, BASEDIRNAME) || (*dir == 0))
	{
		Cvar_FullSet("gamedir", BASEDIRNAME, CVAR_SERVERINFO | CVAR_NOSET, true); // jit, always display gamedir as "pball"
		Cvar_FullSet("game", BASEDIRNAME, CVAR_LATCH | CVAR_SERVERINFO, true); // jit
	}
	else
	{
		char szGameDir[MAX_QPATH];

		Cvar_FullSet("gamedir", dir, CVAR_SERVERINFO|CVAR_NOSET, true);

		if (fs_cddir->string[0])
		{
			Com_sprintf(szGameDir, sizeof(szGameDir), "%s/%s", fs_cddir->string, dir); // jit - fix bad case of string reuse.  va() is evil.
			FS_AddGameDirectory(szGameDir); // jit
		}

#if defined (LIBDIR)
		Com_sprintf(szGameDir, sizeof(szGameDir), "%s/%s", LIBDIR, dir); // jit - fix bad case of string reuse.  va() is evil.
		FS_AddGameDirectory(szGameDir); // jit
#endif

		Com_sprintf(szGameDir, sizeof(szGameDir), "%s/%s", fs_basedir->string, dir); // jit - fix bad case of string reuse.  va() is evil.
		FS_AddGameDirectory(szGameDir); // jit
#if defined (DATADIR)
		FS_AddHomeAsGameDirectory(dir);
#endif
	}
}


/*
================
FS_Link_f

Creates a filelink_t
================
*/
void FS_Link_f (void)
{
	filelink_t	*l, **prev;

	if (Cmd_Argc() != 3)
	{
		Com_Printf("Usage: link <from> <to>\n");
		return;
	}

	// see if the link already exists
	prev = &fs_links;
	for (l=fs_links; l; l=l->next)
	{
		if (Q_streq(l->from, Cmd_Argv(1)))
		{
			Z_Free(l->to);
			if (!strlen(Cmd_Argv(2)))
			{	// delete it
				*prev = l->next;
				Z_Free(l->from);
				Z_Free(l);
				return;
			}
			l->to = CopyString(Cmd_Argv(2));
			return;
		}
		prev = &l->next;
	}

	// create a new link
	l = Z_Malloc(sizeof(*l));
	l->next = fs_links;
	fs_links = l;
	l->from = CopyString(Cmd_Argv(1));
	l->fromlength = strlen(l->from);
	l->to = CopyString(Cmd_Argv(2));
}

/*
** FS_ListFiles
*/
char **FS_ListFiles (const char *findname, int *numfiles, unsigned musthave, unsigned canthave, qboolean sort)
{
	char *s;
	int nfiles = 0;
	char **list = 0;

	if (!findname || !*findname)
		return NULL;

	s = Sys_FindFirst(findname, musthave, canthave);

	while(s)
	{
		if (s[strlen(s)-1] != '.')
			nfiles++;

		s = Sys_FindNext(musthave, canthave);
	}

	Sys_FindClose();

	if (!nfiles)
		return NULL;

	nfiles++; // add space for a guard
	*numfiles = nfiles;
	list = malloc(sizeof(char*) * nfiles);
	memset(list, 0, sizeof(char*) * nfiles);
	s = Sys_FindFirst(findname, musthave, canthave);
	nfiles = 0;

	while(s)
	{
		if (s[strlen(s)-1] != '.')
		{
			list[nfiles] = strdup(s);
#ifdef _WIN32
			strlwr(list[nfiles]);
#endif
			nfiles++;
		}

		s = Sys_FindNext(musthave, canthave);
	}

	Sys_FindClose();
	qsort(list, nfiles, sizeof(char *), SortList);
	return list;
}


void FS_FreeFileList (char **list, int n) // jit
{
	int i;

	for (i = 0; i < n; i++)
	{
		if (list[i])
		{
			free(list[i]);
			list[i] = 0;
		}
	}

	free(list);
}

/*
 * FS_FreeList
 *
 * Free list of files created by FS_ListFiles().
 * viciouz - ogg support - similar but different, the other one causes a protection fault for some reason.
 */
void FS_FreeList(char **list, int nfiles)
{
	int		i;

	for (i = 0; i < nfiles - 1; i++)
		free(list[i]);

	free(list);
}


void FS_cwd_f (void)
{
	char dir[1024];

	getcwd(dir, sizeof(dir));
	Com_Printf("%s\n", dir);
}


/*
** FS_Dir_f
*/
void FS_Dir_f (void)
{
	char	*path = NULL;
	char	findname[1024];
	char	wildcard[1024] = "*.*";
	char	**dirnames;
	int		ndirs;

	if (Cmd_Argc() != 1)
	{
		strcpy(wildcard, Cmd_Argv(1));
	}

	while((path = FS_NextPath(path)) != NULL)
	{
		char *tmp = findname;

		Com_sprintf(findname, sizeof(findname), "%s/%s", path, wildcard);

		while(*tmp != 0)
		{
			if (*tmp == '\\') 
				*tmp = '/';

			tmp++;
		}

		Com_Printf("Directory of %s\n", findname);
		Com_Printf("----\n");

		if ((dirnames = FS_ListFiles(findname, &ndirs, 0, 0, true)) != 0)
		{
			int i;

			for (i = 0; i < ndirs-1; i++)
			{
				if (strrchr(dirnames[i], '/'))
					Com_Printf("%s\n", strrchr(dirnames[i], '/') + 1);
				else
					Com_Printf("%s\n", dirnames[i]);

				free(dirnames[i]);
			}
			free(dirnames);
		}
		Com_Printf("\n");
	};
}

/*
============
FS_Path_f

============
*/
void FS_Path_f (void)
{
	searchpath_t	*s;
	filelink_t		*l;

	Com_Printf("Current search path:\n");

	for (s = fs_searchpaths; s; s = s->next)
	{
		if (s == fs_base_searchpaths)
			Com_Printf("----------\n");

		if (s->pack)
			Com_Printf("%s(%i files)\n", s->pack->filename, s->pack->numfiles);
		else
			Com_Printf("%s\n", s->filename);
	}

	Com_Printf("\nLinks:\n");

	for (l = fs_links; l; l = l->next)
		Com_Printf("%s : %s\n", l->from, l->to);
}


/*
================
FS_NextPath

Allows enumerating all of the directories in the search path
================
*/
char *FS_NextPath (char *prevpath)
{
	searchpath_t	*s;
	char			*prev;

	if (!prevpath)
		return fs_gamedir;

	prev = fs_gamedir;
	for (s=fs_searchpaths; s; s=s->next)
	{
		if (s->pack)
			continue;
		if (prevpath == prev)
			return s->filename;
		prev = s->filename;
	}

	return NULL;
}


/*
================
FS_InitFilesystem
================
*/
void FS_InitFilesystem (void)
{
	Cmd_AddCommand("path", FS_Path_f);
// jitsecurity -- disabled:	Cmd_AddCommand("link", FS_Link_f);
	Cmd_AddCommand("dir", FS_Dir_f);
	Cmd_AddCommand("pwd", FS_cwd_f); // jit
	Cmd_AddCommand("cwd", FS_cwd_f); // jit

	//
	// basedir <path>
	// allows the game to run from outside the data tree
	//
#if defined (DATADIR)
	fs_basedir = Cvar_Get("basedir", DATADIR, CVAR_NOSET);
#else
	fs_basedir = Cvar_Get("basedir", ".", CVAR_NOSET);
#endif

	//
	// cddir <path>
	// Logically concatenates the cddir after the basedir for 
	// allows the game to run from outside the data tree
	//
	fs_cddir = Cvar_Get("cddir", "", CVAR_NOSET);
	if (fs_cddir->string[0])
		FS_AddGameDirectory(va("%s/"BASEDIRNAME, fs_cddir->string));

	//
	// start up with baseq2 by default
	//
#if defined (LIBDIR)
	FS_AddGameDirectory(va("%s/"BASEDIRNAME, LIBDIR));
#endif
	FS_AddGameDirectory(va("%s/"BASEDIRNAME, fs_basedir->string));
#if defined (DATADIR)
	FS_AddHomeAsGameDirectory(BASEDIRNAME);
#endif

	// any set gamedirs will be freed up to here
	fs_base_searchpaths = fs_searchpaths;

	// check for game override
	fs_gamedirvar = Cvar_Get("game", BASEDIRNAME, CVAR_LATCH | CVAR_SERVERINFO); // jit

	if (fs_gamedirvar->string[0])
		FS_SetGamedir(fs_gamedirvar->string);
}



