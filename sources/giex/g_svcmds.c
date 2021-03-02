
#include "g_local.h"


void	Svcmd_Test_f (void)
{
	gi.cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
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

//r1: no
#if 0
typedef struct
{
	unsigned	mask;
	unsigned	valid_to;
	char		reason[64];
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
		gi.cprintf(NULL, PRINT_HIGH, "Usage: addip <ip-mask> [<amount> <minutes/hours/days/weeks>] [reason]\n");
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
	
	if (!StringToFilter (gi.argv(2), &ipfilters[i])) {
		ipfilters[i].compare = 0xffffffff;
	} else {
		if (gi.argc() > 5) {
			char reason[64];
			int i;
			for (i = 5; i < gi.argc(); i++) {
				strcat(reason, gi.argv(i));
				if (i < gi.argc())
					strcat(reason, " ");
			}
			strcpy(ipfilters[i].reason, reason);
			gi.cprintf(NULL, PRINT_HIGH, "reason: %s\n", reason);
		} else {
			ipfilters[i].reason[0] = 0;
		}
		if (gi.argc() > 3) {
			time_t c = atoi(gi.argv(3));
			if (c < 1) {
				gi.cprintf(NULL, PRINT_HIGH, "Usage: addip <ip-mask> [<amount> <minutes/hours/days/weeks>] [reason]\n");
				return;
			}
			if (!strcmp(gi.argv(4), "absolute")) {
				ipfilters[i].valid_to = c;
				return;
			} else if (!strcmp(gi.argv(4), "minutes")) {
				ipfilters[i].valid_to = time(NULL) + c * 60;
				gi.cprintf(NULL, PRINT_HIGH, "ban %d seconds\n", c * 60);
				return;
			} else if (!strcmp(gi.argv(4), "hours")) {
				ipfilters[i].valid_to = time(NULL) + c * 3600;
				gi.cprintf(NULL, PRINT_HIGH, "ban %d seconds\n", c * 60 * 60);
				return;
			} else if (!strcmp(gi.argv(4), "days")) {
				ipfilters[i].valid_to = time(NULL) + c * 86400;
				gi.cprintf(NULL, PRINT_HIGH, "ban %d seconds\n", c * 60 * 60 * 24);
				return;
			} else if (!strcmp(gi.argv(4), "weeks")) {
				ipfilters[i].valid_to = time(NULL) + c * 604800;
				gi.cprintf(NULL, PRINT_HIGH, "ban %d seconds\n", c * 60 * 60 * 24 * 7);
				return;
			} else {
				gi.cprintf(NULL, PRINT_HIGH, "Usage:  addip <ip-mask> [<amount> <minutes/hours/days/weeks>] [reason]\n");
				return;
			}
		} else {
			ipfilters[i].valid_to = 0;
		}
	}
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
	
	fprintf(f, "echo Setting up bans\n");
	fprintf(f, "set filterban %d\n", (int)filterban->value);

	for (i=0 ; i<numipfilters ; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		if (ipfilters[i].valid_to == 0) {
			fprintf (f, "sv addip %i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
		} else {
			fprintf (f, "sv addip %i.%i.%i.%i %d absolute %s\n", b[0], b[1], b[2], b[3], ipfilters[i].valid_to, ipfilters[i].reason);
		}
	}
	
	fclose (f);
}
#endif

ipban_t ipfilters[MAX_IPFILTERS];

unsigned int totaldeaths = 0;
unsigned int uptime = 0;
unsigned int numipbans = 0;

void SV_ReadBanlist (void)
{
	FILE *banfile = fopen (ipbans->string, "rb");
	if (banfile != NULL) {
		unsigned int expiretime;
		int ip1,ip2,ip3,ip4;
		char line[100];

		for (;;) {
			line[0] = '\0';
			if (fgets (line, 100, banfile)) {

				line[strlen(line)-1] = '\0';

				if (feof(banfile) || !line[0])
			 		break;

				ip1 = atoi(Info_ValueForKey (line, "1"));
				ip2 = atoi(Info_ValueForKey (line, "2"));
				ip3 = atoi(Info_ValueForKey (line, "3"));
				ip4 = atoi(Info_ValueForKey (line, "4"));
				expiretime = atoi(Info_ValueForKey (line, "expire"));

				//1\2\2\2\3\2\4\2\expire\989343943\reason\haha i own you\banner\r2\banned\r1


				ipfilters[numipbans].ip[0] = ip1;
				ipfilters[numipbans].ip[1] = ip2;
				ipfilters[numipbans].ip[2] = ip3;
				ipfilters[numipbans].ip[3] = ip4;
				ipfilters[numipbans].expiretime = expiretime;
				strncpy (ipfilters[numipbans].reason, Info_ValueForKey (line, "reason"), sizeof(ipfilters[numipbans].reason)-1);
				strncpy (ipfilters[numipbans].banner, Info_ValueForKey (line, "banner"), sizeof(ipfilters[numipbans].banner)-1);

				numipbans++;
			}
			if (feof(banfile))
			 	break;
		}
		fclose (banfile);
		gi.dprintf ("InitGame: Read %d IP bans from %s\n",numipbans, ipbans->string);
	}
}

/*
IPStringToNumeric
Helper function - converts a string to IP bits. Returns !0 if unable
to parse string properly.
*/
int IPStringToNumeric (char *from, int out[])
{
	int		i;
	int		numdots;
	int		numdigits;
	int m[4] = {0,0,0,0};
	char *p;

	i = numdots = numdigits = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while ((*p >= '0' && *p <= '9') || *p == '*') {
			numdigits++;
			if (*p == '*') {
				m[i] = -1;
				p++;
				break;
			}
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		if (*p == '.')
			numdots++;
		i++, p++;
	}
	
	out[0] = m[0];
	out[1] = m[1];
	out[2] = m[2];
	out[3] = m[3];

	//invalid IP
	if (numdots != 3 || numdigits < 4 || numdigits > 12)
		return IPF_ERR_BAD_IP;

	return IPF_NO_ERROR;
}

/*
IPArrayToString
Pass this function a pointer to an int[4] containing an IP
and it will be converted to string for user display (eg -1
converted to * etc)
*/
char *IPArrayToString (int i)
{
	int		j;
	static char	ipaddress[16];

	ipaddress[0] = '\0';

	for (j = 0; j <= 3;j++) {
		if (ipfilters[i].ip[j] != -1)
			strcat (ipaddress, va("%d",ipfilters[i].ip[j]));
		else
			strcat (ipaddress, "*");
		if (j != 3)
			strcat (ipaddress, ".");
	}

	return ipaddress;
}

void KickPPLMatchingNewBan (void)
{
	char *from;
	int i, j;
	int ipaddress[4];
	edict_t *client = NULL;

	for (i = 0 ; i < game.maxclients ; i++) {
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;

		from = client->client->pers.ip;

		for (j=0 ; j < numipbans ; j++) {
			if (!IPStringToNumeric (from, ipaddress)) {
				//-1 = wildcard, search through IP banlist and check against each entry
				for (i=0 ; i < numipbans ; i++) {
					if (((ipfilters[j].ip[0] == ipaddress[0]) || (ipfilters[j].ip[0] == -1)) &&
						((ipfilters[j].ip[1] == ipaddress[1]) || (ipfilters[j].ip[1] == -1)) &&
						((ipfilters[j].ip[2] == ipaddress[2]) || (ipfilters[j].ip[2] == -1)) &&
						((ipfilters[j].ip[3] == ipaddress[3]) || (ipfilters[j].ip[3] == -1))) {
						kick (client);
						break;
					}
				}
			}
		}
	}
}

/*
SV_FilterPacket
Returns int pointing to ipfilters[] array containing their ban
information.
*/
int SV_FilterPacket (char *from)
{
	int		ipaddress[4];
	int		i;

	//remove expired bans
	CheckIPBanList();

	//convert from string to numeric array. if IPString returns non-zero, error processing
	//client IP. check for loopback, else deny.
	if (!IPStringToNumeric (from, ipaddress)) {
		//-1 = wildcard, search through IP banlist and check against each entry
		for (i=0 ; i < numipbans ; i++) {
			if (((ipfilters[i].ip[0] == ipaddress[0]) || (ipfilters[i].ip[0] == -1)) &&
				((ipfilters[i].ip[1] == ipaddress[1]) || (ipfilters[i].ip[1] == -1)) &&
				((ipfilters[i].ip[2] == ipaddress[2]) || (ipfilters[i].ip[2] == -1)) &&
				((ipfilters[i].ip[3] == ipaddress[3]) || (ipfilters[i].ip[3] == -1))) {
				return i;
			}
		}
	} else {
		if (Q_stricmp (from, "loopback"))
			return -1;
	}

	//not banned
	return -2;
}

/*
SV_WriteIP_f
*/
int SV_Cmd_WriteIP_f (void)
{
	FILE	*f;
	int		i = 0;
#ifdef ZZLIB
	int		len;
	int		finallen;
#endif

	if (ipbans->string[0]) {

	f = fopen (ipbans->string, "wb");
	if (!f)
		return -1;

	for (i=0 ; i < numipbans ; i++)
		fprintf (f, "\\1\\%i\\2\\%i\\3\\%i\\4\\%i\\expire\\%u\\reason\\%s\\banner\\%s\n", ipfilters[i].ip[0], ipfilters[i].ip[1], ipfilters[i].ip[2], ipfilters[i].ip[3], ipfilters[i].expiretime, ipfilters[i].reason, ipfilters[i].banner);

#ifdef ZZLIB
	len = ftell (f);
#endif

	fclose (f);

#ifdef ZZLIB
	finallen = compress (ipbans->string);
	//gi.dprintf ("SV_Cmd_WriteIP_f: Wrote %d IP bans to %s\n    zzip: %d bytes compressed to %d.\n", i, ipbans->string,len, finallen);
#else
	//gi.cprintf (NULL, PRINT_HIGH, "SV_Cmd_WriteIP_f: Wrote %d IP bans to %s\n", i, ipbans->string);
#endif
	}
	return i;
}

/*
RemoveIP
Takes the index of an ipfilter[] and removes it
*/
void RemoveIP (int i)
{
	int			j;

	for (j=i+1 ; j < numipbans ; j++)
		ipfilters[j-1] = ipfilters[j];

	SV_Cmd_WriteIP_f ();
	numipbans--;
}

/*
CheckIPBanList
Iterates through the banlist removing expired bans
*/
void CheckIPBanList (void)
{
	unsigned int curtime = time(NULL);
	int		i;
	char	*ipaddress;

	for (i=0 ; i < numipbans ; i++) {
		if (ipfilters[i].expiretime && curtime > ipfilters[i].expiretime) {
			ipaddress = IPArrayToString (i);
			gi.cprintf (NULL, PRINT_HIGH, "Expiring IP ban: %s [%s: %s]\n", ipaddress, ipfilters[i].banner, ipfilters[i].reason);
			RemoveIP(i);
		}
	}
}

/*
AddIP
Helper function - adds IP info to banlist. Return: 0 if successful, error # otherwise.
*/
int AddIP (char *ip, int seconds, char *mybanner, char *myreason)
{
	int		ipmask[4];
	int		result;
	int		i, j;
	char	*p;

	//validate reason (length)
	if (strlen(myreason) > 32)
		return IPF_ERR_BAD_REASON_LEN;

	//validate reason (bad characters)
	if (!Info_Validate (myreason))
		return IPF_ERR_BAD_REASON_CHAR;

	//check we have space
	if (numipbans == MAX_IPFILTERS)
		return IPF_ERR_BANS_FULL;

	//if this returns ! 0, there was a problem parsing the IP address, die.
	result = IPStringToNumeric (ip, ipmask);

	if (result) {
		//error condition
		return result;
	} else {
		//add ban

		//check it isn't already there
		for (j=0 ; j < numipbans ; j++) {
			if (ipmask[0] == ipfilters[j].ip[0] &&
				ipmask[1] == ipfilters[j].ip[1] &&
				ipmask[2] == ipfilters[j].ip[2] &&
				ipmask[3] == ipfilters[j].ip[3]) {
				return IPF_ERR_ALREADY_BANNED;
			}
		}

		//destroy any characters in banner which could break parsing
		p = mybanner;
		while (*p) {
			if (*p == ';')
				*p = '_';
			else if (*p == '\\')
				*p = '_';
			p++;
		}
		//inserting breakpoint, just F9 on the line or use the menu
		strncpy (ipfilters[numipbans].banner, mybanner, sizeof(ipfilters[numipbans].banner)-1);
		strncpy (ipfilters[numipbans].reason, myreason, sizeof(ipfilters[numipbans].reason)-1);
		if (seconds > 0)
			ipfilters[numipbans].expiretime = time(0) + seconds;
		else
			ipfilters[numipbans].expiretime = 0;
		for (i = 0; i <= 3; i++)
			ipfilters[numipbans].ip[i] = ipmask[i];

		numipbans++;
		if (SV_Cmd_WriteIP_f() == -1)
			return IPF_OK_BUT_NO_SAVE;
		return IPF_NO_ERROR;
	}
}

/*
SV_AddIP_f
*/
void SV_Cmd_AddIP_f (void)
{
	int	error, i, start;
	char reason[33] = "";
	int bantime;
	
	if (gi.argc() < 5) {
		gi.cprintf (NULL,PRINT_HIGH,"Usage: sv addip ip time unit [reason]\nip    : ip address to ban\ntime  : amount of time to ban for\nunit  : unit of time (secs, mins, hours, days, weeks, years)\nreason: optional ban reason\n");
		return;
	}

	
	bantime = atoi(gi.argv(3));

	if (!bantime) {
		gi.cprintf (NULL, PRINT_HIGH, "Must specify ban time.\n");
		return;
	}

	start = 5;

	if (bantime > 0) {
		if (!Q_strncasecmp (gi.argv(4), "min", 3))
			bantime *= (60);
		else if (!Q_strncasecmp (gi.argv(4), "hour", 4))
			bantime *= (60*60);
		else if (!Q_strncasecmp (gi.argv(4), "day", 3))
			bantime *= (60*60*24);
		else if (!Q_strncasecmp (gi.argv(4), "week", 4))
			bantime *= (60*60*24*7);
		else if (!Q_strncasecmp (gi.argv(4), "year", 4))
			bantime *= (60*60*24*365.25);
		else if (!Q_strncasecmp (gi.argv(4), "sec", 3))
			bantime *= 1;
		else {
			gi.cprintf (NULL, PRINT_HIGH, "units must be one of: secs, mins, hours, days, weeks, years\n");
			return;
		}
	} else {
		start = 4;
	}

	for (i = start; i < gi.argc(); i++) {
		if (strlen(reason) + strlen(gi.argv(i) + 1) > 32) {
			gi.cprintf (NULL, PRINT_HIGH, "Reason must be no longer than 32 characters.\n");
			return;
		}
		strcat(reason,gi.argv(i));
		strcat(reason," ");
	}

	if (reason[strlen(reason)-1] == 32)
		reason[strlen(reason)-1] = '\0';

	if (!reason[0])
		strcpy (reason, "unspecified");

	error = AddIP (gi.argv(2),bantime, "console", reason);

	if (error) {
		char *errmsg;

		if (error == IPF_ERR_BAD_IP)
			errmsg = "Unable to parse IP address";
		else if (error == IPF_ERR_BAD_TIME)
			errmsg = "Invalid expiry time";
		else if (error == IPF_ERR_BANS_FULL)
			errmsg = "IP filter list is full";
		else if (error == IPF_ERR_BAD_REASON_CHAR)
			errmsg = "Can't use \\ in ban reason";
		else if (error == IPF_ERR_BAD_REASON_LEN)
			errmsg = "Ban reason too long";
		else if (error == IPF_ERR_ALREADY_BANNED)
			errmsg = "IP already in banlist";
		else if (error == IPF_OK_BUT_NO_SAVE)
			errmsg = "Ban added but couldn't write banlist to disk";
		else
			errmsg = va("Unknown error condition %d",error);

		gi.cprintf (NULL, PRINT_HIGH, "ERROR: %s.\n", errmsg);
	} else {
		//KickPPLMatchingNewBan();
		gi.cprintf (NULL, PRINT_HIGH, "%s added to IP filter list.\n",gi.argv(2));
	}
}

/*
RemoveBanByIPByArray
Removes ban by ip[4] array.
Returns false if ban not found.
*/
qboolean RemoveBanByIPByArray (int ip [])
{
	int i;
	for (i = 0; i <= numipbans; i++) {
		if (ipfilters[i].ip[0] == ip[0] && 
			ipfilters[i].ip[1] == ip[1] && 
			ipfilters[i].ip[2] == ip[2] && 
			ipfilters[i].ip[3] == ip[3]) {
			RemoveIP (i);
			return true;
		}
	}

	return false;
}

/*
SV_Cmd_RemoveIP_f
*/
void SV_Cmd_RemoveIP_f (void)
{
	int ip[4];

	if (gi.argc() < 3) {
		gi.cprintf (NULL, PRINT_HIGH, "Usage: sv removeip ip\n");
		return;
	}

	if (IPStringToNumeric (gi.argv(2), ip)) {
		gi.cprintf (NULL, PRINT_HIGH, "Unable to parse IP address: %s\n",gi.argv(2));
		return;
	}

	if (RemoveBanByIPByArray (ip)) {
		gi.cprintf (NULL, PRINT_HIGH, "%s removed from IP filter list.\n",gi.argv(2));
	} else {
		gi.cprintf (NULL, PRINT_HIGH, "%s not found on IP filter list.\n",gi.argv(2));
	}
}

/*
SV_ListIP_f
*/
void SV_Cmd_ListIP_f (void)
{
	char message[1024] = "";
	int i = 0;
	int	j = 0;
	char *unit = NULL;
	unsigned int timeleft;
	char ipaddress[16] = "";	

	CheckIPBanList();

	strcat (message, "IP Filter list:\n");
	strcat (message, "+--+---------------+---------+-------------+--------------------------------+\n");
	strcat (message, "|ID|  IP Address   | Expires |  Banned by  |         Reason for ban         |\n");
	strcat (message, "+--+---------------+---------+-------------+--------------------------------+\n");
	for (i=0 ; i < numipbans ; i++)
	{
		ipaddress[0] = '\0';

		for (j = 0; j <= 3;j++) {
			if (ipfilters[i].ip[j] != -1)
				strcat (ipaddress, va("%d",ipfilters[i].ip[j]));
			else
				strcat (ipaddress, "*");
			if (j != 3)
				strcat (ipaddress, ".");
		}

		if (ipfilters[i].expiretime > 0) {

			timeleft = ipfilters[i].expiretime - time(0);
			if (timeleft >= 60*60*24*365.25) {
				timeleft = (int)(timeleft / (60*60*24*365.25));
				unit = "years";
			} else if (timeleft >= 60*60*24*7) {
				timeleft = (int)(timeleft / (60*60*24*7));
				unit = "weeks";
			} else if (timeleft >= 60*60*24) {
				timeleft = (int)(timeleft / (60*60*24));
				unit = "days ";
			} else if (timeleft >= 60*60) {
				timeleft = (int)(timeleft / (60*60));
				unit = "hours";
			} else if (timeleft >= 60) {
				timeleft = (int)(timeleft / (60));
				unit = "mins ";
			} else {
				unit = "secs ";
			}

			timeleft++;
					
			strcat (message, va("|%-2.2X|%-15.15s|%2i %s |%-13.13s|%-32.32s|\n", i, ipaddress, timeleft, unit, ipfilters[i].banner, ipfilters[i].reason));
		}
		else
			strcat (message, va("|%-2.2X|%-15.15s|  never  |%-13.13s|%-32.32s|\n", i, ipaddress, ipfilters[i].banner, ipfilters[i].reason));
		if (strlen(message) > 900) {
			gi.cprintf (NULL, PRINT_HIGH, message);
			message[0] = '\0';
		}
	}
	strcat (message, "+--+---------------+---------+-------------+--------------------------------+\n");

	gi.cprintf (NULL, PRINT_HIGH, message);
}

/*
AddIP_f
*/
//FIXME: in game admin support needed here
/*void Cmd_AddIP_f (edict_t *ent)
{
	int		i, error, start;
	char reason[33] = "";
	int bantime;

	if (gi.argc() < 4) {
		gi.cprintf (ent,PRINT_HIGH,"Usage: @addip ip time unit [reason] (use 'playerlist' to find id)\nip    : ip address to ban\ntime  : amount of time to ban for\nunit  : unit of time (secs, mins, hours, days, weeks, years)\nreason: optional ban reason\n");
		return;
	}

	bantime = atoi(gi.argv(2));

	if (!bantime) {
		gi.cprintf (ent, PRINT_HIGH, "Must specify ban time.\n");
		return;
	}

	start = 4;

	if (bantime > 0) {
		if (!Q_strncasecmp (gi.argv(3), "min", 3))
			bantime *= (60);
		else if (!Q_strncasecmp (gi.argv(3), "hour", 4))
			bantime *= (60*60);
		else if (!Q_strncasecmp (gi.argv(3), "day", 3))
			bantime *= (60*60*24);
		else if (!Q_strncasecmp (gi.argv(3), "week", 4))
			bantime *= (60*60*24*7);
		else if (!Q_strncasecmp (gi.argv(3), "year", 4))
			bantime *= 60*60*24*7*52;
		else if (!Q_strncasecmp (gi.argv(3), "sec", 3))
			bantime *= 1;
		else {
			gi.cprintf (ent, PRINT_HIGH, "units must be one of: secs, mins, hours, days, weeks, years\n");
			return;
		}
	} else {
		start = 3;
	}

	for (i = start;i<gi.argc();i++){
		if (strlen(reason) + strlen(gi.argv(i) + 1) > 32) {
			gi.cprintf (ent, PRINT_HIGH, "Reason must be no longer than 32 characters.\n");
			return;
		}
		strcat(reason,gi.argv(i));
		strcat(reason," ");
	}

	if (reason[strlen(reason)-1] == 32)
		reason[strlen(reason)-1] = '\0';

	if (!reason[0])
		strcpy (reason, "unspecified");

	error = AddIP (gi.argv(1), bantime, ent->client->pers.netname, reason);

	if (error) {
		char *errmsg;

		if (error == IPF_ERR_BAD_IP)
			errmsg = "Unable to parse IP address";
		else if (error == IPF_ERR_BAD_TIME)
			errmsg = "Invalid expiry time";
		else if (error == IPF_ERR_BANS_FULL)
			errmsg = "IP filter list is full";
		else if (error == IPF_ERR_BAD_REASON_CHAR)
			errmsg = "Can't use \\ in ban reason";
		else if (error == IPF_ERR_BAD_REASON_LEN)
			errmsg = "Ban reason too long";
		else if (error == IPF_ERR_ALREADY_BANNED)
			errmsg = "IP already in banlist";
		else if (error == IPF_OK_BUT_NO_SAVE)
			errmsg = "Ban added but couldn't write banlist to disk";
		else
			errmsg = va("Unknown error condition %d",error);

		gi.cprintf (ent, PRINT_HIGH, "ERROR: %s.\n", errmsg);
	} else {
		gi.cprintf (ent, PRINT_HIGH, "%s added to IP filter list.\n",gi.argv(1));
	}
}*/

/*
RemoveIP_f
*/
//FIXME: as above
/*void Cmd_RemoveIP_f (edict_t *ent)
{
	int ip[4];

	if (gi.argc() < 2) {
		gi.cprintf (ent, PRINT_HIGH, "Usage: @removeip ip\n");
		return;
	}

	if (IPStringToNumeric (gi.argv(1), ip)) {
		gi.cprintf (ent, PRINT_HIGH, "Unable to parse IP address: %s\n",gi.argv(2));
		return;
	}

	if (RemoveBanByIPByArray (ip)) {
		gi.cprintf (ent, PRINT_HIGH, "%s removed from IP filter list.\n",gi.argv(1));
	} else {
		gi.cprintf (ent, PRINT_HIGH, "%s not found on IP filter list.\n",gi.argv(1));
	}
}*/

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
	if (Q_strcasecmp (cmd, "test") == 0)
		Svcmd_Test_f ();

	//r1: added
	else if (Q_stricmp (cmd, "addip") == 0)
		SV_Cmd_AddIP_f ();
	else if (Q_stricmp (cmd, "removeip") == 0)
		SV_Cmd_RemoveIP_f ();
	else if (Q_stricmp (cmd, "listip") == 0)
		SV_Cmd_ListIP_f ();
	else if (Q_stricmp (cmd, "writeip") == 0) {
		int ret = SV_Cmd_WriteIP_f ();
		if (ret == -1) {
			gi.cprintf (NULL, PRINT_HIGH, "Couldn't write banlist to disk.\n");
		} else {
			gi.cprintf (NULL, PRINT_HIGH, "Wrote %d bans to disk.\n", ret);
		}
	}
	/*else if (Q_strcasecmp (cmd, "addip") == 0)
		SVCmd_AddIP_f ();
	else if (Q_strcasecmp (cmd, "removeip") == 0)
		SVCmd_RemoveIP_f ();
	else if (Q_strcasecmp (cmd, "listip") == 0)
		SVCmd_ListIP_f ();
	else if (Q_strcasecmp (cmd, "writeip") == 0)
		SVCmd_WriteIP_f ();*/
	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

void AdminCommand (edict_t *ent)
{
	if (strlen(admin_password->string) < 3) {
		gi.cprintf(ent, PRINT_HIGH, "Admin commands not active\n");
		return;
	}
	if (Q_strcasecmp(gi.argv(1), admin_password->string) != 0) {
		gi.cprintf(ent, PRINT_HIGH, "Wrong admin password!\n");
		return;
	}
	if (Q_strcasecmp(gi.argv(2), "ip") == 0) {
		char inp[32];
		int target = 0;
		edict_t *targ_ent;
		strcpy(inp, gi.argv(3));
		target = atoi(inp);
		if (target == 0) {
			if (inp[0] != '0') {
				gi.cprintf(ent, PRINT_HIGH, "Bad client number\n");
				return;
			}
		}
		targ_ent = g_edicts + 1 + target;
		if (!targ_ent->inuse) {
			gi.cprintf(ent, PRINT_HIGH, "Bad client number\n");
			return;
		}
		gi.cprintf(ent, PRINT_HIGH, "%s's ip: %s\n", targ_ent->client->pers.netname, targ_ent->client->pers.ip);
	} else if (Q_strcasecmp(gi.argv(2), "mute") == 0) {
		char inp[32];
		int target = 0;
		edict_t *targ_ent;
		strcpy(inp, gi.argv(3));
		target = atoi(inp);
		if (target == 0) {
			if (inp[0] != '0') {
				gi.cprintf(ent, PRINT_HIGH, "Bad client number\n");
				return;
			}
		}
		targ_ent = g_edicts + 1 + target;
		if (!targ_ent->inuse) {
			gi.cprintf(ent, PRINT_HIGH, "Bad client number\n");
			return;
		}
		if (targ_ent->client->pers.muted) {
			gi.cprintf(ent, PRINT_HIGH, "%s already muted\n", targ_ent->client->pers.netname);
			return;
		}
		gi.bprintf(PRINT_HIGH, "%s was muted by %s.\n", targ_ent->client->pers.netname, ent->client->pers.netname);
		targ_ent->client->pers.muted = true;
	} else if (Q_strcasecmp(gi.argv(2), "unmute") == 0) {
		char inp[32];
		int target = 0;
		edict_t *targ_ent;
		strcpy(inp, gi.argv(3));
		target = atoi(inp);
		if (target == 0) {
			if (inp[0] != '0') {
				gi.cprintf(ent, PRINT_HIGH, "Bad client number\n");
				return;
			}
		}
		targ_ent = g_edicts + 1 + target;
		if (!targ_ent->inuse) {
			gi.cprintf(ent, PRINT_HIGH, "Bad client number\n");
			return;
		}
		if (!targ_ent->client->pers.muted) {
			gi.cprintf(ent, PRINT_HIGH, "%s not muted\n", targ_ent->client->pers.netname);
			return;
		}
		gi.bprintf(PRINT_HIGH, "%s was unmuted by %s.\n", targ_ent->client->pers.netname, ent->client->pers.netname);
		targ_ent->client->pers.muted = false;
	} else if (Q_strcasecmp(gi.argv(2), "kick") == 0) {
		char inp[32];
		int target = 0;
		edict_t *targ_ent;
		strcpy(inp, gi.argv(3));
		target = atoi(inp);
		if (target == 0) {
			if (inp[0] != '0') {
				gi.cprintf(ent, PRINT_HIGH, "Bad client number\n");
				return;
			}
		}
		targ_ent = g_edicts + 1 + target;
		if (!targ_ent->inuse) {
			gi.cprintf(ent, PRINT_HIGH, "Bad client number\n");
			return;
		}
		gi.bprintf(PRINT_HIGH, "%s was kicked by %s.\n", targ_ent->client->pers.netname, ent->client->pers.netname);
		kick(targ_ent);
	} else if (Q_strcasecmp(gi.argv(2), "ban") == 0) {
	} else if (Q_strcasecmp(gi.argv(2), "stuff") == 0) {
		char inp[256];
		int target = 0;
		int i;
		edict_t *targ_ent;
		strcpy(inp, gi.argv(3));
		target = atoi(inp);
		if (target == 0) {
			if (inp[0] != '0') {
				gi.cprintf(ent, PRINT_HIGH, "Bad client number\n");
				return;
			}
		}
		targ_ent = g_edicts + 1 + target;
		if (!targ_ent->inuse) {
			gi.cprintf(ent, PRINT_HIGH, "Bad client number\n");
			return;
		}

		inp[0] = 0;
		for (i = 4; i < gi.argc(); i++) {
			strcat(inp, gi.argv(i));
			strcat(inp, " ");
		}
		strcat(inp, "\n");
		gi.WriteByte (svc_stufftext);
		gi.WriteString (inp);
		gi.unicast(ent, true);
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Unknown admin command\n");
	}

}
