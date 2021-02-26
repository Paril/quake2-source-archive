
//For Menu Code
#include "menu.h"

//For music code
#ifdef USE_FMOD
#include "fmod.h"

// Music stuff

FSOUND_STREAM *stream; //define the MPEG
#endif

void music_init(void);
void music_play_song(void);
void music_pause_song(void);
void music_stop_song (void);
void music_sogmov1_hack(void);

// ARMOR TYPES
//===============================
// Q1 armor - skid
//===============================
#define ARMOR_Q1_GREEN		    5
#define ARMOR_Q1_YELLOW         6
#define ARMOR_Q1_RED            7
//===============================
// DOOM ARMOR   - SKID
//===============================
#define ARMOR_D_GREEN           8
#define ARMOR_D_BLUE            9

#define ARMOR_W_GREEN           10
#define ARMOR_W_BLUE            11

//===============================

//===============================
// ITEM FLAGS
//===============================
// CTF
#define IT_TECH         64     
//===============================
// added for better weapon pickup
// handling - Skid
#define IT_Q1           128
#define IT_DOOM         256
#define IT_WOLF         512
//===============================


//===============================
// Quake1 backpack struct
//===============================
typedef struct
{
	int		ammo_shells;
	int		ammo_bullets;
	int		ammo_rockets;
	int		ammo_cells;
	int     weapindex;
} gitem_q1backpack;
//===============================


//===============================
// Q1 armor
//===============================
extern	int	q1_green_armor_index;
extern	int	q1_yellow_armor_index;
extern	int	q1_red_armor_index;
//===============================
// Doom Armor
//===============================
extern  int d_green_armor_index;
extern  int d_blue_armor_index;
//===============================

//===============================
// Extra MODs
// Quake1     -Skid
//===============================
#define MOD_Q1_AXE			34
#define MOD_Q1_SG			35
#define MOD_Q1_SSG			36
#define MOD_Q1_NG			37
#define MOD_Q1_SNG			38
#define MOD_Q1_GL			39
#define MOD_Q1_RL			40
#define MOD_Q1_LG			41
#define MOD_Q1_GL_SPLASH	42
#define MOD_Q1_RL_SPLASH	43
#define MOD_Q1_LG_SPLASH	44
//===============================
// Doom
//===============================
#define MOD_DOOM_SAW		45
#define MOD_DOOM_PISTOL		46
#define MOD_DOOM_SG			47
#define MOD_DOOM_SSG		48
#define MOD_DOOM_CG			49
#define MOD_DOOM_PG			50
#define MOD_DOOM_RL			51
#define MOD_DOOM_RL_SPLASH	52
#define MOD_DOOM_BFG_LASER	53
#define MOD_DOOM_BFG_BLAST	54
#define MOD_DOOM_BFG_EFFECT	55
#define MOD_DOOM_FISTS		57
//===============================
// Wolf
//===============================
#define MOD_WOLF_KNIFE      58
#define MOD_WOLF_PISTOL		59
#define MOD_WOLF_MACHINEGUN	60
#define MOD_WOLF_GG		61
#define MOD_WOLF_GG2	62
//#define MOD_WOLF_HG			62
//#define MOD_WOLF_HG_SPLASH  63
#define MOD_WOLF_RL			64
#define MOD_WOLF_RL_SPLASH	65
#define	MOD_WOLF_FLAME		66
#define MOD_WOLF_FLAMEBURN	67

#define MOD_DOOM_BARREL		63

#define MOD_KEEN_RAYGUN     68


// DOOM DEATHVIEW
#define DEAD_GIBBED				4

//Valid Skin flag  - used for Q2 player HUD
#define FL_VALID_MODEL			0x00002000
//used to limit player movement when being sawed :D	
#define FL_Q2VALIDMALE			0x00004000
//invisible flag, more efficient than comparing framenums
#define FL_INVIS				0x00008000	

//its a Quake1 monster, its stupid :D
#define FL_Q1_MONSTER			0x00100000	
#define FL_D_MONSTER			0x00200000

#define FL_D_BERSERK			0x00800000	

#define FL_D_GIBBED				0x01000000	

//Quake1 Runes - set when player picks up rune
#define FL_RUNE_RESIST			0x00010000
#define FL_RUNE_STRENGTH		0x00020000
#define FL_RUNE_HASTE			0x00040000
#define FL_RUNE_REGEN			0x00080000
#define FL_RUNE_MASK			(FL_RUNE_HASTE|FL_RUNE_STRENGTH|FL_RUNE_REGEN|FL_RUNE_RESIST)


//===============================
// CTF Stuff
//===============================
#define MOD_GRAPPLE         68
extern	qboolean		is_quad;

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, 
					  vec3_t forward, vec3_t right, vec3_t result);

//===============================
// Map Voting Stuff
//===============================

#define CM_VOTE_NO	0
#define CM_VOTE_YES	1

#define CM_VOTETHINKTIME	15
#define CM_VOTETIMEOUT      90


//===============================


//===============================
// New Weapon Functions -Skid
//===============================

// Quake1
void fire_shotty (edict_t *self, vec3_t start, vec3_t aimdir, 
				 int damage, int kick, int count, int hspread, int vspread, int mod);
void q1_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void q1_fire_nail (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, qboolean sng);
void q1_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void q1_fire_lightning (edict_t *self, vec3_t start, vec3_t dir, int damage);
void q1_fire_axe ( edict_t *self, vec3_t start, vec3_t dir, int damage);

// Doom
void d_fire_punch ( edict_t *self, vec3_t start, vec3_t dir, int damage);
void d_fire_saw ( edict_t *self, vec3_t start, vec3_t dir, int damage);
void d_fire_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void d_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void d_fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);

//Wolf
void w_fire_knife ( edict_t *self, vec3_t start, vec3_t dir, int damage);
void w_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void w_fire_flame (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);

//Keen
void keen_fire_raygunshot (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
//===============================

//===============================
// Stat bars
//===============================
void G_WolfSetStats(edict_t *ent);
void G_DoomSetStats (edict_t *ent);
void G_Q1SetStats (edict_t *ent);

//===============================
// WEAPON CHANGE
void Oldie_NoAmmoWeaponChange( edict_t *ent);
void Q1_NoAmmoWeaponChange (edict_t *ent);
void Doom_NoAmmoWeaponChange (edict_t *ent);
void Wolf_NoAmmoWeaponChange (edict_t *ent);

//===============================
//Misc Q1 Pickup/Drop
void Q1DropAmmoPack(edict_t *ent);
qboolean Pickup_q1_backpack(edict_t *ent, edict_t *other);
void Q1_DropBackPack(edict_t *ent);

//Rocket Explosions
void q1_explode (edict_t *self);
void GenRockExplode(edict_t *ent, vec3_t origin);
void d_texplode (edict_t *self);  // from m_idg2_cu.c

void ChasecamRemove (edict_t *ent);