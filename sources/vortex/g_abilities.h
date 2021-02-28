#ifndef G_ABILITIES_H
#define G_ABILITIES_H

typedef struct upgrade_s
{
	int			level;
	int			current_level;
	int			max_level;
	int			hard_max;
	int			modifier;
	float		delay;
	int			charge; // 3.5 percent ability is charged up
	int			ammo; // ability-specific ammo
	int			max_ammo; // maximum ability-specific ammo
	int			ammo_regenframe; // frame ability ammo should regenerate
	qboolean	disable;
	int     	general_skill; // vrxchile 2.7 allow mobility menu
	qboolean	hidden;
	qboolean	runed;
}upgrade_t;

typedef struct muted_s
{
	edict_t		*player;
	int			time;
}muted_t;

typedef struct skills_s
{
	muted_t		mutelist[MAX_CLIENTS];	//mute certain players

	long experience;
	long next_level;
	int administrator;
	int level;
	int speciality_points;
	int weapon_points;
	int respawn_weapon;

	unsigned int frags;
	unsigned int fragged;
	unsigned int credits;
	unsigned int weapon_respawns;

	int class_num;
	int boss;
	int streak;

	int current_health;
	int max_health;
	int current_armor;
	int max_armor;

	unsigned long shots;
	unsigned long shots_hit;

	unsigned int num_sprees;
	int max_streak;
	int suicides;
	int teleports;
	int spree_wars;
	int break_sprees;
	int break_spree_wars;
	int num_2fers;

	//ctf
	int flag_pickups;	//number of times player grabs the flag
	int flag_captures;	//number of times player caps the flag
	int flag_returns;	//number of times player returns his own flag
	int flag_kills;		//number of times player kills the flag carrier
	int offense_kills;	//number of times player kills a defender
	int defense_kills;	//number of times player defends his base by killing a player
	int assists;		//number of times player gains an assist (flag_kill or flag_return just before a flag_cap)
	//end ctf

	int playingtime;			//Playing time today (in seconds)
	int total_playtime;			//Total playing time in minutes

	int	inventory[MAX_ITEMS];
	char password[24];
	char member_since[30];
	char last_played[30];
	char player_name[24];
	char owner[24];
	char email[64];
	char title[24];

	int nerfme;
	int inuse;

	item_t			items[MAX_VRXITEMS];
	weapon_t		weapons[MAX_WEAPONS];
	upgrade_t		abilities[MAX_ABILITIES];

	talentlist_t	talents;
}skills_t;

#endif