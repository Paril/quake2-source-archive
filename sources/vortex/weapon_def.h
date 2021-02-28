#define MAX_WEAPONS				13
#define MAX_WEAPONMODS			5

//New weapon defines	(doomie)
#define WEAPON_BLASTER			0
#define WEAPON_SHOTGUN			1
#define WEAPON_SUPERSHOTGUN		2
#define WEAPON_MACHINEGUN		3
#define WEAPON_CHAINGUN			4
#define WEAPON_GRENADELAUNCHER	5
#define WEAPON_ROCKETLAUNCHER	6
#define WEAPON_HYPERBLASTER		7
#define WEAPON_RAILGUN			8
#define WEAPON_BFG10K			9
#define WEAPON_SWORD			10
#define WEAPON_20MM				11
#define WEAPON_HANDGRENADE		12

typedef struct
{
	int			level;			//Level before rune
	int			current_level;	//Level after rune
	int			soft_max;		//Max level
	int			hard_max;		//Max current_level
}weaponskill_t;


typedef struct
{
	qboolean		disable;					//disabled weapon? (future versions?)
	weaponskill_t	mods[MAX_WEAPONMODS];		//Store weapon upgrades in an array
}weapon_t;


/* 

Weapon skills key.. Please keep this updated so we can 
	keep track of what index is used for what. (doomie)

Blaster
	0 - Damage
	1 - Bounce
	2 - Speed
	3 - Trails
	4 - Noise/Flash

Shotgun
	0 - Damage
	1 - Range
	2 - Pellets
	3 - Spread
	4 - Noise/Flash

Super Shotgun
	0 - Damage
	1 - Range
	2 - Pellets
	3 - Spread
	4 - Noise

Machinegun
	0 - Damage
	1 - Pierce
	2 - Tracers
	3 - Spread
	4 - Noise

Chaingun
	0 - Damage
	1 - Spinup/Spindown
	2 - Tracers
	3 - Spread
	4 - Noise

Grenade Launcher
	0 - Damage
	1 - Radus
	2 - Range
	3 - Trails
	4 - Noise

Rocket Launcher
	0 - Damage
	1 - Radius
	2 - Speed
	3 - Trails
	4 - Noise

Hyperblaster
	0 - Damage
	1 - Refire
	2 - Speed
	3 - Light
	4 - Noise
	5 - Flash

Railgun
	0 - Damage
	1 - Pierce
	2 - Burn
	3 - Trails
	4 - Noise

BFG10K
	0 - Damage
	1 - Stick
	2 - Speed
	3 - Pull
	4 - Noise

Sword
	0 - Damage
	1 - Refire
	2 - Length/Range
	3 - Burn
	4 - Noise

20mm Cannon
	0 - Damage
	1 - Range
	2 - Recoil
	3 - Caliber
	4 - Noise

Hand Grenade
	0 - Damage
	1 - Refire
	2 - Radius
	3 - Trails
	4 - Noise

*/