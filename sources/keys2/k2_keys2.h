//K2:Begin

extern cvar_t	*giveshotgun;			//0 off 1 on
extern cvar_t	*givesupershotgun;		//0 off 1 on
extern cvar_t	*givemachinegun;		//0 off 1 on
extern cvar_t	*givechaingun;			//0 off 1 on
extern cvar_t	*givegrenadelauncher;	//0 off 1 on
extern cvar_t	*giverocketlauncher;	//0 off 1 on
extern cvar_t	*giverailgun;			//0 off 1 on
extern cvar_t	*givehyperblaster;		//0 off 1 on
extern cvar_t	*givebfg;				//0 off 1 on
extern cvar_t	*nobfg;					//0 off 1 on
extern cvar_t	*noshotgun;				//0 off 1 on
extern cvar_t	*nosupershotgun;		//0 off 1 on
extern cvar_t	*nomachinegun;			//0 off 1 on
extern cvar_t	*nochaingun;			//0 off 1 on
extern cvar_t	*nogrenadelauncher;		//0 off 1 on
extern cvar_t	*norocketlauncher;		//0 off 1 on
extern cvar_t	*nohyperblaster;		//0 off 1 on
extern cvar_t	*norailgun;				//0 off 1 on
extern cvar_t	*nomegahealth;			//0 off 1 on
extern cvar_t	*noquad;				//0 off 1 on
extern cvar_t	*noinvulnerability;		//0 off 1 on
extern cvar_t	*swaat;				//0 off 1 on
extern cvar_t	*startinghealth;		//100 - 1000
extern cvar_t	*startingshells;		//100 - 1000
extern cvar_t	*startingbullets;		//100 - 1000
extern cvar_t	*startinggrenades;		//100 - 1000
extern cvar_t	*startingrockets;		//100 - 1000
extern cvar_t	*startingslugs;			//100 - 1000
extern cvar_t	*startingcells;			//100 - 1000
extern cvar_t	*maxhealth;				//100 - 1000
extern cvar_t	*maxshells;				//100 - 1000
extern cvar_t	*maxbullets;			//100 - 1000
extern cvar_t	*maxgrenades;			//100 - 1000
extern cvar_t	*maxrockets;			//100 - 1000
extern cvar_t	*maxslugs;				//100 - 1000
extern cvar_t	*maxcells;				//100 - 1000
extern cvar_t	*startingweapon;		//100 - 1000
extern cvar_t	*startingarmorcount;	//100 - 1000
extern cvar_t	*startingarmortype;		//100 - 1000
extern cvar_t	*hook_time;				//0 = infinite 999 max
extern cvar_t	*hook_speed;			//100 - 1400 Hook velocity
extern cvar_t	*hook_damage;			//2 - 50 Damage done by hook
extern cvar_t	*pull_speed;			//100 - 1400 How fast the player is pulled;
extern cvar_t	*skyhook;			//100 - 1400 How fast the player is pulled;
extern cvar_t	*protecttime;			//5 - 30
extern cvar_t	*gibtime;				//10 - 60
extern cvar_t	*burntime;				//10 - 60				
extern cvar_t	*blindtime;				//Value is multiplied by 10 to come up with blind frames

extern cvar_t	*freezetime;				//Value is multiplied by 10 to come up with freeze frames
extern cvar_t	*flash_radius;			//???
extern cvar_t	*freeze_radius;			//???
extern cvar_t	*pickuptime;		//???
extern cvar_t	*gibdamage;				//2 - 10
extern cvar_t	*burndamage;			//2 - 10
extern	cvar_t	*regentime;				//30 - 300
extern	cvar_t	*hastetime;				//30 - 300
extern	cvar_t	*futilitytime;			//30 - 300
extern	cvar_t	*inflictiontime;		//30 - 300
extern	cvar_t	*bfktime;				//30 - 300
extern	cvar_t	*stealthtime;			//30 - 300
extern	cvar_t	*homingtime;			//30 - 300
extern	cvar_t	*antitime;				//30 - 300
extern	cvar_t	*regeneration;	//0 - 4 for number of keys
extern	cvar_t	*haste;			//0 - 4 for number of keys
extern	cvar_t	*futility;		//0 - 4 for number of keys
extern	cvar_t	*infliction;		//0 - 4 for number of keys
extern	cvar_t	*bfk;			//0 - 4 for number of keys
extern	cvar_t	*stealth;		//0 - 4 for number of keys
extern	cvar_t	*antikey;		//0 - 4 for number of keys
extern	cvar_t	*homing;			//0 - 4 for number of keys
extern	cvar_t	*droppable;			//0 off 1 on
extern	cvar_t	*playershells;		//0 off 1 on
extern	cvar_t	*keyshells;			//0 off 1 on
extern	cvar_t	*respawntime;		//1 or 2 minutes
extern	cvar_t	*qwfraglog;			//0 off 1 on

extern	cvar_t	*levelcycle;			//0 off 1 random 2 sequential
extern	cvar_t	*resetlevels;
extern	cvar_t	*pickupannounce;		//0 off 1 on
extern	cvar_t	*gibgun	;			//0 off 1 on
extern	cvar_t	*flashgrenades;		//0 off 1 on
extern	cvar_t	*freezegrenades;		//0 off 1 on
extern	cvar_t	*firegrenades;		//0 off 1 on
extern	cvar_t	*firerockets;		//0 off 1 on
extern	cvar_t	*drunkrockets;		//0 off 1 on
extern	cvar_t	*hominghyperblaster;		//0 off 1 on
extern	cvar_t	*allowfeigning;		//0 off 1 on
extern	cvar_t	*usevwep;		//0 off 1 on
extern	cvar_t	*damagemultiply;		//0 off 1 on
extern	cvar_t	*motd1;
extern	cvar_t	*motd2;
extern	cvar_t	*motd3;
extern	cvar_t	*motd4;
extern	cvar_t	*motd5;
extern	cvar_t	*motd6;
extern	cvar_t	*motd7;

extern	cvar_t	*nextleveldelay;
//weapon damage
extern cvar_t	*bfgdamage;
extern cvar_t	*raildamage;
extern cvar_t	*supershotgundamage;
extern cvar_t	*shotgundamage;
extern cvar_t	*chaingundamage;
extern cvar_t	*machinegundamage;
extern cvar_t	*hyperdamage;
extern cvar_t	*blasterdamage;
extern cvar_t	*rocketdamage;
extern cvar_t	*rocketradiusdamage;
extern cvar_t	*rocketdamageradius;
extern cvar_t	*grenadelauncherdamage;
extern cvar_t	*handgrenadedamage;
extern cvar_t	*totalstealth;
extern cvar_t	*nozbots;
extern cvar_t	*maxbots;
extern cvar_t	*botfraglogging;
extern cvar_t   *connectlogging;

//K2 Global vars
extern	float	k2_keyframes;		//Used for counting down timed keys
extern	int		k2_timeleft;
extern	int		k2_fragsleft;
extern  float	nextlevelstart;
extern	int		k2_capsleft;
extern	float	qversion;
extern	FILE	*QWLogFile;

//HUD Stats
#define	STAT_KEY_ICON			16
#define	STAT_KEYTIMER			28
#define STAT_TIMELEFT			29
#define STAT_FRAGSLEFT			30
#define	STAT_RANK				31

//New Means of Death
#define	MOD_GIBGUN				34
#define	MOD_FLAME				35
#define MOD_FLASH				36
#define	MOD_DRUNK				37
#define	MOD_DRUNK_SPLASH		38
#define MOD_REVERSE_TELEFRAG	39
#define MOD_HOMING_BLASTER		40
#define MOD_HOMING_HYPERBLASTER	41
#define MOD_HOMING_SPLASH		42
#define MOD_HOMING_ROCKET		43
#define	MOD_FREEZE				44			

//Key definitions and k2keys->value flags
#define KEY_ANTIKEY			1
#define	KEY_REGENERATION	2
#define KEY_FUTILITY		4
#define	KEY_INFLICTION		8
#define KEY_HASTE			16
#define KEY_STEALTH			32
#define KEY_HOMING			64
#define	KEY_BFK				128

//shell colors
#define	RF_REDSHELL			RF_SHELL_RED
#define	RF_BLUESHELL		RF_SHELL_BLUE
#define	RF_GREENSHELL		RF_SHELL_GREEN
#define	RF_AQUASHELL		RF_SHELL_BLUE
#define	RF_PURPLESHELL		RF_SHELL_RED
#define	RF_YELLOWSHELL		RF_SHELL_GREEN
#define	RF_WHITESHELL		(RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE)
#define EF_K2COLORSHELL		(0x08000000|EF_COLOR_SHELL)

#define EF_FROZEN	 0x01000000


void		K2_InitClientVars(edict_t *ent);
void		K2_ResetClientKeyVars(edict_t *ent);
void		K2_Drop_Key (edict_t *ent, gitem_t *item);
void		K2_SpawnAllKeys (void);
void		K2_HomingThink (edict_t *self);
void		K2_SpawnKey (edict_t *self, int key, int spawnflag);
void		K2_RemoveKeyFromInventory (edict_t *ent);
void		K2_droptofloor (edict_t *ent);
void		K2_Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void		K2_RespawnKey (edict_t *ent);
void		K2_UseKey(edict_t *ent, gitem_t *item);
void		K2_HomeTarget (edict_t *self);
float		PlayersRangeFromSpot (edict_t *spot);
edict_t		*K2_SelectRandomDeathmatchSpawnPoint (void);
void		K2_BonusFrags(edict_t *self, edict_t *inflictor, edict_t *attacker);
void		K2_DropKeyCommand (edict_t *self);
void		K2_DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer);
void		K2_TakePlayerKey(edict_t *ent);
void		K2_InitClientPersistant (gclient_t *client);
void		K2_SetClientStats(edict_t *ent, qboolean hasPowerup);
void		K2_SetClientEffects(edict_t *ent);
float		K2_SetClientKeyTimer(int key);
void		K2_ChaseCam(edict_t *ent, pmenu_t *p);
qboolean	K2_StartClient(edict_t *ent);
void		K2_Regeneration(edict_t *ent);
void		K2_KeyExpiredCheck(edict_t *ent);
void		K2_ClearPrevOwner(edict_t *ent);

//Player key status checking
qboolean	K2_IsHaste(edict_t *ent);
qboolean	K2_IsRegen(edict_t *ent);
qboolean	K2_IsInfliction(edict_t *ent);
qboolean	K2_IsFutility(edict_t *ent);
qboolean	K2_IsHoming(edict_t *ent);
qboolean	K2_IsBFK(edict_t *ent);
qboolean	K2_IsAnti(edict_t *ent);
qboolean	K2_IsStealth(edict_t *ent);
int			K2_KeyType(edict_t *ent);

qboolean	K2_IsProtected(edict_t *ent);
qboolean	K2_CanPickupKey(edict_t *ent);

//QW Logging
void WriteQWLogDeath(edict_t *self, edict_t *inflictor, edict_t *attacker);

// ******** LEVELCYCLE ***********
void K2_ReadDMLevelCycleFile ( void );
void K2_ReadCTFLevelCycleFile ( void );
qboolean K2_CheckLevelCycle ( void );

//
// hook.c
//
qboolean Started_Grappling(gclient_t *client);
qboolean Ended_Grappling(gclient_t *client);
qboolean Is_Grappling(gclient_t *client);
void Throw_Grapple(edict_t *ent);
void Release_Grapple(edict_t *ent);
void Grapple_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void Think_Grapple(edict_t *ent);
void Make_Hook(edict_t *ent);
void Pull_Grapple(edict_t *ent);
void P_ProjectHookSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);


//feign.c
void Client_EndFeign (edict_t *ent);
void Client_BeginFeign (edict_t *self);
void TossClientWeapon (edict_t *self);

//k2_weapons.c
//
//Flash Grenades
enum {
	GRENADE_NORMAL,
    GRENADE_FLASH,
	GRENADE_FIRE,
	GRENADE_FREEZE
};
void Flash_Explode (edict_t *ent);
void Flash_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void Freeze_Explode (edict_t *ent);
void Freeze_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

//Fire Weapons
void K2_Fire_Radius_Explosions(edict_t *ent);
void K2_FlameBurnDamage(edict_t *ent);
void K2_BurnPlayer(edict_t *ent, edict_t *attacker);

//GIBGUN
enum {
	BLASTER_NORMAL,
	BLASTER_GIBGUN
};
void K2_FireGibGun(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void K2_GibGunTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void K2_GibGunThink(edict_t *ent);

//Killable Rockets
void Rocket_Explode (edict_t *ent);
void Rocket_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

//Drunk Rockets
enum {
	ROCKET_NORMAL,
	ROCKET_DRUNK,
	ROCKET_FIRE
};
void	fire_rocket_drunk(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void	Rocket_DrunkThink (edict_t *self);
//This function is not defined anywehere else, but is standard Quake2 function
void	rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

//Used to fix NULL ent->think errors with EndDMLevel
//Not necessarily a fix, but a band-aid
void	DummyThink(edict_t*ent);

//Fix Weapon cycling using prev and next weapon commands
void		Cycle_Weapon (edict_t *ent, gitem_t *inv);

//Stuff commands to clients
void stuffcmd(edict_t *ent, char *s);

//K2: Menu
void K2Menu_Open(edict_t *ent, pmenu_t *entries, int cur, int num);
void K2Menu_Close(edict_t *ent);
void K2Menu_Update(edict_t *ent);
void K2Menu_Next(edict_t *ent);
void K2Menu_Prev(edict_t *ent);
void K2Menu_Select(edict_t *ent);
void K2EnterGame(edict_t *ent, pmenu_t *p);
void K2_OpenJoinMenu(edict_t *ent);
void K2Credits(edict_t *ent, pmenu_t *p);
void K2ReturnToMain(edict_t *ent, pmenu_t *p);
void K2ReturnToHelp(edict_t *ent, pmenu_t *p);
void K2_OpenHelpMenu(edict_t *ent, pmenu_t *p);
void K2_OpenKeysMenu(edict_t *ent, pmenu_t *p);
void K2_OpenKeysMenu2(edict_t *ent, pmenu_t *p);
void K2_OpenKeysMenu3(edict_t *ent, pmenu_t *p);
void K2_OpenWeaponsMenu(edict_t *ent, pmenu_t *p);
void K2_OpenCommandsMenu(edict_t *ent, pmenu_t *p);
int	 K2UpdateJoinMenu(edict_t *ent);
int	 K2UpdateWelcomeMenu(edict_t *ent);
void K2_OpenWelcomeMenu(edict_t *ent);

//Bot stuff
qboolean Pickup_Key (edict_t *ent, edict_t *other);
int K2_botCompareKey(edict_t *ent, int key);
qboolean K2_botCheckKeyTimer(edict_t *self);
void fire_homing_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper);
void fire_homing_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void K2_botBFKInformDanger(edict_t *ent);
void K2_FixGSLogFile(void);


void K2_FreezeEffect(edict_t *ent);
void K2_LogPlayerIP(char *userinfo);
void K2ApplyHasteSound(edict_t *ent);

void K2MenuCameraCommand(edict_t *ent, pmenu_t *p);
