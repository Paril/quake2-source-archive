qboolean zbot_cmd(edict_t *ent, char *cmd);
void zbot_client_think(edict_t *ent, usercmd_t *ucmd);
void zbot_client_begin(edict_t *ent);
void zbot_init_game();
static void zbot_punish(edict_t *ent);
void zbot_client_begin_server_frame(edict_t *self);
