//
//	Chicken Header
//
#ifndef _CTC_H_
#define _CTC_H_

#include "ctclocal.h"

#define CHICKEN_STRING_VERSION  "2.0"
#define CHICKEN_STRING_FULLNAME "CTC "CHICKEN_STRING_VERSION
#define CHICKEN_STRING_MENU_VERSION  "v"CHICKEN_STRING_VERSION

//
// Positioning stuff for tctc scoreboard
//
#define LEFT_OFFSET		24
#define RIGHT_WIDTH		144
#define DOWN_WIDTH		88
#define PIC_WIDTH		16
#define TOP_OFFSET		15
#define TOP_TEXT_OFFSET (PIC_WIDTH+TOP_OFFSET)


#define INI_FILE				"ctc.ini"
#define MAPLIST_FILE			"maplist.ini"
#define MOD_FILE				"motd.ini"

//
// MIN/MAX values for number of feather
//
#define MAX_FEATHER_FRAMES		15
#define MIN_FEATHER_FRAMES		1

#define TIME_LOW_TIMEOUT        5 // How low chicken hold timer gets before making sounds

//
//	Chicken throw parameters taken from grenade
//
#define CHICKEN_TIMER		2.0
#define CHICKEN_MINSPEED	400
#define CHICKEN_MAXSPEED	600
#define EGG_SPEED			1000	// Speed of egg


//
// Models used in CTC
//
#define MODEL_WEAPON_THROW		"models/weapons/v_throw/tris.md2"
#define MODEL_WEAPON_NORMAL		"models/weapons/v_chick/tris.md2"

#define MODEL_CHICKEN_ITEM		"models/items/chicken/tris.md2"
#define MODEL_CHICKEN_FLY		"models/objects/fly_chick/tris.md2"
#define MODEL_EGG				"models/objects/egg/tris.md2"
#define MODEL_EGG_SPLAT			"models/objects/eggsplat/tris.md2"
#define MODEL_FEATHER			"models/objects/feather/tris.md2"
#define MODEL_EGG_SHELL			"models/objects/debris2/tris.md2"

//
//	Location and name of files required for CTC
//
#define SPACER					" "
#define SOUND_RANDOM1			"chicken/random1.wav"
#define SOUND_RANDOM2			"chicken/random2.wav"
#define SOUND_RANDOM3			"chicken/random3.wav"
#define SOUND_FIRE_WEAPON		"chicken/fire.wav"
#define	SOUND_READY_TO_THROW	""
#define	SOUND_CHICKEN_ANGRY		""
#define SOUND_CHICKEN_SPAWN		"chicken/respawn.wav"
#define SOUND_CHICKEN_RESPAWN	"chicken/respawn.wav"
#define SOUND_CHICKEN_HURT_25	""
#define SOUND_CHICKEN_HURT_50	""
#define SOUND_CHICKEN_HURT_75	""
#define SOUND_CHICKEN_HURT_100	""
#define SOUND_CHICKEN_DIE		"chicken/chickdie.wav"
#define SOUND_GAME_START		"chicken/start.wav"
#define SOUND_GAME_END			"chicken/end.wav"
#define SOUND_MESSAGE			"chicken/message.wav"
#define SOUND_SWIM				""
#define	SOUND_CHICKEN_SCRATCH	""
#define	SOUND_CHICKEN_PECK		"chicken/peck.wav"
#define	SOUND_OBSERVER			"items/protect3.wav"
#define	SOUND_CHICKEN_PICKUP	"chicken/pickup.wav"

#define	SOUND_EGGGUN_READY		""
#define	SOUND_EGGGUN_FIRE		"chicken/egggun.wav"
#define	SOUND_EGG_SPLAT			"chicken/splat.wav"
#define	SOUND_TIME_UP			"chicken/clock/bell.wav"
#define	SOUND_TIME_LOW			"chicken/clock/tick.wav"

#define SOUND_PRECACHE \
			SOUND_RANDOM1			SPACER \
			SOUND_RANDOM2			SPACER \
			SOUND_RANDOM3			SPACER \
			SOUND_FIRE_WEAPON		SPACER \
			SOUND_READY_TO_THROW	SPACER \
			SOUND_CHICKEN_ANGRY		SPACER \
			SOUND_CHICKEN_SPAWN		SPACER \
			SOUND_CHICKEN_RESPAWN	SPACER \
			SOUND_CHICKEN_HURT_25	SPACER \
			SOUND_CHICKEN_HURT_50	SPACER \
			SOUND_CHICKEN_HURT_75	SPACER \
			SOUND_CHICKEN_HURT_100	SPACER \
			SOUND_CHICKEN_DIE		SPACER \
			SOUND_GAME_START		SPACER \
			SOUND_GAME_END			SPACER \
			SOUND_MESSAGE			SPACER \
			SOUND_SWIM				SPACER \
			SOUND_CHICKEN_SCRATCH	SPACER \
			SOUND_CHICKEN_PECK		SPACER \
			SOUND_OBSERVER			SPACER \
			SOUND_CHICKEN_PICKUP    SPACER \
			SOUND_EGGGUN_READY		SPACER \
			SOUND_EGGGUN_FIRE		SPACER \
			SOUND_EGG_SPLAT			SPACER \
			SOUND_TIME_UP			SPACER \
			SOUND_TIME_LOW			

//
// Game parameters accessed by quake functions
//
extern int		chickenGame;		// Has the Chicken game started
extern int		allowSmallHealth;	// Can players pickup small health packs
extern int		allowBigHealth;		// Can players pickup big health packs
extern int		allowInvulnerable;	// Can players be Invulnerable
extern int		allowArmour;		// Can players pickup armour
extern int		allowGlow;			// Can Chicken Player Glow
extern int		menuAllowed;		// Is menu allowed up
extern int		observerAllowed;	// Are players allowed to go into observer mode
extern int		kickback;			// Kickback when players hit by egg

extern int		chickenItemIndex;	// Global index of Chicken item
extern int		eggGunItemIndex;	// Index of egg gun item in item list
extern struct gitem_s *eggGunItem;	// Pointer to egg gun item
extern struct gitem_s *chickenItem;	// Global Pointer to Chicken Item

extern char		ctc_statusbar[];	// Statusbar for CTC

#ifdef STD_LOGGING
extern qboolean	loggingStarted;		// Has Logging begun
#endif

//
// Chicken Teamplay Options
//
extern char		teamPlayerMenuLine[MAX_TEAMS][MENU_ITEM_LINE_LEN];	// Menu line string for each team
extern int		teams;														// How many teams can be in tctc
extern int		teamWithChicken;											// Which team has chicken

//
// Used Quake Functions
//
void	 Touch_Item			(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void	 Weapon_Generic		(edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
void	 NoAmmoWeaponChange	(edict_t *ent);
void	 P_ProjectSource    (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void	 debris_die			(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);	// Chicken Egggun

void	 ClientUserinfoChanged (edict_t *ent, char *userinfo);

edict_t *SelectRandomDeathmatchSpawnPoint	(void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);

//
// Chicken Callable Functions from quake
//
void		Chicken_Drop			(edict_t *ent,struct gitem_s *item);
void		Chicken_Toss			(edict_t *ent,struct gitem_s *item);
void		Chicken_Weapon			(edict_t *ent);
qboolean	Chicken_Pickup			(edict_t *ent, edict_t *other);
qboolean	Chicken_PlayerDie		(edict_t *self, edict_t *inflictor, edict_t *attacker, int meansOfDeath);
void		Chicken_ScoreCheck		(edict_t *self, edict_t *inflictor, edict_t *attacker, int meansOfDeath);
int			Chicken_TakeDamage		(edict_t *self, edict_t *attacker, int damage);
void		Chicken_Spawn			(void);
void		Chicken_Beta			(edict_t *ent);

void		Chicken_Stats			(edict_t *ent);
void		Chicken_GameInit		();
void		Chicken_GoObserver		(edict_t *ent);
void		Chicken_CheckInPlayer	(edict_t *ent);
void		Chicken_ObserverEnd		(edict_t *ent);

void		Chicken_Command			(edict_t *ent);
qboolean	Chicken_AllowPowerUp	(edict_t *ent, edict_t *other);

int			Chicken_Respawn			(void *data, int itemId);
void		Chicken_Banner			(edict_t *ent);
qboolean	Chicken_ShowMenu		(edict_t *ent);
qboolean	Chicken_PlayerReadyEggGun(edict_t *ent, int team);

void		Weapon_Egggun			(edict_t *ent);

void		Chicken_NewPlayer		(edict_t *ent);
void		Chicken_CheckGlow		(edict_t *ent);
void		Chicken_Kill			(edict_t *ent);
void		Chicken_RunFrameEnd		(edict_t *ent);
void		Chicken_CheckOutPlayer	(edict_t *ent);
void		Chicken_CycleLevel		(edict_t *ent);
char		*Chicken_GetModelName	(edict_t *ent);
void		Chicken_ResetScoreboard	();
void		Chicken_ChangeMap		(edict_t *ent);
void		Chicken_MOTD			(edict_t *ent);
void		Chicken_ClientBegin		(edict_t *ent);
qboolean	Chicken_SelectNextItem	(edict_t *ent);
qboolean	Chicken_SelectPrevItem	(edict_t *ent);
qboolean	Chicken_Cheat			(edict_t *ent);
qboolean	Chicken_InvUse			(edict_t *ent);
qboolean	Chicken_ItemTouch		(edict_t *ent, edict_t *other);
qboolean	Chicken_DropMakeTouchable(edict_t *ent);
qboolean	Chicken_CanPickup		(edict_t *ent, int allow);
qboolean	Chicken_Ready			(edict_t *ent);
qboolean	Chicken_InvDrop			(edict_t *ent);
qboolean	Chicken_CheckForTeam	(edict_t *ent, int team);
qboolean	Chicken_CheckPlayerModel(edict_t *ent);
qboolean	Chicken_TossCheck		(edict_t *ent);
qboolean	Chicken_TCTCScoreboard	(edict_t *ent, edict_t *killer);
qboolean	Chicken_CanPickupHealth (edict_t *ent, edict_t *other);

// ### Hentai ### BEGIN
void ShowGun(edict_t *ent);
// ###	Hentai ### END

//
// Teamplay Functions
//
void Chicken_TeamMenuCreate();
void Chicken_PlayerSelectMenuCreate();
void Chicken_TeamReadyEggGun(int team);

#endif

