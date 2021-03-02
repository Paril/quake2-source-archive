#define	TAG_TEMP	767
#define MAX_TEAMS 8

// Stays intact whole game.
struct team_s
{
	char teamname[32]; // 32 chars max for a name of the team
	int blaster_ban;
	int shotgun_ban;
	int supershotgun_ban;
	int machinegun_ban;
	int chaingun_ban;
	int grenadelauncher_ban;
	int rocketlauncher_ban;
	int hyperblaster_ban;
	int railgun_ban;
	int bfg_ban;
	int grenade_ban;
	int feature_ban;
	int item_ban;
};

typedef struct team_s team_t;

struct teamplay_s
{
	char gamename[64]; // Max of 64 chars
	team_t *teams[MAX_TEAMS]; // Eight teams max
};

typedef struct teamplay_s teamplay_t;

extern teamplay_t *teamplay_main;
