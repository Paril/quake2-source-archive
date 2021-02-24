#define CTF_VERSION			1.02
#define CTF_VSTRING2(x) #x
#define CTF_VSTRING(x) CTF_VSTRING2(x)
#define CTF_STRING_VERSION  CTF_VSTRING(CTF_VERSION)

// GRIM
typedef enum {
	CTF_GRAPPLE_STATE_FLY,
        CTF_GRAPPLE_STATE_ATTACHED,
        CTF_GRAPPLE_STATE_HANG,
        CTF_GRAPPLE_STATE_PULL,
        CTF_GRAPPLE_STATE_SLACK
} ctfgrapplestate_t;
// GRIM

#define DF_CTF_FORCEJOIN	131072	
#define DF_ARMOR_PROTECT	262144
#define DF_CTF_NO_TECH      524288

#define CTF_CAPTURE_BONUS		15	// what you get for capture
#define CTF_TEAM_BONUS			10	// what your team gets for capture
#define CTF_RECOVERY_BONUS		1	// what you get for recovery
#define CTF_FLAG_BONUS			0	// what you get for picking up enemy flag

// what you get for fragging enemy flag carrier
#define CTF_FRAG_CARRIER_BONUS  1
// GRIM - New, if its your flag, get 2, otherwise 1
#define CTF_FRAG_YOUR_CARRIER_BONUS  2
// GRIM

#define CTF_FLAG_RETURN_TIME	40	// seconds until auto return

#define CTF_CARRIER_DANGER_PROTECT_BONUS	2	// bonus for fraggin someone who has recently hurt your flag carrier
#define CTF_CARRIER_PROTECT_BONUS			1	// bonus for fraggin someone while either you or your target are near your flag carrier
#define CTF_FLAG_DEFENSE_BONUS				1	// bonus for fraggin someone while either you or your target are near your flag
#define CTF_RETURN_FLAG_ASSIST_BONUS		1	// awarded for returning a flag that causes a capture to happen almost immediately
#define CTF_FRAG_CARRIER_ASSIST_BONUS		2	// award for fragging a flag carrier if a capture happens almost immediately

#define CTF_TARGET_PROTECT_RADIUS			400	// the radius around an object being defended where a target will be worth extra frags
#define CTF_ATTACKER_PROTECT_RADIUS			400	// the radius around an object being defended where an attacker will get extra frags when making kills

#define CTF_CARRIER_DANGER_PROTECT_TIMEOUT	8
#define CTF_FRAG_CARRIER_ASSIST_TIMEOUT		10
#define CTF_RETURN_FLAG_ASSIST_TIMEOUT		10

#define CTF_AUTO_FLAG_RETURN_TIMEOUT		30	// number of seconds before dropped flag auto-returns

#define CTF_TECH_TIMEOUT					60  // seconds before techs spawn again

#define CTF_GRAPPLE_SPEED					650 // speed of grapple in flight
#define CTF_GRAPPLE_PULL_SPEED				650	// speed player is pulled at

char *CTFTeamName(int team);
char *CTFOtherTeamName(int team);
void CTFAssignSkin(edict_t *ent, char *s);
//GRIM
edict_t *SelectCTFSpawnPoint (edict_t *ent);
qboolean CTFPickup_Flag(edict_t *ent, edict_t *other);
// GRIM
void CTFDrop_Flag(edict_t *ent, gitem_t *item);
// GRIM
void CTFEffects(edict_t *player);
void CTFDeadDropFlag(edict_t *self);
void CTFSay_Team(edict_t *who, char *msg);
void CTFFlagSetup (edict_t *ent);
void CTFFragBonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker);
void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker);

// GRAPPLE
void CTFWeapon_Grapple (edict_t *ent);
void CTFPlayerResetGrapple(edict_t *ent);
void CTFGrapplePull(edict_t *self);
void CTFResetGrapple(edict_t *self);

//TECH
gitem_t *CTFWhat_Tech(edict_t *ent);
qboolean CTFPickup_Tech (edict_t *ent, edict_t *other);
void CTFDrop_Tech(edict_t *ent, gitem_t *item);
void CTFDeadDropTech(edict_t *ent);
void CTFSetupTechSpawn(void);
int CTFApplyResistance(edict_t *ent, int dmg);
int CTFApplyStrength(edict_t *ent, int dmg);
qboolean CTFApplyStrengthSound(edict_t *ent);
qboolean CTFApplyHaste(edict_t *ent);
void CTFApplyHasteSound(edict_t *ent);
void CTFApplyRegeneration(edict_t *ent);
qboolean CTFHasRegeneration(edict_t *ent);
void CTFRespawnTech(edict_t *ent);

void SP_misc_ctf_banner (edict_t *ent);
void SP_misc_ctf_small_banner (edict_t *ent);

void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);

void SP_trigger_teleport (edict_t *ent);
void SP_info_teleport_destination (edict_t *ent);
