//=======================================================================
//=======================================================================

void gi_cprintf(edict_t *ent, int printlevel, char *fmt, ...);
void gi_centerprintf(edict_t *ent, char *fmt, ...);
void gi_bprintf(int printlevel, char *fmt, ...);

void Use_Plat(edict_t *ent, edict_t *other, edict_t *activator);
void train_use(edict_t *self, edict_t *other, edict_t *activator);
void button_use(edict_t *self, edict_t *other, edict_t *activator);
void door_use(edict_t *self, edict_t *other, edict_t *activator);
void rotating_use(edict_t *self, edict_t *other, edict_t *activator);
void trigger_relay_use(edict_t *self, edict_t *other, edict_t *activator);
void plat_go_up(edict_t *ent);
void SV_Physics_Step(edict_t *ent);
void path_corner_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void DoRespawn(edict_t *ent);
void Cmd_Kill_f(edict_t *ent);
void ClientUserinfoChanged(edict_t *ent, char *userinfo);
void ClientDisconnect(edict_t *ent);
void Use_Item(edict_t *ent, edict_t *other, edict_t *activator);

//weapon
int WeapIndex(int weap);
void Weapon_Sword (edict_t *ent);
void Weapon_Blaster (edict_t *ent);
void Weapon_Shotgun (edict_t *ent);
void Weapon_SuperShotgun (edict_t *ent);
void Weapon_Machinegun (edict_t *ent);
void Weapon_Chaingun (edict_t *ent);
void Weapon_AssRifle (edict_t *ent);
void Weapon_Laser (edict_t *ent);
void Weapon_RocketLauncher (edict_t *ent);
void Weapon_Grenade (edict_t *ent);
// ion mine support
void Weapon_Proxmine (edict_t *ent);
void Weapon_GrenadeLauncher (edict_t *ent);
void Weapon_Railgun (edict_t *ent);
void Weapon_BFG (edict_t *ent);

void LoadBotNames(void);
int GetKindWeapon(gitem_t *it);
//void ReadRouteFile(void);
void ReadRouteFile(void);
void droptofloor2(edict_t *ent);
void SpawnNumBots(int num);
void RespawnAllBots(void);
void TauntVictim(edict_t *ent, edict_t *victim);
void InsultVictim(edict_t *ent, edict_t *victim);
qboolean Pickup_Navi(edict_t *ent, edict_t *other);
void BotThink(edict_t *ent);
void CheckCampSite(edict_t *ent,edict_t *other);
void bFuncPlat(edict_t *ent);
void bFuncButton(edict_t *ent);
void bDoorBlocked(edict_t *ent);
void bFuncDoor(edict_t *ent);
void bFuncTrain(edict_t *ent);

void bFuncExplosive(edict_t *ent);

void ResetGroundSlope(edict_t *ent);
void TraceAllSolid(edict_t *ent, vec3_t point, trace_t tr);
void CheckPrimaryWeapon(edict_t *ent, edict_t *other);
void InitAllItems(void);
void G_FindTrainTeam(void);
void ForceRouteReset(edict_t *other);
float SetBotXYSpeed(edict_t *ent, float *xyspeed);
void SetBotThink(edict_t *ent);
void CheckBotCrushed(edict_t *targ,edict_t *inflictor,int mod);
void BotCheckEnemy(gclient_t *client, edict_t *attacker, edict_t *targ, int mod);
void RemoveAllBots(void);
void BotCheckGrapple(edict_t *ent);
