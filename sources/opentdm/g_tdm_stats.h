/*===============
Stats Stuff
===============*/
//extern teamplayer_t	*current_teamplayers ;
//extern int			num_current_teamplayers;

//extern teamplayer_t	*old_teamplayers ;
//extern int			num_old_teamplayers;

void TDM_Stats_f (edict_t *ent, matchinfo_t *m_info);
void TDM_WeaponsStats_f (edict_t *ent, matchinfo_t *m_info, teamplayer_t *p_info);
void TDM_ItemsStats_f (edict_t *ent, matchinfo_t *m_info, teamplayer_t *p_info);

void TDM_TeamStats_f (edict_t *ent, matchinfo_t *m_info);
void TDM_TeamWeaponsStats_f (edict_t *ent, matchinfo_t *m_info, int team);
void TDM_TeamItemsStats_f (edict_t *ent, matchinfo_t *m_info, int team);

void TDM_TopBottomShots_f (edict_t *ent, qboolean team_stats, qboolean top_shots);

void TDM_RemoveStatsLink (edict_t *ent);
void TDM_SetupMatchInfoAndTeamPlayers (void);
void TDM_WriteStatsString (edict_t *ent, matchinfo_t *m_info);
teamplayer_t *TDM_FindTeamplayerForJoinCode (unsigned code);
teamplayer_t *TDM_GetInfoForPlayer (edict_t *ent, matchinfo_t *matchinfo);
void TDM_SetupTeamInfoForPlayer (edict_t *ent, teamplayer_t *info);
void TDM_Killed (edict_t *attacker, edict_t *victim, int mod);
