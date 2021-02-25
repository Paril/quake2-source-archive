/*
    Quake II Glooom, a total conversion mod for Quake II
    Copyright (C) 1999-2007  Gloom Developers

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "g_local.h"
#include "g_map.h"
#include "g_wordfilter.h"

#include <stdio.h>

const char * permission_names[] = {
        "PERMISSION_BAN",
        "PERMISSION_KICK",
        "PERMISSION_MUTE",
        "PERMISSION_TOYS",
        "PERMISSION_STUFF",
        "PERMISSION_SUMMON",
        "PERMISSION_CHANGEMAP",
        "PERMISSION_PAUSE",
        "PERMISSION_PUSH",
        "PERMISSION_BLAST",
        "PERMISSION_DEBUG",
        "PERMISSION_DEATHBALL",
        "PERMISSION_SUDDENDEATH",
        "PERMISSION_SETTEAM",
        "PERMISSION_VIEW",
        "PERMISSION_CLASSBAN",
        "PERMISSION_WORDFILTERS",
        "PERMISSION_SHOT",
        "PERMISSION_ACCOUNTS",
        "PERMISSION_ADMINMODE",
        "PERMISSION_BENEFITS",
        "PERMISSION_VIEWGAMESTUFF",
        "PERMISSION_VETO",
        "PERMISSION_CLOAK",
        "PERMISSION_ADMINSLOT",
        "PERMISSION_CHECK",
        "UNDEFINED_P26",
        "UNDEFINED_P27",
        "UNDEFINED_P28",
        "UNDEFINED_P29",
        "UNDEFINED_P30",
        "PERMISSION_BOT",
        NULL
};

void AdminLog (edict_t *ent, char *message);
void centerprint_all(char *text);
void stuff_all(char *text);

void    SV_Cmd_Kickobs_f (void)
{
        int i;
        edict_t *client;

        gi.bprintf (PRINT_CHAT,"server: observer removal started!\n");

        for (i = 0 ; i < game.maxclients ; i++) {
                client = g_edicts + 1 + i;
                if (!client->inuse)
                        continue;
                if (client->client->resp.team == TEAM_NONE) {
                        kick (client);
                }
        }
}

void EXPORT ClientUserinfoChanged (edict_t *ent, char *userinfo);
void SV_Cmd_Updateuserinfo_f (void)
{
        int num = 0;
        int i;
        edict_t *client;
        for (i = 0 ; i < game.maxclients ; i++) {
                client = g_edicts + 1 + i;
                if (!client->inuse)
                        continue;
                if (!client->client->pers.connected)
                        continue;
                ClientUserinfoChanged (client,client->client->pers.userinfo);
                num++;
        }
        gi.cprintf (NULL,PRINT_HIGH,"ClientUserinfoChanged called for %d clients.\n",num);
}

#define CLEAN(x) \
        x->spawnflags = x->s.skinnum = x->s.sound = x->s.frame = x->svflags = x->s.effects = x->s.renderfx = x->takedamage = x->health = x->enttype = x->count = x->random = x->nextthink = 0; \
        x->think = NULL; \
        x->pain = NULL; \
        x->die = NULL; \
        x->touch = NULL; \
        x->owner = NULL; \
        x->groundentity = NULL; \
        x->target_ent = NULL; \
        VectorClear (x->mins); \
        VectorClear (x->maxs); \
        VectorClear (x->size); \
        x->s.event = EV_OTHER_TELEPORT;

void ED_CallSpawn (edict_t *ent);
void SV_Invert_f (void)
{
        int                             i;
        unsigned                old_framenum;
        edict_t                 *ent = NULL;

        memset (&spawnlist, 0, sizeof(spawnlist));

        team_info.maxpoints[TEAM_HUMAN] = (int)teambonus->value;
        team_info.maxpoints[TEAM_ALIEN] = (int)teambonus->value;

        team_info.points[TEAM_HUMAN] = 0;
        team_info.points[TEAM_ALIEN] = 0;

        for(i=0; i<MAXTEAMS; i++)
                team_info.spawns[i] = 0;

        //hack for ed_callspawn setting bps
        old_framenum = level.framenum;
        level.framenum = 0;

        for (ent = g_edicts + 1 + game.maxclients; ent < g_edicts + globals.num_edicts; ent++)
        {
                if (!ent->inuse)
                        continue;

                switch (ent->enttype)
                {
                        case ENT_SPIKER:
                                CLEAN (ent);
                                ent->classname = "misc_turret";
                                ent->s.origin[2] += 32;
                                ED_CallSpawn (ent);
                                break;
                        case ENT_OBSTACLE:
                                CLEAN (ent);
                                VectorSet (ent->s.angles, -90, 0, 0);
                                ent->classname = "misc_tripwire";
                                ED_CallSpawn (ent);
                                break;
                        case ENT_GASSER:
                                CLEAN (ent);
                                ent->classname = "misc_detector";
                                ED_CallSpawn (ent);
                                break;
                        case ENT_COCOON:
                                CLEAN (ent);
                                ent->classname = "info_player_deathmatch";
                                ED_CallSpawn (ent);
                                break;
                        case ENT_HEALER:
                                if (ent->owner)
                                        G_FreeEdict (ent->owner);
                                CLEAN (ent);
                                ent->s.origin[2] += 32;
                                ent->classname = "misc_ammo_depot";
                                ED_CallSpawn (ent);
                                break;

                        case ENT_MGTURRET:
                        case ENT_TURRET:
                                G_FreeEdict (ent->target_ent);
                                CLEAN (ent);
                                ent->classname = "monster_spiker";
                                ED_CallSpawn (ent);
                                break;
                        case ENT_TRIPWIRE_BOMB:
                                if (ent->owner)
                                        G_FreeEdict (ent->owner);
                                CLEAN (ent);
                                ent->s.origin[2] += 32;
                                VectorClear (ent->s.angles);
                                ent->classname = "monster_obstacle";
                                ED_CallSpawn (ent);
                                break;
                        case ENT_DETECTOR:
                                CLEAN (ent);
                                ent->classname = "monster_gasser";
                                ED_CallSpawn (ent);
                                break;
                        case ENT_AMMO_DEPOT:
                                CLEAN (ent);
                                ent->classname = "monster_healer";
                                ED_CallSpawn (ent);
                                break;
                        case ENT_TELEPORTER:
                                CLEAN (ent);
                                ent->classname = "monster_cocoon";
                                ED_CallSpawn (ent);
                                break;
                }
        }

        level.framenum = old_framenum;
}

unsigned int CalculateAdminPermissions (char *permissionstring)
{
        unsigned int permissions = 0;
        char *p, *o;
        const char **s;

        if (isdigit(*permissionstring))
                return strtoul (permissionstring, 0, 10);

        if (!Q_stricmp (permissionstring, "ALL"))
                return 0xFFFFFFFF;
        
        p = o = permissionstring;
        
        while (*p) {
                p++;
                if (*p == '|') {
                        *p = 0;
                        for (s = permission_names; *s; s++) {
                                if (Q_stricmp (o, *s) == 0) {
                                        permissions |= (1 << (s - permission_names));
                                        break;
                                }
                        }
                        if (!*s)
                                gi.cprintf (NULL, PRINT_HIGH, "Ignoring unknown permission field '%s'\n", o);
                        o = p + 1;
                        p++;
                }
        }

        if (*o) {
                for (s = permission_names; *s; s++) {
                        if (Q_stricmp (o, *s) == 0) {
                                permissions |= (1 << (s - permission_names));
                                break;
                        }
                }
                if (!*s)
                        gi.cprintf (NULL, PRINT_HIGH, "Ignoring unknown permission field '%s'\n", o);
        }
        return permissions;
}

void SV_Cmd_Modifyaccount_f (int enable)
{
        unsigned int permissions;

        if (!accounts->string[0]) {
                gi.cprintf (NULL, PRINT_HIGH, "Accounts are disabled.\n");
                return;
        }

        //syntax check
        if (!(*gi.argv(2))) {
                gi.cprintf (NULL,PRINT_HIGH,"Usage: sv (enable|disable)account username [permissions]\n");
                return;
        }

        if (gi.argc() == 4) {
                permissions = CalculateAdminPermissions (gi.argv(3));
        } else {
                permissions = 0;
        }
        Cmd_Modifyaccount (NULL, gi.argv(2), enable, permissions);
}

void SV_Cmd_Listaccounts_f (void);

void SV_Cmd_Addaccount_f (void)
{
        unsigned int permissions;

        if (!accounts->string[0]) {
                gi.cprintf (NULL, PRINT_HIGH, "Accounts are disabled.\n");
                return;
        }

        //invalid argument check
        if (!Q_stricmp(gi.argv(2), "") || !Q_stricmp(gi.argv(3), "")) {
                gi.cprintf (NULL,PRINT_HIGH,"Usage: sv addaccount username password [permissions]\n");
                return;
        }

        if (gi.argc() == 5) {
                permissions = CalculateAdminPermissions (gi.argv(4));
        } else {
                permissions = 0;
        }

        Cmd_Addaccount (NULL, gi.argv(2), gi.argv(3), permissions);
}

void SV_Cmd_Deleteaccount_f (void)
{
        //int i = 0, j = 0;
        //edict_t *victim;

        if (!accounts->string[0]) {
                gi.cprintf (NULL, PRINT_HIGH, "Accounts are disabled.\n");
                return;
        }

        //syntax check
        if (!(*gi.argv(2))) {
                gi.cprintf (NULL,PRINT_HIGH,"Usage: sv delaccount username\n");
                return;
        }

        Cmd_Removeaccount (NULL, gi.argv(2));
}

void SV_Cmd_Update_f (void)
{
    int updated;
    if (Q_stricmp(gi.argv(2), "game") == 0) {
            updated = system(va("%s/%s game", gamedir->string, sv_updater->string));
            gi.dprintf ("GLOOM: Game file updating... reloading on next map change.\n");
            gi.AddCommandString("sv_recycle 1");
    }
    if (!Q_stricmp(gi.argv(2), "game") || !Q_stricmp(gi.argv(2), "damages"))
    {
            updated = system(va("%s/%s dmgs", gamedir->string, sv_updater->string));
            gi.dprintf("GLOOM: Damage file updating... reloading damages\n");
            LoadDamageOverrideTable();
    }
    //invalid argument check
    else {
           gi.cprintf (NULL,PRINT_HIGH,"Usage: sv update r1q2ded|game|damages\n");
    }
}

/*
IPStringToNumeric
Helper function - converts a string to IP bits. Returns !0 if unable
to parse string properly.
*/
int IPStringToNumeric (char *from, int out[])
{
        int             i;
        int             numdots;
        int             numdigits;
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
        int             j;
        static char     ipaddress[16];

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
                if (!client->client->pers.ingame)
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

int ClientMatchesClassB (edict_t *client)
{
        char *from;
        int i, j;
        int ipaddress[4];

        from = client->client->pers.ip;

        for (j=0 ; j < numipbans ; j++) {
                if (!IPStringToNumeric (from, ipaddress)) {
                        //-1 = wildcard, search through IP banlist and check against each entry
                        for (i=0 ; i < numipbans ; i++) {
                                if (((ipfilters[j].ip[0] == ipaddress[0]) || (ipfilters[j].ip[0] == -1)) &&
                                        ((ipfilters[j].ip[1] == ipaddress[1]) || (ipfilters[j].ip[1] == -1))) {
                                        return j;
                                }
                        }
                }
        }

        return -2;
}

/*
SV_FilterPacket
Returns int pointing to ipfilters[] array containing their ban
information.
*/
int SV_FilterPacket (char *from)
{
        int             ipaddress[4];
        int             i;

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
        FILE    *f;
        int             i = 0;
#ifdef ZZLIB
        int             len;
        int             finallen;
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
        int                     j;

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
        int             i;
        char    *ipaddress;

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
        int             ipmask[4];
        int             result;
        int             i, j;
        char    *p;

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
        int     error, i, start;
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
                        bantime *= (60*60*24*365);
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
        for (i = 0; i < numipbans; i++) {
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
void SV_Cmd_ListIP_f (edict_t *ent)
{
        char message[1024] = "";
        int i = 0;
        int     j = 0;
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
                        //r1: safety, print %s not message
                        gi.cprintf (ent, PRINT_HIGH, "%s", message);
                        message[0] = '\0';
                }
        }
        strcat (message, "+--+---------------+---------+-------------+--------------------------------+\n");

        //r1: safety, print %s not message
        gi.cprintf (ent, PRINT_HIGH, "%s", message);
}

/*
AddIP_f
*/
void Cmd_AddIP_f (edict_t *ent)
{
        int             i, error, start;
        char reason[33] = {0};
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

        error = AddIP (gi.argv(1), bantime, ent->client->pers.username, reason);

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
}

/*
RemoveIP_f
*/
void Cmd_RemoveIP_f (edict_t *ent)
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
}

void SV_Cmd_Kick_f (void)
{
        int x;

        if (gi.argc() < 3) {
                gi.cprintf (NULL, PRINT_HIGH, "Usage: sv kick name|id\n");
                return;
        }

        x = GetClientIDbyNameOrID (gi.argv(2));
        if (CheckInvalidClientResponse (NULL, x, gi.argv(2)))
                return;

        kick (g_edicts + 1 + x);
}

/*
ListIP_f
*/
void Cmd_ListIP_f (edict_t *ent)
{
//      int             i;

        //too overflowy, use rcon
        //stuffcmd (ent, "rcon sv listip\n");
        SV_Cmd_ListIP_f (ent);
}

void SV_Cmd_Who (edict_t *targ)
{
        edict_t *ent;
        int i;
        char *what;
        char message[1024] = "";

        char *bp;

        strcat (message,"+---+---------------+---------------+\n|ID |     Name      |  IP or user   |\n+---+---------------+---------------+\n");

        for (i=0 ; i < game.maxclients ; i++)
        {
                ent = g_edicts + 1 + i;
                if (ent->inuse && ent->client->pers.connected) {
                        if (ent->client->pers.username[0])
                                what = ent->client->pers.username;
                        else {
                                what = ent->client->pers.ip;

                                //strncpy (ip,victim->client->pers.ip,sizeof(ip));
                                bp = what;
                                while(*bp && *bp != ':') {
                                        bp++;
                                }
                                *bp = 0;
                        }

                        strcat (message, va("|%3d|%-15.15s|%-15.15s|\n",i,ent->client->pers.netname,what));
                        if (strlen(message) > 900) {
                                gi.cprintf (targ, PRINT_HIGH, "%s", message);
                                message[0] = '\0';
                        }
                }
        }
        strcat (message, "+---+---------------+---------------+\n");

        //r1: safety, print %s not message
        gi.cprintf (targ, PRINT_HIGH, "%s", message);
}

void SV_Cmd_ShowMaps_f (void)
{
        ShowMapList();
}

void SV_Cmd_AddMap_f (void)
{
        size_t  len = 0;
        unsigned int i, start=0;
        char *command=NULL, *p;
        long min_players=0, max_players=0;
        
        if (gi.argc() < 3) {
                gi.cprintf (NULL,PRINT_HIGH,"Usage: sv addmap mapname [min_players] [max_players] [command]\n");
                return;
        }

        if (gi.argc() > 3) {
                min_players = strtol (gi.argv(3), &p, 10);

                if (gi.argv(3) != p && *p != '\0') {
                        // digits followed directly by chars = command
                        min_players = 0;
                        start = 3;
                }
                else if (gi.argc() > 4) {
                        max_players = strtol (gi.argv(4), &p, 10);
                        // same
                        if (gi.argv(4) != p && *p != '\0') {
                                max_players = 0;
                                start = 4;
                        } else if (gi.argc() > 5)
                                start = 5;
                }

                // sanitize
                if (min_players < 0 || max_players < 0 || (max_players > 0 && max_players < min_players)) {
                        gi.cprintf (NULL, PRINT_HIGH, "Invalid player limits.\n");
                        return;
                }
        }

        if (start)
        {
                for (i = start; i < gi.argc(); i++)
                {
                        if (i > start)
                                len += 1;
                        len += strlen (gi.argv(i));
                }

                if (len > 50) {
                        gi.cprintf (NULL, PRINT_HIGH, "Too long command given.\n");
                        return;
                }

                p = command = gi.TagMalloc ((int)len+1, TAG_GAME);

                for (i = start; i < gi.argc(); i++) {
                        if (i > start)
                                *p++ = ' ';
                        strcpy (p, gi.argv(i));
                        p += strlen(gi.argv(i));
                }

                //gi.dprintf("command '%s'\n", command);
        }

        AddToMaplist(gi.argv(2), min_players, max_players, command);

        if (command)
                gi.TagFree (command);

        //gi.dprintf("map %s minp %d maxp %d\n", gi.argv(2), min_players, max_players);
}

void SV_Cmd_RemoveMap_f (void)
{
        int index;

        if (gi.argc() < 3) {
                gi.cprintf (NULL,PRINT_HIGH,"Usage: sv removemap mapname\n");
                return;
        }

        index = IndexByMapname (gi.argv(2), 0);
        if (!index) {
                gi.cprintf (NULL, PRINT_HIGH, "Couldn't find map '%s' in rotation.\n", gi.argv(2));
                return;
        }

        RemoveFromMaplist (index);
        gi.cprintf (NULL, PRINT_HIGH, "%s removed from map list.\n", gi.argv(2));
}

void SV_Cmd_Adminlist_f (void)
{
        edict_t *ent;
        int i;

        for (i = 0; i < game.maxclients; i++) {
                ent = g_edicts +1 + i;
                if (ent->inuse && ent->client->pers.adminpermissions)
                        gi.cprintf (NULL, PRINT_HIGH, "ADMIN: %s (%s)\n", ent->client->pers.netname, ent->client->pers.ip);
        }

}

/*
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
*/
//void  DumpClasses(void);
void    Cmd_Uptime_f (edict_t *ent);
void    TestClasses(void);
void    ListGhosts(void);
void    GrappleCount(void);
void    SV_Cmd_Search_f (edict_t *ent);
extern const char *defaultlightlevels[];
void EXPORT ServerCommand (void)
{
        char    *cmd;
        char text[125]={0};
        int i;

        cmd = gi.argv(1);

        if (Q_stricmp (cmd, "kickobs") == 0)
                SV_Cmd_Kickobs_f ();
        else if (Q_stricmp (cmd, "kick") == 0)
                SV_Cmd_Kick_f ();       
        else if (Q_stricmp (cmd, "adminlist") == 0)
                SV_Cmd_Adminlist_f ();
        else if (Q_stricmp (cmd, "addip") == 0)
                SV_Cmd_AddIP_f ();
        else if (Q_stricmp (cmd, "removeip") == 0)
                SV_Cmd_RemoveIP_f ();
        else if (Q_stricmp (cmd, "invert") == 0)
                SV_Invert_f ();
        else if (Q_stricmp (cmd, "search") == 0)
                SV_Cmd_Search_f (NULL);
        else if (Q_stricmp (cmd, "listip") == 0)
                SV_Cmd_ListIP_f (NULL);
        else if (Q_stricmp (cmd, "writeip") == 0) {
                int ret = SV_Cmd_WriteIP_f ();
                if (ret == -1) {
                        gi.cprintf (NULL, PRINT_HIGH, "Couldn't write banlist to disk.\n");
                } else {
                        gi.cprintf (NULL, PRINT_HIGH, "Wrote %d bans to disk.\n", ret);
                }
        } else if (Q_stricmp (cmd, "addaccount") == 0)
                SV_Cmd_Addaccount_f ();
        else if (Q_stricmp (cmd, "listaccounts") == 0)
                SV_Cmd_Listaccounts_f ();
        else if (Q_stricmp (cmd, "listwordfilters") == 0)
                ListWordFilters ();
        else if (Q_stricmp (cmd, "delaccount") == 0)
                SV_Cmd_Deleteaccount_f ();
        else if (Q_stricmp (cmd, "enableaccount") == 0)
                SV_Cmd_Modifyaccount_f (1);
        else if (Q_stricmp (cmd, "disableaccount") == 0)
                SV_Cmd_Modifyaccount_f (0);
        else if (Q_stricmp (cmd, "updateuserinfo") == 0)
                SV_Cmd_Updateuserinfo_f();
        else if (Q_stricmp (cmd, "showmapcycles") == 0)
                SV_Cmd_ShowMaps_f ();
        else if (Q_stricmp (cmd, "reloadmapcycles") == 0)
                //InitMapCycles ();
                UpdateMapCycles();
        else if (Q_stricmp (cmd, "addmap") == 0)
                SV_Cmd_AddMap_f ();
        else if (Q_stricmp (cmd, "removemap") == 0)
                SV_Cmd_RemoveMap_f ();
        else if (Q_stricmp (cmd, "saveuseraccounts") == 0)
                if (accounts->string[0]) {
                        SaveUserAccounts (accounts->string);
                } else {
                }
        else if (Q_stricmp (cmd, "who") == 0)
                SV_Cmd_Who (NULL);
        else if (Q_stricmp (cmd,"uptime") == 0) {
                Cmd_Uptime_f (NULL);
        } else
        if (Q_stricmp (cmd, "dmgreload") == 0) {
          gi.dprintf("Reloading damages\n");
          LoadDamageOverrideTable();
        } else
        if (Q_stricmp (cmd, "dmgset") == 0) {
          char*s = gi.args();
          gi.dprintf("DmgSet: %s\n", s+7);
          ModifyOverride(NULL, s+7);
        } else
        if (Q_stricmp (cmd, "entcount") == 0)
                GrappleCount ();
        //else if (Q_stricmp (cmd, "dumpclasses") == 0)
        //      DumpClasses();
        else if (Q_stricmp (cmd, "listghosts") == 0)
                ListGhosts();
        else if (Q_stricmp (cmd, "brightness") == 0) {
                int i;
                for (i = 0; i < 11;i++) {
                        gi.configstring(CS_LIGHTS+i,"z");
                }
        } else if (Q_stricmp (cmd, "fulgore") == 0) {
                int i;
                for (i = 0; i < 11;i++) {
                        gi.configstring(CS_LIGHTS+i,"a");
                }
        } else if (Q_stricmp (cmd, "darkness") == 0) {
                int i, j;
                int n = atoi(gi.argv(2));
                if (n < 1 || n > 20) n = 10;
                for (i = 0; i < 12;i++) {
                  char light[128];
                  int l = strlen(defaultlightlevels[i]);
                  for (j = 0; j < l; j++) {
                    light[j] = defaultlightlevels[i][j] - (char)n;
                    if (light[j] < 'a') light[j] = 'a';
                  }
                  light[l] = 0;
                  gi.configstring(CS_LIGHTS+i, light);
                }
        } else if (Q_stricmp (cmd, "monolight") == 0) {
                int i, j;
                int n = atoi(gi.argv(2));
                if (n < 0) n = 0;
                else
                if (n > 20) n = 20;
                for (i = 0; i < 12;i++) {
                  char light[128];
                  int l = strlen(defaultlightlevels[i]);
                  for (j = 0; j < l; j++) {
                    light[j] = 'a'+n;
                  }
                  light[l] = 0;
                  gi.configstring(CS_LIGHTS+i, light);
                }
        } else if (Q_stricmp (cmd, "normal") == 0) {
                int     i;
                for (i = 0; i < 12; i++)
                {
                        gi.configstring (CS_LIGHTS+i, defaultlightlevels[i]);
                }
        } else if (Q_stricmp (cmd, "darkness1") == 0) {
                gi.bprintf (PRINT_HIGH,"Hey! Who killed the lights? :P\n");
                //
                // Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
                //

                        // 0 normal
                        gi.configstring(CS_LIGHTS+0, "e");
                        
                        // 1 FLICKER (first variety)
                        gi.configstring(CS_LIGHTS+1, "eefeegeejjigheefggiheef");
                        
                        // 2 SLOW STRONG PULSE
                        gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
                        
                        // 3 CANDLE (first variety)
                        gi.configstring(CS_LIGHTS+3, "eeeeeaaaaaeeeeeaaaaaabcdefgabcdefg");
                        
                        // 4 FAST STROBE
                        gi.configstring(CS_LIGHTS+4, "eaeaea");
                        
                        // 5 GENTLE PULSE 1
                        gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
                        
                        // 6 FLICKER (second variety)
                        gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");
                        
                        // 7 CANDLE (second variety)
                        gi.configstring(CS_LIGHTS+7, "gggaaaabcdefgggggaaaagggaagg");
                        
                        // 8 CANDLE (third variety)
                        gi.configstring(CS_LIGHTS+8, "fffaaafffaaafffabcdefaaaaffffabcdeffffaaaa");
                        
                        // 9 SLOW STROBE (fourth variety)
                        gi.configstring(CS_LIGHTS+9, "aaaaaaaagggggggg");
                        
                        // 10 FLUORESCENT FLICKER
                        gi.configstring(CS_LIGHTS+10, "ffafaffffaffafafaaafafffa");

                        // 11 SLOW PULSE NOT FADE TO BLACK
                        gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

                        // 12 Fast pulse
                        gi.configstring(CS_LIGHTS+11, "acegikmoqrpnljhfdb");
        }
        else if (Q_stricmp (cmd, "darkness2") == 0) {
                gi.bprintf (PRINT_HIGH,"Hey! Who killed the lights? :P\n");
                //
                // Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
                //

                        // 0 normal
                        gi.configstring(CS_LIGHTS+0, "d");
                        
                        // 1 FLICKdR (first varidty)
                        gi.configstring(CS_LIGHTS+1, "ddfddgddjjighddfggihddf");
                        
                        // 2 SLOW STRONG PULSd
                        gi.configstring(CS_LIGHTS+2, "abcddfghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfddcba");
                        
                        // 3 CANDLd (first varidty)
                        gi.configstring(CS_LIGHTS+3, "dddddaaaaadddddaaaaaabcddfgabcddfg");
                        
                        // 4 FAST STROBd
                        gi.configstring(CS_LIGHTS+4, "dadada");
                        
                        // 5 GdNTLd PULSd 1
                        gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
                        
                        // 6 FLICKdR (sdcond varidty)
                        gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");
                        
                        // 7 CANDLd (sdcond varidty)
                        gi.configstring(CS_LIGHTS+7, "gggaaaabcddfgggggaaaagggaagg");
                        
                        // 8 CANDLd (third varidty)
                        gi.configstring(CS_LIGHTS+8, "fffaaafffaaafffabcddfaaaaffffabcddffffaaaa");
                        
                        // 9 SLOW STROBd (fourth varidty)
                        gi.configstring(CS_LIGHTS+9, "aaaaaaaagggggggg");
                        
                        // 10 FLUORdSCdNT FLICKdR
                        gi.configstring(CS_LIGHTS+10, "ffafaffffaffafafaaafafffa");

                        // 11 SLOW PULSd NOT FADd TO BLACK
                        gi.configstring(CS_LIGHTS+11, "abcddfghijklmnopqrrqponmlkjihgfddcba");

                        // 12 Fast pulsd
                        gi.configstring(CS_LIGHTS+11, "acdgikmoqrpnljhfdb");
        }
        else if (Q_stricmp (cmd, "darkness3") == 0) {
                int i = 0;
                for (i = 0;i < 12;i++) {
                        gi.configstring(CS_LIGHTS+i, "abcdefghijklmnopqrrqponmlkjihgfedcba");
                }
        }
        else if (Q_stricmp (cmd, "centerprint") == 0){
                for (i=2;i<gi.argc();i++){
                        if (i>5)
                                break;
                        strcat(text,gi.argv(i));
                        strcat(text,"\n");
                }
                centerprint_all (text);
        }
        //Update server files via rcon
        else if (!Q_stricmp (cmd, "update") && sv_extended) {
            if (strlen(sv_updater->string) && !strstr(sv_updater->string, "../")) {
                FILE *f = fopen(va("%s/%s", gamedir->string, sv_updater->string), "r");
                if (!f) {
                    gi.dprintf("ERROR: Failed to open autoupdater executable %s.\n", sv_updater->string);
                    return;
                }
            }
            else if (!Q_stricmp(sv_updater->string, "")) {
                    gi.dprintf("ERROR: No autoupdater executable specified.\n");
                    return;
            }
            
            if (!Q_stricmp (gi.argv(2), "r1q2ded")) {
                gi.dprintf ("GLOOM: R1Q2ded update initialized...\n");
                gi.cprintf(NULL, PRINT_HIGH, "Warning: Server going down for restart, this should only take under a minute.\n");
                gi.AddCommandString("update");
            }
            else
                SV_Cmd_Update_f();
        }
        //r1: test error handling. !!! note we cause fatal error to be thrown next frame
        //since calling this from the console is an event 
        else if (Q_stricmp (cmd, "crashgame") == 0) {
                gi.error ("Manual crash initiated");
        }
        else if (Q_stricmp (cmd, "globalstuff") == 0){
                for (i=2;i<gi.argc();i++){
                        strcat(text,gi.argv(i));
                        strcat(text," ");
                }
                strcat(text,"\n");
                stuff_all (text);
        }
        else if (sv_testing) {
            sv_extended = true; //The only time this can be called with sv_testing set to true is while the addcommandstring("get test")
            sv_testing = false;
        }
        else
                gi.dprintf ("Unknown server command \"%s\"\n", cmd);
}

