/*
=======================
HOLY WARS: declarations
=======================
*/


// Status of player:
typedef enum {
        SINNER,
        SAINT,
        HERETIC,
        OBSERVER
} hw_playerstatus_t;


// Status of halo:
typedef enum {
        NONE,           // The Halo entity never spawned yet.
        HEAVEN,         // The Halo is waiting to respawn in base.
        BASE,           // The Halo is in its spawning position.
        OWNED,          // The Halo is on the head of a Saint.
        FREE            // The Saint was killed, the Halo fell somewhere.
} hw_halostatus_t;

// Game parameters:
typedef struct
{
        int    SCORE_NormalKill;        // Normal frag value.
        int    SCORE_HolyKill;          // Frags for the Saint when killing someone else.
        int    SCORE_KillHeretic;       // Bonus frags when killing an heretic.
        int    SCORE_KillSaint;         // Frags when killing the Saint.
        int    SCORE_TakeHalo;          // Frags when taking the Halo.
		int SCORE_BecomeHeretic;		// Frags for becoming an Heretic.
		int SCORE_Llama;					// Get lower than this and you get kicked.
        float  RULES_HeresyThreshold;   // Threshold to pass to become an heretic.
        int    RULES_SinnerDamage;      // True if sinners can always damage each other
        float  HERESY_MeleeRange;       // melee range for heresy calc.
        float  HERESY_NearRange;        // near  range for heresy calc.
        float  HERESY_FarRange;         // far   range for heresy calc.
        int    TIMES_FirstSpawn;        // Time before first halo spawn.
        int    TIMES_Disappear;         // Time before a fallen halo disappears.
        int    TIMES_Respawn;           // Time before halo respawns.
        int    BONUS_HealthForEnemy;    // Bonus health for the Saint for each enemy.
        int    BONUS_ArmorForEnemy;     // Bonus armor for the Saint for each enemy.
        int    BONUS_Max;               // The maximum total bonus allowed.
        int    MISC_ReportFreq;         // Frequency of on-screen reports.
		int    MISC_IntermissionTime;   // Minimum pause for intermission between levels.
        int    MISC_SaintSound;         // Saint shoot sound on/off.
        int    MISC_UseBFG;             // If false, all BFGs are turned into hyperblasters.
        int    MISC_ShootHalo;          // If true, the fallen halo can be shot and moved
        int    PHYS_HaloInertia;        // Inertia effect for the halo on/off.
        int    PHYS_HaloHeight;         // The height of the halo over the Saint's eyes.
        int    PHYS_HaloTolerance;      // Tolerance for halo position control.
        int    PHYS_HaloVelocity;       // Maximum halo velocity (for each axis).
        vec3_t PHYS_HaloAccel;          // Halo accelerations.
        vec3_t PHYS_HaloBrake;          // Halo decelerations.
        int    PHYS_CyclesForFrame;     // Number of cycles for phisics calculations.
        int    PHYS_HaloFall;           // Multiplier of velocity for falling halo.

} hw_parameters_t;

// Game globals:
typedef struct
{
        edict_t *halobase;         // The halo base spot.
        double  nextreport;        // The time when the next report will happen.
        int     teleported;        // True if the Saint was just teleported.
        int     halostatus;        // The status of the halo.
        int     lasthalosound;     // Used for sound control
} hw_game_t;


extern hw_game_t hw;            // game globals
extern cvar_t   *hw_edit;       // console variable: toggle "editing mode"
extern cvar_t   *hw_debug;      // console variable: toggle debugging


// FUNCTIONS:

// hw_main.c:
void HW_InitGame(void);
void HW_CreateHaloBase(void);
void HW_Frag(edict_t *killed, edict_t *killer, int mod, char *weapon);
void HW_BecomeHeretic(edict_t *p);
void HW_BecomeSaint(edict_t *p);
void HW_PlayerDie(edict_t *ent);
void HW_SwarmThink(edict_t *ent);
void HW_SetClientEffects(edict_t *player);
void HW_SaintShootSound(edict_t *ent);
void HW_EndOfLevel(void);

// hw_halo.c:
void HW_SpawnHalo(edict_t *spot);
qboolean HW_PickupHalo(edict_t *ent, edict_t *other);
void HW_VisibleHaloThink(edict_t *self);
void HW_DropHalo(edict_t *player);
void HW_HaloDisappear(edict_t *ent);
void HW_HaloRespawn(edict_t *ent);
void HW_HaloReset(void);

// hw_utils.c:
void HW_ReportToRespawningPlayer(edict_t *self);
void HW_ReportSaint(edict_t *saint);
void HW_ReportNewSaint(edict_t *saint);
void HW_ReportMartyr(edict_t *saint);   // HWv2.1
void HW_ReportNewHeretic(edict_t *heretic);
int  HW_CountEnemies(edict_t *p);
int  HW_GetPlayerAndMediumPing(int *mean_ping);
// HWv2.1 - Next function changed name
void HW_DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer);  // HWv2.1
void HW_DeathmatchScoreboard (edict_t *ent);	// HWv2.1
vec3_t *HW_LoadHaloPos(char *map_name);
int  HW_SaveHaloPos(char *map_name, vec3_t position);
int  HW_BaseInLevel(edict_t *self);
void HW_DumpEntities(void);

// hw.heres.c
void HW_Heresy_MainCalc(edict_t *dead, edict_t *killer, int mod);
void HW_Heresy_SaintKilled(edict_t *killer);

// cnf_load.c:
int CNF_LoadVariables(void);

