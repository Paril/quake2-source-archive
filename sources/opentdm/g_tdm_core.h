/*===============
Core Stuff
===============*/
//web config cache
typedef struct tdm_config_s
{
	struct tdm_config_s	*next;
	char				name[32];
	char				description[128];
	vote_t				settings;
	unsigned int		last_downloaded;
} tdm_config_t;

void JoinedTeam (edict_t *ent, qboolean reconnected, qboolean notify);
void droptofloor (edict_t *ent);
void JoinTeam1 (edict_t *ent);
void JoinTeam2 (edict_t *ent);
void ToggleChaseCam (edict_t *ent);
void SelectNextHelpPage (edict_t *ent);
void TDM_UpdateTeamNames (void);
void TDM_SetSkins ();
void TDM_SetTeamSkins (edict_t *cl, edict_t *target_to_set_skins_for);
void TDM_SetAllTeamSkins (edict_t *target_to_set_skins_for);
void TDM_SaveDefaultCvars (void);
qboolean TDM_Is1V1 (void);
void TDM_ResumeGame (void);
qboolean TDM_ProcessJoinCode (edict_t *ent, unsigned value);
const char *TDM_SecsToString (int seconds);
void UpdateMatchStatus (void);
void TDM_HandleDownload (tdm_download_t *download, char *buff, int len, int code);
qboolean TDM_ProcessText (char *buff, int len, qboolean (*func)(char *, int, void *), void *param);
void TDM_ResetVotableVariables (void);
int TDM_GetTeamFromArg (edict_t *ent, const char *value);
void TDM_FixDeltaAngles (void);
void TDM_UpdateSpectator (edict_t *ent);

void TDM_BeginCountdown (void);
void TDM_BeginMatch (void);
void TDM_EndMatch (void);

qboolean TDM_Checkmap (edict_t *ent, const char *mapname);
void TDM_WriteMaplist (edict_t *ent);
void TDM_CreateMaplist (void);

extern char		**tdm_maplist;

extern tdm_download_t	tdm_vote_download;
