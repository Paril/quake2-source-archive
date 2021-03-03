#include "g_local.h"

int DL_GetMapVotes(const char *name) {
int		i, votes = 0;

	for (i = 0; i < game.maxclients; i++) {
		if (!game.clients[i].pers.connected)
			continue;
		if (!game.clients[i].prefs.mapvote)
			continue;

		if (!DL_strcmp(name, game.clients[i].prefs.mapvote, -1, false))
			votes++;
	}
	return votes;
}

const char *DL_TallyMapVotes(void) {
int		numitems, j;
int		votes, top = 0;
char	**entries, *winner = NULL;

	numitems = Ini_NumberOfEntries(dlsys.sysIni, "Vote Maps");
	entries = Ini_SectionEntries(dlsys.sysIni, "Vote Maps");
	for (j=0; j<numitems; j++) {
		votes = DL_GetMapVotes(entries[j]);
		if (votes > top) {
			top = votes;
			winner = entries[j];
		}
	}	
	if (!winner)
		winner = entries[0];

	return winner;
}
