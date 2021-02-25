//
// cch_items.h
//
// Chris Hilton (dctank@jump.net)

typedef struct behsub_s
{
	char	*classname;
	char	*subname;
} behsub_t;

behsub_t	sublist[] = 
{
	// sub chainsaw for chaingun
	{
		"weapon_chaingun",
		"weapon_chainsaw"
	},

	// sub ripper for machinegun
	{
		"weapon_machinegun",
		"weapon_ripper"
	},
	// sub ripper discs for bullets
	{
		"ammo_bullets",
		"ammo_discs"
	},
	// sub plasma gun for shotgun
	{
		"weapon_shotgun",
		"weapon_plasmagun"
	},
	// sub plasma slugs for shells
	{
		"ammo_shells",
		"ammo_plasma_slugs"
	},
};

int		num_subs = sizeof(sublist) / sizeof(sublist[0]);

char	*behweaplist[] = { 
	"weapon_chainsaw", 
	"weapon_plasmagun", 
	"weapon_ripper",
	"weapon_rcdclauncher",
	"weapon_proximlauncher"
};

int		num_behweaps = sizeof(behweaplist) / sizeof(behweaplist[0]);

char	*q2weaplist[] = {
	"weapon_shotgun",
	"weapon_supershotgun",
	"weapon_machinegun",
	"weapon_chaingun",
	"weapon_grenadelauncher",
	"weapon_rocketlauncher",
	"weapon_hyperblaster",
	"weapon_railgun",
	"weapon_bfg"
};

int		num_q2weaps = sizeof(q2weaplist) / sizeof(q2weaplist[0]);
