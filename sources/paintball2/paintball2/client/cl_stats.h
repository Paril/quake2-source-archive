// SIMPLE CLIENT SIDE STATS SYSTEM
// FIRST DRAFT 28 DECEMBER 2008 VICIOUZ

#ifndef STATS_GUARD
#define STATS_GUARD

typedef struct
{
	int kills;
	int deaths;
	int grabs;
	int caps;
	int playtime;
	float kdratio;
	float gcratio;
} stats_t;

#endif
