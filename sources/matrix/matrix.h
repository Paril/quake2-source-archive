#include "matrix_def.h"
/*
==================
Structs
==================
*/
typedef struct
{
	float		start_tank_time;
	qboolean	started;
	qboolean	count_started;
	qboolean	lightsout;
	int			team_red_score;
	int			team_blue_score;
	float		lightsout_framenum;
	int			tankrounds;
	edict_t		*triggerent;
} matrix_locals_t;
typedef enum
{
	HAND_DEFAULT,
	HAND_OTHER,
	HAND_BOTH
} fromhand_t;
/*
==================
matrix_movement.c
==================
*/
void KungFu_l33t_Skillz (edict_t *ent, int damage, int kick, int mod);
void MatrixJump (edict_t *ent);
void MatrixFlip (edict_t *ent);
void MatrixRunUpWalls (edict_t *ent);
void MatrixRunAlongWalls (edict_t *ent);
int MatrixFallingRoll(edict_t *ent, int damage);
void MatrixSpeed (edict_t *ent);
void SuperJump (edict_t *ent);
void SpawnShadow (edict_t *ent);
void MatrixRunLAlongWalls (edict_t *ent);
void MatrixRunRAlongWalls (edict_t *ent);
void SpawnWave (edict_t *ent);
void SpeedWaveThink (edict_t *wave);
//SKULL
extern  cvar_t  *tpp;
extern  cvar_t  *crossh;
void CheckChasecam_Viewent (edict_t *ent);
void Cmd_Chasecam_Toggle (edict_t *ent);
void ChasecamRemove (edict_t *ent, char *opt);
void ChasecamStart (edict_t *ent);
void Cmd_ToggleHud ();
extern char *single_statusbar;
void MakeFakeCrosshair (edict_t *ent);
void UpdateFakeCrosshair (edict_t *ent);
void DestroyFakeCrosshair (edict_t *ent);
void fire_streak (edict_t *self, vec3_t start, vec3_t dir, int damage, int MOD);
void MatrixOlympics (edict_t *ent);
void matrix_streak_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void Cmd_ScreenTilt_f (edict_t *ent);
void MatrixComboTally (edict_t *ent);
void kuml (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
//END
/*
=================
Globals
=================
*/

edict_t *SelectDeathmatchSpawnPoint (void);
extern char *matrix_statusbar;
qboolean SpellFull(edict_t *ent);
void MatrixSniperHud(edict_t *ent);
void MatrixRespawn (edict_t *self, edict_t *attacker);
void MatrixChangeWeapon (edict_t *ent);




/*
=================
Weapon C Vars
=================
*/

//damage
extern	cvar_t	*damage_deserts;
extern	cvar_t	*damage_mk23;
extern	cvar_t	*damage_mp5;
extern	cvar_t	*damage_m4;
extern	cvar_t	*damage_pumps;
extern	cvar_t	*damage_smc;
extern	cvar_t	*damage_sniper;
extern	cvar_t	*damage_knife;
extern	cvar_t	*damage_fist;

//damage radius/radius damage (where applicable)
extern	cvar_t	*damageradius_rack; //how far the damage spreads out
extern	cvar_t	*damageradius_grenade;
extern	cvar_t	*radiusdamage_rack; //how much damage is done with distance.
extern	cvar_t	*radiusdamage_grenade;

//ammo use
extern	cvar_t	*ammo_deserts;
extern	cvar_t	*ammo_mk23;
extern	cvar_t	*ammo_mp5;
extern	cvar_t	*ammo_m4;
extern	cvar_t	*ammo_pumps;
extern	cvar_t	*ammo_smc;
extern	cvar_t	*ammo_sniper;
extern	cvar_t	*ammo_knife;
extern	cvar_t	*ammo_grenade;
extern	cvar_t	*ammo_rack;

//integrated kungfu damage
extern	cvar_t	*damage_jab;
extern	cvar_t	*damage_hook;
extern	cvar_t	*damage_uppercut;
extern	cvar_t	*damage_hoverkick;
extern	cvar_t	*damage_spinkick;

// integrated kungfu "reload" times.
extern	cvar_t	*reload_jab;
extern	cvar_t	*reload_hook;
extern	cvar_t	*reload_uppercut;
extern	cvar_t	*reload_hoverkick;
extern	cvar_t	*reload_spinkick;

//game options
//extern	cvar_t	*leper;
extern	cvar_t	*possesban;
extern	cvar_t	*sv_maxlevel; // maximum amount of upgrades allowed.
extern	cvar_t	*zk_logonly; // something to do with zbot.
extern	cvar_t	*teamplay; // duh
extern	cvar_t	*tankmode; // tank mode on off (don't think tank mode is in there anymore)
extern	cvar_t	*weaponrespawntime; // in seconds
extern	cvar_t	*weaponban; //stops weapons spawning, fag.
extern	cvar_t	*laseroff; //sniper lasersight on/off (lag)
extern	cvar_t	*streakoff; //bullet streaks on/off (lag)
extern	cvar_t	*shellsoff; //casing ejection on/off (lag)
extern	cvar_t	*redteamskin; // i don't understannd????
extern	cvar_t	*redteamname; //wtf is this anyway?
extern	cvar_t	*blueteamskin; //huh?
extern	cvar_t	*blueteamname; //AHHGG!!
extern	cvar_t	*matchtimelimit;  //help i have no friends!
extern	cvar_t	*teamfraglimit; /// fag.
extern	cvar_t	*matchmode; // i literally have "no idea"
extern	cvar_t	*choosestuff; // was going to be like action's select weapons before play, but that sucks.
extern	cvar_t	*hop; // how high you hop if you only have 1 leg.
extern	cvar_t	*action; // 1/0. should you hop around if you lose a leg? (realistic damage - but it sucks)
extern	cvar_t	*faglimit; //max number of neos/morpheuses/trinities allowed on a server


//message options. hopefully someone will bring out a matrix wavpack, 
//so morph says "he's beginning to believe" when you hit kill streak 3. YEAH.
extern	cvar_t  *combomessage;
extern	cvar_t  *killstreakmessage;
extern	cvar_t  *streakmessage2;
extern	cvar_t  *streakmessage3;
extern	cvar_t  *streakmessage4;
extern	cvar_t  *streakmessage5;
extern	cvar_t  *streakmessage6;


extern	matrix_locals_t	matrix;