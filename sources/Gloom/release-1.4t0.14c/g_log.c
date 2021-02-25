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

#ifdef ZZLIB
        #include "zzlib/zzlib.h"
        #include <sys/stat.h>
#endif

FILE *adminlogf = NULL;
FILE *gloomlog = NULL;

char gloomlogname[MAX_QPATH];
char lastlogname[MAX_QPATH];

iplog_t *iplog;

int iplog_position = 0;

int stringContains(char *buff1, char *buff2);

#define MAX_CHANGECLASS_LOG 10

typedef struct changeclasslog_s
{
        char    netname[16];
        int             from;
        int             to;
        int             time;
} changeclasslog_t;

int     changeclasslogposition = 0;
changeclasslog_t changeclasslog[MAX_CHANGECLASS_LOG];

void log_death (edict_t *killer, edict_t *killed, int mod)
{
        if (gloomlog) {
                if (killer->client)
                        fprintf (gloomlog, "0\t%u\t%s\t%s\t%d\n", (unsigned int)time(0), killer->client->pers.netname, killed->client->pers.netname, mod);
                else
                        fprintf (gloomlog, "0\t%u\t%s\t%s\t%d\n", (unsigned int)time(0), killer->classname, killed->client->pers.netname, mod);
                fflush(gloomlog);
        }
}

void log_connection (edict_t *ent)
{
        if (gloomlog) {
                fprintf (gloomlog, "1\t%u\t%s\n", (unsigned int)time(0), ent->client->pers.netname);
                fflush(gloomlog);
        }
}

void log_disconnection (edict_t *ent)
{
        if (gloomlog) {
                fprintf (gloomlog, "2\t%u\t%s\n", (unsigned int)time(0), ent->client->pers.netname);
                fflush(gloomlog);
        }
}

void log_namechange (char *oldname, char *newname)
{
        if (gloomlog) {
                fprintf (gloomlog, "3\t%u\t%s\t%s\n", (unsigned int)time(0), oldname, newname);
                fflush(gloomlog);
        }
}

void log_changeclass (edict_t *ent, int newclass)
{
        if (gloomlog) {
                fprintf (gloomlog, "4\t%u\t%s\t%d\n", (unsigned int)time(0), ent->client->pers.netname, newclass);
                fflush(gloomlog);
        }
}

void AddToChangeLog (edict_t *ent, int newclass)
{
        //r1: log live changeclasses for spotting death evaders.
                int i = changeclasslogposition++;

        if (changeclasslogposition == MAX_CHANGECLASS_LOG)
                changeclasslogposition = 0;

        strcpy (changeclasslog[i].netname, ent->client->pers.netname);
        changeclasslog[i].from = ent->client->resp.class_type;
        changeclasslog[i].to = newclass;
        changeclasslog[i].time = time(0);
}

void Cmd_ViewChangeLog (edict_t *ent) {
        int i;
        int count;
        char message[800];

        i = changeclasslogposition-1;
        if (i == -1)
                i = 9;

        message[0] = '\0';

        for (count = 0; count < MAX_CHANGECLASS_LOG; count++) {
                if (changeclasslog[i].time)
                        strcat (message, va ("%d secs ago: %s -> %s by %s\n", time(0) - changeclasslog[i].time, classlist[changeclasslog[i].from].classname, classlist[changeclasslog[i].to].classname, changeclasslog[i].netname));
                if (--i == -1)
                        i = 9;
        }

        gi.cprintf (ent, PRINT_HIGH, "%s", message);
}

void InitChangeClassLog (void)
{
        memset (&changeclasslog, 0, sizeof(changeclasslog));
}

void log_killstructure (edict_t *attacker, edict_t *ent)
{
        if (gloomlog) {
                fprintf (gloomlog, "5\t%u\t%s\t%s\t%d\t%d\n", (unsigned int)time(0), attacker->client->pers.netname, ent->classname, ent->s.number, meansOfDeath);
                fflush(gloomlog);
        }
}
//FIXME: what is snumber?
//structure number, for determining unique structures
void log_makestructure (edict_t *creator, char *object, int snumber)
{
        if (gloomlog) {
                fprintf (gloomlog, "6\t%u\t%s\t%d\n", (unsigned int)time(0), creator->client->pers.netname, snumber);
                fflush(gloomlog);
        }
}

void log_iplog (char *name, char *ip)
{
        int i;
        char *x;
        // patent pending "roll over the log" method

        i = iplog_position++;

        if (iplog_position > MAX_IP_LOG)
                iplog_position = 0;

//      gi.dprintf("ip log position: %d\n", i);
        memset (&iplog[i], 0, sizeof(iplog_t));

        strncpy (iplog[i].name, name, (sizeof (iplog[i].name)) -1);
        strncpy (iplog[i].ip, ip, (sizeof (iplog[i].ip)) -1);
        x = strstr (iplog[i].ip, ":");
        if (x)
                *x = 0;
        iplog[i].time = time(NULL);
        iplog[i].inuse = true;
}

void Cmd_EvadeList_f (edict_t *ent)
{
        edict_t *client;
        qboolean gotone = false;
        int i, j, found, threshold, banid;

        if (gi.argc() < 2)
                threshold = 10;
        else
                threshold = atoi(gi.argv(1));

        for (i=0 ; i<game.maxclients ; i++)
        {
                if (!g_edicts[i+1].inuse)
                        continue;

                client = g_edicts + i + 1;

                banid = ClientMatchesClassB(client);
                if (banid < 0)
                        continue;

                found = 0;

                for (j = MAX_IP_LOG; j >= 0; j--)
                {
                        if (!iplog[j].inuse)
                                continue;

                        if (!Q_stricmp (iplog[i].name, client->client->pers.netname))
                                found++;
                }

                if (found < threshold) {
                        gotone = true;
                        gi.cprintf (ent, PRINT_HIGH, "%s[%s], connected %d times [%s:%s]\n", client->client->pers.netname, client->client->pers.ip, found, IPArrayToString(banid), ipfilters[banid].reason);
                }
        }

        if (!gotone)
                gi.cprintf (ent, PRINT_HIGH, "No matches found at the moment.\n");
}


//same, but from sv cmd
void SV_Cmd_Search_f (edict_t *ent)
{
        char            message[1024];
        int                     i;
        qboolean        ip, substring;
        char            *p;
        char            *givenstring;
        char            timestr[64];

        if (ent)
        {
                if (gi.argc() < 3) {
                        gi.cprintf (ent, PRINT_HIGH, "Syntax: @search (ip|subip|name|subname) searchstring\n");
                        return;
                }

                p = gi.argv(1);
                givenstring = gi.argv(2);
        }
        else
        {
                if (gi.argc() < 4) {
                        gi.cprintf (ent, PRINT_HIGH, "Syntax: sv search (ip|subip|name|subname) searchstring\n");
                        return;
                }

                p = gi.argv(2);
                givenstring = gi.argv(3);
        }

        if (Q_strncasecmp(p, "sub", 3) == 0) {
                substring = true;
                p+= 3;
        } else {
                substring = false;
        }

        if (!Q_stricmp (p, "ip"))
                ip = true;
        else
                ip = false;

        strcpy (message, va("Results for '%s':\n", givenstring));

        for (i = MAX_IP_LOG; i >= 0; i--)
        {

                if (!iplog[i].inuse)
                        continue;

                strftime (timestr, sizeof(timestr)-1, "%Y-%m-%d %H:%M", localtime(&iplog[i].time));

                if (ip) {
                        if (!substring) {
                                if (!Q_stricmp (iplog[i].ip, givenstring)) {
                                        strcat (message, va("[%s] %s: %s\n", timestr, iplog[i].ip, iplog[i].name));
                                        if (strlen(message) > 960)
                                                break;
                                }
                        } else {
                                if (stringContains (iplog[i].ip, givenstring)) {
                                        strcat (message, va("[%s] %s: %s\n", timestr, iplog[i].ip, iplog[i].name));
                                        if (strlen(message) > 960)
                                                break;
                                }
                        }
                } else {
                        if (!substring) {
                                if (!Q_stricmp (iplog[i].name, givenstring)) {
                                        strcat (message, va("[%s] %s: %s\n", timestr, iplog[i].name, iplog[i].ip));
                                        if (strlen(message) > 960)
                                                break;
                                }
                        } else {
                                if (stringContains (iplog[i].name, givenstring)) {
                                        strcat (message, va("[%s] %s: %s\n", timestr, iplog[i].name, iplog[i].ip));
                                        if (strlen(message) > 960)
                                                break;
                                }
                        }
                }
        }

        gi.cprintf (ent, PRINT_HIGH, "%s", message);
}


//search through IP logs (here instead of clientcommand so i don't have to g_local
//the iplog)
void Cmd_Search_f (edict_t *ent)
{
        SV_Cmd_Search_f (ent);
}

/*
        char            message[1024];
        int                     i;
        char            *p;
        qboolean        ip, substring;
        char            *givenstring;

        SV_Cmd_Search_f (ent);
        return;

        if (gi.argc() < 3) {
                gi.cprintf (ent, PRINT_HIGH, "Syntax: @search (ip|subip|name|subname) searchstring\n");
                return;
        }

        p = gi.argv(1);

        if (Q_strncasecmp(p, "sub", 3) == 0) {
                substring = true;
                p+= 3;
        } else {
                substring = false;
        }

        if (!Q_stricmp (p, "ip"))
                ip = true;
        else
                ip = false;

        givenstring = gi.argv(2);

        strcpy (message, va("Results for '%s'\n", givenstring));

        for (i = MAX_IP_LOG; i >= 0; i--) {

                if (!iplog[i].inuse)
                        continue;

                if (ip) {
                        if (!substring) {
                                if (!Q_stricmp (iplog[i].ip, givenstring)) {
                                        strcat (message, va("[%d] %s: %s\n", iplog_position - i, iplog[i].ip, iplog[i].name));
                                        if (strlen(message) > 960)
                                                break;
                                }
                        } else {
                                if (stringContains (iplog[i].ip, givenstring)) {
                                        strcat (message, va("[%d] %s: %s\n", iplog_position - i, iplog[i].ip, iplog[i].name));
                                        if (strlen(message) > 960)
                                                break;
                                }
                        }
                } else {
                        if (!substring) {
                                if (!Q_stricmp (iplog[i].name, givenstring)) {
                                        strcat (message, va("[%d] %s: %s\n", iplog_position - i, iplog[i].name, iplog[i].ip));
                                        if (strlen(message) > 960)
                                                break;
                                }
                        } else {
                                if (stringContains (iplog[i].name, givenstring)) {
                                        strcat (message, va("[%d] %s: %s\n", iplog_position - i, iplog[i].name, iplog[i].ip));
                                        if (strlen(message) > 960)
                                                break;
                                }
                        }
                }
        }

        gi.cprintf (ent, PRINT_HIGH, "%s", message);
}*/

//write out file to disk
void SaveIPLog (char * filename)
{
        FILE    *iplogfile = NULL;
        int             records = 0;
        int             i = 0;
#ifdef ZZLIB
        int             len;
        int             finallen;
#endif

        iplogfile = fopen(filename, "wb");

        if (iplogfile) {
                for (i = 0; i < MAX_IP_LOG; i++) {
                        if (iplog[i].inuse) {
                                fprintf (iplogfile, "%s\n%s\n%u\n", iplog[i].name, iplog[i].ip, iplog[i].time);
                                fflush(gloomlog);
                                records++;
                        }
                }
#ifdef ZZLIB
                len = ftell(iplogfile);
#endif
                fclose (iplogfile);
#ifdef ZZLIB
                finallen = compress (filename);
                gi.dprintf ("SaveIPLog: %d entries written to %s\n    zzip: %d bytes compressed to %d\n", records, filename, len, finallen);
#else
                gi.dprintf ("SaveIPLog: %d entries written to %s\n", records, filename);
#endif
        } else {
                gi.dprintf ("SaveIPLog: Saving of log failed!!\n");
        }
}

//read in file from disk
void ReadIPLogs (char * filename)
{
#ifdef ZZLIB
        char    *contents;
        int             len;
#else
        FILE    *iplogfile;
#endif

        //initialize structure, gcc may complain otherwise
        /*for (i = 0; i < MAX_IP_LOG; i++) {
                iplog[i].inuse = false;
                iplog[i].name[0] = '\0';
                iplog[i].ip[0] = '\0';
        }*/

        //memset (&iplog, 0, sizeof(iplog));


        //compress (filename);
#ifndef ZZLIB
        iplogfile = fopen(filename, "rb");
        if (iplogfile != NULL) {
                int             stat = 0;
                int             lines = 0;
                size_t  len;
                int             nowtime = time(NULL);
                char    line[100];
                while (lines < MAX_IP_LOG) {
                        //memset (line, 0, sizeof(line));
                        fgets(line, 100, iplogfile);
                        len = strlen(line);
                        if (len) {
                                line[len-1] = '\0';

                                if (stat == 0)
                                {
                                        strncpy(iplog[lines].name, line, (sizeof(iplog[lines].name))-1);
                                }
                                else if (stat == 1)
                                {
                                        strncpy(iplog[lines].ip, line, (sizeof(iplog[lines].ip))-1);
                                }
                                else if (stat == 2)
                                {
                                        iplog[lines].time = (time_t)strtoul (line, 0, 10);
                                        if (iplog[lines].time == 0)
                                        {
                                                gi.dprintf ("WARNING: iplog entry with 0 time index (old file format?)\n");
                                                iplog[lines].time = nowtime;
                                                iplog[lines].inuse = true;
                                                stat = 0;
                                                lines++;
                                                continue;
                                        }
                                }
                                
                                if (++stat == 3)
                                {
                                        iplog[lines].inuse = true;
                                        stat = 0;
                                        lines++;
                                }
                        }
                        if (feof(iplogfile))
                                break;
                }
                fclose (iplogfile);

                iplog_position = lines;

                gi.dprintf ("ReadIPLogs: Read %d IP log entries from %s\n", lines, filename);
        } else {
                gi.dprintf ("ReadIPLogs: Unable to read IP log file from %s\n", filename);
        }

#else
        if (contents = GloomReadFile (filename, &len)) {
                char *line;
                int num;
                int stat;
                int lines;
                int finallen;

                contents = decompress_block (contents, len, &finallen);

                if (!contents) {
                        gi.dprintf ("ReadIPLogs: decompression error reading IP logs\n");
                        return;
                }

                lines = 0;
                stat = 0;
                num = 0;

                while ((line = GetLine(&contents, &finallen)) != NULL) {
                        if (stat == 0)
                                strncpy(iplog[lines].name, line, (sizeof(iplog[lines].name))-1);
                        else if (stat == 1) {
                                strncpy(iplog[lines].ip, line, (sizeof(iplog[lines].ip))-1);
                        }
                        if (++stat == 2) {
                                iplog[lines].inuse = true;
                                stat = 0;
                                lines++;
                        }
                }

                iplog_position = lines;

                gi.dprintf ("ReadIPLogs: Read %d IP log entries from %s\n", lines, filename);
        }

#endif
}

void CloseGamelog()
{
#ifdef ZZLIB
                int len, finallen;
                char oldname[MAX_QPATH];
                char newname[MAX_QPATH];
#endif

        if (gloomlog) {
                strcpy (lastlogname, gloomlogname);

                #ifdef ZZLIB
                len = ftell (gloomlog);
                #endif

                fclose (gloomlog);
                gloomlog = NULL;

                #ifdef ZZLIB
                finallen = compress (gloomlogname);
                gi.dprintf ("CloseGamelog: compressed logfile from %d bytes to %d\n", len, finallen);
                sprintf (oldname, gloomlogname);
                sprintf (newname, "%s.zz", gloomlogname);
                rename (oldname, newname);
                strcat (lastlogname, ".zz");
                #endif

                #ifdef ZZLIB
                        gi.dprintf("CloseGamelog: closed game log file %s\n", newname);
                #else
                        gi.dprintf("CloseGamelog: closed game log file %s\n", gloomlogname);
                #endif
        }
}

void OpenGamelog(void)
{
        char *filename;

        filename = va ("%s/%s/logs/%s-%u.txt", gamedir->string, game.port, level.mapname, time(0));

        strncpy (gloomlogname, filename, sizeof(gloomlogname)-1);

        gloomlog = fopen (gloomlogname, "w");

        if (gloomlog) {
                gi.dprintf ("OpenGamelog: writing to game log file %s\n", gloomlogname);
        } else {
                gi.dprintf ("OpenGamelog: error opening game log file %s\n", gloomlogname);
        }

}

void OpenAdminLog (char *fname)
{
        time_t t;
        time(&t);
        adminlogf = fopen (fname, "awb");
        if (!adminlogf) {
                gi.dprintf ("OpenAdminLog: couldn't open %s\n", fname);
        }
}

void CloseAdminLog (void)
{
        if (adminlogf)
                fclose (adminlogf);
}

void AdminLog (edict_t *ent, char *message)
{
        if (adminlogf) {
                time_t t;
                time(&t);
                fprintf (adminlogf, "[%.24s] <%s> %s %s %s\n", ctime(&t), ent->client->pers.username, gi.argv(0), gi.args(), message);
                fflush(gloomlog);
        }
}

void AdminLogNewMap (char *mapname)
{
        if (adminlogf) {
                time_t t;
                time(&t);
                fprintf (adminlogf, "=== Admin Log Started at %.24s on map %s ===\n", ctime(&t), mapname);
                fflush(gloomlog);
        }
}
