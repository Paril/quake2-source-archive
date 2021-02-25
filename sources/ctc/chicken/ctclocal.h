//
//	Chicken Shared Header
//
#ifndef _CTCLOCAL_
#define _CTCLOCAL_

#include "menu.h"

//
// Model Details Used in CTC
//
typedef struct
{
	char	*playerModel;
	char	*menuText;
	char	*playerGender;
	char	*playerModelPath;
	char	*chickenModel;
	char	*eggGunModel;
} ModelDetails;

// Must match order	{"Red","Blue","Yellow","White","Green","Pink","Aqua","Black"};

typedef struct
{
	char	*colour;
	char	*skin;
	char	*icon;
	char	*holdIcon;
	char	*scoreIcon;
	char	*scoreHoldIcon;
	int		score;
	int		players;
	char	menu[MENU_ITEM_LINE_LEN];
} TeamDetails;

extern TeamDetails	teamDetails[];
extern ModelDetails playerModels[];

typedef struct NextLevel
{
	char				*mapName;
	struct NextLevel	*next;
} LevelName;


#define MAX_TEAMS				4
#define MAX_PLAYER_MODELS		3

//
// Statusbar defines
//
#define STAT_DISPLAY_CLOCK			16	// Display Clock
#define STAT_CHICKEN_OBSERVER		17	// Is player an Observer

#define STAT_CHICKEN_TIME_2SEC		18
#define STAT_CHICKEN_TIME_1SEC		19
#define STAT_CHICKEN_COLON			20
#define STAT_CHICKEN_TIME_MIN		21
#define STAT_CHICKEN_CLOCK			22

#define STAT_SCORES					23

#define STAT_PIC_TEAM1				24
#define STAT_PIC_TEAM2				25
#define STAT_PIC_TEAM3				26
#define STAT_PIC_TEAM4				27

#define STAT_DISPLAY_SCORE1			28	
#define STAT_DISPLAY_SCORE2			29	
#define STAT_DISPLAY_SCORE3			30	
#define STAT_DISPLAY_SCORE4			31

#define	STAT_SCOREBOARD				STAT_FRAGS // Run out of STATS start re-using

#define ICON_CLOCK					"clock"
#define ICON_CLOCK_COLON			"num_colon"
#define ICON_CHICKEN_WEAPON			"w_chicken"

#endif

