/*==============================================================================
The Weapons Factory - 
Global Include File
Original code by Gregg Reno
==============================================================================*/

#define WF_VERSION  "WF Version 4.2"
#define WF_URL "www.captured.com/weaponsfactory"

/* -------------------------------
   CONSTANTS AND MACROS 
------------------------------- */

//id number for WF approved class file
#define MOD_ID_WF		3784
#define MOD_ID_OMEGA	4791
#define MOD_ID_Q2TF		1949	//www.planetice.net/q2tf

//Damage Values
#define DAMAGE_BLASTER          15
#define DAMAGE_SHOTGUN          4
#define DAMAGE_SUPERSHOTGUN     6
#define DAMAGE_MACHINEGUN       8
#define DAMAGE_CHAINGUN         6
#define DAMAGE_HYPERBLASTER     12
#define DAMAGE_ROCKETLAUNCHER   105
#define DAMAGE_GRENADELAUNCHER  120
#define DAMAGE_RAILGUN          80
#define DAMAGE_BFG              200
#define DAMAGE_NEEDLER			3
#define DAMAGE_NAG				5
#define DAMAGE_TELSA			5
#define DAMAGE_LIGHTNING		45
#define DAMAGE_NAILGUN			4
#define DAMAGE_CLUSTERROCKET    45
#define DAMAGE_MAGBOLTED		25
#define DAMAGE_PELLET			110
#define DAMAGE_PULSE			3
#define DAMAGE_SHC				3
#define DAMAGE_FLAREGUN			0
#define DAMAGE_NAPALMMISSLE		6
#define DAMAGE_FLAMETHROWER		15
#define DAMAGE_TRANQUILIZER		5
#define DAMAGE_INFECTEDDART		20
#define DAMAGE_LASERSNIPER		0
#define DAMAGE_ARMORDART		20
#define DAMAGE_SHOTGUNCHOKE		4
#define DAMAGE_SNIPERRIFLE		160
#define DAMAGE_LRPROJECTILE		0
#define DAMAGE_SENTRYKILLER		120
#define DAMAGE_MEGACHAINGUN		5
#define DAMAGE_TRANQUILDART		5
#define DAMAGE_KNIFE			80
#define DAMAGE_AK47				13
#define	DAMAGE_PISTOL			35
#define DAMAGE_STINGER			125
#define DAMAGE_DISRUPTOR		30
#define DAMAGE_ETF_RIFLE		10
#define DAMAGE_PLASMA_BEAM		0
#define DAMAGE_ION_RIPPER		30
#define DAMAGE_PHALANX			70
#define DAMAGE_FREEZER			15

//Speed values
#define SPEED_BLASTER			1000
#define SPEED_SHOTGUN			10
#define SPEED_SUPERSHOTGUN		10
#define SPEED_MACHINEGUN		10
#define SPEED_CHAINGUN			10
#define SPEED_HYPERBLASTER		10
#define SPEED_ROCKETLAUNCHER	725
#define SPEED_GRENADELAUNCHER	600
#define SPEED_RAILGUN			10
#define SPEED_BFG				400

#define SPEED_NEEDLER			10
#define SPEED_NAG				1000
#define SPEED_TELSA				10
#define SPEED_LIGHTNING			950
#define SPEED_NAILGUN			10
#define SPEED_CLUSTERROCKET		370
#define SPEED_MAGBOLTED			1000
#define SPEED_PELLET			550
#define SPEED_PULSE				10
#define SPEED_SHC				10
#define SPEED_FLAREGUN			850
#define SPEED_NAPALMMISSLE		390
#define SPEED_FLAMETHROWER		400
#define SPEED_TRANQUILIZER		10
#define SPEED_INFECTEDDART		390
#define SPEED_LASERSNIPER		10
#define SPEED_ARMORDART			850
#define SPEED_SHOTGUNCHOKE		10
#define SPEED_SNIPERRIFLE		10
#define SPEED_LRPROJECTILE		10
#define SPEED_SENTRYKILLER		325
#define SPEED_MEGACHAINGUN		10
#define SPEED_TRANQUILDART		10
#define SPEED_KNIFE				10
#define SPEED_AK47				10
#define	SPEED_PISTOL			10
#define SPEED_STINGER			800
#define SPEED_DISRUPTOR			1000
#define SPEED_ETF_RIFLE			750
#define SPEED_PLASMA_BEAM		10
#define SPEED_ION_RIPPER		500
#define SPEED_PHALANX			725
#define SPEED_FREEZER			1000

//Grenade damage
#define DAMAGE_GRENADE			125
#define DAMAGE_LASERBALL		2
#define DAMAGE_GOODYEAR			80
#define DAMAGE_PROXIMITY		80
#define DAMAGE_FLASH			0
#define DAMAGE_CLUSTER			65
#define DAMAGE_EARTHQUAKE		0
#define DAMAGE_TURRET			37
#define DAMAGE_NAPALM			18
#define DAMAGE_CONCUSSION		0
#define DAMAGE_NARCOTIC			0
#define DAMAGE_PLAGUE			11
#define DAMAGE_MAGNOTRON		80
#define DAMAGE_SHOCK			9
#define DAMAGE_PIPEBOMB			110
#define DAMAGE_SHRAPNEL			15
#define DAMAGE_FLARE			0
#define DAMAGE_SLOW				0
#define DAMAGE_PLAGUETIME		0
#define DAMAGE_LASERCUTTER		5
#define DAMAGE_TESLA			3
#define DAMAGE_GAS				0

#define SPEED_GRENADE			800
#define SPEED_LASERBALL			800
#define SPEED_GOODYEAR			800
#define SPEED_PROXIMITY			800
#define SPEED_FLASH				800
#define SPEED_CLUSTER			800
#define SPEED_EARTHQUAKE		800
#define SPEED_TURRET			800
#define SPEED_NAPALM			800
#define SPEED_CONCUSSION		800
#define SPEED_NARCOTIC			800
#define SPEED_PLAGUE			800
#define SPEED_MAGNOTRON			800
#define SPEED_SHOCK				800
#define SPEED_PIPEBOMB			800
#define SPEED_SHRAPNEL			800
#define SPEED_FLARE				800
#define SPEED_SLOW				800
#define SPEED_PLAGUETIME		800
#define SPEED_LASERCUTTER		800
#define SPEED_TESLA				800
#define SPEED_GAS				800


//Some Max Values
#define MAX_ARMOR	400
#define MAX_HEALTH	200

// Grenade types
#define GRENADE_TYPE_NONE			0
#define GRENADE_TYPE_NORMAL			1
#define GRENADE_TYPE_LASERBALL		2
#define GRENADE_TYPE_GOODYEAR		3
#define GRENADE_TYPE_PROXIMITY		4
#define GRENADE_TYPE_FLASH			5
#define GRENADE_TYPE_CLUSTER		6
#define GRENADE_TYPE_EARTHQUAKE		7
#define GRENADE_TYPE_TURRET			8
#define GRENADE_TYPE_NAPALM			9
#define GRENADE_TYPE_CONCUSSION		10
#define GRENADE_TYPE_NARCOTIC		11
#define GRENADE_TYPE_PLAGUE			12
#define GRENADE_TYPE_MAGNOTRON		13
#define GRENADE_TYPE_SHOCK			14
#define GRENADE_TYPE_PIPEBOMB		15
#define GRENADE_TYPE_SHRAPNEL		16
#define GRENADE_TYPE_FLARE			17
#define GRENADE_TYPE_SLOW			18
#define GRENADE_TYPE_PLAGUETIME		19
#define GRENADE_TYPE_LASERCUTTER	20
#define GRENADE_TYPE_TESLA			21
#define GRENADE_TYPE_GAS			22
#define GRENADE_TYPE_COUNT			22  //How many grenade types?

//Grenade skins & models
#define GRMODEL_GRENADE1	"models/objects/grenade/tris.md2"
#define GRMODEL_GRENADE2	"models/objects/grenade2/tris.md2"
#define GRMODEL_GRENADE3	"models/objects/grenade3/tris.md2"
#define GRMODEL_V_GRENADE1	"models/weapons/v_handgr/tris.md2"
#define GRMODEL_V_GRENADE2	"models/weapons/v_handgr2/tris.md2"
#define GRMODEL_V_GRENADE3	"models/weapons/v_handgr3/tris.md2"

#define GRNORMAL_MODEL		GRMODEL_GRENADE1
#define GRNORMAL_SKIN		0

#define GRLAUNCHER_MODEL	GRMODEL_GRENADE2
#define GRLAUNCHER_SKIN		0

#define GRLASERBALL_MODEL	GRMODEL_GRENADE3
#define GRLASERBALL_SKIN	1

#define GRGOODYEAR_MODEL	GRMODEL_GRENADE3
#define GRGOODYEAR_SKIN		1
//#define GRGOODYEAR_SKIN		3

#define GRPROXIMITY_MODEL	GRMODEL_GRENADE2
#define GRPROXIMITY_SKIN	2

#define GRFLASH_MODEL		GRMODEL_GRENADE3
//define GRFLASH_MODEL	"models/objects/flashgr/tris.md2"
#define GRFLASH_SKIN		4

#define GRCLUSTER_MODEL		GRMODEL_GRENADE2
#define GRCLUSTER_SKIN		4

#define GRTURRET_MODEL		GRMODEL_GRENADE3
#define GRTURRET_SKIN		2

#define GRNAPALM_MODEL		GRMODEL_GRENADE2
#define GRNAPALM_SKIN		6

#define GRCONCUSSION_MODEL	GRMODEL_GRENADE3
#define GRCONCUSSION_SKIN	0

#define GRPLAGUE_MODEL		GRMODEL_GRENADE2
#define GRPLAGUE_SKIN		1

#define GRMAGNOTRON_MODEL	GRMODEL_GRENADE2
#define GRMAGNOTRON_SKIN	0

#define GRSHOCK_MODEL		GRMODEL_GRENADE2
#define GRSHOCK_SKIN		3

#define GRPIPEBOMB_MODEL	GRMODEL_GRENADE2
#define GRPIPEBOMB_SKIN		7

#define GRSHRAPNEL_MODEL	GRMODEL_GRENADE3
#define GRSHRAPNEL_SKIN		5

#define GRFLARE_MODEL		GRMODEL_GRENADE2
#define GRFLARE_SKIN		5

#define GRPLAGUETIME_MODEL	GRMODEL_GRENADE1
#define GRPLAGUETIME_SKIN	0

#define GRLASERCUTTER_MODEL	GRMODEL_GRENADE1
#define GRLASERCUTTER_SKIN	0

//need new skins
#define GRTESLA_MODEL		GRMODEL_GRENADE3
#define GRTESLA_SKIN		0

//need new skins
#define GRGAS_MODEL			GRMODEL_GRENADE2
#define GRGAS_SKIN			0

//#define GREARTHQUAKE_MODEL	GRMODEL_GRENADE1
//#define GREARTHQUAKE_SKIN	999

//#define GRNARCOTIC_MODEL	GRMODEL_GRENADE
//#define GRNARCOTIC_SKIN		9999

//#define GRSLOW_MODEL		GRMODEL_GRENADE
//#define GRSLOW_SKIN

//Maximum number of active grenades for each type
#define MAX_TYPE_NORMAL     999
#define MAX_TYPE_LASERBALL  1
#define MAX_TYPE_GOODYEAR   4
#define MAX_TYPE_PROXIMITY  4
#define MAX_TYPE_FLASH      999
#define MAX_TYPE_CLUSTER    999
#define MAX_TYPE_EARTHQUAKE 999
#define MAX_TYPE_TURRET		2
#define MAX_TYPE_NAPALM     2
#define MAX_TYPE_CONCUSSION	1
#define MAX_TYPE_NARCOTIC	999
#define MAX_TYPE_PLAGUE		2
#define MAX_TYPE_MAGNOTRON	1
#define MAX_TYPE_SHOCK		2
#define MAX_TYPE_PIPEBOMB	6
#define MAX_TYPE_SHRAPNAL	999
#define MAX_TYPE_FLARE		999
#define MAX_TYPE_SLOW		999
#define MAX_TYPE_PLAGUETIME	999
#define MAX_TYPE_LASERCUTTER 1
#define MAX_TYPE_TESLA		1
#define MAX_TYPE_GAS		2

// proximity grenade
#define PROXIMITY_TYPE_GRENADE	1
#define PROXIMITY_TYPE_PIPE     2

// grenade requirements
#define LASERBALL_CELLS			18
#define DECOY_CELLS			    0
#define EARTHQUAKE_GRENADES		6
#define TURRET_GRENADES			2
#define TURRET_SLUGS			4

//Other
#define RESPAWN_PROTECT_TIME	8.0

//Model Types
#define CLASS_MODEL_MALE		1
#define CLASS_MODEL_FEMALE		2
#define CLASS_MODEL_CYBORG		3
#define CLASS_MODEL_ANY			4	//use the player's choice

// Weapon types 
#define WEAPON_NONE				0
#define WEAPON_BLASTER          1
#define WEAPON_SHOTGUN          2
#define WEAPON_SUPERSHOTGUN     3
#define WEAPON_MACHINEGUN       4
#define WEAPON_CHAINGUN         5
#define WEAPON_HYPERBLASTER     6
#define WEAPON_ROCKETLAUNCHER   7
#define WEAPON_GRENADELAUNCHER  8
#define WEAPON_RAILGUN          9
#define WEAPON_BFG              10

#define WEAPON_NEEDLER			11
#define WEAPON_NAG				12
#define WEAPON_TELSA			13
#define WEAPON_LIGHTNING		14
#define WEAPON_NAILGUN			15
#define WEAPON_CLUSTERROCKET	16
#define WEAPON_MAGBOLTED		17
#define WEAPON_PELLET			18
#define WEAPON_PULSE			19
#define WEAPON_SHC				20

#define WEAPON_FLAREGUN			21
#define WEAPON_NAPALMMISSLE		22
#define WEAPON_FLAMETHROWER		23
#define WEAPON_TRANQUILIZER		24
#define WEAPON_INFECTEDDART		25
#define WEAPON_LASERSNIPER		26	//unused now
#define WEAPON_ARMORDART		27
#define WEAPON_SHOTGUNCHOKE		28	//unused now
#define WEAPON_SNIPERRIFLE		29
#define WEAPON_LRPROJECTILE		30

#define WEAPON_SENTRYKILLER		31
#define WEAPON_MEGACHAINGUN		32
#define WEAPON_TRANQUILDART		33
#define WEAPON_KNIFE			34
#define WEAPON_AK47				35
#define	WEAPON_PISTOL			36
#define WEAPON_STINGER			37
#define WEAPON_DISRUPTOR		38
#define WEAPON_ETF_RIFLE		39
#define WEAPON_PLASMA_BEAM		40

#define WEAPON_ION_RIPPER		41
#define WEAPON_PHALANX			42
#define WEAPON_FREEZER          43 //acrid3

#define WEAPON_COUNT			43  //How many grenade types?

// Player items
#define ITEM_NONE				0
#define ITEM_REBREATHER			1
#define ITEM_BODYARMOR			2
#define ITEM_COMBATARMOR		4
#define ITEM_JACKETARMOR		8
#define ITEM_POWERSCREEN		16
#define ITEM_POWERSHIELD		32
#define ITEM_SILENCER			64

//Temporary - use these for new items
#define ITEM_t1		128
#define ITEM_t2		256
#define ITEM_t3		512
#define ITEM_t4		1024
#define ITEM_t5		2048
#define ITEM_t6		4096

// Type of ammo player can pick up
#define WF_AMMO_CELLS			1
#define WF_AMMO_SHELLS			2
#define WF_AMMO_BULLETS		4
#define WF_AMMO_ROCKETS		8
#define WF_AMMO_SLUGS			16
#define WF_AMMO_GRENADES		32

// Player special abilities
#define SPECIAL_NONE				0
#define SPECIAL_JETPACK				1		//1
#define SPECIAL_BIOSENTRY			2		//2	- was binoculars
#define SPECIAL_REMOTE_CAMERA		4		//3
#define SPECIAL_ANTIGRAV_BOOTS		8		//4
#define SPECIAL_DISGUISE			16		//5
#define SPECIAL_FEIGN				32		//6
#define SPECIAL_HEALING				64		//7
#define SPECIAL_SENTRY_GUN			128		//8
#define SPECIAL_MISSILE_LAUNCHER	256		//9
#define SPECIAL_MERCENARY			512		//10 - was repair armor
#define SPECIAL_ALARMS				1024	//11
#define SPECIAL_KAMIKAZE			2048	//12
#define SPECIAL_LASER_DEFENSE		4096	//13
#define SPECIAL_FLAME_RESISTANCE	8192	//14
#define SPECIAL_TRIPBOMB			16384	//15
#define SPECIAL_CLOAK				32768	//16
#define SPECIAL_SUPPLY_DEPOT		65536	//17
#define SPECIAL_PLASMA_BOMB			131072 	//18
#define SPECIAL_GRAPPLE				262144	//19
#define SPECIAL_SNIPING				524288	//20
#define SPECIAL_LASER				1048576	//21
#define SPECIAL_AUTOZOOM			2097152	//22
#define SPECIAL_QUICKAIM			4194304	//23 The new sniper ability 
//Temporary - use these for new special abilities
//#define SPECIAL_t6			4194304	
//THESE TWO ARE RESERVED	
#define SPECIAL_OPTION1		8388608
#define SPECIAL_OPTION2		16777216
#define SPECIAL_COUNT		24

//MAX COUNTS FOR EACH SPECIAL THING
#define MAX_SPECIAL_REMOTE_CAMERA		1
#define MAX_SPECIAL_SENTRY_GUN			1
#define MAX_SPECIAL_MISSILE_LAUNCHER	1
#define MAX_SPECIAL_ALARMS				2
#define MAX_SPECIAL_LASER_DEFENSE		1
#define MAX_SPECIAL_TRIPBOMB			2
#define MAX_SPECIAL_QUAKE				2
#define MAX_SPECIAL_SUPPLY_DEPOT		1
#define MAX_SPECIAL_PLASMA_BOMB			1

//Sequential list of special items, rather than bitmask
#define ITEM_SPECIAL_REMOTE_CAMERA		1
#define ITEM_SPECIAL_SENTRY_GUN			2
#define ITEM_SPECIAL_MISSILE_LAUNCHER	3
#define ITEM_SPECIAL_ALARMS				4
#define ITEM_SPECIAL_LASER_DEFENSE		5
#define ITEM_SPECIAL_TRIPBOMB			6
#define ITEM_SPECIAL_QUAKE				7
#define ITEM_SPECIAL_SUPPLY_DEPOT		8
#define ITEM_SPECIAL_PLASMA_BOMB		9


#define CLASS_DEF_NAME			"WF Standard"


//Scanner stuff
#define SCANNER_UNIT                   32
#define SCANNER_RANGE                  100
#define SCANNER_UPDATE_FREQ            1
#define PIC_LEFT                       "pics/arrowleft.pcx"
#define PIC_LEFT_TAG                   "scanner/arrowleft"
#define PIC_RIGHT                       "pics/arrowright.pcx"
#define PIC_RIGHT_TAG                   "scanner/arrowright"
#define PIC_UP                       "pics/arrowup.pcx"
#define PIC_UP_TAG                   "scanner/arrowup"
#define PIC_DOWN                       "pics/arrowdown.pcx"
#define PIC_DOWN_TAG                   "scanner/arrowdown"
#define PIC_SCANNER                    "pics/scanner/scanner.pcx"
#define PIC_SCANNER_TAG                "scanner/scanner"
#define SAFE_STRCAT(org,add,maxlen)    if ((strlen(org) + strlen(add)) < maxlen)    strcat(org,add);
#define LAYOUT_MAX_LENGTH              1400

//Fast weapons switching
#define fastswitch 1

//Cloaking stuff
#define CLOAK_ACTIVATE_TIME		1.5		// cloak after 1.5 seconds
#define CLOAK_DRAIN				2		// every CLOAK_DRAIN frames,
#define CLOAK_AMMO				1		// drain CLOAK_AMMO amount of cells

/* -------------------------------
      FUNCTION PROTOTYPES
------------------------------- */
// wf_jetpack.c 
void ApplyThrust (edict_t *ent);

// wf_proximity.c
void fire_proximity (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, int proximity_type);

// wf_laserball.c
void fire_laserball (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);

// wf_decoy.c
void SP_Decoy(edict_t *self);

// wf_goodyear.c
void fire_goodyear (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);

// wf_flash.c
void fire_flash (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);

// wf_cluster.c
void fire_cluster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);

// wf_pipebomb.c
void fire_pipebomb (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void Cmd_DetPipes_f (edict_t *ent);

// wf_misc.c
void Cmd_Grenade_f (edict_t *ent);
void Cmd_Grenade1 (edict_t *ent);
void Cmd_Grenade2 (edict_t *ent);
void Cmd_Grenade3 (edict_t *ent);
void Cmd_Thrust_f (edict_t *ent);
void Cmd_Homing_f (edict_t *ent);
void Cmd_Feign_f (edict_t *ent);
void Cmd_Reno_f (edict_t *ent);
void Cmd_WFFlags_f (edict_t *ent);
qboolean K2_IsProtected(edict_t *ent);
void wf_show_grenade_type(edict_t *ent);
qboolean wf_IsWeapon(char *classname);
void wf_InitPlayerClass(gclient_t *client);
void Cmd_DSkin_f (edict_t *ent);
void Cmd_ShowClass (edict_t *ent);
int wf_GetClassModel(int p_class);
void WFPlayer_Die (edict_t *ent);
void WFPlayer_ChangeClassTeam (edict_t *ent);

// b_turret.c
void turret_remove(edict_t *ent);


//wf_lasersight.c
void LaserSightThink (edict_t *self);
void SP_LaserSight(edict_t *self);

// wf_scanner.c 
void Toggle_Scanner (edict_t *ent);
void ShowScanner(edict_t *ent,char *layout);
void ClearScanner(gclient_t *client);
qboolean Pickup_Scanner (edict_t *ent, edict_t *other);

// wf_jet.c
qboolean Jet_AvoidGround( edict_t *ent );
qboolean Jet_Active( edict_t *ent );
void Jet_BecomeExplosion( edict_t *ent, int damage );
void Jet_ApplyJet( edict_t *ent, usercmd_t *ucmd );

// wf_earthquake.c
void fire_earthquake_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);

//wf_turret.c
void grenturret_think2(edict_t *ent);  //Prototypes for grenade turret
void grenturret_think4(edict_t *ent);  //think functions
void Cmd_Turret_f(edict_t *ent);
void grenturret_think4 (edict_t *ent);
void grenturret_think3 (edict_t *ent);
void grenturret_think2 (edict_t *ent);
void grenturret_think1 (edict_t *ent);
void fire_turret_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);

//wf_napalm.c
void fire_napalm (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);

//wf_flame.c
void fire_flame (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);


// Map Rotation Support
// DEFINES

#define MAX_MAPS           32

#define MAX_MAPNAME_LEN    16 

#define ML_ROTATE_SEQ          0 
#define ML_ROTATE_RANDOM       1 
#define ML_ROTATE_NUM_CHOICES  2 
      

// STRUCTURES 
 typedef struct 
{ 
   int  nummaps;          // number of maps in list 
   char mapnames[MAX_MAPS][MAX_MAPNAME_LEN]; 
   char rotationflag;     // set to ML_ROTATE_* 
   int  currentmap;       // index to current map 
   int  votes[MAX_MAPS];
   qboolean  voteonly[MAX_MAPS];  //True if this map should only be
								  //voted on, but not in rotation
   qboolean warning_given; // Have we given people a warning to vote?
   int	active;				//is map list active?
} maplist_t; 

 //Different flame info
 typedef struct
 {
	vec3_t	pos1;
	vec3_t	vel1;
	vec3_t	pos2;
	vec3_t	vel2;
	vec3_t	pos3;
	vec3_t	vel3;
	vec3_t	pos4;
	vec3_t	vel4;
 } Flame_Info;

 // IP ban list
#define MAX_IP_LENGTH 25
//#define MIN_IP_LENGTH 3
#define MIN_IP_LENGTH 1
typedef struct ban_s {
	char ip[MAX_IP_LENGTH + 1];     // store a banned IP
	int subnet;                     // index of last '.' if last IP field
	// is 0 (i.e. subnet is banned)
    struct ban_s *next;             // pointer to next banned IP
} ban_t;

#define MAX_MOTD_LINES		3
#define MAX_BANWORDS		25
#define	MAX_MOTD_LENGTH		60
#define MAX_BANWORDS_LENGTH	40
#define MAX_GAMEINFO_LENGTH	40


typedef struct
{
	char	ref_password[MAX_GAMEINFO_LENGTH];	//password of the ref
	edict_t *ref_ent;			//entity of the ref
	int		game_halted;		//set if scoring and damage is halted
	char	motd[MAX_MOTD_LINES][MAX_MOTD_LENGTH+1];
	char	banwords[MAX_BANWORDS+1][MAX_BANWORDS_LENGTH+1];
	char	classdef_name[MAX_GAMEINFO_LENGTH+1];
	char	stdlog_name[MAX_GAMEINFO_LENGTH+1];
	int		unbalanced_limit;
	int		floodertime;
	int		show_ref_skin;		//Set if ref wants to show as different skin
	char	weaponfile_name[MAX_GAMEINFO_LENGTH+1];
	int		weapon_damage[WEAPON_COUNT + 1];
	int		weapon_speed[WEAPON_COUNT + 1];
	int		grenade_damage[GRENADE_TYPE_COUNT + 1];
	int		grenade_speed[GRENADE_TYPE_COUNT + 1];
	int		ref_picked_map;

} game_info_t;

  
// GLOBALS 
maplist_t		maplist; 
ban_t			*banlist; 
game_info_t		wf_game;
  
// PROTOTYPES 

//wf_maplist.c
int  LoadMapList(edict_t *ent, char *filename); 
void ClearMapList(); 
void Cmd_Maplist_f (edict_t *ent); 
void Display_Maplist_Usage(edict_t *ent);
void ClearMapVotes() ;
int MapMaxVotes();
void VoteForMap(int i);
void DumpMapVotes();
void Cmd_Maplist_f (edict_t *ent);
void MaplistNextMap();

// g_ctf.c
void WFSpecialMenu(edict_t *ent);
void WFMapVote(edict_t *ent);

// g_main.c
void EndDMLevel (void);

// remotecam.c
void cmd_CameraPlace(edict_t *ent);
void cmd_CameraToggle(edict_t *ent);


// fileio.c 
#include "wf_fileio.h" 

//w_laser.c
void cmd_TripBomb(edict_t *self);

//wf_ipban.c
int IsBanned(char *ip);
void ReadBans();
void WriteBans();
void Adm_Ban(char *cmd);
void Adm_Unban(char *cmd);
void Adm_Bans(char *cmd);
void Adm_KickBan(char *cmd);

//misc function prototypes
void SP_SupplyDepot(edict_t *self);
void place_alarm (int number,edict_t *ent);
void cmd_PlasmaBomb(edict_t *ent);
void cmd_LaserDefense(edict_t *ent);
void fire_napalm (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void burn_person(edict_t *target, edict_t *owner, int damage, int mod);
void infect_person(edict_t *target, edict_t *owner);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed);
void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void Grenade_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void Grenade_Explode (edict_t *ent);
void GenericGrenade_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void GenericGrenade_Explode (edict_t *ent);
void Rocket_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void NoAmmoWeaponChange (edict_t *ent);
void ThrowUpNow(edict_t *self);
void TimedNuke_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void DiseaseGrenade_Explode (edict_t *ent);
void fire_concussiongrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_diseasegrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_magnogrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_bulletgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_flare (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_shockgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_freezer (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect);//acrid 3/99

qboolean wf_CanUse(gclient_t *cl, edict_t *ent);
void WFRemoveDisguise(edict_t *ent);
void fire_laser_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void lasersight_off (edict_t *ent);
void lasersight_on (edict_t *ent);
void feign_off (edict_t *ent);
void feign_on (edict_t *ent);
void Cmd_WFPlayTeam (edict_t *self, char *wavename, int all);
void WFOpenClassMenu(edict_t *ent);
void stuffcmd(edict_t *e, char *s);
void Cmd_ShowPlayers(edict_t *ent);
void place_missile (edict_t *ent);
void UpgradeMissileTurret(edict_t *self);
void Create_All_Bases();
qboolean WFPickup_Flag(edict_t *ent, edict_t *other);
char getClassCode (gclient_t *cl);
void WFChooseDisguise(edict_t *ent);
void WFRemoveDisguise(edict_t *ent);
qboolean WFUnbalanced_Teams(edict_t *flag, edict_t *player);
void alarm_remove(edict_t *self);
void SP_HealingDepot(edict_t *self);
void botDebugPrint (char *fmt, ...);


#include "m_player.h"
#include "dwm.h"
#include "q_devels.h"
#include "kamikaze.h"

extern qboolean is_quad;
extern byte is_silenced;

//JR Stuff for flames and a max amount of Fire
#define MAX_FLAMES         60
int TOTALWORLDFLAMES;

//Don't allow players to drop an unlimited number of ammo ents
#define MAX_DROPPEDAMMO		30
int TOTALDROPPEDAMMO;


