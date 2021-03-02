/*
===================
Render Effects
===================
*/
#define RF_SHELL_CYAN		RF_SHELL_GREEN|RF_SHELL_BLUE
#define RF_SHELL_MAGENTA	RF_SHELL_RED|RF_SHELL_BLUE
#define RF_SHELL_YELLOW		RF_SHELL_RED|RF_SHELL_GREEN
#define RF_SHELL_WHITE		RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE

/*
===================
entity->flags
===================
*/
#define JUMP_TIME 2
/*
===================
Head up Display
===================
*/
#define STAT_STAMINA			18
#define STAT_STAMINA_ICON		19
#define STAT_SNIPERCROSS		20
#define STAT_SNIPER_OK			31
#define STAT_TIMER2_ICON		21
#define STAT_TIMER2				22
#define STAT_DAMAGELEVEL		23
#define STAT_DAMAGELEVEL_ICON	24
#define STAT_HEALTHLEVEL		25
#define STAT_HEALTHLEVEL_ICON	26
#define STAT_STAMINALEVEL		27
#define STAT_STAMINALEVEL_ICON	28
#define STAT_TEAM_RED_SCORE		29
#define STAT_TEAM_BLUE_SCORE	30

#define STAT_CLEARSHOT			32//is this possible as maxstats is 32?
								  //i think we'll be getting rid of armour, so we can use those 2
								  //for the "clear shot" icon as well.
/*
===================
Upgrade Types
===================
*/
#define HEALTH	0
#define DAMAGE	1
#define STAMINA	2
#define SPEED	3
/*
===================
Hit Locations
===================
*/
typedef enum
{
	LOC_NULL,
	LOC_LEFTARM,
	LOC_RIGHTARM,
	LOC_LEFTLEG,
	LOC_RIGHTLEG,
	LOC_CHEST,
	LOC_BACK,
	LOC_FACE,
	LOC_HEAD
} location_t;
/*
#define		LOC_NULL		0
#define		LOC_LEFTARM		1
#define		LOC_RIGHTARM	2
#define		LOC_LEFTLEG		3
#define		LOC_RIGHTLEG	4
#define		LOC_CHEST		5
#define		LOC_BACK		6
#define		LOC_FACE		7
#define		LOC_HEAD		8*/

/*
===================
Flip Types
===================
*/
/*
===================
MATRIXFLAGS BUTTONS
six is loads too many
===================
*/
#define BUTTON_0	1
#define BUTTON_1	2
#define BUTTON_2	4
#define BUTTON_3	8
#define BUTTON_4	16
#define BUTTON_5	32


typedef enum
{
	NOFLIP,
	LFLIP,
	RFLIP,
	BFLIP,
	FFLIP,
	RWALL,
	FALLROLL,
	RUN_RIGHT_ALONG_WALL,
	RUN_LEFT_ALONG_WALL,
	KUNGFU
} flip_t;
typedef enum
{
	STRAIGHTUP,
	LEFT,
	RIGHT,
	CEILING
} wallruntype;

typedef enum //for kungfu animations
{
	NONE,
	JAB,
	HOOK,
	HOVERKICK,
	SPINKICK,
	UPPERCUT
} kungfu_type;


#define IT_STAMINA		64



/*
===================
Teamplay Shit
===================
*/
#define TEAM_RED		1
#define TEAM_BLUE		2
#define TEAM_FREE		3

/*
===================
Weapon Muzzle Flash

Just Renamed old weapon
flashes to make the code clearer 
===================
*/
#define MZ_AK_MK23		16

#define IT_AKIMBO		64
