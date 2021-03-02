#include "fileio.h" 
#include "qmenu.h"
#include "q_devels.h"
#include "x_fire.h"
#include "x_fbomb.h"

#define entity edict_t
#define flag1_entity G_Find(NULL,FOFS(classname),"item_flag_team1")
#define flag2_entity G_Find(NULL,FOFS(classname),"item_flag_team2")

void ClassFunction (edict_t *ent, int i);
void muzzleflash (edict_t *ent, int mz_);

// set class equipment
void SetCustomEquipment (gclient_t *client);
// utily and debug functions

void PrintTeamState (entity *ent);
void PrintClass (edict_t *ent); //print class
void Rip_SetSpeed (edict_t *self); // set speed
void Rip_SetAll(edict_t *self); // doesn't really needed
void Rip_SetSkin (edict_t *ent);
void LessHealth (edict_t *ent, float x); // ent->health = ent->health - x;

// teamplay

void JoinTeam (edict_t *ent, int s_team);
void Cmd_Team_f (edict_t *ent);
void Cmd_Class_f (edict_t *ent);

void MageJump (edict_t *ent);
void MageJump1 (edict_t *ent);
void LessAmmo (edict_t *ent, float x, char * t);
void CheckX (edict_t *self, float x, char *r);

void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);

void tprintf (edict_t *self, edict_t *ignore, int mod, int printlevel, char *fmt, ...);
void eprintf (edict_t *self, edict_t *ignore, int mod, int printlevel, char *fmt, ...); // not really used yet
void strcat_ (char *it, char *fmt, ...);
void strcpy_ (char *s, char *fmt, ...);
void localcmd (char *s, ...);
void stuffcmd(edict_t *e, char *s);
void centerprint_all (char *msg,...);

#define STATE_LEGSHOT  1
#define STATE_SKIN     2
#define STATE_SPEED    4
#define STATE_VICTIM   6
#define STATE_NOTSAY   8
#define STATE_BUILDING 16
#define STATE_CLOAKING 32
#define STATE_FEINING  64

extern char *cla_names[11];