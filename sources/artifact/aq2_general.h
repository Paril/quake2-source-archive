#ifndef AQ2_GENERAL_H
#define AQ2_GENERAL_H

// ********************************************
// Artifact Quake 2 constants and 2-B-Used vars
// ********************************************

// Artifact and Artifact Quake 2 are (c) Copyright 2000 Nick "Singe" Bousman and Glenn "Griphis" Saaiman

// Do not modify and redistribute any AQ2 materials without Singe's or Griphis's permission.
// Do not modify and redistribute this source without providing publically the modified source.
// Do not modify and redistribute this code without giving Singe approrpiate credit!
// Do not modify and redistribute pictures or models from this patch without giving Griphis appropriate credit!

// Constants ******************************************

#define GAMEVERSION         "Artifact Quake 2" // Name of the game
#define MAX_ARTIFACT_TYPES  37                 // The maximum number of artifact types
#define AQ_LEVELTHINK       10                 // How often the level thinks about artifacts
#define AQ_MAX_ARTIFACTS    500                // Maximum number that the server can set the cvar to
#define AQ_HAT              50                 // How high above people's heads various things sit
#define AQ_FREEZE_TIME      12                 // How many seconds Ice Traps work;

// constant integers to identify artifacts with
#define NO_ARTIFACT -1
#define AQ_FLIGHT	0
#define AQ_GWELL	1
#define AQ_REGEN	2
#define AQ_RADIO	3
#define AQ_HEALTH	4
#define AQ_RESIST	5
#define AQ_HASTE	6
#define AQ_EPA	7
#define AQ_BERSERK	8
#define AQ_RECALL	9
#define AQ_ZAP	10
#define AQ_CLOAK	11
#define AQ_KAMIKAZE	12
#define AQ_DEATH	13
#define AQ_HOLOGRAM	14
#define AQ_ORB	15
#define AQ_BLINK	16
#define AQ_CAMO	17
#define AQ_JUMPY	18
#define AQ_ACTION	19
#define AQ_BUFFY	20
#define AQ_SHIELD	21
#define AQ_STRENGTH	22
#define AQ_SWITCH	23
#define AQ_FREEZE	24
#define AQ_TELE	25
#define AQ_IMPACT	26
#define AQ_VENG	27
#define AQ_SHUB	28
#define AQ_REPULSE	29
#define AQ_PHASED	30
#define AQ_HELPER	31
#define AQ_RECALL2	32
#define AQ_WEIRD	33
#define AQ_PHOENIX	34
#define AQ_SPIKY	35
#define AQ_FIREWALK	36

// For AQ-related pieces of the HUD
#define AQ_STAT_NAME     18 // Name of artifact
#define AQ_STAT_ICON     19 // Icon representing artifact
#define AQ_STAT_NUMERAL  20 // Special number displaying info about your equipped artifact
#define AQ_STAT_EQUIPPED 21 // Whether or not you have any artifact equipped
#define AQ_STAT_BIGPIC   22 // Big picture to display in the center of the screen, as a special effect :)

// Bitflags for players who are being fucked over by artifacts
#define AQ_STATE_FROZEN  1
#define AQ_STATE_WEIRD   2
#define AQ_STATE_NOSHOOT 4

// How to be killed by an artifact
#define MOD_AQ_KAMIKAZE     34 // Blowing yourself up with Divine Wind, or killing others in the blast
#define MOD_AQ_GRAVITY_WELL 35 // Getting sucked into a Gravity Well
#define MOD_AQ_IMPACT       36 // Being smashed by someone with Impact
#define MOD_AQ_DEATH_ORB    37 // Being killed by a Death Orb
#define MOD_AQ_ELECTRIC     38 // Being zapped by somebody with Electric
#define MOD_AQ_DEATH_CLOUD  39 // Being killed by chaff from a Death Cloud
#define MOD_AQ_RADIOACTIVE  40 // Dying of radiation poisoning
#define MOD_AQ_VENG         41 // Dying by getting feedback damage from vengeance
#define MOD_AQ_FIREWALK     42 // Getting burned up by the fire of someone's Fire Walking
#define MOD_AQ_KAMIKAZE2    43 // Dying from the explosion when you kill someone with Divine Wind
#define MOD_AQ_DEATH_SPOT   44 // Dying on a Death Spot
#define MOD_AQ_DEATH_SPOT2  45 // Dying from the explosions of dying Death Spots
#define MOD_AQ_DEATH_SPOT3  46 // Dying from laying too many Death Spots
#define MOD_AQ_ELECTRIC2    47 // Dying from jumping in the water with Electric, or being in the water at the time.
#define MOD_AQ_DEATH_ORB2   48 // Being killed by a free-range Death Orb

// Structs ******************************************

// The structure which holds artifact data in an array inside the client structure
typedef struct
{
   int       held;      // "Is this rune being held?"
   float     pTime;     // Passive effect timer
   float     dTime;     // Drop timer
   int       on;        // Makes for less code when checking for active passive effects (no pun intended)

   edict_t*  aEnt;      // just data (reference to entity)
   int       aInt;      // just data
   float     aFloat;    // just data
   vec3_t    aVector;   // just data
   float     aTime;     // just data
} AQ_Held_Artifact;

// Console Variables ******************************************

// The maximum number of artifacts a player can pick up
extern cvar_t *AQ_PlayerMax;

// Determines the behaviour of artifacts regarding mortality.
// 0: (mortal) dissapear when picked up, don't reappear when owner killed
// 1: (immortal) dissapear when picked up, reappear when owner killed
// 2: (ghost) don't dissapear when picked up, don't reappear when owner killed, much like weaponstay
extern cvar_t *AQ_Mortality;

// Maximum number of artifacts allowed in the level
extern cvar_t *AQ_MaxArtifacts;

// Spawn options for artifacts
// 0: artifacts appear gradually during game.
// 1: all artifacts are spawned at level spawn
extern cvar_t *AQ_SpawnAllAtOnce;

// Determines how often (in seconds) artifacts migrate to a new spot (+/- 10 seconds or so)
// 0 implies no migration
extern cvar_t *AQ_MigrationTime;

// Whether or not the type of artifact is visisble to the player.
// 0: artifacts all use their own models
// 1: artifacts all use a generic artifact model
extern cvar_t *AQ_TypeInvisible;

// The seconds a player us required to wait before ditching an artifact
// A negative value implies it impossible to drop an artifact.
extern cvar_t *AQ_Droptime;

// Whether all passive artifact effects are active when a player carries many artifacts,
// or whether only the equipped artifact exhibits its passive effect.
extern cvar_t *AQ_AllArtifactsEffect;

// Variables ******************************************
// Ok, all of these are now in the game.h or g_local.h, cause they're better there.
// Nonetheless, the descriptions will stay here.

// Current number of artifacts in level
//  extern int AQ_CurrentNum;

// Which artifact types are banned from appearing
// 0: not banned
// 1: banned
//  extern int AQ_Artifact_Banned[MAX_ARTIFACT_TYPES];
//TODO: make these booleans somehow

// Which artifact types have recently been picked (for exclusive random feature)
// 0: not picked recently
// 1: picked recently
//  extern int AQ_Artifact_Picked[MAX_ARTIFACT_TYPES];
//TODO: make these booleans somehow

// Artifacts can't spawn immediately at worldspawn. Thus, we make them appear about a second or
// Two into the game. The first variable here tracks whether or not the artifacts have been
// mass-spawned. The second keeps track of how often the level "thinks" about Artifacts.
//  extern int AQ_Artifacts_In;
//  extern int AQ_Artifact_Timer;

// This keeps track of how many Artifacts have been banned.
// Good for keeping morons from trying to ban all artifacts. :)
//  extern int AQ_Banned_Num;

// Message Of The Day sent to all connecting clients.
//extern char AQ_motd[1024];

// Function Prototypes ******************************************

// "g_ai.c"

qboolean visible (edict_t *self, edict_t *other);

// "m_tank.c"

void SP_monster_tank (edict_t *self);

// "g_target.c"

void target_earthquake_think (edict_t *self); // Not mine, but Divine Wind uses it

// "aq2_general.c" ***

void AQ_GlobalInit();
void AQ_LevelItems();
void AQ_WorldSpawn();
void AQ_ClientInit();
void AQ_Player_Die();
void AQ_Level_Think();
int AQ_Get_Random_Type();
void AQ_Get_Random_Location();
void AQ_Spawn_Many_Artifacts();
void AQ_Warp();
void AQ_Artifact_Think();
void AQ_Artifact_Touch();
edict_t *AQ_Spawn_Artifact();
char* AQ_Name_Of();
char* AQ_SName_Of();
char* AQ_Icon_Of();
int AQ_Pickup();
void AQ_Equip();
void AQ_InvUse();
void AQ_Drop();
char* AQ_Desc_Of();
void AQ_Info_Held();
void AQ_Info_Server();
void AQ_Info_BanList();
void AQ_Info_Help();
void AQ_Ban();
void AQ_Unban();
void AQ_Con_Banlist();
void AQ_Give();
void AQ_BecomeExplosion1();
float AQ_Nearest_DMSpawn(vec3_t location);
float AQ_Nearest_TeleDest(vec3_t location);
void AQ_HUD_Num (edict_t *ent);
void AQ_Special_Activate (edict_t *ent, int type);
void AQ_Remove_ALL_Playthings();
void AQ_LevelInit();
void AQ_Remove_Playthings (edict_t *owner);
void AQ_Client_Startup (edict_t *ent);

// "aq2_passive.c" ***

void AQ_Passive_Start();
void AQ_Passive_Shutdown();
void AQ_Passive_Think();
void AQ_Passive_Effect();
void AQ_GWell_Think();
void AQ_GWell_Touch();
void AQ_Impact_Touch (edict_t *ent, edict_t *other);
void AQ_Strength (edict_t *ent);
void AQ_Haste (edict_t *ent);
void AQ_UnCloak (edict_t *ent);
void AQ_Health_Flash (edict_t *ent);
void AQ_EPA_Liquid(edict_t *ent);
void AQ_Vampire (edict_t *ent, edict_t *victim, int amount);
void AQ_VengBack (edict_t *ent, edict_t *attacker, int damage, int knockback, int origMOD);
int AQ_EPA_Damage(edict_t *targ, edict_t *attacker, edict_t *inflictor, int damageStart, int mod);
void AQ_Resist (edict_t *ent);

// "aq2_active.c" ***

void AQ_Active_Effect();
void AQ_Death_Explode (edict_t *ent);
void AQ_TeleTempEnt (vec3_t origin);

#endif



