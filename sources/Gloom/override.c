#include "g_local.h"
#include "glob.h"

//#define SPAMMY_DEBUG

#ifndef LENGTH
#define LENGTH(a) (sizeof(a)/sizeof(a[0]))
#endif

#define eprintf(...) {fprintf(stderr, __VA_ARGS__); fflush(stderr);}

uint8_t dmg_override;
uint8_t score_override;

typedef enum {
    w_unknown,

    w_engi_poke,
    w_autogun,
    w_grunt_pistol,
    w_shotgun,
    w_st_pistol,
    w_ex_shell,
    w_st_grenade,
    w_scattergun,
    w_bio_pistol,
    w_bio_grenade,
    w_ht_rocket,
    w_ht_pistol,
    w_ht_shrapnel,
    w_smg,
    w_magnum,
    w_cmd_grenade,
    w_cmd_shrapnel,
    w_c4,
    w_c4_failed,
    w_laser,
    w_pulse, //for 60 cell, need to scale for grim's charged pulse
    w_xt_grenade,
    w_xt_shrapnel,
    w_mech_cannon,
    w_mech_bay_rocket,
    w_mech_grim_rocket,
    w_mech_drunk_rocket,
    w_electrify,

    w_mg_turret,
    w_blob_turret,
    w_mine_explode,
    w_mine_destroy, //engi destroyed it

    w_breeder_bite,
    w_breeder_touch,
    w_hatchie_slash,
    w_hatchie_touch,
    w_drone_slash,
    w_drone_spit,
    w_drone_touch,
    w_wraith_slash,
    w_wraith_spit,
    w_wraith_touch,
    w_poison_spit,
    w_poison_time,
    w_kamikaze_explode,
    w_kamikaze_die, //smaller explosion when dies
    w_stinger_flame,
    w_stinger_slash,
    w_stinger_gas,
    w_stinger_touch,
    w_guard_slash,
    w_guard_spike,
    w_guard_spore,
    w_guard_infest,
    w_guard_revenge,
    w_guard_touch,
    w_stalker_slash,
    w_stalker_spike,
    w_stalker_touch,
    w_stalker_spore,
    w_spore_spike,

    w_egg_touch,
    w_healer_touch,
    w_obstacle_touch,
    w_spiker_touch,
    w_spiker_spike,
    w_gasser_touch,
    w_gasser_gas,

    w_mown_alien, //velocity is lost after pmove, can only use mass based. This is when attacker is alien
    w_mown_human, //Same when attacker is human (so score/frags/tk can be set for humans and aliens differently)
    w_squish_alien, //Old squish. Need special handling for squish since need big damage This is when attacker is alien
    w_squish_human, //Old squish. Same when attacker is human

    w_ent_infest, //misc_infestation spawned in map

    //Override based per class squish
    w_squish_grunt,
    w_squish_hatch,
    w_squish_ht,
    w_squish_commando,
    w_squish_drone,
    w_squish_mech,
    w_squish_st,
    w_squish_stalker,
    w_squish_breeder,
    w_squish_engi,
    w_squish_guard,
    w_squish_kamikaze,
    w_squish_exterminator,
    w_squish_stinger,
    w_squish_wraith,
    w_squish_biotech,

    w_new_mow,


    w_count
} weapon_t;

typedef enum {
    v_unknown,

    v_engi,
    v_grunt,
    v_st,
    v_bio,
    v_ht,
    v_cmd,
    v_xt,
    v_mech,

    v_tele,
    v_mg_turret,
    v_blob_turret,
    v_detector,
    v_mine,
    v_depot,

    v_breeder,
    v_hatchie,
    v_drone,
    v_wraith,
    v_kami,
    v_stinger,
    v_guard,
    v_stalker,

    v_egg,
    v_spiker,
    v_gasser,
    v_obstacle,
    v_healer,

    v_spore, //alien spore
    v_infest,

    v_explosive, //grenades, rockets
    v_c4,
    v_flare,

    v_corpse,

    v_count
} victim_t;

const char* weaponpaths[w_count] = {
    /*w_unknown     */ "unknown/weapon",

    /*w_engi_poke   */ "poke",
    /*w_autogun     */ "machinegun/autogun",
    /*w_grunt_pistol*/ "pistol/grunt",
    /*w_shotgun     */ "shotgun/shell",
    /*w_st_pistol   */ "pistol/st",
    /*w_ex_shell    */ "shotgun/ex",
    /*w_st_grenade  */ "grenade/smoke",
    /*w_scattergun  */ "shotgun/scatter",
    /*w_bio_pistol  */ "pistol/bio",
    /*w_bio_grenade */ "grenade/flash",
    /*w_ht_rocket   */ "rocket/ht",
    /*w_ht_pistol   */ "pistol/ht",
    /*w_ht_shrapnel */ "shrapnel/ht",
    /*w_smg         */ "machinegun/smg",
    /*w_magnum      */ "magnum",
    /*w_cmd_grenade */ "grenade/cmd",
    /*w_cmd_shrapnel*/ "shrapnel/cmd",
    /*w_c4          */ "c4/cmd",
    /*w_c4_failed   */ "c4/priming",
    /*w_laser       */ "laser",
    /*w_pulse       */ "pulse",
    /*w_xt_grenade  */ "grenade/xt",
    /*w_xt_shrapnel */ "shrapnel/xt",
    /*w_mech_cannon */ "cannon",
    /*w_mech_bay_roc*/ "rocket/mech",
    /*w_mech_grim_ro*/ "rocket/grimmech",
    /*w_mech_drunk_r*/ "rocket/drunk",
    /*w_electrify   */ "electrify",

    /*w_mg_turret   */ "turret/mg",
    /*w_blob_turret */ "turret/blob",
    /*w_mine_explode*/ "mine/explode",
    /*w_mine_destory*/ "mine/destroy",

    /*w_breeder_bite */ "bite/breeder",
    /*w_breeder_touch*/ "ptouch/breeder",
    /*w_hatchie_slash*/ "slash/hatchie",
    /*w_hatchie_touch*/ "ptouch/hatchie",
    /*w_drone_slash  */ "slash/drone",
    /*w_drone_spit   */ "spit/drone",
    /*w_drone_touch  */ "ptouch/drone",
    /*w_wraith_slash */ "slash/wraith",
    /*w_wraith_spit  */ "spit/wraith",
    /*w_wraith_touch */ "ptouch/wraith",
    /*w_poison_spit  */ "spit/poison",
    /*w_poison_time  */ "spit/poisoned",
    /*w_kamikaze_expl*/ "explode/kamikaze",
    /*w_kamikaze_die */ "die/kamikaze",
    /*w_stinger_flame*/ "fire/stinger",
    /*w_stinger_slash*/ "slash/stinger",
    /*w_stinger_gas  */ "gas/stinger",
    /*w_stinger_touch*/ "ptouch/stinger",
    /*w_guard_slash  */ "slash/guardian",
    /*w_guard_spike  */ "spike/guardian",
    /*w_guard_spore  */ "spore/guardian",
    /*w_guard_infest */ "infest/guardian",
    /*w_guard_revenge*/ "c4/revenge",
    /*w_guard_touch  */ "ptouch/guardian",
    /*w_stalker_slash*/ "slash/stalker",
    /*w_stalker_spike*/ "spike/stalker",
    /*w_stalker_touch*/ "ptouch/stalker",
    /*w_stalker_spore*/ "spore/stalker",
    /*w_spore_spike  */ "spike/spore",

    /*w_egg_touch    */ "touch/egg",
    /*w_healer_touch */ "touch/healer",
    /*w_obstacle_touc*/ "touch/obstacle",
    /*w_spiker_touch */ "touch/spiker",
    /*w_spiker_spike */ "spike/spiker",
    /*w_gasser_touch */ "touch/gasser",
    /*w_gasser_gas   */ "gas/gasser",

    /*w_mown_alien  */ "mown/alien",
    /*w_mown_human  */ "mown/human",
    /*w_squish_alien*/ "squish/alien",
    /*w_squish_human*/ "squish/human",

    /*w_ent_infest   */ "infest/world",

    /*w_squish_grunt	*/ "squish/grunt",
    /*w_squish_hatch	*/ "squish/hatchling",
    /*w_squish_ht		*/ "squish/ht",
    /*w_squish_commando	*/ "squish/cmd",
    /*w_squish_drone	*/ "squish/drone",
    /*w_squish_mech	*/ "squish/mech",
    /*w_squish_st		*/ "squish/st",
    /*w_squish_stalker	*/ "squish/stalker",
    /*w_squish_breeder	*/ "squish/breeder",
    /*w_squish_engi	*/ "squish/engineer",
    /*w_squish_guard	*/ "squish/guardian",
    /*w_squish_kamikaze	*/ "squish/kamikaze",
    /*w_squish_exterminato*/ "squish/exterm",
    /*w_squish_stinger	*/ "squish/stinger",
    /*w_squish_wraith	*/ "squish/wraith",
    /*w_squish_biotech	*/ "squish/biotech",

    /*w_new_mow      */ "new/mow",

};

const char* victimpaths[v_count] = {
    /*v_unknown*/ "unknown/victim",

    /*v_engi   */ "human/player/engi",
    /*v_grunt  */ "human/player/grunt",
    /*v_st     */ "human/player/st",
    /*v_bio    */ "human/player/bio",
    /*v_ht     */ "human/player/ht",
    /*v_cmd    */ "human/player/cmd",
    /*v_xt     */ "human/player/xt",
    /*v_mech   */ "human/player/mech",

    /*v_tele        */ "human/structure/tele",
    /*v_mg_turret   */ "human/structure/turret/mg",
    /*v_blob_turret */ "human/structure/turret/blob",
    /*v_detector    */ "human/structure/detector",
    /*v_mine        */ "human/structure/mine",
    /*v_depot       */ "human/structure/depot",

    /*v_breeder  */ "alien/player/breeder",
    /*v_hatchie  */ "alien/player/hatchie",
    /*v_drone    */ "alien/player/drone",
    /*v_wraith   */ "alien/player/wraith",
    /*v_kami     */ "alien/player/kamikaze",
    /*v_stinger  */ "alien/player/stinger",
    /*v_guard    */ "alien/player/guardian",
    /*v_stalker  */ "alien/player/stalker",

    /*v_egg      */ "alien/structure/egg",
    /*v_spiker   */ "alien/structure/spiker",
    /*v_gasser   */ "alien/structure/gasser",
    /*v_obstacle */ "alien/structure/obstacle",
    /*v_healer   */ "alien/structure/healer",

    /*v_spore    */ "spore",
    /*v_infest   */ "infestation",

    /*v_explosive*/ "explosive/other", //nades, rockets, etc
    /*v_c4       */ "explosive/c4",
    /*v_flare    */ "flare",

    /*v_corpse   */ "corpse",
};

static weapon_t identifyweapon(edict_t*inflictor, edict_t*attacker, int mod) {
    int t = (inflictor ? inflictor : attacker)->enttype;

    if (inflictor && t == ENT_FAKE) return inflictor->count; //a fake explosion entity, original weapon has been stored in count

    if (mod == MOD_EAT_C4) {
        if (attacker->client && attacker->client->resp.class_type == CLASS_GUARDIAN) return w_guard_revenge;
        return w_c4;
    }


    if (mod == MOD_NEW_SQUISH && attacker->client) return w_squish_grunt + attacker->client->resp.class_type;
    if (mod == MOD_NEW_MOW) return w_new_mow;


    if (t == ENT_PROXYSPIKE) {
        if (inflictor->target_ent == inflictor) return w_guard_spore;
        return w_guard_spike;
    }

    if (attacker->client
#ifdef ENT_PRACTICE_TARGET
            || attacker->enttype == ENT_PRACTICE_TARGET //gloom 1.4t0 only
#endif
            ) {
        int c = attacker->client ? attacker->client->resp.class_type : attacker->radius_dmg;
        if (!inflictor || attacker == inflictor) {
            int w = attacker->client ? ITEM_INDEX(attacker->client->weapon) : -1;

            if (c == CLASS_MECH) return w_electrify;
            if (c == CLASS_ENGINEER) return w_engi_poke;

            if (mod == MOD_MELEE) {
                if (c == CLASS_BREEDER) return w_breeder_bite;
                if (c == CLASS_HATCHLING) return w_hatchie_slash;
                if (c == CLASS_DRONE) return w_drone_slash;
                if (c == CLASS_WRAITH) return w_wraith_slash;
                if (c == CLASS_KAMIKAZE) return w_hatchie_slash;
                if (c == CLASS_STINGER) return w_stinger_slash;
                if (c == CLASS_GUARDIAN) return w_guard_slash;
                if (c == CLASS_STALKER) return w_stalker_slash;
                return w_unknown;
            }
            if (mod == MOD_JUMPATTACK) {
                if (c == CLASS_BREEDER) return w_breeder_touch;
                if (c == CLASS_HATCHLING) return w_hatchie_touch;
                if (c == CLASS_DRONE) return w_drone_touch;
                if (c == CLASS_WRAITH) return w_wraith_touch;
                if (c == CLASS_KAMIKAZE) return w_hatchie_touch;
                if (c == CLASS_STINGER) return w_stinger_touch;
                if (c == CLASS_GUARDIAN) return w_guard_touch;
                if (c == CLASS_STALKER) return w_stalker_touch;
                if (c == CLASS_MECH) return w_electrify;
                return w_unknown;
            }
            if (mod == MOD_KAMIKAZE) return w_kamikaze_explode;
            if (mod == MOD_R_SPLASH) return w_kamikaze_die;
            if (mod == MOD_R_SPLASH && c == CLASS_COMMANDO) return w_c4_failed;
            if (mod == MOD_RUN_OVER) {
                if (attacker->client->resp.team == TEAM_ALIEN) return w_mown_alien;
                return w_mown_human;
            }
            if (mod == MOD_OW_MY_HEAD_HURT || mod == MOD_MECHSQUISH || mod == MOD_MECHSQUISH_SQUISH) {
                if (!attacker->client || attacker->client->resp.team == TEAM_ALIEN) return w_squish_alien;
                return w_squish_human;
            }
            if (mod == MOD_POISON) return w_poison_time;
            if (w == ITEM_WEAPON_PISTOL) {
                if (c == CLASS_GRUNT) return w_grunt_pistol;
                if (c == CLASS_SHOCK) return w_st_pistol;
                if (c == CLASS_BIO) return w_bio_pistol;
                return w_unknown;
            }
            if (mod == MOD_EXSHELL) return w_ex_shell;
            if (w == ITEM_WEAPON_MAGNUM) return w_magnum;
            if (w == ITEM_WEAPON_SHOTGUN) return w_shotgun;
            if (w == ITEM_WEAPON_SCATTERGUN) return w_scattergun;
            if (w == ITEM_WEAPON_AUTOGUN) return w_autogun;
            if (w == ITEM_WEAPON_SUBGUN) return w_smg;
            if (mod == MOD_RAIL) return w_pulse;
            if (w == ITEM_WEAPON_PULSE_LASER) return w_laser;
            return w_unknown;
        } //melee

        if (mod == MOD_R_SPLASH && c == CLASS_SHOCK) return w_ex_shell;

        if (mod == MOD_SHRAPNEL) {
            if (c == CLASS_HEAVY) return w_ht_shrapnel;
            if (c == CLASS_COMMANDO) return w_cmd_shrapnel;
            if (c == CLASS_EXTERM) return w_xt_shrapnel;
            return w_unknown;
        }

        if (mod == MOD_HYPERBLASTER) return w_mech_cannon;
        if (mod == MOD_MECH_MISSILE) return w_mech_bay_rocket;
        if (mod == MOD_DRUNK_MISSILE) return w_mech_drunk_rocket;
        if (mod == MOD_GRIM_ROCKET) return w_mech_grim_rocket;

        if (t == ENT_DRONEWEB) return w_drone_spit;
        if (t == ENT_WRAITHBOMB) {
            if (attacker->client && attacker->client->resp.upgrades & UPGRADE_VENOM_GLAND)
                return w_poison_spit;
            return w_wraith_spit;
        }
#ifdef ENT_GRENADE_NEW
        if (t == ENT_GRENADE_NEW) t = ENT_GRENADE; //only in 1.4t0
#endif
        if (t == ENT_GRENADE) {
            if (attacker->air_finished == CLASS_EXTERM) return w_xt_grenade;
            return w_cmd_grenade;
        }
        if (t == ENT_FLASH_GRENADE) return w_bio_grenade;
        if (t == ENT_INFEST) return w_guard_infest;
        if (t == ENT_SPIKE) {
            if (!attacker->client || attacker->client->resp.class_type != CLASS_STALKER) return w_spore_spike;
            return w_stalker_spike;
        }

        if (t == ENT_ROCKET) return w_ht_rocket;
        if (t == ENT_C4) return w_c4;
        if (t == ENT_GAS_SPORE) return w_stinger_gas;
        if (t == ENT_SPIKE_SPORE) return w_stalker_spore;
        if (t == ENT_PROXY_SPORE) return w_guard_spore;
        if (t == ENT_FLAMES) return w_stinger_flame;
    }

    if (t == ENT_COCOON_U || t == ENT_COCOON_D || t == ENT_COCOON) return w_egg_touch;
    if (t == ENT_SPIKER) return w_spiker_touch;
    if (t == ENT_OBSTACLE) return w_obstacle_touch;
    if (t == ENT_GASSER) return w_gasser_touch;
    if (t == ENT_HEALER) return w_healer_touch;
    if (t == ENT_SPIKE) return w_spiker_spike;
    if (t == ENT_TURRET_BLOB || t == ENT_TURRET) return w_blob_turret;
    if (t == ENT_MGTURRET) return w_mg_turret;
    if (t == ENT_TRIPWIRE_BOMB) {
        if (inflictor->spawnflags & 1048576)
            return w_mine_destroy; //magic number identification as set in lmine.c
        return w_mine_explode;
    }
    if (t == ENT_INFEST) return w_ent_infest;

    if (t == ENT_GASSER_GAS) return w_gasser_gas;

    return w_unknown;
}

static victim_t identifyvictim(edict_t*ent) {
    if (ent->client) {
        int c = ent->client->resp.class_type;
        if (c == CLASS_ENGINEER) return v_engi;
        if (c == CLASS_GRUNT) return v_grunt;
        if (c == CLASS_SHOCK) return v_st;
        if (c == CLASS_BIO) return v_bio;
        if (c == CLASS_HEAVY) return v_ht;
        if (c == CLASS_COMMANDO) return v_cmd;
        if (c == CLASS_EXTERM) return v_xt;
        if (c == CLASS_MECH) return v_mech;
        if (c == CLASS_BREEDER) return v_breeder;
        if (c == CLASS_HATCHLING) return v_hatchie;
        if (c == CLASS_DRONE) return v_drone;
        if (c == CLASS_WRAITH) return v_wraith;
        if (c == CLASS_KAMIKAZE) return v_kami;
        if (c == CLASS_STINGER) return v_stinger;
        if (c == CLASS_GUARDIAN) return v_guard;
        if (c == CLASS_STALKER) return v_stalker;

        return v_unknown;
    }
    int t = ent->enttype;

    if (t == ENT_COCOON_U || t == ENT_COCOON_D || t == ENT_COCOON) return v_egg;
    if (t == ENT_SPIKER) return v_spiker;
    if (t == ENT_OBSTACLE) return v_obstacle;
    if (t == ENT_GASSER) return v_gasser;
    if (t == ENT_HEALER) return v_healer;

    if (t == ENT_TURRETBASE) {
        ent = ent->target_ent;
        t = ent->enttype;
    }

    if (t == ENT_TURRET) return v_blob_turret;
    if (t == ENT_MGTURRET) return v_mg_turret;
    if (t == ENT_DETECTOR) return v_detector;
    if (t == ENT_TRIPWIRE_BOMB) return v_mine;
    if (t == ENT_AMMO_DEPOT) return v_depot;
    if (t == ENT_TELEPORTER_U || t == ENT_TELEPORTER_D || t == ENT_TELEPORTER) return v_tele;
    if (t == ENT_ROCKET || t == ENT_GRENADE) return v_explosive;
    if (t == ENT_C4) return v_c4;
    if (t == ENT_FLARE) return v_flare;
    if (t == ENT_CORPSE || t == ENT_HUMAN_BODY || t == ENT_FEMALE_BODY) return v_corpse;
    if (t == ENT_GAS_SPORE || t == ENT_SPIKE_SPORE || t == ENT_PROXY_SPORE) return v_spore;
    if (t == ENT_INFEST) return v_infest;
    return v_unknown;
}

typedef struct {
    int16_t hitpoints;
    uint16_t knockback;
    uint16_t flags; //do_
    int8_t frags; //frags given (or taken) if victim dies
    int8_t score; //score given (or taken) if victim dies
    int8_t tkcounter;
    uint8_t uflags;
    float fragsper10tkpoint; //extra frags to subtract for each 10 tk counter point (before adding new tk counter)
    float ascale;
    uint8_t location_head;
    uint8_t location_torso;
    uint8_t location_legs;
    uint8_t location_step;

    rewardinfo_t rewards;
} tableent_t;

static tableent_t table[w_count][v_count];

static uint16_t radiustable[w_count];

cvaroverride_t cvaroverride;

typedef struct {
    uint8_t weapons[w_count];
    uint8_t victims[v_count];
    uint8_t bit;
    uint8_t mask;
    uint8_t vor, wor;
} parsestate_t;

typedef struct {
    char*name;
    char*shortname;
    uint16_t value;
    uint16_t conflict;
    char*veryshortname;
} namedbit_t;

typedef struct {
    char ops[8];
    uint8_t count;
    uint8_t optional;
    uint8_t isset;
    float num[8];
    float min, max;
    char** constants;
} numop_t;

typedef struct {
    namedbit_t*bits;
    int addmask;
    int rmvmask;
    uint8_t unset;
} flagop_t;

static namedbit_t flagnames[] = {
    {"linearexplosion", "linear", do_linear_explosion, do_original_explosion, "lexp"},
    {"originalexplosion", "oldexplosion", do_original_explosion, do_linear_explosion, "oexp"},
    {"forceexplosion", "explosion", do_force_explosion, 0, NULL},
    {"eggkiller", "killegg", do_kill_egg, 0, "keg"},
    {"piercing", "pierce", do_piercing, 0, "ap"},
    {"noteamkill", "notk", do_no_tk_players | do_no_tk_structures, do_tk_players | do_tk_structures, NULL},
    {"teamkill", "tk", do_tk_players | do_tk_structures, do_no_tk_players | do_no_tk_structures, NULL},
    {"notkplayers", "noplayertk", do_no_tk_players, do_tk_players, "notkp"},
    {"notkstructures", "nostructtk", do_no_tk_structures, do_tk_structures, "notkst"},
    {"tkplayers", "playertk", do_tk_players, do_no_tk_players, "ptk"},
    {"tkstructures", "structtk", do_tk_structures, do_no_tk_structures, "stk"},
    {"massscale", "mass", do_mass_scale, 0, "mass"},
    {"volumescale", "volume", do_volume_scale, 0, "vol"},
    {"nohealthdmg", "keephealth", do_no_health_dmg, 0, "nohp"},
    {"noarmourdmg", "keeparmour", do_no_armour_dmg, 0, "noa"},
    {"ignorearmour", "noarmour", do_ignore_armour, 0, "igna"},
    {"revenge", "c4revenge", do_revenge, 0, "rev"},
    {"noselfdmg", "noselfdamage", do_no_self_dmg, 0, "noself"},
    {NULL}
};


static char description[32];
static uint8_t desc_set = false;

static char* flnames(int bits, namedbit_t*names) {
    static char buf[1024];
    char*w = buf;
    while (names->name) {
        if ((bits & names->value) == names->value && names->veryshortname)
            w += sprintf(w, "%s,", names->veryshortname);
        names++;
    }
    if (w == buf) return "none";
    w--;
    *w = 0;
    return buf;
}

static float ApplyNumOp(float n, numop_t*op) {
    int i;
    for (i = 0; i < op->count; i++) {
        char o = op->ops[i];
        float num = op->num[i];
        if (o == '=') n = num;
        else
            if (o == '+') n += num;
        else
            if (o == '-') n -= num;
        else
            if (o == '*') n *= num;
        else
            if (o == '%') n *= num / 100.0f;
        else
            if (o == '/') {
            if (n < -0.00001 || n > 0.00001)
                n /= num;
        }
    }
    if (n > op->max) n = op->max;
    else
        if (n < op->min) n = op->min;
    return n;
}

static int ApplyFlagOp(int m, flagop_t*op) {
    return m & ~op->rmvmask | op->addmask;
}


static DEBUGITEM(override);
static DEBUGSUBITEM(override_unknown, override);
static DEBUGSUBITEM(override_data, override);

int OverrideDamage(dmgoverride_t*dmg, edict_t*victim, edict_t*inflictor, edict_t*attacker, int dflags, int mod) {
    weapon_t w = identifyweapon(inflictor, attacker, mod);
    victim_t v = identifyvictim(victim);
    tableent_t*t = table[w] + v;

    if (v == v_unknown && Debug(override_unknown))
        gi.dprintf("Unknown victim found: name=%s type=%d num=%d\n", victim->classname, victim->enttype, victim - g_edicts);
    if (w == w_unknown && Debug(override_unknown))
        gi.dprintf("Unknown weapon found: att=%s(type=%d)%d  inf=%s(type=%d)%d  class=%s\n",
            attacker->classname, attacker->enttype, attacker - g_edicts,
            inflictor->classname, inflictor->enttype, inflictor - g_edicts,
            attacker->client ? classlist[attacker->client->resp.class_type].classname : "no"
            );

    dmg->weaponindex = w;
    dmg->hitpoints = t->hitpoints;
    dmg->knockback = t->knockback;
    if (w == w_pulse) { //need some magic from weapon_pulselaser_fire_discharge to scale by cell count
        float scale = attacker->newcount / 60.0f;
        dmg->hitpoints *= scale;
        dmg->knockback *= scale;
    }
    dmg->flags = t->flags;
    dmg->radius = radiustable[w];
    dmg->frags = t->frags;
    dmg->score = t->score;
    dmg->tkcounter = t->tkcounter;
    dmg->uflags = t->uflags;
    dmg->fragsper10tkpoint = t->fragsper10tkpoint;
    dmg->ascale = t->ascale;
    dmg->lshead = t->location_head * 0.01f;
    dmg->lstorso = t->location_torso * 0.01f;
    dmg->lslegs = t->location_legs * 0.01f;
    dmg->lsstep = t->location_step * 0.01f;

    if (Debug(override_data))
        gi.dprintf("Override: %s vs %s -> dmg=%d kick=%d flags=%s;%c%c%c\n",
            weaponpaths[w], victimpaths[v], dmg->hitpoints, dmg->knockback,
            flnames(dmg->flags, flagnames),
            (dmg->uflags & dou_damage_set) ? 'D' : '-',
            (dmg->uflags & dou_knockback_set) ? 'K' : '-',
            (dmg->uflags & dou_other_set) ? 'O' : '-'
            );

    return dmg->uflags & (dou_knockback_set | dou_damage_set) || dmg->radius > 0 || dmg->flags;
}

int GetExplosionInfo(explosioninfo_t*ei, edict_t*inflictor, edict_t*attacker, int mod) {
    weapon_t w = identifyweapon(inflictor, attacker, mod);
    if (radiustable[w] <= 0) return 0;
    if (!ei) return 1; //just testing if there is data when ei is NULL
    ei->radius = radiustable[w];
    if (Debug(override_data))
        gi.dprintf("Override: blast of %s -> %d\n", weaponpaths[w], (int) ei->radius);
    return 1;
}

float OverrideSlashDistance(edict_t*ent, float original_range, int mod) {
    weapon_t w = identifyweapon(ent, ent, mod);
    if (radiustable[w] <= 0) return original_range;
    if (Debug(override_data))
        gi.dprintf("Override: slash of %s -> radius=%.1f old=%.1f\n", weaponpaths[w], 0.0f + radiustable[w], 0.0f + original_range);
    return radiustable[w];
}

#define PRINTF(...) {if (ent) gi.cprintf(ent, PRINT_HIGH, "DamageOverride: " __VA_ARGS__); else gi.dprintf("DamageOverride: " __VA_ARGS__);}

static void SkipToEnd(char**buf) {
    while (**buf && **buf != 10 && **buf != ';') (*buf)++;
}

static char*ReadLine(edict_t*ent, char**buf) {
    static char str[128];
    int len = 0;
    while (**buf && **buf <= 32 && **buf != 10) (*buf)++;
    while (**buf && **buf != 10 && **buf != ';') {
        if (len < 127)
            str[len++] = **buf;
        (*buf)++;
    }
    while (len > 0 && str[len - 1] <= 32) len--;
    str[len] = 0;
    return str;
}

int ReadNumOp(edict_t*ent, char**buf, numop_t*op) {
    qboolean ismod = false;
    op->count = 0;
    op->isset = false;

    while (**buf && **buf <= 32 && **buf != 10) (*buf)++;

    if (!**buf || **buf == 10) {
        if (!op->optional)
            PRINTF("End of line found where numeric value was expected\n");
        return 0;
    }

    if (!strncasecmp(*buf, "mod", 3)) {
        *buf += 3;
        if (**buf > 32 || **buf == 10 || !**buf)
            PRINTF("Modifiers expected after 'mod' word\n");
        while (**buf && **buf <= 32 && **buf != 10) (*buf)++;
        ismod = true;
    } else
        if (!strncasecmp(*buf, "unset", 3)) {
        *buf += 5;
        while (**buf && **buf <= 32 && **buf != 10) (*buf)++;
        if (**buf != 10 && !**buf)
            PRINTF("End of line expected after 'unset' word\n");
        while (**buf && **buf <= 32 && **buf != 10) (*buf)++;
        op->isset = false;
        return 1;
    }

    while (1) {
        float dummy;
        float *f = &dummy;

        while (**buf <= 32 || **buf == ';') {
            if (!**buf || **buf == 10 || **buf == ';') {
                if (op->count > LENGTH(op->ops))
                    op->count = LENGTH(op->ops);
                return 1;
            }
            (*buf)++;
        }

        if (!ismod) {
            op->ops[0] = '=';
            op->count = 1;
            f = op->num;
            ismod = true;
        } else
            if (**buf == '*' || **buf == '/' || **buf == '+' || **buf == '/' || **buf == '%') {
            if (op->count < LENGTH(op->ops)) {
                f = op->num + op->count;
                op->ops[op->count++] = **buf;
            } else
                if (op->count == LENGTH(op->ops))
                PRINTF("Maximum of %d operations reached, ignoring excess ops\n", op->count);
            (*buf)++;
        } else {
            PRINTF("Unknown operator '%c'\n", **buf);
            SkipToEnd(buf);
            return 0;
        }
        while (**buf && **buf <= 32 && **buf != 10) (*buf)++;

        op->isset = true;

        if (**buf >= '0' && **buf <= '9' || **buf == '+' || **buf == '-')
            *f = strtof(*buf, buf); //FIXME: \n after operator will break line counts when crossing line boundary, which shouldn't be allowed on current syntax
        else {
            PRINTF("Numeric constant expected, found '%c'\n", **buf);
            SkipToEnd(buf);
            return 0;
        }
    }
}

int ReadFlagsOp(edict_t*ent, char**buf, flagop_t*op) {
    int retval = 1;
    int setmask = 0;
    uint8_t clear = false;
    op->addmask = 0;
    op->rmvmask = 0;

    while (**buf && **buf <= 32 && **buf != 10) (*buf)++;
    if (op->unset && !strncasecmp(*buf, "unset", 3)) {
        *buf += 5;
        while (**buf && **buf <= 32 && **buf != 10) (*buf)++;
        if (**buf != 10 && !**buf)
            PRINTF("End of line expected after 'unset' word\n");
        while (**buf && **buf <= 32 && **buf != 10) (*buf)++;
        op->rmvmask = -1;
        return 1;
    }

    while (**buf && **buf != 10 && **buf != ';') {
        while (**buf && **buf <= 32 && **buf != 10) (*buf)++;

        char ot = 0;
        if (**buf == '+' || **buf == '-')
            ot = *((*buf)++);

        char name[32] = {0};
        int len = 0;
        while ((**buf | 32) >= 'a' && (**buf | 32) <= 'z') {
            if (len < 31) name[len] = **buf | 32;
            len++;
            (*buf)++;
        }
        if (!len || **buf > 32 && **buf != ';') {
            PRINTF("Invalid character in flag description: '%c'\n", **buf);
            SkipToEnd(buf);
            return 0;
        }

        int bit = 0;
        int badbits = 0;
        if (strcmp(name, "none")) {
            namedbit_t*list = op->bits;
            while (list->name) {
                if (!strcmp(list->name, name) || !strcmp(list->shortname, name)) {
                    bit = list->value;
                    badbits = list->conflict;
                    break;
                }
                list++;
            }
            if (!list->name) {
                PRINTF("Unknown flag '%s'\n", name);
                retval = 0;
            }
        } else
            if (ot) {
            PRINTF("%sing flag 'none' has no effect\n", (ot == '+') ? "Add" : "Remov");
            continue;
        }

        op->unset = false;

        if (!ot) {
            clear = true;
            setmask = setmask &~badbits | bit;
        } else
            if (ot == '+') {
            op->addmask = op->addmask &~badbits | bit;
        } else {
            op->rmvmask = op->rmvmask &~badbits | bit;
            op->addmask &= ~bit;
        }
    }

    if (retval && !op->addmask && !op->rmvmask && !clear) {
        PRINTF("End of line found where flags where expected\n");
        return 0;
    }

    if (clear) {
        setmask |= op->addmask;
        op->addmask = setmask;
        op->rmvmask = ~setmask;
    }

    return retval;
}

#ifdef SPAMMY_DEBUG
#define SPRINTF(...) printf(__VA_ARGS__)
#define SPAMMY(...) __VA_ARGS__
#else
#define SPRINTF(...) 
#define SPAMMY(...)
#endif

static void debugps(parsestate_t*ps) {
#ifdef SPAMMY_DEBUG
    int i, v, w, bv, bw;
    for (w = bw = 0, i = 0; i < w_count; i++) {
        if (ps->weapons[i] & ps->mask) w++;
        if (ps->weapons[i] & (ps->mask - 1)) bw++;
    }
    for (v = bv = 0, i = 0; i < v_count; i++) {
        if (ps->weapons[i] & ps->mask) v++;
        if (ps->weapons[i] & (ps->mask - 1)) bv++;
    }
    printf("parsestate[%d]: %d(%d) weaps (wor %02x), %d(%d) victims (vor %02x). Mask=%.2x\n", ps->bit, w, bw, ps->wor, v, bv, ps->vor, ps->mask);
#endif
}


#define APPLY(code) {  debugps(&ps); \
  int v, w; \
  SPAMMY(int vc = 0, wc = 0); \
  for (w = 0; w < w_count; w++) if (!ps.wor || ps.weapons[w] & ps.mask) { \
    tableent_t*item = table[w]; \
    SPAMMY(wc++); \
    for (v = 0; v < v_count; v++, item++) \
      if (!ps.vor || ps.victims[v] & ps.mask) { \
        SPAMMY(vc++); \
        code; \
      } \
  } \
  SPRINTF("APPLY: Matched %d weapons, %d victims\n", wc, vc); \
}

#define APPLYW(code) {\
  int w; \
  SPAMMY(int wc = 0); \
  for (w = 0; w < w_count; w++) if (!ps.wor || ps.weapons[w] & ps.mask) { \
    SPAMMY(wc++); \
    code; \
  } \
  SPRINTF("APPLY: Matched %d weapons\n", wc); \
}

static void SetDescription(char*s) {
    strncpy(description, s, sizeof (description) - 1);
    description[sizeof (description) / sizeof (description[0]) - 1] = 0;
    for (s = description; *s; s++) {
        *s &= 127;
        if (*s < 32 || *s == 127) *s = 32;
        if (*s == '$' || *s == '"') *s = '#';
        if (*s == ';') *s = ',';
    }

    gi.cvar("damages", description, CVAR_SERVERINFO /*| CVAR_NOSET*/); //stupid q2 doesnt allow game modify own vars!
    gi.cvar_set("damages", description);
    desc_set = true;
}

static void CheckComment(char**buf) {
    while (**buf && **buf <= 32 && **buf != 10) (*buf)++;
    if (**buf == '#') {
#ifdef SPAMMY_DEBUG
        char part[128] = {0};
        strncpy(part, *buf, 127);
        char*z = strchr(part, 10);
        if (z) *z = 0;
        printf("Comment: %s\n", part);
#endif
        *buf = strchr(*buf, 10);
        if (!*buf) *buf = "";
    }
}

static void ClearBit(parsestate_t*ps, uint8_t zero) {
    int notbit = ~ps->mask;
    int basebit = ps->bit ? 1 << (ps->bit - 1) : 0;
    int i;
    ps->wor = ps->vor = 0;
    if (!zero) {
        for (i = 0; i < w_count; i++) {
            ps->weapons[i] = (ps->weapons[i] & notbit) | ((ps->weapons[i] & basebit) << 1);
            ps->wor |= ps->weapons[i];
        }
        for (i = 0; i < v_count; i++) {
            ps->victims[i] = (ps->victims[i] & notbit) | ((ps->victims[i] & basebit) << 1);
            ps->vor |= ps->victims[i];
        }
    } else {
        for (i = 0; i < w_count; i++) {
            ps->weapons[i] = ps->weapons[i] & notbit;
            ps->wor |= ps->weapons[i];
        }
        for (i = 0; i < v_count; i++) {
            ps->victims[i] = ps->victims[i] & notbit;
            ps->vor |= ps->victims[i];
        }
    }
    SPRINTF("cleared: ");
    debugps(ps);
}

static void ParseOverride(edict_t*ent, char*buf) {
    parsestate_t ps = {0};
    char name[128];
    int nlen = 0;
    int line = 0;
    CheckComment(&buf);
    ps.mask = 1;
    while (*buf) {
        if (*buf <= 32 || *buf == '{' || *buf == '}' || *buf == ';') {
            char end = 0;

            while (*buf && !end && (*buf <= 32 || *buf == '{' || *buf == '}')) {
                if (*buf == 10 || *buf == '{' || *buf == '}') end = *buf;
                if (*buf == ';') end = 10;
                buf++;
            }

            if (nlen) {
                name[nlen] = 0;
                nlen = 0;

                dmg_override = true;

                if (!strcmp(name, "damage")) {
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 9999;
                    if (ReadNumOp(ent, &buf, &op)) APPLY(
                            item->hitpoints = ApplyNumOp(item->hitpoints, &op);
                        if (op.isset)
                                item->uflags |= dou_damage_set;
                        else
                            item->uflags &= ~dou_damage_set;
                            if (op.isset) {
                            SPRINTF("DMG %s vs %s set to %d, line %d\n", weaponpaths[w], victimpaths[v], item->hitpoints, line);
                        } else {
                            SPRINTF("DMG %s vs %s reset, line %d\n", weaponpaths[w], victimpaths[v], line);
                        }
                    )
                } else
                    if (!strcmp(name, "heal")) {
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 9999;
                    if (ReadNumOp(ent, &buf, &op)) APPLY(
                            item->hitpoints = -ApplyNumOp(-item->hitpoints, &op);
                        if (op.isset)
                                item->uflags |= dou_damage_set;
                        else
                            item->uflags &= ~dou_damage_set;
                            if (op.isset) {
                            SPRINTF("HEAL %s vs %s set to %d, line %d\n", weaponpaths[w], victimpaths[v], item->hitpoints, line);
                        } else {
                            SPRINTF("HEAL %s vs %s reset, line %d\n", weaponpaths[w], victimpaths[v], line);
                        }
                    )
                } else
                    if (!strcmp(name, "knockback")) {
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 9999;
                    if (ReadNumOp(ent, &buf, &op)) APPLY(
                            item->knockback = ApplyNumOp(item->knockback, &op);
                        if (op.isset)
                                item->uflags |= dou_knockback_set;
                        else
                            item->uflags &= ~dou_knockback_set;
                                )
                        } else
                    if (!strcmp(name, "flags")) {
                    flagop_t op = {0};
                    op.bits = flagnames;
                    if (ReadFlagsOp(ent, &buf, &op)) APPLY(
                            item->flags = ApplyFlagOp(item->flags, &op);

                        if (!op.unset) {
                            SPRINTF("FLAGS %s vs %s set to %.4x(%s), line %d\n", weaponpaths[w], victimpaths[v], item->flags, flnames(item->flags, flagnames), line);
                        } else {
                            SPRINTF("FLAGS %s vs %s reset, line %d\n", weaponpaths[w], victimpaths[v], line);
                        }
                    );
                } else
                    if (!strcmp(name, "frags")) {
                    numop_t op = {0};
                    op.min = -100;
                    op.max = 100;
                    if (ReadNumOp(ent, &buf, &op)) {
                        score_override = true;
                        APPLY(item->frags = ApplyNumOp(item->frags, &op));
                    }
                } else
                    if (!strcmp(name, "score")) {
                    numop_t op = {0};
                    op.min = -125;
                    op.max = 125;
                    if (ReadNumOp(ent, &buf, &op)) {
                        score_override = true;
                        APPLY(item->score = ApplyNumOp(item->score, &op));
                    }
                } else
                    if (!strcmp(name, "tkcounter")) {
                    numop_t op = {0};
                    op.min = -125;
                    op.max = 125;
                    if (ReadNumOp(ent, &buf, &op)) {
                        score_override = true;
                        APPLY(item->tkcounter = ApplyNumOp(item->tkcounter, &op));
                    }
                } else
                    if (!strcmp(name, "tkextra")) {
                    numop_t op = {0};
                    op.min = -100;
                    op.max = +100;
                    if (ReadNumOp(ent, &buf, &op))
                        APPLY(item->fragsper10tkpoint = ApplyNumOp(item->fragsper10tkpoint, &op));
                } else
                    if (!strcmp(name, "ascale") || !strcmp(name, "armourscale")) {
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 10;
                    if (ReadNumOp(ent, &buf, &op))
                        APPLY(item->ascale = ApplyNumOp(item->ascale, &op));
                } else
                    if (!strcmp(name, "radius")) {
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 4096;
                    if (ReadNumOp(ent, &buf, &op))
                        APPLYW(radiustable[w] = ApplyNumOp(radiustable[w], &op));
                } else
                    if (!strcmp(name, "headscale")) {
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 2.55;
                    if (ReadNumOp(ent, &buf, &op))
                        APPLY(item->location_head = ApplyNumOp(item->location_head * 0.01f, &op)*100);
                } else
                    if (!strcmp(name, "torsoscale")) {
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 2.55;
                    if (ReadNumOp(ent, &buf, &op))
                        APPLY(item->location_torso = ApplyNumOp(item->location_torso * 0.01f, &op)*100);
                } else
                    if (!strcmp(name, "legsscale") || !strcmp(name, "legscale")) {
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 2.55;
                    if (ReadNumOp(ent, &buf, &op))
                        APPLY(item->location_legs = ApplyNumOp(item->location_legs * 0.01f, &op)*100);
                } else
                    if (!strcmp(name, "stepscale")) {
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 2.55;
                    if (ReadNumOp(ent, &buf, &op))
                        APPLY(item->location_step = ApplyNumOp(item->location_step * 0.01f, &op)*100);
                } else

                    //Fragmode FS_FRACTIONAL
                    if (!strcmp(name, "damagepoints")) {
                    numop_t op = {0};
                    op.min = -80;
                    op.max = +80;
                    if (ReadNumOp(ent, &buf, &op)) {
                        APPLY(item->rewards.instant_h_points = ApplyNumOp(item->rewards.instant_h_points, &op));
                        APPLY(item->rewards.instant_a_points = ApplyNumOp(item->rewards.instant_a_points, &op));
                    }
                } else
                    if (!strcmp(name, "hdamagepoints")) {
                    numop_t op = {0};
                    op.min = -80;
                    op.max = +80;
                    if (ReadNumOp(ent, &buf, &op))
                        APPLY(item->rewards.instant_h_points = ApplyNumOp(item->rewards.instant_h_points, &op));
                } else
                    if (!strcmp(name, "adamagepoints")) {
                    numop_t op = {0};
                    op.min = -80;
                    op.max = +80;
                    if (ReadNumOp(ent, &buf, &op))
                        APPLY(item->rewards.instant_a_points = ApplyNumOp(item->rewards.instant_a_points, &op));
                } else
                    if (!strcmp(name, "coopkillpoints")) {
                    numop_t op = {0};
                    op.min = -80;
                    op.max = +80;
                    if (ReadNumOp(ent, &buf, &op))
                        APPLY(item->rewards.coop_points = ApplyNumOp(item->rewards.coop_points, &op));
                } else
                    if (!strcmp(name, "lasthitpoints")) {
                    numop_t op = {0};
                    op.min = -80;
                    op.max = +80;
                    if (ReadNumOp(ent, &buf, &op))
                        APPLY(item->rewards.kill_points = ApplyNumOp(item->rewards.kill_points, &op));
                } else
                    if (!ps.bit && !strcmp(name, "fragmode")) {
                    static char* cs[] = {"unlimited", "classic", "fractional", "combined", NULL};
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 3;
                    op.constants = cs;
                    if (ReadNumOp(ent, &buf, &op)) {
                        if (op.isset)
                            cvaroverride.fragmode = ApplyNumOp(cvaroverride.fragmode, &op);
                        else
                            cvaroverride.fragmode = nocvaroverride_fragmode;
                    }
                } else

                    if (!ps.bit && !strcmp(name, "description")) {
                    char*s;
                    if ((s = ReadLine(ent, &buf)) && strlen(s))
                        SetDescription(s);
                } else
                    if (!ps.bit && !strcmp(name, "warning")) {
                    char*s;
                    if ((s = ReadLine(ent, &buf)) && strlen(s)) {
                        gi.bprintf(PRINT_HIGH, "%s\n%s\n", description, s);
                        gi.dprintf("damages> %s\ndamages> %s\n", description, s);
                    }
                } else
                    if (!ps.bit && !strcmp(name, "firemode")) {
                    static char* cs[] = {"original", "explosion", "distance", NULL};
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 2;
                    op.constants = cs;
                    if (ReadNumOp(ent, &buf, &op)) {
                        if (op.isset)
                            cvaroverride.fire_mode = ApplyNumOp(cvaroverride.fire_mode, &op);
                        else
                            cvaroverride.fire_mode = nocvaroverride_fire_mode;
                    }
                } else
                    if (!ps.bit && !strcmp(name, "autoslash")) {
                    static char* cs[] = {"default", NULL};
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 1;
                    op.constants = cs;
                    if (ReadNumOp(ent, &buf, &op)) {
                        if (op.isset)
                            cvaroverride.autoslash_delay = ApplyNumOp(cvaroverride.autoslash_delay, &op);
                        else
                            cvaroverride.autoslash_delay = nocvaroverride_autoslash_delay;
                    }
                } else
                    if (!ps.bit && !strcmp(name, "hatchlingflags")) {
                    static namedbit_t hatchflags[] = {
                        {"grapple", "1", 1, 0},
                        {"grapplejump", "2", 3, 0},
                        {"walljump", "4", 4, 0},
                        {NULL}
                    };
                    flagop_t op = {0};
                    op.bits = hatchflags;
                    op.unset = true;
                    if (ReadFlagsOp(ent, &buf, &op)) {
                        if (!op.unset)
                            cvaroverride.hatchling_flags = ApplyFlagOp(cvaroverride.hatchling_flags, &op);
                        else
                            cvaroverride.hatchling_flags = nocvaroverride_hatchling_flags;
                    }
                } else
                    if (!ps.bit && !strcmp(name, "spikerflags")) {
                    static namedbit_t spikerflags[] = {
                        {"spew", "1", 1, 0},
                        {"explode", "2", 3, 0},
                        {NULL}
                    };
                    flagop_t op = {0};
                    op.bits = spikerflags;
                    op.unset = true;
                    if (ReadFlagsOp(ent, &buf, &op)) {
                        if (!op.unset)
                            cvaroverride.spiker_flags = ApplyFlagOp(cvaroverride.spiker_flags, &op);
                        else
                            cvaroverride.spiker_flags = nocvaroverride_spiker_flags;
                    }
                } else
                    if (!ps.bit && !strcmp(name, "mechrockets")) {
                    static char* cs[] = {"none", "drunk", "grim", "bay", "bigbay", NULL};
                    numop_t op = {0};
                    op.min = 0;
                    op.max = 4;
                    op.constants = cs;
                    if (ReadNumOp(ent, &buf, &op)) {
                        if (op.isset) {
                            cvaroverride.mech_rockets = ApplyNumOp(cvaroverride.mech_rockets, &op);
                            op.optional = true;
                            op.max = 10;
                            if (ReadNumOp(ent, &buf, &op) & op.isset)
                                cvaroverride.mech_rockets_count = ApplyNumOp(cvaroverride.mech_rockets_count, &op);
                            else
                                cvaroverride.mech_rockets_count = 0;
                        } else
                            cvaroverride.mech_rockets = nocvaroverride_mech_rockets;
                    }
                } else {
                    int valid = 0;
                    int i;
                    int bit = ps.mask;
                    char add = true;
                    char next = true;
                    char*np = name;

                    if (*np == '-') {
                        np++;
                        add = false;
                    } else
                        if (*np == '+')
                        np++;

                    if (!strcmp(np, "weapons")) {
                        for (i = 0; i < w_count; i++) {
                            valid++;

                            if (add)
                                ps.weapons[i] |= bit;
                            else
                                ps.weapons[i] &= ~bit;
                        }
                        if (add)
                            ps.wor |= bit;
                        else
                            if (bit == 1)
                            ps.wor &= ~bit;
                    } else
                        if (!strcmp(np, "victims")) {
                        for (i = 0; i < v_count; i++) {
                            valid++;

                            if (add)
                                ps.victims[i] |= bit;
                            else
                                ps.victims[i] &= ~bit;
                        }
                        if (add)
                            ps.vor |= bit;
                        else
                            if (bit == 1)
                            ps.vor &= ~bit;
                    } else
                        for (i = 0; i < w_count; i++)
                            if (glob_match((char*) np, (char*) weaponpaths[i])) {
                                valid++;

                                if (add) {
                                    ps.weapons[i] |= bit;
                                    ps.wor |= bit;
                                } else {
                                    ps.weapons[i] &= ~bit;
                                    if (bit == 1)
                                        ps.wor &= ~bit;
                                }
                            }

                    if (!valid) for (i = 0; i < v_count; i++)
                            if (glob_match((char*) np, (char*) victimpaths[i])) {
                                valid++;

                                if (add) {
                                    ps.victims[i] |= bit;
                                    ps.vor |= bit;
                                } else {
                                    ps.victims[i] &= ~bit;
                                    if (bit == 1)
                                        ps.vor &= ~bit;
                                }
                            }
                    if (!valid)
                        PRINTF("Ignoring unknown name %s\n", name);
                    SPRINTF("%d matches for %s: ", valid, name);
                    debugps(&ps);
                }
            } //nlen > 0

            if (end == 10) {
                ClearBit(&ps, false);
                CheckComment(&buf);
                line++;
            } else
                if (end == '{') {
                int i;
                if (++ps.bit == 8)
                    PRINTF("Overflowed { line %d\n", line);
                for (i = 0; i < w_count; i++) ps.weapons[i] |= (ps.weapons[i] & ps.mask) << 1;
                for (i = 0; i < v_count; i++) ps.victims[i] |= (ps.victims[i] & ps.mask) << 1;
                ps.mask = 1 << ps.bit;
            } else
                if (end == '}') {
                if (ps.bit) {
                    ClearBit(&ps, true);
                    ps.bit--;
                    ps.mask = 1 << ps.bit;
                } else
                    PRINTF("Unmatched } line %d\n", line);
            }
        } else
            if (*buf >= 'a' && *buf <= 'z' || *buf >= '0' && *buf <= '9' || *buf == '/' || *buf == '?' || *buf == '*' || *buf == '[' || *buf == ']' || !nlen && (*buf == '+' || *buf == '-')) {
            if (nlen < sizeof (name)) name[nlen++] = *buf;
            buf++;
        } else
            if (*buf >= 'A' && *buf <= 'Z') {
            if (nlen < sizeof (name)) name[nlen++] = *buf | 32;
            buf++;
        } else {
            PRINTF("Invalid char '%c' in file, line %d\n", *buf, line);
            buf++;
        }
    }
}

void ModifyOverride(edict_t*ent, char*buf) {
    if (description[0]) {
        if (!strstr(description, "(modified)"))
            SetDescription(va("(modified) %s", description));
    } else
        SetDescription("(modified) (default)");

    ParseOverride(ent, buf);
}

void LoadDamageOverrideTable() {
    int v, w;

    memset(table, 0, sizeof (table));
    memset(radiustable, 0, sizeof (radiustable));
    for (w = 0; w < w_count; w++)
        for (v = 0; v < v_count; v++) {
            table[w][v].ascale = 1;
            table[w][v].location_head = 100;
            table[w][v].location_torso = 100;
            table[w][v].location_legs = 100;
            table[w][v].location_step = 100;
        }

    description[0] = 0;
    cvaroverride.fire_mode = nocvaroverride_fire_mode;
    cvaroverride.mech_rockets = nocvaroverride_mech_rockets;
    cvaroverride.mech_rockets_count = 0;
    cvaroverride.hatchling_flags = nocvaroverride_hatchling_flags;
    cvaroverride.autoslash_delay = nocvaroverride_autoslash_delay;
    cvaroverride.exterm_pulse_overheat = nocvaroverride_exterm_pulse_overheat;
    cvaroverride.inven_refill_flags = nocvaroverride_inven_refill_flags;
    cvaroverride.spiker_flags = nocvaroverride_spiker_flags;
    dmg_override = false;
    score_override = false;

    g_override_damages->modified = false;

    if (strlen(g_override_damages->string) && !strstr(g_override_damages->string, "../")) {
        FILE*f = fopen(va("%s/%s", gamedir->string, g_override_damages->string), "r");
        if (!f) {
            gi.dprintf("ERROR: Failed to open damage override file %s\n", g_override_damages->string);
            return;
        }

#ifndef __WIN32__
        char buf[1024 * 1024];
#else
        char buf[61440];
#endif

        int n = fread(buf, 1, sizeof (buf), f);
        fclose(f);
        if (n < 0 || n == sizeof (buf)) {
            if (n != sizeof (buf))
                gi.dprintf("ERROR: Failed to read damage override file %s\n", g_override_damages->string);
            else
                gi.dprintf("ERROR: Damage override file %s is too big\n", g_override_damages->string);
            return;
        }
        buf[n] = 0;

        char*d = strrchr(g_override_damages->string, '/');
        SetDescription(d ? d : g_override_damages->string);

        ParseOverride(NULL, buf);
    } else
        if (desc_set)
        SetDescription("(default)");
}

const char* GetWeaponName(uint8_t index) {
    if (index >= w_count) index = w_unknown;
    return weaponpaths[index];
}

rewardinfo_t GetCombatReward(edict_t *attacker, edict_t *inflictor, edict_t *target, int mod_weapon) {
    weapon_t w = identifyweapon(inflictor, attacker, mod_weapon);
    victim_t v = identifyvictim(target);
    tableent_t *t = table[w] + v;

    rewardinfo_t r;

    r.instant_h_points= t->rewards.instant_h_points;
    r.instant_a_points = t->rewards.instant_a_points;
    r.coop_points = t->rewards.coop_points;
    r.kill_points = t->rewards.kill_points;

    return r;
}

