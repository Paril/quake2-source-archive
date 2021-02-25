
// SUBSTITUTION INDEXES 

//================
// WEAPON INDEXES
//================

// Quake2
#define Q2_BLASTER	7
#define Q2_SG		8
#define Q2_SSG		9
#define Q2_MG		10
#define Q2_CG		11
#define Q2_HG		12
#define Q2_GL		13
#define Q2_RL		14
#define Q2_HB		15
#define Q2_RG		16
#define Q2_BFG		17
#define Q2_HOOK		41


// Quake1
#define Q1_AXE		18
#define Q1_SG		19
#define Q1_SSG		20
#define Q1_NG		21
#define Q1_SNG		22
#define Q1_GL		23
#define Q1_RL		24
#define Q1_LG		25
#define Q1_HOOK		97

// Doom
#define D_FISTS		26
#define D_CSAW		27
#define	D_PISTOL	28
#define D_SG		29
#define D_SSG		30
#define D_CG		31
#define D_RL		32
#define D_PG		33
#define D_BFG		34

// Wolf
#define W_KNIFE		35
#define W_PISTOL	36
#define W_MG		37
#define	W_GG		38
#define W_RL		39
#define W_FT		40
#define W_GG2		120


//================
// AMMO INDEXES
//================

// Quake2
#define AMMO_GRENADES_INDEX		12
#define AMMO_SHELLS_INDEX		42
#define AMMO_BULLETS_INDEX		43
#define AMMO_CELLS_INDEX		44
#define AMMO_ROCKETS_INDEX		45
#define AMMO_SLUGS_INDEX		46

#define INDEX_Q2_RG 123
#define INDEX_W_RG 124

// Quake1
#define INDEX_Q1_SL		47
#define INDEX_Q1_SS		48
#define INDEX_Q1_NL		49
#define INDEX_Q1_NS		50
#define INDEX_Q1_CL		51
#define INDEX_Q1_CS		52
#define INDEX_Q1_RL		53
#define INDEX_Q1_RS		54

#define INDEX_Q1_RG		114

// Doom
#define INDEX_D_SL		55
#define INDEX_D_SS		56
#define INDEX_D_BL		57
#define INDEX_D_BS		58
#define INDEX_D_CL		59
#define INDEX_D_CS		60
#define INDEX_D_RL		61
#define INDEX_D_RS		62

//Wolf ammo
#define INDEX_W_BL		116	
#define INDEX_W_BS		117
#define INDEX_W_ROCKS	118
#define INDEX_W_PETROL	119


//================
// Armor
//================

#define	Q2_BODY			1
#define Q2_COMBAT		2
#define	Q2_JACKET		3
#define	Q2_SHARD		4
#define	Q2_POWERSCREEN	5
#define	Q2_POWERSHIELD	6

#define	Q1_RED			63
#define	Q1_YELLOW		64
#define Q1_GREEN		65
#define Q1_SHARD		115

#define	D_BLUE			66
#define D_GREEN			67
#define	D_HELMET		68

#define W_BLUE			121
#define W_GREEN			122


//================
// Powerups
//================

// Quake1

#define	Q1_QUAD			69
#define	Q1_RING			70
#define	Q1_PENT			71
#define Q1_SUIT			72
#define Q1_PACK			73

// Doom

#define	D_BERSERK		74
#define	D_INVIS			75
#define	D_GOD			76
#define	D_SOULSPHERE	77
#define	D_RADSUIT		78
#define	D_BACKPACK		79

// Quake2

#define	Q2_QUAD			80
#define	Q2_GOD			81
#define	Q2_SILENCER		82
#define	Q2_REBREATHER	83
#define	Q2_ENVIROSUIT	84

#define	Q2_ADERNALINE	86
#define	Q2_BANDOLIER	87
#define	Q2_BACKPACK		88

//================
// Techs/Runes
//================

#define Q1_EARTH		93
#define	Q1_BLACK		94
#define	Q1_HELL			95
#define Q1_ELDER		96

#define Q2CTF_TECH1		110
#define Q2CTF_TECH2		111
#define Q2CTF_TECH3		112
#define Q2CTF_TECH4		113

#define	Q2_HEALTH_MEGA	900
#define	Q1_HEALTH_MEGA	901

// Q1 SHOTGUN SPREAD
#define DEFAULT_Q1_SG_HSPREAD	500
#define DEFAULT_Q1_SG_VSPREAD	300
#define DEFAULT_Q1_SG_COUNT		6
#define DEFAULT_Q1_SSG_COUNT	7

//DOOM
#define DEFAULT_D_SG_HSPREAD	800
#define DEFAULT_D_SG_VSPREAD	390
#define DEFAULT_D_SG_COUNT		10
#define DEFAULT_D_SSG_COUNT		15

extern generations_game gengame;


int GetRandomClass();

// SUBSTITUTION FUNCTIONS

// god substitution function
gitem_t * GiveNewItem(gitem_t *olditem, int player_class);
void GiveWeapon(edict_t *ent,int index,qboolean ammo);

// weapons ammo
int GiveDoomWeapAmmo(int weap_index);
int GiveQ1WeapAmmo(int weap_index);

//for weapon switching
qboolean Is_new_weapon_better (int index, gitem_t *old_weap, int player_class);

// oldie powerups
void Use_Invis(edict_t *ent, gitem_t *item);
void Use_MegaSphere(edict_t *ent, gitem_t *item);
void Use_Berserk(edict_t *ent, gitem_t *item);
qboolean Pickup_Rune (edict_t *ent, edict_t *other);

// TELEPORTER SPLASH
void DoomTeleporterSplash(vec3_t spot, vec3_t offset);
void GenClientRespawnFX(edict_t *ent);
void GenQ1TeleportSounds(edict_t *ent);
void GenQ1TeleportSounds2(edict_t *ent1, edict_t *ent2);

gitem_t * InitQ1Persistant(gclient_t *client);
gitem_t * InitQ2Persistant(gclient_t *client);
gitem_t * InitDoomPersistant(gclient_t *client);
gitem_t * InitWolfPersistant(gclient_t *client);

edict_t *loc_findradius (edict_t *from, vec3_t org, float rad);
qboolean loc_CanSee (edict_t *targ, edict_t *inflictor);

//Manage Team Score
void AddToTeamScore(int team_class, int score);
void RemoveFromTeamScore(int team_class, int score);

void LookAtKiller2 (edict_t *ent);
void WhoKilledDoomguy (edict_t *self, edict_t *attacker) ;

//Cyberdemon Salute
void cyberdemon_salute (edict_t *self);


