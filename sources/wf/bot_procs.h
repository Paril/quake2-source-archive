/*****************************************************************

	Eraser Bot source code - by Ryan Feltrin, Added to by Acrid-

	..............................................................

	This file is Copyright(c) 1998, Ryan Feltrin, All Rights Reserved.

	..............................................................

	All other files are Copyright(c) Id Software, Inc.

	Please see liscense.txt in the source directory for the copyright
	information regarding those files belonging to Id Software, Inc.

	..............................................................
	
	Should you decide to release a modified version of Eraser, you MUST
	include the following text (minus the BEGIN and END lines) in the 
	documentation for your modification.

	--- BEGIN ---

	The Eraser Bot is a product of Ryan Feltrin, and is available from
	the Eraser Bot homepage, at http://impact.frag.com.

	This program is a modification of the Eraser Bot, and is therefore
	in NO WAY supported by Ryan Feltrin.

	This program MUST NOT be sold in ANY form. If you have paid for 
	this product, you should contact Ryan Feltrin immediately, via
	the Eraser Bot homepage.

	--- END ---

	..............................................................

	You will find p_trail.c has not been included with the Eraser
	source code release. This is NOT an error. I am unable to 
	distribute this file because it contains code that is bound by
	legal documents, and signed by myself, never to be released
	to the public. Sorry guys, but law is law.

	I have therefore include the compiled version of these files
	in .obj form in the src\Release and src\Debug directories.
	So while you cannot edit and debug code within these files,
	you can still compile this source as-is. Although these will only
	work in MSVC v5.0, linux versions can be made available upon
	request.

	NOTE: When compiling this source, you will get a warning
	message from the compiler, regarding the missing p_trail.c
	file. Just ignore it, it will still compile fine.

	..............................................................

	I, Ryan Feltrin/Acrid-, hold no responsibility for any harm caused by the
	use of this source code. I also am NOT willing to provide any form
	of help or support for this source code. It is provided as-is,
	as a service by me, with no documentation, other then the comments
	contained within the code. If you have any queries, I suggest you
	visit the "official" Eraser source web-board, at
	http://www.telefragged.com/epidemic/. I will stop by there from
	time to time, to answer questions and help with any problems that
	may arise.

	Otherwise, have fun, and I look forward to seeing what can be done
	with this.

	-Ryan Feltrin
	-Acrid-

 *****************************************************************/
#define	ERASER_VERSION		1.01

#define	MAX_BOTS	25

#define	STEPSIZE	24
#define	BOT_RUN_SPEED		300
#define BOT_STRAFE_SPEED	200
#define	BOT_IDEAL_DIST_FROM_ENEMY	160

#define	WANT_KINDA		1
#define	WANT_YEH_OK		2
#define	WANT_SHITYEAH	3

#define	BOT_GUARDING_RANGE 600.0

// bot_ai.c
// these define how long the bot will search for it's enemy before giving up
#define	BOT_SEARCH_LONG		4
#define	BOT_SEARCH_MEDIUM	2
#define	BOT_SEARCH_SHORT	1

#define	SIGHT_FIRE_DELAY	0.8		// so bot's don't fire straight away after sighting an enemy

int		spawn_bots;
int		roam_calls_this_frame;
int		bestdirection_callsthisframe;

// ---- BOT CHAT DATA ----

#define	CHAT_GREETINGS			0
#define	CHAT_INSULTS_GENERAL	1
#define	CHAT_INSULTS_KICKASS	2
#define	CHAT_INSULTS_LOSING		3
#define	CHAT_COMEBACKS			4
#define	CHAT_TEAMPLAY_HELP		5
#define	CHAT_TEAMPLAY_DROPITEM	6
#define	CHAT_TEAMPLAY_GROUP		7

#define	NUM_CHAT_SECTIONS		8
#define	MAX_CHAT_PER_SECTION	64

char	*bot_chat_text[NUM_CHAT_SECTIONS][MAX_CHAT_PER_SECTION];
int		bot_chat_count[NUM_CHAT_SECTIONS];
float	last_bot_chat[NUM_CHAT_SECTIONS];

int		num_view_weapons;
char	view_weapon_models[64][64];

// -----------------------

int	RoamFindBestItem(edict_t	*self, edict_t	*list_head, int	check_paths);
void	bot_ChangeYaw(edict_t *self);
void	bot_MoveAI(edict_t *self, int dist);
float	bot_ReachedTrail(edict_t *self);
void	botMachineGun (edict_t *self);
int		bot_move(edict_t *self, float dist);
int		bot_oldmove(edict_t *self, float dist);
void	respawn_bot (edict_t *self);
void	bot_SuicideIfStuck(edict_t *self);

void bot_pain (edict_t *self, edict_t *other, float kick, int damage);
void bot_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void bot_run (edict_t *self);

edict_t	*bot_FindAnyTrail(edict_t *bot);
int	CanJump(edict_t *ent);

void bot_AnimateFrames(edict_t *self);
void bot_roam (edict_t *self, int force_enemy);
int	botCheckStuck(edict_t	*self);
int CanStand(edict_t	*self);
int		CanSee(edict_t *self, edict_t *targ);
int		CanReach(edict_t *self, edict_t *targ);

int	botdebug;
void botDebugPrint(char *msg, ...);

// bot_wpns.c

#define	FIRE_INTERVAL_BLASTER			0.6
#define	FIRE_INTERVAL_ROCKETLAUNCHER	0.8
#define	FIRE_INTERVAL_GRENADELAUNCHER	0.9
#define	FIRE_INTERVAL_RAILGUN			1.5
#define	FIRE_INTERVAL_HYPERBLASTER		0
#define	FIRE_INTERVAL_CHAINGUN			0
#define FIRE_INTERVAL_MACHINEGUN		0
#define FIRE_INTERVAL_SHOTGUN			1
#define FIRE_INTERVAL_SSHOTGUN			1
#define FIRE_INTERVAL_BFG				2.8
//ACRID NEW DEFINES FOR WF WEAPONS
#define FIRE_INTERVAL_SNIPERRIFLE       1
#define FIRE_INTERVAL_LIGHTNINGGUN      1.5
#define FIRE_INTERVAL_INFECTEDDART      0.9
#define FIRE_INTERVAL_PULSECANNON       0
#define	FIRE_INTERVAL_TELSACOIL         0//42 Acrid
#define FIRE_INTERVAL_NEEDLER           0
#define FIRE_INTERVAL_FLAMETHROWER      0.3
#define	FIRE_INTERVAL_ROCKETNAPALMLAUNCHER	0.8
#define	FIRE_INTERVAL_PELLETROCKETLAUNCHER	0.8
#define	FIRE_INTERVAL_ROCKETCLUSTERLAUNCHER	0.8
#define	FIRE_INTERVAL_STINGERROCKETLAUNCHER	0.8
#define FIRE_INTERVAL_SHC			1
#define FIRE_INTERVAL_GRENADES      1
#define FIRE_INTERVAL_POISONDART      0.9
#define FIRE_INTERVAL_AK47      		0
#define FIRE_INTERVAL_PISTOL      0.6
#define FIRE_INTERVAL_KNIFE      1
//ACRID NEW DEFINES FOR WF WEAPONS

#define	BOT_CHANGEWEAPON_DELAY	0.9

void	bot_FireWeapon(edict_t	*self);
void	bot_Attack(edict_t *self);

void botBlaster (edict_t *self);
void botMachineGun (edict_t *self);
void botShotgun (edict_t *self);
void botSuperShotgun (edict_t *self);
void botChaingun (edict_t *self);
void botRailgun (edict_t *self);
void botRocketLauncher (edict_t *self);
void botGrenadeLauncher (edict_t *self);
void botHyperblaster (edict_t *self);
void botBFG (edict_t *self);
//ACRID WF STUFF
void botSniperRifle (edict_t *self);
void botLightningGun (edict_t *self);
void botInfectedDart (edict_t *self);
void botPulseCannon (edict_t *self);
void botTelsaCoil (edict_t *self);//42
void botFlameThrower (edict_t *self);
void botNeedler (edict_t *self);
void botPelletRocketLauncher (edict_t *self);
void botRocketNapalmLauncher (edict_t *self);
void botRocketClusterLauncher (edict_t *self);
void botStingerRocketLauncher (edict_t *self);
void botSHC (edict_t *self);
void botGrenades (edict_t *self);
void botPoisonDart (edict_t *self);
void botAk47 (edict_t *self);
void botPistol (edict_t *self);
//ACRID WF STUFF

void	botPickBestWeapon(edict_t *self);
void	botPickBestGrenade(edict_t *self);//acrid
int		botHasWeaponForAmmo (gclient_t *client, gitem_t *item);
//int		ClientHasAnyWeapon(gclient_t	*client);//acrid coed
int		botCanPickupAmmo (gclient_t *client, gitem_t *item);
int		botCanPickupArmor (edict_t *self, edict_t *ent);
void	botPickBestFarWeapon(edict_t *self);
void	botPickBestCloseWeapon(edict_t *self);
int     botAmmoIndex(gitem_t *weapon);
qboolean botHasWeaponInInventory(edict_t *self, gitem_t *weapon);
qboolean botHasAmmoForWeapon(edict_t *self, gitem_t *weapon);
qboolean botHasThisWeapon(edict_t *self, gitem_t *weapon);

void	GetBotFireForWeapon(gitem_t	*weapon,	void (**bot_fire)(edict_t	*self));
int		botCanPickupPack (edict_t *self, edict_t *ent);//acrid
int		botCanPlaceSpecial (edict_t *self, edict_t *ent);//42 acrid 
// bot_spawn.c
edict_t	*spawn_bot (char *botname);
void	botDisconnect(edict_t	*self);

// bot_misc.c
void	ReadBotConfig();
bot_info_t	*GetBotData(char *botname);
void	NodeDebug(char *fmt, ...);
void	FindVisibleItemsFromNode(edict_t	*node);
void	AdjustRatingsToSkill(edict_t *self);
edict_t *DrawLine(edict_t *owner, vec3_t spos, vec3_t epos);
void	TeamGroup(edict_t *ent);
void TeamDisperse(edict_t *self);
void BotGreeting(edict_t *chat);
void BotInsultStart(edict_t *self);
void BotInsult(edict_t *self, edict_t *enemy, int chat_type);
qboolean SameTeam(edict_t *plyr1, edict_t *plyr2);
float	HomeFlagDist(edict_t *self);
qboolean CarryingFlag(edict_t *ent);

// bot_nav.c
void	botRoamFindBestDirection(edict_t	*self);
void	botRandomJump(edict_t	*self);
void	BotMoveThink (edict_t *ent, usercmd_t *ucmd);
int botJumpAvoidEnt(edict_t *self, edict_t *e_avoid);


// FIXME: this should go in g_local.h
qboolean monster_start (edict_t *self);
qboolean monster_start_go (edict_t *self);
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void Use_Quad (edict_t *ent, gitem_t *item);
void ClientDisconnect (edict_t *ent);
void Use_Plat (edict_t *ent, edict_t *other, edict_t *activator);
void ShowGun(edict_t *ent);
void FlagPathTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

extern char	respawn_bots[64][256];

// CTF stuff
extern gitem_t *flag1_item;
extern gitem_t *flag2_item;

extern edict_t *flag1_ent;
extern edict_t *flag2_ent;

extern gitem_t	*item_tech1, *item_tech2, *item_tech3, *item_tech4;

extern edict_t *flagreturn1_ent;//$
extern edict_t *flagreturn2_ent;//$