//WF - New variables for edict structure

    int       homing_lock;    //0 = not locked, 1 = locked
    edict_t   *lasersight;    //pointer to laser sight
    edict_t   *decoy;         //Pointer to decoy
    edict_t   *creator;       //Who created this (used by decoy)
    edict_t   *missile;    //pointer to laser sight
	int		  wf_team;		  //team of player,decoy, whatever
	qboolean  noteamdamage;	//True if same team can't damage entity

	//Used for item limits
	int		grenade_index;	//if set, this is an index into the active_grenades array
	int		special_index;	//if set, this is an index into the active_special array

	//Used by map entities
	int		bonustype;
	int		bonusvalue;
    edict_t *orig_map_entity;	//points to the spawned map flag return map entity, rather
								//than the one created in wf_flagcap.c
	//Turret data - Imp was here
	float 		turrettime;
	float		turretdie;
	int 		turretammo;
	//int			turretsoundcountdown; //Countdown to when next sound is played
	vec3_t 		targetdir;
	vec3_t		orig_angles;

	//Disguise
	int disguised;
	int FrameShot;
	int disguising;
	float disguisetime;
	int disguisingteam;
	int disguisedteam;
	int disguiseshots;

	//sentry stuff
	edict_t *selectedsentry;
	edict_t *standowner;

	//Means of Death for this entity
	int mod;

	int superslow;
	int lame;	//reduce speed if they were shot in the leg
	int nospeed;
	float sentrydelay;

	int CanFloat;
	float FloatTime;
	int HitTopOfWater;
	int GoneDown;

	//a different flame thrower
	Flame_Info flameinfo;

	int SniperDamage;
	int disease;
	edict_t *diseased_by;
	edict_t *alarm1;
	edict_t *alarm2;
	edict_t *alarm3;
	float PlasmaDelay;
	int Flames;
	float PlayerSnipingZoom;
	edict_t *sentry;
	int ShotNumber;
	edict_t *remotecam;//the remote camera
	edict_t *remote;
	edict_t *supply;
	float DrunkTime;
	int cantmove;	//Set if entity should be frozen
	int Slower;			//for slow grenade and tranquilizer
	float DizzyYaw;
	float DizzyPitch;
	float DizzyRoll;
	vec3_t LockedPosition;
	vec3_t camposition; //Camera viewpoint
	int menutime;
	edict_t *orb;
	int kick;

	// freezer code
	char oldskin[60];
	qboolean frozen;
	qboolean frozenbody;
	int frozentime;
	//WF ACRID 3/99


	//Unused items
	/*
	int snipe;
	int boots;
	int ViewPoint; //1 is back 0 is normal
	int ClassSpeed;
	int TeamNumber;
	int HaveFlag;
	int OnTurret;
	int ShootingTurret;
	int LaserOrbs;
	int FireDamage;
	int LaserBomb;
	int ClassNumber;
	int max_armor;//JR 2/20/98
	edict_t *searchlight;//JR 2/23/98
	edict_t *turret1;//JR 2/25/98
	edict_t *turret2;
	edict_t *cable;
	edict_t *satellite;
	usercmd_t *cmd;
	//SBOF: Chasecam variables
	int                     chasedist1;
    int                     chasedist2;
	vec3_t sndvelocity;
*/

	float delay2;			// used for second timer