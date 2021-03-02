#include "g_local.h"

/*
#define EXP_MONSTER_MULT_COOP 300
#define EXP_MONSTER_MULT 110
#define EXP_PLAYER_MULT 550
*/

#define POWERUP_CLASS_INFO {{24, 25, 30, 32, 34, 44, 65, 75, 76, 77, 79, 81,  0}, {31, 33, 59, 60, 61, 28, 23, 52, 68, 72, 78,  0,  0}, {26, 29, 48, 53, 54, 55, 62, 49,  5,  6, 50, 66,  0}, { 1,  3,  7, 27, 51, 47, 56, 57,  2,  4, 67,  0,  0}}

#define POWERUP_CLASS_COUNT 4
#define POWERUP_CLASS_SIZE 13

#define EXP_MONSTER_MULT_COOP 400
#define EXP_MONSTER_MULT 60
#define EXP_PLAYER_MULT 4000

#define EXP_GLOBAL_MULT 0.28
#define EXP_CLASS_SHARES 12
#define EXP_ITEM_SHARES 3
#define EXP_MEMORY_SHARES 2
#define EXP_MAX_CLASSLEVEL_COST 30000
#define EXP_MIN_DISTRIBUTE 100

#define PU_DROP_MAX_MULT 1.0
classinfo_t *getClassInfo(int num) {
	static classinfo_t info[] = {
	//name, baseexp, maxlevel
		{"Soldier",		2800,	0.87,	250},
		{"Technician",	2600,	0.94,	250},
		{"Cleric",		3100,	1.0,	250},
		{"Mage",		2000,	0.82,	250},
		{"Ghoul",		3200,	1.01,	250},
		{"",			3000,	1.0,	250},
		{"",			3000,	1.0,	250},
		{"",			3000,	1.0,	250}
	};
	return &(info[num]);
}

int getClassExpLeft(edict_t *ent, int classId) {
	classinfo_t *info = getClassInfo(classId);
	int level = ent->radius_dmg; //client->pers.skills.classLevel[classId];
	long int sum = 0;
	long int cost;
	long int tmpcost;
	long int mult;
	int i;
	if (ent->client->pers.skills.classLevel[classId] > 0) {
		for (i = 1; i <= ent->client->pers.skills.classLevel[classId]; i++) {
			if (i > 50) { // To avoid unnecessary calculations and stupid variable overflows
				sum += info->expreqmult * EXP_MAX_CLASSLEVEL_COST;
			} else {
				mult = (long int)pow(1.1, i);
				if (mult > info->expreqmult * EXP_MAX_CLASSLEVEL_COST)
					mult = info->expreqmult * EXP_MAX_CLASSLEVEL_COST;
				tmpcost = pow(((double) (level + 1 - i) / (double) (ent->client->pers.skills.classLevel[classId] + 1 - i)), 0.3) * info->baseexp * mult;
				if (tmpcost > info->expreqmult * EXP_MAX_CLASSLEVEL_COST)
					tmpcost = info->expreqmult * EXP_MAX_CLASSLEVEL_COST;
				sum += tmpcost;
			}
/*			if (i > 140) {
				gi.dprintf("(%d) tmpcost: %li sum: %li tmpdiv: %lf %f\n", i, tmpcost, sum, tmpdiv, (double) tmpdiv);
			}*/
		}
	}
	mult = (long int)pow(1.1, ent->client->pers.skills.classLevel[classId] + 1);
	if (mult > info->expreqmult * EXP_MAX_CLASSLEVEL_COST)
		mult = info->expreqmult * EXP_MAX_CLASSLEVEL_COST;
	cost = pow(((float) (level + 1) / (float) (ent->client->pers.skills.classLevel[classId] + 1)), 0.3) * info->baseexp * mult;
	if (cost > info->expreqmult * EXP_MAX_CLASSLEVEL_COST)
		cost = info->expreqmult * EXP_MAX_CLASSLEVEL_COST;
	if (cost < 1) // This is here to prevent stupidity when having high EXP_MULT
		cost = 1;

	//gi.dprintf("cost: %li classexp: %d sum: %li\n", cost, ent->client->pers.skills.classExp[classId], sum);
	return cost - (ent->client->pers.skills.classExp[classId] - sum);
}

powerupinfo_t *getPowerupInfo(int num) {
	static powerupinfo_t info[] = {
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	0},		// 0
{"Weapon damage",				1,	400,	40,		0,	{0,0,0,0,0,0,0,0},	{0.1, 0.15, 0.1, 0.15, 0.15, 0.0, 0.0, 0.0},	220},	// 1
{"Armor piercing",				1,	950,	20,		0,	{0,0,0,0,0,0,0,0},	{0.15, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},	380},	// 2
{"Shotgun pellets",				1,	1220,	12,		0,	{0,0,0,0,0,0,0,0},	{0.15, 0.2, 0.1, 0.2, 0.2, 0.0, 0.0, 0.0},	320},	// 3
{"Machinegun spread",			1,	1640,	7,		0,	{0,0,0,0,0,0,0,0},	{0.15, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},	360},	// 4
{"Chaingun spintime",			1,	1820,	6,		0,	{0,0,0,0,0,0,0,0},	{0.2, 0.3, 0.3, 0.3, 0.3, 0.0, 0.0, 0.0},	380},	// 5
{"Grenade launcher timer",		1,	980,	10,		0,	{0,0,0,0,0,0,0,0},	{0.15, 0.25, 0.25, 0.25, 0.25, 0.0, 0.0, 0.0},	340},	// 6
{"Projectile speed",			1,	790,	20,		0,	{0,0,0,0,0,0,0,0},	{0.1, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},	310},	// 7
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	0},		// 8
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	0},		// 9
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	0},		// 10

{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	0},		// 11
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	0},		// 12
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	0},		// 13
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	0},		// 14
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	0},		// 15
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	0},		// 16
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	0},		// 17
{"Magic penalty",				0,	1000,	10,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	400},	// 18
{"Ammo pickups",				0,	500,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	400},	// 19
{"Armor pickups",				0,	500,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	400},	// 20
{"Health pickups",				0,	500,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	400},	// 21
{"Respawn armor",				0,	500,	10,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	380},	// 22
{"Armor efficiency",			1,	970,	20,		0,	{0,0,0,0,0,0,0,0},	{0.3, 0.1, 0.2, 0.3, 0.3, 0.0, 0.0, 0.0},	380},	// 23
{"Weapon affinity",				1,	1600,	10,		0,	{0,0,0,0,0,0,0,0},	{0.1, 0.3, 0.2, 0.3, 0.3, 0.0, 0.0, 0.0},	400},	// 24
{"Spell affinity",				1,	1650,	10,		0,	{0,0,0,0,0,0,0,0},	{0.3, 0.3, 0.1, 0.1, 0.3, 0.0, 0.0, 0.0},	400},	// 25
{"Max health",					1,	300,	50,		0,	{0,0,0,0,0,0,0,0},	{0.1, 0.1, 0.1, 0.1, 0.1, 0.0, 0.0, 0.0},	195},	// 26
{"Max armor",					1,	300,	50,		0,	{0,0,0,0,0,0,0,0},	{0.1, 0.1, 0.1, 0.1, 0.1, 0.0, 0.0, 0.0},	195},	// 27
{"Max ammo",					1,	300,	50,		0,	{0,0,0,0,0,0,0,0},	{0.1, 0.1, 0.1, 0.1, 0.1, 0.0, 0.0, 0.0},	195},	// 28
{"Max magic",					1,	300,	50,		0,	{0,0,0,0,0,0,0,0},	{0.1, 0.1, 0.1, 0.1, 0.1, 0.0, 0.0, 0.0},	195},	// 29
{"Health regen",				1,	1400,	20,		0,	{0,0,0,0,0,0,0,0},	{0.2, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},	420},	// 30
{"Armor regen",					1,	1300,	20,		0,	{0,0,0,0,0,0,0,0},	{0.2, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},	420},	// 31
{"Ammo regen",					1,	1400,	20,		0,	{0,0,0,0,0,0,0,0},	{0.2, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},	380},	// 32
{"Magic regen",					1,	1200,	20,		0,	{0,0,0,0,0,0,0,0},	{0.2, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},	290},	// 33
{"Vampire",						1,	1500,	20,		0,	{0,0,0,0,0,0,0,0},	{0.3, 0.3, 0.3, 0.3, 0.15, 0.0, 0.0, 0.0},	420},	// 34
{"Pickup bonus",				0,	700,	10,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	400},	// 35
{"Bullet resistance",			0,	700,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	390},	// 36
{"Pellet resistance",			0,	700,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	390},	// 37
{"Explosion resistance",		0,	850,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	400},	// 38
{"Energy resistance",			0,	900,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	400},	// 39
{"High energy resistance",		0,	900,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	400},	// 40
{"World damage resistance",		0,	850,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	410},	// 41
{"Blood magic resistance",		0,	900,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	400},	// 42
{"Fire magic resistance",		0,	900,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	400},	// 43
{"Armor affinity",				1,	1600,	10,		0,	{0,0,0,0,0,0,0,0},	{0.3, 0.1, 0.3, 0.3, 0.3, 0.0, 0.0, 0.0},	395},	// 44
{"Resistance boost",			0,	850,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	410},	// 45
{"Damage resistance",			0,	0,		10,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},	0},		// 46

{"Firebolt",					1,	320,	40,		1,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.15, 0.1, 0.15, 0.0, 0.0, 0.0},	190},	// 47
{"Adrenaline rush",				1,	750,	40,		2,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.15, 0.1, 0.15, 0.0, 0.0, 0.0},	270},	// 48
{"Flight",						1,	2400,	3,		3,	{1,1,1,1,1,0,0,0},	{1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0},		270},	// 49
{"Lightning",					1,	460,	40,		1,	{1,1,1,1,1,0,0,0},	{0.2, 0.2, 0.15, 0.1, 0.2, 0.0, 0.0, 0.0},		310},	// 50
{"Inferno",						1,	450,	40,		1,	{1,1,1,1,1,0,0,0},	{0.2, 0.2, 0.15, 0.1, 0.2, 0.0, 0.0, 0.0},		260},	// 51
{"Fireball",					1,	550,	40,		1,	{1,1,1,1,1,0,0,0},	{0.2, 0.2, 0.15, 0.1, 0.2, 0.0, 0.0, 0.0},		360},	// 52
{"Plague bomb",					1,	420,	40,		1,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.15, 0.1, 0.15, 0.0, 0.0, 0.0},	280},	// 53
{"Corpse explosion",			1,	520,	40,		1,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.15, 0.1, 0.15, 0.0, 0.0, 0.0},	290},	// 54
{"Corpse drain",				1,	420,	40,		3,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.1, 0.1, 0.15, 0.0, 0.0, 0.0},	320},	// 55
{"Create health",				1,	390,	20,		2,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.1, 0.15, 0.15, 0.0, 0.0, 0.0},	290},	// 56
{"Create armor",				1,	390,	20,		2,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.1, 0.15, 0.15, 0.0, 0.0, 0.0},	290},	// 57
{"Silencer",					0,	950,	10,		15,	{1,1,1,1,1,0,0,0},	{0.2, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},		320},	// 58
{"Laser grid",					1,	380,	40,		4,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.15, 0.1, 0.15, 0.0, 0.0, 0.0},	290},	// 59
{"Life drain",					1,	570,	40,		11,	{1,1,1,1,1,0,0,0},	{0.2, 0.2, 0.1, 0.1, 0.1, 0.0, 0.0, 0.0},		310},	// 60
{"Corpse spores",				1,	670,	40,		1,	{1,1,1,1,1,0,0,0},	{0.2, 0.2, 0.15, 0.1, 0.2, 0.0, 0.0, 0.0},		320},	// 61
{"Gib rain",					1,	520,	40,		1,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.15, 0.1, 0.15, 0.0, 0.0, 0.0},	290},	// 62
{"Blizzard",					0,	0,		0,		0,	{1,1,1,1,1,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 63
{"Pizza",						0,	29000,	1,		1,	{1,1,1,1,1,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		900},	// 64 This one's a bit special :)
{"Sanctuary aura",				1,	650,	40,		66,	{1,1,1,1,1,0,0,0},	{2.0, 2.0, 0.4, 0.8, 2.0, 0.0, 0.0, 0.0},		380},	// 65
{"Regeneration aura",			1,	640,	40,		66,	{1,1,1,1,1,0,0,0},	{2.0, 2.0, 0.4, 0.8, 2.0, 0.0, 0.0, 0.0},		310},	// 66
{"Spark",						1,	380,	40,		1,	{1,1,1,1,1,0,0,0},	{0.2, 0.2, 0.15, 0.1, 0.2, 0.0, 0.0, 0.0},		250},	// 67
{"Bolt",						1,	520,	40,		1,	{1,1,1,1,1,0,0,0},	{0.2, 0.2, 0.15, 0.1, 0.2, 0.0, 0.0, 0.0},		320},	// 68
{"Lightning resistance",		0,	900,	20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		400},	// 69
{"Identify",					0,	1500,	3,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 70
{"Armor breaker",				0,	0,		20,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 71
{"Laser beam",					1,	420,	40,		4,	{1,1,1,1,1,0,0,0},	{0.2, 0.2, 0.15, 0.1, 0.2, 0.0, 0.0, 0.0},		320},	// 72
{"Leech",						0,	400,	40,		8,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 73
{"Rush",						0,	20,		20,		8,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 74
{"Drone",						1,	600,	40,		4,	{1,1,1,1,1,0,0,0},	{0.2, 0.2, 0.15, 0.1, 0.2, 0.0, 0.0, 0.0},		450},	// 75
{"Corpse burst",				1,	620,	40,		1,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.15, 0.1, 0.15, 0.0, 0.0, 0.0},	440},	// 76
{"Might",						1,	700,	40,		66,	{0,0,5,5,0,0,0,0},	{0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0},		480},	// 77
{"Blaze",						1,	350,	40,		1,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.15, 0.1, 0.15, 0.0, 0.0, 0.0},	190},	// 78
{"Swarm",						1,	420,	40,		1,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.15, 0.1, 0.15, 0.0, 0.0, 0.0},	280},	// 79

{"Detpipes",					0,	0,		10,		16,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 80
{"Storm",						1,	450,	40,		1,	{1,1,1,1,1,0,0,0},	{0.15, 0.15, 0.15, 0.1, 0.15, 0.0, 0.0, 0.0},	380},	// 81
{"Kill drones",					0,	0,		1,		16,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 82
{"Spawn idiot",					0,	0,		10,		1,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 83
{"Anti resist",					0,	700,	40,		0,	{0,0,5,5,0,0,0,0},	{0.5, 0.5, 0.5, 0.5, 0.5, 0.0, 0.0, 0.0},		480},	// 84
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 85
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 86
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 87
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 88
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 89
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 90
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 91
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 92
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 93
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 94
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 95
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 96
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 97
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 98
{"Not used",					0,	0,		0,		0,	{0,0,0,0,0,0,0,0},	{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},		0},		// 99
};

	return &(info[num]);
}

int getRandomPowerupByClass(int cl) {
	int classinfo[POWERUP_CLASS_COUNT][POWERUP_CLASS_SIZE] = POWERUP_CLASS_INFO;
	/*
	{
		{25, 30, 31, 32, 34, 35, 60, 58, 50, 44,  0,  0,  0,  0,  0}, // Very rare
		{ 2,  6, 10, 12, 33, 53, 55, 61, 59, 38, 40, 19, 42, 24, 22}, // Rare
		{ 4,  8, 14, 16, 54, 56, 57, 36, 37, 39, 41, 51, 23, 62, 43}, // Ordinary
		{ 1,  3,  5,  7,  9, 11, 13, 15, 17, 26, 27, 28, 29, 52, 18} // Common
	};
	*/
	int i, j, num[POWERUP_CLASS_COUNT];

	if ((cl < 1) || (cl > POWERUP_CLASS_COUNT))
		return 0;

// Count the number of upgrades per class
	for (i = 0; i < POWERUP_CLASS_COUNT; i++) {
		j = 0;
		while(classinfo[i][j] && j < POWERUP_CLASS_SIZE)
			j++;
		num[i] = j;
	}

	return classinfo[cl-1][(int) (random() * num[cl-1])];
}

int getPowerupClass(int num) {
	int classinfo[POWERUP_CLASS_COUNT][POWERUP_CLASS_SIZE] = POWERUP_CLASS_INFO;
	int i, j;

	for (i = 0; i < POWERUP_CLASS_COUNT; i++) {
		for (j = 0; j < POWERUP_CLASS_SIZE; j++) {
			if (classinfo[i][j] == num) {
				return i + 1;
			}
		}
	}
	return -1;
}

int getPluginCount(edict_t *ent) {
	int i;
	for (i = 0; i < GIEX_PUPERCHAR; i++) {
		if (ent->client->pers.skills.putype[i] == 0) {
			return i + 1;
		}
	}
	return i + 1;
}

void restructurePowerups(edict_t *ent) {
	int start, i;
	for (start = 0; start < GIEX_PUCARRYPERCHAR; start++) {
		if (ent->client->pers.skills.powerup_type[start] == 0) {
			break;
		}
	}
	if (start > GIEX_PUCARRYPERCHAR - 1) {
		return;
	}
	for (i = start + 1; i < GIEX_PUCARRYPERCHAR; i++) {
		ent->client->pers.skills.powerup_type[i - 1] = ent->client->pers.skills.powerup_type[i];
		ent->client->pers.skills.powerup_level[i - 1] = ent->client->pers.skills.powerup_level[i];
	}
	ent->client->pers.skills.powerup_type[GIEX_PUCARRYPERCHAR - 1] = 0;
	ent->client->pers.skills.powerup_level[GIEX_PUCARRYPERCHAR - 1] = 0;
}
void restructurePlugins(edict_t *ent) {
	int start, i;
	for (start = 0; start < GIEX_PUPERCHAR; start++) {
		if (ent->client->pers.skills.putype[start] == 0) {
			break;
		}
	}
	if (start > GIEX_PUPERCHAR - 1) {
		return;
	}
	for (i = start + 1; i < GIEX_PUPERCHAR; i++) {
		ent->client->pers.skills.putype[i - 1] = ent->client->pers.skills.putype[i];
		ent->client->pers.skills.pucurlvl[i - 1] = ent->client->pers.skills.pucurlvl[i];
		ent->client->pers.skills.pumaxlvl[i - 1] = ent->client->pers.skills.pumaxlvl[i];
		ent->client->pers.skills.puexp[i - 1] = ent->client->pers.skills.puexp[i];
	}
	ent->client->pers.skills.putype[GIEX_PUPERCHAR - 1] = 0;
	ent->client->pers.skills.pucurlvl[GIEX_PUPERCHAR - 1] = 0;
	ent->client->pers.skills.pumaxlvl[GIEX_PUPERCHAR - 1] = 0;
	ent->client->pers.skills.puexp[GIEX_PUPERCHAR - 1] = 0;
}

int getPowerupCost(edict_t *ent, int slot) {
	int cost = 0, tmpcost, i, maxlevel;
	powerupinfo_t *info;

	if (ent->client->pers.skills.putype[slot] == 0) {
		return 0;
	}
	info = getPowerupInfo(ent->client->pers.skills.putype[slot]);

	maxlevel = info->maxlevel;
	if (maxlevel > ent->client->pers.skills.pumaxlvl[slot]) {
		maxlevel = ent->client->pers.skills.pumaxlvl[slot];
	}
	if (ent->client->pers.skills.pucurlvl[slot] >= maxlevel) {
		return 0;
	}

	for (i = 0; i < ent->client->pers.skills.pucurlvl[slot] + 1; i++) {
		tmpcost = info->basecost * pow(1.08, i);
		if (tmpcost > 15000)
			tmpcost = 15000;
		cost += tmpcost;
	}
	cost -= ent->client->pers.skills.puexp[slot];
	if (cost < 1) // This is here to prevent stupidity when having high EXP_MULT
		cost = 1;
	return cost;
}

int getItemPowerupCost(edict_t *ent, int item, int slot) {
	long cost = 0, tmpcost, maxlevel;
	int i, s = -1;
	powerupinfo_t *puinfo;
	iteminfo_t *iteminfo;

	iteminfo = getItemInfo(ent->client->pers.skills.itemId[item]);
	puinfo = getPowerupInfo(ent->client->pers.skills.item_putype[item][slot]);

	if ((iteminfo->name[0] == '\0') || (!puinfo->inuse)) {
		return 0;
	}
	//gi.dprintf("%s %s\n", iteminfo->name, puinfo->name);
	for (i = 0; i < GIEX_PUPERITEM; i++) {
		if (iteminfo->putype[i] == ent->client->pers.skills.item_putype[item][slot]) {
			s = i;
			break;
		}
	}
	if (s == -1) {
//TODO: Something has b0rked if we get here.
		gi.dprintf("%s's %s has an incompatible %s powerup plugged in!\n", ent->client->pers.netname, iteminfo->name, puinfo->name);
		return 0;
	}

	maxlevel = puinfo->maxlevel + iteminfo->pumaxlvl[s];
	if (maxlevel > ent->client->pers.skills.item_pumaxlvl[item][slot]) {
		maxlevel = ent->client->pers.skills.item_pumaxlvl[item][slot];
	}
	if (ent->client->pers.skills.item_pucurlvl[item][slot] >= maxlevel) {
		return 0;
	}

//Calculate cost for every level gained so far...
	for (i = 0; i < ent->client->pers.skills.item_pucurlvl[item][slot] + 1; i++) {
		tmpcost = puinfo->basecost * pow(1.06, i) * iteminfo->pucost[s];
		if (tmpcost > 15000)
			tmpcost = 15000;
		cost += tmpcost;
	}
//...and subtract total amount of exp gained so far on this powerup.
	cost -= ent->client->pers.skills.item_puexp[item][slot];

	if (cost < 1) // This is here to prevent stupidity when having high EXP_MULT
		cost = 1;
	return cost;
}

//returns -2 for no item/memory, -1 for memory, else item slot
int getWornItemPowerupComp(edict_t *ent, int putype) {
	int i, pu;
	powerupinfo_t *puinfo = getPowerupInfo(putype);
	skills_t *skills;
	skills = &ent->client->pers.skills;

//Scan through "memory"
	for (pu = 0; pu < GIEX_PUPERCHAR; pu++) {
		if ((skills->putype[pu] == putype) && (skills->pumaxlvl[pu] < puinfo->maxlevel)) {
			return -1;
		}
	}
//Scan through worn items
	for (i = 0; i < GIEX_BASEITEMS; i++) {
		iteminfo_t *iteminfo = getWornItemInfo(ent, i);
		//iteminfo_t *iteminfo = getItemInfo(skills->itemId[(int) skills->wornItem[i]]);
		int puinfoslot = getItemPowerupInfoSlot(iteminfo, putype);
		if (puinfoslot >= 0) {
			for (pu = 0; pu < GIEX_PUPERITEM; pu++) {
				if ((skills->item_putype[(int) skills->wornItem[i]][pu] == putype) && (skills->item_pumaxlvl[(int) skills->wornItem[i]][pu] < puinfo->maxlevel + iteminfo->pumaxlvl[puinfoslot])) {
					return skills->wornItem[i];
				}
			}
		}
	}

//Scan through whole inventory
	for (i = 0; i < GIEX_ITEMPERCHAR; i++) {
		iteminfo_t *iteminfo = getItemInfo(skills->itemId[i]);
		int puinfoslot = getItemPowerupInfoSlot(iteminfo, putype);
		if (puinfoslot >= 0) {
			for (pu = 0; pu < GIEX_PUPERITEM; pu++) {
				if ((skills->item_putype[i][pu] == putype) && (skills->item_pumaxlvl[i][pu] < puinfo->maxlevel + iteminfo->pumaxlvl[puinfoslot])) {
					return i;
				}
			}
		}
	}
	return -2;
}

void powerup_make_touchable (edict_t *powerup);
void powerup_make_touchable2 (edict_t *powerup);
void powerup_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
	//r1: no need to pollute the stack since this will return more often than run
	powerupinfo_t *info;

	if (!other || !other->client)
		return;

	if (other->client->id_powerup > level.time) {
		powerupinfo_t *info;
		info = getPowerupInfo(ent->radius_dmg);
		gi.cprintf(other, PRINT_HIGH, "A level %d %s powerup (your current total level: %d)\n", (int) ent->dmg_radius, info->name, other->client->pers.skill[ent->radius_dmg]);

		// So powerups won't be id'd several times at once
		ent->touch = NULL;
		if (ent->owner && (ent->owner != other)) {
			ent->think = powerup_make_touchable;
			ent->nextthink = level.time + 0.2;
		} else {
			ent->think = powerup_make_touchable2;
			ent->nextthink = level.time + 0.2;
		}
		return;
	}

	if (ent->owner && (ent->owner != other))
		return;

	if (other->client->get_powerup < level.time && !other->client->pers.skills.autopickup)
		return;

	{
		int itemId = -3;
		int i = 0;

		info = getPowerupInfo(ent->radius_dmg);
		while (i < GIEX_PUCARRYPERCHAR) {
			if (other->client->pers.skills.powerup_type[i] == 0)
				break;
			i++;
		}

		if (i == GIEX_PUCARRYPERCHAR) {
			if (ent->pain_debounce_time < level.time) {
				gi.cprintf(other, PRINT_HIGH, "No free slots for the level %d %s powerup\n", (int) ent->dmg_radius, info->name);
				ent->pain_debounce_time = level.time + 1;
			}
			return;
		}

		//R1CH: autopickup
		if (other->client->pers.skills.autopickup) {
			itemId = getWornItemPowerupComp(other, ent->radius_dmg);
		}
		if ((itemId < -1) && (other->client->get_powerup < level.time)) {
			return;
		}

		other->client->pers.skills.powerup_type[i] = ent->radius_dmg;
		other->client->pers.skills.powerup_level[i] = ent->dmg_radius;
		if (other->client->pers.skills.autoplugin) {
			if (itemId == -3) {
				itemId = getWornItemPowerupComp(other, ent->radius_dmg);
			}
			if (itemId > -2) {
				//gi.dprintf("should put in: %d\n", getWornItemPowerupComp(other, ent->radius_dmg));
				pluginPowerup(other, i, itemId);
				gi.sound(ent, CHAN_VOICE, gi.soundindex("giex/puppkup.wav"), 1, ATTN_NORM, 0);
				G_FreeEdict(ent);
				saveCharacter(other);
				return;
			}
		}
		gi.cprintf(other, PRINT_HIGH, "You placed the level %d %s powerup in slot %d\n", (int) ent->dmg_radius, info->name, i);
		gi.sound(ent, CHAN_VOICE, gi.soundindex("giex/puppkup.wav"), 1, ATTN_NORM, 0);
		G_FreeEdict(ent);
		saveCharacter(other);
	}
}

void powerup_make_touchable2 (edict_t *powerup) {
	powerup->s.effects &= ~EF_COLOR_SHELL;
	powerup->s.renderfx &= ~(RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE);
	powerup->owner = NULL;
	powerup->touch = powerup_touch;
	powerup->think = G_FreeEdict;
	powerup->nextthink = level.time + 45;
}

void powerup_make_touchable (edict_t *powerup) {
	int cl = 5 - getPowerupClass(powerup->radius_dmg);
	powerupinfo_t *info = getPowerupInfo(powerup->radius_dmg);
	float count = (float)powerup->dmg_radius/(float)(info->maxlevel * 2);

	powerup->s.effects |= EF_COLOR_SHELL;
	powerup->s.renderfx |= (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE);
	powerup->touch = powerup_touch;
	powerup->think = powerup_make_touchable2;
	powerup->nextthink = level.time + 8.0 + 4.0 * (count + 0.5) * (float)(cl * cl);
}

qboolean getPowerupLevelable(edict_t *ent, int putype, int lvl) {
	powerupinfo_t *info = getPowerupInfo(putype);
	int i;

	for (i = 0; i < GIEX_NUMCLASSES; i++) {
		if (info->classreq[i] == 0)
			continue;
		if (ent->client->pers.skills.classLevel[i] >= info->classreq[i] + info->reqperlvl[i] * lvl)
			return true;
	}
	return false;
}
qboolean getItemPowerupLevelable(edict_t *ent, int item, int pu) {
	int i;
	iteminfo_t *iteminfo;
	powerupinfo_t *puinfo;
	skills_t *skills = &ent->client->pers.skills;
	iteminfo = getItemInfo(skills->itemId[item]);
	puinfo = getPowerupInfo(skills->item_putype[item][pu]);

	for (i = 0; i < GIEX_NUMCLASSES; i++) {
//		if (puinfo->classreq[i] == 0)
//			continue;
		if (getItemBaseLevelReq(ent, item, i) == -1)
			continue;

//		gi.dprintf("%s %s %d: %d %d\n", iteminfo->name, puinfo->name, (skills->item_pucurlvl[item][pu] + 1), i, (int) (iteminfo->classreq[i] + puinfo->classreq[i] + (int) ceil(puinfo->reqperlvl[i] * (skills->item_pucurlvl[item][pu] + 1))));
		if (skills->classLevel[i] > iteminfo->classreq[i] + ceil(puinfo->reqperlvl[i] * (skills->item_pucurlvl[item][pu] + 1)))
			return true;
	}

	return false;
}

void spawnPowerup(edict_t *targ, edict_t *attacker) {
	int type = 0, amount = 0;
	powerupinfo_t *info;
	edict_t *powerup;
	float mult;

	if (game.monsterhunt != 10) { // In monster hunt, always drop powerup, even if killed by world or self.
		if (!attacker->client)
			return;
		if (targ == attacker)
			return;
	} else { // Never spawn powerups for player kills in Monster hunt
		if (targ->client)
			return;
	}
	if (targ->svflags & SVF_MONSTER) {
		float r;

		if (deathmatch->value) {
			if (random() > targ->monsterinfo.cts_any) {
				return;
			}
		} else { //coop
			if (random() > (targ->monsterinfo.cts_any * 1.5)) {
				return;
			}
		}
		if ((deathmatch->value) && (game.craze == 10) && (random() < 0.01) && (targ->classid == CI_M_MAKRON) && (targ->radius_dmg)) {
			type = 64;
		} else {
			r = random();
			if (r < (targ->monsterinfo.cts_class1))
				type = getRandomPowerupByClass(1);
			else if (r < (targ->monsterinfo.cts_class2))
				type = getRandomPowerupByClass(2);
			else if (r < (targ->monsterinfo.cts_class3))
				type = getRandomPowerupByClass(3);
			else
				type = getRandomPowerupByClass(4);
		}

		info = getPowerupInfo(type);
		if (!info) {
			gi.dprintf("getPowerupInfo() returned null info! type: %d\n", type);
			return;
		}
		mult = PU_DROP_MAX_MULT * (int)(random() * ((float) (targ->monsterinfo.level + 200) / 120)) / (float)(info->bonus_level + 1);
		if (mult < (-0.3 + targ->monsterinfo.skill / 50.0))
			mult = (-0.3 + targ->monsterinfo.skill / 50.0);
		if (mult > PU_DROP_MAX_MULT)
			mult = PU_DROP_MAX_MULT;
		amount = mult * info->maxlevel;
		if (amount < 1)
			amount = 1;

		if (game.monsterhunt != 10) {
			if (type == 64) { // Pizza!
				gi.bprintf(PRINT_HIGH, "OMG!! %s just got a PIZZA powerup!! ORFL ORFL!!!\n", attacker->client->pers.netname);
			} else {
				gi.cprintf(attacker, PRINT_HIGH, "%s dropped a level %d %s powerup!\n", targ->monsterinfo.name, amount, info->name);
			}
		}
	} else if (deathmatch->value && targ->client && attacker->client) {
		if ((!teams->value) || (targ->count != attacker->count)) { // targ->count is it's team, ->radius_dmg number of skill points
			float r;

			if (random() > (0.35 + (targ->radius_dmg / 150.0)))
				return;
			r = random() + targ->radius_dmg * 0.005;
			if (r < 0.10)
				type = getRandomPowerupByClass(1);
			else if (r < 0.25)
				type = getRandomPowerupByClass(2);
			else if (r < 0.55)
				type = getRandomPowerupByClass(3);
			else
				type = getRandomPowerupByClass(4);

			info = getPowerupInfo(type);
			if (!info) {
				gi.dprintf("getPowerupInfo() returned null info! type: %d\n", type);
				return;
			}
			mult = PU_DROP_MAX_MULT * (int)(random() * ((targ->radius_dmg + 5) * 4)) / (float)(info->bonus_level + 1);
			if (mult < (-0.3 + targ->radius_dmg / 50.0))
				mult = (-0.3 + targ->radius_dmg / 50.0);
			if (mult > PU_DROP_MAX_MULT)
				mult = PU_DROP_MAX_MULT;
			amount = mult * info->maxlevel;
			if (amount < 1)
				amount = 1;

			gi.cprintf(attacker, PRINT_HIGH, "%s dropped a level %d %s powerup!\n", targ->client->pers.netname, amount, info->name);
		} else {
			return;
		}
	} else {
		return;
	}

	powerup = G_Spawn();
	powerup->classname = "powerup";
	VectorCopy (targ->s.origin, powerup->s.origin);
	VectorCopy (targ->s.angles, powerup->s.angles);
	VectorSet (powerup->mins, -15, -15, -15);
	VectorSet (powerup->maxs, 15, 15, 15);
	powerup->s.effects |= EF_ROTATE;
	powerup->velocity[0] = 75.0 * crandom();
	powerup->velocity[1] = 75.0 * crandom();
	powerup->velocity[2] = 175.0 + 175.0 * random();
	if (game.monsterhunt == 10) { // Spread out extra in Monster hunt
		powerup->velocity[0] *= 2;
		powerup->velocity[1] *= 2;
		powerup->velocity[2] *= 2;
	}
	powerup->movetype = MOVETYPE_TOSS;
	gi.setmodel (powerup, "models/items/keys/pyramid/tris.md2");
	powerup->nextthink = level.time + 1;
	powerup->solid = SOLID_TRIGGER;
	powerup->radius_dmg = type;
	powerup->dmg_radius = amount;
	if (game.monsterhunt != 10) { // In monster hunt, noone is counted as the killer, FFA powerups! :)
		powerup->owner = attacker;
		powerup->think = powerup_make_touchable;
	} else {
		powerup->owner = NULL;
		powerup->think = powerup_make_touchable2;
	}
	gi.linkentity (powerup);
}

void spawnAmmo(edict_t *targ, edict_t *attacker) {
	gitem_t *item;
	edict_t *spawn;
	int amount;
	float r;

//Extra ammo only spawns in coop.
	if (!coop->value)
		return;
	if (targ == attacker)
		return;
//Only monsters spawn ammo on death
	if (!(targ->svflags & SVF_MONSTER))
		return;
	if (random() > 0.75)
		return;

	r = random();

	if (r < 0.17) {
		item = FindItem("Bullets");
		amount = 25;
	} else if (r < 0.34) {
		item = FindItem("Shells");
		amount = 25;
	} else if (r < 0.51) {
		item = FindItem("Cells");
		amount = 30;
	} else if (r < 0.68) {
		item = FindItem("Grenades");
		amount = 3;
	} else if (r < 0.84) {
		item = FindItem("Rockets");
		amount = 3;
	} else {
		item = FindItem("Slugs");
		amount = 3;
	}
	if (coop->value) {
		amount *= 2;
	}
	if (targ->client) {
		amount *= (0.9 + targ->radius_dmg * 0.05);
	} else {
		amount *= (0.9 + targ->monsterinfo.skill * 0.05);
	}
	spawn = Drop_Item(targ, item);
	spawn->count = amount;

	return;
}

void ApplyMax(edict_t *self) {
	int index = ITEM_INDEX(FindItem("Body Armor"));
	iteminfo_t *armor = getWornItemInfo(self, GIEX_ITEM_ARMOR);
	iteminfo_t *helmet = getWornItemInfo(self, GIEX_ITEM_HELMET);
	iteminfo_t *backpack = getWornItemInfo(self, GIEX_ITEM_BACKPACK);
	if (!self->client)
		return;

// Remember to change health formula for Vamp in T_Damage if you change max health modifier for Vampire class
	self->max_health = 100 + 15 * pow(self->client->pers.skills.classLevel[0], 1.05) + 13 * pow(self->client->pers.skills.classLevel[1], 1.05) + 26 * pow(self->client->pers.skills.classLevel[2], 1.05) + 12 * pow(self->client->pers.skills.classLevel[3], 1.05) + 17 * pow(self->client->pers.skills.classLevel[4], 1.05) + 20 * self->client->pers.skill[26];
	self->gib_health = -(100 + self->radius_dmg * 15); //Corpse health is dependant on player level only.

	self->client->max_armor = 0;
	if (armor->name[0] != '\0') {
		self->client->max_armor += armor->arg1 + armor->arg2 * getWornItemPowerupLevel(self, GIEX_ITEM_ARMOR, 27); //Wearing armor
	}
	if (helmet->name[0] != '\0') {
		self->client->max_armor += helmet->arg1 + helmet->arg2 * getWornItemPowerupLevel(self, GIEX_ITEM_HELMET, 27); //Wearing helmet
	}
	if (backpack->name[0] != '\0') {
		self->client->max_armor += backpack->arg1 + backpack->arg2 * getWornItemPowerupLevel(self, GIEX_ITEM_BACKPACK, 27); //Wearing backpack
	}
	self->client->max_armor += 12 * pow(self->client->pers.skills.classLevel[1], 1.05); // Tech levels give max armor

	self->client->max_magic = 100 + 2 * pow(self->client->pers.skills.classLevel[0], 1.1) + 3 * pow(self->client->pers.skills.classLevel[1], 1.1) + 8 * pow(self->client->pers.skills.classLevel[2], 1.1) + 11 * pow(self->client->pers.skills.classLevel[3], 1.1) + 8 * pow(self->client->pers.skills.classLevel[4], 1.1) + 10 * self->client->pers.skill[29];
	self->client->max_magic *= 1 - 0.05 * self->client->pers.skill[18];

	if (self->health > self->max_health)
		self->health = self->max_health;
	if (self->client->pers.inventory[index] > self->client->max_armor)
		self->client->pers.inventory[index] = self->client->max_armor;
	if (self->client->magic > self->client->max_magic)
		self->client->magic = self->client->max_magic;

	checkAmmoAmounts(self);
}

void deductExp(edict_t *attacker, edict_t *target) {
	int amt = 0;
	float mult;
	skills_t *skills;

	if (!target) {
		return;
	}
	if (target->health > 0) { // only deduct exp for death
		return;
	}
	if (target->radius_dmg < 20) {
		return;
	}

	skills = &target->client->pers.skills;

	if (!attacker) {
		amt = target->radius_dmg;
	} else if (attacker == target) {
		amt = target->radius_dmg * 4;
	} else if (attacker->client) {
		//Reset attacker's playerdamage
		attacker->client->playerdamage = 0;
		if (attacker->radius_dmg < target->radius_dmg) {
			amt = ((target->radius_dmg + 5) / (attacker->radius_dmg + 5)) * 50;
		} else { // Killed by a higher level player, no penalty
			return;
		}
	} else if (attacker->svflags & SVF_MONSTER) {
		int kills = skills->stats[GIEX_STAT_LOW_MONSTER_KILLS] + skills->stats[GIEX_STAT_MED_MONSTER_KILLS] + skills->stats[GIEX_STAT_HI_MONSTER_KILLS] + skills->stats[GIEX_STAT_VHI_MONSTER_KILLS];
		int deaths = skills->stats[GIEX_STAT_MONSTER_DEATHS];
		if (game.monsterhunt == 10)
			return;
		mult = ((float) sqrt(kills)/(float) sqrt(deaths));
		if (mult > 2)
			mult = 2;
		amt = target->radius_dmg * (2.0 + mult);
		if (game.craze == 10)
			amt *= 0.1;
	} else {
		//func_explosive, target_blasters and so on (i think?)
		amt = target->radius_dmg;
	}
//	amt /= 6.0 / (float) (3 + (int) (target->radius_dmg / 12.0));
	target->client->pers.add_exp -= amt * EXP_GLOBAL_MULT;
	if (target->client->pers.add_exp > -1) {
		return;
	}

	skills->classExp[skills->activeClass] += (int) target->client->pers.add_exp;
	target->client->pers.expRemain -= (int) target->client->pers.add_exp;
	target->client->pers.add_exp -= (int) target->client->pers.add_exp;
}

void giveExpToAll(double amount) {
	int i;
	edict_t *scan;
	for (i=0 ; i<maxclients->value ; i++) {
		scan = g_edicts + 1 + i;
		if (!scan->inuse)
			continue;
		if (!scan->client)
			continue;
		if (!scan->client->pers.loggedin)
			continue;

		scan->client->pers.add_exp += amount;
	}
}

void addExp(edict_t *self, edict_t *target, int damage) {
	float amt = 0;
	int points = 0;
	int shares = 0, pershare = 0;
	int i, p;
	skills_t *skills;
	classinfo_t *clInfo;

	if ((!self) || (!self->client) || (!target))
		return;
	if (self == target)
		return;
//	if ((coop->value) && (!(target->svflags & SVF_MONSTER)))
//		return;
//	if (damage < 1)
//		return;

	skills = &self->client->pers.skills;

	if (damage > 0) {
		if (target->health < 1) { // check so we don't give exp for negative health
			points = target->health + damage;
			if (points < 1) {
				return;
			}
		} else {
			points = damage;
		}
	} else {
		if (!target->client)
			return;
		if (self == target)
			return;

		if (target->health > target->max_health) { // check so we don't give exp for negative health
			points = damage - (target->health - target->max_health);
			if (points < 1) {
				return;
			}
		} else {
			points = damage;
		}
		if (coop->value) {
			points *= 8;
		}
	}

	if (target->svflags & SVF_MONSTER) {
		if (damage < 1)
			return;
		if (coop->value) {
			amt = ((float) points/(float) target->max_health) * EXP_MONSTER_MULT_COOP * ((float) (target->monsterinfo.level + 20) / (float) (self->radius_dmg + 20));
		} else {
			amt = ((float) points/(float) target->max_health) * EXP_MONSTER_MULT * ((float) (target->monsterinfo.level + 20) / (float) (self->radius_dmg + 20));
		}

		//Don't give as much exp if player is kamikaze'ing high-level monsters
		if (self->radius_dmg / GIEX_MONSTER_PLAYERLEVEL_MULT < target->monsterinfo.skill * 0.75) {
			amt *= 0.25;
		}
	} else if (target->client) {
		if (damage < 0) { //self has healed target, give exp to Cleric class if target is on same team
			if ((coop->value) || ((teams->value) && (self->count == target->count))) {
				amt = -((float) points/(float) target->max_health) * target->radius_dmg * EXP_PLAYER_MULT * 0.07;
				amt *= 1.0 / (float) (1.6 + (int) (self->radius_dmg / 12.0));
				skills->classExp[2] += (int) ceil(amt * EXP_GLOBAL_MULT);
				if (skills->activeClass == 2) {
					self->client->pers.expRemain -= (int) ceil(amt * EXP_GLOBAL_MULT);
				}
			}
			return;

		} else {
			if (target->client->playerdamage > target->max_health) {
				char msg[1000];
				sprintf(msg, "%s has taken max_health damage from %s without returning fire\n", target->client->pers.netname, self->client->pers.netname);
				logmsg(msg);
				return;
			}
			if (target->client->playerdamage > ceil(target->max_health * 0.25)) {
				return;
			}

			// target->count is it's team, ->radius_dmg = level
			amt = ((float) points/(float) target->max_health) * EXP_PLAYER_MULT * ((float) (1 + pow(target->radius_dmg, 1.5)) / (float) (1 + pow(self->radius_dmg, 1.25)));
			if ((coop->value) || ((teams->value) && (self->count == target->count))) {
				//Hurting a player on same team, give exp penalty!
				self->client->pers.add_exp -= 0.25 * amt * EXP_GLOBAL_MULT;
				gi.cprintf(self, PRINT_HIGH, "You hurt a team mate, lost %d exp\n", (int) ceil(0.25 * amt * EXP_GLOBAL_MULT));
				if (self->client->pers.add_exp < 1) {
					skills->classExp[skills->activeClass] -= (int) self->client->pers.add_exp;
					self->client->pers.expRemain -= (int) self->client->pers.add_exp;
					self->client->pers.add_exp -= (int) self->client->pers.add_exp;
				}
				return;
			}
			self->client->playerdamage -= ceil(damage * 3);
			target->client->playerdamage += damage;
			if (self->client->playerdamage < 0) {
				self->client->playerdamage = 0;
			}
			if (target->client->playerdamage > target->max_health) {
				target->client->playerdamage = target->max_health;
			}

			// Cut down on exp given for fighting higher level or lower level players
			if (self->radius_dmg < target->radius_dmg * 0.15) {
				amt *= 0.25;
			} else if (self->radius_dmg < target->radius_dmg * 0.3) {
				amt *= 0.5;
			} else if (self->radius_dmg > target->radius_dmg * 4) {
				amt *= 0.5;
			}
		}
	} else {
		//Shouldn't be able to get here
		return;
	}

	self->client->pers.add_exp += amt * EXP_GLOBAL_MULT;

	// Add exp to team mates in team games...
	if ((deathmatch->value) && (teams->value) && (teams->value != 3) && (self->count)) {
		int i;
		float mult;
		edict_t *scan;

		if (teams->value == 1) {
			mult = 0.15;
		} else if (teams->value == 2) {
			if (self->count == 1)
				mult = level.red_team_bonus;
			else
				mult = level.green_team_bonus;
		} else {
			//prevent future idiocy
			mult = 0.1;
		}
//		gi.dprintf("Exp to team %d, teams: %f, mult: %f, add: %f\n", self->count, teams->value, mult, mult * amt * EXP_GLOBAL_MULT);
		for (i=0 ; i<maxclients->value ; i++) {
			scan = g_edicts + 1 + i;
			if (!scan->inuse)
				continue;
			if (scan->count != self->count)
				continue;
			if (scan == self)
				continue;
			if (scan->health < 1)
				continue;
			if (scan->client->pers.idletime > 100) //been standing still for more than 10 seconds, probably idling...
				continue;
			scan->client->pers.add_exp += mult * amt * EXP_GLOBAL_MULT;
		}
	}

	if (self->client->pers.add_exp < EXP_MIN_DISTRIBUTE)
		return;

	clInfo = getClassInfo(skills->activeClass);
	shares = 0;
	if (self->client->pers.skills.classLevel[skills->activeClass] < clInfo->maxlevel) {
		shares += EXP_CLASS_SHARES; // Shares to the class exp
	} else {
		gi.cprintf(self, PRINT_HIGH, "You cannot gain more than %d %s levels\n", clInfo->maxlevel, clInfo->name);
	}
	for (p = 0; p < GIEX_PUPERCHAR; p++) {
		if (skills->putype[p] == 0) {
			break;
		}
		if ((skills->pucurlvl[p] < skills->pumaxlvl[p]) && (getPowerupLevelable(self, skills->putype[p], skills->pucurlvl[p] + 1))) {
			shares += EXP_MEMORY_SHARES; // 1 share for each plugged in powerup
		}
	}
//	gi.dprintf("shares mem: %d\n", shares);
	for (i = 0; i < GIEX_BASEITEMS; i++) {
		if ((skills->wornItem[i] > -1) && (skills->itemId[(int) skills->wornItem[i]] > 0)) {
			for (p = 0; p < GIEX_PUPERITEM; p++) {
				if (skills->item_putype[(int) skills->wornItem[i]][p] == 0) {
					break;
				}
				if (skills->item_pucurlvl[(int) skills->wornItem[i]][p] >= skills->item_pumaxlvl[(int) skills->wornItem[i]][p]) {
					continue;
				}
				if (!getItemPowerupLevelable(self, skills->wornItem[i], p)) {
					continue;
				}
				shares += EXP_ITEM_SHARES; // 1 share for each powerup in each worn item
			}
		}
//		gi.dprintf("shares item %d: %d\n", i, shares);
	}

	pershare = (int) (self->client->pers.add_exp / shares);
//	gi.dprintf("shares: %d, Pershare: %d, amt: %f, add_exp: %f", shares, pershare, amt, self->client->pers.add_exp);
	self->client->pers.add_exp -= pershare * shares;
//	gi.dprintf(" add_exp after: %f\n", self->client->pers.add_exp);

	if (self->client->pers.skills.classLevel[skills->activeClass] < clInfo->maxlevel) {
		amt = EXP_CLASS_SHARES * pershare;
		if (self->radius_dmg < 50) {
			amt *= 1 + ((50 - self->radius_dmg) * 0.04);
		}
		if (self->radius_dmg > 50) {
			amt *= 1.0 / (float) (1.0 + (int) ((self->radius_dmg - 50) / 20.0));
		}
		amt = (int) (amt + 1.0);
		self->client->pers.skills.classExp[skills->activeClass] += amt;
		self->client->pers.expRemain -= amt;
	}

	if (self->client->pers.expRemain < 1) {
		//WOOHOOOO!! We gained a level!
		self->client->pers.skills.classLevel[skills->activeClass]++;
		self->radius_dmg++;
		self->client->pers.expRemain = getClassExpLeft(self, skills->activeClass);
		saveCharacter(self);
		gi.cprintf(self, PRINT_HIGH, "You gained %s level %d!\n", clInfo->name, self->client->pers.skills.classLevel[skills->activeClass]);
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/pupmerge.wav"), 1, ATTN_NORM, 0);
		ApplyMax(self);
	}

//	gi.dprintf("%d\n", getItemPowerupCost(self, self->client->pers.skills.wornItem[0], 0));
	for (p = 0; p < GIEX_PUPERCHAR; p++) {
		if (skills->putype[p] == 0) {
			break;
		}
		if ((skills->pucurlvl[p] < skills->pumaxlvl[p]) && (getPowerupLevelable(self, skills->putype[p], skills->pucurlvl[p] + 1))) {
			skills->puexp[p] += pershare * EXP_MEMORY_SHARES;
			self->client->pers.puexpRemain[p] -= pershare * EXP_MEMORY_SHARES;
			if (self->client->pers.puexpRemain[p] < 1) {
				powerupinfo_t *info = getPowerupInfo(skills->putype[p]);
				skills->pucurlvl[p]++;
				self->client->pers.skill[skills->putype[p]]++;
				self->client->pers.puexpRemain[p] = getPowerupCost(self, p);
				gi.cprintf(self, PRINT_HIGH, "Your %s gained level %d!\n", info->name, skills->pucurlvl[p]);
			}
		}
	}
	for (i = 0; i < GIEX_BASEITEMS; i++) {
		if ((skills->wornItem[i] > -1) && (skills->itemId[(int) skills->wornItem[i]] > 0)) {
			for (p = 0; p < GIEX_PUPERITEM; p++) {
				if (skills->item_putype[(int) skills->wornItem[i]][p] == 0) {
					break;
				}
				if (skills->item_pucurlvl[(int) skills->wornItem[i]][p] >= skills->item_pumaxlvl[(int) skills->wornItem[i]][p]) {
					continue;
				}
				if (!getItemPowerupLevelable(self, skills->wornItem[i], p)) {
					continue;
				}

				skills->item_puexp[(int) skills->wornItem[i]][p] += pershare * EXP_ITEM_SHARES;
				self->client->pers.item_puexpRemain[(int) skills->wornItem[i]][p] -= pershare * EXP_ITEM_SHARES;
				if (self->client->pers.item_puexpRemain[(int) skills->wornItem[i]][p] < 1) {
					powerupinfo_t *puinfo = getPowerupInfo(skills->item_putype[(int) skills->wornItem[i]][p]);
					iteminfo_t *iteminfo = getItemInfo(skills->itemId[(int) skills->wornItem[i]]);
					skills->item_pucurlvl[(int) skills->wornItem[i]][p]++;
					self->client->pers.skill[skills->item_putype[(int) skills->wornItem[i]][p]]++;
					self->client->pers.item_puexpRemain[(int) skills->wornItem[i]][p] = getItemPowerupCost(self, skills->wornItem[i], p);
					gi.cprintf(self, PRINT_HIGH, "Your %s gains level %d %s!\n", iteminfo->name, skills->item_pucurlvl[(int) skills->wornItem[i]][p], puinfo->name);
				}
			}
		}
	}
}
