#include "g_local.h"

/******************* Cvar's *******************/

// enable/disable options
cvar_t	*use_runes;          // enables runes
cvar_t	*use_hook;           // enables grappling hook

cvar_t	*use_vwep;           // enables view weapons
cvar_t	*use_mapqueue;       // enables map queueing
cvar_t	*use_fastswitch;     // enables fast weapon switching (ala Rocket Arena 2)

cvar_t	*mapqueue;           // sets plain text file to read for map cycling
cvar_t	*map_random;         // enables random map queue order
cvar_t	*motd;               // sets plain text file to read for MOTD
cvar_t	*def_bestweapon;     // sets default client bestweapon on/off
cvar_t	*def_id;             // sets default client player id on/off
cvar_t	*def_hud;            // sets default client HUD
//                              0 = none, 1 = normal, 2 = lithium, 3 = ammo
cvar_t	*def_scores;         // sets default client scoreboard layout
//                                   0 = old, 1 = by frags, 2 = by FPH
cvar_t	*mega_gibs;			 // whether to spawn extra gibs, default to 0
cvar_t	*am_pod_bounces;  //number of bounces for antimatter pod
cvar_t	*am_pod_mdl;

cvar_t	*ionripper_extra_sounds;

cvar_t	*player_gib_health;  //when to gib
cvar_t	*fall_damagemod;     // fall damage modifier (fall damage multiplied by this)
cvar_t	*intermission_time;  // minimum time the intermission will last
cvar_t	*intermission_sound; // sound to play during intermission
cvar_t	*jacket_armor;       // sets pickup amount, max amount, normal resist, energy resist
cvar_t	*combat_armor;       // sets pickup amount, max amount, normal resist, energy resist
cvar_t	*body_armor;         // sets pickup amount, max amount, normal resist, energy resist
cvar_t	*shard_armor;        // sets amount of armor gained from armor shards
cvar_t	*power_armor_screen; // sets damage saved per cell
cvar_t	*power_armor_shield; // sets damage saved per cell
cvar_t	*dropweapammo;       // percentage of default ammo gained from a dropped weapon
cvar_t	*allow_func_explosives;

// sets damage the weapon does per projectile
cvar_t	*blaster_damage;
cvar_t	*shotgun_damage;
cvar_t	*sshotgun_damage;
cvar_t	*machinegun_damage;
cvar_t	*chaingun_damage;
cvar_t	*grenade_damage;
cvar_t	*rocket_damage;
cvar_t	*hyperblaster_damage;
cvar_t	*railgun_damage;
cvar_t	*bfg_damage;

//ScarFace
cvar_t	*tesla_damage;
cvar_t	*tesla_radius;
cvar_t	*tesla_life;
cvar_t	*tesla_health;
cvar_t	*chainfist_damage;
cvar_t	*plasmabeam_damage;
cvar_t	*etf_rifle_damage;
cvar_t	*disruptor_damage;
cvar_t	*prox_damage;
cvar_t	*prox_radius;
cvar_t	*prox_life;
cvar_t	*prox_health;
cvar_t	*ionripper_damage;
cvar_t	*phalanx_damage;
cvar_t	*phalanx_damage2;
cvar_t	*phalanx_radius_damage;
cvar_t	*phalanx_radius;
cvar_t	*trap_life;
cvar_t	*trap_health;
cvar_t	*nuke_delay;
cvar_t	*nuke_life;
cvar_t	*nuke_radius;
cvar_t	*defender_blaster_damage;
cvar_t	*defender_blaster_speed;
cvar_t	*vengeance_health_threshold;
cvar_t	*blaster_type;  //blaster color- 1=yellow, 2=green, 3=blue 3=green hand blaster, blue hyper
//end ScarFace

// sets speed of projectile fired
cvar_t	*blaster_speed;
cvar_t	*shotgun_speed;
cvar_t	*sshotgun_speed;
cvar_t	*machinegun_speed;
cvar_t	*chaingun_speed;
cvar_t	*grenade_speed;
cvar_t	*rocket_speed;
cvar_t	*hyperblaster_speed;
cvar_t	*railgun_speed;
cvar_t	*bfg_speed;

//ScarFace
cvar_t	*etf_rifle_speed;
cvar_t	*disruptor_speed;
cvar_t	*ionripper_speed;
cvar_t	*phalanx_speed;

cvar_t	*am_rocket_damage;
cvar_t	*am_rocket_damage2;
cvar_t	*am_rocket_rdamage;
cvar_t	*am_rocket_speed;
cvar_t	*am_rocket_radius;

cvar_t	*am_pod_damage;
cvar_t	*am_pod_damage2;
cvar_t	*am_pod_rdamage;
cvar_t	*am_pod_speed;
cvar_t	*am_pod_radius;
cvar_t	*am_pod_effect_damage;
cvar_t	*am_pod_effect_radius;

//end ScarFace

// sets how many projectiles are fired
cvar_t	*blaster_count;
cvar_t	*shotgun_count;
cvar_t	*sshotgun_count;
cvar_t	*machinegun_count;
cvar_t	*chaingun_count;
cvar_t	*grenade_count;
cvar_t	*rocket_count;
cvar_t	*hyperblaster_count;
cvar_t	*railgun_count;
cvar_t	*bfg_count;

// sets the horizontal spread radius of projectiles
cvar_t	*blaster_hspread;
cvar_t	*shotgun_hspread;
cvar_t	*sshotgun_hspread;
cvar_t	*machinegun_hspread;
cvar_t	*chaingun_hspread;
cvar_t	*grenade_hspread;
cvar_t	*rocket_hspread;
cvar_t	*hyperblaster_hspread;
cvar_t	*railgun_hspread;
cvar_t	*bfg_hspread;

// sets the vertical spread radius of projectiles
cvar_t	*blaster_vspread;
cvar_t	*shotgun_vspread;
cvar_t	*sshotgun_vspread;
cvar_t	*machinegun_vspread;
cvar_t	*chaingun_vspread;
cvar_t	*grenade_vspread;
cvar_t	*rocket_vspread;
cvar_t	*hyperblaster_vspread;
cvar_t	*railgun_vspread;
cvar_t	*bfg_vspread;

// sets max random extra damage
cvar_t	*blaster_damage2;
cvar_t	*shotgun_damage2;
cvar_t	*sshotgun_damage2;
cvar_t	*machinegun_damage2;
cvar_t	*chaingun_damage2;
cvar_t	*grenade_damage2;
cvar_t	*rocket_damage2;
cvar_t	*hyperblaster_damage2;
cvar_t	*railgun_damage2;
cvar_t	*bfg_damage2;

// sets radius damage
cvar_t	*blaster_rdamage;
cvar_t	*shotgun_rdamage;
cvar_t	*sshotgun_rdamage;
cvar_t	*machinegun_rdamage;
cvar_t	*chaingun_rdamage;
cvar_t	*grenade_rdamage;
cvar_t	*rocket_rdamage;
cvar_t	*hyperblaster_rdamage;
cvar_t	*railgun_rdamage;
cvar_t	*bfg_rdamage;

cvar_t	*grenade_radius;
cvar_t	*rocket_radius;
cvar_t	*bfg_radius;

// starting values
cvar_t	*start_weapon;

cvar_t	*start_health;		
cvar_t	*start_armor; // what sort of armor????
cvar_t	*start_bullets;	
cvar_t	*start_shells;	
cvar_t	*start_rockets;	
cvar_t	*start_grenades;
cvar_t	*start_cells;
cvar_t	*start_slugs;	
cvar_t	*start_magslugs;
cvar_t	*start_traps;
cvar_t	*start_rounds;
cvar_t	*start_flechettes;
cvar_t	*start_prox;
cvar_t	*start_tesla;

cvar_t	*start_blaster;
cvar_t	*start_shotgun;
cvar_t	*start_sshotgun;
cvar_t	*start_machinegun;
cvar_t	*start_chaingun;
cvar_t	*start_grenadelauncher;
cvar_t	*start_rocketlauncher;
cvar_t	*start_hyperblaster;
cvar_t	*start_railgun;
cvar_t	*start_bfg;
cvar_t	*start_phalanx;
cvar_t	*start_ionripper;
cvar_t	*start_etfrifle;
cvar_t	*start_disruptor;
cvar_t	*start_proxlauncher;
cvar_t	*start_plasmabeam;
cvar_t	*start_chainfist;
cvar_t	*start_shockwave;
cvar_t	*start_rune;

// weapon banning
cvar_t	*no_shotgun;
cvar_t	*no_sshotgun;
cvar_t	*no_machinegun;
cvar_t	*no_chaingun;
cvar_t	*no_grenadelauncher;
cvar_t	*no_rocketlauncher;
cvar_t	*no_hyperblaster;
cvar_t	*no_railgun;
cvar_t	*no_bfg;
cvar_t	*no_phalanx;
cvar_t	*no_ionripper;
cvar_t	*no_traps;
cvar_t	*no_bfg;
cvar_t	*no_quad;
cvar_t	*no_pent;
cvar_t	*no_etfrifle;
cvar_t	*no_disruptor;
cvar_t	*no_proxlauncher;
cvar_t	*no_plasmabeam;
cvar_t	*no_chainfist;

//ScarFace
cvar_t	*armor_bonus_value; //value of armor shards
cvar_t	*health_bonus_value; //value of armor shards
cvar_t	*powerup_max;
cvar_t	*nuke_max;
cvar_t	*doppleganger_max;
cvar_t	*defender_time;
cvar_t	*vengeance_time;
cvar_t	*hunter_time;
cvar_t	*doppleganger_time;
cvar_t	*doppleganger_health;
cvar_t	*quad_time;
cvar_t	*inv_time;
cvar_t	*breather_time;
cvar_t	*enviro_time;
cvar_t	*silencer_shots;
cvar_t	*ir_time;
cvar_t	*double_time;
cvar_t	*quad_fire_time;
//end ScarFace

// maximum values
cvar_t	*max_health;
cvar_t	*max_armor;	
cvar_t	*max_bullets;	
cvar_t	*max_shells;	
cvar_t	*max_rockets;	
cvar_t	*max_grenades;	
cvar_t	*max_cells;		
cvar_t	*max_slugs;
cvar_t	*max_magslugs;
cvar_t	*max_traps;
cvar_t	*max_prox;
cvar_t	*max_tesla;
cvar_t	*max_flechettes;
cvar_t	*max_rounds;

// maximum settings if a player gets a pack
cvar_t	*pack_health;
cvar_t	*pack_armor;
cvar_t	*pack_bullets;  // 300
cvar_t	*pack_shells;   // 200
cvar_t	*pack_rockets;  // 100
cvar_t	*pack_grenades; // 100
cvar_t	*pack_cells;    // 300
cvar_t	*pack_slugs;    // 100
cvar_t	*pack_magslugs;
cvar_t	*pack_flechettes;
cvar_t	*pack_rounds;
cvar_t	*pack_prox;
cvar_t	*pack_tesla;
cvar_t	*pack_traps;

cvar_t	*use_safety;         // enables safety spawn period
cvar_t	*safety_time;        // time the safety spawn shield lasts
cvar_t	*use_observer;       // enables clients to use observer/chasecam modes
cvar_t	*use_hitfactor;      // enables aim to be factored into damage inflicted
cvar_t	*fast_respawn;       // sets how much faster items respawn, as more players join
cvar_t	*fast_pbound;        // respawning gets faster up to this many players

cvar_t	*use_lithiumrunes;   // determines whether to use Runes or Techs
cvar_t	*rune_flags;         // determines which runes will show in the game, add these:
//                              1 = resist, 2 = strength, 4 = haste, 8 = regen, 16 = vampire
cvar_t	*rune_spawn;         // chance a rune will spawn from another item respawning
cvar_t	*rune_perplayer;     // sets runes per player that will appear in map
cvar_t	*rune_life;          // seconds a rune will stay around before disappearing
cvar_t	*rune_min;           // sets minimum number of runes to be in the game
cvar_t	*rune_max;           // sets maximum number of runes to be in the game

/// customisable names for runes
cvar_t	*rune_resist_name;
cvar_t	*rune_strength_name;
cvar_t	*rune_regen_name;
cvar_t	*rune_vampire_name;
cvar_t	*rune_haste_name;

cvar_t	*rune_resist_shortname;
cvar_t	*rune_strength_shortname;
cvar_t	*rune_regen_shortname;
cvar_t	*rune_vampire_shortname;
cvar_t	*rune_haste_shortname;

// customisable md2's for runes
cvar_t	*rune_resist_mdl;
cvar_t	*rune_strength_mdl;
cvar_t	*rune_regen_mdl;
cvar_t	*rune_vampire_mdl;
cvar_t	*rune_haste_mdl;

//customisable icons for runes- ScarFace
cvar_t	*rune_resist_icon;
cvar_t	*rune_strength_icon;
cvar_t	*rune_regen_icon;
cvar_t	*rune_vampire_icon;
cvar_t	*rune_haste_icon;

// customisable sounds for runes
cvar_t	*rune_resist_sound;
cvar_t	*rune_strength_sound;
cvar_t	*rune_regen_sound;
cvar_t	*rune_vampire_sound;
cvar_t	*rune_haste_sound;

cvar_t	*rune_haste; // what should I use this for?

cvar_t	*rune_resist;        // sets how much damage is divided by with resist rune
cvar_t	*rune_strength;      // sets how much damage is multiplied by with strength rune
cvar_t	*rune_regen;         // sets how fast health is gained back
cvar_t	*rune_regen_armor;	 // sets whether armor should be regenerated (CTF style) or not (Lithium)
cvar_t	*rune_regen_health_max;      // sets maximum health that can be gained from regen rune
cvar_t	*rune_regen_armor_max;      // sets maximum armor that can be gained from regen rune
cvar_t	*rune_regen_armor_always;      // sets whether armor should be regened regardless of if currently held

cvar_t	*rune_vampire;       // sets percentage of health gained from damage inflicted
cvar_t	*rune_vampiremax;    // sets maximum health that can be gained from vampire rune
//cvar_t	*rune_*_sound;       sets sounds for runes
cvar_t	*use_masskillkick;   // enables kicking anyone who mass kills
cvar_t	*use_chatfloodkick;  // enables kicking anyone who chat floods
cvar_t	*use_nocamp;         // enables no camping check
cvar_t	*banlist;            // sets plain text file to read for IP banning

cvar_t	*hook_offhand;       // enable off-hand hook - otherwise standard CTF grapple
cvar_t	*hook_speed;         // sets how fast the hook shoots out
cvar_t	*hook_pullspeed;     // sets how fast the hook pulls a player
cvar_t	*hook_sky;           // enables hooking to the sky
cvar_t	*hook_maxtime;       // sets max time you can stay hooked
cvar_t	*hook_damage;        // sets damage hook does to other players

cvar_t	*camp_threshold;     // sets movement required to not be camping
cvar_t	*camp_time;          // seconds a player has before getting a camping warning
cvar_t	*camp_warn;          // seconds a player has after warned, or be killed
cvar_t	*ping_watch;         // seconds until a clients ping (average so far) is checked
cvar_t	*ping_min;           // mi nimum ping allowed
cvar_t	*ping_max;           // maximum ping allowed
//cvar_t	*no_*               inhibit (disable) items, only takes effect after map change
cvar_t	*admin_code;         // sets the admin code, 0 to disable (max 5 digits)
cvar_t	*use_packs;          // enables packs
cvar_t	*use_gslog;          // enables GSLog frag logging (Gibstats Standard Log)
cvar_t	*gslog;              // sets GSLog output file
cvar_t	*gslog_flush;        // enables instant logging
cvar_t	*knockback_adjust;   // knockback adjustment (impact knockbacks multiplied by this)
cvar_t	*knockback_self;     // knockback adjustment for self inflicted impacts
//                                  (use this to modify rocket jumping height)
cvar_t	*pack_spawn;         // chance a pack will spawn from another item respawning
cvar_t	*pack_life;          // seconds a pack will stay around before disappearing

cvar_t	*ctffraglimit;

cvar_t	*lmctf;

cvar_t	*rune_spawn_dmstart;

cvar_t	*use_onegun;

// Bot weapon balancing cvars
cvar_t	*bot_blaster_damage;
cvar_t	*bot_blaster_speed;
cvar_t	*bot_shotgun_damage;
cvar_t	*bot_shotgun_count;
cvar_t	*bot_shotgun_hspread;
cvar_t	*bot_shotgun_vspread;
cvar_t	*bot_sshotgun_damage;
cvar_t	*bot_sshotgun_count;
cvar_t	*bot_sshotgun_hspread;
cvar_t	*bot_sshotgun_vspread;
cvar_t	*bot_machinegun_damage;
cvar_t	*bot_machinegun_hspread;
cvar_t	*bot_machinegun_vspread;
cvar_t	*bot_chaingun_damage;
cvar_t	*bot_chaingun_hspread;
cvar_t	*bot_chaingun_vspread;
cvar_t	*bot_hyperblaster_damage;
cvar_t	*bot_hyperblaster_speed;
cvar_t	*bot_rocket_damage;
cvar_t	*bot_rocket_damage2;
cvar_t	*bot_rocket_rdamage;
cvar_t	*bot_rocket_radius;
cvar_t	*bot_rocket_speed;
cvar_t	*bot_grenade_damage;
cvar_t	*bot_grenade_radius;
cvar_t	*bot_railgun_damage;
cvar_t	*bot_bfg_damage;
cvar_t	*bot_bfg_radius;

cvar_t	*use_replacelist;
cvar_t	*replace_file;

cvar_t	*use_lasers;

cvar_t	*lava_mod;
cvar_t	*slime_mod;
cvar_t	*bot_lava_mod;
cvar_t	*bot_slime_mod;
cvar_t	*bot_aimangle;
cvar_t	*bot_railtime;

cvar_t	*allow_flagdrop;
cvar_t	*allow_pickuprune;
cvar_t	*allow_runedrop;

cvar_t	*config_file;
cvar_t	*use_configlist;

cvar_t *zk_pinglimit;
cvar_t *zk_logonly;
cvar_t *zk_retry;

cvar_t	*ttctf;

cvar_t	*use_iplogging;

cvar_t	*showbotping;

cvar_t	*use_rogueplats;

cvar_t	*grapple_mdl;
cvar_t	*grapple_spin;

cvar_t	*strong_mines;
cvar_t	*hook_style;

cvar_t	*invisible_dmstarts;




void lithium_defaults(void)
{
	use_runes = gi.cvar("use_runes", "1", 0);
	use_hook = gi.cvar("use_hook", "1", 0);
	use_vwep = gi.cvar("use_vwep", "1", CVAR_SERVERINFO|CVAR_ARCHIVE);
	mapqueue = gi.cvar("mapqueue", "maps.lst", 0);
	map_random = gi.cvar("map_random", "1", 0);         // overide ridah's one?

	motd = gi.cvar("motd", "motd.txt", 0);
	def_bestweapon = gi.cvar("def_bestweapon", "0", 0);
	def_id = gi.cvar("def_id", "1", 0);
	def_hud = gi.cvar("def_hud", "2", 0);
	def_scores = gi.cvar("def_scores", "1", 0);
	mega_gibs = gi.cvar("mega_gibs", "0", 0);
	am_pod_bounces = gi.cvar("am_pod_bounces", "5", 0);
	am_pod_mdl = gi.cvar("am_pod_mdl", "models/items/tagtoken/tris.md2", 0);
	ionripper_extra_sounds = gi.cvar("ionripper_extra_sounds", "0", 0);

	player_gib_health = gi.cvar("player_gib_health", "-40", 0);
	fall_damagemod = gi.cvar("fall_damagemod", "1.0", 0);
	showbotping = gi.cvar("showbotping", "0", 0);

	intermission_time = gi.cvar("intermission_time", "8.5", 0); 
	intermission_sound = gi.cvar("intermission_sound", "world/xian1.wav", 0);

	jacket_armor = gi.cvar("jacket_armor", "50 100 .30 .00", 0);
	combat_armor = gi.cvar("combat_armor", "75 150 .60 .30", 0);
	body_armor = gi.cvar("body_armor", "100 200 .80 .60", 0);
	shard_armor = gi.cvar("shard_armor", "5", 0);
	power_armor_screen = gi.cvar("power_armor_screen", "0.25", 0);
	power_armor_shield = gi.cvar("power_armor_shield", "0.50", 0);
	dropweapammo = gi.cvar("dropweapammo", "0.5", 0);
	allow_func_explosives = gi.cvar("allow_func_explosives", "0", CVAR_LATCH);

	use_safety = gi.cvar("use_safety", "1", 0);
	safety_time = gi.cvar("safety_time", "3.0", 0);
	use_observer = gi.cvar("use_observer", "1", 0);
	use_hitfactor = gi.cvar("use_hitfactor", "0", 0);
	fast_respawn = gi.cvar("fast_respawn", "0.75", 0);
	fast_pbound = gi.cvar("rast_pbound", "16", 0);

	rune_spawn_dmstart = gi.cvar("rune_spawn_dmstart", "0", 0); // new 

	rune_flags = gi.cvar("rune_flags", "31", 0);
	rune_spawn = gi.cvar("rune_spawn", "0.10", 0);
	rune_perplayer = gi.cvar("rune_perplayer", "0.7", 0);
	rune_life = gi.cvar("rune_life", "20", 0);
	rune_min = gi.cvar("rune_min", "2", 0);
	rune_max = gi.cvar("rune_max", "10", 0);

	rune_resist = gi.cvar("rune_resist", "2.0", 0);
	rune_resist_sound = gi.cvar("rune_resist_sound", "world/force2.wav", 0);
	rune_resist_mdl = gi.cvar("rune_resist_mdl", "models/items/keys/pyramid/tris.md2", 0);
	rune_resist_icon = gi.cvar("rune_resist_icon", "k_pyramid", 0);
	rune_resist_name = gi.cvar("rune_resist_name", "resist rune", 0);
	rune_resist_shortname = gi.cvar("rune_resist_shortname", "Resist", 0);

	rune_strength = gi.cvar("rune_strength", "2.0", 0);
	rune_strength_sound = gi.cvar("rune_strength_sound", "items/damage3.wav", 0);
	rune_strength_mdl = gi.cvar("rune_strength_mdl", "models/items/keys/pyramid/tris.md2", 0);
	rune_strength_icon = gi.cvar("rune_strength_icon", "k_pyramid", 0);
	rune_strength_name = gi.cvar("rune_strength_name", "strength rune", 0);
	rune_strength_shortname = gi.cvar("rune_strength_shortname", "Strength", 0);

	rune_haste = gi.cvar("rune_haste", "", 0);
	rune_haste_sound = gi.cvar("rune_haste_sound", "items/s_health.wav", 0);
	rune_haste_mdl = gi.cvar("rune_haste_mdl", "models/items/keys/pyramid/tris.md2", 0);
	rune_haste_icon = gi.cvar("rune_haste_icon", "k_pyramid", 0);
	rune_haste_name = gi.cvar("rune_haste_name", "Time Accel", 0);
	rune_haste_shortname = gi.cvar("rune_haste_shortname", "Haste", 0);

	rune_regen = gi.cvar("rune_regen", "0.25", 0);
	rune_regen_armor = gi.cvar("rune_regen_armor", "1", 0);
	rune_regen_health_max = gi.cvar("rune_regen_health_max", "150", 0);
	rune_regen_armor_max = gi.cvar("rune_regen_armor_max", "150", 0);
	rune_regen_sound = gi.cvar("rune_regen_sound", "items/s_health.wav", 0);
	rune_regen_mdl = gi.cvar("rune_regen_mdl", "models/items/keys/pyramid/tris.md2", 0);
	rune_regen_icon = gi.cvar("rune_regen_icon", "k_pyramid", 0);
	rune_regen_name = gi.cvar("rune_regen_name", "regen rune", 0);
	rune_regen_shortname = gi.cvar("rune_regen_shortname", "Regen", 0);
	rune_regen_armor_always = gi.cvar("rune_regen_armor_always", "0", 0);

	rune_vampire = gi.cvar("rune_vampire", "0.5", 0);
	rune_vampiremax = gi.cvar("rune_vampiremax", "200", 0);
	rune_vampire_sound = gi.cvar("rune_vampire_sound", "makron/pain2.wav", 0);
	rune_vampire_mdl = gi.cvar("rune_vampire_mdl", "models/items/keys/pyramid/tris.md2", 0);
	rune_vampire_icon = gi.cvar("rune_vampire_icon", "k_pyramid", 0);
	rune_vampire_name = gi.cvar("rune_vampire_name", "vampire rune", 0);
	rune_vampire_shortname = gi.cvar("rune_vampire_shortname", "Vampire", 0);

	use_masskillkick = gi.cvar("use_masskillkick", "1", 0);
	use_chatfloodkick = gi.cvar("use_chatfloodkick", "1", 0);
	use_nocamp = gi.cvar("use_nocamp", "0", 0);
	banlist = gi.cvar("banlist", "banlist.txt", 0);

	hook_offhand = gi.cvar("hook_offhand", "1", 0);
	hook_speed = gi.cvar("hook_speed", "900", 0);
	hook_pullspeed = gi.cvar("hook_pullspeed", "700", 0);
	hook_sky = gi.cvar("hook_sky", "0", 0);
	hook_maxtime = gi.cvar("hook_maxtime", "5", 0);
	hook_damage = gi.cvar("hook_damage", "2", 0);
	hook_style =  gi.cvar("hook_style", "1", 0);

	camp_threshold = gi.cvar("camp_threshold", "250", 0);
	camp_time = gi.cvar("camp_time", "8.0", 0);
	camp_warn = gi.cvar("camp_warn", "5.0", 0);

	ping_watch = gi.cvar("ping_watch", "10", 0);
	ping_min = gi.cvar("ping_min", "0", 0);
	ping_max = gi.cvar("ping_max", "999", 0);

	admin_code = gi.cvar("admin_code", "0", 0);
	use_packs = gi.cvar("use_packs", "1", 0);

	use_gslog = gi.cvar("use_gslog", "0", 0);
	gslog = gi.cvar("gslog", "gslog.log", 0);
	gslog_flush = gi.cvar("gslog_flush", "0", 0);

	knockback_adjust = gi.cvar("knockback_adjust", "1.0", 0);
	knockback_self = gi.cvar("knockback_self", "3.2", 0);

//ScarFace
	armor_bonus_value = gi.cvar("armor_bonus_value", "2", 0);
	health_bonus_value = gi.cvar("health_bonus_value", "2", 0);
	powerup_max = gi.cvar("powerup_max", "2", 0);
	doppleganger_max = gi.cvar("doppleganger_max", "1", 0);
	nuke_max = gi.cvar("nuke_max", "1", 0);
	defender_time = gi.cvar("defender_time", "60", 0);
	vengeance_time = gi.cvar("vengeance_time", "60", 0);
	hunter_time = gi.cvar("hunter_time", "60", 0);
	doppleganger_time = gi.cvar("doppleganger_time", "30", 0);
	doppleganger_health = gi.cvar("doppleganger_health", "30", 0);
	quad_time = gi.cvar("quad_time", "30", 0);
	inv_time = gi.cvar("inv_time", "30", 0);
	breather_time = gi.cvar("breather_time", "30", 0);
	enviro_time = gi.cvar("enviro_time", "30", 0);
	silencer_shots = gi.cvar("silencer_shots", "30", 0);
	ir_time = gi.cvar("ir_time", "30", 0);
	double_time = gi.cvar("double_time", "30", 0);
	quad_fire_time = gi.cvar("quad_fire_time", "30", 0);
//end ScarFace	
	pack_health = gi.cvar("pack_health", "120", 0);
	pack_armor = gi.cvar("pack_armor", "250", 0);
	pack_bullets = gi.cvar("pack_bullets", "360", 0);
	pack_shells = gi.cvar("pack_shells", "200", 0);
	pack_rockets = gi.cvar("pack_rockets", "100", 0);
	pack_grenades = gi.cvar("pack_grenades", "100", 0);
	pack_cells = gi.cvar("pack_cells", "360", 0);
	pack_slugs = gi.cvar("pack_slugs", "100", 0);
	pack_magslugs = gi.cvar("pack_magslugs", "100", 0);
	pack_flechettes = gi.cvar("pack_flechettes", "350", 0);
	pack_rounds = gi.cvar("pack_rounds", "150", 0);
	pack_prox = gi.cvar("pack_prox", "100", 0);
	pack_tesla = gi.cvar("pack_tesla", "100", 0);
	pack_traps = gi.cvar("pack_traps", "50", 0);

	pack_spawn = gi.cvar("pack_spawn", "0.03", 0);
	pack_life = gi.cvar("pack_life", "25", 0);

	max_health = gi.cvar("max_health", "120", 0);
	max_armor = gi.cvar("max_armor", "200", 0);
	max_bullets = gi.cvar("max_bullets", "240", 0);
	max_shells = gi.cvar("max_shells", "120", 0);
	max_rockets = gi.cvar("max_rockets", "60", 0);
	max_grenades = gi.cvar("max_grenades", "60", 0);
	max_cells = gi.cvar("max_cells", "240", 0);
	max_slugs = gi.cvar("max_slugs", "60", 0);
	max_magslugs = gi.cvar("max_magslugs", "60", 0);
	max_traps = gi.cvar("max_traps", "20", 0);
	max_prox = gi.cvar("max_prox", "50", 0);
	max_tesla = gi.cvar("max_tesla", "50", 0);
	max_flechettes = gi.cvar("max_flechettes", "300", 0);
	max_rounds = gi.cvar("max_rounds", "150", 0);

	start_weapon = gi.cvar("start_weapon", "0", 0);

	start_health = gi.cvar("start_health", "100", 0);
	start_armor = gi.cvar("start_armor", "0", 0);
	start_bullets = gi.cvar("start_bullets", "50", 0);
	start_shells = gi.cvar("start_shells", "10", 0);
	start_rockets = gi.cvar("start_rockets", "5", 0);
	start_grenades = gi.cvar("start_grenades", "1", 0);
	start_cells = gi.cvar("start_cells", "50", 0);
	start_slugs = gi.cvar("start_slugs", "5", 0);
	start_magslugs = gi.cvar("start_magslugs", "10", 0);
	start_traps = gi.cvar("start_traps", "0", 0);
	start_rounds = gi.cvar("start_rounds", "5", 0);
	start_flechettes = gi.cvar("start_flechettes", "25", 0);
	start_prox = gi.cvar("start_prox", "0", 0);
	start_tesla = gi.cvar("start_tesla", "0", 0);

	start_blaster = gi.cvar("start_blaster", "1", 0);
	start_shotgun = gi.cvar("start_shotgun", "0", 0);
	start_sshotgun = gi.cvar("start_sshotgun", "0", 0);
	start_machinegun = gi.cvar("start_machinegun", "1", 0);
	start_chaingun = gi.cvar("start_chaingun", "0", 0);
	start_grenadelauncher = gi.cvar("start_grenadelauncher", "0", 0);
	start_rocketlauncher = gi.cvar("start_rocketlauncher", "0", 0);
	start_hyperblaster = gi.cvar("start_hyperblaster", "0", 0);
	start_railgun = gi.cvar("start_railgun", "0", 0);
	start_bfg = gi.cvar("start_bfg", "0", 0);
	start_phalanx = gi.cvar("start_phalanx", "0", 0);
	start_ionripper = gi.cvar("start_ionripper", "0", 0);
	start_etfrifle = gi.cvar("start_etfrifle", "0", 0);
	start_disruptor = gi.cvar("start_disruptor", "0", 0);
	start_proxlauncher = gi.cvar("start_proxlauncher", "0", 0);
	start_plasmabeam = gi.cvar("start_plasmabeam", "0", 0);
	start_chainfist = gi.cvar("start_chainfist", "0", 0);
	start_shockwave = gi.cvar("start_shockwave", "0", 0);

	start_rune = gi.cvar("start_rune", "0", 0);

	no_shotgun = gi.cvar("no_shotgun", "0", 0);
	no_sshotgun = gi.cvar("no_sshotgun", "0", 0);
	no_machinegun = gi.cvar("no_machinegun", "0", 0);
	no_chaingun = gi.cvar("no_chaingun", "0", 0);
	no_grenadelauncher = gi.cvar("no_grenadelauncher", "0", 0);
	no_rocketlauncher = gi.cvar("no_rocketlauncher", "0", 0);
	no_hyperblaster = gi.cvar("no_hyperblaster", "0", 0);
	no_railgun = gi.cvar("no_railgun", "0", 0);
	no_bfg = gi.cvar("no_bfg", "1", 0);
	no_quad = gi.cvar("no_quad", "0", 0);
	no_pent = gi.cvar("no_pent", "1", 0);
	no_phalanx = gi.cvar("no_phalanx", "0", 0);
	no_ionripper = gi.cvar("no_ionripper", "0", 0);
	no_traps = gi.cvar("no_traps", "0", 0);
	no_proxlauncher = gi.cvar("no_proxlauncher", "0", 0);
	no_etfrifle = gi.cvar("no_etfrifle", "0", 0);
	no_disruptor = gi.cvar("no_disruptor", "0", 0);
	no_plasmabeam = gi.cvar("no_plasmabeam", "0", 0);
	no_chainfist = gi.cvar("no_chainfist", "0", 0);

	use_lithiumrunes = gi.cvar("use_lithiumrunes", "1", 0);

	ctffraglimit = gi.cvar("ctffraglimit", "0", 0);

	lmctf = gi.cvar("lmctf", "0", 0);

	blaster_damage = gi.cvar("blaster_damage", "17", 0);
	blaster_speed = gi.cvar("blaster_speed", "1000", 0);
	shotgun_damage = gi.cvar("shotgun_damage", "6", 0);
	shotgun_count = gi.cvar("shotgun_count", "12", 0);
	shotgun_hspread = gi.cvar("shotgun_hspread", "500", 0);
	shotgun_vspread = gi.cvar("shotgun_vspread", "500", 0);
	sshotgun_damage = gi.cvar("sshotgun_damage", "6", 0);
	sshotgun_count = gi.cvar("sshotgun_count", "22", 0);
	sshotgun_hspread = gi.cvar("sshotgun_hspread", "1000", 0);
	sshotgun_vspread = gi.cvar("sshotgun_vspread", "500", 0);
	machinegun_damage = gi.cvar("machinegun_damage", "8", 0);
	machinegun_hspread = gi.cvar("machinegun_hspread", "300", 0);
	machinegun_vspread = gi.cvar("machinegun_vspread", "500", 0);
	chaingun_damage = gi.cvar("chaingun_damage", "8", 0);
	chaingun_hspread = gi.cvar("chaingun_hspread", "300", 0);
	chaingun_vspread = gi.cvar("chaingun_vspread", "500", 0);
	hyperblaster_damage = gi.cvar("hyperblaster_damage", "17", 0);
	hyperblaster_speed = gi.cvar("hyperblaster_speed", "1000", 0);
	rocket_damage = gi.cvar("rocket_damage", "100", 0);
	rocket_damage2 = gi.cvar("rocket_damage2", "20", 0);
	rocket_rdamage = gi.cvar("rocket_rdamage", "120", 0);
	rocket_radius = gi.cvar("rocket_radius", "140", 0);
	rocket_speed = gi.cvar("rocket_speed", "650", 0);
	grenade_damage = gi.cvar("grenade_damage", "100", 0);
	grenade_radius = gi.cvar("grenade_radius", "140", 0);
	railgun_damage = gi.cvar("railgun_damage", "120", 0);
	bfg_damage = gi.cvar("bfg_damage", "200", 0);
	bfg_radius = gi.cvar("bfg_radius", "1000", 0);

//ScarFace
	tesla_damage = gi.cvar("tesla_damage", "3", 0);
	tesla_radius = gi.cvar("tesla_radius", "128", 0);
	tesla_life = gi.cvar("tesla_life", "30", 0);
	tesla_health = gi.cvar("tesla_health", "20", 0);
	chainfist_damage = gi.cvar("chainfist_damage", "30", 0);
	plasmabeam_damage = gi.cvar("plasmabeam_damage", "15", 0);
	etf_rifle_damage = gi.cvar("etf_rifle_damage", "10", 0);
	etf_rifle_speed = gi.cvar("etf_rifle_speed", "750", 0);
	disruptor_damage = gi.cvar("disruptor_damage", "30", 0);
	disruptor_speed = gi.cvar("disruptor_speed", "1000", 0);
	prox_damage = gi.cvar("prox_damage", "90", 0);
	prox_radius = gi.cvar("prox_radius", "192", 0);
	prox_life = gi.cvar("prox_life", "45", 0);
	prox_health = gi.cvar("prox_health", "20", 0);
	ionripper_damage = gi.cvar("ionripper_damage", "30", 0);
	ionripper_speed = gi.cvar("ionripper_speed", "500", 0);
	phalanx_damage = gi.cvar("phalanx_damage", "70", 0);
	phalanx_damage2 = gi.cvar("phalanx_damage2", "10", 0);
	phalanx_radius_damage = gi.cvar("phalanx_radius_damage", "120", 0);
	phalanx_radius = gi.cvar("phalanx_radius", "120", 0);
	phalanx_speed = gi.cvar("phalanx_speed", "725", 0);

	am_rocket_damage = gi.cvar("am_rocket_damage", "250", 0);
	am_rocket_damage2 = gi.cvar("am_rocket_damage2", "120", 0);
	am_rocket_rdamage = gi.cvar("am_rocket_rdamage", "500", 0);
	am_rocket_speed = gi.cvar("am_rocket_speed", "650", 0);
	am_rocket_radius = gi.cvar("am_rocket_radius", "5000", 0);

	am_pod_damage = gi.cvar("am_pod_damage", "40", 0);
	am_pod_damage2 = gi.cvar("am_pod_damage2", "20", 0);
	am_pod_rdamage = gi.cvar("am_pod_rdamage", "100", 0);
	am_pod_speed = gi.cvar("am_pod_speed", "650", 0);
	am_pod_radius = gi.cvar("am_pod_radius", "300", 0);
	am_pod_effect_damage = gi.cvar("am_pod_effect_damage", "180", 0);
	am_pod_effect_radius = gi.cvar("am_pod_effect_radius", "350", 0);

	trap_life = gi.cvar("trap_life", "30", 0);
	trap_health = gi.cvar("trap_health", "30", 0);
	nuke_delay = gi.cvar("nuke_delay", "4", 0);
	nuke_life = gi.cvar("nuke_life", "6", 0);
	nuke_radius = gi.cvar("nuke_radius", "512", 0);
	defender_blaster_damage = gi.cvar("defender_blaster_damage", "10", 0);
	defender_blaster_speed = gi.cvar("defender_blaster_speed", "1000", 0);
	vengeance_health_threshold = gi.cvar("vengeance_health_threshold", "25", 0);
	blaster_type = gi.cvar("blaster_type", "1", 0);
//end ScarFace

	bot_blaster_damage = gi.cvar("bot_blaster_damage", "17", 0);
	bot_blaster_speed = gi.cvar("bot_blaster_speed", "1000", 0);
	bot_shotgun_damage = gi.cvar("bot_shotgun_damage", "6", 0);
	bot_shotgun_count = gi.cvar("bot_shotgun_count", "12", 0);
	bot_shotgun_hspread = gi.cvar("bot_shotgun_hspread", "500", 0);
	bot_shotgun_vspread = gi.cvar("bot_shotgun_vspread", "500", 0);
	bot_sshotgun_damage = gi.cvar("bot_sshotgun_damage", "6", 0);
	bot_sshotgun_count = gi.cvar("bot_sshotgun_count", "22", 0);
	bot_sshotgun_hspread = gi.cvar("bot_sshotgun_hspread", "1000", 0);
	bot_sshotgun_vspread = gi.cvar("bot_sshotgun_vspread", "500", 0);
	bot_machinegun_damage = gi.cvar("bot_machinegun_damage", "8", 0);
	bot_machinegun_hspread = gi.cvar("bot_machinegun_hspread", "300", 0);
	bot_machinegun_vspread = gi.cvar("bot_machinegun_vspread", "500", 0);
	bot_chaingun_damage = gi.cvar("bot_chaingun_damage", "8", 0);
	bot_chaingun_hspread = gi.cvar("bot_chaingun_hspread", "300", 0);
	bot_chaingun_vspread = gi.cvar("bot_chaingun_vspread", "500", 0);
	bot_hyperblaster_damage = gi.cvar("bot_hyperblaster_damage", "17", 0);
	bot_hyperblaster_speed = gi.cvar("bot_hyperblaster_speed", "1000", 0);
	bot_rocket_damage = gi.cvar("bot_rocket_damage", "100", 0);
	bot_rocket_damage2 = gi.cvar("bot_rocket_damage2", "20", 0);
	bot_rocket_rdamage = gi.cvar("bot_rocket_rdamage", "120", 0);
	bot_rocket_radius = gi.cvar("bot_rocket_radius", "140", 0);
	bot_rocket_speed = gi.cvar("bot_rocket_speed", "650", 0);
	bot_grenade_damage = gi.cvar("bot_grenade_damage", "100", 0);
	bot_grenade_radius = gi.cvar("bot_grenade_radius", "140", 0);
	bot_railgun_damage = gi.cvar("bot_railgun_damage", "120", 0);
	bot_bfg_damage = gi.cvar("bot_bfg_damage", "200", 0);
	bot_bfg_radius = gi.cvar("bot_bfg_radius", "1000", 0);

	use_replacelist = gi.cvar("use_replacelist", "1", 0);
	replace_file = gi.cvar("replace_file", "replace.lst", 0);

	use_onegun = gi.cvar("use_onegun", "0", 0);

	use_lasers = gi.cvar("use_lasers", "1", 0);
	bot_lava_mod = gi.cvar("bot_lava_mod", "1.0", 0);
	bot_slime_mod = gi.cvar("bot_slime_mod", "1.0", 0);

	lava_mod = gi.cvar("lava_mod", "1.0", 0);
	slime_mod = gi.cvar("slime_mod", "1.0", 0);

	bot_aimangle = gi.cvar("bot_aimangle", "15", 0);
	bot_railtime = gi.cvar("bot_railtime", "0.2", 0);

	allow_flagdrop = gi.cvar("allow_flagdrop", "0", 0);
	allow_runedrop = gi.cvar("allow_runedrop", "1", 0);
	allow_pickuprune = gi.cvar("allow_pickuprune", "1", 0);

	config_file = gi.cvar("config_file", "config.lst", CVAR_LATCH);
	use_configlist = gi.cvar("use_configlist", "0", 0);

	zk_pinglimit = gi.cvar("zk_pinglimit", "600", CVAR_ARCHIVE);
	zk_logonly = gi.cvar("zk_logonly", "0", CVAR_ARCHIVE);
	zk_retry = gi.cvar("zk_retry", "0", CVAR_ARCHIVE);

	ttctf = gi.cvar("ttctf", "0", 0);
	use_iplogging = gi.cvar("use_iplogging", "0", 0);

	use_rogueplats = gi.cvar("userogueplats", "1", 0);

	grapple_mdl = gi.cvar("grapple_mdl", "models/objects/grenade2/tris.md2", 0);
	grapple_spin = gi.cvar("grapple_spin", "0", 0);

// rogue
	strong_mines = gi.cvar ("strong_mines", "0", 0);

	invisible_dmstarts = gi.cvar("invisible_dmstarts","0", CVAR_LATCH);

}
