/* the "main" prototypes etc. */

/* FAR too much in here for good dependency based rebuilds... */

#define ULTRA_VERSION	0.40


void lithium_init(void);
void lithium_initclient(gclient_t *client);
void lithium_choosestartweap(edict_t *ent);
void lithium_defaults(void);

int lithium_weaponbanning(edict_t *ent);


void lithium_observer(edict_t *ent, pmenu_t *menu);
void camera_on(edict_t *ent, pmenu_t *menu);
void lithium_motd(edict_t *ent);

void lithium_observer(edict_t *ent, pmenu_t *menu);

/******************* Cvar's *******************/

// enable/disable options
extern	cvar_t	*use_hook;           // enables grappling hook
extern	cvar_t	*use_vwep;           // enables view weapons
extern	cvar_t	*use_mapqueue;       // enables map queueing
extern	cvar_t	*use_fastswitch;     // enables fast weapon switching (ala Rocket Arena 2)


extern	cvar_t	*mapqueue;           // sets plain text file to read for map cycling
extern	cvar_t	*map_random;         // enables random map queue order
extern	cvar_t	*motd;               // sets plain text file to read for MOTD
extern	cvar_t	*def_bestweapon;     // sets default client bestweapon on/off
extern	cvar_t	*def_id;             // sets default client player id on/off

extern	cvar_t	*fall_damagemod;     // fall damage modifier (fall damage multiplied by this)
extern	cvar_t	*jacket_armor;       // sets pickup amount, max amount, normal resist, energy resist
extern	cvar_t	*combat_armor;       // sets pickup amount, max amount, normal resist, energy resist
extern	cvar_t	*body_armor;         // sets pickup amount, max amount, normal resist, energy resist
extern	cvar_t	*shard_armor;        // sets amount of armor gained from armor shards
extern	cvar_t	*power_armor_screen; // sets damage saved per cell
extern	cvar_t	*power_armor_shield; // sets damage saved per cell
extern	cvar_t	*dropweapammo;       // percentage of default ammo gained from a dropped weapon
extern	cvar_t	*allow_func_explosives;




//inhibit (disable) items, only takes effect after map change
extern	cvar_t	*no_shotgun;
extern	cvar_t	*no_sshotgun;
extern	cvar_t	*no_machinegun;
extern	cvar_t	*no_chaingun;
extern	cvar_t	*no_grenadelauncher;
extern	cvar_t	*no_rocketlauncher;
extern	cvar_t	*no_hyperblaster;
extern	cvar_t	*no_railgun;
extern	cvar_t	*no_bfg;
extern	cvar_t	*no_quad;
extern	cvar_t	*no_pent;
extern	cvar_t	*no_phalanx;
extern	cvar_t	*no_ionripper;
extern	cvar_t	*no_traps;
extern	cvar_t	*no_etfrifle;
extern	cvar_t	*no_disruptor;
extern	cvar_t	*no_proxlauncher;
extern	cvar_t	*no_plasmabeam;
extern	cvar_t	*no_chainfist;


extern	cvar_t	*use_safety;         // enables safety spawn period
extern	cvar_t	*safety_time;        // time the safety spawn shield lasts
extern	cvar_t	*use_observer;       // enables clients to use observer/chasecam modes
extern	cvar_t	*use_hitfactor;      // enables aim to be factored into damage inflicted
extern	cvar_t	*fast_respawn;       // sets how much faster items respawn, as more players join
extern	cvar_t	*fast_pbound;        // respawning gets faster up to this many players

extern	cvar_t	*hook_offhand;       // enable the off-hand hook - otherwise standard CTF grapple
extern	cvar_t	*hook_speed;         // sets how fast the hook shoots out
extern	cvar_t	*hook_pullspeed;     // sets how fast the hook pulls a player
extern	cvar_t	*hook_sky;           // enables hooking to the sky
extern	cvar_t	*hook_maxtime;       // sets max time you can stay hooked
extern	cvar_t	*hook_damage;        // sets damage hook does to other players

// begin some unused cvars
extern	cvar_t	*use_masskillkick;   // enables kicking anyone who mass kills
extern	cvar_t	*use_chatfloodkick;  // enables kicking anyone who chat floods
extern	cvar_t	*use_nocamp;         // enables no camping check
extern	cvar_t	*camp_threshold;     // sets movement required to not be camping
extern	cvar_t	*camp_time;          // seconds a player has before getting a camping warning
extern	cvar_t	*camp_warn;          // seconds a player has after warned, or be killed
extern	cvar_t	*ping_watch;         // seconds until a clients ping (average so far) is checked
extern	cvar_t	*ping_min;           // mi nimum ping allowed
extern	cvar_t	*ping_max;           // maximum ping allowed
extern	cvar_t	*admin_code;         // sets the admin code, 0 to disable (max 5 digits)
extern	cvar_t	*use_packs;          // enables packes
extern	cvar_t	*use_gslog;          // enables GSLog frag logging (Gibstats Standard Log)
extern	cvar_t	*gslog;              // sets GSLog output file
extern	cvar_t	*gslog_flush;        // enables instant logging
// end unused cvars

extern	cvar_t	*knockback_adjust;   // knockback adjustment (impact knockbacks multiplied by this)
extern	cvar_t	*knockback_self;     // knockback adjustment for self inflicted impacts
//                                      (use this to modify rocket jumping height)
extern	cvar_t	*pack_spawn;         // chance a pack will spawn from another item respawning
extern	cvar_t	*pack_life;          // seconds a pack will stay around before disappearing

extern	cvar_t	*ctffraglimit;
extern	cvar_t	*lmctf;

extern	cvar_t	*rune_spawn_dmstart;

extern	cvar_t	*use_onegun;

extern	cvar_t	*use_lasers;
extern	cvar_t	*bot_lava_mod;
extern	cvar_t	*bot_slime_mod;
extern	cvar_t	*lava_mod;
extern	cvar_t	*slime_mod;
extern	cvar_t	*bot_aimangle;
extern	cvar_t	*bot_railtime;

extern	cvar_t	*allow_flagdrop;
extern	cvar_t	*allow_runedrop;
extern	cvar_t	*allow_pickuprune;

extern cvar_t *zk_pinglimit;
extern cvar_t *zk_logonly;
extern cvar_t *zk_retry;

extern cvar_t	*ttctf;

extern gitem_t *item_phalanx, *item_ionripper, *item_trap, *item_quadfire, *item_magslugs, *item_ir_goggles, *item_double, *item_sphere_vengeance, *item_sphere_hunter, *item_sphere_defender, *item_doppleganger;

extern cvar_t *use_rogueplats;


extern cvar_t *grapple_mdl;
extern cvar_t *grapple_spin;

extern	cvar_t	*strong_mines;
extern	cvar_t	*hook_style;

extern cvar_t *invisible_dmstarts;

//ScarFace
extern cvar_t	*armor_bonus_value; //value of armor shards
extern cvar_t	*health_bonus_value; //value of armor shards
extern cvar_t	*powerup_max;
extern cvar_t	*nuke_max;
extern cvar_t	*doppleganger_max;
extern cvar_t	*defender_time;
extern cvar_t	*vengeance_time;
extern cvar_t	*hunter_time;
extern cvar_t	*quad_time;
extern cvar_t	*inv_time;
extern cvar_t	*breather_time;
extern cvar_t	*enviro_time;
extern cvar_t	*silencer_shots;
extern cvar_t	*ir_time;
extern cvar_t	*double_time;
extern cvar_t	*quad_fire_time;
//end ScarFace