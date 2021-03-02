// WF - Added variables for client structure


	// Jetpack stuff (NOTE: take out unused stuff)
    float        Jet_framenum;   /*burn out time when jet is activated*/
    float        Jet_remaining;  /*remaining fuel time*/
    float        Jet_next_think; 

    // GREGG/MUCE: added for jetpack thrusting.
    qboolean	thrusting;              // 1 on 0 off
    float		next_thrust_sound;
    float       blindTime, blindBase; //for flash grenades

    //Respawn protection
    float protecttime;      //How long to protect (in frames)

	//Flood protection & spamming
	int		floodtime1;
	int		floodtime2;

    //Player class variables
	int grenade_type1;
	int grenade_type2;
	int grenade_type3;
	int player_armor;
	int player_model;
	int player_ammo;
	int player_speed;
	int player_maxhealth;
	int player_special;
	int player_items;

	//Camera variables
	//SBOF: chasecam variables
    int             chasetoggle;
    edict_t         *chasecam;
    edict_t         *oldplayer;
	// JDB: new variable for lowlight vision 4/4/98     
	qboolean        lowlight;
	int				remotetoggle;

	//kamikaze support
	int		kamikaze_mode;
	float	kamikaze_framenum; 
	float	kamikaze_timeleft;

	//Give a menu structure to each player so it
	//isn't changed between players
	#define MAX_SPECIAL_MENU_ITEMS 12

	pmenu_t wfspecial[MAX_SPECIAL_MENU_ITEMS];
	char wfmenustr[MAX_SPECIAL_MENU_ITEMS][64];

	pmenu_t sentrymenu[MAX_SPECIAL_MENU_ITEMS];
	char wfsentrystr[MAX_SPECIAL_MENU_ITEMS][64];

	//Id function arrays. Hardcoded for no particular reason. 
	pmenu_t iddisplay[30];
	char wfidstr[30][64];
	
	//Tracker target
	edict_t	*tracker_target;

	//Gas grenade settings
	float	Gas_Time;
	float	Gas_Delay;
	float	GasR;
	float	GasG;
	float	GasB;
	vec3_t	GasViewOffset;
	int		GasFOV;

	//Weapon damage
	int		weapon_damage;

	int		silenced;		//doesn't allow a player to talk or play sounds

	//NEWGrapple Variables 4/99
	edict_t        *hook;
	edict_t        *hook_touch;
	qboolean       on_hook;
	int            hook_frame;

	// cloaking
	qboolean	cloaking;
	float		cloaktime;
	int			cloakdrain;
