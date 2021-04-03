#define BRAZEN_DATE "16/07/99f"

#define LEAD_LIMIT 96 // 192
#define INSTANT_LEAD_LIMIT 32 // 64

typedef enum {
        CS_NOT_READY,
        CS_DEAD,
        CS_READY,
        CS_PLAYING
} clientstate_t;

typedef enum {
        MATCH_CHECKING,
        MATCH_PRE_START,
        MATCH_START,
        MATCH_DURING,
        MATCH_FINISHED
} matchstate_t;

// bflags
#define BF_CHOOSE_STUFF         1
#define BF_JOHN_WOO             2
#define BF_LIGHTS_OUT           4
#define BF_MATRIX               8
// FIX ME - insert more here...
#define BF_CTF_ANYTIME          128
#define BF_CTF_TOUCH            256
#define BF_FLAGS_STAY           512
#define BF_FLAGS_MBCB           1024
#define BF_CTF_R_RESPAWN        2048

// Real Flags
#define RF_NO_EQUIPMENT         1
#define RF_NO_GLOWING           2
#define RF_NO_DEATHMESSAGES     4
#define BF_NO_BLEED_DEATH       8
#define RF_USE_HANDS            16

#define W_EMPTY                 1  // Weapon empty (out of ammo etc)
#define W_WATER_JAMMED          2
#define W_ALTERNATE1            4
#define W_ALTERNATE2            8
#define W_ALTERNATE3            16
// Dirty
#define W_UNLOADED              32 // <- for shotguns and sniper rifle
// Dirty

#define rndnum(y,z) ((random()*((z)-((y)+1)))+(y))

void MakeObserver(edict_t *ent);
void ThrowEmIn(edict_t *ent);

void UpdateChoosingMenu(edict_t *ent);
qboolean StartClient(edict_t *ent);

void AdjustTeamScore(teams_t *team, int change, qboolean force);
void AdjustScore(edict_t *player, int change, qboolean force);
edict_t *SelectRandomDMSpot ();

int GetHitLocation (edict_t *targ, vec3_t dir, vec3_t point, int dflags, int mod);
void Calc_Arc (edict_t *ent);

// HitLocation defines...
#define HIT_FRONT               1
#define HIT_BACK                2
#define HIT_HEAD                4
#define HIT_CHEST               8
#define HIT_MID                 16
#define HIT_RIGHT_LEG           32
#define HIT_LEFT_LEG            64
#define HIT_UPPER_LEGS          128
#define HIT_RIGHT_ARM           256
#define HIT_LEFT_ARM            512
#define HIT_UPPER_ARMS          1024
#define HIT_SOAKED              2048
#define HIT_THROAT              4096
#define HIT_UPPER_HEAD          8192
#define HIT_SLOWED              16384 // Dirty

        
// Wounds area defines...
#define W_HEAD                0
#define W_CHEST               1
#define W_RLEG                2
#define W_LLEG                3
#define W_RARM                4
#define W_LARM                5
                
// Type Of Damage flags...
#define TOD_LEAD               1
#define TOD_PELLET             2
#define TOD_AP                 4
#define TOD_HV                 8
#define TOD_EXPLOSIVE          16
#define TOD_RADIUS             32
#define TOD_NO_ARMOR           64
#define TOD_NO_KNOCKBACK       128
#define TOD_NO_PROTECTION      256
#define TOD_NO_WOUND           512

// Dirty
// Important to spread modifiers
extern float   xyspeed;
// Dirty

// z_brazen.c
void BraZenInit(void);
void OpenChoosingMenu(edict_t *ent, pmenu_t *p);
void StageTwo(edict_t *ent, pmenu_t *p);
void OpenChooseWeaponMenu(edict_t *ent);
void ChosenEquipMent (edict_t *ent);
void HideSpot(edict_t *ent);
void UnHideSpot(edict_t *ent);
void HideAllSpots(void);
void UnHideAllSpots(void);
qboolean CheckBox (edict_t *ent);
void SelectReStartPoint (edict_t *ent);
void BrazenCmdsInit(edict_t *ent);

// other
void Cmd_WeapLast_f (edict_t *ent);
qboolean IsFemale (edict_t *ent);

// z_board.c
void TeamScoreBoard (edict_t *ent);
void Say_Team(edict_t *who, char *msg);

// Audio Code Stuff - Based on Expert TeamAudio

// z_comm.c
#define WAVE_FLIPOFF    0
#define WAVE_SALUTE     1
#define WAVE_TAUNT      2
#define WAVE_WAVE       3
#define WAVE_POINT      4

// Reply priorities
typedef enum {
        MATE_DOWN,        // Buddie just bought it
        HURT,             // Just been wounded
        ENEMY_DOWN,       // Killed/Wasted an enemy
        ENEMY_HERE,       // Visualed the NME & they're close!
        ENEMY_SPOTTED,    // Visualed the NME
        TAKING_FIRE,      // Been shot or shot at, no damage
        IDLE              // Request made for the living
} event_t;

// Request priorities - Based on events stated by the reply info...
typedef enum {
        REQUEST_ONE, // Ignore this...
        REQUEST_BACK_UP,// under fire or team mate down, want help...
        REQUEST_MEDIC,  // hurt or team mate down, want health/medic...
        REQUEST_INFO,   // Nothing happened for a while Gimme info...
        REQUEST_PRAISE,
        REQUEST_NOTHING  
} request_t;            
                        

void Cmd_Audio_f(edict_t *player);
void wave(edict_t *ent, int waveNum);
qboolean InRange (edict_t *ent1, edict_t *ent2, float range);
void Cmd_Reply_f (edict_t *ent);
void Cmd_Request_f (edict_t *ent);
void NewEvent (edict_t *ent, int event);
void KilledMakeEvent (edict_t *attacker, edict_t *targ);

// p_weapon.c
void ChangeWeapon2 (edict_t *ent);
void Reset_Weapon (edict_t *ent);
void Cmd_Weapon_OffHand (edict_t *ent);
void Cmd_Weapon_GoodHand (edict_t *ent);
qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void Drop_CurrentWeapon (edict_t *ent, int which);
void Drop_Weapon (edict_t *ent, gitem_t *item);
void Drop_Pack (edict_t *ent, gitem_t *item);
void SetSlot (edict_t *ent, char *slot_name, char *pickup_name, int flags, int damage, char *ammo, int quantity, char *s_ammo, int s_quantity);
void ChangeQuick (edict_t *ent, gitem_t *item, qboolean left);

void Cmd_DropWepQuick (edict_t *ent);
int ContEffect (edict_t *ent);
void Cmd_Hands_f (edict_t *ent);
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);

// z_match.c
void DisplayCenterMessageAll(char *message);
void CheckMatchStart(void);
void SetupMatch(void);
void MatchWinner(void);
void MatchOverCheck(void);
void MatchOutOfTime(void);
void SoundToAllPlayers(int soundIndex, float volume);
qboolean MatchInProgress(void);
void PracticeReset(void);

// z_misc.c
void HealthCalc (edict_t *ent);
void Cmd_Action_On (edict_t *ent);
void Cmd_Action_Off (edict_t *ent);
qboolean ApplyHealth (edict_t *ent, int count);
qboolean BindWounds (edict_t *ent, int count);
void ToastThatFucker (edict_t *self);
void Cmd_IdState_f (edict_t *ent);
int Wounded(edict_t *ent);
int ArmWounds(edict_t *ent);
int LegWounds(edict_t *ent);
void WoundAdjust(edict_t *ent, int head, int chest, int lleg, int rleg, int larm, int rarm);
void fire_blood (edict_t *self, vec3_t start, vec3_t dir, int speed);
void InitOtherQues (void);
void TakeFromBHoleQue (edict_t *ent, char *model, vec3_t pos, vec3_t angle);
void SpawnBloodPool (edict_t *self);
void SetIDView(edict_t *ent);

// g_items.c or z_items.c
void DoRespawn (edict_t *ent);
void ED_CallSpawn (edict_t *ent);
void Drop_Ammo_Specific (edict_t *ent, gitem_t *item, int count);
void RemoveItemWeight (edict_t *ent, gitem_t *item);
void Drop_Armor (edict_t *ent, gitem_t *item);
void Use_Health (edict_t *ent, gitem_t *item);
void Use_Hologram (edict_t *self);
void Drop_ArmorQuick (edict_t *ent);
void SetArmorHUD (edict_t *ent);
void ShowItems(edict_t *ent);
void TossVisualItems(edict_t *ent);
void GrenadeTrap (edict_t *ent, gitem_t *item);

qboolean Pickup_Powerup (edict_t *ent, edict_t *other);

weapons_t *FindClientWeapon (edict_t *ent, char *pickup_name, int occ);
weapons_t *FindSlot (edict_t *ent, char *slot_name);
weapons_t *FindBlank (edict_t *ent, int occ);

void RemoveAmmoWeight (edict_t *ent, gitem_t *item, float count);
void droptofloor (edict_t *ent);
void SetupItems (edict_t *ent);
void Cmd_Light_f (edict_t *ent);
void Drop_Flashlight (edict_t *ent, gitem_t *item);
void Use_Light (edict_t *ent, gitem_t *item);
qboolean Pickup_Flashlight (edict_t *ent, edict_t *other);
void SetSpeed (edict_t *ent);
void SetWeight (edict_t *ent);
void BadCombo (edict_t *ent);

typedef struct
{
        char            *name;     
} clips_t;

extern clips_t cliplist[];

#define CLIP_INDEX(x) ((x)-cliplist)
clips_t *FindClip (char *name);
void Drop_Clip (edict_t *ent, gitem_t *item, int count);

gitem_t *FindCombo (char *first, char *second, qboolean anything);
gitem_t *FindComboByName (char *name);

// Drop_Item flags
#define ITEM_UNDEF   0
#define ITEM_DROPPED 1
#define ITEM_THROWN  2
#define ITEM_PLACED  3

#define WEP_INDEX(x) ((x)-weaponlist)

// z_corpse.c
void CorpseDropItem (edict_t *ent);
void SelectNextCorpseItem (edict_t *ent, int itflags);
void SelectPrevCorpseItem (edict_t *ent, int itflags);
void ValidateSelectedCorpseItem (edict_t *ent);

// g_ctf.c
qboolean loc_CanSee (edict_t *targ, edict_t *inflictor);
void CTFResetFlag(teams_t *team);
void CreateBaseLaser (edict_t *ent, int team);
void SetupMultiAmmo(void);
void stuffcmd(edict_t *ent, char *s);  
void SetupSpecialsSpawn(void);
void DropFlag(edict_t *self);
void CTFDropFlagTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void CTFDropFlagThink(edict_t *ent);
void create_base (edict_t *ent, qboolean red);
void CTFBaseInit(void);
void CTFReSetupTechSpawn(void);
void ClearSpawners(void);
void EndDMLevel (void);

/*
===============
Grenade Defs
===============
*/
#define GREN_HAND       1
#define GREN_HELD       2
#define GREN_CONTACT    4
#define GREN_CLUSTER    8
#define GREN_FLASH      16

// Cluster grenade defs...
#define CLUSTER_DAMAGE	120	// points of damage
#define CLUSTER_TIME	1.0	// seconds

void CheckForWeapons(edict_t *ent);
void WeaponsStayThink (edict_t *ent);

// Special Item defines...
#define SI_FLASH_LIGHT          1
#define SI_LASER_SIGHT          2
#define SI_STEALTH_SLIPPERS     4
#define SI_SILENCER             8
#define SI_SILENT               16
#define SI_LASER_SIGHT_HELP     32

void UpdateTrace (edict_t *ent);
void SwitchGuns(edict_t *ent);
void CheckForSpecials(edict_t *ent);
void RespawnThisWeapon (char *classname);
qboolean ObjectAndFunction (edict_t *ent);
void Drop_Bandolier (edict_t *ent, gitem_t *item);
void SpawnSpecialItems(edict_t *ent);
void TakingFireCheck (edict_t *ent, vec3_t point);

void ChangeThings (edict_t *ent, pmenu_t *p);
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);
void SetGoal1 (edict_t *ent);
void SetGoal2 (edict_t *ent);
void SendAudio (edict_t *ent, char *filename, int scope, int event);
qboolean FloodCheck (edict_t *ent);
qboolean point_infront (edict_t *self, vec3_t point);


char *strtostr2 (char *s);

#define CHASE_ROTATE       1
#define CHASE_POV          2
qboolean CheckSkin(edict_t *ent, char *s);
void general_reset (edict_t *ent);
void SpawnThinker (gitem_t *item, float delay);
void ChangeMenu(edict_t *ent);

void Drop_Sentry (edict_t *ent, gitem_t *item);
void ZoomOff (edict_t *ent);
void OpenMarineMenu(edict_t *ent);
void ShowProg (edict_t *ent, int current, int max);
qboolean UnderWaterCheck (vec3_t point);

void Cmd_MeGoal_f (edict_t *ent);
void DisplayGoalCenter (edict_t *ent);
void GoalDescription (edict_t *ent);
void SetHelpChanged(int team_index);

// Paranoid style CTF
void SingleFlagSetup(void);
void SingleFlagPlace (edict_t *ent, edict_t *spot);
void SingleFlagThink (edict_t *ent);
edict_t *SelectFarthestFlagSpotSpawnPoint (void);
void TechThink(edict_t *tech);
// Paranoid style CTF

void GibClient (edict_t *ent, int damage);

// Actions
#define A_SOMERSAULT    1
#define A_BACKFLIP      2
#define A_RIGHTFLIP     4
#define A_LEFTFLIP      8
#define A_JUMPED        16 // Dirty - Matrix mode part
#define A_WANT_FLIP     32 // Dirty - Paril fix

void CreateBodyInventory (edict_t *ent);
void CopyToBodyQue (edict_t *ent);

void MuzzleFlash (edict_t *ent, char *soundname, int left);
//void RunTracker (void);

// Dirty
#define A_KNIFE_THROW           1 // Dirty

void SwitchSight (edict_t *ent);
void Drop_Slippers (edict_t *ent, gitem_t *item);
qboolean Pickup_Slippers (edict_t *ent, edict_t *other);
void Drop_LaserSight (edict_t *ent, gitem_t *item);
qboolean Pickup_LaserSight (edict_t *ent, edict_t *other);
void Drop_Silencer (edict_t *ent, gitem_t *item);
qboolean Pickup_Silencer (edict_t *ent, edict_t *other);
qboolean Grab_n_Climb (edict_t *ent);
qboolean HangingCheck (edict_t *ent);
qboolean CheckHang (edict_t *ent);
// Dirty

void TracenPlace (edict_t *ent, int y, int z);
void ChooseNextStep (edict_t *ent, pmenu_t *p);
void OpenInitialMenu(edict_t *ent, pmenu_t *p);
qboolean IsOurTeam (edict_t *ent, int teamindex);
void SetLargeWeaponHUD (edict_t *ent);
void Cmd_Weapon_Reload (edict_t *ent);
void shit_free (edict_t *self);
qboolean ShitAvailable (void);
qboolean AllyCheck (edict_t *targ, edict_t *attacker);


void SetWoundsHud (edict_t *ent);
void SetAmmoHud (edict_t *ent);
void A1HudOff (edict_t *ent);
void A2HudOff (edict_t *ent);
void NoLongerChase (edict_t *ent);
void HelpMenu (edict_t *ent, pmenu_t *p);
edict_t *FindEdictByClassnum (char *classname, int classnum); // AQ2 code
qboolean AllyCheckOk (edict_t *ent, edict_t *other);
qboolean LiveGrenadeCheck (edict_t *ent);
void SetHands (edict_t *ent);

