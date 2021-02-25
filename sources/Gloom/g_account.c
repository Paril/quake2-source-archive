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

typedef struct useraccount_s useraccount_t;

struct useraccount_s
{
        useraccount_t   *next;
        qboolean                inuse;
        qboolean                enabled;
        unsigned int    permissions;
        char    *username;
        char    *password;
};

useraccount_t   useraccounts;

int AddAccount (char * username, char * password, unsigned int permissions)
{
        useraccount_t   *user;
        int i=1;

        if (*username == '\0' || *password == '\0')
                return 0;

        user = &useraccounts;

        while(user->next) {
                user = user->next;
                i++;
        }

        user->next = gi.TagMalloc(sizeof(useraccount_t), TAG_GAME);
        user = user->next;
        // strlen doesn't count null byte
        user->username = gi.TagMalloc((int)strlen(username)+1, TAG_GAME);
        user->password = gi.TagMalloc((int)strlen(password)+1, TAG_GAME);

        user->permissions = permissions;

        strncpy (user->username, username, strlen(username)+1);
        strncpy (user->password, password, strlen(password)+1);
        user->inuse = user->enabled = true;

        return i;
}

qboolean RemoveAccount (int index)
{
        useraccount_t   *last, *temp;
        int i;

        last = temp = &useraccounts;
        i = index;
        while (temp->next)
        {
                last = temp;
                temp = temp->next;
                i--;
                if (i == 0)
                        break;
                else if (i < 0)
                        return false;
        }

        // if list ended before we could get to index
        if (i>0)
                return false;

        // just copy the next over, don't care if it's null
        last->next = temp->next;

        // free!
        gi.TagFree(temp->username);
        gi.TagFree(temp->password);
        gi.TagFree(temp);
        return true;
}

void ClearAccounts (void)
{
        useraccount_t *temp, *next;
        int i=0;

        temp = &useraccounts;
        // skip the first
        temp = temp->next;

        while(temp) {
                // save next for processing
                next = temp->next;
                gi.TagFree(temp->username);
                gi.TagFree(temp->password);
                gi.TagFree(temp);
                temp = next;
                i++;
        }
}

void SV_Cmd_Listaccounts_f (void)
{
        useraccount_t *temp;

        if (!accounts->string[0]) {
                gi.cprintf (NULL, PRINT_HIGH, "Accounts are disabled.\n");
                return;
        }

        gi.cprintf (NULL, PRINT_HIGH, "+--------+--------+----------+-------+\n");
        gi.cprintf (NULL, PRINT_HIGH, "|Username|Password|Permission|Enabled|\n");
        gi.cprintf (NULL, PRINT_HIGH, "+--------+--------+----------+-------+\n");

        temp = &useraccounts;
        while (temp->next)
        {
                temp = temp->next;
                gi.cprintf (NULL, PRINT_HIGH, "|%-8s|%-8s|%10d|%-7s|\n", temp->username,temp->password, temp->permissions, temp->enabled ? "Yes" : "No");
        }

        gi.cprintf (NULL, PRINT_HIGH, "+--------+--------+----------+-------+\n");
}

int CheckAccount (char * username, char * password, unsigned int *permissions)
{
        useraccount_t *temp;
        int i = 1;

        temp = &useraccounts;
        while (temp->next)
        {
                temp = temp->next;

                if (!strcmp (temp->username, username) && !strcmp (temp->password, password)) {
                        if (temp->enabled) {
                                *permissions = temp->permissions;
                                return i;
                        } 
                        else
                        {
                                return -1;
                        }
                }

                i++;
        }

        return 0;
}

int FindAccount (char *username)
{
        useraccount_t *temp;
        int i = 1;

        temp = &useraccounts;
        while (temp->next)
        {
                temp = temp->next;

                if (!Q_stricmp (temp->username, username)) {
                        return i;
                }

                i++;
        }

        return 0;
}

qboolean ToggleAccount (int index, qboolean enable)
{
        useraccount_t   *temp;
        int i;

        temp = &useraccounts;
        i = index;
        while (temp->next)
        {
                //last = temp;
                temp = temp->next;
                i--;
                if (i == 0)
                        break;
                else if (i < 0)
                        return false;
        }

        // if list ended before we could get to index
        if (i>0)
                return false;

        temp->enabled = enable;

        return true;
}


qboolean SetPermissions (int index, unsigned int permissions)
{
        useraccount_t   *temp;
        int i;

        temp = &useraccounts;
        i = index;
        while (temp->next)
        {
                //last = temp;
                temp = temp->next;
                i--;
                if (i == 0)
                        break;
                else if (i < 0)
                        return false;
        }

        // if list ended before we could get to index
        if (i>0)
                return false;

        temp->permissions = permissions;

        return true;
}

// support functions

void Cmd_Addaccount (edict_t *ent, char * username, char * password, unsigned int permissions)
{

        int i = 0;

        //check for illegal characters
        if (strstr(username," ") != NULL || strstr(password," ") != NULL) {
                gi.cprintf (ent,PRINT_HIGH,"No spaces allowed in username or password.\n");
                return;
        }

        if (strstr(username,":") != NULL || strstr(password,":") != NULL) {
                gi.cprintf (ent,PRINT_HIGH,"No colons allowed in username or password.\n");
                return;
        }

        //insanity check
        if (strstr(username,"\n") != NULL || strstr(username,"\n") != NULL)
                return;

        if (FindAccount(username)) {
                gi.cprintf (ent,PRINT_HIGH,"The username '%s' is already taken!\n",username);
                return;
        }

        if (strlen(username) > 8) {
                gi.cprintf (ent,PRINT_HIGH,"Username '%s' is too long. Max of 8 characters.\n",username);
                return;
        }

        if (strlen(password) > 8) {
                gi.cprintf (ent,PRINT_HIGH,"Password '%s' is too long. Max of 8 characters.\n",username);
                return;
        }

        i = AddAccount(username, password, permissions);

        gi.cprintf (ent,PRINT_HIGH,"Account added, %d accounts total\n", i);
}

void Cmd_Removeaccount (edict_t *ent, char * username)
{
        int i;

        if ((i = FindAccount(username))) {
                RemoveAccount(i);

                //show report
                gi.cprintf (ent,PRINT_HIGH,"User '%s' deleted.\n",username);

        } else {

                //user not found
                gi.cprintf (ent,PRINT_HIGH,"User '%s' not found.\n",username);

        }

}

void Cmd_Modifyaccount (edict_t *ent, char *username, int enable, unsigned int permissions)
{
        int i;

        if ((i = FindAccount(username))) {
                ToggleAccount(i, enable ? true : false);
                if (permissions)
                        SetPermissions (i, permissions);

                //show results
                gi.cprintf (ent,PRINT_HIGH,"Account has been %s%s.\n", enable ? "enabled" : "disabled", permissions ? ", permissions modified." : "");

        } else {
                //not found
                gi.cprintf (ent,PRINT_HIGH,"User '%s' not found.\n",username);
        }
}

// load/save

int SaveUserAccounts (char * filename)
{
        FILE    *accounts = NULL;
        int             records = 0;
        useraccount_t   *user;
#ifdef ZZLIB
        int             len;
        int             finallen;
#endif

        user = &useraccounts;

        // don't do squat if no records
        if (user->next) {
        accounts = fopen(filename, "wb");
        if (accounts != NULL) {

                while(user->next) {
                        user = user->next;
                        fprintf(accounts,"%s\n%s\n%d\n%u\n", user->username, user->password, user->enabled ? (int)1 : (int)0, user->permissions);
                        records++;
                }

                //fseek (accounts, SEEK_END, 0);
#ifdef ZZLIB
                len = ftell (accounts);
#endif
                fclose (accounts);
#ifdef ZZLIB
                finallen = compress (filename);
                gi.dprintf ("SaveUserAccounts: %d accounts written to %s.\n    zzip: %d bytes compressed to %d.\n", records, filename, len, finallen);
#else
                gi.dprintf ("SaveUserAccounts: %d accounts written to %s.\n", records, filename);
#endif
        } else {
                gi.dprintf ("SaveUserAccounts: Saving of accounts failed!!\n");
        }
        }
        return records;
}

void LoadUserAccounts (char * filename)
{
        FILE    *accounts = NULL;
        int             stat = 0;
        int             lines = 0;
        size_t  len;
        char    username[9],password[9];
        unsigned int permissions;
        
#ifdef ZZLIB
        char    *contents;
#else
        char    line[100];
#endif
        int             enabled=0;

#ifndef ZZLIB
        accounts = fopen(filename, "r");
        if (accounts != NULL) {
                for (;;) {
                        line[0] = '\0';
                        fgets(line, 100, accounts);
                        len = strlen(line);
                        if (len) {
                                line[len-1] = '\0';
                                if (stat == 0)
                                        strncpy(username,line,8);
                                else if (stat == 1)
                                        strncpy(password,line,8);
                                else if (stat == 2) {
                                        enabled = atoi(line);
                                } else if (stat == 3) {
                                        permissions = strtoul (line, 0, 10);
                                }
                                if (++stat == 4) {
                                        username[8] = password[8] = 0;
                                        stat = AddAccount(username,password,permissions);
                                        ToggleAccount(stat,enabled ? true : false);
                                        stat = 0;
                                        lines++;
                                }
                        }
                        if (feof(accounts))
                                break;
                }
                fclose (accounts);
                gi.dprintf ("LoadUserAccounts: Read %d user accounts from %s.\n", lines, filename);
        }
#else
        if (contents = GloomReadFile (filename, &len)) {
                char *line;
                int finallen;
                int     enabled;
                contents = decompress_block (contents, len, &finallen);

                if (!contents) {
                        gi.dprintf ("LoadUserAccounts: decompression error reading accounts.\n");
                        return;
                }

                lines = 0;
                stat = 0;

                while ((line = GetLine(&contents, &finallen)) != NULL) {
                        if (stat == 0)
                                strncpy(username,line,8);
                        else if (stat == 1)
                                strncpy(password,line,8);
                        else if (stat == 2) {
                                enabled = atoi(line);
                        }
                        if (++stat == 3) {
                                stat = AddAccount(username,password);
                                ToggleAccount(stat,enabled ? true : false);
                                stat = 0;
                                lines++;
                        }
                }
                gi.dprintf ("LoadUserAccounts: Read %d user accounts from %s.\n", lines, filename);
        }
#endif
}
