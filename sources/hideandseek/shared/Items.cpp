#include "shared.h"

gitem_t	itemlist[] = 
{
	{
		NULL
	},	// leave index 0 alone

	//
	// ARMOR
	//

	/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_armor_body", 
			"misc/ar1_pkup.wav",
			"models/items/armor/body/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"i_bodyarmor",
			/* pickup */	"Body Armor",
			/* width */		3,
			0,
			NULL,
			IT_ARMOR|IT_GRABBABLE,
			"",
			/* precache */ ""
	},

	/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_armor_combat", 
			"misc/ar1_pkup.wav",
			"models/items/armor/combat/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"i_combatarmor",
			/* pickup */	"Combat Armor",
			/* width */		3,
			0,
			NULL,
			IT_ARMOR|IT_GRABBABLE,
			"",
			/* precache */ ""
	},

	/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_armor_jacket", 
			"misc/ar1_pkup.wav",
			"models/items/armor/jacket/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"i_jacketarmor",
			/* pickup */	"Jacket Armor",
			/* width */		3,
			0,
			NULL,
			IT_ARMOR|IT_GRABBABLE,
			"",
			/* precache */ ""
	},

	/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_armor_shard", 
			"misc/ar2_pkup.wav",
			"models/items/armor/shard/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"i_jacketarmor",
			/* pickup */	"Armor Shard",
			/* width */		3,
			0,
			NULL,
			IT_ARMOR|IT_GRABBABLE,
			"",
			/* precache */ ""
	},


	/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_power_screen", 
		"misc/ar3_pkup.wav",
		"models/items/armor/screen/tris.md2", EF_ROTATE,
		NULL,
		/* icon */		"i_powerscreen",
		/* pickup */	"Power Screen",
		/* width */		0,
		60,
		NULL,
		IT_ARMOR|IT_USABLE|IT_DROPPABLE|IT_GRABBABLE,
		"",
		/* precache */ ""
	},

	/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_power_shield",
		"misc/ar3_pkup.wav",
		"models/items/armor/shield/tris.md2", EF_ROTATE,
		NULL,
		/* icon */		"i_powershield",
		/* pickup */	"Power Shield",
		/* width */		0,
		60,
		NULL,
		IT_ARMOR|IT_USABLE|IT_DROPPABLE|IT_GRABBABLE,
		"",
		/* precache */ "misc/power2.wav misc/power1.wav"
	},


	//
	// WEAPONS 
	//

	/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
	always owned, never in the world
	*/
	{
		"weapon_blaster", 
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_blast/tris.md2",
		/* icon */		"w_blaster",
		/* pickup */	"Blaster",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_USABLE,
		"w_blaster",
		/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},

	/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"weapon_shotgun", 
		"misc/w_pkup.wav",
		"models/physics/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
		/* icon */		"w_shotgun",
		/* pickup */	"Shotgun",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
		"w_shotgun",
		/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav"
	},

	/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"weapon_supershotgun", 
			"misc/w_pkup.wav",
			"models/physics/g_shotg2/tris.md2", EF_ROTATE,
			"models/weapons/v_shotg2/tris.md2",
			/* icon */		"w_sshotgun",
			/* pickup */	"Super Shotgun",
			0,
			2,
			"Shells",
			IT_WEAPON|IT_STAY_COOP|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			"w_sshotgun",
			/* precache */ "weapons/sshotf1b.wav"
	},

	/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"weapon_machinegun", 
			"misc/w_pkup.wav",
			"models/physics/g_machn/tris.md2", EF_ROTATE,
			"models/weapons/v_machn/tris.md2",
			/* icon */		"w_machinegun",
			/* pickup */	"Machinegun",
			0,
			1,
			"Bullets",
			IT_WEAPON|IT_STAY_COOP|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			"w_machinegun",
			/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
	},

	/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"weapon_chaingun", 
			"misc/w_pkup.wav",
			"models/physics/g_chain/tris.md2", EF_ROTATE,
			"models/weapons/v_chain/tris.md2",
			/* icon */		"w_chaingun",
			/* pickup */	"Chaingun",
			0,
			1,
			"Bullets",
			IT_WEAPON|IT_STAY_COOP|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			"w_chaingun",
			/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav weapons/chngnd1a.wav"
	},

	/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"ammo_grenades",
			"misc/am_pkup.wav",
			"models/items/ammo/grenades/medium/tris.md2", 0,
			"models/weapons/v_handgr/tris.md2",
			/* icon */		"a_grenades",
			/* pickup */	"Grenades",
			/* width */		3,
			5,
			"grenades",
			IT_AMMO|IT_WEAPON|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			"a_grenades",
			/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav"
	},

	/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"weapon_grenadelauncher",
			"misc/w_pkup.wav",
			"models/physics/g_launch/tris.md2", EF_ROTATE,
			"models/weapons/v_launch/tris.md2",
			/* icon */		"w_glauncher",
			/* pickup */	"Grenade Launcher",
			0,
			1,
			"Grenades",
			IT_WEAPON|IT_STAY_COOP|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			"w_glauncher",
			/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

	/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"weapon_rocketlauncher",
			"misc/w_pkup.wav",
			"models/physics/g_rocket/tris.md2", EF_ROTATE,
			"models/weapons/v_rocket/tris.md2",
			/* icon */		"w_rlauncher",
			/* pickup */	"Rocket Launcher",
			0,
			1,
			"Rockets",
			IT_WEAPON|IT_STAY_COOP|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			"w_rlauncher",
			/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

	/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"weapon_hyperblaster", 
			"misc/w_pkup.wav",
			"models/physics/g_hyperb/tris.md2", EF_ROTATE,
			"models/weapons/v_hyperb/tris.md2",
			/* icon */		"w_hyperblaster",
			/* pickup */	"HyperBlaster",
			0,
			1,
			"Cells",
			IT_WEAPON|IT_STAY_COOP|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			"w_hyperblaster",
			/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"
	},

	/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"weapon_railgun", 
			"misc/w_pkup.wav",
			"models/physics/g_rail/tris.md2", EF_ROTATE,
			"models/weapons/v_rail/tris.md2",
			/* icon */		"w_railgun",
			/* pickup */	"Railgun",
			0,
			1,
			"Slugs",
			IT_WEAPON|IT_STAY_COOP|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			"w_railgun",
			/* precache */ "weapons/rg_hum.wav"
	},

	/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"weapon_bfg",
			"misc/w_pkup.wav",
			"models/physics/g_bfg/tris.md2", EF_ROTATE,
			"models/weapons/v_bfg/tris.md2",
			/* icon */		"w_bfg",
			/* pickup */	"BFG10K",
			0,
			50,
			"Cells",
			IT_WEAPON|IT_STAY_COOP|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			"w_bfg",
			/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav"
	},

	//
	// AMMO ITEMS
	//

	/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"ammo_shells",
			"misc/am_pkup.wav",
			"models/items/ammo/shells/medium/tris.md2", 0,
			NULL,
			/* icon */		"a_shells",
			/* pickup */	"Shells",
			/* width */		3,
			10,
			NULL,
			IT_AMMO|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"ammo_bullets",
			"misc/am_pkup.wav",
			"models/items/ammo/bullets/medium/tris.md2", 0,
			NULL,
			/* icon */		"a_bullets",
			/* pickup */	"Bullets",
			/* width */		3,
			50,
			NULL,
			IT_AMMO|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"ammo_cells",
			"misc/am_pkup.wav",
			"models/items/ammo/cells/medium/tris.md2", 0,
			NULL,
			/* icon */		"a_cells",
			/* pickup */	"Cells",
			/* width */		3,
			50,
			NULL,
			IT_AMMO|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"ammo_rockets",
			"misc/am_pkup.wav",
			"models/items/ammo/rockets/medium/tris.md2", 0,
			NULL,
			/* icon */		"a_rockets",
			/* pickup */	"Rockets",
			/* width */		3,
			5,
			NULL,
			IT_AMMO|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"ammo_slugs",
			"misc/am_pkup.wav",
			"models/items/ammo/slugs/medium/tris.md2", 0,
			NULL,
			/* icon */		"a_slugs",
			/* pickup */	"Slugs",
			/* width */		3,
			10,
			NULL,
			IT_AMMO|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},


	//
	// POWERUP ITEMS
	//
	/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_quad", 
			"items/pkup.wav",
			"models/items/quaddama/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"p_quad",
			/* pickup */	"Quad Damage",
			/* width */		2,
			60,
			NULL,
			IT_POWERUP|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav"
	},

	/*QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_invulnerability",
			"items/pkup.wav",
			"models/items/invulner/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"p_invulnerability",
			/* pickup */	"Invulnerability",
			/* width */		2,
			300,
			NULL,
			IT_POWERUP|IT_USABLE|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
	},

	/*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_silencer",
			"items/pkup.wav",
			"models/items/silencer/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"p_silencer",
			/* pickup */	"Silencer",
			/* width */		2,
			60,
			NULL,
			IT_POWERUP|IT_USABLE|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_breather",
			"items/pkup.wav",
			"models/items/breather/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"p_rebreather",
			/* pickup */	"Rebreather",
			/* width */		2,
			60,
			NULL,
			IT_STAY_COOP|IT_POWERUP|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ "items/airout.wav"
	},

	/*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_enviro",
			"items/pkup.wav",
			"models/items/enviro/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"p_envirosuit",
			/* pickup */	"Environment Suit",
			/* width */		2,
			60,
			NULL,
			IT_STAY_COOP|IT_POWERUP|IT_USABLE|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ "items/airout.wav"
	},

	/*QUAKED item_ancient_head (.3 .3 1) (-16 -16 -16) (16 16 16)
	Special item that gives +2 to maximum health
	*/
	{
		"item_ancient_head",
			"items/pkup.wav",
			"models/items/c_head/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"i_fixme",
			/* pickup */	"Ancient Head",
			/* width */		2,
			60,
			NULL,
			IT_GRABBABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16)
	gives +1 to maximum health
	*/
	{
		"item_adrenaline",
			"items/pkup.wav",
			"models/items/adrenal/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"p_adrenaline",
			/* pickup */	"Adrenaline",
			/* width */		2,
			60,
			NULL,
			IT_GRABBABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_bandolier",
			"items/pkup.wav",
			"models/items/band/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"p_bandolier",
			/* pickup */	"Bandolier",
			/* width */		2,
			60,
			NULL,
			IT_GRABBABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_pack",
			"items/pkup.wav",
			"models/items/pack/tris.md2", EF_ROTATE,
			NULL,
			/* icon */		"i_pack",
			/* pickup */	"Ammo Pack",
			/* width */		2,
			180,
			NULL,
			IT_GRABBABLE,
			0,
			/* precache */ ""
	},

	//
	// KEYS
	//
	/*QUAKED key_data_cd (0 .5 .8) (-16 -16 -16) (16 16 16)
	key for computer centers
	*/
	{
		"key_data_cd",
			"items/pkup.wav",
			"models/items/keys/data_cd/tris.md2", EF_ROTATE,
			NULL,
			"k_datacd",
			"Data CD",
			2,
			0,
			NULL,
			IT_STAY_COOP|IT_KEY|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED key_power_cube (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN NO_TOUCH
	warehouse circuits
	*/
	{
		"key_power_cube",
			"items/pkup.wav",
			"models/items/keys/power/tris.md2", EF_ROTATE,
			NULL,
			"k_powercube",
			"Power Cube",
			2,
			0,
			NULL,
			IT_STAY_COOP|IT_KEY|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16)
	key for the entrance of jail3
	*/
	{
		"key_pyramid",
			"items/pkup.wav",
			"models/items/keys/pyramid/tris.md2", EF_ROTATE,
			NULL,
			"k_pyramid",
			"Pyramid Key",
			2,
			0,
			NULL,
			IT_STAY_COOP|IT_KEY|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16)
	key for the city computer
	*/
	{
		"key_data_spinner",
			"items/pkup.wav",
			"models/items/keys/spinner/tris.md2", EF_ROTATE,
			NULL,
			"k_dataspin",
			"Data Spinner",
			2,
			0,
			NULL,
			IT_STAY_COOP|IT_KEY|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16)
	security pass for the security level
	*/
	{
	"key_pass",
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"k_security",
		"Security Pass",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_GRABBABLE|IT_DROPPABLE,
		0,
		/* precache */ ""
	},

	/*QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16)
	normal door key - blue
	*/
	{
		"key_blue_key",
			"items/pkup.wav",
			"models/items/keys/key/tris.md2", EF_ROTATE,
			NULL,
			"k_bluekey",
			"Blue Key",
			2,
			0,
			NULL,
			IT_STAY_COOP|IT_KEY|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16)
	normal door key - red
	*/
	{
		"key_red_key",
			"items/pkup.wav",
			"models/items/keys/red_key/tris.md2", EF_ROTATE,
			NULL,
			"k_redkey",
			"Red Key",
			2,
			0,
			NULL,
			IT_STAY_COOP|IT_KEY|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16)
	tank commander's head
	*/
	{
		"key_commander_head",
			"items/pkup.wav",
			"models/monsters/commandr/head/tris.md2", EF_GIB,
			NULL,
			/* icon */		"k_comhead",
			/* pickup */	"Commander's Head",
			/* width */		2,
			0,
			NULL,
			IT_STAY_COOP|IT_KEY|IT_GRABBABLE|IT_DROPPABLE,
			0,
			/* precache */ ""
	},

	/*QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16)
	tank commander's head
	*/
	{
		"key_airstrike_target",
		"items/pkup.wav",
		"models/items/keys/target/tris.md2", EF_ROTATE,
		NULL,
		/* icon */		"i_airstrike",
		/* pickup */	"Airstrike Marker",
		/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_GRABBABLE|IT_DROPPABLE,
		0,
		/* precache */ ""
	},

	{
		"item_health_small",
			"items/s_health.wav",
			"models/items/healing/stimpack/tris.md2", 0,
			NULL,
			/* icon */		"i_health",
			/* pickup */	"Health",
			/* width */		HEALTH_IGNORE_MAX,
			2,
			NULL,
			IT_HEALTH|IT_GRABBABLE,
			""
	},

	{
		"item_health",
		"items/n_health.wav",
		"models/items/healing/medium/tris.md2", 0,
		NULL,
		/* icon */		"i_health",
		/* pickup */	"Health",
		/* width */		0,
		10,
		NULL,
		IT_HEALTH|IT_GRABBABLE,
		0,
		""
	},
	
	{
		"item_health_large",
		"items/l_health.wav",
		"models/items/healing/large/tris.md2", 0,
		NULL,
		/* icon */		"i_health",
		/* pickup */	"Health",
		/* width */		0,
		25,
		NULL,
		IT_HEALTH|IT_GRABBABLE,
		0,
		""
	},

	{
		"item_health_mega",
		"items/m_health.wav",
		"models/items/mega_h/tris.md2", 0,
		NULL,
		/* icon */		"i_health",
		/* pickup */	"Health",
		/* width */		HEALTH_IGNORE_MAX|HEALTH_TIMED,
		100,
		NULL,
		IT_HEALTH|IT_GRABBABLE,
		""
	},

	// end of list marker
	{NULL}
};

int Items::numItems = sizeof(itemlist) / sizeof(itemlist[0]) - 1;

struct itemRegistry
{
	UNUSED_COMPARISON(itemRegistry);

	gitem_t		*item;
	int			realIndex;
	int			index;
};

static TList<itemRegistry> registry;
static bool initialized = false;

/*static*/ void Items::InitWeaponRegistry()
{
	if (initialized)
		return;

	initialized = true;

	for (int i = 0; i < Items::numItems; ++i)
	{
		if (itemlist[i].flags & IT_WEAPON)
		{
			itemRegistry reg = {&itemlist[i], i, registry.Count()};
			registry.Add(reg);
		}
	}
}

/*static*/ int Items::GetWeaponID (gitem_t *item)
{
	InitWeaponRegistry();

	for (uint32 i = 0; i < registry.Count(); ++i)
		if (registry[i].item == item)
			return registry[i].index;

	throw Exception();
}

/*static*/ gitem_t *Items::WeaponFromID (int id)
{
	InitWeaponRegistry();

	return registry[id].item;
}

/*static*/ int Items::WeaponRegistryCount()
{
	InitWeaponRegistry();

	return registry.Count();
}



/*
===============
FindItemByClassname

===============
*/
gitem_t	*FindItemByClassname (const char *classname)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<Items::numItems ; i++, it++)
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
gitem_t	*FindItem (const char *pickup_name)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<Items::numItems ; i++, it++)
	{
		if (!it->pickup_name)
			continue;
		if (!Q_stricmp(it->pickup_name, pickup_name))
			return it;
	}

	return NULL;
}
