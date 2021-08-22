#ifndef __BOT_H__
#define __BOT_H__

#include "g_local.h"


// *********************** //
// * Forgotten functions * //
// *********************** //

void SelectSpawnPoint(edict_t *ent, vec3_t origin, vec3_t angles);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
qboolean ClientConnect(edict_t *ent, char *userinfo);
void ClientDisconnect(edict_t *ent);
void CopyToBodyQue(edict_t *ent);
void ClientThink(edict_t *ent, usercmd_t *cmd);

// ********************* //
// * Command functions * //
// ********************* //

void Svcmd_Bot_f(void);
void Bot_Create(void);
void Bot_Spawn(edict_t *ent);
void Bot_Respawn(edict_t *ent);

void Bot_Pain(edict_t *ent, edict_t *other, float kickback, int damage);

void Bot_Think(edict_t *ent);
void Bot_Aim(edict_t *ent, edict_t *target, vec3_t angles);
void Bot_Attack(edict_t *ent, usercmd_t *cmd, vec3_t angles);
void Bot_Move(edict_t *ent, edict_t *goal, usercmd_t *cmd, vec3_t angles);

void Bot_FindStuff(edict_t *ent);

//checking routiunes
qboolean bot_test_ground (edict_t *ent);
qboolean bot_check_frame (edict_t *ent);

//movement
void bot_move (edict_t *ent, edict_t *goal, vec3_t angles);

//strings
extern bot_message_string bot_msg_killer[60];
extern bot_message_string bot_msg_killed[100];
extern bot_message_string bot_names[300];

//bot load stuff (server only)
void load_bot_names(edict_t *ent);
void Make_bot_points (edict_t *ent);
void load_bot_messages(edict_t *ent);
void check_for_stay_at_base(edict_t *ent);
void ctf_wait_base(edict_t *ent, vec3_t angles);
#endif
