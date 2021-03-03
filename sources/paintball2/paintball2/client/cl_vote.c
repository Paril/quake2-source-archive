/*
Copyright (C) 2004 Nathan Wulf and Digital Paint

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

// cl_vote.c -- handle map voting menus

// ===
// jitvote
#include "client.h"
#include "menu.h"

#define MAX_MAPNAME_LEN    32
#define MAX_MAPLIST_ITEMS  256
#define MAX_MAPLIST_MODES  8 // default, dm, 1flag, ctf, koth, elim, ...

#define MAPLIST_FLAGS_FIRST_MAP  0x01
#define MAPLIST_FLAGS_TEMP       0x02
#define MAPLIST_FLAGS_UNPLAYABLE 0x04
#define MAPLIST_FLAGS_LAST_MAP   0x08

typedef struct CL_MAPLIST_ITEM_S {
	unsigned int votes;
	unsigned int gamemode_flags;
	unsigned int maplist_flags;
	unsigned char mapname[MAX_MAPNAME_LEN];
	struct CL_MAPLIST_ITEM_S *next;
} cl_maplist_item_t;

cl_maplist_item_t *cl_maplist_root = NULL;
cl_maplist_item_t *cl_maplist_tail = NULL;

char **cl_maplist_info = NULL;
char **cl_maplist_names = NULL;
char **cl_maplist_modes = NULL;
int cl_maplist_count = 0;
int cl_maplist_modes_count = 0;

cvar_t *menu_votemap;

#define MAX_DECODE_ARRAY 256
static unsigned int temp_array[MAX_DECODE_ARRAY];

#define GAMEMODE_DM				1
#define GAMEMODE_1FLAG			2
#define GAMEMODE_2FLAG			4
#define GAMEMODE_SIEGE			8
#define GAMEMODE_KOTH			16
#define GAMEMODE_ELIM			32
#define GAMEMODE_PONG			64
#define GAMEMODE_TDM			128


void CL_UpdateMaplistModes (void)
{
	cl_maplist_item_t *finger;

	for (finger = cl_maplist_root; finger; finger = finger->next)
	{
		if (Q_streq(finger->mapname, menu_votemap->string))
		{
			register int i = 1;

			if (finger->gamemode_flags & GAMEMODE_ELIM)
				cl_maplist_modes[i++] = "Elim";
			if (finger->gamemode_flags & GAMEMODE_DM)
				cl_maplist_modes[i++] = "DM";
			if (finger->gamemode_flags & (GAMEMODE_DM | GAMEMODE_TDM))
				cl_maplist_modes[i++] = "TDM";
			if (finger->gamemode_flags & GAMEMODE_1FLAG)
				cl_maplist_modes[i++] = "1-Flag";
			if (finger->gamemode_flags & GAMEMODE_2FLAG)
				cl_maplist_modes[i++] = "CTF";
			if (finger->gamemode_flags & GAMEMODE_SIEGE)
				cl_maplist_modes[i++] = "Siege";
			if (finger->gamemode_flags & GAMEMODE_KOTH)
				cl_maplist_modes[i++] = "KOTH";
			if (finger->gamemode_flags & GAMEMODE_PONG)
				cl_maplist_modes[i++] = "Pong";

			cl_maplist_modes_count = i;
			return;
		}
	}

	// didn't find it:
	cl_maplist_modes_count = 1; // just use the default "------"
}

static void update_maplist_widget_arrays (void)
{
	cl_maplist_item_t *finger;
	register int i;
	char buf[48];

	for (finger = cl_maplist_root, i = 0; finger && i < MAX_MAPLIST_ITEMS; i++, finger = finger->next)
	{
		buf[0] = 0;

		if (cl_maplist_names[i])
			Z_Free(cl_maplist_names[i]);

		if (cl_maplist_info[i])
			Z_Free(cl_maplist_info[i]);

		if (finger->maplist_flags & MAPLIST_FLAGS_UNPLAYABLE)
			sprintf(buf, "%c4", SCHAR_COLOR);

		if (finger->maplist_flags & MAPLIST_FLAGS_TEMP)
			sprintf(buf, "%s%c", buf, SCHAR_ITALICS);

		sprintf(buf, "%s%2d %s", buf, finger->votes, finger->mapname);
		cl_maplist_info[i] = CopyString(buf);
		cl_maplist_names[i] = CopyString(finger->mapname);
	}

	cl_maplist_count = i;
	//pthread_mutex_unlock(&m_mut_widgets); // hack to keep mutexes from locking up - jitodo - this shouldn't even be locked at this point. :/
	M_RefreshWidget("vote_map", false);
	//pthread_mutex_lock(&m_mut_widgets);
}

static void clear_maplist (void)
{
	cl_maplist_item_t *finger, *temp;
	for (finger = cl_maplist_root; finger; )
	{
		temp = finger->next;
		Z_Free(finger);
		finger = temp;
	}

	cl_maplist_root = cl_maplist_tail = NULL;
}

#define MAPLIST_FLAGS_FIRST_MAP  0x01
#define MAPLIST_FLAGS_TEMP       0x02
#define MAPLIST_FLAGS_UNPLAYABLE 0x04
#define MAPLIST_FLAGS_LAST_MAP   0x08

// format: variable reserved, null, variable mapname, null, gamemode flags, votes, maplist flags
void CL_ParseMaplistData (const unsigned char *data)
{
	register unsigned int i=0, j, count, len;
	cl_maplist_item_t *newnode;

	count = decode_unsigned(data, temp_array, MAX_DECODE_ARRAY);

	while (i < count)
	{
		len = temp_array[i++];
		newnode = Z_Malloc(sizeof(cl_maplist_item_t));
		memset(newnode, 0, sizeof(cl_maplist_item_t));

		// copy map name
		j = 0;
		while ((newnode->mapname[j++] = temp_array[i++]))
			len--;
		len--;

		newnode->gamemode_flags = temp_array[i++];
		newnode->votes = temp_array[i++];
		newnode->maplist_flags = temp_array[i++];
		len -= 3;

		if (newnode->maplist_flags & MAPLIST_FLAGS_FIRST_MAP)
		{
			// Print to console if user manually typed the command
			if (cls.key_dest == key_console)
				Com_Printf("-------------------------------\n"); // arbitrary length - adjust if needed.

			clear_maplist();
		}

		
		// Print to console if user manually typed the command
		if (cls.key_dest == key_console)
		{
			if (newnode->maplist_flags & MAPLIST_FLAGS_UNPLAYABLE)
				Com_Printf("%c5", SCHAR_COLOR);

			if (newnode->maplist_flags & MAPLIST_FLAGS_TEMP)
				Com_Printf("(%3d %s)\n", newnode->votes, newnode->mapname);
			else
				Com_Printf("%4d %s\n", newnode->votes, newnode->mapname);

			if (newnode->maplist_flags & MAPLIST_FLAGS_LAST_MAP)
				Com_Printf("-------------------------------\n"); // arbitrary length - adjust if needed.
		}

		// skip past any extra data (leave room for future expansion)
		while (len > 0)
		{
			len--;
			i++;
		}

		// add new node to the tail of the list
		if (!cl_maplist_tail)
			cl_maplist_root = cl_maplist_tail = newnode;
		else
			cl_maplist_tail = cl_maplist_tail->next = newnode;
	}

	update_maplist_widget_arrays();
}

static void getmaplist (void)
{
	if (Cmd_Argc() >= 2)
		Cbuf_AddText(va("cmd getmaplist %s\n", Cmd_Argv(1)));
	else
		Cbuf_AddText("cmd getmaplist\n");
}

void init_cl_vote (void)
{
	menu_votemap = Cvar_Get("menu_votemap", "midnight2", 0);
	cl_maplist_info = Z_Malloc(sizeof(char*)*MAX_MAPLIST_ITEMS);
	cl_maplist_names = Z_Malloc(sizeof(char*)*MAX_MAPLIST_ITEMS);
	cl_maplist_modes = Z_Malloc(sizeof(char*)*MAX_MAPLIST_MODES);
	memset(cl_maplist_info, 0, sizeof(char*)*MAX_MAPLIST_ITEMS);
	memset(cl_maplist_names, 0, sizeof(char*)*MAX_MAPLIST_ITEMS);
	memset(cl_maplist_modes, 0, sizeof(char*)*MAX_MAPLIST_MODES);
	cl_maplist_modes[0] = "------";
	Cmd_AddCommand("getmaplist", getmaplist);
	Cmd_AddCommand("maplist", getmaplist);
}

void shutdown_cl_vote (void)
{
	// jitodo - need to free each element here... need to actually CALL this function.

	if (cl_maplist_info)
		Z_Free(cl_maplist_info);
	
	if (cl_maplist_names)
		Z_Free(cl_maplist_names);

	if (cl_maplist_modes)
		Z_Free(cl_maplist_modes);
}

// jitvote
// ===

