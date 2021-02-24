// m_actor.c
void FindSomeMonster (edict_t *self);


//g_combat.c
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage);

// z_client.c
void z_PutClientInServer(edict_t *ent);
void z_InitClientPers(edict_t *ent);

// g_weapon.c
void fire_cgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, int ammotype);

// z_weapon.c
#define IDLE_DELAY      8       // FIX ME - 8 seconds too long??
void Calc_Arc (edict_t *ent);
void SetupItemModels(edict_t *ent);
void ChangeRightWeapon(edict_t *ent);
void ChangeLeftWeapon(edict_t *ent);
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void PlayerNoise(edict_t *who, vec3_t where, int type);
qboolean CheckAltAmmo (edict_t *ent, gitem_t *curweapon, int ammoTag);
void AutoSwitchWeapon(edict_t *ent, int last_right, int last_left);

// z_items.c
void DroppedThink( edict_t *ent );
int CountItemByTag(edict_t *ent, int index);
gitem_t	*GetItemByTag (int tag);
void RemoveItem (edict_t *player, int bodyarea);
edict_t *LaunchItem (edict_t *ent, gitem_t *item, vec3_t origin, vec3_t velocity);
void GiveItem(edict_t *ent, int tag);
void DropClip(edict_t *ent, int index, int count);
void ReloadHand (edict_t *ent, int hand);
qboolean CanRightReload (edict_t *ent);
qboolean CanLeftReload (edict_t *ent);
void UpdateExtraAmmoCounts (edict_t *ent);
int GetFreeBodyArea (gitem_t *item, edict_t *ent);
void StashItem (edict_t *player, gitem_t *item, int bodyarea, int quantity, int flags, int ammotype);
void RemoveItem (edict_t *player, int bodyarea);
int CountOffHandGrenades(edict_t *ent, int type);
qboolean Pickup_BAItem (gitem_t *item, int count, int flags, int ammotype, edict_t *other);

// z_handgrenade.c
void ThrowOffHandGrenade(edict_t *ent);

// z_coop.c
void G_RunEditFrame (void);
void StepShake(vec3_t pos, float dist, float speed);
void MonsterDropItem (edict_t *self, gitem_t *item, int count, int flags, int ammoTag);
void G_TouchDeadBodies(edict_t *ent);
void EndCoopView(edict_t *ent);
void CheckCoopAllDead (void);

// z_cmds.c
qboolean z_ClientCommand (edict_t *ent);
edict_t *ThrowBodyAreaItem(edict_t *ent, int speed, int bodyarea);
void ThrowLeftHandItem(edict_t *ent, int speed );
void ThrowRightHandItem(edict_t *ent, int speed );
void DrawItemSelect(edict_t *ent);
void UpdateInv(edict_t *ent);
void CmdCoopView(edict_t *ent);

#define CYCLE_ITEMS_TIME        1
#define MAX_WEAPONS             16

// means of death
#define MOD_PISTOL		34
#define MOD_TWIN_PISTOL		35
#define MOD_ARIFLE		36
#define MOD_EXP_SHOTGUN		37
#define MOD_SOLID_SHOTGUN	38
#define MOD_HV_SUBMACH		39
#define MOD_EMP_HANDGRENADE	40
#define MOD_EMP_HG_SPLASH	41
#define MOD_EMP_GRENADE		42
#define MOD_EMP_G_SPLASH	43
#define MOD_HEP_GRENADE		44
#define MOD_HEP_G_SPLASH	45

// edict->flags
#define	FL_CYBERNETIC	0x00020000	// creature is cyber-netic components, thus greatly affected by EMP grenades etc



// hit locations
#define LOCATION_NONE           0

#define LOCATION_FRONT          1
#define LOCATION_LEFT           2
#define LOCATION_RIGHT          4
#define LOCATION_BACK           8

#define LOCATION_HEAD           16
#define LOCATION_CHEST          32
#define LOCATION_STOMACH        64
#define LOCATION_LEGS           128
#define LOCATION_ARMS		256

// GRIM 26/06/2001 11:44AM - holds current weapon info
#define CSTAT_RIGHTHAND                 0
#define CSTAT_RIGHTHAND_AMMO            1
#define CSTAT_RIGHTHAND_FLAGS		2
#define CSTAT_RIGHTHAND_AMMOTYPE	3
#define CSTAT_LEFTHAND                  4
#define CSTAT_LEFTHAND_AMMO             5
#define CSTAT_LEFTHAND_FLAGS            6
#define CSTAT_LEFTHAND_AMMOTYPE		7
// GRIM
