qboolean mbmStartClient(edict_t *ent);
void mbmOpenJoinMenu(edict_t *ent);
void mbmChaseCam(edict_t *ent, pmenu_t *p);

typedef enum {
	mbm_NOTEAM,
	mbm_TEAM1,
	mbm_TEAM2
} mbmteam_t;
	

void mbm_complete(edict_t *ent);