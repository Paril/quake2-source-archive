
#include "g_local.h"

/**
 * Switch the team of a player 
 * 
 * argv(1) switch
 *
 * FIXME: both atoi() below are not checked for errors,
 * so if bogus text is entered, the value 0 is used
 */
void Svcmd_Switch_f (void)
{
	edict_t *player;
	int playerNum, team;

	if (!(expflags & EXPERT_ENFORCED_TEAMS)) {
		gi.cprintf(NULL, PRINT_HIGH, "Enforced Teamplay not enabled\n");
		return;
	}	

	if (gi.argc() != 4) {
		gi.cprintf(NULL, PRINT_HIGH, "switch: Usage: switch [playernum] [team]\n");
		return;
	}

	playerNum = atoi(gi.argv(2));
	if (playerNum < 0 || playerNum > (int)maxclients->value - 1) {
		gi.cprintf(NULL, PRINT_HIGH, "switch: invalid player number: %d\n", playerNum);
		return;
	}
	player = g_edicts + 1 + playerNum;	
	if (!player->inuse) {
		gi.cprintf(NULL, PRINT_HIGH, "switch: player %d is not active\n", playerNum);
		return;
	}
	if (!playerIsOnATeam(player)) {
		gi.cprintf(NULL, PRINT_HIGH, "switch: player %d is not assigned to a team\n", playerNum);
		return;
	}

	// allow either team name or team number to be used
	team = teamForName(gi.argv(3));
	if (team == -1) {
		// invalid team name; assume number
		team = atoi(gi.argv(3));
		if (team < 0 || team > (int)sv_numteams->value - 1) {
			gi.cprintf(NULL, PRINT_HIGH, "switch: %s is neither a valid "
			                             "team name nor team number\n", gi.argv(3));
			return;
		}
	}
	if (player->client->resp.team == team) {
		gi.cprintf(NULL, PRINT_HIGH, "switch: player %s is already on team %d\n", 
		           player->client->pers.netname, team);
		return;
	}
	killAndSwitchTeam(player, team);
	gi.cprintf(NULL, PRINT_HIGH, "switch: player %d switched to team %d\n", 
	           playerNum, team);
	
	gi.bprintf(PRINT_HIGH, "%s was switched to team %s\n",
	           player->client->pers.netname, nameForTeam(team));
}

/**
 * Clear all properties
 * 
 * argv(1) clearprops
 */
void Svcmd_ClearProps_f (void)
{
	if (gi.argc() != 2) {
		gi.cprintf(NULL, PRINT_HIGH, "clearprops: Usage: clearprops\n");
	} else {
		gi.cprintf(NULL, PRINT_HIGH, "All properties cleared\n");
		freeProps(gProperties);
		gProperties = newProps();
	}
}

/**
 * Set, change or view global properties
 * 
 * argv(1) prop
 */
void Svcmd_Prop_f (void) {

	if (gi.argc() > 4 || gi.argc() < 2) {
		gi.cprintf(NULL, PRINT_HIGH, "prop: Usage: prop [property] [value]\n");
		return;
	}
	
	if (gi.argc() == 2) {
		// show all global properties
		gi.cprintf(NULL, PRINT_HIGH, "Properties defined:\n");
		printProps(gProperties);
		return;
	} else if (gi.argc() == 3) {
		// display the value of a particular property
		char *value = getProp(gProperties, gi.argv(2)); 
		gi.cprintf(NULL, PRINT_HIGH, "\"%s\" is %s\n", gi.argv(2), value);
		return;
	} else { // gi.argc() == 4
		// set a new property (or override an old one)
		if (Q_stricmp("", gi.argv(3)) == 0) {
			// user entered : sv prop someprop ""
			// treat as removing a property
			removeProp(gProperties, gi.argv(2));
			gi.cprintf(NULL, PRINT_HIGH, "Removed property \"%s\"\n", gi.argv(2)); 
		} else {
			// user entered either a new prop or a real value
			// with which to override an old one
			addProp(gProperties, gi.argv(2), gi.argv(3));
			gi.cprintf(NULL, PRINT_HIGH, "Property \"%s\" is now \"%s\"\n", gi.argv(2), gi.argv(3));
		}
		return;
	}
}

/**
 * Send a stuffcmd to all players.  Use to do things
 * like move all players to another server.
 */
void Svcmd_StuffAll_f (void) 
{
	edict_t *player;
	int i;
	char text[2048];

	if (gi.argc() != 3) {
		gi.cprintf(NULL, PRINT_HIGH, "stuffall: Usage: stuffall \"command text\"\n");
		return;
	}

	gi.cprintf(NULL, PRINT_HIGH, "Stuffing \"%s\" to all players\n", gi.argv(2));

	strcat(text, gi.argv(2));
	strcat(text + strlen(gi.argv(2)), "\n");

	player = g_edicts + 1;
	for (i = 1; i <= maxclients->value; i++) {
		if (player->inuse)
		{
			StuffCmd(player, text);
		}
		player = player + 1;
	}

}

/**
 * Send a stuffcmd to a single player for entertainment purposes.
 */
void Svcmd_Stuff_f (void) 
{
	edict_t *player;
	int playerNum;
	char text[2048];

	if (gi.argc() != 4) {
		gi.cprintf(NULL, PRINT_HIGH, "stuff: Usage: stuff [playernum] \"command text\"\n");
		return;
	}

	playerNum = atoi(gi.argv(2));
	if (playerNum < 0 || playerNum > (int)maxclients->value - 1) {
		gi.cprintf(NULL, PRINT_HIGH, "stuff: invalid player number: %d\n", playerNum);
		return;
	}

	player = g_edicts + 1 + playerNum;	
	if (!player->inuse) {
		gi.cprintf(NULL, PRINT_HIGH, "stuff: player %d is not active\n", playerNum);
		return;
	}

	gi.cprintf(NULL, PRINT_HIGH, "Stuffing \"%s\" to player %s\n", 
			gi.argv(3), player->client->pers.netname);

	strcat(text, gi.argv(3));
	strcat(text + strlen(gi.argv(3)), "\n");

	StuffCmd(player, text);
}

/**
 * Set or unset a flag in expflags
 * 
 * argv(1) set
 */
void Svcmd_ExpflagsSet_f (void)
{
	char* settingName;
	int settingBit;

	if (gi.argc() < 3 || gi.argc() > 4) {
		gi.cprintf(NULL, PRINT_HIGH, "set: Usage: set \"flagname\" [off|on]\n");
		return;
	} 

	settingName = gi.argv(2);
	settingBit = getSettingBit(settingName);
	if (settingBit <= 0) {
		gi.cprintf(NULL, PRINT_HIGH, "Unrecognized setting \"%s\"\n", settingName);
		return;
	}

	if (gi.argc() == 3) {
		// used form "sv set setting", which means turn setting on
		gi.cvar_set("expflags", va("%d", expflags | settingBit));
		gi.cprintf(NULL, PRINT_HIGH, "Setting \"%s\" is now enabled\n", settingName);
	} else {
		// used form "sv set setting off"
		if (Q_stricmp("off", gi.argv(3)) == 0) {
			gi.cvar_set("expflags", va("%d", expflags & ~settingBit));
			gi.cprintf(NULL, PRINT_HIGH, "Setting \"%s\" is now disabled\n", settingName);
		} else if (Q_stricmp("on", gi.argv(3)) == 0) {
			gi.cvar_set("expflags", va("%d", expflags | settingBit));
			gi.cprintf(NULL, PRINT_HIGH, "Setting \"%s\" is now enabled\n", settingName);
		} else {
			gi.cprintf(NULL, PRINT_HIGH, "set: Usage: set \"flagname\" [off|on]\n");
		}
	}
}

// Pause/unpause a multiplayer game
void Svcmd_Pause_f (void)
{
	if ((int) sv_paused->value == 0) {
		// Don't pause the game in an intermission
		if (level.intermissiontime) {
			gi.cprintf(NULL, PRINT_HIGH, "Can't pause in an intermission.\n");
		}
		sv_paused->value = 1;
		// Display a pause message to all players
		gi.bprintf(PRINT_HIGH, "Game paused by console\n");
		// FIXME: Display a pause graphic to all players like in single-player
	} else {
		sv_paused->value = 0;
		// Display an unpause message to all players
		gi.bprintf(PRINT_HIGH, "Game unpaused\n");
		// FIXME: Turn off the pause graphic?
	}
}

// Pause/unpause a multiplayer game
void Svcmd_ClearPass_f (void)
{
	gi.AddCommandString("password \"\"");
}

/*
 * IP Filtering Code
 * Copied from id 3.20 sources (g_svcmds.c) 12/18/98
 */

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
			gi.cprintf(NULL, PRINT_HIGH, "Bad filter address: %s\n", s);
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
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  addip <ip-mask>\n");
		return;
	}

	for (i=0 ; i<numipfilters ; i++)
		if (ipfilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numipfilters)
	{
		if (numipfilters == MAX_IPFILTERS)
		{
			gi.cprintf (NULL, PRINT_HIGH, "IP filter list is full\n");
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
		gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv removeip <ip-mask>\n");
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
			gi.cprintf (NULL, PRINT_HIGH, "Removed.\n");
			return;
		}
	gi.cprintf (NULL, PRINT_HIGH, "Didn't find %s.\n", gi.argv(2));
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

	gi.cprintf (NULL, PRINT_HIGH, "Filter list:\n");
	for (i=0 ; i<numipfilters ; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		gi.cprintf (NULL, PRINT_HIGH, "%3i.%3i.%3i.%3i\n", b[0], b[1], b[2], b[3]);
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

	gi.cprintf (NULL, PRINT_HIGH, "Writing %s.\n", name);

	f = fopen (name, "wb");
	if (!f)
	{
		gi.cprintf (NULL, PRINT_HIGH, "Couldn't open %s\n", name);
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
 * End of IP Filtering Code
 */


/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void	ServerCommand (void)
{
	char	*cmd;

	cmd = gi.argv(1);
//	if (Q_stricmp (cmd, "loadprops") == 0)
//		Svcmd_LoadProps_f ();
	if (Q_stricmp (cmd, "prop") == 0)
		Svcmd_Prop_f ();
	else if (Q_stricmp (cmd, "clearprops") == 0)
		Svcmd_ClearProps_f ();
	else if (Q_stricmp (cmd, "switch") == 0)
		Svcmd_Switch_f ();
	else if (Q_stricmp (cmd, "stuff") == 0)
		Svcmd_Stuff_f();
	else if (Q_stricmp (cmd, "stuffall") == 0)
		Svcmd_StuffAll_f();
	else if (Q_stricmp (cmd, "set") == 0)
		Svcmd_ExpflagsSet_f();
	// Expert: alternate way to clear password
	else if (Q_stricmp (cmd, "clearpass") == 0)
		Svcmd_ClearPass_f();
	// Expert: Pause/unpause the game
	else if (Q_stricmp (cmd, "pause") == 0)
		Svcmd_Pause_f();
	// Expert: IP Filtering code from id 3.20 sources
	else if (Q_stricmp (cmd, "addip") == 0)
		SVCmd_AddIP_f ();
	else if (Q_stricmp (cmd, "removeip") == 0)
		SVCmd_RemoveIP_f ();
	else if (Q_stricmp (cmd, "listip") == 0)
		SVCmd_ListIP_f ();
	else if (Q_stricmp (cmd, "writeip") == 0)
		SVCmd_WriteIP_f ();	
	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

