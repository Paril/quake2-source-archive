#include "g_local.h"

void printMapList(edict_t *ent)
{
	FILE *file;
	char inp[32];
	memset(inp, 0, 32);

	if ((file = fopen(mapspath->string, "r")) == NULL) {
		gi.dprintf("Failed to open maplist %s!\n", mapspath->string);
		return;
	}

	while (fgets(inp, 32, file) != NULL) {
		gi.cprintf(ent, PRINT_HIGH, "%s", inp);
	}
	fclose(file);
}
char *getNextMap(const char *oldmap)
{
	FILE *file;
	static char *map = NULL;
	char inp[32];
	char first[32];

	if (!map) {
		map = gi.TagMalloc(32, TAG_GAME);
	}

	memset(map, 0, 32);
	memset(first, 0, 32);

	if ((file = fopen(mapspath->string, "r")) == NULL) {
		gi.dprintf("Failed to open maplist %s!\n", mapspath->string);
		strcpy(map, "q2dm1");
		return map;
	}

	while (fgets(inp, 32, file) != NULL) {
		inp[strlen(inp) - 1] = '\0'; //to cut off the \n
		if (first[0] == '\0') {
			strcpy(first, inp);
		}
		if (!strcmp(oldmap, inp)) {
			if (fgets(inp, 32, file) != NULL) {
				inp[strlen(inp) - 1] = '\0'; //to cut off the \n
				strcpy(map, inp);
			} else {
				strcpy(map, first);
			}
		}
	}
	fclose(file);
	if (map[0] == '\0')
		strcpy(map, first);
	return map;
}
