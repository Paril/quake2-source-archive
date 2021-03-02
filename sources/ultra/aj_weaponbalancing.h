/* Weapon balancing CVARs */

extern	cvar_t	*mega_gibs; //extra gibs
extern 	cvar_t	*player_gib_health;  //when to gib
extern	cvar_t	*am_pod_bounces;  //number of bounces for antimatter pod
extern	cvar_t	*am_pod_mdl;

extern	cvar_t	*ionripper_extra_sounds;

// sets damage the weapon does per projectile
extern	cvar_t	*blaster_damage;
extern	cvar_t	*shotgun_damage;
extern	cvar_t	*sshotgun_damage;
extern	cvar_t	*machinegun_damage;
extern	cvar_t	*chaingun_damage;
extern	cvar_t	*grenade_damage;
extern	cvar_t	*rocket_damage;
extern	cvar_t	*hyperblaster_damage;
extern	cvar_t	*railgun_damage;
extern	cvar_t	*bfg_damage;

//ScarFace
extern cvar_t	*tesla_damage;
extern cvar_t	*tesla_radius;
extern cvar_t	*tesla_life;
extern cvar_t	*tesla_health;
extern cvar_t	*chainfist_damage;
extern cvar_t	*plasmabeam_damage;
extern cvar_t	*etf_rifle_damage;
extern cvar_t	*disruptor_damage;
extern cvar_t	*prox_damage;
extern cvar_t	*prox_radius;
extern cvar_t	*prox_life;
extern cvar_t	*prox_health;
extern cvar_t	*ionripper_damage;
extern cvar_t	*phalanx_damage;
extern cvar_t	*phalanx_damage2;
extern cvar_t	*phalanx_radius_damage;
extern cvar_t	*phalanx_radius;
extern cvar_t	*trap_life;
extern cvar_t	*trap_health;
extern cvar_t	*nuke_delay;
extern cvar_t	*nuke_life;
extern cvar_t	*nuke_radius;
extern cvar_t	*double_time;
extern cvar_t	*quad_fire_time;
extern cvar_t	*defender_blaster_damage;
extern cvar_t	*defender_blaster_speed;
extern cvar_t	*vengeance_health_threshold;
extern cvar_t	*blaster_type;
//end ScarFace

// sets speed of projectile fired
extern	cvar_t	*blaster_speed;
extern	cvar_t	*shotgun_speed;
extern	cvar_t	*sshotgun_speed;
extern	cvar_t	*machinegun_speed;
extern	cvar_t	*chaingun_speed;
extern	cvar_t	*grenade_speed;
extern	cvar_t	*rocket_speed;
extern	cvar_t	*hyperblaster_speed;
extern	cvar_t	*railgun_speed;
extern	cvar_t	*bfg_speed;
//ScarFace
extern cvar_t	*etf_rifle_speed;
extern cvar_t	*disruptor_speed;
extern cvar_t	*ionripper_speed;
extern cvar_t	*phalanx_speed;
extern cvar_t	*doppleganger_time;
extern cvar_t	*doppleganger_health;

extern cvar_t	*am_rocket_damage;
extern cvar_t	*am_rocket_damage2;
extern cvar_t	*am_rocket_rdamage;
extern cvar_t	*am_rocket_speed;
extern cvar_t	*am_rocket_radius;

extern cvar_t	*am_pod_damage;
extern cvar_t	*am_pod_damage2;
extern cvar_t	*am_pod_rdamage;
extern cvar_t	*am_pod_speed;
extern cvar_t	*am_pod_radius;
extern cvar_t	*am_pod_effect_damage;
extern cvar_t	*am_pod_effect_radius;

//end ScarFace

// sets how many projectiles are fired
extern	cvar_t	*blaster_count;
extern	cvar_t	*shotgun_count;
extern	cvar_t	*sshotgun_count;
extern	cvar_t	*machinegun_count;
extern	cvar_t	*chaingun_count;
extern	cvar_t	*grenade_count;
extern	cvar_t	*rocket_count;
extern	cvar_t	*hyperblaster_count;
extern	cvar_t	*railgun_count;
extern	cvar_t	*bfg_count;

// sets the horizontal spread radius of projectiles
extern	cvar_t	*blaster_hspread;
extern	cvar_t	*shotgun_hspread;
extern	cvar_t	*sshotgun_hspread;
extern	cvar_t	*machinegun_hspread;
extern	cvar_t	*chaingun_hspread;
extern	cvar_t	*grenade_hspread;
extern	cvar_t	*rocket_hspread;
extern	cvar_t	*hyperblaster_hspread;
extern	cvar_t	*railgun_hspread;
extern	cvar_t	*bfg_hspread;

// sets the vertical spread radius of projectiles
extern	cvar_t	*blaster_vspread;
extern	cvar_t	*shotgun_vspread;
extern	cvar_t	*sshotgun_vspread;
extern	cvar_t	*machinegun_vspread;
extern	cvar_t	*chaingun_vspread;
extern	cvar_t	*grenade_vspread;
extern	cvar_t	*rocket_vspread;
extern	cvar_t	*hyperblaster_vspread;
extern	cvar_t	*railgun_vspread;
extern	cvar_t	*bfg_vspread;

// sets max random extra damage
extern	cvar_t	*blaster_damage2;
extern	cvar_t	*shotgun_damage2;
extern	cvar_t	*sshotgun_damage2;
extern	cvar_t	*machinegun_damage2;
extern	cvar_t	*chaingun_damage2;
extern	cvar_t	*grenade_damage2;
extern	cvar_t	*rocket_damage2;
extern	cvar_t	*hyperblaster_damage2;
extern	cvar_t	*railgun_damage2;
extern	cvar_t	*bfg_damage2;

// sets radius damage
extern	cvar_t	*blaster_rdamage;
extern	cvar_t	*shotgun_rdamage;
extern	cvar_t	*sshotgun_rdamage;
extern	cvar_t	*machinegun_rdamage;
extern	cvar_t	*chaingun_rdamage;
extern	cvar_t	*grenade_rdamage;
extern	cvar_t	*rocket_rdamage;
extern	cvar_t	*hyperblaster_rdamage;
extern	cvar_t	*railgun_rdamage;
extern	cvar_t	*bfg_rdamage;

extern	cvar_t	*grenade_radius;
extern	cvar_t	*rocket_radius;
extern	cvar_t	*bfg_radius;

// Bot weapon balancing cvars

extern	cvar_t	*bot_blaster_damage;
extern	cvar_t	*bot_blaster_speed;
extern	cvar_t	*bot_shotgun_damage;
extern	cvar_t	*bot_shotgun_count;
extern	cvar_t	*bot_shotgun_hspread;
extern	cvar_t	*bot_shotgun_vspread;
extern	cvar_t	*bot_sshotgun_damage;
extern	cvar_t	*bot_sshotgun_count;
extern	cvar_t	*bot_sshotgun_hspread;
extern	cvar_t	*bot_sshotgun_vspread;
extern	cvar_t	*bot_machinegun_damage;
extern	cvar_t	*bot_machinegun_hspread;
extern	cvar_t	*bot_machinegun_vspread;
extern	cvar_t	*bot_chaingun_damage;
extern	cvar_t	*bot_chaingun_hspread;
extern	cvar_t	*bot_chaingun_vspread;
extern	cvar_t	*bot_hyperblaster_damage;
extern	cvar_t	*bot_hyperblaster_speed;
extern	cvar_t	*bot_rocket_damage;
extern	cvar_t	*bot_rocket_damage2;
extern	cvar_t	*bot_rocket_rdamage;
extern	cvar_t	*bot_rocket_radius;
extern	cvar_t	*bot_rocket_speed;
extern	cvar_t	*bot_grenade_damage;
extern	cvar_t	*bot_grenade_radius;
extern	cvar_t	*bot_railgun_damage;
extern	cvar_t	*bot_bfg_damage;
extern	cvar_t	*bot_bfg_radius;

