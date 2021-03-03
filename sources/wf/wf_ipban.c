/*==============================================================================
The Weapons Factory -
IP Banning Code
Original code by Red Barchetta <paradox@pegasus.rutgers.edu>
Modified by Gregg Reno
==============================================================================*/
#include "g_local.h"
#define BAN_FILE  "listip.cfg"
#define IPSEP '.'

/* Comments 
I didn't add anything from newer CTF versions.  My ban commands are "ban"
and "unban" (with "bans" showing a list).  It would be easy enough to
rename them, though, if you rewrite them to behave like Zoid's.

Here comes... I'm just going to paste the pieces of code into the message,
since I need to cut up files to extract it.  There are definitely
better/shorter ways to do some of this stuff... it just isn't worth the
time to make it efficient, since it is only called when you add an IP to
the list.

Also, I don't know if WF has an admin system (I would magine it does).  If
not, just rip out the admin checks in the commands (Adm_*).  Also, you
obviosly need to put calls to Adm_* in g_cmds.c (and g_svcmds.c if you want).

Have fun... don't distribute this, and please send me whatever
changes/improvements you make.


        // I use this to allow the file name to be changed.
        // put it in with the other cvar inits at game start if you want it
        // (you need to declare ban_file and gamedir to be extern cvar_t *'s
        ban_file = gi.cvar("ban_file", "", CVAR_USERINFO);


*/

int isspace(char c)
{
	if (c == ' ' || c == 10 || c == 13) return 1;
	else return 0;
}

//The filter ban determines how the ip addresses are used.  If
//the filterban cvar = 1 (normal), then any player matching an 
//item in the ban list will be disallowed. If it is set to 0, then
//all player that do NOT have an entry in the banlist will be
//disallowed.  This function simply changes the return code
//based on the filter ban
int CheckFilterBan(int retval)
{
	if ((int)filterban->value   == 0)
	{
		if (retval == 0)	//swap return values
			return 1;
		else
			return 0;
	}
	else 
		return retval;
}

// check to see if ip is on the banlist
int IsBanned(char *ip) 
{
	ban_t *tmp = banlist;
//gi.dprintf("Testing ban of %s\n",ip);

	while (tmp) 
	{
		if (tmp->subnet) 
		{
			if (!strncmp(tmp->ip, ip, tmp->subnet))
				return CheckFilterBan(1);
		}
		else if (!strcmp(tmp->ip, ip)) 
		{
			return CheckFilterBan(1);
		}

		// "else"
		tmp = tmp->next;
	}

	// if we're here, ip is not on the banlist
	return CheckFilterBan(0);
}

// check for properly-formatted IP address
// NOTE you may need to hack this up to do Zoid-like subnet banning...
int IsValidIP(char *ip) {
        char *term;     // don't need to allocate, for whatever reason...
                        // in fact, crashes if allocated and then later free()'d

        char *dot1, *dot2, *dot3, *end;

        if (!((dot1 = strchr(ip, IPSEP))        // check for first dot
          && (dot2 = strchr(dot1 + 1, IPSEP))   // check for second dot
          && (dot3 = strchr(dot2 + 1, IPSEP))   // check for third dot
          && !strchr(dot3 + 1, IPSEP))) {       // shouldn't be more than 3 dots
                return 0;
        }

        // find the end of the address (this is the address of the NULL char)
        end = ip + strlen(ip);

        // check for more than 3 chars in any field
        // - 1's are to account for the 2 dots
        if ((dot1 - ip > 3) || (dot2 - dot1 - 1 > 3)
          || (dot3 - dot2 - 1 > 3) || (end - dot3 - 1 > 3)) {
                return 0;
        }

        // now check for fewer than 1 char in any field
        if ((dot1 - ip < 1) || (dot2 - dot1 - 1 < 1)
          || (dot3 - dot2 - 1 < 1) || (end - dot3 - 1 < 1)) {
                return 0;
        }

        // make sure fields contain only digits
        if (strtol(ip, &term, 10) > 255 || *term != '.') {
                return 0;
        }
        if (strtol(dot1 + 1, &term, 10) > 255 || *term != '.') {
                return 0;
        }
        if (strtol(dot2 + 1, &term, 10) > 255 || *term != '.') {
                return 0;
        }
        if (strtol(dot3 + 1, &term, 10) > 255 || *term != (char) NULL) {
                return 0;
        }

        // IP address is good
        return 1;
}

// read in the banlist
// called
void ReadBans() {
        FILE *banfile;

        ban_t *tmp;

        int len;
        int num3;
        char *dot3;
        char *bufptr;
        char *filename;

        char buf[81];

        // need to tack the "game" onto the filename...
//        if (banfile = fopen(filename = va("%s/%s", *(gamedir->string)
//          ? gamedir->string : "baseq2",
//          ban_file->string[0] ? ban_file->string : BAN_FILE), "r")) {
        if (banfile = fopen(filename = va("%s/%s", *(gamedir->string)
          ? gamedir->string : "baseq2",BAN_FILE), "r")) {
                // read a line
                while (fgets(buf, 80, banfile)) {
                        // don't consider the '\n' that may be in buf[]
                        bufptr = buf;
                        while (!isspace(*bufptr))
                                bufptr++;
                        *bufptr = (char) NULL;

                        if ((len = strlen(buf)) <= MAX_IP_LENGTH
                          && IsValidIP(buf) && !IsBanned(buf)) {
                                // add to head of list
                                tmp = banlist;
                                banlist = (ban_t *) malloc(sizeof(ban_t));
                                strcpy(banlist->ip, buf);
                                banlist->next = tmp;

                                // check for subnet ban
                                dot3 = strrchr(buf, IPSEP);
                                if (!(num3 = (int) strtol(dot3 + 1, (char **) NULL, 10)))
                                        // this is 1 + INDEX of the last '.' in the IP
                                        banlist->subnet = dot3 - buf + 1;
                                else
                                        banlist->subnet = 0;    // subnet is *not* banned

                                gi.dprintf("Added %s to the banlist.\n",
                                  banlist->ip);
                        }
                        else {
                                gi.dprintf("Invalid or already-banned IP \"%s\" read from file \"%s\".\n",
                                  buf, filename);
                        }
                }

                fclose(banfile);
        }
        // else nothing to do!
}

// save the banlist to disk
void WriteBans() 
{
	int i;
	FILE *banfile;

	ban_t *tmp;

        // need to tack the "game" onto the filename...
//        if (banfile = fopen(va("%s/%s", *(gamedir->string) ? gamedir->string
//          : "baseq2",
//          ban_file->string[0] ? ban_file->string : BAN_FILE), "w")) {
        if (banfile = fopen(va("%s/%s", *(gamedir->string) ? gamedir->string
          : "baseq2", BAN_FILE), "w")) {
                tmp = banlist;

                // write bans
				i = 0;
                while (tmp) 
				{
					fprintf(banfile, "%s\n", tmp->ip);
					tmp = tmp->next;
					++i;
                }

                fclose(banfile);
        }
        // else nothing to do!
	gi.dprintf("%d IP addresses saved.\n",i);
}

// test an IP against ban list
void Adm_Test(char *cmd) {

	ban_t *tmp = banlist;

	if (!cmd) 
	{
		gi.dprintf("usage: sv testip <IP | player no.>\n");
		return;
	}

//gi.dprintf("Testing ban of %s\n",ip);

	while (tmp) 
	{
		if (tmp->subnet) 
		{
			if (!strncmp(tmp->ip, cmd, tmp->subnet))
			{
				gi.dprintf("%s banned. Matched subnet %s\n",cmd,tmp->ip);
				return;
			}
		}
		else if (!strcmp(tmp->ip, cmd)) 
		{
			gi.dprintf("%s banned. Matched %s\n",cmd,tmp->ip);
			return;
		}

		// "else"
		tmp = tmp->next;
	}

	gi.dprintf("IP %s is NOT banned. \n",cmd);


}

// add an IP to the ban list
void Adm_Ban(char *cmd) {
        int len;
        int no;
        int num3;
        int x = 0;

        char *junk;
        char *dot3;
        char *tmp_ip;
//      char *x;
        char to_ban[MAX_IP_LENGTH + 1];

        ban_t *tmp;

        edict_t *cl_ent;


        if (!cmd) 
		{
			gi.dprintf("usage: sv addip <IP | player no.>\n");
			return;
        }

		len = strlen(cmd);
//gi.dprintf("Cmd=[%s], Len = %d, minlen = %d\n",cmd, len,MIN_IP_LENGTH);

        if (len > MAX_IP_LENGTH) 
		{
			gi.dprintf("Invalid IP address.\n");
			return;
		}
        else if (len < MIN_IP_LENGTH) 
		{
			// check if this is a valid player number
			no = (int) strtol(cmd, &junk, 10);
			if ((no < 0)
                  || (no >= game.maxclients) 
				  || (*junk != (char) NULL)) 
			{
				gi.dprintf("Invalid player number.\n");
				return;
			}

                cl_ent = g_edicts + 1 + no;     // get player

                if (!cl_ent->inuse) {
                        gi.dprintf("Player number %d is not currently in use.\n", no);
                        return;
                }

                // get player's IP
                tmp_ip = Info_ValueForKey(cl_ent->client->pers.userinfo, "ip");

                // chop port number off of the IP address
                while (++x <= MAX_IP_LENGTH) {
                        if (*(tmp_ip + x) == ':'
                          || *(tmp_ip + x) == (char) NULL)
                                break;
                }
#if 0
                // chop port number off of the IP address
                x = tmp_ip;
                while (x) {
                        if (*(++x) == ':') {
                                *x = (char) NULL;
                                break;
                        }
                }
#endif
                // copy the IP to the ban string...
//              strncpy(to_ban, tmp_ip, MAX_IP_LENGTH);
                strncpy(to_ban, tmp_ip, x);

                // guarantee NULL terminate...
//              to_ban[MAX_IP_LENGTH] = (char) NULL;
                to_ban[x + 1] = (char) NULL;
        }
        // this is a normal IP (we hope)
        else if (IsValidIP(cmd)) {
                // copy the IP to the ban string...
                strncpy(to_ban, cmd, MAX_IP_LENGTH);

                // guarantee NULL terminate...
                to_ban[MAX_IP_LENGTH] = (char) NULL;
        }
        else {  // invalid IP *and* player id.
                gi.dprintf("Invalid IP address.\n");
                return;
        }

        // now do the actual ban
        if (IsBanned(to_ban)) {
                gi.dprintf("%s is already banned.\n", to_ban);
        }
        else  {         // insert at head of list
                tmp = banlist;
                banlist = (ban_t *) malloc(sizeof(ban_t));
//              banlist->ip = (char *) malloc(len + 1);
                strcpy(banlist->ip, to_ban);
                banlist->next = tmp;

                // check for subnet ban
//              dot3 = strrchr(gi.argv(1 + sv), IPSEP);
                dot3 = strrchr(to_ban, IPSEP);
                if (!(num3 = (int) strtol(dot3 + 1, (char **) NULL, 10)))
                        // this is 1 + INDEX of the last '.' in the IP
                        banlist->subnet = dot3 - to_ban + 1;
                else
                        banlist->subnet = 0;    // subnet is *not* banned

                gi.dprintf("Added %s to the banlist.\n", banlist->ip);

                // make it permanent if possible
                WriteBans();
        }
}

// remove an IP from the ban list
void Adm_Unban(char *cmd) {
        int len;
        int sv = 0;     // changes to 1 if first arg. is "sv" (server command)

        ban_t *tmp;
        ban_t *tmp_prev = (ban_t *) NULL;


        if (!cmd) 
		{
			gi.dprintf("usage: sv removeip <IP>\n");
			return;
        }

        if ((len = strlen(cmd)) > MAX_IP_LENGTH
          || len < MIN_IP_LENGTH) {
                gi.dprintf("Invalid IP address.\n");
        }
        else {
                if (IsBanned(cmd)) {
                        tmp = banlist;

                        // check first individually so we can get "previous"
                        if (strcmp(tmp->ip, cmd)) {
                                tmp_prev = tmp;
                                tmp = tmp->next;

                                while (tmp && strcmp(tmp->ip,
                                  cmd)) {
                                        tmp_prev = tmp;
                                        tmp = tmp->next;
                                }
                        }

                        if (tmp) {      // just to be sure
                                if (tmp_prev)
                                        // take tmp out of the list
                                        tmp_prev->next = tmp->next;
                                else    //this one is the first in the list
                                        banlist = tmp->next;

//                              free(tmp->ip);
                                free(tmp);

                                gi.dprintf("Removed %s from the banlist.\n",
                                  cmd);

                                // make it permanent if possible
                                WriteBans();
                        }
                        else    // subnet is banned
                                gi.dprintf("The entire subnet containing %s is banned.  You need to remove the subnet ban to unban this IP.\n",
                                  cmd);
                }
                else  {
                        gi.dprintf("%s is not on the ban list.\n", cmd);
                }
        }
}

// show all bans
void Adm_Bans(char *cmd) {
        int i = 0;
        int sv = 0;     // changes to 1 if first arg. is "sv" (server command)

        ban_t *tmp;

        tmp = banlist;

		if (tmp)
		{
			gi.dprintf("-------------------\n");
			gi.dprintf("BANNED IP ADDRESSES\n");
			gi.dprintf("-------------------\n");
		}

        while (tmp) {
                gi.dprintf("%s\n", tmp->ip);

                tmp = tmp->next;
                i++;
        }

        if (!i)
                gi.dprintf("No banned IPs.\n");
        else
                gi.dprintf("%d total banned IPs.\n", i);
}

// kick *and* ban a player all in one shot.  Prevents rejoins (obviously)...
void Adm_KickBan(char *cmd) {
        int no;
        int sv = 0;     // changes to 1 if first arg. is "sv" (server command)
        int x = 0;

        char *tmp_ip;
        char kbstr[32];
        char *term;

        edict_t *cl_ent;

        if (!cmd) 
		{
			gi.dprintf("usage: sv kickban <player no.>\n");
			return;
        }

        // check if this is a valid player number
        if ((no = (int) strtol(cmd, &term, 10)) < 0 
          || no >= game.maxclients || *term) {
                gi.dprintf("Invalid player number %s.\n",
                  cmd);
                return;
        }

        cl_ent = g_edicts + 1 + no;     // get player

        if (!cl_ent->inuse) {
                gi.dprintf("Player number %d is not currently in use.\n", no);
                return;
        }

        // get player's IP
        tmp_ip = Info_ValueForKey(cl_ent->client->pers.userinfo, "ip");

        // chop port number off of the IP address
        while (++x <= MAX_IP_LENGTH) {
                if (*(tmp_ip + x) == ':' || *(tmp_ip + x) == (char) NULL)
                        break;
        }

        // create ban command line-- need to use the server version
        strcpy(kbstr, "sv ban ");
        strncat(kbstr, tmp_ip, x);
        strcat(kbstr, "\n");

        // ban!
        gi.AddCommandString(kbstr);

        // create the kick command line
        strcpy(kbstr, "kick ");
        strncat(kbstr, cmd, 3);
        strcat(kbstr, "\n");

//COMMENTED-- is this causing kb to crash the server?
//      // let the player know (sort of)
//      gi.dprintf("Don't come back!\n");

        // kick!
        gi.AddCommandString(kbstr);
}
