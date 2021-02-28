#define RED_TEAM					1
#define BLUE_TEAM					2
#define BASE_FLAG_TAKEN				1
#define BASE_FLAG_SECURE			2

#define GROUP_ATTACKERS				1
#define GROUP_DEFENDERS				2
#define GROUP_SHARE_MULT			0.4		// multiplier to points awarded to team members
#define CTF_ASSIST_DURATION			10.0		// time (in seconds) before an assist bonus expires
#define CTF_SUMMONABLE_AUTOREMOVE	10.0	// time (in seconds) before summonable removes after flag taken

#define CTF_FLAG_CAPTURE_EXP		2000		// points for capturing enemy flag
#define CTF_FLAG_ASSIST_EXP			1000		// points for assisting a capture
#define CTF_FLAG_CAPTURE_CREDITS	350	
#define CTF_MINIMUM_PLAYERS			4
#define CTF_FLAG_DEFEND_RANGE		512		// max range of flag defender to flag carrier
#define CTF_BASE_DEFEND_RANGE		1024	// max range of defender to base
#define CTF_FLAG_DEFEND_EXP			120		// points for defending flag carrier
#define CTF_FLAG_DEFEND_CREDITS		50
#define CTF_FLAG_KILL_EXP			450		// points for killing enemy flag carrier
#define CTF_FLAG_KILL_CREDITS		200
#define CTF_BASE_DEFEND_EXP			125		// points for defending base
#define CTF_BASE_DEFEND_CREDITS		125
#define CTF_BASE_KILL_EXP			400		// points for killing enemy base defender
#define CTF_BASE_KILL_CREDITS		20
#define CTF_FLAG_RETURN_EXP			650		// points for returning the flag
#define CTF_FLAG_RETURN_CREDITS		100
#define CTF_FLAG_TAKE_EXP			2500		// points for picking up enemy flag
#define CTF_FLAG_TAKE_CREDITS		2500
#define CTF_FRAG_EXP				175		// points for normal frag (outside of base)
#define CTF_FRAG_CREDITS			100

#define CTF_PLAYERSPAWN_HEALTH					1750
#define CTF_PLAYERSPAWN_CAPTURE_EXPERIENCE	    750
#define CTF_PLAYERSPAWN_CAPTURE_CREDITS			350
#define CTF_PLAYERSPAWN_DEFENSE_RANGE			512
#define CTF_PLAYERSPAWN_DEFENSE_EXP				30
#define CTF_PLAYERSPAWN_DEFENSE_CREDITS			10
#define CTF_PLAYERSPAWN_OFFENSE_EXP				30
#define CTF_PLAYERSPAWN_OFFENSE_CREDITS			10
#define CTF_PLAYERSPAWN_TIME					1.0
#define CTF_PLAYERSPAWN_MAX_TIME				3.0					

qboolean CTF_PickupFlag (edict_t *ent, edict_t *other);
void CTF_DropFlag (edict_t *ent, gitem_t *item);
void CTF_SpawnFlag (int teamnum, vec3_t point);
char *CTF_GetTeamString (int teamnum);
void CTF_SpawnFlagBase (int teamnum, vec3_t point);
void CTF_Init (void);
void CTF_ShutDown (void);
void CTF_WriteFlagPosition (edict_t *ent);
void CTF_OpenJoinMenu (edict_t *ent);
float PlayersRangeFromSpot (edict_t *spot);
edict_t *CTF_SelectSpawnPoint (edict_t *ent);
void CTF_SpawnPlayersInBase (int teamnum);
void CTF_AwardTeam (edict_t *ent, int teamnum, int points, int credits);
void CTF_AwardPlayer (edict_t *ent, int points, int credits);
void CTF_AwardFrag (edict_t *attacker, edict_t *target);
void CTF_AwardFlagCapture (edict_t *carrier, int teamnum);
edict_t *CTF_GetFlagBaseEnt (int teamnum);
float CTF_DistanceFromBase(edict_t *ent, vec3_t start, int base_teamnum);
void CTF_CheckFlag (edict_t *ent);
int CTF_GetGroupNum (edict_t *ent, edict_t *base);
int CTF_GetBaseStatus (int teamnum);
int CTF_GetSpecialFcHealth (edict_t *ent, qboolean max);
int CTF_GetTeamCaps (int teamnum);
int CTF_GetEnemyTeam (int teamnum);
void CTF_SpawnFlagAtBase (edict_t *base, int teamnum);
int CTF_GetNumSummonable (char *classname, int teamnum);
int CTF_GetNumClassPlayers (int classnum, int teamnum);
void CTF_RecoverFlag (edict_t *ent);
void CTF_RemovePlayerFlags (void);
int CTF_NumPlayerSpawns (int type, int teamnum);
void CTF_PlayerRespawnTime (edict_t *ent);

void OrganizeTeams (qboolean remove_summonables);
edict_t *SelectDeathmatchSpawnPoint (edict_t *ent);