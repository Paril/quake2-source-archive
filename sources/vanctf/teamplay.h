qboolean OnSameTeam(edict_t *a, edict_t *b);
void team_scores(int *team1score, int *team2score, int *team1players, int *team2players);
void team_change(edict_t *ent, char *s);
void Cmd_Team_f(edict_t *ent);
void Svcmd_ForceTeam_f ();
void SP_info_player_team1(edict_t *self);
void SP_info_player_team2(edict_t *self);
edict_t *SelectCTFSpawnPoint (edict_t *ent);
void team_assign(edict_t *ent);
char *team_skin(edict_t *ent);

//#define	SKIN_RED	(ctfskins->value ? "male/m_ctf1" : "male/claymore")
//#define	SKIN_BLUE	(ctfskins->value ? "female/f_ctf2" : "female/lotus")

#define	SKIN_ID_RED		"male/claymore"
#define	SKIN_ID_BLUE	"female/lotus"
#define	SKIN_MILL_RED	"male/m_ctf1"
#define	SKIN_MILL_BLUE	"female/f_ctf2"
#define SKIN_IDCTF_RED	"male/ctf_r"
#define	SKIN_IDCTF_BLUE "female/ctf_b"
#define	SKIN_TEAM1		(zoidctf->value ? SKIN_IDCTF_RED  : SKIN_ID_RED )
//(ent->client->pers.ctfskins ? SKIN_MILL_RED  : SKIN_ID_RED )
#define	SKIN_TEAM2		(zoidctf->value ? SKIN_IDCTF_BLUE : SKIN_ID_BLUE)
//(ent->client->pers.ctfskins ? SKIN_MILL_BLUE : SKIN_ID_BLUE)
#define	IS_TEAM1(ent)	(ent->client->resp.ctf_team == CTF_TEAM1)
#define	IS_TEAM2(ent)	(ent->client->resp.ctf_team == CTF_TEAM2)

#define	CTF_NOTEAM		0
#define	CTF_TEAM1		1
#define	CTF_TEAM2		2

#define CTF_STATE_START 0
#define CTF_STATE_PLAYING 1

extern	char	*team_nameforteam[];
//extern	char	*team_padnamefornum[];