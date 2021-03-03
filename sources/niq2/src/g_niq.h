/////////////////////////////////////////////////////////////////////////////
// g_niq.h:

#ifdef NIQ

#ifndef __G_NIQ_H__
#define __G_NIQ_H__

// niq: CTF STAT_ defines end at 27, so these should be fine here
#define STAT_CTF_ID_VIEW					27		// moved here from g_ctf.h so can be used in g_niq.c
#define	STAT_PPH							29		// PPH
#define	STAT_RANK							30		// rank
#define	STAT_DUMMY							31		// used to toggle score/pph/rank

// useful defines
#define NIQ_ONESECOND						1

#define NIQ_MAXSCOREBOARD					8
#define NIQ_MAXHELPSCREEN					9

#ifdef OHOOK
#define HOOK_READY							0
#define HOOK_ON								1
#define HOOK_OUT							2
#endif

extern char	szNIQCTFMENUStr1[];
extern char	szNIQCTFMENUStr2[];

// niq: exported cvars for NIQ
extern cvar_t* niq_enable;
extern cvar_t* niq_ebots;
extern cvar_t* niq_allmaps;
extern cvar_t* niq_sbhp;
extern cvar_t* niq_hlthmax;
extern cvar_t* niq_inttime;
extern cvar_t* niq_playerid;
extern cvar_t* niq_blk1;
extern cvar_t* niq_blk2;
extern cvar_t *niq_tractor;
extern cvar_t *niq_hooksky;

// BUNGLE Standard Logging Support
extern cvar_t* niq_logfile;
// BUNGLE

extern char *niq_single_statusbar;
extern char *niq_dm_statusbar;
extern char *niq_ctf_statusbar;

void        niq_removeallweapons(edict_t* ent);
void        niq_checkiftimetochangeweapon();
void        niq_monsterdeath(edict_t *monster, edict_t *attacker);
void        niq_drop_item(edict_t *self, gitem_t *item);

int         niq_strncmp(char* str1, char* str2);

qboolean	niq_has_enough_ammo(edict_t* ent);
int			niq_getnumclients();
void        niq_setstats(edict_t *ent);
void        niq_setweapontimer();
void        niq_deathmatchscoreboardmessage(edict_t *ent, edict_t *killer, qboolean reliable);
void        niq_initall();
void		niq_initdefaults(edict_t* ent);

void        niq_settimers(gclient_t* client);
void        niq_checktimers(edict_t* ent);

void        niq_findtriggers(char* entities);
qboolean    niq_istriggered(edict_t* self);
qboolean    niq_free_triggered_entity(edict_t* ent);

void		niq_adjustdamage(edict_t *targ, edict_t *attacker, int* damage);

void        niq_showmotd(edict_t *ent, qboolean bFirst);
void        niq_help(edict_t* ent);
void        niq_info(edict_t* ent);
void		niq_putaway(edict_t* ent);
qboolean    niq_clientcommand(edict_t* ent, char* cmd);
void		niq_Svcmd_Status_f (void);

qboolean	niq_zapitem(edict_t *ent, gitem_t *item);
void		niq_updatescreen(edict_t* ent);

void		niq_suicide(edict_t* self);
void		niq_kill(edict_t* self, edict_t* attacker, qboolean ff, char* message1, char* message2);
void		niq_die(edict_t* self);

void		niq_botPickBestWeapon(edict_t *self);
void		niq_showscoreboards(edict_t *ent);
void		niq_InitClientPersistant (gclient_t *client);
void		niq_handleclientinit(gclient_t* client);
void		niq_CTFScoreboardMessage (edict_t *ent, edict_t *killer);
void		niq_botsetstuff(edict_t *self);
int			niq_RoamFindBestItem(edict_t *self, edict_t	*list_head, int	check_paths);
qboolean	NIQStartClient(edict_t *ent);

//$$$
// copied here from bot_procs.h to be able to strip bot code more easily
extern int	 num_view_weapons;
extern char	 view_weapon_models[64][64];

// CTF stuff
extern gitem_t *flag1_item;
extern gitem_t *flag2_item;

extern edict_t *flag1_ent;
extern edict_t *flag2_ent;

extern gitem_t	*item_tech1, *item_tech2, *item_tech3, *item_tech4;
//$$$

#endif//__G_NIQ_H__

#endif//NIQ