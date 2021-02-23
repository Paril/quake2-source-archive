#include "g_local.h"

// widths of the numerals in our numeral set
int numwidths[] = { 
	21, 13, 17, 14, 17,
	16, 16, 18, 19, 17
};

void showScores (edict_t *ent) {
	char string[1024];
	char buf[100];
	int i, j;
	int index = 6;
	int digits[10];
	int offsets[10];
	int score = ent->client->score;
	
	i = 0; j = -4;
	while (score > 0) {
		digits[i] = score % 10;
		j = j - numwidths[digits[i]];
		offsets[i++] = j;
		score /= 10;
	}

	ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	Com_sprintf(string, sizeof(string), "yv 25  ");
	for ( j = 0; j < i ; j++ ) {
		Com_sprintf(buf, sizeof(buf),
			" xr %d picn num_%d ",
			offsets[j],
			digits[j] );
		Com_sprintf(string + index, sizeof(string) - index, buf);
		index += strlen(buf);
	}
	gi.WriteByte(svc_layout);
	gi.WriteString(string);
	gi.unicast(ent, true);
		
}

