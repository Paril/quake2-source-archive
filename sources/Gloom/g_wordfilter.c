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

typedef struct wordfilter_s wordfilter_t;
struct wordfilter_s
{
        wordfilter_t    *next;
        char            *word;
};

wordfilter_t wordfilters;

qboolean AddWordFilter (char * word)
{
        wordfilter_t *templist;

        // don't use index 0, it's special
        unsigned int i = 1;

        templist = &wordfilters;

        while(templist->next) {
                templist = templist->next;
                if (Q_stricmp (templist->word, word) == 0)
                        return false;
                i++;
        }

        templist->next = gi.TagMalloc(sizeof(wordfilter_t), TAG_GAME);
        templist = templist->next;
        // strlen doesn't count null byte
        templist->word = gi.TagMalloc((int)strlen(word)+1, TAG_GAME);
        strncpy (templist->word, word, strlen(word)+1);

        //gi.cprintf (NULL, PRINT_HIGH, "Entry added, number of words = %d\n", i);
        return true;
}

qboolean RemoveWordFilter (char * word)
{
        qboolean found = false;
        wordfilter_t *templist;
        wordfilter_t *last;

        // start at index 1, 0 is reserved
        last = templist = &wordfilters;
        while (templist->next)
        {
                last = templist;
                templist = templist->next;
                if (!Q_stricmp (word, templist->word)) {
                        found = true;
                        break;
                }
        }

        // if list ended before we could get to index
        if (!found) {
                //gi.cprintf (NULL, PRINT_HIGH, "Entry not found.\n");
                return false;
        }

        // just copy the next over, don't care if it's null
        last->next = templist->next;

        // free!
        gi.TagFree(templist->word);
        gi.TagFree(templist);

        //gi.cprintf (NULL, PRINT_HIGH, "Entry removed.\n");
        return true;
}

qboolean MatchWordFilter (char * string)
{
        wordfilter_t *templist;

        // start at index 1, 0 is reserved
        templist = &wordfilters;
        while (templist->next)
        {
                templist = templist->next;
                if (strstr (string, templist->word)) {
                        return true;
                }
        }

        return false;
}

void ClearWordFilters(void)
{
        wordfilter_t *templist;
        wordfilter_t *next;

        // skip the first
        templist = wordfilters.next;
        wordfilters.next = NULL;

        while(templist) {
                // save next for processing
                next = templist->next;
                gi.TagFree(templist->word);
                gi.TagFree(templist);
                templist = next;
        }

        gi.cprintf (NULL, PRINT_HIGH, "WordFilters removed.\n");
}

void ListWordFilters (void)
{
        wordfilter_t *templist;

        // start at index 1, 0 is reserved
        templist = &wordfilters;
        while (templist->next)
        {
                templist = templist->next;
                gi.cprintf (NULL, PRINT_HIGH, "%s%s", templist->word, templist->next ? ", " : "");
        }
        gi.cprintf (NULL, PRINT_HIGH, "\n");
}


void ReadWordFilters (char * filename)
{
        int wordnum;
        FILE *fileptr;
        int linenum = 0;

        char *p;
        char line[64];
        size_t len;

        wordnum = 0;

        memset (&wordfilters, 0, sizeof(wordfilter_t));

        // try opening the config file
        fileptr = fopen ( filename, "rb");

        if (!fileptr) {
                gi.cprintf (NULL, PRINT_HIGH, "ReadWordFilters: Couldn't open %s\n", filename);
                return;
        }

        for (;;)
        {
                wordnum++;
                linenum++;
                line[0] = '\0';
                fgets(line, 64, fileptr);
                len = strlen(line);
                if (len) {
                        line[len-1] = '\0';

                        if (strstr(line, " ")) {
                                gi.cprintf (NULL, PRINT_HIGH, "ReadWordFilters: Bad line %d '%s'\n", linenum, line);
                                wordnum--;
                                continue;
                        }

                        if (strstr(line, "\"")) {
                                gi.cprintf (NULL, PRINT_HIGH, "ReadWordFilters: Bad line %d '%s'\n", linenum, line);
                                wordnum--;
                                continue;
                        }

                        p = line;
                        while (*p && isspace(*p))
                                p++;

                        if (*p) {
                                AddWordFilter (p);
                        } else {
                                wordnum--;
                        }
                }

                if (feof(fileptr))
                        break;
        }
        fclose (fileptr);

        gi.cprintf (NULL, PRINT_HIGH, "ReadWordFilters: Read %d words from %s\n", wordnum - 1, filename);
}

void WriteWordFilters (char * filename)
{
        FILE *fileptr;
        int i=0;
        wordfilter_t *templist;

        // try opening the file
        fileptr = fopen ( filename, "wb");
        if (!fileptr) {
                gi.cprintf (NULL, PRINT_HIGH, "  - WriteWordFilters: Couldn't open %s\n", filename);
                return;
        }

        templist = &wordfilters;

        while(templist->next) {
                templist = templist->next;
                if (!fprintf (fileptr, "%s\n", templist->word))
                        break;
                i++;
        }

        fclose (fileptr);
        gi.cprintf (NULL, PRINT_HIGH, "  + WriteWordFilters: Wrote %d words to %s\n", i, filename);
}
