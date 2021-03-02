
typedef enum {
	CTF_NOTEAM,
	CTF_TEAM1,
	CTF_TEAM2
} ctfteam_t;

//typedef enum {
//	CTF_STATE_START,
//	CTF_STATE_PLAYING
//} ctfstate_t;


#define DF_CTF_FORCEJOIN	131072	
#define DF_ARMOR_PROTECT	262144
#define DF_CTF_NO_TECH      524288



void CTFOpenJoinMenu(edict_t *ent);
// Sconfig 2.8
void CTFOpenJoinMenu2(edict_t *ent);
// Sconfig 2.8

qboolean CTFStartClient(edict_t *ent);

void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
