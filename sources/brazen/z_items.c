#include "g_local.h"

void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void DoRespawn (edict_t *ent);

/*
======================================================================
GRIM 26/06/2001 10:05AM
======================================================================
*/

gitem_t	itemlist[] = 
{
	{
		NULL
	},	// leave index 0 alone

/*QUAKED do_not_put_me_in_map - hands (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"do_not_put_me_in_map",
		"",
		"",
/* icon */      "w_hands",
/* pickup */	"HANDS",
/* tag */       II_HANDS,
/* quant */     0,
/* ammoTag */	0,
		IT_WEAPON,
		0,
/* precache */ ""
	},


/*QUAKED weapon_pistol (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_pistol",
		"models/g_pistol/tris.md2",
		"models/v_pistol/tris.md2",
/* icon */      "w_pistol",
/* pickup */	"Pistol",
/* tag */       II_PISTOL,
/* quant */     24,
/* ammoTag */	II_PISTOL_CLIP,
		IT_WEAPON,
		10,
/* precache */ "models/v_twin_pistols/tris.md2 weapons/blastf1a.wav weapons/pistol_clipin.wav weapons/pistol_clipout.wav"
	},

/*QUAKED ammo_pistolclip (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_pistolclip",
		"models/a_pistol/tris.md2",
		NULL,
/* icon */      "a_pistol",
/* pickup */	"Pistol Clip",
/* tag */       II_PISTOL_CLIP,
/* quant */     24,
/* ammoTag */	0,
		IT_AMMO | IT_CLIP,
		6,
/* precache */ ""
	},

/*QUAKED weapon_submach (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_submach",
		"models/g_submach/tris.md2",
		"models/v_submach/tris.md2",
/* icon */      "w_submach",
/* pickup */	"Sub-Machinegun",
/* tag */       II_SUBMACH,
/* quant */     30,
/* ammoTag */	II_SUBMACH_CLIP,
		IT_WEAPON,
		10,
/* precache */ "models/v_twin_submach/tris.md2 weapons/blastf1a.wav weapons/sub_clipin.wav weapons/sub_clipout.wav"
	},

/*QUAKED ammo_submachclip (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_submachclip",
		"models/a_submach/tris.md2",
		NULL,
/* icon */      "a_submach",
/* pickup */	"Sub-Machingun Clip",
/* tag */       II_SUBMACH_CLIP,
/* quant */     30,
/* ammoTag */	0,
		IT_AMMO | IT_CLIP,
		6,
/* precache */ ""
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shotgun",
		"models/g_shotgun/tris.md2",
		"models/v_shotgun/tris.md2",
/* icon */      "w_shotgun",
/* pickup */	"Shotgun",
/* tag */       II_SHOTGUN,
/* quant */     18,
/* ammoTag */	II_SHOTGUN_CLIP,
		IT_WEAPON | IT_SLING,
		30,
/* precache */ "models/v_twin_shotguns/tris.md2 weapons/sshotf1b.wav weapons/shotgun_clipin.wav weapons/shotgun_clipout.wav weapons/shotcock.wav"
	},

/*QUAKED ammo_shotgunclip (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_shotgunclip",
		"models/a_shotgun/tris.md2",
		NULL,
/* icon */      "a_shotgun",
/* pickup */	"Shotgun Clip",
/* tag */       II_SHOTGUN_CLIP,
/* quant */     18,
/* ammoTag */	0,
		IT_AMMO | IT_CLIP,
		12,
/* precache */ ""
	},

/*QUAKED weapon_fraghgrenade (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_fraghgrenade",
		"models/g_handgrenade/tris.md2",
		"models/weapons/v_handgr/tris.md2",
/* icon */      "a_fraghgren",
/* pickup */	"Fragmentation Hand Grenade",
/* tag */       II_FRAG_HANDGRENADE,
/* quant */     2,
/* ammoTag */	II_FRAG_HANDGRENADE,
		IT_WEAPON | IT_AMMO,
		6,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED weapon_emphgrenade (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_emphgrenade",
		"models/g_emphandgrenade/tris.md2",
		"models/v_emphandgr/tris.md2",
/* icon */      "a_emphgren",
/* pickup */	"EMP Hand Grenade",
/* tag */       II_EMP_HANDGRENADE,
/* quant */     2,
/* ammoTag */	II_EMP_HANDGRENADE,
		IT_WEAPON | IT_AMMO,
		6,
/* precache */ "weapons/bfg__x1b.wav weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},


	/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chaingun", 
		"models/g_chain/tris.md2",
		"models/weapons/v_chain/tris.md2",
/* icon */	"w_chaingun",
/* pickup */	"Chaingun",
/* tag */       II_CHAINGUN,
/* quant */     999,
/* ammoTag */	II_CHAINGUN_PACK,
		IT_WEAPON | IT_SLING,
		30,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav"
	},

/*QUAKED ammo_chaingunpack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_chaingunpack",
		"models/g_chain/tris.md2",
		NULL,
/* icon */      "a_chpack",
/* pickup */	"Chaingun Pack",
/* tag */       II_CHAINGUN_PACK,
/* quant */     999,
/* ammoTag */	0,
		IT_AMMO | IT_SLING,
		50,
/* precache */ ""
	},


/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		"models/g_launch/tris.md2",
		"models/weapons/v_launch/tris.md2",
/* icon */	"w_glauncher",
/* pickup */	"Grenade Launcher",
/* tag */       II_GRENADE_LAUNCHER,
/* quant */     0,
/* ammoTag */	II_FRAG_GRENADES,
		IT_WEAPON | IT_SLING,
		30,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		"models/a_grenades/tris.md2",
		NULL,
/* icon */      "a_grenades",
/* pickup */	"Grenades",
/* tag */       II_FRAG_GRENADES,
/* quant */     2,
/* ammoTag */	0,
		IT_AMMO,
		6,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED weapon_arifle (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_arifle",
		"models/g_arifle/tris.md2",
		"models/v_arifle//tris.md2",
/* icon */	"w_arifle",
/* pickup */	"Assault RIfle",
/* tag */       II_ASSAULT_RIFLE,
/* quant */     55,
/* ammoTag */	II_ARIFLE_CLIP,
		IT_WEAPON | IT_SLING,
		36,
/* precache */ "weapons/arifle_clipin.wav weapons/arifle_clipout.wav weapons/shotcock.wav weapons/hyprbf1a.wav "
	},

/*QUAKED ammo_arifleclip (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_arifleclip",
		"models/a_arifle/tris.md2",
		NULL,
/* icon */      "a_arifle",
/* pickup */	"Assault Rifle Clip",
/* tag */       II_ARIFLE_CLIP,
/* quant */     55,
/* ammoTag */	0,
		IT_AMMO | IT_CLIP,
		12,
/* precache */ ""
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun", 
		"models/g_rail/tris.md2",
		"models/weapons/v_rail/tris.md2",
/* icon */	"w_railgun",
/* pickup */	"Railgun",
/* tag */       II_RAILGUN,
/* quant */     0,
/* ammoTag */	II_SLUGS,
		IT_WEAPON | IT_SLING,
		45,
/* precache */ "weapons/rg_hum.wav"
	},

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_slugs",
		"models/items/ammo/slugs/medium/tris.md2",
		NULL,
/* icon */      "a_slugs",
/* pickup */	"Slugs",
/* tag */       II_SLUGS,
/* quant */     5,
/* ammoTag */	0,
		IT_AMMO,
		12,
/* precache */ ""
	},

/*QUAKED ammo_expshotgunclip (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_expshotgunclip",
		"models/a_shotgun/tris.md2",
		NULL,
/* icon */      "a_expshotgun",
/* pickup */	"Explosive Shell Shotgun Clip",
/* tag */       II_EXP_SHOTGUN_CLIP,
/* quant */     18,
/* ammoTag */	0,
		IT_AMMO | IT_CLIP,
		12,
/* precache */ ""
	},

/*QUAKED ammo_solidshotgunclip (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_solidshotgunclip",
		"models/a_shotgun/tris.md2",
		NULL,
/* icon */      "a_solidshotgun",
/* pickup */	"Solid Shell Shotgun Clip",
/* tag */       II_SOLID_SHOTGUN_CLIP,
/* quant */     18,
/* ammoTag */	0,
		IT_AMMO | IT_CLIP,
		12,
/* precache */ ""
	},

/*QUAKED ammo_hepgrenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_hepgrenades",
		"models/a_grenades/tris.md2",
		NULL,
/* icon */      "a_hepgrenades",
/* pickup */	"Hep Grenades",
/* tag */       II_HEP_GRENADES,
/* quant */     2,
/* ammoTag */	0,
		IT_AMMO,
		6,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED ammo_empgrenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_empgrenades",
		"models/a_grenades/tris.md2",
		NULL,
/* icon */      "a_empgrenades",
/* pickup */	"EMP Grenades",
/* tag */       II_EMP_GRENADES,
/* quant */     2,
/* ammoTag */	0,
		IT_AMMO,
		6,
/* precache */ "weapons/bfg__x1b.wav weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED ammo_hvsubmachclip (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_hvsubmachclip",
		"models/a_submach/tris.md2",
		NULL,
/* icon */      "a_hvsubmach",
/* pickup */	"High-Velocity Sub-Mach Clip",
/* tag */       II_HV_SUBMACH_CLIP,
/* quant */     30,
/* ammoTag */	0,
		IT_AMMO | IT_CLIP,
		6,
/* precache */ ""
	},

// armor
/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_jacket", 
		"models/items/armor/jacket/tris.md2",
		NULL,
/* icon */      "i_jacketarmor",
/* pickup */	"Jacket Armor",
/* tag */       II_JACKET_ARMOUR,
/* quant */     25,
/* ammoTag */	II_JACKET_ARMOUR,
		IT_WEAPON | IT_ARMOR,
		6,
/* precache */ "misc/ar1_pkup.wav "
	},

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_combat", 
		"models/items/armor/combat/tris.md2",
		NULL,
/* icon */      "i_combatarmor",
/* pickup */	"Combat Armor",
/* tag */       II_COMBAT_ARMOUR,
/* quant */     50,
/* ammoTag */	II_COMBAT_ARMOUR,
		IT_WEAPON | IT_ARMOR,
		6,
/* precache */ "misc/ar1_pkup.wav "
	},

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_body", 
		"models/items/armor/body/tris.md2",
		NULL,
/* icon */      "i_bodyarmor",
/* pickup */	"Body Armor",
/* tag */       II_BODY_ARMOUR,
/* quant */     100,
/* ammoTag */	II_BODY_ARMOUR,
		IT_WEAPON | IT_ARMOR,
		6,
/* precache */ "misc/ar1_pkup.wav "
	},

// health items
/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_health",
		"models/items/healing/medium/tris.md2",
		NULL,
/* icon */      "i_health",
/* pickup */	"Bind-kit",
/* tag */       II_HEALTH,
/* quant */     1,
/* ammoTag */	II_HEALTH,
		IT_WEAPON | IT_AMMO | IT_CLIP,
		6,
/* precache */ "items/n_health.wav "
	},

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_health_large",
		"models/items/healing/large/tris.md2",
		NULL,
/* icon */      "i_health_large",
/* pickup */	"Medi-kit",
/* tag */       II_HEALTH_LARGE,
/* quant */     1,
/* ammoTag */	II_HEALTH_LARGE,
		IT_WEAPON | IT_AMMO | IT_CLIP,
		6,
/* precache */ "items/l_health.wav "
	},

/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_bandolier",
		"models/items/band/tris.md2", 
		NULL,
/* icon */      "p_bandolier",
/* pickup */	"Bandolier",
/* tag */       II_BANDOLIER,
/* quant */     1,
/* ammoTag */	II_BANDOLIER,
		IT_ARMOR,
		-1,
/* precache */ "items/pkup.wav "
	},

/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_pack",
		"models/items/pack/tris.md2",
		NULL,
/* icon */      "i_pack",
/* pickup */	"Back Pack",
/* tag */       II_BACK_PACK,
/* quant */     1,
/* ammoTag */	II_BACK_PACK,
		IT_ARMOR | IT_SLING,
		-1,
/* precache */ "items/pkup.wav "
	},

// GRIM - not a real weapon - simply gives info on item pos etc
/*QUAKED weapon_edit (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_edit", 
		"models/weapons/g_blast/tris.md2",
		"models/weapons/v_blast/tris.md2",
/* icon */	"w_blaster",
/* pickup */	"WEAPON EDIT - GET INFO ON STUFF",
/* tag */       II_WEAPON_EDIT,
/* quant */     0,
/* ammoTag */	0,
		IT_WEAPON | IT_SLING,
		1,
/* precache */ ""
	},

// KEYS
/*QUAKED key_data_cd (0 .5 .8) (-16 -16 -16) (16 16 16)
key for computer centers
*/
	{
		"key_data_cd",
		"models/items/keys/data_cd/tris.md2",
		NULL,
/* icon */      "k_datacd",
/* pickup */	"Data CD",
/* tag */       II_DATA_CD,
/* quant */     1,
/* ammoTag */	0,
		IT_WEAPON | IT_AMMO | IT_CLIP | IT_KEY,
		6,
/* precache */ ""
	},

/*QUAKED key_power_cube (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_power_cube",
		"models/items/keys/power/tris.md2",
		NULL,
/* icon */      "k_powercube",
/* pickup */	"Power Cube",
/* tag */       II_POWER_CUBE,
/* quant */     1,
/* ammoTag */	0,
		IT_WEAPON | IT_AMMO | IT_CLIP | IT_KEY,
		6,
/* precache */ ""
	},

/*QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_pyramid",
		"models/items/keys/pyramid/tris.md2",
		NULL,
/* icon */      "k_powercube",
/* pickup */	"Pyramid Key",
/* tag */       II_PYRAMID_KEY,
/* quant */     1,
/* ammoTag */	0,
		IT_WEAPON | IT_AMMO | IT_CLIP | IT_KEY,
		6,
/* precache */ ""
	},

/*QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_data_spinner",
		"models/items/keys/spinner/tris.md2",
		NULL,
/* icon */      "k_dataspin",
/* pickup */	"Data Spinner",
/* tag */       II_DATA_SPINNER,
/* quant */     1,
/* ammoTag */	0,
		IT_WEAPON | IT_AMMO | IT_CLIP | IT_KEY,
		6,
/* precache */ ""
	},

/*QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_pass",
		"models/items/keys/pass/tris.md2",
		NULL,
/* icon */      "k_security",
/* pickup */	"Security Pass",
/* tag */       II_SECURITY_PASS,
/* quant */     1,
/* ammoTag */	0,
		IT_WEAPON | IT_AMMO | IT_CLIP | IT_KEY,
		6,
/* precache */ ""
	},

/*QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_blue_key",
		"models/items/keys/key/tris.md2",
		NULL,
/* icon */      "k_bluekey",
/* pickup */	"Blue Key",
/* tag */       II_BLUE_KEY,
/* quant */     1,
/* ammoTag */	0,
		IT_WEAPON | IT_AMMO | IT_CLIP | IT_KEY,
		6,
/* precache */ ""
	},

/*QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_red_key",
		"models/items/keys/red_key/tris.md2",
		NULL,
/* icon */      "k_redkey",
/* pickup */	"Red Key",
/* tag */       II_RED_KEY,
/* quant */     1,
/* ammoTag */	0,
		IT_WEAPON | IT_AMMO | IT_CLIP | IT_KEY,
		6,
/* precache */ ""
	},

/*QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_commander_head",
		"models/monsters/commandr/head/tris.md2",
		NULL,
/* icon */      "k_comhead",
/* pickup */	"Commander's Head",
/* tag */       II_COM_HEAD,
/* quant */     1,
/* ammoTag */	0,
		IT_WEAPON | IT_AMMO | IT_CLIP | IT_KEY,
		6,
/* precache */ ""
	},

/*QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"key_airstrike_target",
		"models/items/keys/target/tris.md2",
		NULL,
/* icon */      "i_airstrike",
/* pickup */	"Airstrike Marker",
/* tag */       II_AIRSTRIKE_MARKER,
/* quant */     1,
/* ammoTag */	0,
		IT_WEAPON | IT_AMMO | IT_CLIP | IT_KEY,
		6,
/* precache */ ""
	},

/*QUAKED weapon_stroggblaster(.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_stroggblaster",
		"models/g_sblaster/tris.md2",
		"models/v_sblaster/tris.md2",
/* icon */      "w_sblaster",
/* pickup */	"Strogg-Blaster",
/* tag */       II_STROGG_BLASTER,
/* quant */     10,
/* ammoTag */	0,
		IT_WEAPON,
		10,
/* precache */ "models/objects/laser/tris.md2 misc/lasfly.wav soldier/solatck2.wav"
	},

/*QUAKED weapon_stroggshotgun(.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_stroggshotgun",
		"models/g_sshotgun/tris.md2",
		"models/v_sshotgun/tris.md2",
/* icon */      "w_sshot",
/* pickup */	"Strogg-Shotgun",
/* tag */       II_STROGG_SHOTGUN,
/* quant */     6,
/* ammoTag */	0,
		IT_WEAPON,
		10,
/* precache */ "soldier/solatck1.wav"
	},

/*QUAKED weapon_stroggsubmach(.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_stroggsubmach",
		"models/g_ssubmach/tris.md2",
		"models/v_ssubmach/tris.md2",
/* icon */      "w_ssubmach",
/* pickup */	"Strogg-SubMachinegun",
/* tag */       II_STROGG_SUBMACH,
/* quant */     20,
/* ammoTag */	0,
		IT_WEAPON,
		10,
/* precache */ "soldier/solatck3.wav"
	},

/*QUAKED weapon_infchain(.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_infchain",
		"models/g_inf/tris.md2",
		"models/v_inf/tris.md2",
/* icon */      "w_inf",
/* pickup */	"Infantry-Chaingun",
/* tag */       II_INF_CHAINGUN,
/* quant */     20,
/* ammoTag */	0,
		IT_WEAPON,
		10,
/* precache */ "infantry/infatck1.wav"
	},

/*QUAKED weapon_gunchain(.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_gunchain",
		"models/g_gunner/tris.md2",
		"models/v_gunner/tris.md2",
/* icon */      "w_gunchain",
/* pickup */	"Gunner-Chaingun",
/* tag */       II_GUN_CHAINGUN,
/* quant */     40,
/* ammoTag */	0,
		IT_WEAPON,
		10,
/* precache */ "gunner/gunatck1.wav gunner/gunatck2.wav"
	},

/*QUAKED weapon_medichyper(.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_medichyper",
		"models/g_medic/tris.md2",
		"models/v_medic/tris.md2",
/* icon */      "w_medicgun",
/* pickup */	"Medic Hyper-Blaster",
/* tag */       II_MEDIC_HYPER_BLASTER,
/* quant */     20,
/* ammoTag */	0,
		IT_WEAPON,
		10,
/* precache */ "medic/medatck1.wav"
	},

/*QUAKED weapon_bitchrlauncher(.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bitchrlauncher",
		"models/g_bitch/tris.md2",
		"models/v_bitch/tris.md2",
/* icon */      "w_bitch",
/* pickup */	"Bitch Rocket-Launcher",
/* tag */       II_BITCH_ROCKET_LAUNCHER,
/* quant */     8,
/* ammoTag */	0,
		IT_WEAPON | IT_SLING,
		35,
/* precache */ "chick/chkatck2.wav"
	},

/*QUAKED weapon_tankrlauncher(.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_tankrlauncher",
		"models/g_tank/tris.md2",
		"models/v_tank/tris.md2",
/* icon */      "w_tank",
/* pickup */	"Tank Rocket-Launcher",
/* tag */       II_TANK_ROCKET_LAUNCHER,
/* quant */     12,
/* ammoTag */	0,
		IT_WEAPON | IT_SLING,
		45,
/* precache */ "tank/tnkatck1.wav"
	},

	// end of list marker
	{NULL}
};

/*
===============
SetItemNames

Called by worldspawn
===============
*/
void SetItemNames (void)
{
	int		i;
	gitem_t	*it;

	for (i=0 ; i<game.num_items ; i++)
	{
		it = &itemlist[i];
		gi.configstring (CS_ITEMS+i, it->pickup_name);
	}
}
//======================================================================

/*
===============
GetItemByIndex
===============
*/
gitem_t	*GetItemByIndex (int index)
{
	if (index == 0 || index >= game.num_items)
		return NULL;

	return &itemlist[index];
}

/*
===============
GetItemByTag
===============
*/
gitem_t	*GetItemByTag (int tag)
{
	gitem_t	*it;

        if ((tag < II_HANDS) || (tag >= II_MAX) || (tag == II_MAX_WEAPONS))
                return NULL;
                
        for (it = itemlist + 1; it->classname; it++)
        {
                if (it->tag == tag)
			return it;
	}

	return NULL;
}


/*
===============
FindItemByClassname

===============
*/
gitem_t	*FindItemByClassname (char *classname)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i = 0 ; i < game.num_items ; i++, it++)
	{
		if (!it->classname)
			continue;
		if (!Q_stricmp(it->classname, classname))
			return it;
	}

	return NULL;
}

/*
===============
FindItem

===============
*/
gitem_t	*FindItem (char *pickup_name)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if (!it->pickup_name)
			continue;
		if (!Q_stricmp(it->pickup_name, pickup_name))
			return it;
	}

	return NULL;
}


/*
===============
CountItemByTag

===============
*/
int CountItemByTag(edict_t *ent, int index)
{
	int count, i;
	
	count = 0;
	
	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        {
        	if (ent->client->pers.item_bodyareas[i] == index)
        		count++;
	}
	return count;
}

//======================================================================

/*
================
Use_Item
================
*/
void Use_Item (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->svflags &= ~SVF_NOCLIENT;
	ent->use = NULL;

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->touch = Touch_Item;
	}

	gi.linkentity (ent);
}

/*
================
droptofloor
================
*/
void droptofloor (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		if (!sv_edit->value)
		{
			G_FreeEdict (ent);
			return;
		}
	}

	VectorCopy (tr.endpos, ent->s.origin);

	if (ent->team)
	{
		ent->flags &= ~FL_TEAMSLAVE;
		ent->chain = ent->teamchain;
		ent->teamchain = NULL;

		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		if (ent == ent->teammaster)
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = DoRespawn;
		}
	}

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
		ent->s.effects &= ~EF_ROTATE;
		ent->s.renderfx &= ~RF_GLOW;
	}

	if (ent->spawnflags & ITEM_TRIGGER_SPAWN)
	{
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		// GRIM 26/06/2001 5:11PM
		ent->use = Use_Item;
		// GRIM
	}

	if (sv_edit->value)
	{
		ent->solid = SOLID_BBOX;
		ent->svflags |= SVF_DEADMONSTER;
		if (ent->spawnflags & ITEM_TRIGGER_SPAWN)
		{
			ent->svflags &= ~SVF_NOCLIENT;
			ent->s.effects |= EF_BFG;
		}
	}

	gi.linkentity (ent);
}


/*
===============
PrecacheItem

Precaches all data needed for a given item.
This will be called for each item spawned in a level,
and for each item in each client's inventory.
===============
*/
void PrecacheItem (gitem_t *it)
{
	char	*s, *start;
	char	data[MAX_QPATH];
	int     len;
	gitem_t	*ammo;

	if (!it)
		return;

	/*if (it->pickup_sound)
		gi.soundindex (it->pickup_sound);*/
	if (it->world_model)
		gi.modelindex (it->world_model);
	if (it->view_model)
		gi.modelindex (it->view_model);
	if (it->icon)
		gi.imageindex (it->icon);

	// parse everything for its ammo
	if (it->ammoTag > 0)
	{
		ammo = GetItemByTag(it->ammoTag);
		if (ammo != it)
			PrecacheItem (ammo);
	}

	// parse the space seperated precache string for other items
	s = it->precaches;
	if (!s || !s[0])
		return;

	while (*s)
	{
		start = s;
		while (*s && *s != ' ')
			s++;

		len = s-start;
		if (len >= MAX_QPATH || len < 5)
			gi.error ("PrecacheItem: %s has bad precache string", it->classname);
		memcpy (data, start, len);
		data[len] = 0;
		if (*s)
			s++;

		// determine type based on extension
		if (!strcmp(data+len-3, "md2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "sp2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "wav"))
			gi.soundindex (data);
		if (!strcmp(data+len-3, "pcx"))
			gi.imageindex (data);
	}
}

/*
============
SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void SpawnItem (edict_t *ent, gitem_t *item)
{
	PrecacheItem (item);

	ent->item = item;
	ent->count = item->quantity;    // dropped items set this too
	ent->viewheight = 0;            // dropped item set this too
	                                // it's item flags (ie. silenced, empty etc)

	// HACK for hand grenades
	if ((item->tag == II_FRAG_HANDGRENADE) || (item->tag == II_EMP_HANDGRENADE))
		ent->count = 1;

	// special handling for assault rifle (which has alt-ammo)
	if (ent->item->tag == II_ASSAULT_RIFLE)
	{
		ent->viewheight |= (SHF_EXTRA_AMMO1 | SHF_EXTRA_AMMO2 | SHF_EXTRA_AMMO3 | SHF_EXTRA_AMMO4);
	}

	ent->last_fire = item->ammoTag;
	
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = 0; // item->world_model_flags;
	ent->s.renderfx = RF_GLOW;

	if (ent->model)
		gi.modelindex (ent->model);

	if (sv_edit->value)
		ent->think(ent);
}

void InitItems (void)
{
	game.num_items = sizeof(itemlist)/sizeof(itemlist[0]) - 1;
}


//======================================================================

void DoRespawn (edict_t *ent)
{
	if (ent->team)
	{
		edict_t	*master;
		int	count;
		int choice;

		master = ent->teammaster;

		for (count = 0, ent = master; ent; ent = ent->chain, count++)
			;

		choice = rand() % count;

		for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
			;
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->count = ent->item->quantity;    // reset
	// HACK for hand grenades
	if ((ent->item->tag == II_FRAG_HANDGRENADE) || (ent->item->tag == II_EMP_HANDGRENADE))
		ent->count = 1;
	gi.linkentity (ent);

	// send an effect
	ent->s.event = EV_ITEM_RESPAWN;
}

void SetRespawn (edict_t *ent, float delay)
{
	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoRespawn;
	gi.linkentity (ent);
}

//======================================================================

void RemoveItem (edict_t *player, int bodyarea)
{
        player->client->pers.item_bodyareas[bodyarea] = 0;
        player->client->pers.item_quantities[bodyarea] = 0;
        player->client->pers.item_flags[bodyarea] = 0;
        player->client->pers.item_ammotypes[bodyarea] = 0;
}

void StashItem (edict_t *player, gitem_t *item, int bodyarea, int quantity, int flags, int ammotype)
{
        player->client->pers.item_bodyareas[bodyarea] = item->tag;
        /*if (item->tag == II_KNIFE) // special handling
                player->item_quantities[bodyarea] += quantity;
        else*/
                player->client->pers.item_quantities[bodyarea] = quantity;
        player->client->pers.item_flags[bodyarea] = flags;
        player->client->pers.item_ammotypes[bodyarea] = ammotype;
}


/*
===============
GetFreeBodyArea
===============
*/

int GetFreeBodyArea (gitem_t *item, edict_t *ent)
{
        gitem_t *stashed;
        int i;
        
        if (item->size == 0)
                return 255; // items that can go anywhere

        if (item->flags & IT_ARMOR)
        {
	        if ((ent->client->pers.cstats[CSTAT_RIGHTHAND] != II_HANDS) || (ent->client->pers.cstats[CSTAT_LEFTHAND] != II_HANDS))
	        {
			gi.cprintf (ent, PRINT_HIGH, "Your hands must be free to pickup this item\n");
	        	return -1;
		}
	}

	if (ent->client->pers.item_quantities[BA_BANDOLIER] > 0)
        {
        	if ((item->flags & IT_AMMO) && (item->size <= 15) && (item->size >= 4))
        	{
        		for (i = 0; i < 6; i++)
        		{
                		if (ent->client->pers.item_bodyareas[BA_BANDOLIER_SLOT1 + i] == 0)
                		{
					//gi.dprintf("GetFreeBodyArea - BA_BANDOLIER > 0, using %i\n", BA_BANDOLIER_SLOT1 + i);
                        		return BA_BANDOLIER_SLOT1 + i;
				}
			}
		}
		//gi.dprintf("GetFreeBodyArea - BA_BANDOLIER > 0, but not using it\n");
	}
	
	// HACK - I should really re-do this in a way that doesn't suck =[
	if ((ent->client->pers.item_quantities[BA_OVER_BACK] > 0) && (ent->client->pers.item_bodyareas[BA_OVER_BACK] == II_BACK_PACK))
        {
        	gitem_t *check;
        	int bigItem = 0;
        	int n = 10;
        	int used = 0;

	        //gi.dprintf("GetFreeBodyArea - HAS BACK PACK\n");
        	
        	for (i = 0; i < 10; i++)
		{
			if (ent->client->pers.item_bodyareas[BA_BACK_PACK_SLOT1 + i] > 0)
			{
				check = GetItemByTag(ent->client->pers.item_bodyareas[BA_BACK_PACK_SLOT1 + i]);
				if (check && (check->size >= 30))
					n -= 4;
				used++;
			}

		}
	
		if ((n <= used) || ((item->size >= 30) && ((n - used) < 4)))
		{
			//if (n < 10)
		        	//gi.dprintf("have too many big weapons in back pack\n");
			//else
		        	//gi.dprintf("back pack full\n");
		}
		else
		{
        		for (i = 0; i < n; i++)
			{
                		if (ent->client->pers.item_bodyareas[BA_BACK_PACK_SLOT1 + i] == 0)
                		{
					//gi.dprintf("using %i\n", BA_BACK_PACK_SLOT1 + i);
	                		return BA_BACK_PACK_SLOT1 + i;
				}
			}
		}
	        //gi.dprintf("GetFreeBodyArea - HAS BACK PACK, but aint gonna use it\n");
	}

        // Check for SLING
        if (item->flags & IT_SLING)
        {
	        //gi.dprintf("GetFreeBodyArea : item %s has IT_SLING\n", item->classname);
                if (item->size <= 30)
                {
		        //gi.dprintf("GetFreeBodyArea : item %s itemsize < 30\n", item->classname);
                        if (ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER] != 0)
                        {
                        	if ((ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER] != 0)
                                 || (ent->client->pers.item_bodyareas[BA_OVER_BACK] != 0)
                                 || (ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER_CLIPA] != 0)
                                  || (ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER_CLIPB] != 0))
                                        return -1;

			        //gi.dprintf("GetFreeBodyArea : item %s can go under RIGHT SHOULDER\n", item->classname);
                                return BA_RIGHT_SHOULDER;
                        }

                        if ((ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER_CLIPA] == 0) && (ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER_CLIPB] == 0))
                        {
				//gi.dprintf("GetFreeBodyArea : item %s can go under LEFT SHOULDER\n", item->classname);
	                        return BA_LEFT_SHOULDER;
			}
                }

                if (ent->client->pers.item_bodyareas[BA_OVER_BACK] == 0)
                {
                        int i;
                        
                        i = 0;
                        if (ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER] != 0)
                                i++;
                        if (ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER] != 0)
                                i++;
                        if (i > 1)
                                return -1;

			//gi.dprintf("GetFreeBodyArea : item %s can go OVER BACK\n", item->classname);
                                
                        return BA_OVER_BACK;
                }
        }

        if ((item->flags & IT_CLIP) && (item->size <= 15) && (item->size >= 8))
        {
                if ((ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER_CLIPA] == 0) || (ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER_CLIPB] == 0))
                {
                        stashed = GetItemByTag(ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER]);
                        
                        if ((stashed == NULL) || (!(stashed->flags & IT_SLING)))
                        {
                                if (ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER_CLIPA] == 0)
                                        return BA_LEFT_SHOULDER_CLIPA;

                                if (ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER_CLIPB] == 0)
                                        return BA_LEFT_SHOULDER_CLIPB;
                        }
                }

                if ((ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER_CLIPA] == 0) || (ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER_CLIPB] == 0))
                {
                        stashed = GetItemByTag(ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER]);
                        
                        if ((stashed == NULL) || (!(stashed->flags & IT_SLING)))
                        {
                                if (ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER_CLIPA] == 0)
                                        return BA_RIGHT_SHOULDER_CLIPA;

                                if (ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER_CLIPB] == 0)
                                        return BA_RIGHT_SHOULDER_CLIPB;
                        }
                }
        }

        if (item->size <= 15)
        {
                if (ent->client->pers.item_bodyareas[BA_PANTS_FL] == 0)
                        return BA_PANTS_FL;
                if (ent->client->pers.item_bodyareas[BA_PANTS_FR] == 0)
                        return BA_PANTS_FR;
                if (ent->client->pers.item_bodyareas[BA_PANTS_BL] == 0)
                        return BA_PANTS_BL;
                if (ent->client->pers.item_bodyareas[BA_PANTS_BR] == 0)
                        return BA_PANTS_BR;

                if (item->flags & IT_WEAPON)
                {
                        if (ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER] == 0)
                                return BA_LEFT_SHOULDER;

                        if (ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER] == 0)
                                return BA_RIGHT_SHOULDER;
                }
        }

        if ((item->flags & IT_CLIP) && (item->size <= 15) && (item->size >= 4))
        {
                if ((ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER_CLIPA] == 0) || (ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER_CLIPB] == 0))
                {
                        stashed = GetItemByTag(ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER]);
                        
                        if ((stashed == NULL) || (!(stashed->flags & IT_SLING)))
                        {
                                if (ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER_CLIPA] == 0)
                                        return BA_LEFT_SHOULDER_CLIPA;

                                if (ent->client->pers.item_bodyareas[BA_LEFT_SHOULDER_CLIPB] == 0)
                                        return BA_LEFT_SHOULDER_CLIPB;
                        }
                }

                if ((ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER_CLIPA] == 0) || (ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER_CLIPB] == 0))
                {
                        stashed = GetItemByTag(ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER]);
                        
                        if ((stashed == NULL) || (!(stashed->flags & IT_SLING)))
                        {
                                if (ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER_CLIPA] == 0)
                                        return BA_RIGHT_SHOULDER_CLIPA;

                                if (ent->client->pers.item_bodyareas[BA_RIGHT_SHOULDER_CLIPB] == 0)
                                        return BA_RIGHT_SHOULDER_CLIPB;
                        }
                }
        }

        return -1; // no place with enough space
}

/*
===============
Pickup_BAItem
===============
*/
qboolean Pickup_LooseAmmo(edict_t *ent, edict_t *other)
{
	int i, calc;
	int count = ent->count;

	if (ent->item->flags & IT_CLIP)
		return false;
			
        if (!(ent->item->flags & IT_AMMO))
		return false;

	for (i = BA_LEG_ARMOUR; i < BA_MAX; i++)
        {
        	if (other->client->pers.item_bodyareas[i] == ent->item->tag)
        	{
			if (other->client->pers.item_quantities[i] < ent->item->quantity)
			{
				calc = ent->item->quantity - other->client->pers.item_quantities[i];
				if (count < calc)
				{
					other->client->pers.item_quantities[i] += count;
					count = 0;
				}
				else
				{
					other->client->pers.item_quantities[i] = ent->item->quantity;
					count -= calc;
				}
				
				
				if (count < 1)			
					break;
			}
		}
	}

	if (count == 0)
		return true;
	
	if (ent->count != count)
	{	
		ent->count = count;

		// set it up to reset (if it should)
		if (deathmatch->value)
		{
			ent->nextthink = level.time + 12;
			ent->think = DoRespawn;
			gi.linkentity (ent);
		}
	}
	
	return false;
}

qboolean Pickup_BAItem (gitem_t *item, int count, int flags, int ammotype, edict_t *other)
{
        int bodyarea;

        bodyarea = -1;
        if (((item->flags & IT_ARMOR) || (item->flags & IT_WEAPON)) && (other->client->pers.cstats[CSTAT_RIGHTHAND] == II_HANDS) && (other->client->pers.cstats[CSTAT_LEFTHAND]== II_HANDS))
                bodyarea = -2;
        else
                bodyarea = GetFreeBodyArea(item, other);

        if (bodyarea == -2)
        {       // lefthandedness simply flips the view model... can't change that, so forget about it
                other->client->pers.cstats[CSTAT_RIGHTHAND] = item->tag;
                other->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = count;
                other->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] = flags;
                other->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE] = ammotype;
                other->client->ps.gunframe = 0;
                other->client->weaponstate = W_RIGHT_RAISING;
                SetupItemModels(other);
                //gi.dprintf("Pickup_BAItem : putting item straight into hand\n");
                return true;
        }
        else if (bodyarea > -1)
        {
                StashItem (other, item, bodyarea, count, flags, ammotype);
                //gi.dprintf("Pickup_BAItem : stashing item in ba %i\n", bodyarea);
                return true;
        }

        //gi.dprintf("Pickup_BAItem : no place to stash item %s\n", item->classname);

        return false;
}


/*
===============
Touch_Item
===============
*/
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//qboolean	taken;

	if (!other->client)
		return;

	if (other->health < 1)
		return;		// dead people can't pickup

        if (!other->client->action)
                return;

	if (ent->item->tag == II_CHAINGUN)
	{
                //gi.dprintf("Touch_Item -> chaingun\n");
                if (other->client->pers.item_bodyareas[BA_OVER_BACK] != 0)
                {
	                //gi.dprintf("back not free\n");
                	return;
		}

		if ((other->client->pers.cstats[CSTAT_RIGHTHAND] != II_HANDS) || (other->client->pers.cstats[CSTAT_LEFTHAND] != II_HANDS))
		{
			if ((other->client->pers.item_bodyareas[BA_RIGHT_SHOULDER] != 0)
			 && (other->client->pers.item_bodyareas[BA_LEFT_SHOULDER] != 0))
        	        {
	                	//gi.dprintf("no shoulder or hands free\n");
                		return;
			}
		}
	
		if (!Pickup_BAItem(GetItemByTag(II_CHAINGUN_PACK), ent->count, 0, II_CHAINGUN_PACK, other))
		{
			//gi.dprintf("could not pickup up chaigun pack\n");
			return;
		}

		if (!Pickup_BAItem (ent->item, 0, ent->viewheight, ent->last_fire, other))
			return;

		//gi.dprintf("Picked up chaingun and chaigun pack\n");
	}
        else if (!Pickup_BAItem (ent->item, ent->count, ent->viewheight, ent->last_fire, other))
        {
		if (!Pickup_LooseAmmo(ent, other))
			return;
	}
	
        if (level.time - other->client->cycleItems <= CYCLE_ITEMS_TIME)
		UpdateInv(other);

	// flash the screen
        other->client->bonus_alpha = 0.25;	

	// show icon and name on status bar
	other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
	other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + ITEM_INDEX(ent->item);
	other->client->pickup_msg_time = level.time + 3.0;

	if (!(ent->spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets (ent, other);
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

	if (ent->spawnflags & DROPPED_ITEM)
	{
		if (ent->flags & FL_RESPAWN)
			ent->flags &= ~FL_RESPAWN;
		else if (ent->activator && ((ent->item->tag == II_BANDOLIER) || (ent->item->tag == II_BACK_PACK)))
		{
		        //gi.dprintf("touch_item -> ent->activator & is storage item\n");
			ent->svflags |= SVF_NOCLIENT;
			ent->solid = SOLID_NOT;
			ent->nextthink = 0;
			ent->think = NULL;
			gi.linkentity (ent);
			other->storage_item = ent;
		}
		else
			G_FreeEdict (ent);
	}
	else if (deathmatch->value)
	{
	        SetRespawn (ent, 12);
	        //SetRespawn (ent, 4);
	        //SetRespawn (ent, 30);
	}
	else
		G_FreeEdict (ent);
}

//======================================================================

void GiveItem(edict_t *ent, int tag)
{
        gitem_t *item;

        item = GetItemByTag(tag);

        if (item)
        {
                //gi.dprintf("GiveItem : found item %s\n", item->classname);
                Pickup_BAItem(item, item->quantity, 0, item->ammoTag, ent);
        }
        //else
                //gi.dprintf("GiveItem : did not find item with tag %i\n", tag);
}


//======================================================================


/*
================
LaunchItem

Spawns an item and tosses it forward
================
*/
void DroppedThink( edict_t *ent ) 
{
        level.dropped_item_count--;
        if (level.dropped_item_count < 0)
                level.dropped_item_count = 0;
           
	if (deathmatch->value) // only remove if multiplayer deathmatch
		G_FreeEdict(ent);
}

// Checks if a key ended up in lava or slime
// if so, it will respawn the key at the last rally point
// or failing that, back at the start of the level
void DroppedKeyCheck(edict_t *key)
{
	edict_t	*ent, *point = NULL;
	int player, contents;
	vec3_t origin;

	if (!key->groundentity)
	{
		key->nextthink = level.time + 1.0;
		return;
	}

	contents = gi.pointcontents (key->s.origin);

	if (!(contents & (CONTENTS_LAVA|CONTENTS_SLIME)))
	{
		key->think = DroppedThink;
		key->nextthink = level.time + 0.1;
		return;
	}

	for (player = 1; player <= game.maxclients; player++)
	{
		ent = &g_edicts[player];
		if (!ent->inuse)
			continue;
		if (!ent->client)
			continue;

		// check for last touched rally point
		if (ent->goalentity != NULL)
		{
			if (point == NULL)
				point = ent->goalentity;
			else if (point->last_fire < ent->goalentity->last_fire)
				point = ent->goalentity;
		}
	}

	if (!point)
	{
		while ((point = G_Find (point, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !point->targetname)
				break;

			if (!game.spawnpoint[0] || !point->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, point->targetname) == 0)
				break;
		}

		if (!point)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				point = G_Find (point, FOFS(classname), "info_player_start");
			}
			if (!point)
				gi.error ("DroppedKeyCheck : Couldn't find spawn point %s\n", game.spawnpoint);
		}

		gi.bprintf(PRINT_HIGH, "The %s has re-appeared at the last entry point to this level\n", key->item->pickup_name);
	}
	else
		gi.bprintf(PRINT_HIGH, "The %s has re-appeared back at the last rally point\n", key->item->pickup_name);
	
	if (point)
	{
		VectorCopy (point->s.origin, origin);
		origin[0] += 32 * crandom() * 1;
		origin[1] += 32 * crandom() * 1;
		VectorCopy (origin, key->s.origin);
		droptofloor(key);
	}
}

edict_t *GetDroppedItem (int num)
{
        edict_t *ent;
        int     i;

	ent = &g_edicts[0];
	for (i = 0; i < globals.num_edicts ; i++, ent++)
	{
		if ( !ent->inuse ) 
			continue;

                if (ent->item && (ent->flags & DROPPED_ITEM) && (ent->dmg == num))
                        return ent;
        }

        return NULL;
}

void DropBandolier (edict_t *ent, edict_t *self)
{
	edict_t *dropped = NULL;
	int i;

	//gi.dprintf ("DropBandolier\n");
	
	for (i = 0; i < 6; i++)
	{
		if (ent->client->pers.item_bodyareas[BA_BANDOLIER_SLOT1 + i] > 0)
		{
			dropped = ThrowBodyAreaItem(ent, 1, BA_BANDOLIER_SLOT1 + i);
			if (dropped)
			{
				dropped->last_hitloc = BA_BANDOLIER_SLOT1 + i;
				dropped->activator = self->activator;
				dropped->flags |= FL_RESPAWN;
				dropped->svflags |= SVF_NOCLIENT;
				dropped->solid = SOLID_NOT;
				dropped->nextthink = 0;
				dropped->think = NULL;
				gi.linkentity (dropped);
				self->activator = dropped;
			}
		}
	}
}

void DropBackPack (edict_t *ent, edict_t *self)
{
	edict_t *dropped = NULL;
	int i;
	
	//gi.dprintf ("DropBackPack\n");
	
	for (i = 0; i < 10; i++)
	{
		if (ent->client->pers.item_bodyareas[BA_BACK_PACK_SLOT1 + i] > 0)
		{
			//gi.dprintf ("BA_BACK_PACK_SLOT1 + %i > 0\n", i);
			dropped = ThrowBodyAreaItem(ent, 1, BA_BACK_PACK_SLOT1 + i);
			if (dropped)
			{
				//gi.dprintf ("dropped->classname = %s\n", dropped->classname);
				dropped->last_hitloc = BA_BACK_PACK_SLOT1 + i;
				dropped->activator = self->activator;
				dropped->flags |= FL_RESPAWN;
				dropped->svflags |= SVF_NOCLIENT;
				dropped->solid = SOLID_NOT;
				dropped->nextthink = 0;
				dropped->think = NULL;
				gi.linkentity (dropped);
				self->activator = dropped;
			}
		}
	}
}

edict_t *LaunchItem(edict_t *ent, gitem_t *item, vec3_t origin, vec3_t velocity ) 
{
        edict_t *dropped;

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = 0; //item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
        VectorCopy (origin, dropped->s.origin);
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;  
	dropped->touch = Touch_Item;

        VectorCopy (ent->s.angles, dropped->s.angles);
        dropped->s.angles[0] = 0;

        // Check limit here so we can't have shitloads in the air as well...
        if ((int)sv_maxdropped->value > 0)
        {
                level.dropped_item_index++;
                if (level.dropped_item_index > (int)sv_maxdropped->value)
                        level.dropped_item_index = 1;

                level.dropped_item_count++;

                if (level.dropped_item_count > (int)sv_maxdropped->value)
                {
                        edict_t *hippy_smurf = GetDroppedItem (level.dropped_item_index);
                        
                        if (hippy_smurf)
                                hippy_smurf->nextthink = level.time + 0.1;
                }

                dropped->dmg = level.dropped_item_index;
        }

	VectorCopy (velocity, dropped->velocity);

	if (item->flags & IT_KEY)
	{
        	dropped->think = DroppedKeyCheck;
		dropped->nextthink = level.time + 1.0;
	}
	else
	{
        	dropped->think = DroppedThink;
		dropped->nextthink = level.time + 30;
	}
	
	// special handling for bandolier & back pack
	if (item->tag == II_BANDOLIER)
		DropBandolier(ent, dropped);
	else if (item->tag == II_BACK_PACK)
		DropBackPack(ent, dropped);

	gi.linkentity (dropped);

	return dropped;
}


//======================================================================

/*
==============
DropClip
==============
*/
void DropClip(edict_t *ent, int index, int count)
{
	vec3_t	velocity, angles, org;
	vec3_t	forward, right, up;
	edict_t *dropped;
	gitem_t *item;
	
	item = GetItemByTag(index);
	
	if (!item)
		return;

        // calc angles
	VectorCopy(ent->client->v_angle, angles );
	angles[PITCH] = 0;      // flat
	AngleVectors( angles, velocity, NULL, NULL );
        VectorScale(velocity, 5, velocity );

        // calc position
	AngleVectors (angles, forward, right, up);
	VectorCopy(ent->s.origin, org);

        if (forward[2] < 0)
                VectorMA(org, 2 * forward[2], up, org);
        else if (forward[2] > 0)
                VectorMA(org, -2 * forward[2], up, org);

        VectorMA(org, 16, up, org);
        VectorMA(org, 12, forward, org);
        VectorMA(org, 8, right, org);

        dropped = LaunchItem(ent, item, org, velocity);
        
        if (dropped)
        {
                dropped->count = count;
                dropped->nextthink = level.time + 15;
                dropped->s.frame = 0;

		if ((item->quantity > 0) && (dropped->count > 0)) // the lower the ammo, the faster it disappears
	        	dropped->nextthink += (item->quantity / dropped->count) * 45;

		if (dropped->count < 1)
			dropped->touch = NULL; // cannot pickup empty clips

		dropped->s.renderfx &= ~RF_GLOW;
        }
}

/*
==============
ReloadHand
==============
*/
void ReloadHand (edict_t *ent, int hand)
{
        gitem_t *item, *ammoItem;
        int i, c;
        qboolean found = false;

        item = GetItemByTag(ent->client->pers.cstats[hand]);
        
        if (!item)
        	return;

	if (ent->client->pers.cstats[hand + 3] > 0)
		ammoItem = GetItemByTag(ent->client->pers.cstats[hand + 3]);
        else if (item->ammoTag)
		ammoItem = GetItemByTag(item->ammoTag);
	else
		ammoItem = NULL;

	if ((hand == CSTAT_RIGHTHAND) && (ent->client->newRightAmmoType > 0))
	{
		ammoItem = GetItemByTag(ent->client->newRightAmmoType);
		ent->client->newRightAmmoType = 0;
	}
	else if ((hand == CSTAT_LEFTHAND) && (ent->client->newLeftAmmoType > 0))
	{
		ammoItem = GetItemByTag(ent->client->newLeftAmmoType);
		ent->client->newLeftAmmoType = 0;
	}

	if (ammoItem)
        {
                c = 0;
                        
                switch (item->tag)
                {
                case II_PISTOL:
                        if (ent->client->pers.cstats[hand + 1] > 0)
                                c = 1;
                        break;

                default:
                        break;
                }

                for (i = BA_LEG_ARMOUR; i < BA_MAX; i++)
                {
                        if (ent->client->pers.item_bodyareas[i] == ammoItem->tag)
                        {
                                ent->client->pers.cstats[hand + 1] = ent->client->pers.item_quantities[i] + c;
                                ent->client->pers.cstats[hand + 3] = ammoItem->tag;
                                RemoveItem(ent, i);
                                found = true;
                                break;
                        }
                }

		if (found)
			return;
			
                for (i = BA_LEG_ARMOUR; i < BA_MAX; i++)
                {
			if (CheckAltAmmo (ent, item, ent->client->pers.item_bodyareas[i]))
                        {
                                ent->client->pers.cstats[hand + 1] = ent->client->pers.item_quantities[i] + c;
                                ent->client->pers.cstats[hand + 3] = ent->client->pers.item_bodyareas[i];
                                RemoveItem(ent, i);
                                found = true;
                                break;
                        }
                }
        }
}

//======================================================================

qboolean CheckAltAmmo (edict_t *ent, gitem_t *curweapon, int ammoTag);

/*
==============
CanRightReload

Checks if the weapon has a clip
==============
*/
qboolean CanRightReload (edict_t *ent)
{
        gitem_t *item;
        int i, c;

        item = GetItemByTag(ent->client->pers.cstats[CSTAT_RIGHTHAND]);

        if (item && item->ammoTag)
        {
                c = item->quantity;
                        
                switch (item->tag)
                {
                case II_PISTOL:
                        c++;
                        break;

                default:
                        break;
                }

                if (ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] >= c)
                        return false;

                for (i = BA_LEG_ARMOUR; i < BA_MAX; i++)
                {
                        //if (ent->client->pers.item_bodyareas[i] == item->ammoTag)
                        if (ent->client->pers.item_bodyareas[i] == ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE])
                                return true;

			if (CheckAltAmmo (ent, item, ent->client->pers.item_bodyareas[i]))
			{
			        //gi.dprintf("CanRightReload : %i passed CheckAltAmmo for %s\n", ent->client->pers.item_bodyareas[i], item->classname);
       				return true;
			}
                }
        }
        
        return false;
}

qboolean CanLeftReload (edict_t *ent)
{
        gitem_t *item;
        int i, c;

        item = GetItemByTag(ent->client->pers.cstats[CSTAT_LEFTHAND]);

        if (item && item->ammoTag)
        {
                c = item->quantity;
                        
                switch (item->tag)
                {
                case II_PISTOL:
                        c++;
                        break;

                default:
                        break;
                }

                if (ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] >= c)
                        return false;

                for (i = BA_LEG_ARMOUR; i < BA_MAX; i++)
                {
                        //if (ent->client->pers.item_bodyareas[i] == item->ammoTag)
                        if (ent->client->pers.item_bodyareas[i] == ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE])
                                return true;
                }
        }
        
        return false;
}

//======================================================================

// check for offhand grenades
void OffHandGrenades(edict_t *ent, int handstat, int ammostat, int ammopicstat)
{
	gitem_t *item;
	int count = -1;
	int tag = ent->client->pers.hgren_type;
	int alt_tag;
	
	if (ent->client->pers.hgren_type == II_FRAG_HANDGRENADE)
		alt_tag = II_EMP_HANDGRENADE;
	else
		alt_tag = II_FRAG_HANDGRENADE;
	
	if (ent->client->pers.cstats[handstat] == II_PISTOL)
	{
		count = CountOffHandGrenades(ent, tag);
		if (!count)
		{
			tag = alt_tag;
			ent->client->pers.hgren_type = alt_tag;
			count = CountOffHandGrenades(ent, alt_tag);
		}
	}
	else if (ent->client->pers.cstats[handstat] == II_SUBMACH)
	{
		count = CountOffHandGrenades(ent, tag);
		if (!count)
		{
			tag = alt_tag;
			ent->client->pers.hgren_type = alt_tag;
			count = CountOffHandGrenades(ent, alt_tag);
		}
	}
	else if (ent->client->pers.cstats[handstat] == II_SHOTGUN)
	{
		count = CountOffHandGrenades(ent, tag);
		if (!count)
		{
			tag = alt_tag;
			ent->client->pers.hgren_type = alt_tag;
			count = CountOffHandGrenades(ent, alt_tag);
		}
	}
	else if (ent->client->pers.cstats[handstat] == II_ASSAULT_RIFLE)
	{
		count = 0;
                if (ent->client->pers.cstats[handstat + 2] & SHF_EXTRA_AMMO1)
                	count++;
                if (ent->client->pers.cstats[handstat + 2] & SHF_EXTRA_AMMO2)
                	count++;
                if (ent->client->pers.cstats[handstat + 2] & SHF_EXTRA_AMMO3)
                	count++;
                if (ent->client->pers.cstats[handstat + 2] & SHF_EXTRA_AMMO4)
                	count++;
		tag = II_FRAG_GRENADES;
	}

	if (count > 0)
	{
		ent->client->ps.stats[ammostat] = count;

		item = GetItemByTag(tag);
		if (item)
			ent->client->ps.stats[ammopicstat] = gi.imageindex(item->icon);
	}
}

void UpdateExtraAmmoCounts (edict_t *ent)
{
        gitem_t *item, *ammoItem;
        int i, count;
        
        ent->client->ps.stats[STAT_LEFT_AMMO] = 0;
        ent->client->ps.stats[STAT_LEFT_EXTRA_AMMO] = 0;
	ent->client->ps.stats[STAT_LEFT_AMMO_ICON] = 0;

	item = NULL;
	ammoItem = NULL;
        if (ent->client->pers.cstats[CSTAT_LEFTHAND] != II_HANDS)
	        item = GetItemByTag(ent->client->pers.cstats[CSTAT_LEFTHAND]);
	
        if (item)
        {
                if (item->ammoTag == item->tag)
                {
                        ent->client->ps.stats[STAT_LEFT_AMMO] = 0;
        	        if (item->ammoTag > 0)
		                ammoItem = GetItemByTag(item->ammoTag);
                }
                else if (ent->flags & FL_GODMODE)
                        ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] = item->quantity;
                else if (ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] > 0)
                        ent->client->ps.stats[STAT_LEFT_AMMO] = ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO];

		if ((ammoItem == NULL) && (ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE] > 0))
			ammoItem = GetItemByTag(ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE]);
		else if (item->ammoTag > 0)
			ammoItem = GetItemByTag(item->ammoTag);
                
		if (ammoItem)
                {
                        count = 0;
                        
                        for (i = BA_LEG_ARMOUR; i < BA_MAX; i++)
                        {       
                                if (ent->client->pers.item_bodyareas[i] == ammoItem->tag)
                                {
        				if (ammoItem->flags & IT_CLIP)
                                        	count++;
					else
						count += ent->client->pers.item_quantities[i];
                                }
                        }

	                if (item->ammoTag == item->tag)
	                	count += ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO];

			if (ammoItem->flags & IT_CLIP)
                        	ent->client->ps.stats[STAT_LEFT_EXTRA_AMMO] = count;
			else
	                        ent->client->ps.stats[STAT_LEFT_AMMO] = count;

			ent->client->ps.stats[STAT_LEFT_AMMO_ICON] = gi.imageindex(ammoItem->icon);
                }
                /*else if (item->tag == II_KNIFE)// special handling
                        ent->client->ps.stats[STAT_LEFT_EXTRA_AMMO] = ent->client->pers.item_quantities[BA_LEG_ARMOUR];*/
        }
	else	// check for offhand grenades
		OffHandGrenades(ent, CSTAT_RIGHTHAND, STAT_LEFT_EXTRA_AMMO, STAT_LEFT_AMMO_ICON);

        ent->client->ps.stats[STAT_RIGHT_AMMO] = 0;
        ent->client->ps.stats[STAT_RIGHT_EXTRA_AMMO] = 0;
	ent->client->ps.stats[STAT_RIGHT_AMMO_ICON] = 0;

	item = NULL;
	ammoItem = NULL;
        if (ent->client->pers.cstats[CSTAT_RIGHTHAND] != II_HANDS)
	        item = GetItemByTag(ent->client->pers.cstats[CSTAT_RIGHTHAND]);

        if (item)
        {
                if (item->ammoTag == item->tag)
                {
                        ent->client->ps.stats[STAT_RIGHT_AMMO] = 0;
        	        if (item->ammoTag > 0)
		                ammoItem = GetItemByTag(item->ammoTag);
                }
                else if (ent->flags & FL_GODMODE)
                        ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = item->quantity;
                else if (ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] > 0)
                        ent->client->ps.stats[STAT_RIGHT_AMMO] = ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO];

		if ((ammoItem == NULL) && (ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE] > 0))
			ammoItem = GetItemByTag(ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE]);
		else if (item->ammoTag > 0)
			ammoItem = GetItemByTag(item->ammoTag);
                
		if (ammoItem)
                {
                        count = 0;
                        
                        for (i = BA_LEG_ARMOUR; i < BA_MAX; i++)
                        {       
                                if (ent->client->pers.item_bodyareas[i] == ammoItem->tag)
                                {
        				if (ammoItem->flags & IT_CLIP)
                                        	count++;
					else
						count += ent->client->pers.item_quantities[i];
                                }
                        }

	                if (item->ammoTag == item->tag)
	                	count += ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO];

			if (ammoItem->flags & IT_CLIP)
                        	ent->client->ps.stats[STAT_RIGHT_EXTRA_AMMO] = count;
			else
	                        ent->client->ps.stats[STAT_RIGHT_AMMO] = count;

			ent->client->ps.stats[STAT_RIGHT_AMMO_ICON] = gi.imageindex(ammoItem->icon);
                }
                /*else if (item->tag == II_KNIFE)// special handling
                        ent->client->ps.stats[STAT_RIGHT_EXTRA_AMMO] = ent->client->pers.item_quantities[BA_LEG_ARMOUR];*/
        }
	else	// check for offhand grenades
		OffHandGrenades(ent, CSTAT_LEFTHAND, STAT_RIGHT_EXTRA_AMMO, STAT_RIGHT_AMMO_ICON);
}
