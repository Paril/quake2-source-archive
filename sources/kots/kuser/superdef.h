//*************************************************************************************
//*************************************************************************************
// File: superdef.h
// All levels of super powers can be changed here
//*************************************************************************************
//*************************************************************************************

#define KOTS_MAX_SAMEKILL 5
#define KOTS_STREAK_COUNT 5
#define KOTS_STREAK_BONUS 10
#define KOTS_LEVEL_BONUS  20 // if greater than 8 people playing

#define KOTS_2FER_FRAME  5
#define KOTS_2FER_BONUS	10

//*************************************************************************************
//*************************************************************************************
// Function: Level bonuses
//*************************************************************************************
//*************************************************************************************

#define KOTS_LEVEL_RESPAWN 5
#define KOTS_LEVEL_DAMAGE  7
#define KOTS_LEVEL_ARMOR   8

#define KOTS_LEVEL_ARMOR_BONUS 50

// health & ammo
#define KOTS_HEALTH_BASE 100 
#define KOTS_ARMOR_BASE  200 

//*************************************************************************************
//*************************************************************************************
// Defines: Weapons
//*************************************************************************************
//*************************************************************************************

// sword
#define KOTS_SWORD_BASE_DAMAGE 80
#define KOTS_SWORD_BASE_LENGTH 80

#define KOTS_SWORD_15X_LENGTH 1
#define KOTS_SWORD_SILENT     2
#define KOTS_SWORD_2X_DAMAGE  3
#define KOTS_SWORD_2X_LENGTH  4
#define KOTS_SWORD_3X_DAMAGE  5
#define KOTS_SWORD_4X_DAMAGE  6

// machine gun
#define KOTS_MGUN_BASE_DAMAGE   8
#define KOTS_MGUN_TRACER_DAMAGE 50
#define KOTS_MGUN_TRACER_SPEED  1500

#define KOTS_MGUN_2X_DAMAGE  2
#define KOTS_MGUN_3X_DAMAGE  3
#define KOTS_MGUN_SILENT     4
#define KOTS_MGUN_TRACERS    5
#define KOTS_MGUN_4X_DAMAGE  6
#define KOTS_MGUN_5X_DAMAGE  7

// shotgun
#define KOTS_SHOTGUN_BASE_DAMAGE  4
#define KOTS_SHOTGUN_BULLET_COUNT 18
#define KOTS_SHOTGUN_BULLET_SHOT  1.2

#define KOTS_SHOTGUN_2X_DAMAGE  3
#define KOTS_SHOTGUN_2X_SHOT    4
#define KOTS_SHOTGUN_3X_DAMAGE  5
#define KOTS_SHOTGUN_SILENT     6
#define KOTS_SHOTGUN_4X_DAMAGE  7
#define KOTS_SHOTGUN_5X_DAMAGE  8

// hand grenade
#define KOTS_HGRENADE_BASE_DAMAGE 125

#define KOTS_HGRENADE_BASE_SPEED  800
#define KOTS_HGRENADE_BASE_SPEED2 1000

#define KOTS_HGRENADE_DAMAGE1    4
#define KOTS_HGRENADE_TRAILS     5
#define KOTS_HGRENADE_DAMAGE2    6
#define KOTS_HGRENADE_NOEXPLODE  7
#define KOTS_HGRENADE_DAMAGE3    8
#define KOTS_HGRENADE_DAMAGE4    9

// super shotgun
#define KOTS_SSHOTGUN_BASE_DAMAGE 6

#define KOTS_SSHOTGUN_1X_RADIUS  5
#define KOTS_SSHOTGUN_SILENT     6
#define KOTS_SSHOTGUN_2X_RADIUS  7
#define KOTS_SSHOTGUN_2X_DAMAGE  8
#define KOTS_SSHOTGUN_3X_DAMAGE  9
#define KOTS_SSHOTGUN_4X_DAMAGE  10

// chain gun
#define KOTS_CGUN_BASE_DAMAGE 6
#define KOTS_CGUN_TRACER_DAMAGE 30
#define KOTS_CGUN_TRACER_SPEED  1500

#define KOTS_CGUN_2X_DAMAGE 6 
#define KOTS_CGUN_NOSPIN    7 
#define KOTS_CGUN_SILENT    8 
#define KOTS_CGUN_TRACERS   9 
#define KOTS_CGUN_3X_DAMAGE 10
#define KOTS_CGUN_4X_DAMAGE 11

// grenade launcher
#define KOTS_GLAUNCHER_BASE_DAMAGE 120
#define KOTS_GLAUNCHER_BASE_SPEED  600
#define KOTS_GLAUNCHER_BASE_SPEED2 800
#define KOTS_GLAUNCHER_TIMER       2.5

#define KOTS_GLAUNCHER_15X_DAMAGE 7 
#define KOTS_GLAUNCHER_2X_BLAST   8 
#define KOTS_GLAUNCHER_SILENT     9 
#define KOTS_GLAUNCHER_2X_DAMAGE  10
#define KOTS_GLAUNCHER_2X_SPEED   11
#define KOTS_GLAUNCHER_TRAILS     12
#define KOTS_GLAUNCHER_3X_DAMAGE  13

// rocket launcer
#define KOTS_RLAUNCHER_BASE_DAMAGE 120
#define KOTS_RLAUNCHER_BASE_SPEED  650
#define KOTS_RLAUNCHER_BASE_SPEED2 800

#define KOTS_RLAUNCHER_SILENT    8 
#define KOTS_RLAUNCHER_TRAILS    9 
#define KOTS_RLAUNCHER_2X_BLAST  10
#define KOTS_RLAUNCHER_2X_SPEED  11
#define KOTS_RLAUNCHER_2X_DAMAGE 12
#define KOTS_RLAUNCHER_3X_DAMAGE 13

// hyperblaster
#define KOTS_HYPER_BASE_DAMAGE 20
#define KOTS_HYPER_BASE_SPEED  1000
#define KOTS_HYPER_BASE_SPEED2 1500

#define KOTS_HYPER_SILENT     9 
#define KOTS_HYPER_15X_DAMAGE 10
#define KOTS_HYPER_2X_DAMAGE  11
#define KOTS_HYPER_2X_SPEED   12
#define KOTS_HYPER_3X_DAMAGE  13
#define KOTS_HYPER_4X_DAMAGE  14

// railgun
#define KOTS_RAILGUN_BASE_DAMAGE 100
																 
#define KOTS_RAILGUN_SILENT     10
#define KOTS_RAILGUN_15X_DAMAGE 11
#define KOTS_RAILGUN_HASTE      12
#define KOTS_RAILGUN_2X_DAMAGE  13
#define KOTS_RAILGUN_3X_DAMAGE  14
#define KOTS_RAILGUN_4X_DAMAGE  15

// bfg
#define KOTS_BFG_BASE_DAMAGE 35
														
#define KOTS_BFG_15X_DAMAGE 11
#define KOTS_BFG_SILENT     12
#define KOTS_BFG_2X_DAMAGE  13
#define KOTS_BFG_LONGER     14
#define KOTS_BFG_3X_DAMAGE  15
#define KOTS_BFG_4X_DAMAGE  16

// armor
#define KOTS_ARMOR_BONUS 10

//*************************************************************************************
//*************************************************************************************
// Defines: Players
//*************************************************************************************
//*************************************************************************************

// dexterity
#define HOOK_ON		0x00000001		// set if hook command is active
#define HOOK_IN		0x00000002		// set if hook has attached
#define SHRINK_ON	0x00000004		// set if shrink chain is active 
#define GROW_ON		0x00000008		// set if grow chain is active

#define KOTS_DEXTERITY_HOOK_COST  5 //power cost for firing hook

#define KOTS_DEXTERITY_SWITCHING  1 
#define KOTS_DEXTERITY_SWALK	    2 
#define KOTS_DEXTERITY_SPICKUP    3 
#define KOTS_DEXTERITY_GRAPPLE    4 
#define KOTS_DEXTERITY_SJUMP	    5 
#define KOTS_DEXTERITY_25XFALLING 6 
#define KOTS_DEXTERITY_25XWATER   7 
#define KOTS_DEXTERITY_CREDITS    8

// strength
#define KOTS_STRENGTH_HJUMP_BASE 500 // gravity for jump

#define KOTS_STRENGTH_2XAMMO	 3	
#define KOTS_STRENGTH_3XAMMO	 4	
#define KOTS_STRENGTH_HJUMP	   5	
#define KOTS_STRENGTH_4XAMMO	 6	
#define KOTS_STRENGTH_25XGREN  7	
#define KOTS_STRENGTH_KNOCK    8	

// karma
#define KOTS_KARMA_MEGA_CELLS  25

#define KOTS_KARMA_ID		    5
#define KOTS_KARMA_HEALTHID	6
#define KOTS_KARMA_REGEN	  7
#define KOTS_KARMA_MEGA		  8
#define KOTS_KARMA_2XREGEN	9
#define	KOTS_KARMA_BOOST    10

// techno
#define KOTS_TECHNO_ARMOR_BASE 1
#define KOTS_TECHNO_ARMOR_2X	 2
#define KOTS_TECHNO_ARMOR_3X   3

#define KOTS_TECHNO_25XSLIME	 7 
#define KOTS_TECHNO_25XLAVA	   8 
#define KOTS_TECHNO_2XARMOR	   9 
#define KOTS_TECHNO_LASERSITE	 10
#define KOTS_TECHNO_3XARMOR	   11
#define KOTS_TECHNO_FLASHLIGHT 12	

// wisdom
#define KOTS_WISDOM_RESIST_BASE  0.75 // user only takes 75% of damage
#define KOTS_WISDOM_VAMPIRE_BASE 0.25 // user receive 25% of damage inflicted
#define KOTS_WISDOM_CLOAK_DRAIN  10   // every CLOAK_DRAIN frames,
#define KOTS_WISDOM_CLOAK_AMMO   1    // drain CLOAK_AMMO amount of power
#define KOTS_WISDOM_FLY_DRAIN    10   // every FLY_DRAIN frames,
#define KOTS_WISDOM_FLY_AMMO     1    // drain FLY_AMMO amount of power

#define KOTS_WISDOM_VAMPIRE	  9 
#define KOTS_WISDOM_RESIST    10
#define KOTS_WISDOM_25XROCKET 11
#define KOTS_WISDOM_FLY		    12
#define KOTS_WISDOM_CLOAK     13

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************

