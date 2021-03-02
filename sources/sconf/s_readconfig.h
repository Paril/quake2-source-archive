/* s_readconfig.h */
/* Keeps track of all global used for reading in config files which
   are referenced in s_readconfig.c */

typedef struct node {
		char value[120];
		struct node *next;
	} Node, *NodePtr;

// HIGHSCORE


void parseFileLocations(char pair[]);
void parseServObits(char pair[]);
void parseGrapple(char pair[]);
void parseCTF(char pair[]);
void parseMOTD(char pair[]);
void parseBan(char pair[]);
void parseHealth(char pair[]);
void parseMaxAmmo(char pair[]);
void parseStartAmmo(char pair[]);
void parseStartItems(char pair[]);
void parseStartWeapons(char pair[]);
void parseQuad(char pair[]);
void parseMisc(char pair[]);
void parseLevelChange(char pair[]);
void parseLogging(char pair[]);
void parseLostQuad(char pair[]);
void parseRocket(char pair[]);
void parseGLauncher(char pair[]);
void parseGrenade(char pair[]);
void parseBlaster(char pair[]);
void parseGuns(char pair[]);


// CYCLE ADD
char configloc[50];
char cycleloc[50];

int scoreboard; // style of scoreboard for dm
char GlobalFragLimit[5]; // Actual value
char GlobalTimeLimit[5]; // Actual value
char GlobalGravity[5]; // Actual value
int QWLOG; // > 0 enables logging
char directory[40]; // full path to logfile
char recordLOG[2];

char ModelGenDir[50];
char obitsDir[50];
char HIGHSCORE_DIR[50];
char PLAYERS_LOGFILE[50];
char MAX_CLIENT_RATE_STRING[10];
int MAX_CLIENT_RATE;
int clientlog;
int showed;

// Will be on by default unless stated in config file
int rankhud;
int playershud;
int timehud;
//int helphud;

int cloakgrapple;
int hookcolor;
// Sconfig 2.6 (grapple)
int allowgrapple;
int HOOK_TIME;		  // Number of seconds that hook can live
int HOOK_SPEED;       // Velocity of the hook itself
int EXPERT_SKY_SOLID; // Change later
int HOOK_DAMAGE; 	  // Damage done by hook
int PULL_SPEED;  	  // How fast the player is pulled 
					  // once the hook is anchored


int LoseQ;
int LoseQ_Fragee;
int ConfigRD; // Rocket damage
int CRD; // (in g_main)
int rocketSpeed; // Rocket speed
int Q_Killer;
int Q_Killee;
int CF_StartHealth;
int CF_MaxHealth;
int MA_Bullets;
int	MA_Shells;
int	MA_Cells;
int	MA_Grenades;
int	MA_Rockets;
int MA_Slugs;
int SA_Bullets;
int SA_Shells;
int SA_Cells;
int SA_Grenades;
int SA_Rockets;
int SA_Slugs;
int SI_QuadDamage;
int SI_Invulnerability;
int SI_Silencer;
int SI_Rebreather;
int SI_EnvironmentSuit;
int SI_PowerScreen;
int SI_PowerShield;
int QuadDamageTime;
int RebreatherTime;
int EnvironmentSuitTime;
int InvulnerabilityTime;
int SilencerShots;
int RegenInvulnerability;
int RegenInvulnerabilityTime;
int AutoUseQuad;
int AutoUseInvulnerability;
int SW_Blaster;
int SW_ShotGun;
int SW_SuperShotGun;
int SW_MachineGun;
int SW_ChainGun;
int SW_GrenadeLauncher;
int SW_RocketLauncher;
int SW_HyperBlaster;
int SW_RailGun;
int SW_BFG10K;


//int spectator;

// SCONFIG START
int rocketspeed;
int RadiusDamage;
int DamageRadius;

float GLauncherTimer;
int GLauncherFireDistance;
int	GLauncherDamage;
int GLauncherRadius;

float   GRENADE_TIMER;
int     GRENADE_MINSPEED; //400
int     GRENADE_MAXSPEED; //800


float GrenadeTimer;
int GrenadeMinSpeed;
int GrenadeMaxSpeed;
int	GrenadeDamage;
int GrenadeRadius;

int HyperBlasterDamage;
int BlasterProjectileSpeed;
int BlasterDamage;

int MachinegunDamage;
int MachinegunKick;
int ChaingunDamage;
int ChaingunKick;
int ShotgunDamage;
int ShotgunKick;
int SuperShotgunDamage;
int SuperShotgunKick;
int RailgunDamage;
int RailgunKick;
int BFGDamage;
float BFGDamageRadius;
int BFGProjectileSpeed;

// For banning of weapons, cilent won't be able to pick them up when set.

int namebanning; // Version 2.3 (Clan/Name banning)
char bandirectory[50];
int ingamenamebanningstate;
int wasbot;

enum indexlist {
	index_shotgun=8,
	index_supershotgun,
	index_machinegun,
	index_chaingun,
	index_grenadelauncher=13,
	index_rocketlauncher,
	index_hyperblaster,
	index_railgun,
	index_BFG
};
	
int ban_BFG;
int ban_hyperblaster;
int ban_rocketlauncher;
int ban_railgun;
int ban_chaingun;
int ban_machinegun;
int ban_shotgun;
int ban_supershotgun;
int ban_grenadelauncher;

// Version 2.3
int matchfullnamevalue;
int fullnamevalue;
int fastrailgun;
int fastrocketfire;
int cloaking;
int CLOAK_DRAIN; // took it off define... to be customizable.
int chasekeepscore;
int fastchange;
int fraghit;

// Visible weapon patch?
//int VWPATCH;

// Handle MOTD text (15 lines total (18 with chase cam and continue)
char somevar0[30];
char somevar1[30];
char somevar2[30];
char somevar3[30];
char somevar4[30];
char somevar5[30];
char somevar6[30];
char somevar7[30];
char somevar8[30];
char somevar9[30];
char somevar10[30];
char somevar11[30];
char somevar12[30];
char somevar13[30];
char somevar14[30];

int totalrank;

char hi_head1[60];
char hi_head2[60];
char hi_line1[60];
char hi_line2[60];
char hi_line3[60];
char hi_line4[60];
char hi_line5[60];
char hi_line6[60];
char hi_line7[60];
char hi_line8[60];
char hi_line9[60];
char hi_line10[60];
char hi_line11[60];
char hi_line12[60];
char hi_line13[60];
char hi_line14[60];
char hi_line15[60];
