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

#include "g_local.h"


void	Svcmd_Test_f (void)
{
	safe_cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

writeip
Dumps "addip <ip>" commands to listip.cfg so it can be execed at a later date.  The filter lists are not saved and restored by default, because I beleive it would cause too much confusion.

filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

typedef struct
{
	unsigned	mask;
	unsigned	compare;
} ipfilter_t;

#define	MAX_IPFILTERS	1024

ipfilter_t	ipfilters[MAX_IPFILTERS];
int			numipfilters;

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipfilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			safe_cprintf(NULL, PRINT_HIGH, "Bad filter address: %s\n", s);
			return false;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return true;
}

/*
=================
SV_FilterPacket
=================
*/
qboolean SV_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4];
	char *p;

	i = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned *)m;

	for (i=0 ; i<numipfilters ; i++)
		if ( (in & ipfilters[i].mask) == ipfilters[i].compare)
			return (int)filterban->value;

	return (int)!filterban->value;
}


/*
=================
SV_AddIP_f
=================
*/
void SVCmd_AddIP_f (void)
{
	int		i;
	
	if (gi.argc() < 3) {
		safe_cprintf(NULL, PRINT_HIGH, "Usage:  addip <ip-mask>\n");
		return;
	}

	for (i=0 ; i<numipfilters ; i++)
		if (ipfilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numipfilters)
	{
		if (numipfilters == MAX_IPFILTERS)
		{
			safe_cprintf (NULL, PRINT_HIGH, "IP filter list is full\n");
			return;
		}
		numipfilters++;
	}
	
	if (!StringToFilter (gi.argv(2), &ipfilters[i]))
		ipfilters[i].compare = 0xffffffff;
}

/*
=================
SV_RemoveIP_f
=================
*/
void SVCmd_RemoveIP_f (void)
{
	ipfilter_t	f;
	int			i, j;

	if (gi.argc() < 3) {
		safe_cprintf(NULL, PRINT_HIGH, "Usage:  sv removeip <ip-mask>\n");
		return;
	}

	if (!StringToFilter (gi.argv(2), &f))
		return;

	for (i=0 ; i<numipfilters ; i++)
		if (ipfilters[i].mask == f.mask
		&& ipfilters[i].compare == f.compare)
		{
			for (j=i+1 ; j<numipfilters ; j++)
				ipfilters[j-1] = ipfilters[j];
			numipfilters--;
			safe_cprintf (NULL, PRINT_HIGH, "Removed.\n");
			return;
		}
	safe_cprintf (NULL, PRINT_HIGH, "Didn't find %s.\n", gi.argv(2));
}

/*
=================
SV_ListIP_f
=================
*/
void SVCmd_ListIP_f (void)
{
	int		i;
	byte	b[4];

	safe_cprintf (NULL, PRINT_HIGH, "Filter list:\n");
	for (i=0 ; i<numipfilters ; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		safe_cprintf (NULL, PRINT_HIGH, "%3i.%3i.%3i.%3i\n", b[0], b[1], b[2], b[3]);
	}
}

/*
=================
SV_WriteIP_f
=================
*/
void SVCmd_WriteIP_f (void)
{
	FILE	*f;
	char	name[MAX_OSPATH];
	byte	b[4];
	int		i;
	cvar_t	*game;

	game = gi.cvar("game", "", 0);

	if (!*game->string)
		sprintf (name, "%s/listip.cfg", GAMEVERSION);
	else
		sprintf (name, "%s/listip.cfg", game->string);

	safe_cprintf (NULL, PRINT_HIGH, "Writing %s.\n", name);

	f = fopen (name, "wb");
	if (!f)
	{
		safe_cprintf (NULL, PRINT_HIGH, "Couldn't open %s\n", name);
		return;
	}
	
	fprintf(f, "set filterban %d\n", (int)filterban->value);

	for (i=0 ; i<numipfilters ; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		fprintf (f, "sv addip %i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
	}
	
	fclose (f);
}

/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
/*void stuffcmd(edict_t *pent, char *pszCommand)
{
	gi.WriteByte(svc_stufftext);
	gi.WriteString(pszCommand);
	gi.unicast(pent, true);
}*/

void SVCmd_StuffCmd_f (void)
{
	int i;
	char *player_name = gi.argv(2);
	char *command = gi.argv(3);
	char *s;
	edict_t *player;
	char userinfo[MAX_INFO_STRING];
	int worked = 0;

	for (i = 1; i <= maxclients->value; i++) 
	{
		player = g_edicts + i;

		memcpy (userinfo, player->client->pers.userinfo, sizeof(userinfo));
		s = Info_ValueForKey (userinfo, "name");

		if (Q_stricmp(s, player_name) == 0)
		{
			stuffcmd (player, command);
			worked = 1;
		}
	}

	if (!worked)
	{
		gi.dprintf ("Player not found: %s\n", player_name);
		return;
	}
}

char *InverseAlpha (char *alpha)
{
	if (Q_stricmp(alpha, "a") == 0)
		return "z";
	else if (Q_stricmp(alpha, "b") == 0)
		return "y";
	else if (Q_stricmp(alpha, "c") == 0)
		return "x";
	else if (Q_stricmp(alpha, "d") == 0)
		return "w";
	else if (Q_stricmp(alpha, "e") == 0)
		return "v";
	else if (Q_stricmp(alpha, "f") == 0)
		return "u";
	else if (Q_stricmp(alpha, "g") == 0)
		return "t";
	else if (Q_stricmp(alpha, "h") == 0)
		return "s";
	else if (Q_stricmp(alpha, "i") == 0)
		return "r";
	else if (Q_stricmp(alpha, "j") == 0)
		return "q";
	else if (Q_stricmp(alpha, "k") == 0)
		return "p";
	else if (Q_stricmp(alpha, "l") == 0)
		return "o";
	else if (Q_stricmp(alpha, "m") == 0)
		return "n";
	else if (Q_stricmp(alpha, "z") == 0)
		return "a";
	else if (Q_stricmp(alpha, "y") == 0)
		return "b";
	else if (Q_stricmp(alpha, "x") == 0)
		return "c";
	else if (Q_stricmp(alpha, "w") == 0)
		return "d";
	else if (Q_stricmp(alpha, "v") == 0)
		return "e";
	else if (Q_stricmp(alpha, "u") == 0)
		return "f";
	else if (Q_stricmp(alpha, "t") == 0)
		return "g";
	else if (Q_stricmp(alpha, "s") == 0)
		return "h";
	else if (Q_stricmp(alpha, "r") == 0)
		return "i";
	else if (Q_stricmp(alpha, "a") == 0)
		return "j";
	else if (Q_stricmp(alpha, "p") == 0)
		return "k";
	else if (Q_stricmp(alpha, "o") == 0)
		return "l";
	else if (Q_stricmp(alpha, "n") == 0)
		return "m";

	return NULL;
}

void	ServerCommand (void)
{
	char	*cmd;

	// JABot[start]
//	if (BOT_ServerCommand ())
//		return;
	// [end]

	cmd = gi.argv(1);
	if (Q_stricmp (cmd, "test") == 0)
		Svcmd_Test_f ();
	else if (Q_stricmp (cmd, "addip") == 0)
		SVCmd_AddIP_f ();
	else if (Q_stricmp (cmd, "removeip") == 0)
		SVCmd_RemoveIP_f ();
	else if (Q_stricmp (cmd, "listip") == 0)
		SVCmd_ListIP_f ();
	else if (Q_stricmp (cmd, "writeip") == 0)
		SVCmd_WriteIP_f ();
	else if (Q_stricmp(cmd, "stuffcmd") == 0)
	{
		SVCmd_StuffCmd_f ();
	}
	// Paril: Bot
	else if (Q_stricmp(cmd, "bot") == 0)
		Svcmd_Bot_f();
	else if (Q_stricmp(cmd, "reset_daynight") == 0)
	{
		// 0 normal
		gi.configstring(CS_LIGHTS+0, "m");
		
		// 1 FLICKER (first variety)
		gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");
		
		// 2 SLOW STRONG PULSE
		gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
		
		// 3 CANDLE (first variety)
		gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
		
		// 4 FAST STROBE
		gi.configstring(CS_LIGHTS+4, "mamamamamama");
		
		// 5 GENTLE PULSE 1
		gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
		
		// 6 FLICKER (second variety)
		gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");
		
		// 7 CANDLE (second variety)
		gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");
		
		// 8 CANDLE (third variety)
		gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
		
		// 9 SLOW STROBE (fourth variety)
		gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");
		
		// 10 FLUORESCENT FLICKER
		gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");
		
		// 11 SLOW PULSE NOT FADE TO BLACK
		gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");
		
		// styles 32-62 are assigned by the light program for switchable lights
		
		// 63 testing
		gi.configstring(CS_LIGHTS+63, "a");
	}
	else
		safe_cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

