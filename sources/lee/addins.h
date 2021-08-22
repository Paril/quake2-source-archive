//in p_client.c
void make_light (edict_t *self, vec3_t start);
void toggle_torch(edict_t *ent);

//in addins.c
void zoomin(edict_t *ent);
void zoomout(edict_t *ent);
void make_dead_player(edict_t *ent);
void Help_on_patch (edict_t *ent);
void the_time (edict_t *ent);

//in bot_setup
void create_bot_point(edict_t *ent);
void load_bot_messages(edict_t *ent);
void load_bot_names(edict_t *ent);
void Make_bot_points (edict_t *ent);

//in torch
void light(edict_t *ent);
void del_light(edict_t *ent);

#ifdef MIDI
//in midi
int ShutdownMidi(void);
#endif

//connect_menu
void MPOpenJoinMenu(edict_t *ent);
qboolean MPStartClient(edict_t *ent);