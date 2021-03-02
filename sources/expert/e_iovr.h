//==============================
//Expert Quake Overlay Constants
//==============================

#define	EXPERT_UPDATE_FRAMES	8

//debugging
#define OVERLAYVER "Mar 11th, R27"

//Grrr...
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))

//Some coding practices that VC warns against have been proven to cause bugs.
//Therefore, turn on and fix all warnings
#ifdef _WIN32
#pragma warning(default : 4100 4127 4701 4054)
#pragma warning(default : 4136 4051 4018 4305 4244)
#endif

//screen dimensions
//Remember to change MAX_HORIZ/VERT if you changed ScrWidth/Height
#define SCREENS 2
#define INIT_SCREEN 0
char scrPic[SCREENS] = {'1','2'};
short scrWidth[SCREENS] = {32, 64};
short scrHeight[SCREENS] = {32, 64};
#define MAX_HORIZ 64 //highest scrHeight
#define MAX_VERT 64 //highest scrWidth

//overlay help
#define LEGEND_ITEMS "Green dots are weapons. Yellow dots are powerups. Items taken do not disappear from the radar, as that would supply too much knowledge about player movements.\n"

//clMode bitvector
#define RADARON		1 << 1
#define SHOWBACK	1 << 2
#define SHOWWEAP	1 << 3
#define SHOWPOWER	1 << 4
#define SHOWBASE	1 << 5
#define LOCKFIT		1 << 6
#define ZVERT		1 << 7
#define MATRIXON	1 << 9
#define LEARNON		1 << 10

//defaults
#define INIT_HSCALE 100
#define INIT_VSCALE 100
#define INIT_HPOS 4
#define INIT_VPOS 4
#define INIT_MODE (RADARON | SHOWBACK | LEARNON)
#define LEARN_MODE (SHOWWEAP | SHOWPOWER) //default on if gametype is dm or single

//for z axis plotting
#define Y_LAYERSCALE 32
#define ABOVE_FEET 64
#define BELOW_FEET -64

//other
#define PLOT_EDGE 1
#define PLOT_FORCE 2
//#define PLOT_LAYER 4
#define PLOT_LEVEL 8

#define NOPIC 1
#define WEAP 'f' //f
#define ITEM 'p' //p

#define FRIEND 'w'
#define OURBASE 'g'
#define OURCARRIER 'b'

#define ENEMY 'r'
#define ENEMYBASE 'y' 
#define ENEMYCARRIER 'o'

//persistent blips
typedef struct {
	edict_t*	pedTarget;
	byte		teamSeenBy;
	int			expiryframe;
} sightEnt_t;

typedef struct {
	edict_t*	pedTarget;
	char		pic;
} plotEnt_t;

typedef unsigned short ushort;