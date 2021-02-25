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

typedef struct maplist_s maplist_t;
struct maplist_s
{
        maplist_t       *next;
        char            *mapname;
        char            *command;
        int                     min_players;
        int                     max_players;
        int                     play_count;
        int                     finish_count;
};

maplist_t maplist;

void UpdateCurrentMapFinishCount(int offset)
{
        int index = currentmapindex;
        maplist_t *templist;

        // to simplify
        if (index == 0)
                return;

        // maps start at index 1, 0 is reserved
        templist = &maplist;
        while (templist->next)
        {
                templist = templist->next;
                index--;
                if (index == 0) {
                        templist->finish_count += offset;
                }
        }
}

void UpdateCurrentMapPlayedCount(int offset)
{
        int index = currentmapindex;
        maplist_t *templist;

        // to simplify
        if (index == 0)
                return;

        // maps start at index 1, 0 is reserved
        templist = &maplist;
        while (templist->next)
        {
                templist = templist->next;
                index--;
                if (index == 0) {
                        templist->play_count += offset;
                        return;
                }
        }
}

void ShowMapList (void)
{
        maplist_t *templist;
        int index = 0;

        templist = &maplist;

        if (!maplist.next) {
                gi.cprintf (NULL, PRINT_HIGH, "No map cycle defined.\n");
                return;
        }

        gi.cprintf (NULL, PRINT_HIGH, "+-----+-----------------+-----+-----+-----+-----+-------|\n");
        gi.cprintf (NULL, PRINT_HIGH, "|Index|     Mapname     | Max | Min | Pld | Com | Ratio |\n");
        gi.cprintf (NULL, PRINT_HIGH, "+-----+-----------------+-----+-----+-----+-----+-------|\n");

        while (templist->next)
        {
                templist = templist->next;
                index++;
                gi.cprintf (NULL, PRINT_HIGH, "| %-3d |%-17s| %-3d | %-3d | %-3d | %-3d |%-5.5f|\n", index, templist->mapname, templist->max_players, templist->min_players, templist->play_count, templist->finish_count, templist->play_count ? ((float)templist->finish_count / (float)templist->play_count) : 0);
        }
        gi.cprintf (NULL, PRINT_HIGH, "+-----+-----------------+-----+-----+-----+-----+-------|\n");
}

unsigned int AddToMaplist(char * mapname, unsigned int min_players, unsigned int max_players, char * command)
{
        maplist_t *templist;

        // don't use index 0, it's special
        unsigned int i = 1;
        int count = 0;
        int complete, played;

        if (*mapname == '\0')
                gi.error("AddToMaplist: empty mapname given.\n");

        templist = &maplist;

        complete = 0;
        played = 0;

        while(templist->next)
        {
                templist = templist->next;
                if (!Q_stricmp (mapname, templist->mapname)) 
                {
                        gi.cprintf (NULL, PRINT_HIGH, "AddToMaplist: ignoring duplicate entry for '%s'\n", mapname);
                        return i;
                }

                if (templist->min_players <= min_players && ((max_players == 0 && templist->max_players == 0) || templist->max_players >= max_players))
                {
                        complete += templist->finish_count;
                        played += templist->play_count;
                        i++;
                        count++;
                }
        }

        templist->next = gi.TagMalloc(sizeof(maplist_t), TAG_GAME);
        templist = templist->next;

        // strlen doesn't count null byte
        templist->mapname = gi.TagMalloc((int)strlen(mapname)+1, TAG_GAME);
        strncpy (templist->mapname, mapname, strlen(mapname)+1);

        if (command) {
                templist->command = gi.TagMalloc((int)strlen(command)+1, TAG_GAME);
                strncpy (templist->command, command, strlen(command)+1);
        }

        templist->min_players = min_players;
        templist->max_players = max_players;

        //r1: use the average so maps inserted after server has been up a while don't skew
        if (count) {
          float f = 1.0f/count;
          templist->finish_count = (int)(complete*f);
          templist->play_count = (int)(played*f);
        } else
          templist->finish_count = templist->play_count = 0;

        //gi.dprintf("adding map %s to last index %d\n", templist->mapname, i);

        return i;
}

qboolean RemoveFromMaplist(unsigned int index)
{
        maplist_t *templist;
        maplist_t *last;

        // maps start at index 1, 0 is reserved
        last = templist = &maplist;
        while (templist->next)
        {
                last = templist;
                templist = templist->next;
                index--;
                if (index == 0)
                        break;
                /*else if (index < 0)
                        return false;*/
        }

        // if list ended before we could get to index
        if (index > 0)
                return false;

        //gi.dprintf("removing index %d, %s from maplist\n", index, templist->mapname);

        // just copy the next over, don't care if it's null
        last->next = templist->next;

        // free!
        gi.TagFree(templist->mapname);
        if (templist->command)
                gi.TagFree(templist->command);
        gi.TagFree(templist);
        return true;
}

int GetMapPlayers (int currentmap, int minormax) {
    maplist_t *templist;
    templist = &maplist;
    unsigned int i = 1;

    while (templist->next) 
        {
        templist = templist->next;
        if (i == currentmap)
            break;
        i++;
    }
    if (minormax == 0) {
        if (templist->min_players)
            return templist->min_players;
        else return 0;
    } else {
        if (templist->max_players)
            return templist->max_players;
        else return -1;
    }
}

void ClearMaplist(void)
{
        maplist_t *templist;
        maplist_t *next;

        // skip the first
        templist = maplist.next;
        maplist.next = NULL;

        while(templist) {
                // save next for processing
                next = templist->next;
                gi.TagFree(templist->mapname);
                if (templist->command)
                        gi.TagFree(templist->command);
                gi.TagFree(templist);
                templist = next;
        }
}

unsigned int CountMaps (void)
{
        maplist_t *templist;
        unsigned int i = 0;

        templist = &maplist;
        while (templist->next)
        {
                templist = templist->next;
                i++;
        }

        return i;
}

unsigned int NextByIndex (unsigned int startindex, unsigned int players)
{
        maplist_t *templist;
        unsigned int i = 1;

        templist = &maplist;
        while (templist->next)
        {
                templist = templist->next;

                if (i > startindex && players >= templist->min_players) {
                        if (templist->max_players == 0 || players <= templist->max_players) {
                                /*if (templist->finish_count < templist->play_count) {
                                        if ((random() * (float)((templist->play_count - templist->finish_count) * 0.25)) <= 1)
                                                continue;
                                }*/
                                return i;
                        }
                }

                i++;
        }

        return 0;
}

unsigned int NextByMagicAlgorithm (unsigned int players)
{
        maplist_t *templist;
        unsigned int i = 1, bestmap = 0, j = 0;
        int *winnars;
        int nummaps = CountMaps();
        int     numInArray = 0;
        float score, best;

        if (!nummaps)
                return 0;

        best = 0x10000;

        winnars = gi.TagMalloc (nummaps*sizeof(int), TAG_GAME);

        templist = &maplist;

        while (templist->next)
        {
                templist = templist->next;

                if (players >= templist->min_players)
                {
                        if ((templist->max_players == 0 || players < templist->max_players) && (currentmapindex != i+1))
                        {
                                score = (float)templist->play_count + (float)(templist->play_count - templist->finish_count)/4;
                                if (score < best)
                                {
                                        numInArray = 1;
                                        best = score;
                                        winnars[0] = i;
                                }
                                else if (score == best)
                                {
                                        winnars[numInArray++] = i;
                                }
                        }
                }

                i++;
        }

        i = (int)(random() * numInArray);
        bestmap = winnars[i];

        gi.TagFree (winnars);

        return bestmap;
}

char * CommandByIndex(unsigned int index)
{
        maplist_t *templist;

        // to simplify
        if (index == 0)
                return NULL;

        // maps start at index 1, 0 is reserved
        templist = &maplist;
        while (templist->next)
        {
                templist = templist->next;
                index--;
                if (index == 0) {
                        return templist->command;
                }
        }

        // list ended before we could get to index
        return NULL;
}

char * MapnameByIndex(unsigned int index)
{
        maplist_t *templist;

        // to simplify
        if (index == 0)
                return NULL;

        // maps start at index 1, 0 is reserved
        templist = &maplist;
        while (templist->next)
        {
                templist = templist->next;
                index--;
                if (index == 0) {
                        return templist->mapname;
                }
        }

        // list ended before we could get to index
        return NULL;
}

unsigned int IndexByMapname(char * mapname, unsigned int startindex)
{
        maplist_t *templist;
        unsigned int i = 1;

        templist = &maplist;
        while (templist->next)
        {
                templist = templist->next;

                if (i > startindex && !Q_stricmp (templist->mapname, mapname)) {
                        //gi.dprintf("mapname %s is at index %d\n", templist->mapname, i);
                        return i;
                }

                i++;
        }

        return 0;
}

// additional support functions

unsigned int ReadMapCycle (char *filename)
{
        FILE *fileptr;
        char    line[MAX_OSPATH];       //FIXME: MAX_OSPATH too short?
        char *p, *c;
        unsigned int mapnum=0, linenum=0;
        long min_players, max_players;

        // try opening the config file
        fileptr = fopen ( filename, "r");

        // if file didn't open, don't do anything more
        if (fileptr == NULL) {
                gi.dprintf ("ReadMapCycle: Unable to read map cycle from %s\n", filename);
                return 0;
        }

        // <mapname><w-s><optional min_pl><w-s><optional max_pl><optional commands>
        while ((p = fgets (line, sizeof (line)-1, fileptr)))
        {
                // line can end with EOF, eg. no w-s
                linenum++;

                // find the first white-space
                while (*p && !isspace(*p))
                        p++;

                // line can't start with white-space
                if (p == line)
                        continue;

                // ignore mission maps and comments
                if (*line == '_' || *line == '#')
                        continue;

                // don't advance if line ended with EOF
                if (*p)
                        *p++ = '\0'; // tag the mapname

                // strtol skips leading w-s
                min_players = strtol (p, &c, 10);

                max_players = 0;

                // don't bother finding more digits, if there weren't any to begin with
                if (p != c) {
                        // try finding more! (notice the pointer swap)
                        max_players = strtol (c, &p, 10);

                        // if we found more, p points to end of digits, otherwise it is unchanged
                }

                // skip any white-space
                while (isspace(*p))
                        p++;

                // no command if we hit end of line
                if (*p == '\0')
                        p = NULL;
                else {
                        char *e = p;
                        // chomp white-space
                        while (*e)
                                e++;
                        while (isspace(*--e))
                                *e = '\0';
                }

                // sanitize values
                if (min_players < 0 || max_players < 0 || (max_players > 0 && max_players < min_players)) {
                        gi.dprintf ("ReadMapCycle: invalid player limits on line %d\n", linenum);
                        continue;
                }

                //FIXME: check if the command was cut?
                AddToMaplist (line, min_players, max_players, p);

                mapnum++;
        }
        fclose (fileptr);

        gi.dprintf ("ReadMapCycle: Read %d maps from %s\n", mapnum, filename);
        return mapnum;
}

typedef struct infosave_s
{
        struct infosave_s       *next;
        char    mapname[MAX_OSPATH];
        int             played;
        int             finished;
} infosave_t;

//reload map cycles, but preserve played/finished count
void UpdateMapCycles (void)
{
        infosave_t      savedbase;
        infosave_t      *saved = &savedbase, *next;
        maplist_t       *templist;

        templist = &maplist;
        
        while (templist->next)
        {
                templist = templist->next;
                saved->next = gi.TagMalloc (sizeof(infosave_t), TAG_GAME);
                saved = saved->next;
                strncpy (saved->mapname, templist->mapname, sizeof(saved->mapname)-1);
                saved->finished = templist->finish_count;
                saved->played = templist->play_count;
        }

        ClearMaplist();
        ReadMapCycle (default_maps->string);

        templist = &maplist;
        
        while (templist->next)
        {
                templist = templist->next;
                saved = &savedbase;
                while (saved->next)
                {
                        saved = saved->next;
                        if (!Q_stricmp (saved->mapname, templist->mapname))
                        {
                                templist->finish_count = saved->finished;
                                templist->play_count = saved->played;
                                break;
                        }
                }
        }

        saved = savedbase.next;

        while (saved) {
                // save next for processing
                next = saved->next;
                gi.TagFree(saved);
                saved = next;
        }
}

void InitMapCycles (void)
{
        // reset rotation to beginning
        ClearMaplist();
        currentmapindex = 1;

        // init map cycles
        /*if (dynamic_configs->value > 0) {
                if (tiny_maps->string[0]) {
                        currentmapmode = MAPS_TINY;
                        ReadMapCycle (tiny_maps->string);
                }
                if (small_maps->string[0]) {
                        currentmapmode = MAPS_SMALL;
                        ReadMapCycle (small_maps->string);
                }
                if (medium_maps->string[0]) {
                        currentmapmode = MAPS_MEDIUM;
                        ReadMapCycle (medium_maps->string);
                }
                if (large_maps->string[0]) {
                        currentmapmode = MAPS_LARGE;
                        ReadMapCycle (large_maps->string);
                }
        }*/

        if (default_maps->string[0]) {
                currentmapmode = MAPS_DEFAULT;
                ReadMapCycle (default_maps->string);
        }
}
