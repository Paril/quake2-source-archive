/* included from g_local.h */



/**************/
/* override.c */

extern uint8_t dmg_override;
extern uint8_t score_override;
extern cvar_t* g_override_damages;

typedef struct dmgoverride_s {
    int16_t hitpoints;
    uint16_t knockback;
    uint16_t flags; //do_
    uint16_t radius; //per weapon, victim doesn't alter this
    int8_t frags; //frags given (or taken) if victim dies
    int8_t score; //score given (or taken) if victim dies
    int8_t tkcounter;
    uint8_t uflags; //which fields are in use
    uint8_t weaponindex; //w_ constant for use when converting hitscan into explosion
    float fragsper10tkpoint; //extra frags to subtract for each 10 tk counter points (before adding new tk counter)
    float ascale; //how much armour is lost from inventory for each hitpoint taken
    float lshead;
    float lstorso;
    float lslegs;
    float lsstep;
} dmgoverride_t;

typedef struct explosioninfo_s {
    uint16_t radius;
} explosioninfo_t;

typedef struct rewardinfo_s {
    uint8_t instant_h_points; //dmg points given per HP immediately
    uint8_t instant_a_points; //dmg points given per AP immediately
    uint8_t coop_points; //dmg points given on kill	(only clients)
    uint8_t kill_points; //dmg points given for last hit
} rewardinfo_t;

enum { //dmg override flags
    do_linear_explosion = 1, //force explosion to be linear
    do_original_explosion = 2, //force explosion to be dmg-distance
    do_force_explosion = 3, //Force to default type of explosion
    do_kill_egg = 4, //Prevents egg from regrowing, killing it definitively
    do_piercing = 8,
    do_no_tk_players = 0x10,
    do_no_tk_structures = 0x20,
    do_tk_players = 0x40,
    do_tk_structures = 0x80,
    do_mass_scale = 0x100, //mowning damage uses this. Bigger mass does more dmg, bigger mass takes less dmg.
    do_volume_scale = 0x200, //box size affects damage. Bigger box takes more damage. Ignores attacker size.
    do_no_health_dmg = 0x400, //Stops damage from affecting health, but armour can be killed (used for breeder vs mech)
    do_no_armour_dmg = 0x800, //Stops damage from draining armour, but it still can protect reducing damage on health
    do_ignore_armour = 0x1000, //Completely ignore armour.
    do_revenge = 0x2000, //All teamkills of guardian C4 revenge are given to the owner(s) of the C4 (once each up to the number of eaten C4 in order of eating)
    do_no_self_dmg = 0x4000,
    do_no_locations = 0x8000, //When damage is not an explosion will use location scalers, unless this flag is set

    dou_knockback_set = 1, //dmg.knockback has a valid value. If this flag is not set, will fall back to default
    dou_damage_set = 2, //dmg.damage has a valid value. If this flag is not set, will fall back to default
    dou_other_set = 4, //any field not above has been set
};

typedef struct cvaroverride_s {
    int8_t fire_mode;
    int8_t mech_rockets;
    int8_t mech_rockets_count;
    int8_t hatchling_flags;
    float autoslash_delay;
    float exterm_pulse_overheat;
    uint32_t inven_refill_flags;
    int8_t spiker_flags;
    int8_t fragmode;
} cvaroverride_t;

enum {
    nocvaroverride_fire_mode = -1,
    nocvaroverride_mech_rockets = -1,
    nocvaroverride_hatchling_flags = -1,
    nocvaroverride_autoslash_delay = -1,
    nocvaroverride_exterm_pulse_overheat = -1,
    nocvaroverride_inven_refill_flags = 0x80000000,
    nocvaroverride_spiker_flags = -1,
    nocvaroverride_fragmode = -1
};

extern cvaroverride_t cvaroverride;

#define CDIGITS(n) ((n==1)?10:(n==2)?100:1000)

#define SETTING_INT(name) ((abs((int)g_##name->value) <10000 && cvaroverride.name != nocvaroverride_##name)?cvaroverride.name:((int)g_##name->value)%10000)
#define SETTING_COUNT(name, n) ((abs((int)g_##name->value) <10000 && cvaroverride.name != nocvaroverride_##name)?cvaroverride.name##_count:((int)fabs(fmod(g_##name->value, 1)*CDIGITS(n))))
#define SETTING_FLT(name) ((fabs(g_##name->value) <10000 && cvaroverride.name != nocvaroverride_##name)?cvaroverride.name:fmod(g_##name->value, 10000))
#define SETTING_BF(name) ((!((int)g_##name->value & 0x80000000) && cvaroverride.name != nocvaroverride_##name)?cvaroverride.name:(int)g_##name->value)

int OverrideDamage(dmgoverride_t*dmg, edict_t*victim, edict_t*inflictor, edict_t*attacker, int dflags, int mod);

int GetExplosionInfo(explosioninfo_t*ei, edict_t*inflictor, edict_t*attacker, int mod);

float OverrideSlashDistance(edict_t*ent, float original_range, int mod);

void LoadDamageOverrideTable();

const char* GetWeaponName(uint8_t index);

void ModifyOverride(edict_t*ent, char*buf); //for rcon modifydamages and admin @modifydamages


//New data not in original game
rewardinfo_t GetCombatReward(edict_t*attacker, edict_t* inflictor, edict_t*target, int mod_weapon);

