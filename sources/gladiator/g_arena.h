

//Rocket Arena 2 Bot Support Routines
// Copyright 1998, David Wright
// For non-commercial use only

typedef enum {
	NORMAL,
	FREEFLYING,
	TRACKCAM,
	EYECAM
} observer_mode_t;

typedef struct arena_settings_s {
    int needed_players_team;
	int rounds;
	int weapons;
	int armor;
	int health;
	int minping;
	int maxping;
	int rocket_speed;
	int shells, bullets, slugs, grenades, rockets, cells;
	int startdelay;
	int fastswitch;
	int armorprotect;
	int healthprotect;
	int changed;
} arena_settings_t;

void RA2_MoveToArena(edict_t *ent, int arena, qboolean observer);
void RA2_Init(edict_t *wsent);
void RA2_CheckRules(void);
int RA2_NumArenas(void);
void Cmd_toarena_f(edict_t *ent, int context);
void Cmd_start_match_f(edict_t *ent, int context);
void Cmd_stop_match_f(edict_t *ent, int context, int delay);

extern cvar_t *ra;
extern cvar_t *arena;
extern cvar_t *selfdamage;
extern cvar_t *healthprotect;
extern cvar_t *armorprotect;
extern cvar_t *ra_playercycle;
extern cvar_t *ra_botcycle;
