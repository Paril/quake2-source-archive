
#include "g_local.h"


void	Svcmd_Test_f (void)
{
	safe_cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

void cmd_Save_f (void)
{
	counter_t		i = 0, j = 0;
	float			loc_x, loc_y, loc_z;
	char			file_name[256];
	char			game_dir_name[80];
	cvar_t			*game_dir, *basedir;

	counter_t		link;
	distance_t		dist;
	FILE			*output;
	counter_t		num_nodes = node_count;
	int			ver, flags;

	ver = NODEFILE_VERSION;

	game_dir = gi.cvar ("game", "", 0);
	basedir = gi.cvar ("basedir", ".", 0);

	if (!Q_stricmp (game_dir->string, ""))
		sprintf (game_dir_name, "baseq2");
	else 
		sprintf (game_dir_name, "%s", game_dir->string);

	sprintf (file_name, "%s\\%s\\routes\\%s.dat", basedir->string, game_dir_name, level.mapname);

	output = fopen (file_name, "wb");

	if (!output)
	{
		safe_bprintf (PRINT_MEDIUM, "Error opening file\n");
		return;
	}

	safe_bprintf (PRINT_MEDIUM, "Saving file with %d nodes\n", num_nodes);

	fwrite (&ver, sizeof(int), 1, output);

	fwrite (&num_nodes, sizeof(counter_t), 1, output);

	for (i = 0; i < num_nodes; i++)
	{
		loc_x = node_list[i][0];
		loc_y = node_list[i][1];
		loc_z = node_list[i][2];
		
		fwrite (&loc_x, sizeof (float), 1, output);
		fwrite (&loc_y, sizeof (float), 1, output);
		fwrite (&loc_z, sizeof (float), 1, output);

		flags = node_flags[i];

		fwrite (&flags, sizeof (int), 1, output);

		for (j = 0; j < num_nodes; j++)
		{
			if (distance_table[i][j] < INFINITY)
			{
				link = j;
				fwrite (&link, sizeof (counter_t), 1, output);

				dist = distance_table[i][j];

				fwrite (&dist, sizeof (distance_t), 1, output);
			}
		}
		link = NO_NODES;
		fwrite (&link, sizeof (counter_t), 1, output);
	}

	fclose (output);
}

void cmd_Load_f (void)
{
	counter_t		i = 0, j = 0;
	float			loc_x, loc_y, loc_z;
	char			file_name[256];
	char			game_dir_name[80];
	cvar_t			*game_dir, *basedir;
	int			ver, flags;
	counter_t		link;
	distance_t		dist;
	FILE			*input;
	counter_t		num_nodes = node_count;

	game_dir = gi.cvar ("game", "", 0);
	basedir = gi.cvar ("basedir", ".", 0);

	if (!Q_stricmp (game_dir->string, ""))
		sprintf (game_dir_name, "baseq2");
	else 
		sprintf (game_dir_name, "%s", game_dir->string);

	sprintf (file_name, "%s\\%s\\routes\\%s.dat", basedir->string, game_dir_name, level.mapname);

	input = fopen (file_name, "rb");

	if (!input)
	{
		gi.dprintf (".....route file for %s not found\n", level.mapname);
//		read_map ();
		if(deathmatch->value)
		{
			path_time = 0;
			gi.dprintf (".....mapping turned off\n");
		}
		{
			path_time = 150;
			gi.dprintf (".....mapping turned on\n");
		}
		return;
	}

	fread (&ver, sizeof(int), 1, input);
	if(ver != NODEFILE_VERSION)
	{
		fclose(input);
		gi.dprintf("ERROR: Nodefile is incorrect version\n");
		return;
	}

	fread (&num_nodes, sizeof(counter_t), 1, input);

	gi.dprintf (".....route file for %s found\n", level.mapname);
	gi.dprintf (".....loading file with %d nodes\n", num_nodes);

	if (num_nodes < 1)
	{
		fclose (input);
		gi.dprintf ("\nError loading file with %d nodes\n", num_nodes);
		gi.dprintf ("Mapping automatically turned on\n");

		node_count = 0;
		return;			
	}

	node_count = num_nodes;

	for (i = 0; i < num_nodes; i++)
	{
		fread (&loc_x, sizeof (float), 1, input);
		fread (&loc_y, sizeof (float), 1, input);
		fread (&loc_z, sizeof (float), 1, input);
	
		node_list[i][0] = loc_x;
		node_list[i][1] = loc_y;
		node_list[i][2] = loc_z;

		fread (&flags, sizeof (int), 1, input);
		node_flags[i] = flags;

		for (j = 0; j < num_nodes; j++)
		{
			fread (&link, sizeof (counter_t), 1, input);
			if (link == NO_NODES)
				break;

			fread (&dist, sizeof (distance_t), 1, input);

			distance_table[i][link] = dist;
		}
	}

	fclose (input);

	path_time = 0;
	gi.dprintf (".....loading complete\n");
	gi.dprintf (".....mapping turned off\n");

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
void	ServerCommand (void)
{
	char	*cmd;

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
	else
		safe_cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

