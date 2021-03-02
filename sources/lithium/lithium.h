/*============================================================================

    This file is part of Lithium II Mod for Quake II
    Copyright (C) 1997, 1998, 1999, 2010 Matthew A. Ayres

    Lithium II Mod is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Lithium II Mod is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lithium II Mod.  If not, see <http://www.gnu.org/licenses/>.

    Quake II is a trademark of Id Software, Inc.

    Code by Matt "WhiteFang" Ayres, matt@lithium.com

============================================================================*/

extern char lithium_version[];
extern char lithium_modname[];

struct lclient_s {
	edict_t *ent;

	int ip[4];
	int port;

	int id;
	int chan_id;

	int cam_freelook;
	int cam_distance;
	int cam_height;
	int cam_jump;
	int cam_maxpitch;

	int vote;
	int vote_tries;
	char vote_map[32];

	char chan[32];

	int ping;
	float ping_last;
	qboolean rate_check;

	int board_show;

	int muzzle_says;
	float muzzle_say_time;
	qboolean muzzled;
	float muzzle_time;
	int muzzles;
	int kick_says;
	float kick_say_time;
};

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define NUM_RUNES 5

#define LITHIUM_OBSERVER 1
#define LITHIUM_PLAYING 2
#define LITHIUM_GOCHASECAM 4
#define LITHIUM_CHASENOTICE 8
#define LITHIUM_CHASELEADER 16
#define LITHIUM_VERIFIED 32
#define LITHIUM_ATTACKWAIT 512
#define LITHIUM_STATUSBAR 2048
#define LITHIUM_MAINMENU 4096
#define LITHIUM_HIDDEN 16384

#define CS_LITHIUM (CS_ITEMS - 20)
#define CS_RUNE1 (CS_LITHIUM + 1)
#define CS_RUNE2 (CS_LITHIUM + 2)
#define CS_RUNE3 (CS_LITHIUM + 3)
#define CS_RUNE4 (CS_LITHIUM + 4)
#define CS_RUNE5 (CS_LITHIUM + 5)
#define CS_OBSERVING (CS_LITHIUM + 9)
#define CS_CHASING (CS_LITHIUM + 10)
#define CS_SAFETY (CS_LITHIUM + 11)
#define CS_HUDRUNE0 (CS_LITHIUM + 12)
#define CS_HUDRUNE1 (CS_LITHIUM + 13)
#define CS_HUDRUNE2 (CS_LITHIUM + 14)
#define CS_HUDRUNE3 (CS_LITHIUM + 15)
#define CS_HUDRUNE4 (CS_LITHIUM + 16)
#define CS_HUDRUNE5 (CS_LITHIUM + 17)

#define HUD_NONE 0
#define HUD_NORMAL 1
#define HUD_LITHIUM 2
#define HUD_AMMO 3
#define HUD_MAX 3

#define LAYOUT_SCORES 1
#define LAYOUT_CENTERPRINT 2
#define LAYOUT_MENU 4
#define LAYOUT_MOTD 16
#define LAYOUT_CHASECAM 32
#define LAYOUT_ID 64
#define LAYOUT_NEWS 128
#define LAYOUT_HSCORES 256

#define MENU_TEXT 1
#define MENU_BLANK 2
#define MENU_LVAR 3
#define MENU_FUNC 4
#define MENU_CMD 5
#define MENU_SVCMD 6
#define MENU_PVAR 7

#define INFOFLAG_IDSTATE 1
#define INFOFLAG_BESTWEAP 2
#define INFOFLAG_CHASING 4
#define INFOFLAG_CHASELEADER 8

#define SCORES_NORMAL 0
#define SCORES_BYFRAGS 1
#define SCORES_BYFPH 2

#define MOD_HOOK 35

#define VAR_NONE 0
#define VAR_USE 1
#define VAR_START 2
#define VAR_STARTWEAP 3
#define VAR_MAX 4
#define VAR_WEAPON 5
#define VAR_INHIBIT 6
#define VAR_OTHER 7
#define VAR_PACK 8
#define VAR_RUNE 9
#define VAR_HOOK 10
#define VAR_GENERAL 11
#define VAR_DEF 12
#define VAR_VOTE 13
#define VAR_CTF 14

typedef struct menuline_s menuline_t;
typedef struct menu_s menu_t;
typedef struct lvar_s lvar_t;

struct menuline_s { 
	menuline_t *prev, *next;
	int type;
	int line;
	char *text;
	void *data;
	qboolean selectable;
	qboolean textp;
};

struct menu_s {
	menuline_t *firstline, *lastline;
	menuline_t *sel;
	int startline, endline;
	int page;
	qboolean changed;
	qboolean editing;
	void (*cancel_func)(edict_t *ent);
	char edit[12];
	menuline_t *lastedit;
	float lastedit_time;
	int xoff;
	int yoff;
};

struct lvar_s {
	cvar_t *cvar;
	char *string;
	float value;
	int admin;
	char *edit;
	int cat;
	void (*func)(void);
	lvar_t *next;
};

struct pvar_s {
	char *name;
	int *value;
	int old;
	char *edit;
	pvar_t *next;
};

int countclients(void);
int countplayers(void);
int countteam1(void);
int countteam2(void);

void Lithium_InitGame(void);
void Lithium_InitLevel(void);
void Lithium_RunFrame(void);
void Lithium_ClientBegin(edict_t *ent);
void Lithium_ClientDisconnect(edict_t *ent);
void Lithium_PutClientInServer(edict_t *ent);
qboolean Lithium_ClientThink(edict_t *ent, usercmd_t *ucmd);
void Lithium_ClientBeginFrame(edict_t *ent);
void Lithium_ClientEndFrame(edict_t *ent);
void Lithium_ExitLevel(void);
void Lithium_DoUpgrade(void);
void Lithium_PlayerDie(edict_t *attacker, edict_t *self);
qboolean Lithium_Observer(edict_t *ent, qboolean check);
void Lithium_Playing(edict_t *ent);
void Lithium_SetStats(edict_t *ent);
int Lithium_Scoreboard(edict_t *ent, edict_t *killer);
void Lithium_SelectPrevItem(edict_t *ent);
void Lithium_SelectNextItem(edict_t *ent);
void Lithium_ClientUserinfoChanged(edict_t *ent, char *userinfo);
qboolean Lithium_ClientCommand(edict_t *ent);
void Lithium_SetClientEffects(edict_t *ent);
void Lithium_BeginIntermission(void);
qboolean Lithium_Use(edict_t *ent);
void Lithium_Menu(edict_t *ent);
void Lithium_CTFMenu(edict_t *ent);
void Lithium_UpdateCTFMenu(edict_t *ent);
void Lithium_CTFCredits(edict_t *ent);
void Lithium_LayoutOn(edict_t *ent, int flag);
void Lithium_LayoutOff(edict_t *ent, int flag);
void Lithium_LayoutTog(edict_t *ent, int flag);
qboolean Lithium_ClientConnect (edict_t *ent, char *userinfo);
qboolean Lithium_IsObserver(edict_t *ent);
qboolean Lithium_IsHidden(edict_t *ent);
void Lithium_MaxRate(edict_t *ent);
void Lithium_Shutdown(void);
void Lithium_NoAmmoWeaponChange(edict_t *ent);

// l_admin.c
void Admin_InitGame(void);
void Admin_ClientThink(edict_t *ent);
void Admin(edict_t *ent);
void Admin_Code(edict_t *ent);
void Admin_Var(edict_t *ent);
void Admin_DMFlags(edict_t *ent);
void Admin_Kick(edict_t *ent);
void Admin_Ban(edict_t *ent);
void Admin_Map(edict_t *ent);
void Admin_MapI(edict_t *ent);
void Admin_MapW(edict_t *ent);
qboolean Admin_ClientCommand(edict_t *ent);

// l_mod.c
void Mod_InitGame(void);

// l_misc.c
void wf_strlwr(char *str);
char *file_gamedir(char *name);
qboolean file_exist(char *name);
void stuffcmd(edict_t *e, char *s);
void centerprintf(edict_t *ent, char *format, ...);
void centerprintf2(edict_t *ent, char *format, ...);
int strip(char *str);

// l_rune.c
void Rune_InitGame(void);
void Rune_InitLevel(void);
void Rune_RunFrame(void);
char *Rune_GetName(int type);
void Rune_Reset(void);
void Rune_MaybeSpawn(vec3_t origin);
edict_t *Rune_Spawn(vec3_t origin, int type);
void Rune_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void Rune_Remove(edict_t *self);
void Rune_Drop(edict_t *player);
int Rune_AdjustDamage(edict_t *targ, edict_t *attacker, int damage);
void Rune_TDamage(edict_t *targ, edict_t *attacker, int damage);
void Rune_ClientFrame(edict_t *player);
int Rune_HasHaste(edict_t *player);
int Rune_HasRegen(edict_t *player);
void Rune_RemoveAll(void);
void Rune_FireSound(edict_t *ent);
qboolean Rune_Give(edict_t *ent, char *name);

// l_pack.c
void Pack_InitGame(void);
void Pack_MaybeSpawn(vec3_t origin);
void Pack_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void Pack_Remove(edict_t *self);

// l_fragtrak.c
void FragTrak_InitLevel(void);
void FragTrak_Frag(edict_t *killer, edict_t *killee);
void FragTrak_Save(void);
void FragTrak_ClientBegin(edict_t *ent);
void FragTrak_ClientDisconnect(edict_t *ent);

// l_gibstats.c
void GSLog_InitGame(void);
void GSLog_InitLevel(void);
void GSLog_ExitLevel(void);
qboolean Lithium_EndDMLevel(void);
void GSLog_Shutdown(void);
void GSLog_ClientBegin(edict_t *ent);
void GSLog_ClientDisconnect(edict_t *ent);
void GSLog_PlayerRename(char *oldname, char *newname);
void GSLog_Score(char *attacker, char *target, char *type, char *weapon, int score, int time, int ping);
void GSLog_Bonus(edict_t *ent, char *type, int score);

// l_mapqueue.c
void Mapqueue_InitGame(void);
void Mapqueue_InitLevel(void);
edict_t *Mapqueue_EndDMLevel(void);
char *Mapqueue_GetMapName(void);
void Mapqueue_Override(char *mapname);
void Mapqueue_Menu(edict_t *ent, char *cmd);
qboolean Mapqueue_Valid(char *filename, char *mapname);

// l_nocamp.c
void NoCamp_InitGame(void);
void NoCamp_PutClientInServer(edict_t *ent);
void NoCamp_ClientThink(edict_t *ent, usercmd_t *ucmd);

// l_hook.c
void Hook_InitGame(void);
void Hook_PlayerDie(edict_t *attacker, edict_t *self);
void Hook_Think(edict_t *self);
edict_t *Hook_Start(edict_t *ent);
void Hook_Reset(edict_t *rhook);
qboolean Hook_Check(edict_t *self);
void Hook_Service(edict_t *self);
void Hook_Track(edict_t *self);
void Hook_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void Hook_Fire(edict_t *owner, vec3_t start, vec3_t forward);
void Weapon_Hook_Fire(edict_t *ent);
void Weapon_Hook(edict_t *ent);

// l_menu.c
void Menu_Create(edict_t *ent, int startline, int endline);
void Menu_Offsets(edict_t *ent, int xoff, int yoff);
void Menu_Title(edict_t *ent, char *title);
void Menu_AddLine(edict_t *ent, int type, int line, char *text, void *data);
void Menu_AddText(edict_t *ent, int line, char *format, ...);
void Menu_CancelFunc(edict_t *ent, void (*func)(edict_t *ent));
void Menu_Destroy(edict_t *ent);
int Menu_Update(edict_t *ent);
void Menu_Prev(edict_t *ent);
void Menu_Next(edict_t *ent);
void Menu_PrevPage(edict_t *ent);
void Menu_NextPage(edict_t *ent);
void Menu_Use(edict_t *ent);
void Menu_Key(edict_t *ent, int key);
void Menu_EditBegin(edict_t *ent);
void Menu_EditEnd(edict_t *ent);
void Menu_LVarChanged(lvar_t *lvar);
void Menu_ClientFrame(edict_t *ent);
void Menu_Cancel(edict_t *ent);
qboolean Menu_ClientCommand(edict_t *ent);

qboolean Var_SpawnEntity(edict_t *ent);

// l_team.c
void Team_ClientThink(edict_t *ent);
void Team_Set(edict_t *ent, int team_num);
void Team_Cmd(edict_t *ent, char *str);

// other
void Observer_Cmd(edict_t *ent, char *cmd);
void Armor_Realize(gitem_armor_t *armor_info, char *string);
void StatusBar_UpdateAll(void);
int StatusBar_Update(edict_t *ent);
int Layout_Update(edict_t *ent);
int FasterRespawn(edict_t *ent, float delay);
void Weapon_PickBest(edict_t *ent, edict_t *other);
void Scores_Cmd(edict_t *ent, char *cmd);
qboolean ChaseFind(edict_t *ent, int place, int searchdir);
void ChaseStart(edict_t *ent);
void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
void ChaseSet(edict_t *self, edict_t *ent);

// l_net.c
void LNet_InitGame(void);
void LNet_InitLevel(void);
void LNet_ExitLevel(void);
void LNet_Shutdown(void);
void LNet_RunFrame(void);
void LNet_ClientBegin(edict_t *ent);
void LNet_ClientDisconnect(edict_t *ent);
qboolean LNet_ClientCommand(edict_t *ent);
void LNet_ClientRename(edict_t *ent, char *name);
void LNet_Observe(edict_t *ent);
void LNet_Playing(edict_t *ent);
void LNet_Score(edict_t *attack, edict_t *target, int mod, int score, int time);
void LNet_Rune(edict_t *ent, int rune);
void LNet_ReturnStat(edict_t *ent);
void LNet_Send(char *buf);
void LNet_SV(char *buf);
void LNet_BotNotice(edict_t *ent);

// l_var.c
void Var_InitGame(void);
void Var_InitLevel(void);
void Var_RunFrame(void);
void Var_PutClientInServer(edict_t *ent);
void Var_InitClientPersistant(gclient_t *client);
void Var_ClientThink(edict_t *ent);
void Var_Shutdown(void);

// l_vote.c
void Vote_InitGame(void);
qboolean Vote_ClientCommand(edict_t *ent);
void Vote_RunFrame(void);

lvar_t *lvar(char *name, char *string, char *edit, int cat);
void pvar(edict_t *ent, char *name, int *ptr, char *edit);
pvar_t *pvar_find(edict_t *ent, char *name);

void IP_Scan(char *ipstr, int ip[4]);
qboolean IP_Match(int ip1[4], int ip2[4]);

extern lvar_t *shard_armor;
extern lvar_t *max_armor;
extern lvar_t *small_health;
extern lvar_t *use_suicidekick;
extern lvar_t *use_chatmuzzle;
extern lvar_t *use_chatkick;
extern lvar_t *use_vwep;
extern lvar_t *use_packs;
extern lvar_t *use_highscores;


// weapon vars
extern lvar_t *blaster_damage;
extern lvar_t *blaster_speed;
extern lvar_t *shotgun_damage;
extern lvar_t *shotgun_count;
extern lvar_t *shotgun_hspread;
extern lvar_t *shotgun_vspread;
extern lvar_t *sshotgun_damage;
extern lvar_t *sshotgun_count;
extern lvar_t *sshotgun_hspread;
extern lvar_t *sshotgun_vspread;
extern lvar_t *machinegun_damage;
extern lvar_t *machinegun_hspread;
extern lvar_t *machinegun_vspread;
extern lvar_t *chaingun_damage;
extern lvar_t *chaingun_hspread;
extern lvar_t *chaingun_vspread;
extern lvar_t *grenade_damage;
extern lvar_t *grenade_radius;
extern lvar_t *grenadelauncher_damage;
extern lvar_t *grenadelauncher_radius;
extern lvar_t *rocket_damage;
extern lvar_t *rocket_damage2;
extern lvar_t *rocket_rdamage;
extern lvar_t *rocket_radius;
extern lvar_t *rocket_speed;
extern lvar_t *rocket_knockback;
extern lvar_t *hyperblaster_damage;
extern lvar_t *hyperblaster_speed;
extern lvar_t *railgun_damage;
extern lvar_t *bfg_damage;
extern lvar_t *bfg_radius;
extern lvar_t *bfg_speed;
extern lvar_t *bfg_raydamage;
extern lvar_t *bfg_balldamage;
extern lvar_t *bfg_ballradius;

extern lvar_t *dropweapammo;

extern lvar_t *start_weapon;
extern lvar_t *start_grenades;

extern lvar_t *power_armor_screen;
extern lvar_t *power_armor_shield;

extern lvar_t *fall_damagemod;
extern lvar_t *knockback_adjust;
extern lvar_t *knockback_self;

extern lvar_t *motd;
extern lvar_t *news;
extern lvar_t *news_time;
extern lvar_t *banlist;

extern lvar_t *fast_respawn;

extern lvar_t *fast_wep;

extern lvar_t *chatmuzzle_says;
extern lvar_t *chatmuzzle_time;
extern lvar_t *chatmuzzle_holdtime;
extern lvar_t *chatkick_says;
extern lvar_t *chatkick_time;
extern lvar_t *chatkick_muzzles;

extern lvar_t *start_health;

extern cvar_t *bounce;

void Lithium_CalcPlaces(void);
qboolean Lithium_CmdSay(edict_t *ent);
qboolean IsFemale (edict_t *ent);

extern int sorted_ents;
extern edict_t *sorted_ent[MAX_CLIENTS];
extern edict_t *fph_sorted_ent[MAX_CLIENTS];

extern float empty_time, end_time;

extern int clients;

extern lvar_t *ctf_grapple;
extern lvar_t *ctf_coloredhook;
extern lvar_t *ctf_coloredquad;

extern lvar_t *first_lvar;

extern lvar_t *debug;
extern FILE *debug_file;
void Debug(char *fmt, ...);

char *Lithium_GetAd(int down);
void Lithium_SetAd(int num, char *ad);

extern float qver;

// high scores stuff
typedef struct place_t place_t;
struct place_t {
	place_t *prev;
	place_t *next;
	int score;
	char name[16];
	char date[12];
};

void Highscores_Read(void);
void Highscores_Update(void);
void Highscores_List(edict_t *ent);
void Highscores_Scoreboard(char *string, unsigned int strlen, int *down);
int Highscores_FullScoreboard(edict_t *ent);

//#include "camclient.h"
