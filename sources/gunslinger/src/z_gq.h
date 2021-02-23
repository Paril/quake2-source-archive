#define SPECIAL_RESPAWN_TIME	60
#define SPECIAL_RESPAWN_RANGE	30
#define RESPAWN_INVULN_TIME 5

void GQ_CountTeamPlayers(void);

int GQ_FindSmallestTeam(edict_t *ent);

void GQ_AttachTriangle(edict_t *ent);

void CTFAssignSkin(edict_t *ent, char *s);

void GQ_SetTeamIdentifier(edict_t *ent);

void GQ_ChooseTeam(edict_t *ent, int choice);

void GQ_SetupClient (edict_t *ent, vec3_t spawn_origin, vec3_t spawn_angles);

edict_t *GQ_FindEnemy (edict_t *from, edict_t *self, vec3_t org, float rad);

void GQ_RespawnSpecial (edict_t *self);

void GQ_FinishSpecialSpawn (edict_t *ent);

edict_t *GQ_FindRandomSpawn(void);

void GQ_DropUniques (edict_t *self);

void GQ_DelayedRespawn (edict_t *ent);

void GQ_InitGame(void);

void GQ_SpawnSpecials(void);

void GQ_ResetPrepTime(void);

int GQ_TeamsLeft(void);

void GQ_StartLastManStanding(void);

void GQ_GameEffects (edict_t *ent);

qboolean GQ_PickupArtifact (edict_t *ent, edict_t *other);

void GQ_DropBadge (edict_t *ent, gitem_t *item);

void GQ_DropArtifact (edict_t *ent, gitem_t *item);

qboolean GQ_PickupMoneyBag (edict_t *ent, edict_t *other);

qboolean GQ_PickupVaultBag (edict_t *ent, edict_t *other);

void GQ_MakeTouchable (edict_t *ent);

void GQ_DropMoneyBag (edict_t *ent, gitem_t *item);

void GQ_PoisonThink (edict_t *ent);

void GQ_CreateSatellite(edict_t *ent);

void GQ_ClearSatellite(edict_t *ent);

void GQ_AddSatelliteEffect(edict_t *ent, int effect);

void GQ_DestroySatellite(edict_t *ent);

void GQ_UpdateSatellite(edict_t *ent);

void GQ_MOTD(edict_t *ent);

void GQ_TeamMenu(edict_t *ent);

void GQ_FFAMenu(edict_t *ent);

void GQ_ChooseClass(edict_t *ent, int choice);

void GQ_ClassMenu(edict_t *ent);

void GQ_ChooseWeapon(edict_t *ent, int choice);

void GQ_WeaponsMenu(edict_t *ent);

void GQ_AmmoMenu(edict_t *ent);

void GQ_FormatString(char *message, int size, char *left, char *right, int y);

void GQ_MatchStats(edict_t *ent, qboolean unicast);

void GQ_SetupTeamBase(edict_t *ent, int team);

void SP_item_flag_team1(edict_t *ent);
void SP_item_flag_team2(edict_t *ent);
void SP_item_flag_team3(edict_t *ent);
void SP_item_flag_team4(edict_t *ent);

void GQ_SetupTeamSpawnPoint(edict_t *ent, int team);

void SP_info_player_team1(edict_t *ent);
void SP_info_player_team2(edict_t *ent);
void SP_info_player_team3(edict_t *ent);
void SP_info_player_team4(edict_t *ent);

void GQ_UnzoomPlayer(edict_t *ent);

edict_t *GQ_SelectTeamSpawnPoint (edict_t *player);

void GQ_PlayerIdMessage (edict_t *ent, qboolean unicast);

void GQ_FreeHole(edict_t *ent);

char *GQ_TextBar(int length);

void GQ_SettingsChoice(edict_t *ent, int choice);

void GQ_GetSettingValue(char *string, char *field, int flag);
void GQ_GetSettingFlag(char *string, char *field, int flag);

void GQ_SettingsMenu1(edict_t *ent);
void GQ_SettingsMenu2(edict_t *ent);

qboolean GQ_MatchItem(gitem_t *item, char *name);

void MakeBreakable (edict_t *ent);

void SP_light_lamp1 (edict_t *ent);
void SP_light_lamp2 (edict_t *ent);
void SP_light_lamp3 (edict_t *ent);
void SP_light_candle1 (edict_t *ent);
void SP_light_candle2 (edict_t *ent);
void SP_light_torch1 (edict_t *ent);
void SP_light_hearth1 (edict_t *ent);
void SP_light_campfire1 (edict_t *ent);
void SP_misc_mug (edict_t *ent);
void SP_misc_glass (edict_t *ent);
void SP_misc_plate (edict_t *ent);
void SP_misc_brown_bottle (edict_t *ent);
void SP_misc_green_bottle (edict_t *ent);
void SP_misc_noose (edict_t *ent);

