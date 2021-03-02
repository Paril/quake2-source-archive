#include "g_local.h"
#define GIEX_ITEMDROPS_MULTGOD 1.0
#define GIEX_ITEMDROPS_MULT1 1.0
#define GIEX_ITEMDROPS_MULT2 1.0
#define GIEX_ITEMDROPS_MULT3 1.0
#define GIEX_ITEMDROPS_MULT4 1.0

const char *getBaseTypeName(int num) {
	switch(num) {
		case 0:
			return "Weapon";
		case 1:
			return "Armor";
		case 2:
			return "Helmet";
		case 3:
			return "Amulet";
		case 4:
			return "Belt";
		case 5:
			return "Backpack";
		default:
			return "";
	}
}

//*************************************
//TODO:
//Most items up to 134 are misnumbered!
//*************************************

iteminfo_t *getItemInfo(int num) {
	static iteminfo_t info[] = {
	//name, type, *item, item_name, model_name, rarity, auto_types, auto_levels, maxpowerups, arg1 - arg7
		{"", 0, NULL, 0, "", 0, {0,0,0,0,0}, {0,0,0,0,0}, 0, /*0*/\
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, // arguments
		{"Blaster", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 0, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*1*/\
			{1,		2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			20.0, 0.5, 0.0, 0.0, 0.0375, 1000.0, 7.5, 5.5, 0.19, 0.0},
		{"Shotgun", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_SHOTGUN, "models/weapons/g_shotg/tris.md2", 950, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*2*/\
			{1,		3,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.1,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, count, +count, spread
			60.0, 1.5, 0.0, 8.0, 2.0, 550.0, 0.0, 0.0, 0.0, 0.0},
		{"Super Shotgun", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_SUPERSHOTGUN, "models/weapons/g_shotg2/tris.md2", 900, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*3*/\
			{1,		3,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, count, +count, spread, delaybarrels
			128.0, 2.4, 0.0, 12.0, 3.0, 550.0, 2.0, 0.0, 0.0, 0.0},
		{"Machinegun", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 950, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*4*/\
			{1,		4,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			10.0, 0.15, 0.0, 300.0, 500.0, 35.0, 60.0, 0.0, 0.0, 0.0},
		{"Chaingun", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_CHAINGUN, "models/weapons/g_chain/tris.md2", 870, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*5*/\
			{1,		5,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, notused, numshots, addspread
			7.0, 0.1, 0.0, 1.0, 500.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Handgrenades", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_AM_GRENADES, "models/items/ammo/grenades/medium/tris.md2", 900, {80,0,0,0,0}, {10,0,0,0,0}, 2,/*6*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.1,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, primetime, primetime2, recycletime, radius, +radius, speed, +speed, timer
			125.0, 2.375, 0.0, 0.0, 0.0, 145.0, 0.1, 1.0, 0.05, 0.0},
		{"Grenade Launcher", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_GRENADELAUNCHER, "models/weapons/g_launch/tris.md2", 850, {80,0,0,0,0}, {10,0,0,0,0}, 2,/*7*/\
			{1,		6,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, gravity, speed, defaulttimer, numbounces, timer, +timer
			120.0, 2.2, 0.0, 130.0, 1.0, 600.0, 2.5, 1.0, 2.1, -0.2},
		{"Rocket Launcher", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_ROCKETLAUNCHER, "models/weapons/g_rocket/tris.md2", 840, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*8*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, +firerate, radius, +radius, speed, +speed
			110.0, 2.1, 0.0, 120.0, 0.2, 650.0, 16.0, 0.0, 0.0, 0.0},
		{"Hyperblaster", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_HYPERBLASTER, "models/weapons/g_hyperb/tris.md2", 875, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*9*/\
			{1,	7,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, boltfiredelay (1-3, or 6), notused, notused, speed, +speed, ammouse, +ammouse
			15.0, 0.3, 1.0, 0.0, 0.0, 800.0, 35.0, 0.0, 0.06, 0.0},
		{"Railgun", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 800, {0,0,0,0,0}, {0,0,0,0,0}, 1, /*10*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{35,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			90.0, 1.0, -4.0, 40.0, 0.7, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"BFG10k", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_BFG, "models/weapons/g_bfg/tris.md2", 890, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*11*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, *dmg_lvl, +dmg_charge, speed, +speed, -speed_charge, *ammouse, +mincharge, +maxcharge
			50.0, 0.025, 11.11, 700.0, 15.0, 40.0, 1.6, 2.0, -2.0, 0.0},
		{"Light armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/jacket/tris.md2", 980, {22,0,0,0,0}, {1,0,0,0,0}, 2, /*12*/ \
			{27,	31,	23,	0,		0,		0,		0,		0,		0,		0}, //putypes
			{-25,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			35.0, 3.0, 0.2, 0.016, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0},
/*			25.0, 1.0, 0.3, 0.032, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},*/
		{"Nebelwerfer", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_ROCKETLAUNCHER, "models/weapons/g_rocket/tris.md2", 260, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*13*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.6,	1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{25, 35, 0, 0, 38, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, +radius, speed, +speed
			95.0, 2.0, -3.0, 105.0, 0.1, 750.0, 20.0, 0.0, 0.0, 0.0},
		{"Mortar", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_GRENADELAUNCHER, "models/weapons/g_launch/tris.md2", 45, {80,0,0,0,0}, {20,0,0,0,0}, 1,/*14*/\
			{1,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{20,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.6,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{65, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, gravity, speed, defaulttimer, numbounces, timer, +timer
			220.0, 4.25, 8.0, 140.0, 0.5, 650.0, 5.0, 0.0, 0.1, 0.0},
		{"Railcannon", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 25, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*15*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{35,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{60, 70, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			210.0, 2.5, 4.0, 35.0, 1.3, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Gattling cannon", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_CHAINGUN, "models/weapons/g_chain/tris.md2", 25, {0,0,0,0,0}, {0,0,0,0,0}, 1,/*16*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{70, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, notused, numshots, addspread
			4.67, 0.09, 0.0, 8.0, 850.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Pineapple", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_AM_GRENADES, "models/items/ammo/grenades/medium/tris.md2", 290, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*17*/\
			{1,	7,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.1,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{35, 0, 30, 0, 40, 0, 0, 0}, //classreq
//dmg, +dmg, -primetime, -primetime2, -recycletime, radius, +radius, speed, +speed, timer
			170.0, 3.125, 3.0, 5.0, 2.0, 135.0, 0.15, 1.5, 0.15, -1.0},
		{"Mammoth armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 210, {44,22,0,0,0}, {5,5,0,0,0}, 3, /*18*/ \
			{27,	31,		23,		44,		0,		0,		0,		0,		0,		0}, //putypes
			{20,	0,		10,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 36, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			280.0, 6.4, 0.75, 0.014, 1.0, 3.0, 0.0, 0.0, 0.0, 0.0},
		{"Shotty", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_SHOTGUN, "models/weapons/g_shotg/tris.md2", 475, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*19*/\
			{1,		3,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{18, 26, 16, 0, 26, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, count, +count, spread
			88.0, 2.4, 0.0, 4.0, 1.0, 150.0, 0.0, 0.0, 0.0, 0.0},
		{"Jetpack", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/jacket/tris.md2", 125, {49,0,0,0,0}, {5,0,0,0,0}, 0, /*20*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 15, 20, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			25.0, 0.0, 0.4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Bio armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 200, {26,0,0,0,0}, {28,0,0,0,0}, 2, /*21*/ \
			{30,	26,		34,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		-10,	-5,		0,		0,		0,		0,		0,		0, 	0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{48, 40, 30, 0, 40, 0, 0, 0}, //classreq

//armor, +armor, absorb, +absorb, regen, +regen
			20.0, 4.0, 0.35, 0.01, 1.0, 3.0, 0.0, 0.0, 0.0, 0.0},
		{"Wand", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE | GIEX_ITEM_CLERIC, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 620, {29,47,0,0,0}, {5,2,0,0,0}, 2, /*22*/\
			{29,	33,		47,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-35,	-15,	-30,	0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 8, 5, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			45.0, 5.0, 0.0, 0.3, 0.0, 1250.0, 0.0, 6.0, 0.5, 0.0},
		{"Short staff", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE | GIEX_ITEM_CLERIC, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 380, {29,33,47,0,0}, {7,2,5,0,0}, 2, /*23*/\
			{29,	25,	33,	47,	0,		0,		0,		0,		0,		0}, //putypes
			{-25,	-5,	-10,	-20,	0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 25, 18, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			65.0, 5.0, 0.0, 0.3, 0.0, 1250.0, 0.0, 8.0, 0.5, 0.0},
		{"Staff", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 180, {29,25,33,47,0}, {10,5,4,5,0}, 3, /*24*/\
			{29,	25,	51,	52,	53,	54,	61,	62,	33,	47}, //putypes
			{-10,	0,		-20,	-20,	-20,	-20,	-20,	-20,	-5,	-10}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 35, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			85.0, 5.0, 0.0, 0.3, 0.0, 1250.0, 0.0, 12.0, 0.5, 0.0},
		{"Fire staff", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 25, {78,25,51,52,47}, {10,10,10,10,10}, 4, /*25*/\
			{29,	25,		51,		52,		78,		33,		47,		0,		0,		0}, //putypes
			{0,		0,		-10,	-10,	-10,	0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 55, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			145.0, 5.0, 0.0, 0.3, 0.0, 1250.0, 0.0, 16.0, 0.5, 0.0},
		{"Blood staff", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 25, {60,25,53,62,79}, {10,10,10,10,10}, 4, /*26*/\
			{29,	25,		53,		79,		62,		33,		34,		0,		0,		0}, //putypes
			{0,		0,		-10,	0,		-10,	0,		-10,	0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 60, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			145.0, 5.0, 0.0, 0.3, 0.0, 1250.0, 0.0, 16.0, 0.5, 0.0},
		{"Lifevest", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/jacket/tris.md2", 350, {26,0,0,0,0}, {15,0,0,0,0}, 2, /*27*/ \
			{30,	26,		34,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-10,	-20,	-10,	0,		0,		0,		0,		0,		0, 	0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{36, 25, 15, 40, 27, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			10.0, 0.0, 0.35, 0.01, 1.0, 3.0, 0.0, 0.0, 0.0, 0.0},
		{"Vita Suit", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 30, {26,0,0,0,0}, {45,0,0,0,0}, 3, /*28*/ \
			{30,	26,		34,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	5,		0,		0,		0,		0,		0,		0,		0, 	0}, //pumaxlvl
			{1.2,	1.0,	1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{74, 63, 58, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			30.0, 4.0, 0.35, 0.01, 1.0, 3.0, 0.0, 0.0, 0.0, 0.0},
		{"Potatoemasher", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_AM_GRENADES, "models/items/ammo/grenades/medium/tris.md2", 249, {80,0,0,0,0}, {20,0,0,0,0}, 2,/*29*/\
			{1,	7,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.1,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{25, 0, 20, 0, 30, 0, 0, 0}, //classreq
//dmg, +dmg, primetime, primetime2, recycletime, radius, +radius, speed, +speed, timer
			190.0, 3.5, 0.0, 0.0, 0.0, 135.0, 0.2, 1.9, 0.25, 4.0},
		{"Mage robe", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/jacket/tris.md2", 125, {29,33,0,0,0}, {20,5,0,0,0}, 3, /*30*/ \
			{27,	29,	23,	31,	33,	0,		0,		0,		0,		0}, //putypes
			{-25,	-10,	0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 42, 34, 38, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			80.0, 6.8, 0.3, 0.015, 2.0, 0.5, 0.0, 0.0, 0.0, 0.0},
		{"Handcannon", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_SUPERSHOTGUN, "models/weapons/g_shotg2/tris.md2", 210, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*31*/\
			{1,	3,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{40, 50, 55, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, count, +count, spread, delaybarrels
			204.0, 3.6, 2.0, 10.0, 2.5, 700.0, 1.0, 0.0, 0.0, 0.0},
		{"Soldier armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 150, {24,28,32,1,0}, {5,10,5,5,0}, 4, /*32*/ \
			{27,	28,	23,	31,	24,	32,	1,		0,		0,		0}, //putypes
			{-25,	0,		0,		0,		-5,	0,		-20,	0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{35, 0, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			80.0, 6.8, 0.35, 0.018, 3.0, 0.7, 0.0, 0.0, 0.0, 0.0},
		{"Warrior plate", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 50, {24,28,32,1,0}, {10,25,10,10,0}, 4, /*33*/ \
			{27,	28,		23,		31,		24,		32,		1,		0,		0,		0}, //putypes
			{-15,	0,		0,		0,		0,		0,		-10,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{70, 0, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			120.0, 10.0, 0.4, 0.02, 3.0, 0.7, 0.0, 0.0, 0.0, 0.0},
		{"Combat armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 200, {22,0,0,0,0}, {5,0,0,0,0}, 3, /*34*/ \
			{27,	31,	23,	0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{35, 24, 32, 40, 40, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			75.0, 3.5, 0.3, 0.022, 2.0, 3.5, 0.0, 0.0, 0.0, 0.0},
		{"Armadillo scales", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 200, {31,22,0,0,0}, {5,4,0,0,0}, 3, /*35*/ \
			{27,	31,		23,		44,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	5,		0,		-5,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 39, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			200.0, 5.4, 0.75, 0.02, 5.0, 3.0, 0.0, 0.0, 0.0, 0.0},
		{"Full battle suit", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 40, {44,22,0,0,0}, {10,8,0,0,0}, 3, /*36*/ \
			{27,	31,		23,		44,		0,		0,		0,		0,		0,		0}, //putypes
			{50,	0,		10,		10,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 64, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			420.0, 4.8, 0.85, 0.02, 2.0, 4.0, 0.0, 0.0, 0.0, 0.0},
		{"Turtle shell", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 30, {31,22,0,0,0}, {10,5,0,0,0}, 3, /*37*/ \
			{27,	31,		23,		44,		0,		0,		0,		0,		0,		0}, //putypes
			{30,	15,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 58, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			350.0, 5.0, 0.85, 0.025, 3.0, 5.0, 0.0, 0.0, 0.0, 0.0},
		{"Heavy armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 175, {22,0,0,0,0}, {4,0,0,0,0}, 3, /*38*/ \
			{27,	31,	23,	0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{35, 15, 26, 0, 40, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			105.0, 4.5, 0.3, 0.025, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0},
		{"BFG40k", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_BFG, "models/weapons/g_bfg/tris.md2", 190, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*39*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{45, 41, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, *dmg_lvl, +dmg_charge, speed, +speed, -speed_charge, *ammouse
			80.0, 0.025, 17.0, 600.0, 10.0, 35.0, 2.5, 3.0, 0.0, 0.0},
		{"FEG-5", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_BFG, "models/weapons/g_bfg/tris.md2", 120, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*40*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{38, 34, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, *dmg_lvl, +dmg_charge, speed, +speed, -speed_charge, *ammouse
			60.0, 0.025, 15.55, 800.0, 20.0, 15.0, 2.6, 1.0, -7.0, 0.0},
		{"FEG-10", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_BFG, "models/weapons/g_bfg/tris.md2", 20, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*40*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{60, 55, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, *dmg_lvl, +dmg_charge, speed, +speed, -speed_charge, *ammouse
			70.0, 0.025, 18.89, 850.0, 25.0, 20.0, 2.8, 1.0, -7.0, 0.0},
		{"HEAT Launcher", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_ROCKETLAUNCHER, "models/weapons/g_rocket/tris.md2", 15, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*41*/\
			{1,	2,		7,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{20,	10,	0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{55, 65, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, +radius, speed, +speed
			210.0, 3.875, 4.0, 80.0, 0.05, 750.0, 15.0, 0.0, 0.0, 0.0},
		{"Powerblaster", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_HYPERBLASTER, "models/weapons/g_hyperb/tris.md2", 175, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*42*/\
			{1,	7,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{33, 44, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, boltfiredelay (1-3), notused, notused, speed, +speed, ammouse, +ammouse
			38.0, 1.0, 2.0, 0.0, 0.0, 600.0, 10.0, 3.5, 0.15, 0.0},
		{"Megablaster", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_HYPERBLASTER, "models/weapons/g_hyperb/tris.md2", 25, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*43*/\
			{1,	7,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{20,	10,	0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{62, 74, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, boltfiredelay (1-3), notused, notused, speed, +speed, ammouse, +ammouse
			60.0, 1.75, 3.0, 0.0, 0.0, 1000.0, 30.0, 7.0, 0.22, 0.0},
		{"Pegger", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 350, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*44*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{19, 25, 16, 32, 26, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			11.0, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"SMG", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 450, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*44*/\
			{1,	2,		4,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{24, 30, 20, 40, 32, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			14.0, 0.3, 0.0, 750.0, 900.0, 20.0, 30.0, 0.0, 0.0, 0.0},
		{"Hagelbrak", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_SHOTGUN, "models/weapons/g_shotg/tris.md2", 220, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*45*/\
			{1,	2,		3,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.2,	1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{42, 52, 38, 0, 50, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, count, +count, spread
			100.0, 2.0, -1.0, 8.0, 2.5, 800.0, 0.0, 0.0, 0.0, 0.0},
		{"Boomstick", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_SUPERSHOTGUN, "models/weapons/g_shotg2/tris.md2", 380, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*46*/\
			{1,	2,		3,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.3,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{18, 25, 27, 0, 25, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, count, +count, spread, delaybarrels
			96.0, 1.8, -3.0, 8.0, 1.5, 800.0, 4.0, 0.0, 0.0, 0.0},
		{"Snipe gun", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_SHOTGUN, "models/weapons/g_shotg/tris.md2", 30, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*47*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.1,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{65, 0, 58, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, count, +count, spread
			150.0, 3.0, 2.0, 2.0, 0.0, 25.0, 0.0, 0.0, 0.0, 0.0},
		{"Spamblaster", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_HYPERBLASTER, "models/weapons/g_hyperb/tris.md2", 15, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*48*/\
			{1,		7,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{58, 70, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, boltfiredelay (1-3), notused, notused, speed, +speed, ammouse, +ammouse
			28.0, 0.75, 1.0, 0.0, 0.0, 600.0, 10.0, 3.5, 0.16, 0.0},
		{"Speedblaster", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_HYPERBLASTER, "models/weapons/g_hyperb/tris.md2", 25, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*49*/\
			{1,		7,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{20,	10,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{58, 70, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, boltfiredelay (1-3), notused, notused, speed, +speed, ammouse, +ammouse
			25.0, 0.42, 1.0, 0.0, 0.0, 1000.0, 30.0, 3.5, 0.16, 0.0},
		{"Buster", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 270, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*50*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{17, 27, 32, 42, 27, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			25.0, 0.625, -2.0, 0.0, 0.015, 900.0, 6.0, 5.5, 0.19, 0.0},
		{"Magnum", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 230, {2,0,0,0,0}, {1,0,0,0,0}, 2, /*51*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{45, 60, 50, 70, 60, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			50.0, 1.25, 2.0, 0.2, 0.025, 0.0, 0.0, 5.5, 0.19, 0.0},
		{"Lillpuffra", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 40, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*52*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{55, 65, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			50.0, 1.25, -2.0, 0.0, 0.02, 850.0, 5.0, 5.5, 0.19, 0.0},
		{"Storpuffra", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 30, {2,0,0,0,0}, {1,0,0,0,0}, 2, /*53*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{65, 75, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			60.0, 1.35, 2.0, 0.3, 0.025, 1600.0, 15.0, 5.5, 0.19, 0.0},
		{"Med kit", GIEX_ITEM_WEAPON | GIEX_ITEM_CLERIC, NULL, CI_W_BLASTER, "models/items/healing/medium/tris.md2", 210, {56,66,0,0,0}, {5,5,0,0,0}, 2, /*54*/\
			{56,	57,	66,	0,		0,		0,		0,		0,		0,		0}, //putypes
			{-20,	-20,	-20,	0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 38, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			-40.0, 0.0, -1.0, 0.0, 0.0, 900.0, 0.0, 10.0, 0.0, 0.0},
		{"Healer pack", GIEX_ITEM_WEAPON | GIEX_ITEM_CLERIC, NULL, CI_W_BLASTER, "models/items/healing/large/tris.md2", 15, {56,57,66,0,0}, {10,5,10,0,0}, 2, /*55*/\
			{56,	57,	66,	0,		0,		0,		0,		0,		0,		0}, //putypes
			{-5,	-10,	-10,	0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 60, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			-80.0, 0.0, -2.0, 0.0, 0.0, 1500.0, 0.0, 8.0, 0.0, 0.0},
		{"Apprentice cloak", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/jacket/tris.md2", 325, {29,0,0,0,0}, {10,0,0,0,0}, 3, /*56*/ \
			{27,	29,	23,	31,	33,	0,		0,		0,		0,		0}, //putypes
			{-35,	-30,	0,		0,		-5,	0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 21, 16, 19, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			45.0, 7.0, 0.25, 0.012, 1.0, 0.3, 0.0, 0.0, 0.0, 0.0},
		{"Wizard garments", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 65, {29,33,0,0,0}, {40,8,0,0,0}, 3, /*57*/ \
			{27,	29,	23,	31,	33,	0,		0,		0,		0,		0}, //putypes
			{-15,	0,		0,		0,		10,	0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 64, 58, 60, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			105.0, 6.3, 0.35, 0.019, 2.5, 0.7, 0.0, 0.0, 0.0, 0.0},
		{"Railrifle", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 290, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*58*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{35,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{22, 32, 0, 0, 33, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			140.0, 1.8, 2.0, 40.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Cap", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 550, {0,0,0,0,0}, {0,0,0,0,0}, 1, /*59*/ \
			{27,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-40,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor
			10.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Vampire skull", GIEX_ITEM_HELMET, NULL, 0, "models/objects/gibs/skull/tris.md2", 50, {34,0,0,0,0}, {5,0,0,0,0}, 1, /*60*/ \
			{34,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-5,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{65, 57, 54, 67, 40, 0, 0, 0}, //classreq
//armor, +armor
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Thinking cap", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 180, {25,0,0,0,0}, {2,0,0,0,0}, 1, /*61*/ \
			{29,	33,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		-10,	0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{45, 45, 36, 34, 40, 0, 0, 0}, //classreq
//armor, +armor
			10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Healer's helm", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 50, {26,30,0,0,0}, {10,10,0,0,0}, 2, /*62*/ \
			{56,	66,	0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-10,	-20,	0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{50, 40, 33, 38, 0, 0, 0, 0}, //classreq
//armor, +armor
			40.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Health hat", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 340, {26,0,0,0,0}, {4,0,0,0,0}, 1, /*63*/ \
			{26,	30,	0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-44,	-15,	0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{26, 20, 16, 27, 20, 0, 0, 0}, //classreq
//armor, +armor
			15.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Heavy visor", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 195, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*64*/ \
			{27,	44,	0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	-5,	0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 30, 40, 0, 0, 0, 0, 0}, //classreq
//armor, +armor
			80.0, 1.6, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"ATS mkI", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 55, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*65*/ \
			{24,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-5,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{58, 65, 65, 70, 0, 0, 0, 0}, //classreq
//armor, +armor
			10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Body armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 50, {22,0,0,0,0}, {5,0,0,0,0}, 3, /*66*/ \
			{27,	31,		23,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{55, 25, 35, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			140.0, 5.0, 0.3, 0.03, 0.0, 2.5, 0.0, 0.0, 0.0, 0.0},
		{"Platemail", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 350, {44,22,0,0,0}, {5,4,0,0,0}, 3, /*67*/ \
			{27,	31,		23,		44,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	0,		0,		5,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 16, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			140.0, 5.0, 0.5, 0.03, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0},
		{"Medium armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 420, {22,0,0,0,0}, {4,0,0,0,0}, 3, /*68*/ \
			{27,	31,		23,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{15, 8, 10, 18, 16, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			70.0, 3.75, 0.3, 0.022, 0.0, 1.5, 0.0, 0.0, 0.0, 0.0},
		{"Cap of Prot", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 150, {46,0,0,0,0}, {15,0,0,0,0}, 0, /*69*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{40, 30, 30, 45, 40, 0, 0, 0}, //classreq
//armor, +armor
			40.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Helm of Pent", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 15, {46,0,0,0,0}, {30,0,0,0,0}, 0, /*70*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{60, 50, 50, 65, 60, 0, 0, 0}, //classreq
//armor, +armor
			80.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Can Lobber", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_GRENADELAUNCHER, "models/weapons/g_launch/tris.md2", 170, {0,0,0,0,0}, {0,0,0,0,0}, 1,/*71*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{38, 46, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, gravity, speed, defaulttimer, numbounces, timer, +timer
			130.0, 2.25, -4.0, 120.0, 1.0, 600.0, 1.5, 1.0, 2.1, -0.2},
		{"Carl Gustaf", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_GRENADELAUNCHER, "models/weapons/g_launch/tris.md2", 50, {80,0,0,0,0}, {15,0,0,0,0}, 2,/*72*/\
			{1,	6,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{56, 65, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, gravity, speed, defaulttimer, numbounces, timer, +timer
			210.0, 3.75, 0.0, 130.0, 1.0, 800.0, 2.5, 1.0, 2.1, -0.2},
		{"Chaincannon", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_CHAINGUN, "models/weapons/g_chain/tris.md2", 70, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*73*/\
			{1,	5,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{53, 64, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, notused, numshots, spread
			8.0, 0.19, 0.0, 0.1, 300.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Minigun", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_CHAINGUN, "models/weapons/g_chain/tris.md2", 175, {0,0,0,0,0}, {0,0,0,0,0}, 1,/*74*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{45, 57, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, notused, numshots, addspread
			5.7, 0.125, 0.0, 5.0, 650.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Wizard helm", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 25, {29,33,25,0,0}, {10,4,5,0,0}, 2, /*75*/ \
			{29,	33,	0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	-10,	0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{75, 75, 60, 55, 65, 0, 0, 0}, //classreq
//armor, +armor
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Bio helm", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 165, {26,30,0,0,0}, {7,2,0,0,0}, 1, /*76*/ \
			{26,	30,	0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-41,	-13,	0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{50, 40, 35, 52, 46, 0, 0, 0}, //classreq
//armor, +armor
			10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Vita helmet", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 15, {26,30,0,0,0}, {15,5,0,0,0}, 2, /*77*/ \
			{26,	30,	0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-35,	-10,	0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{65, 59, 53, 66, 60, 0, 0, 0}, //classreq
//armor, +armor
			5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Hat of Plenty", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 35, {30,31,32,33,35}, {3,3,3,3,5}, 2, /*78*/ \
			{30,	31,	32,	33,	0,		0,		0,		0,		0,		0}, //putypes
			{-13,	-13,	-13,	-13,	0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{50, 50, 50, 50, 50, 0, 0, 0}, //classreq
//armor, +armor
			10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Helmet", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 330, {0,0,0,0,0}, {0,0,0,0,0}, 1, /*79*/ \
			{27,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-30,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{20, 12, 15, 22, 20, 0, 0, 0}, //classreq
//armor, +armor
			20.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Visor", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 125, {0,0,0,0,0}, {0,0,0,0,0}, 1, /*80*/ \
			{27,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-20,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{38, 25, 29, 40, 35, 0, 0, 0}, //classreq
//armor, +armor
			40.0, 1.4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Hand of God", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_SHOTGUN, "models/weapons/g_shotg/tris.md2", 1, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*81*/\
			{1,	2,		3,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.2,	1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{86, 95, 82, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, count, +count, spread
			180.0, 4.5, 2.0, 8.0, 3.0, 400.0, 0.0, 0.0, 0.0, 0.0},
		{"Wrath of God", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_SUPERSHOTGUN, "models/weapons/g_shotg2/tris.md2", 1, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*82*/\
			{1,	2,		3,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.5,	1.7,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{90, 0, 95, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, count, +count, spread, delaybarrels
			264.0, 5.0, 3.0, 12.0, 3.0, 750.0, 0.0, 0.0, 0.0, 0.0},
		{"Holy Handgrenade of Antioch", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_AM_GRENADES, "models/items/ammo/grenades/medium/tris.md2", 1, {80,0,0,0,0}, {20,0,0,0,0}, 2,/*83*/\
			{1,		7,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.1,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{92, 0, 85, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, primetime, primetime2, recycletime, radius, +radius, speed, +speed, timer
			250.0, 4.5, 0.0, 0.0, 0.0, 130.0, 0.1, 1.3, 0.3, 0.0},
		{"Pebble", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_AM_GRENADES, "models/items/ammo/grenades/medium/tris.md2", 30, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*84*/\
			{1,	7,		2,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.1,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{55, 0, 45, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, primetime, primetime2, recycletime, radius, +radius, speed, +speed, timer
			160.0, 4.1, 3.0, 5.0, 3.0, 130.0, 0.1, 1.2, 0.12, -1.5},
		{"Slugthrower", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 270, {0,0,0,0,0}, {0,0,0,0,0}, 1, /*85*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{30,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{18, 32, 0, 0, 34, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			120.0, 1.42, -2.0, 40.0, 0.7, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Slugcannon", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 15, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*86*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{40,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{57, 70, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			155.0, 1.8, -4.0, 35.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"God's Lance", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 1, {2,0,0,0,0}, {5,0,0,0,0}, 2, /*87*/\
			{1,		2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{40,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{95, 0, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			290.0, 3.2, 6.0, 40.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Godshell", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 1, {44,22,0,0,0}, {15,5,0,0,0}, 3, /*88*/ \
			{27,	31,		23,		44,		0,		0,		0,		0,		0,		0}, //putypes
			{75,	0,		10,		10,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 90, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			560.0, 4.5, 0.9, 0.022, 3.0, 3.5, 0.0, 0.0, 0.0, 0.0},
		{"Skin of God", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 1, {31,22,0,0,0}, {15,5,0,0,0}, 3, /*89*/ \
			{27,	31,		23,		44,		0,		0,		0,		0,		0,		0}, //putypes
			{50,	15,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 85, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			450.0, 5.0, 0.9, 0.03, 4.0, 8.0, 0.0, 0.0, 0.0, 0.0},
		{"God's Brain", GIEX_ITEM_HELMET, NULL, 0, "models/objects/gibs/skull/tris.md2", 1, {25,33,29,34,0}, {7,8,20,5,0}, 3, /*90*/ \
			{25,	33,	29,	0,		0,		0,		0,		0,		0,		0}, //putypes
			{-2,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 95, 90, 100, 0, 0, 0}, //classreq
//armor, +armor
			100.0, 1.4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"FNC80", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 50, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*91*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{20,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{65, 75, 58, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			16.0, 0.25, 0.0, 250.0, 300.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"AK-M", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 45, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*92*/\
			{1,	2,		4,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{59, 65, 54, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			20.0, 0.375, 0.0, 600.0, 800.0, 25.0, 35.0, 0.0, 0.0, 0.0},
		{"Lightning staff", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 25, {81,25,50,67,68}, {10,10,10,10,10}, 4, /*93*/\
			{29,	25,		50,		67,		68,		33,		81,		0,		0,		0}, //putypes
			{0,		10,		-10,	-10,	-10,	0,		-10,	0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 58, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			145.0, 5.0, 0.0, 0.3, 0.0, 1250.0, 0.0, 16.0, 0.5, 0.0},
		{"Flak vest", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/jacket/tris.md2", 780, {22,0,0,0,0}, {2,0,0,0,0}, 2, /*94*/ \
			{27,	31,	23,	0,		0,		0,		0,		0,		0,		0}, //putypes
			{-25,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{5, 2, 3, 5, 6, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			25.0, 2.0, 0.2, 0.016, 2.0, 2.0, 0.0, 0.0, 0.0, 0.0},
		{"Kevlar vest", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/jacket/tris.md2", 390, {22,0,0,0,0}, {3,0,0,0,0}, 2, /*95*/ \
			{27,	31,	23,	0,		0,		0,		0,		0,		0,		0}, //putypes
			{-10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{17, 9, 11, 20, 20, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			50.0, 3.0, 0.2, 0.018, 2.0, 2.5, 0.0, 0.0, 0.0, 0.0},
		{"Ribs of Nosferatu", GIEX_ITEM_ARMOR, NULL, 0, "models/objects/gibs/chest/tris.md2", 2, {34,29,33,74,0}, {10,20,8,10,0}, 2, /*96*/ \
			{34,	29,	33,	0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	-20,	-10,	0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 82, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			5.0, 0.0, 0.2, 0.018, 2.0, 2.5, 0.0, 0.0, 0.0, 0.0},
		{"Katyusha", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_ROCKETLAUNCHER, "models/weapons/g_rocket/tris.md2", 120, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*97*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.6,	1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{35, 45, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, +radius, speed, +speed
			120.0, 2.2, -2.0, 110.0, 0.15, 650.0, 15.0, 0.0, 0.0, 0.0},
		{"M16", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 150, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*98*/\
			{1,	2,		4,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{38, 52, 32, 0, 50, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			14.0, 0.35, 0.0, 400.0, 450.0, 20.0, 25.0, 0.0, 0.0, 0.0},
		{"M242", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 1, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*99*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{30,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{95, 0, 90, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			38.0, 0.65, 1.0, 25.0, 50.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"KSP58b", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_CHAINGUN, "models/weapons/g_chain/tris.md2", 130, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*100*/\
			{1,	5,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{45, 52, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, notused, numshots, spread
			6.7, 0.16, 0.0, 1.0, 400.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"KSP90", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_CHAINGUN, "models/weapons/g_chain/tris.md2", 170, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*101*/\
			{1,	5,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{35, 43, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, notused, numshots, spread
			6.0, 0.13, 0.0, 1.0, 300.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Bandolier", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/band/tris.md2", 310, {28,32,0,0,0}, {5,2,0,0,0}, 2, /*102*/ \
			{27,	28,		32,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-25,	-30,	-12,	0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{15, 20, 17, 25, 25, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			26.0, 1.6, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Ammopack", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/pack/tris.md2", 170, {28,32,0,0,0}, {10,5,0,0,0}, 2, /*103*/ \
			{27,	28,		32,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-10,	-10,	-5,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{32, 40, 35, 50, 50, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			40.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Combatant vest", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/jacket/tris.md2", 260, {28,32,1,0,0}, {3,1,2,0,0}, 4, /*104*/ \
			{27,	28,	23,	31,	32,	1,		0,		0,		0,		0}, //putypes
			{-30,	-23,	0,		0,		-15,	-27,	0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{18, 30, 23, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			60.0, 5.2, 0.35, 0.018, 3.0, 0.7, 0.0, 0.0, 0.0, 0.0},
		{"Desert Eagle", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 150, {2,0,0,0,0}, {2,0,0,0,0}, 2, /*105*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{40, 60, 50, 70, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			45.0, 1.25, 1.5, 0.2, 0.025, 0.0, 0.0, 7.5, 0.19, 0.0},
		{"Casull .454", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 120, {2,1,0,0,0}, {2,2,0,0,0}, 2, /*106*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{5,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{43, 35, 50, 70, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			60.0, 1.30, 1.5, 0.2, 0.025, 0.0, 0.0, 5.0, 0.19, 0.0},
		{"RRS10", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 285, {0,0,0,0,0}, {0,0,0,0,0}, 1, /*107*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{25,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{10, 15, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			90.0, 1.40, -2.5, 40.0, 0.7, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"RRS20", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 270, {0,0,0,0,0}, {0,0,0,0,0}, 1, /*108*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{25,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{15, 20, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			100.0, 1.40, -2.4, 40.0, 0.7, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"RRS40", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 200, {0,0,0,0,0}, {0,0,0,0,0}, 1, /*109*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{30,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{25, 30, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			110.0, 1.40, -2.3, 40.0, 0.7, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"RRS80", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 180, {0,0,0,0,0}, {0,0,0,0,0}, 1, /*110*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{30,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{35, 40, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			115.0, 1.45, -2.2, 40.0, 0.7, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"RRS100", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 150, {0,0,0,0,0}, {0,0,0,0,0}, 1, /*111*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{30,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{45, 50, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			130.0, 1.45, -2.0, 40.0, 0.7, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"RRS250", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 30, {0,0,0,0,0}, {0,0,0,0,0}, 1, /*112*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{40,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{60, 60, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			140.0, 1.5, -2.0, 40.0, 0.7, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"HEWS10", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 200, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*113*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{35,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{20, 15, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			150.0, 2.0, 2.3, 35.0, 1.3, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"HEWS100", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 100, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*114*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{35,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{38, 23, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			170.0, 2.5, 2.5, 35.0, 1.3, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"HEWS500", GIEX_ITEM_WEAPON | GIEX_ITEM_TANK, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 50, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*115*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{40,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{54, 38, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			230.0, 2.5, 4.3, 35.0, 1.3, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"HEWS1k Widowmaker", GIEX_ITEM_WEAPON | GIEX_ITEM_TANK, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 1, {1,2,0,0,0}, {5,5,0,0,0}, 2, /*116*/\
			{1,	2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{40,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{95, 90, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			280.0, 3.0, 4.3, 35.0, 1.3, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"M79", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_GRENADELAUNCHER, "models/weapons/g_launch/tris.md2", 375, {80,0,0,0,0}, {10,0,0,0,0}, 2,/*117*/\
			{1,	6,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{18, 18, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, gravity, speed, defaulttimer, numbounces, timer, +timer
			140.0, 2.2, 1.0, 130.0, 1.0, 600.0, 2.5, 1.0, 2.1, -0.2},
		{"Big Bertha", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_GRENADELAUNCHER, "models/weapons/g_launch/tris.md2", 1, {1,80,0,0,0}, {10,20,0,0,0}, 1,/*118*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{20,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.6,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{75, 70, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, gravity, speed, defaulttimer, numbounces, timer, +timer
			270.0, 4.25, 9.0, 140.0, 0.5, 650.0, 5.0, 0.0, 0.1, 0.0},
		{"MP40 Schmeisser", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 350, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*119*/\
			{1,	2,		4,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{19, 25, 16, 32, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			12, 0.2, 0.0, 650.0, 700.0, 20.0, 30.0, 0.0, 0.0, 0.0},
		{"Suomi KP", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 40, {2,0,0,0,0}, {1,0,0,0,0}, 2,/*120*/\
			{1,	2,		4,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{28, 53, 29, 56, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			18, 0.225, 0.0, 650.0, 700.0, 25.0, 35.0, 0.0, 0.0, 0.0},
		{"MG42", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC | GIEX_ITEM_TANK, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 120, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*121*/\
			{1,	2,		4,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{40, 48, 46, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			12, 0.2, 0.0, 650.0, 700.0, 25.0, 35.0, 0.0, 0.0, 0.0},
		{"Staff of Gorgil", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 1, {78,25,51,52,47}, {15,15,15,15,15}, 4, /*122*/\
			{29,	25,		51,		52,		78,		33,		47,		0,		0,		0}, //putypes
			{10,	10,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 89, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			105.0, 5.0, 0.0, 0.3, 0.0, 1250.0, 0.0, 25.0, 0.5, 0.0},
		{"Staff of Murgh", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 1, {79,25,53,62,60}, {15,15,15,15,15}, 4, /*123*/\
			{29,	25,		53,		60,		62,		33,		79,		34,		0,		0}, //putypes
			{10,	10,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 89, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			105.0, 5.0, 0.0, 0.3, 0.0, 1250.0, 0.0, 25.0, 0.5, 0.0},
		{"Staff of Drafgir", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 1, {81,25,50,67,68}, {15,15,15,15,15}, 4, /*124*/\
			{29,	25,		50,		67,		68,		33,		81,		0,		0,		0}, //putypes
			{10,	10,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 89, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			105.0, 5.0, 0.0, 0.3, 0.0, 1250.0, 0.0, 25.0, 0.5, 0.0},
		{"Cloak of Gorgil", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 1, {78,33,51,52,47}, {5,10,5,5,5}, 3, /*125*/ \
			{27,	29,		23,		31,		33,		51,		52,		47,		78,		0}, //putypes
			{-15,	0,		0,		0,		10,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 85, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			105.0, 6.3, 0.35, 0.019, 2.5, 0.7, 0.0, 0.0, 0.0, 0.0},
		{"Cloak of Murgh", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 1, {62,33,53,79,60}, {5,10,5,5,5}, 3, /*126*/ \
			{27,	29,		23,		31,		33,		53,		79,		34,		62,		60}, //putypes
			{-15,	0,		0,		0,		10,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 85, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			105.0, 6.3, 0.35, 0.019, 2.5, 0.7, 0.0, 0.0, 0.0, 0.0},
		{"Cloak of Drafgir", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 1, {81,33,50,67,68}, {5,10,5,5,5}, 3, /*127*/ \
			{27,	29,		23,		31,		33,		50,		67,		68,		81,		0}, //putypes
			{-15,	0,		0,		0,		10,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 85, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			105.0, 6.3, 0.35, 0.019, 2.5, 0.7, 0.0, 0.0, 0.0, 0.0},
		{"Enviro suit", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/enviro/tris.md2", 130, {41,0,0,0,0}, {10,0,0,0,0}, 0, /*128*/ \
			{27,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-20,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{25, 25, 25, 25, 25, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			20.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Impact suit", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/enviro/tris.md2", 25, {41,36,37,0,0}, {15,15,15,0,0}, 0, /*129*/ \
			{27,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-20,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{60, 55, 65, 80, 70, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			40.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Spell shield", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/enviro/tris.md2", 20, {42,43,69,0,0}, {15,15,15,0,0}, 0, /*130*/ \
			{27,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-20,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{80, 65, 60, 50, 60, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			40.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Demolitioners suit", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/enviro/tris.md2", 50, {38,46,0,0,0}, {13,5,0,0,0}, 0, /*131*/ \
			{27,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{45, 40, 50, 60, 55, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			40.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Energy dampener", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/enviro/tris.md2", 30, {39,40,46,0,0}, {13,13,5,0,0}, 0, /*134*/ \
			{27,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-20,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{47, 43, 52, 64, 60, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			40.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Scanner", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 350, {70,0,0,0,0}, {1,0,0,0,0}, 0, /*135*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{15, 15, 15, 15, 15, 0, 0, 0}, //classreq
//armor, +armor
			20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Probe", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 175, {70,0,0,0,0}, {2,0,0,0,0}, 0, /*136*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{30, 30, 30, 30, 30, 0, 0, 0}, //classreq
//armor, +armor
			30.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Targetting system", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 50, {70,0,0,0,0}, {3,0,0,0,0}, 0, /*137*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{50, 50, 50, 50, 50, 0, 0, 0}, //classreq
//armor, +armor
			40.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Tankbane", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_SUPERSHOTGUN, "models/weapons/g_shotg2/tris.md2", 1, {71,0,0,0,0}, {40,0,0,0,0}, 2, /*138*/\
			{1,	3,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.7,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{82, 0, 86, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, count, +count, spread, delaybarrels
			160.0, 6.0, 1.0, 10.0, 2.0, 750.0, 0.0, 0.0, 0.0, 0.0},
		{"Armorbuster", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_ROCKETLAUNCHER, "models/weapons/g_rocket/tris.md2", 25, {71,0,0,0,0}, {20,0,0,0,0}, 2,/*139*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{20,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{60, 0, 75, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, +radius, speed, +speed
			110.0, 2.3, 0.0, 120.0, 0.5, 750.0, 15.0, 0.0, 0.0, 0.0},
		{"Lowling's arm", GIEX_ITEM_WEAPON | GIEX_ITEM_VAMPIRE, NULL, CI_W_ARM, "models/objects/gibs/arm/tris.md2", 750, {34,0,0,0,0}, {2,0,0,0,0}, 1,/*140*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 2, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, recycle, range, count, hspread, vspread, kick, magic_cost
			4.0, 0.08, 0.0, 0.0, 300.0, 3.0, 800.0, 100.0, -10.0, 8.0},
		{"Initiate's arm", GIEX_ITEM_WEAPON | GIEX_ITEM_VAMPIRE, NULL, CI_W_ARM, "models/objects/gibs/arm/tris.md2", 400, {34,0,0,0,0}, {4,0,0,0,0}, 1,/*141*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 16, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, recycle, range, count, hspread, vspread, kick, magic_cost
			5.0, 0.1, 0.0, 0.0, 500.0, 3.0, 350.0, 100.0, -24.0, 12.0},
		{"Acolyte's arm", GIEX_ITEM_WEAPON | GIEX_ITEM_VAMPIRE, NULL, CI_W_ARM, "models/objects/gibs/arm/tris.md2", 350, {34,0,0,0,0}, {3,0,0,0,0}, 1,/*142*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 18, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, recycle, range, count, hspread, vspread, kick, magic_cost
			7.0, 0.175, 1.0, 4.0, 250.0, 5.0, 1200.0, 300.0, -30.0, 14.0},
		{"Tormentor's arm", GIEX_ITEM_WEAPON | GIEX_ITEM_VAMPIRE, NULL, CI_W_ARM, "models/objects/gibs/arm/tris.md2", 180, {34,29,0,0,0}, {8,5,0,0,0}, 1,/*143*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 34, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, recycle, range, count, hspread, vspread, kick, magic_cost
			8.0, 0.225, 0.0, 0.0, 450.0, 2.0, 250.0, 100.0, -20.0, 12.0},
		{"Slayer's arm", GIEX_ITEM_WEAPON | GIEX_ITEM_VAMPIRE, NULL, CI_W_ARM, "models/objects/gibs/arm/tris.md2", 160, {34,26,0,0,0}, {6,6,0,0,0}, 1,/*144*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 38, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, recycle, range, count, hspread, vspread, kick, magic_cost
			6.0, 0.15, 1.0, 5.0, 350.0, 6.0, 700.0, 160.0, -30.0, 22.0},
		{"Highlord's arm", GIEX_ITEM_WEAPON | GIEX_ITEM_VAMPIRE, NULL, CI_W_ARM, "models/objects/gibs/arm/tris.md2", 50, {34,29,0,0,0}, {12,10,0,0,0}, 1,/*145*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 56, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, recycle, range, count, hspread, vspread, kick, magic_cost
			5.0, 0.125, 0.0, 0.0, 450.0, 4.0, 300.0, 120.0, -15.0, 18.0},
		{"Warlord's arm", GIEX_ITEM_WEAPON | GIEX_ITEM_VAMPIRE, NULL, CI_W_ARM, "models/objects/gibs/arm/tris.md2", 40, {34,26,0,0,0}, {9,12,0,0,0}, 1,/*146*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 58, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, recycle, range, count, hspread, vspread, kick, magic_cost
			5.0, 0.15, 1.0, 6.0, 300.0, 8.0, 650.0, 150.0, -40.0, 30.0},
		{"Arm of Nosferatu", GIEX_ITEM_WEAPON | GIEX_ITEM_VAMPIRE, NULL, CI_W_ARM, "models/objects/gibs/arm/tris.md2", 2, {34,29,0,0,0}, {16,15,0,0,0}, 1,/*147*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 88, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, recycle, range, count, hspread, vspread, kick, magic_cost
			12.0, 0.3, 0.0, 0.0, 500.0, 2.0, 150.0, 150.0, -10.0, 36.0},
		{"Arm of Brujha", GIEX_ITEM_WEAPON | GIEX_ITEM_VAMPIRE, NULL, CI_W_ARM, "models/objects/gibs/arm/tris.md2", 2, {34,26,0,0,0}, {12,18,0,0,0}, 1,/*148*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 94, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, recycle, range, count, hspread, vspread, kick, magic_cost
			8.0, 0.2, 1.0, 2.0, 450.0, 6.0, 450.0, 120.0, -25.0, 42.0},
		{"Ribs of Brujha", GIEX_ITEM_ARMOR, NULL, 0, "models/objects/gibs/chest/tris.md2", 2, {34,26,30,1,0}, {10,26,10,10,0}, 2, /*149*/ \
			{34,	26,	30,	0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	-20,	-10,	0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 82, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			5.0, 0.0, 0.2, 0.018, 2.0, 2.5, 0.0, 0.0, 0.0, 0.0},
		{"Adrenal belt", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/band/tris.md2", 250, {21,26,30,0,0}, {5,5,3,0,0}, 0, /*150*/ \
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{20, 20, 20, 20, 20, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Toolbelt", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/band/tris.md2", 250, {20,27,31,0,0}, {5,5,3,0,0}, 0, /*151*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{20, 20, 20, 20, 20, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			30.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Ammobelt", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/band/tris.md2", 250, {19,28,32,0,0}, {5,5,3,0,0}, 0, /*152*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{20, 20, 20, 20, 20, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Utility belt", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/band/tris.md2", 120, {35,26,27,28,0}, {5,4,4,4,0}, 0, /*153*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{30, 30, 30, 30, 30, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Utility vest", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/band/tris.md2", 120, {35,26,27,28,0}, {10,8,8,8,0}, 0, /*154*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{50, 50, 50, 50, 50, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			40.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Adrenal vest", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/band/tris.md2", 80, {21,26,30,0,0}, {10,10,6,0,0}, 0, /*155*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{40, 40, 40, 40, 40, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			30.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Toolkit", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/band/tris.md2", 80, {20,27,31,0,0}, {10,10,6,0,0}, 0, /*156*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{40, 40, 40, 40, 40, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			40.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Ammo vest", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/band/tris.md2", 80, {19,28,32,0,0}, {10,10,6,0,0}, 0, /*157*/ \
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{40, 40, 40, 40, 40, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			30.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"BFG20k", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_BFG, "models/weapons/g_bfg/tris.md2", 390, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*158*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{25, 21, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, *dmg_lvl, +dmg_charge, speed, +speed, -speed_charge, *ammouse
			68.0, 0.025, 14.5, 650.0, 15.0, 40.0, 2.0, 3.0, -1.0, 0.0},
		{"FEG-2", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_BFG, "models/weapons/g_bfg/tris.md2", 300, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*159*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{22, 18, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, *dmg_lvl, +dmg_charge, speed, +speed, -speed_charge, *ammouse
			50.0, 0.025, 13.0, 900.0, 25.0, 15.0, 2.2, 1.0, -7.0, 0.0},
		{"BFG80k", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_BFG, "models/weapons/g_bfg/tris.md2", 30, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*160*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{65, 58, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, *dmg_lvl, +dmg_charge, speed, +speed, -speed_charge, *ammouse
			95.0, 0.03,  19.5, 550.0, 20.0, 30.0, 3.2, 4.0, 0.0, 0.0},
		{"Fetto 2000", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_BFG, "models/weapons/g_bfg/tris.md2", 1, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*161*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{95, 89, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, *dmg_lvl, +dmg_charge, speed, +speed, -speed_charge, *ammouse
			110.0, 0.028,  22.0, 500.0, 40.0, 30.0, 4.0, 5.0, 0.0, 0.0},
		{"Mental hat", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 290, {25,0,0,0,0}, {1,0,0,0,0}, 1, /*162*/ \
			{29,	33,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-20,	-15,	0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{25, 25, 18, 16, 20, 0, 0, 0}, //classreq
//armor, +armor
			8.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Avenger", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 1, {1,2,0,0,0}, {10,5,0,0,0}, 2, /*163*/\
			{1,		2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{90, 85, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			30.0, 0.55, -3.0, 0.3, 0.025, 1500.0, 1.0, 5.5, 0.15, 0.0},
		{"Vulcan", GIEX_ITEM_WEAPON | GIEX_ITEM_TANK, NULL, CI_W_CHAINGUN, "models/weapons/g_chain/tris.md2", 1, {5,0,0,0,0}, {6,0,0,0,0}, 2,/*164*/\
			{1,		2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{2.0,	1.5,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{96, 100, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, notused, numshots, addspread
			4.2, 0.09, 0.0, 12.0, 750.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Firestorm", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_ROCKETLAUNCHER, "models/weapons/g_rocket/tris.md2", 1, {71,1,0,0,0}, {10,10,0,0,0}, 2,/*165*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{20,	10,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.5,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{90, 90, 85, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, +radius, speed, +speed
			120.0, 3.0, -3.0, 90.0, 0.5, 850.0, 15.0, 0.0, 0.0, 0.0},
		{"Wave", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_ROCKETLAUNCHER, "models/weapons/g_rocket/tris.md2", 1, {2,0,0,0,0}, {10,0,0,0,0}, 1,/*166*/\
			{1,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{30,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{3.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{105, 105, 95, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, +radius, speed, +speed
			390.0, 5.0, 6.0, 120.0, 1.0, 550.0, 0.0, 0.0, 0.0, 0.0},
		{"Catapult", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_GRENADELAUNCHER, "models/weapons/g_launch/tris.md2", 150, {80,0,0,0,0}, {20,0,0,0,0}, 1,/*167*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{40, 44, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, gravity, speed, defaulttimer, numbounces, timer, +timer
			160.0, 2.25, 3.0, 120.0, 0.3, 700.0, 1.5, 1.0, 2.1, -0.2},
		{"Holy Might", GIEX_ITEM_WEAPON | GIEX_ITEM_CLERIC, NULL, CI_W_SHOTGUN, "models/weapons/g_shotg/tris.md2", 5, {1,3,0,0,0}, {10,8,0,0,0}, 2, /*168*/\
			{1,		2,		3,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.1,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 75, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, count, +count, spread
			150.0, 4.0, 1.0, 7.0, 3.2, 750.0, 0.0, 0.0, 0.0, 0.0},
		{"Vampire Cloak", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/jacket/tris.md2", 250, {26,74,0,0,0}, {15,10,0,0,0}, 2, /*169*/ \
			{34,	26,	0,	0,		0,		0,		0,		0,		0,		0}, //putypes
			{-10,	-20,	0,	0,		0,		0,		0,		0,		0, 	0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 25, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			15.0, 0.0, 0.35, 0.01, 1.0, 3.0, 0.0, 0.0, 0.0, 0.0},
		{"Cryptlord's arm", GIEX_ITEM_WEAPON | GIEX_ITEM_VAMPIRE, NULL, CI_W_ARM, "models/objects/gibs/arm/tris.md2", 25, {34,29,0,0,0}, {10,15,0,0,0}, 1,/*170*/\
			{1,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 70, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, recycle, range, count, hspread, vspread, kick, magic_cost
			7.0, 0.2, 1.0, 3.0, 400.0, 6.0, 550.0, 130.0, -35.0, 30.0},
		{"Big Earl", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_ROCKETLAUNCHER, "models/weapons/g_rocket/tris.md2", 100, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*171*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.6,	1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{37, 47, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, +radius, speed, +speed
			150.0, 2.5, 3.0, 120.0, 0.2, 550.0, 15.0, 0.0, 0.0, 0.0},
		{"Disintegrator", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_HYPERBLASTER, "models/weapons/g_hyperb/tris.md2", 1, {2,0,0,0,0}, {5,0,0,0,0}, 2,/*172*/\
			{1,		2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{20,	20,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{88, 95, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, boltfiredelay (1-3), notused, notused, speed, +speed, ammouse, +ammouse
			100.0, 2.5, 6.0, 0.0, 0.0, 2200.0, 0.0, 25.0, 0.2, 0.0},
		{"Lich head", GIEX_ITEM_HELMET, NULL, 0, "models/objects/gibs/skull/tris.md2", 30, {29,73,74,0,0}, {10,5,5,0,0}, 1, /*173*/ \
			{29,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-20,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 56, 0, 0, 0}, //classreq
//armor, +armor
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"KVKK", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 35, {0,0,0,0,0}, {0,0,0,0,0}, 2,/*174*/\
			{1,		2,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{20,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{67, 76, 59, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			9.5, 0.3, 0.0, 150.0, 150.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Staff of Magewarrior", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 1, {29,25,65,0,0}, {30,15,15,0,0}, 4, /*175*/\
			{29,	25,		1,		65,		77,		0,		0,		0,		0,		0}, //putypes
			{10,	10,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 95, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			200.0, 5.8, 4.0, 0.4, 0.0, 0.0, 0.0, 35.0, 0.5, 0.0},
		{"Battle Staff", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 180, {29,25,33,65,0}, {10,5,5,10,0}, 3, /*176*/\
			{29,	25,		65,		1,		 77,	0,		0,		0,		0,		0}, //putypes
			{-10,	0,		-20,	0,		-20,	0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 45, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			100.0, 4.8, 4.0, 0.3, 0.0, 0.0, 0.0, 22.0, 0.5, 0.0},
		{"Stimpack", GIEX_ITEM_WEAPON | GIEX_ITEM_CLERIC, NULL, CI_W_BLASTER, "models/items/healing/stimpack/tris.md2", 400, {56,66,0,0,0}, {3,3,0,0,0}, 2, /*177*/\
			{56,	57,		66,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-10,	-10,	-30,	0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 15, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			-28.0, 0.0, -1.0, 0.0, 0.0, 900.0, 0.0, 7.0, 0.0, 0.0},
		{"Peacemaker", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER, NULL, CI_W_SUPERSHOTGUN, "models/weapons/g_shotg2/tris.md2", 5, {0,0,0,0,0}, {0,0,0,0,0}, 2, /*178*/\
			{1,	3,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{80, 80, 82, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, count, +count, spread, delaybarrels
			247.0, 3.9, 3.0, 9.0, 2.5, 600.0, 0.5, 0.0, 0.0, 0.0},
		{"Crippler", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_AM_GRENADES, "models/items/ammo/grenades/medium/tris.md2", 350, {80,0,0,0,0}, {10,0,0,0,0}, 2,/*179*/\
			{1,	7,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.2,	1.1,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{26, 0, 22, 0, 30, 0, 0, 0}, //classreq
//dmg, +dmg, primetime, primetime2, recycletime, radius, +radius, speed, +speed, timer
			150.0, 2.75, 0.0, 0.0, 0.0, 135.0, 0.5, 1.0, 0.05, 0.0},
		{"Bouncer", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_TANK, NULL, CI_W_GRENADELAUNCHER, "models/weapons/g_launch/tris.md2", 300, {80,0,0,0,0}, {10,0,0,0,0}, 2,/*180*/\
			{1,	6,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{15, 17, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, gravity, speed, defaulttimer, numbounces, timer, +timer
			130.0, 2.2, 0.0, 130.0, 0.3, 800.0, 2.5, 4.0, 2.1, -0.2},
		{"Mask of the Intimidator", GIEX_ITEM_HELMET, NULL, 0, "models/items/invulner/tris.md2", 1, {46,0,0,0,0}, {12,0,0,0,0}, 2, /*181*/ \
			{26,	30,		24,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{95, 95, 95, 0, 0, 0, 0, 0}, //classreq
//armor, +armor
			200.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Mask of the Negator", GIEX_ITEM_HELMET, NULL, 0, "models/items/invulner/tris.md2", 1, {46,34,0,0,0}, {12,15,0,0,0}, 2, /*182*/ \
			{29,	33,		25,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{-10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 95, 95, 0, 0, 0}, //classreq
//armor, +armor
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Arm of Endbringer", GIEX_ITEM_WEAPON | GIEX_ITEM_VAMPIRE, NULL, CI_W_ARM, "models/objects/gibs/arm/tris.md2", 1, {34,29,0,0,0}, {18,20,0,0,0}, 1,/*183*/\
			{1,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 102, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, recycle, range, count, hspread, vspread, kick, magic_cost
			15.0, 0.3, 0.0, 0.0, 550.0, 2.0, 150.0, 150.0, -10.0, 28.0},
		{"Eternal Hunger", GIEX_ITEM_WEAPON | GIEX_ITEM_VAMPIRE, NULL, CI_W_ARM, "models/objects/gibs/arm/tris.md2", 1, {34,26,0,0,0}, {16,20,0,0,0}, 1,/*184*/\
			{1,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 105, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, recycle, range, count, hspread, vspread, kick, magic_cost
			14.0, 0.32, 1.0, 2.0, 450.0, 4.0, 430.0, 120.0, -45.0, 45.0},
		{"Commando Vest", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 45, {22,28,32,26,1}, {4,20,10,20,5}, 5, /*185*/ \
			{27,	31,		23,		1,		24,		32,		28,		0,		0,		0}, //putypes
			{-10,	-5,		-14,	-16,	-3,		-10,	-15,	0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{69, 0, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			75.0, 10.0, 0.35, 0.05, 0.0, 2.5, 0.0, 0.0, 0.0, 0.0},
		{"Unholy Armor", GIEX_ITEM_ARMOR, NULL, 0, "models/objects/gibs/chest/tris.md2", 1, {34,26,1,73,0}, {10,20,15,3,0}, 3, /*186*/ \
			{34,	26,	30,	1,		0,		0,		0,		0,		0,		0}, //putypes
			{0,	-10,	-10,	-20,	0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 0, 90, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			0.0, 0.0, 0.0, 0.00, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Paladin's Platemail", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 1, {26,41,66,29,22}, {45,13,10,30,4}, 4, /*187*/ \
			{1,		24,		34,		29,		33,		0,		0,		0,		0,		0}, //putypes
			{-20,	0,		0,		20,		-10,	0,		0,		0,		0, 	0}, //pumaxlvl
			{1.5,	1.0,	2.0,	0.75,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 94, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			70.0, 4.0, 0.35, 0.01, 1.0, 3.0, 0.0, 0.0, 0.0, 0.0},
		{"Shadow Coat", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 150, {29,33,34,1,0}, {15,5,5,5,0}, 2, /*188*/ \
			{29,	33,	 0,	 0,	 0,	0,	0,		0,		0,		0}, //putypes
			{-25,	-15,	 0,	 0,	 0,	0,		0, 	0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 35, 32, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			30.0, 0.0, 0.35, 0.018, 3.0, 0.7, 0.0, 0.0, 0.0, 0.0},
		{"Protector Helmet", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 1, {46,36,37,38,0}, {22,5,5,5,0}, 1, /*189*/ \
			{30,	31,		0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 89, 99, 0, 0, 0, 0, 0}, //classreq
//armor, +armor
			100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Improved Jetpack", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 35, {49,74,0,0,0}, {10,10,0,0,0}, 3, /*190*/ \
			{29,	33,		25,		65,		0,		0,		0,		0,		0,		0}, //putypes
			{-10,	-5,		0,		-25,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 55, 70, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			60.0, 0.0, 0.6, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Assault Armor mk IV", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 1, {1,2,20,22,24}, {10,10,10,5,10}, 4, /*191*/ \
			{1,		27,		31,		32,		24,		34,		0,		0,		0,		0}, //putypes
			{-10,	0,		0,		0,		0,		-10,		0,		0,		0,		0}, //pumaxlvl
			{1.8,	1.2,	1.2,	1.2,	1.9,	2.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{95, 0, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			180.0, 8.0, 0.4, 0.02, 3.0, 0.7, 0.0, 0.0, 0.0, 0.0},
		{"Viking helm", GIEX_ITEM_HELMET, NULL, 0, "models/monsters/commandr/head/tris.md2", 10, {26,30,18,0,0}, {45,10,18,0,0}, 2, /*192*/ \
			{26,	30,	0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{ 0,	 0,	0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{70, 85, 0, 0, 76, 0, 0, 0}, //classreq
//armor, +armor
			50.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Knight Armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 20, {26,66,22,0,0}, {35,5,4,0,0}, 3, /*193*/ \
			{  1,	 24,	29,	 33,	 0,	0,		0,		0,		0,		0}, //putypes
			{-25,	 -5,	-10,  -10,	 0,	0,		0,		0,		0, 	0}, //pumaxlvl
			{1.5,	1.0,	2.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 64, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			50.0, 4.0, 0.35, 0.01, 1.0, 3.0, 0.0, 0.0, 0.0, 0.0},
		{"Squire's Vest", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 150, {26,22,0,0,0}, {15,3,0,0,0}, 2, /*194*/ \
			{  1,	  29,		33,	 0,	0,		0,		0,		0,		0,		0}, //putypes
			{-30,  -15,   -15,	 0,	0,		0,		0,		0,		0, 	0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 34, 0, 42, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			40.0, 4.0, 0.30, 0.01, 1.0, 3.0, 0.0, 0.0, 0.0, 0.0},
		{"Beam blaster", GIEX_ITEM_WEAPON | GIEX_ITEM_TANK, NULL, CI_W_HYPERBLASTER, "models/weapons/g_hyperb/tris.md2", 250, {0,0,0,0,0}, {0,0,0,0,0}, 3,/*195*/\
			{1,		7,		72,		59,		75,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		-30,	-30,	-30,	0,		0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 17, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, boltfiredelay (1-3, or 6), notused, notused, speed, +speed, ammouse, +ammouse
			9.0, 0.2, 1.0, 0.0, 0.0, 800.0, 35.0, 0.0, 0.06, 0.0},
		{"Disruptor", GIEX_ITEM_WEAPON | GIEX_ITEM_TANK, NULL, CI_W_CHAINGUN, "models/weapons/g_chain/tris.md2", 250, {0,0,0,0,0}, {0,0,0,0,0}, 3,/*196*/\
			{1,		5,		72,		59,		75,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		-30,	-30,	-30,	0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 16, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, notused, numshots, addspread
			4.4, 0.095, 0.0, 1.0, 300.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Pulse cannon", GIEX_ITEM_WEAPON | GIEX_ITEM_TANK, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 102, {0,0,0,0,0}, {0,0,0,0,0}, 3, /*197*/\
			{1,		2,		72,		59,		75,		0,		0,		0,		0,		0}, //putypes
			{20,	0,		-20,	-20,	-20,	0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 34, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			70.0, 1.2, 2.0, 40.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Laser blaster", GIEX_ITEM_WEAPON | GIEX_ITEM_TANK, NULL, CI_W_HYPERBLASTER, "models/weapons/g_hyperb/tris.md2", 105, {0,0,0,0,0}, {0,0,0,0,0}, 3,/*198*/\
			{1,		7,		2,		72,		59,		75,		0,		0,		0,		0}, //putypes
			{0,		0,		0,		-20,	-20,	-20,	0,		0,		0,		0}, //pumaxlvl
			{1.4,	1.2,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 45, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, boltfiredelay (1-3), notused, notused, speed, +speed, ammouse, +ammouse
			12.0, 0.225, 1.0, 0.0, 0.0, 600.0, 10.0, 3.5, 0.15, 0.0},
		{"Radiation Disperser", GIEX_ITEM_WEAPON | GIEX_ITEM_TANK, NULL, CI_W_BFG, "models/weapons/g_bfg/tris.md2", 10, {0,0,0,0,0}, {0,0,0,0,0}, 3, /*199*/\
			{1,		7,		72,		59,		75,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		-10,	-10,	-10,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.4,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 60, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, *dmg_lvl, +dmg_charge, speed, +speed, -speed_charge, *ammouse
			50.0, 0.02,  9.5, 550.0, 20.0, 30.0, 3.2, 4.0, 0.0, 0.0},
		{"Fusion Launcher", GIEX_ITEM_WEAPON | GIEX_ITEM_TANK, NULL, CI_W_ROCKETLAUNCHER, "models/weapons/g_rocket/tris.md2", 12, {0,0,0,0,0}, {0,0,0,0,0}, 3,/*200*/\
			{1,		2,		7,		72,		59,		75,		0,		0,		0,		0}, //putypes
			{10,	0,		0,		-10,	-10,	-10,	0,		0,		0,		0}, //pumaxlvl
			{1.5,	1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 61, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, +radius, speed, +speed
			110.0, 1.4, 0.0, 110.0, 0.08, 750.0, 15.0, 0.0, 0.0, 0.0},
		{"Derailer", GIEX_ITEM_WEAPON | GIEX_ITEM_TANK, NULL, CI_W_RAILGUN, "models/weapons/g_rail/tris.md2", 1, {2,0,0,0,0}, {5,0,0,0,0}, 3, /*201*/\
			{1,		2,		72,		59,		75,		0,		0,		0,		0,		0}, //putypes
			{20,	0,		5,		5,		5,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 92, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, lvl_for_bonus_dmg, +bonusdmg_per_lvl
			130.0, 2.0, 3.0, 45.0, 1.3, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Howitzer", GIEX_ITEM_WEAPON | GIEX_ITEM_TANK, NULL, CI_W_GRENADELAUNCHER, "models/weapons/g_launch/tris.md2", 1, {80,0,0,0,0}, {20,0,0,0,0}, 2,/*202*/\
			{1,		72,		59,		75,		0,		0,		0,		0,		0,		0}, //putypes
			{10,	5,		5,		5,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.6,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 93, 0, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, radius, gravity, speed, defaulttimer, numbounces, timer, +timer
			120.0, 1.76, -1.0, 140.0, 0.4, 690.0, 5.0, 1.0, 0.1, 0.0},
		{"War Staff", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 30, {29,25,33,65,0}, {20,10,10,12,0}, 3, /*203*/\
			{29,	25,		65,		1,		 77,	0,		0,		0,		0,		0}, //putypes
			{ 0,	5,		-10,	0,		-10,	0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 58, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			150.0, 6.0, 4.0, 0.5, 0.0, 0.0, 0.0, 27.0, 0.4, 0.0},
		{"Battlemage's Armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 1, {29,33,65,22,0}, {50,15,5,8,0}, 4, /*204*/ \
			{29,	33,		27,		31, 	24,			1,  		28,			32,			0,		0}, //putypes
			{10,	 0,		-25,	0,		0,			-20,		0,			0,			0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,		1.0,		1.0,		1.0,		1.0,	1.0}, //pucost
			{0, 0, 0, 100, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			150.0, 10.0, 0.5, 0.02, 6.0, 0.5, 0.0, 0.0, 0.0, 0.0},
		{"Disc of Life Drain", GIEX_ITEM_BACKPACK, NULL, 0, "models/items/keys/data_cd/tris.md2", 5, {0,0,0,0,0}, {0,0,0,0,0}, 1, /*205*/ \
			{ 60,	  0,	 0,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{  0,	  0,	 0,		0,	 	0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 77, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
		{"Beam armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/jacket/tris.md2", 256, {22,0,0,0,0}, {3,0,0,0,0}, 4, /*206*/ \
			{27,	 29,	33,		72,		59,		75,		23,		44,		0,		0}, //putypes
			{ 0,	-25,	-12,	-30,	-30,	-30,	0,		-10,	0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 17, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			150.0, 4.0, 0.5, 0.02, 3.0, 0.5, 0.0, 0.0, 0.0, 0.0},
		{"Energy armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 111, {22,0,0,0,0}, {4,0,0,0,0}, 4, /*207*/ \
			{27,	29,		33,		72,		59,		75,		23,		44,		0,		0}, //putypes
			{ 0,	-20,	-10,	-20,	-20,	-20,	0,		-5,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 37, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			280.0, 5.0, 0.6, 0.02, 4.0, 0.6, 0.0, 0.0, 0.0, 0.0},
		{"Ion armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 33, {22,0,0,0,0}, {5,0,0,0,0}, 4, /*208*/ \
			{27,	29,		33,		72,		59,		75,		23,		44,		0,		0}, //putypes
			{ 0,	-10,	-5,		-10,	-10,	-10,	0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 66, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			440.0, 5.0, 0.7, 0.025, 4.0, 0.8, 0.0, 0.0, 0.0, 0.0},
		{"Nano armor", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/body/tris.md2", 1, {70,22,0,0,0}, {3,5,0,0,0}, 4, /*209*/ \
			{27,	29,		33,		72,		59,		75,		23,		44,		0,		0}, //putypes
			{ 0,	10,		0,		0,		0,		0,		0,		5,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 93, 0, 0, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			580.0, 5.0, 0.8, 0.03, 5.0, 1.0, 0.0, 0.0, 0.0, 0.0},
		{"MP5", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 125, {58,0,0,0,0}, {4,0,0,0,0}, 2,/*210*/\
			{1,		2,		4,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		3,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.3,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{40, 54, 34, 0, 50, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			14.0, 0.25, 0.0, 320.0, 370.0, 23.0, 27.0, 0.0, 0.0, 0.0},
		{"UMP-45", GIEX_ITEM_WEAPON | GIEX_ITEM_SOLDIER | GIEX_ITEM_CLERIC, NULL, CI_W_MACHINEGUN, "models/weapons/g_machn/tris.md2", 1, {58,0,0,0,0}, {10,0,0,0,0}, 2,/*211*/\
			{1,		2,		4,		0,		0,		0,		0,		0,		0,		0}, //putypes
			{0,		0,		3,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{2.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{95, 0, 90, 0, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, hspread, vspread, -hspread/lvl, -vspread/lvl
			17.0, 0.3, 0.0, 300.0, 350.0, 25.0, 30.0, 0.0, 0.0, 0.0},
		{"Corpse staff", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 25, {54,25,83,61,76}, {10,10,2,10,10}, 4, /*212*/\
			{29,	25,		54,		55,		61,		76,		34,		0,		0,		0}, //putypes
			{0,		0,		-10,	0,		-10,	0,		-10,	0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 60, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			150.0, 0.0, -1.0, 0.3, 0.0, 1250.0, 0.0, 16.0, 0.5, 0.0},
		{"Staff of Necrosis", GIEX_ITEM_WEAPON | GIEX_ITEM_MAGE, NULL, CI_W_BLASTER, "models/weapons/g_blast/tris.md2", 1, {54,25,83,61,76}, {15,15,4,15,15}, 4, /*213*/\
			{29,	25,		54,		55,		61,		76,		0,		0,		0,		0}, //putypes
			{10,	10,		0,		0,		0,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 89, 0, 0, 0, 0}, //classreq
//dmg, +dmg, firerate, pierce, +pierce, speed, +speed, ammouse, +ammouse
			105.0, 0.0, -2.0, 0.3, 0.0, 1250.0, 0.0, 25.0, 0.5, 0.0},
		{"Cloak of Necrosis", GIEX_ITEM_ARMOR, NULL, 0, "models/items/armor/combat/tris.md2", 1, {54,33,83,61,76}, {5,10,4,5,5}, 4, /*214*/ \
			{27,	29,		23,		31,		33,		55,		61,		76,		54,		0}, //putypes
			{-15,	0,		0,		0,		10,		0,		0,		0,		0,		0}, //pumaxlvl
			{1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0,	1.0}, //pucost
			{0, 0, 0, 85, 0, 0, 0, 0}, //classreq
//armor, +armor, absorb, +absorb, regen, +regen
			200.0, 8.0, 0.35, 0.019, 2.5, 0.7, 0.0, 0.0, 0.0, 0.0},
  	};
	iteminfo_t *iteminfo = &(info[num]);
	if (num < 0)
		iteminfo = &(info[0]);
	if ((iteminfo->item == NULL) && (iteminfo->itemid != 0)) {
		iteminfo->item = FindItemByClassid(iteminfo->itemid);
	}
	return iteminfo;
}
#define GIEX_NUMITEMS 214

void giexitem_make_touchable (edict_t *item);
void giexitem_make_touchable2 (edict_t *item);
void giexitem_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
	iteminfo_t *info;
	int i;

	if (!other || !other->client)
		return;

	if (other->client->id_powerup > level.time) {
		info = getItemInfo(ent->radius_dmg);
		gi.cprintf(other, PRINT_HIGH, "A %s\n", info->name);

		// So powerups won't be id'd several times at once
		ent->touch = NULL;
		if (ent->owner && (ent->owner != other)) {
			ent->think = giexitem_make_touchable;
			ent->nextthink = level.time + 0.2;
		} else {
			ent->think = giexitem_make_touchable2;
			ent->nextthink = level.time + 0.2;
		}
		return;
	}

	if (ent->owner && (ent->owner != other))
		return;
	if (other->client->get_powerup < level.time)
		return;

	info = getItemInfo(ent->radius_dmg);
	for (i = 0; i < GIEX_ITEMPERCHAR; i++) {
		if (other->client->pers.skills.itemId[i] == 0) {
			int pu;
			other->client->pers.skills.itemId[i] = ent->radius_dmg;
			for (pu = 0; pu < GIEX_PUPERITEM; pu++) {
				other->client->pers.skills.item_putype[i][pu] = ent->itempuinfo.putype[pu];
				other->client->pers.skills.item_pumaxlvl[i][pu] = ent->itempuinfo.pumaxlvl[pu];
				other->client->pers.skills.item_pucurlvl[i][pu] = ent->itempuinfo.pucurlvl[pu];
				other->client->pers.skills.item_puexp[i][pu] = ent->itempuinfo.puexp[pu];
			}
			gi.sound(ent, CHAN_VOICE, gi.soundindex("giex/puppkup.wav"), 1, ATTN_NORM, 0);
			G_FreeEdict(ent);
			gi.cprintf(other, PRINT_HIGH, "Picked up the %s\n", info->name);
			return;
		}
	}
	if (ent->pain_debounce_time < level.time) {
		gi.cprintf(other, PRINT_HIGH, "Inventory full!\n");
		ent->pain_debounce_time = level.time + 1;
	}
}

void giexitem_make_touchable2 (edict_t *item) {
	item->s.effects &= ~EF_COLOR_SHELL;
	item->s.renderfx &= ~(RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE);
	item->owner = NULL;
	item->touch = giexitem_touch;
	item->think = G_FreeEdict;
	item->nextthink = level.time + 60;
}

void giexitem_make_touchable (edict_t *item) {
	iteminfo_t *info = getItemInfo(item->radius_dmg);

	item->s.effects |= EF_COLOR_SHELL;
	item->s.renderfx |= (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE);
	item->touch = giexitem_touch;
	item->think = giexitem_make_touchable2;
	item->nextthink = level.time + 30 + (30 - (info->rarity / 33));
}

int getRealRarity(iteminfo_t *info, float mult1, float mult2, float mult3, float mult4) {
	if (info->rarity < 100) {
		if (info->rarity < 6) {
			return info->rarity * mult1 * GIEX_ITEMDROPS_MULTGOD;
		} else {
			return info->rarity * mult1 * GIEX_ITEMDROPS_MULT1;
		}
	} else if (info->rarity < 250) {
		return info->rarity * mult2 * GIEX_ITEMDROPS_MULT2;
	} else if (info->rarity < 500) {
		return info->rarity * mult3 * GIEX_ITEMDROPS_MULT3;
	}
	return info->rarity * mult4 * GIEX_ITEMDROPS_MULT4;
}
void spawnItem(edict_t *targ, edict_t *attacker) {
	int total_rarity = 0;
	iteminfo_t *info;
	edict_t *item;
	int spawn_num, item_id = 0, i;
	float mult1, mult2, mult3, mult4;

	if (game.monsterhunt != 10) { //In monster hunt, always spawn items
		if (!attacker->client) {
			return;
		}
		if (targ == attacker) {
			return;
		}
	} else { // Never spawn items for player kills in Monster hunt
		if (targ->client)
			return;
	}

	if (targ->svflags & SVF_MONSTER) {
//		gi.dprintf("%f %f\n", random(), targ->monsterinfo.item_any);
		if (random() > (targ->monsterinfo.item_any + 0.005 * targ->monsterinfo.skill) * (1 + coop->value)) {
			return;
		}
		mult1 = pow(targ->monsterinfo.itemmult_class1, (1 + (float) targ->monsterinfo.skill / 8.0));
		mult2 = pow(targ->monsterinfo.itemmult_class2, (1 + (float) targ->monsterinfo.skill / 8.0));
		mult3 = pow(targ->monsterinfo.itemmult_class3, (1 + (float) targ->monsterinfo.skill / 8.0));
		mult4 = pow(targ->monsterinfo.itemmult_class4, (1 + (float) targ->monsterinfo.skill / 8.0));

	} else if (targ->client && attacker->client) {
		if ((coop->value) || ((teams->value) && (attacker->count == targ->count))) {
			return;
		}
		if (random() > ((float) targ->radius_dmg / 120.0)) {
			return;
		}
		mult1 = pow(1.4, (1 + (float) targ->radius_dmg / 12.0));
		mult2 = pow(0.9999, (1 + (float) targ->radius_dmg / 12.0));
		mult3 = pow(0.95, (1 + (float) targ->radius_dmg / 12.0));
		mult4 = pow(0.9, (1 + (float) targ->radius_dmg / 12.0));
	} else {
		return;
	}
	if (mult1 > 250)
		mult1 = 250;
	for (i = 0; i <= GIEX_NUMITEMS; i++) {
		info = getItemInfo(i);
		if (info->rarity > 0) {
			total_rarity += getRealRarity(info, mult1, mult2, mult3, mult4);
		}
	}

	spawn_num = ceil(random() * total_rarity);
	//gi.dprintf("%d %d (%f %f %f %f)\n", spawn_num, total_rarity, mult1, mult2, mult3, mult4);
	for (i = 0; i <= GIEX_NUMITEMS; i++) {
		info = getItemInfo(i);
		if (info->rarity > 0) {
			if (spawn_num <= getRealRarity(info, mult1, mult2, mult3, mult4)) {
				item_id = i;
				break;
			}
			spawn_num -= getRealRarity(info, mult1, mult2, mult3, mult4);
		}
	}
	if (game.monsterhunt != 10) {
		if (targ->svflags & SVF_MONSTER) {
			gi.cprintf(attacker, PRINT_HIGH, "%s dropped a %s!\n", targ->monsterinfo.name, info->name);
		} else {
			gi.cprintf(attacker, PRINT_HIGH, "%s dropped a %s!\n", targ->client->pers.netname, info->name);
		}
	}

	item = G_Spawn();
	item->classid = CI_GIEX_ITEM;
	item->classname = "giexitem";
	VectorCopy (targ->s.origin, item->s.origin);
	VectorCopy (targ->s.angles, item->s.angles);
	VectorSet (item->mins, -15, -15, -15);
	VectorSet (item->maxs, 15, 15, 15);
	item->s.effects |= EF_ROTATE;
	item->velocity[0] = 175.0 * crandom();
	item->velocity[1] = 175.0 * crandom();
	item->velocity[2] = 275.0 + 75.0 * random();
	if (game.monsterhunt == 10) { // Spread out extra in Monster hunt
		item->velocity[0] *= 2;
		item->velocity[1] *= 2;
		item->velocity[2] *= 2;
	}
	item->movetype = MOVETYPE_TOSS;
	gi.setmodel (item, info->modelname);
	item->nextthink = level.time + 1;
	item->solid = SOLID_TRIGGER;
	item->radius_dmg = item_id;
	if (game.monsterhunt != 10) {
		item->owner = attacker;
		item->think = giexitem_make_touchable;
	} else {
		item->owner = NULL;
		item->think = giexitem_make_touchable2;
	}
	gi.linkentity(item);
}

int getWornItem(edict_t *ent, int basetype) {
	return ent->client->pers.skills.wornItem[basetype];
}
iteminfo_t *getWornItemInfo(edict_t *ent, int basetype) {
	if (ent->client->pers.skills.wornItem[basetype] == -1)
		return getItemInfo(0);
	return getItemInfo(ent->client->pers.skills.itemId[(int) ent->client->pers.skills.wornItem[basetype]]);
}

int getItemPowerupLevel(edict_t *ent, int item, int powerup) {
	int pu;
	for (pu = 0; pu < GIEX_PUPERITEM; pu++) {
		if (ent->client->pers.skills.item_putype[item][pu] == 0)
			break;
		if (ent->client->pers.skills.item_putype[item][pu] == powerup)
			return ent->client->pers.skills.item_pucurlvl[item][pu];
	}
	return 0;
}
int getWornItemPowerupLevel(edict_t *ent, int basetype, int powerup) {
	if (ent->client->pers.skills.wornItem[basetype] == -1) {
		return 0;
	}
	return getItemPowerupLevel(ent, ent->client->pers.skills.wornItem[basetype], powerup);
}


//returns 0 for no requirements, -1 if classId may not wear this item!
int getItemBaseLevelReq(edict_t *ent, int item, int classId) {
	iteminfo_t *info = getItemInfo(ent->client->pers.skills.itemId[item]);
	int i;
	qboolean reqs = false;

	for (i = 0; i < GIEX_NUMCLASSES; i++) {
		if (info->classreq[i] > 0) {
			reqs = true;
			break;
		}
	}
	if (reqs && (info->classreq[classId] == 0)) {
		return -1;
	}
	return info->classreq[classId];
}
int getItemLevelReq(edict_t *ent, int item, int classId) {
	iteminfo_t *info = getItemInfo(ent->client->pers.skills.itemId[item]);
	int req = info->classreq[classId];
	int pu, i;
	qboolean reqs = false;

	for (i = 0; i < GIEX_NUMCLASSES; i++) {
		if (info->classreq[i] > 0) {
			reqs = true;
			break;
		}
	}
	if (reqs && (info->classreq[classId] == 0)) {
		return -1;
	}

	reqs = false;
	for (pu = 0; pu < GIEX_PUPERITEM; pu++) {
		powerupinfo_t *puinfo;
		if (ent->client->pers.skills.item_putype[item][pu] == 0) {
			break;
		}
		puinfo = getPowerupInfo(ent->client->pers.skills.item_putype[item][pu]);
		for (i = 0; i < GIEX_NUMCLASSES; i++) {
			if ((puinfo->classreq[classId] + ceil(puinfo->reqperlvl[classId] * ent->client->pers.skills.item_pucurlvl[item][pu])) > 0) {
				reqs = true;
				break;
			}
		}
//TODO: This is bugged and maybe not even necessary, since no powerups are totally prohibited to any class
/*		if (reqs && ((puinfo->classreq[classId] + ceil(puinfo->reqperlvl[classId] * ent->client->pers.skills.item_pucurlvl[item][pu])) == 0)) {
			return -1;
		}*/
		req += puinfo->classreq[classId] + ceil(puinfo->reqperlvl[classId] * ent->client->pers.skills.item_pucurlvl[item][pu]);
	}
	return req;
}
int getWornItemLevelReq(edict_t *ent, int basetype, int classId) {
	if (ent->client->pers.skills.wornItem[basetype] == -1)
		return 0;
	return getItemLevelReq(ent, (int) ent->client->pers.skills.wornItem[basetype], classId);
}

void restructureItems(edict_t *ent) {
	int start, i, p;
	for (start = 0; start < GIEX_ITEMPERCHAR; start++) {
		if (ent->client->pers.skills.itemId[start] == 0) {
			break;
		}
	}
	if (start > GIEX_ITEMPERCHAR -1) {
		return;
	}
	for (i = start + 1; i < GIEX_ITEMPERCHAR; i++) {
		iteminfo_t *info = getItemInfo(ent->client->pers.skills.itemId[i]);
		if (ent->client->pers.skills.wornItem[info->type & 255] == i) {
			ent->client->pers.skills.wornItem[info->type & 255] = i - 1;
		}
		ent->client->pers.skills.itemId[i - 1] = ent->client->pers.skills.itemId[i];
		for (p = 0; p < GIEX_PUPERITEM; p++) {
			ent->client->pers.skills.item_putype[i - 1][p] = ent->client->pers.skills.item_putype[i][p];
			ent->client->pers.skills.item_pumaxlvl[i - 1][p] = ent->client->pers.skills.item_pumaxlvl[i][p];
			ent->client->pers.skills.item_pucurlvl[i - 1][p] = ent->client->pers.skills.item_pucurlvl[i][p];
			ent->client->pers.skills.item_puexp[i - 1][p] = ent->client->pers.skills.item_puexp[i][p];
		}
	}
	ent->client->pers.skills.itemId[GIEX_ITEMPERCHAR - 1] = 0;
	for (p = 0; p < GIEX_PUPERITEM; p++) {
		ent->client->pers.skills.item_putype[GIEX_ITEMPERCHAR - 1][p] = 0;
		ent->client->pers.skills.item_pumaxlvl[GIEX_ITEMPERCHAR - 1][p] = 0;
		ent->client->pers.skills.item_pucurlvl[GIEX_ITEMPERCHAR - 1][p] = 0;
		ent->client->pers.skills.item_puexp[GIEX_ITEMPERCHAR - 1][p] = 0;
	}
}

int getItemPowerupInfoSlot(iteminfo_t *info, int putype) {
	int i;
	for (i = 0; i < GIEX_PUPERITEM; i++) {
		if (info->putype[i] == putype) {
			return i;
		}
	}
	return -1;
}
int getItemSocketedCount(edict_t *ent, int item) {
	int i;
	for (i = 0; i < GIEX_PUPERITEM; i++) {
		if (ent->client->pers.skills.item_putype[item][i] == 0) {
			return i + 1;
		}
	}
	return i + 1;
}

int getPowerupLevel(edict_t *ent, int powerup) {
	int i, item, sum = 0;
	for (item = 0; item < GIEX_BASEITEMS; item++) {
		iteminfo_t *info = getItemInfo(ent->client->pers.skills.itemId[item]);
		for (i = 0; i < GIEX_AUTOPERITEM; i++) {
			if (info->auto_putype[i] == 0) {
				break;
			}
			if (info->auto_putype[i] == powerup) {
				sum += info->auto_pulvl[i];
				break;
			}
		}
		for (i = 0; i < GIEX_PUPERITEM; i++) {
			if (ent->client->pers.skills.item_putype[item][i] == 0) {
				break;
			}
			if (ent->client->pers.skills.item_putype[item][i] == powerup) {
				sum += ent->client->pers.skills.item_pumaxlvl[item][i];
				break;
			}
		}
	}
	return sum;
}

void updateItemLevels(edict_t *ent) {
	int i, item, add = 0;
	powerupinfo_t *puinfo;
	skills_t *skills = &ent->client->pers.skills;
	for (i = 0; i < GIEX_PUTYPES; i++) {
		ent->client->pers.skill[i] = 0;
	}

	for (i = 0; i < GIEX_PUPERCHAR; i++) {
/*		puinfo = getPowerupInfo(skills->putype[i]);
		add = 0;
		for (c = 0; c < GIEX_NUMCLASSES; c++) {
			if ((puinfo->classreq[c] == 0) || (puinfo->classreq[c] > skills->classLevel[c])) {
				//break;
			} else {
				if (puinfo->classreq[c] + puinfo->reqperlvl[c] * add > skills->classLevel[c]) {
					add = ((float) (skills->pucurlvl[i] - puinfo->classreq[c])) / puinfo->reqperlvl[c];
				} else {
					add = skills->pucurlvl[i];
					break;
				}
			}
		}*/
		if (skills->putype[i] == 0) {
			break;
		}
		ent->client->pers.skill[skills->putype[i]] += skills->pucurlvl[i]; //skills->pucurlvl[i];
	}

	for (item = 0; item < GIEX_BASEITEMS; item++) {
//		gi.dprintf("%d: %d\n", item, (int) skills->wornItem[item]);
		iteminfo_t *info = getWornItemInfo(ent, item);
//		iteminfo_t *info = getItemInfo(skills->itemId[(int) skills->wornItem[item]]);
		if (skills->wornItem[item] == -1) {
			continue;
		}
		for (i = 0; i < GIEX_AUTOPERITEM; i++) {
			if (info->auto_putype[i] == 0) {
				break;
			}
			ent->client->pers.skill[info->auto_putype[i]] += info->auto_pulvl[i];
		}
		for (i = 0; i < GIEX_PUPERITEM; i++) {
//			int newadd = 0;
//			qboolean set = false;
			if (skills->item_putype[(int) skills->wornItem[item]][i] == 0) {
				break;
			}
			puinfo = getPowerupInfo(skills->item_putype[(int) skills->wornItem[item]][i]);
			add = skills->item_pucurlvl[(int) skills->wornItem[item]][i];
/*			for (c = 0; c < GIEX_NUMCLASSES; c++) {
				if (puinfo->classreq[c] > skills->classLevel[c]) {
					if (!set) {
						add = 0;
					}
					continue;
				}
				set = true;
				if (puinfo->classreq[c] + puinfo->reqperlvl[c] * add > skills->classLevel[c]) {
					newadd = ((float) (skills->classLevel[c] - puinfo->classreq[c])) / puinfo->reqperlvl[c];
					gi.dprintf("%d %d %f %d\n", c, (skills->classLevel[c] - puinfo->classreq[c]), puinfo->reqperlvl[c], add);
				}
			}*/
			ent->client->pers.skill[skills->item_putype[(int) skills->wornItem[item]][i]] += add;
		}
	}
	ent->client->pers.skill[49]++; // Flight for everyone!

	if (ent->client->pers.skills.classLevel[4] > 0) { //Has atleast one Vampire level
		int lvl = ent->client->pers.skills.classLevel[4];
		ent->client->pers.skill[73] += (int) (0.6 * lvl); //Bite
		ent->client->pers.skill[74] += (int) (0.4 * lvl); //Rush
	}

	ApplyMax(ent);
}


void Cmd_listItems(edict_t *ent) {
	int i, p;
	iteminfo_t *info;
	powerupinfo_t *pinfo;
	for (i = 0; i < GIEX_ITEMPERCHAR; i++) {
		info = getItemInfo(ent->client->pers.skills.itemId[i]);
		if (info->name[0] != '\0') {
			gi.cprintf(ent, PRINT_HIGH, "%d: %s %d", i, info->name, ent->client->pers.skills.itemId[i]);
			if (ent->client->pers.skills.wornItem[info->type & 255] == i) {
				gi.cprintf(ent, PRINT_HIGH, " (equipped)\n");
			} else {
				gi.cprintf(ent, PRINT_HIGH, "\n");
			}
		}
		for (p = 0; p < GIEX_PUPERITEM; p++) {
			if (ent->client->pers.skills.item_putype[i][p] == 0) {
				break;
			}
			pinfo = getPowerupInfo(ent->client->pers.skills.item_putype[i][p]);
			gi.cprintf(ent, PRINT_HIGH, "   %s (%d/%d)\n", pinfo->name, ent->client->pers.skills.item_pucurlvl[i][p], ent->client->pers.skills.item_pumaxlvl[i][p]);
		}
	}
}

void Cmd_wornItems(edict_t *ent) {
	int i;
	iteminfo_t *info;
	for (i = 0; i < GIEX_BASEITEMS; i++) {
		info = getWornItemInfo(ent, i);
		if (info->name[0] != '\0') {
			gi.cprintf(ent, PRINT_HIGH, "%s: %s\n", getBaseTypeName(i), info->name[0] != '\0' ? info->name : "Empty");
		}
	}
}

qboolean canWearItem(edict_t *ent, int item) {
	qboolean eq = true;
	int c;
	iteminfo_t *info = getItemInfo(ent->client->pers.skills.itemId[item]);
	for (c = 0; c < GIEX_NUMCLASSES; c++) {
		int req = getItemLevelReq(ent, item, c);

		if (req > 0) {
			if (req <= ent->client->pers.skills.classLevel[c]) {
				eq = true;
				break;
			}
			if (req > ent->client->pers.skills.classLevel[c]) {
				eq = false;
			}
		} else if (req == -1) {
			eq = false;
		}
	}
	return eq;
}

void showItemInfo(edict_t *ent, int item) {
	iteminfo_t *info = getItemInfo(ent->client->pers.skills.itemId[item]);
	powerupinfo_t *pinfo;
	int i, type;

	gi.cprintf(ent, PRINT_HIGH, "\n\n==============================\n%s\n", info->name);

//Stats
	type = info->type & 15;
	if (type == GIEX_ITEM_WEAPON) {
		if (info->itemid == CI_W_BLASTER) {
			int pierce = getBlasterPierce(ent, item, info);
			gi.cprintf(ent, PRINT_HIGH, "Damage: %d\n", getBlasterDamage(ent, item, info));
			gi.cprintf(ent, PRINT_HIGH, "Firerate: %f\n", 10.0 / getBlasterFirerate(ent, item, info));
			if (pierce > 0) {
				gi.cprintf(ent, PRINT_HIGH, "Pierce: %d\n", pierce);
			}
			gi.cprintf(ent, PRINT_HIGH, "Flight speed: %d\n", getBlasterSpeed(ent, item, info));
			gi.cprintf(ent, PRINT_HIGH, "Ammo usage: %d\n", getBlasterConsumption(ent, item, info));
		} else if (info->itemid == CI_W_SHOTGUN) {
			int pierce = getWeaponPierce(ent, item);
			gi.cprintf(ent, PRINT_HIGH, "Damage: %d\n", getShotgunTotalDamage(ent, item, info));
			gi.cprintf(ent, PRINT_HIGH, "Firerate: %f\n", 10.0 / getShotgunFirerate(ent, item, info));
			if (pierce > 0) {
				gi.cprintf(ent, PRINT_HIGH, "Pierce: %d\n", pierce);
			}
			gi.cprintf(ent, PRINT_HIGH, "Pellet count: %d\n", getShotgunCount(ent, item, info));
		} else if (info->itemid == CI_W_SUPERSHOTGUN) {
			int pierce = getWeaponPierce(ent, item);
			gi.cprintf(ent, PRINT_HIGH, "Damage: %d\n", getSuperShotgunTotalDamage(ent, item, info));
			gi.cprintf(ent, PRINT_HIGH, "Firerate: %f\n", 10.0 / getSuperShotgunFirerate(ent, item, info));
			if (pierce > 0) {
				gi.cprintf(ent, PRINT_HIGH, "Pierce: %d\n", pierce);
			}
			gi.cprintf(ent, PRINT_HIGH, "Pellet count: %d\n", getSuperShotgunCount(ent, item, info));
			gi.cprintf(ent, PRINT_HIGH, "Barrel delay: %d\n", getSuperShotgunBarrelDelay(ent, item, info));
		} else if (info->itemid == CI_W_MACHINEGUN) {
			int pierce = getWeaponPierce(ent, item);
			gi.cprintf(ent, PRINT_HIGH, "Damage: %d\n", getMachinegunDamage(ent, item, info));
			gi.cprintf(ent, PRINT_HIGH, "Firerate: %f\n", 10.0 / getMachinegunFirerate(ent, item, info));
			if (pierce > 0) {
				gi.cprintf(ent, PRINT_HIGH, "Pierce: %d\n", pierce);
			}
			gi.cprintf(ent, PRINT_HIGH, "Spread: %d\n", getMachinegunSpread(ent, item, info));
		} else if (info->itemid == CI_W_CHAINGUN) {
			gi.cprintf(ent, PRINT_HIGH, "Damage: %d\n", getChaingunTotalDamage(ent, item, info));
			gi.cprintf(ent, PRINT_HIGH, "Spread: %d\n", getChaingunSpread(ent, item, info));
		} else if (info->itemid == CI_AM_GRENADES) {
		} else if (info->itemid == CI_W_GRENADELAUNCHER) {
		} else if (info->itemid == CI_W_ROCKETLAUNCHER) {
		} else if (info->itemid == CI_W_HYPERBLASTER) {
		} else if (info->itemid == CI_W_RAILGUN) {
		} else if (info->itemid == CI_W_BFG) {
		} else if (info->itemid == CI_W_ARM) {
		} else {
			gi.cprintf(ent, PRINT_HIGH, "Unknown weapon type! Please report this bug.\n");
		}
	} else if (info->type & GIEX_ITEM_ARMOR) {
		gi.cprintf(ent, PRINT_HIGH, "Armor: %d\n", (int) (info->arg1 + info->arg2 * getItemPowerupLevel(ent, item, 27)));
		gi.cprintf(ent, PRINT_HIGH, "Absorbtion: %d\n", (int) (info->arg3 + info->arg4 * getItemPowerupLevel(ent, item, 23)));
		gi.cprintf(ent, PRINT_HIGH, "Regeneration: %d\n", (int) (info->arg5 + info->arg6 * getItemPowerupLevel(ent, item, 31)));
	} else if ((info->type & GIEX_ITEM_HELMET) || (info->type & GIEX_ITEM_BACKPACK)) {
		gi.cprintf(ent, PRINT_HIGH, "Armor: %d\n", (int) (info->arg1 + info->arg2 * getItemPowerupLevel(ent, item, 27)));
	}
	gi.cprintf(ent, PRINT_HIGH, "\n");

//Enchantments
	for(i = 0; i < GIEX_ENCHPERITEM; i++) {
		//TODO
	}

//Powerup levels
//TODO: Sum up levels of same powerup type
	for(i = 0; i < 5; i++) {
		if (info->auto_putype[i] == 0) {
			break;
		}
		pinfo = getPowerupInfo(info->auto_putype[i]);
		gi.cprintf(ent, PRINT_HIGH, "+%d %s\n", info->auto_pulvl[i], pinfo->name);
	}
	for (i = 0; i < GIEX_PUPERITEM; i++) {
		if (ent->client->pers.skills.item_putype[item][i] == 0) {
			break;
		}
		pinfo = getPowerupInfo(ent->client->pers.skills.item_putype[item][i]);
		gi.cprintf(ent, PRINT_HIGH, "+%d %s\n", ent->client->pers.skills.item_pucurlvl[item][i], pinfo->name);
	}
}

void wearItem(edict_t *ent, int item) {
	qboolean eq = true;
	int c;
	iteminfo_t *info = getItemInfo(ent->client->pers.skills.itemId[item]);

	if (item == ent->client->pers.skills.wornItem[info->type & 255]) { //Already worn, deequip.
		if ((info->type & 255) != 0) {
			ent->client->pers.skills.wornItem[info->type & 255] = -1;
			gi.cprintf(ent, PRINT_HIGH, "Removing the %s\n", info->name);
			updateItemLevels(ent);
			return;
		} else {
			gi.cprintf(ent, PRINT_HIGH, "You must always wear a weapon!\n");
			return;
		}
	}
	for (c = 0; c < GIEX_NUMCLASSES; c++) {
		int req = getItemLevelReq(ent, item, c);

		if (req > 0) {
			if (req <= ent->client->pers.skills.classLevel[c]) {
				eq = true;
				break;
			}
			if (req > ent->client->pers.skills.classLevel[c]) {
				eq = false;
			}
		} else if (req == -1) {
			eq = false;
		}
	}
	if (!eq) {
		gi.cprintf(ent, PRINT_HIGH, "You do not meet the minimum requirements of this item\n");
		return;
	}
	if (ent->client->pers.skills.wornItem[info->type & 255] == item) {
		return;
	}
	if (ent->client->pers.skills.activatingItem[info->type & 255] != -1) {
		return;
	}
	if (((info->type & 255) == GIEX_ITEM_WEAPON) && (ent->client->weaponstate != WEAPON_READY)) {
		return;
	}
	gi.cprintf(ent, PRINT_HIGH, "Equipping the %s\n", info->name);
	ent->client->pers.skills.wornItem[info->type & 255] = -1;
	ent->client->pers.skills.activatingItem[info->type & 255] = item;
	if ((info->type & 255) == GIEX_ITEM_WEAPON) {
		ent->client->newweapon = info->item;
		//gi.dprintf("switch: %s\n", info->item->classname);
	}
	updateItemLevels(ent);
}
/*
void wearItem(edict_t *ent, int item) {
	int c;

	for (c = 0; c < GIEX_NUMCLASSES; c++) {
		int req = getItemLevelReq(ent, item, c);
		if (req > 0) {
			if (info->classreq[c] <= ent->client->pers.skills.classLevel[c]) {
				eq = true;
				break;
			}
			if (info->classreq[c] > ent->client->pers.skills.classLevel[c]) {
				eq = false;
			}
		} else if (req == -1) {
			eq = false;
		}
	}
	if (!eq) {
		gi.cprintf(ent, PRINT_HIGH, "You do not meet the minimum requirements of this item\n");
		return;
	}
	if (ent->client->pers.skills.wornItem[info->type & 255] == item) {
		return;
	}
	if (ent->client->pers.skills.activatingItem[info->type & 255] != -1) {
		return;
	}
	if (((info->type & 255) == GIEX_ITEM_WEAPON) && (ent->client->weaponstate != WEAPON_READY)) {
		return;
	}
	gi.cprintf(ent, PRINT_HIGH, "Equipping the %s\n", info->name);
	skills->wornItem[info->type & 255] = -1;
	skills->activatingItem[info->type & 255] = item;
	if ((info->type & 255) == GIEX_ITEM_WEAPON) {
		ent->client->newweapon = info->item;
		//gi.dprintf("switch: %s\n", info->item->classname);
	}
	updateItemLevels(ent);
}
*/
void Cmd_wieldItem(edict_t *ent) {
	int i;
	skills_t *skills;
	iteminfo_t *info;

	if (!ent->client)
		return;
	if (ent->client->pers.spectator)
		return;

	skills = &ent->client->pers.skills;

	if (gi.argc() > 1) {
		for (i = 0; i < GIEX_ITEMPERCHAR; i++) {
			if (skills->itemId[i] > 0) {
				info = getItemInfo(skills->itemId[i]);
				if (((info->name[0] != '\0')) && (skills->wornItem[info->type & 255] != i)) {
					if (!Q_strcasecmp(gi.args(), info->name)) {
						wearItem(ent, i);
						return;
					}
				}
			}
		}
		gi.cprintf(ent, PRINT_HIGH, "No %s in inventory\n", gi.argv(1));
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Usage: %s <item name>\n", gi.argv(0));
	}
}
