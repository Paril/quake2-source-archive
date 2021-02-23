#include "g_local.h"
#include "c_base.h"
#include "c_botai.h"
#include "c_botnav.h"

qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
qboolean	Pickup_NoAmmoWeapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);
void		Use_Jet ( edict_t *ent, gitem_t *item );	//MATTHIAS
void		Use_Invisibility (edict_t *ent, gitem_t *item);	//MATTHIAS
qboolean	Jet_Active( edict_t *ent );
void		AddItemToList(edict_t *ent);	//MATTHIAS

void Weapon_AK42 (edict_t *ent);
void Weapon_Shotgun (edict_t *ent);
void Weapon_SuperShotgun (edict_t *ent);
void Weapon_Machinegun (edict_t *ent);
void Weapon_Chaingun (edict_t *ent);
void Weapon_HyperBlaster (edict_t *ent);
void Weapon_RocketLauncher (edict_t *ent);
void Weapon_Grenade (edict_t *ent);
void Weapon_GrenadeLauncher (edict_t *ent);
void Weapon_Railgun (edict_t *ent);
void Weapon_BFG (edict_t *ent);

// MATTHIAS
void Weapon_FlashGrenade (edict_t *ent);
void Weapon_LaserGrenade (edict_t *ent);
void Weapon_PoisonGrenade (edict_t *ent);
void Weapon_ProxyMineLauncher (edict_t *ent);
void Weapon_FlashGrenadeLauncher (edict_t *ent);
void Weapon_PoisonGrenadeLauncher (edict_t *ent);
void Weapon_ExplosiveSuperShotgun (edict_t *ent);
void Weapon_Sword (edict_t *ent);
void Weapon_Chainsaw (edict_t *ent);
void Weapon_Crossbow (edict_t *ent);
void Weapon_ExplosiveCrossbow (edict_t *ent);
void Weapon_PoisonCrossbow (edict_t *ent);
void Weapon_Airfist (edict_t *ent);
void Weapon_HomingLauncher (edict_t *ent);
void Weapon_Buzzsaw (edict_t *ent);
void Weapon_Vortex (edict_t *ent);
void Weapon_LaserTurret (edict_t *ent);
void Weapon_RocketTurret (edict_t *ent);

gitem_armor_t jacketarmor_info	= { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};

int	jacket_armor_index;
int	combat_armor_index;
int	body_armor_index;
int	power_screen_index;
int	power_shield_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

void Use_Quad (edict_t *ent, gitem_t *item);
static int	quad_drop_timeout_hack;

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
FindItemByClassname

===============
*/
gitem_t	*FindItemByClassname (char *classname)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
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

//======================================================================

//MATTHIAS
void droptofloor (edict_t *ent);

void DoRespawn (edict_t *ent)
{
	float    rn; //MATTHIAS
	int      newit;
	gitem_t  *item;

	it_lturret = FindItem("automatic defence turret");	//bugfix
	it_airfist = FindItem("airgun");	//bugfix


	if (ent->team)
	{
		edict_t	*master;
		int	count;
		int choice;

		master = ent->teammaster;

//ZOID
//in ctf, when we are weapons stay, only the master of a team of weapons
//is spawned
		if (ctf->value &&
			((int)dmflags->value & DF_WEAPONS_STAY) &&
			master->item && (master->item->flags & IT_WEAPON))
			ent = master;
		else {
//ZOID

			for (count = 0, ent = master; ent; ent = ent->chain, count++)
				;

			choice = rand() % count;

			for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
				;
		}
	}

	//MATTHIAS - Weapon/Item exchange

	if (ent->item)
	{
		item = ent->item;
		newit = 0;

		if (strcmp(ent->classname, "ammo_shells") == 0
			|| strcmp(ent->classname, "ammo_explosiveshells") == 0)
		{
			if (random() < 0.3)
			{
				item = it_eshells;
				ent->classname = "ammo_explosiveshells";
			}
			else
			{
				item = it_shells;
				ent->classname = "ammo_shells";
			}
			newit = 1;
		}
		else if (strcmp(ent->classname, "ammo_arrows") == 0
			|| strcmp(ent->classname, "ammo_poisonarrows") == 0
			|| strcmp(ent->classname, "ammo_explosivearrows") == 0)
		{

			rn = random();
			if (rn < 0.4)
			{
				item = it_arrows;
				ent->classname = "ammo_arrows";
			}
			else if (rn >= 0.4 && rn < 0.7)
			{
				item = it_poisonarrows;
				ent->classname = "ammo_poisonarrows";
			}
			else if (rn >= 0.7)
			{
				item = it_explosivearrows;
				ent->classname = "ammo_explosivearrows";
			}

			newit = 1;
		}
		else if (strcmp(ent->classname, "item_quad") == 0
			|| strcmp(ent->classname, "item_silencer") == 0
			|| strcmp(ent->classname, "item_adrenaline") == 0
			|| strcmp(ent->classname, "item_invisibility") == 0
			|| strcmp(ent->classname, "item_jet") == 0
			|| strcmp(ent->classname, "item_grapple") == 0)
		{
			rn = random();
			if (rn < 0.1)
			{
				item = FindItem("Quad Damage");
				ent->classname = "item_quad";
			}
			else if (rn >= 0.1 && rn < 0.2)
			{
				item = FindItem("Silencer");
				ent->classname = "item_silencer";
			}
			else if (rn >= 0.2 && rn < 0.3)
			{
				item = FindItem("Adrenaline");
				ent->classname = "item_adrenaline";
			}
			else if (rn >= 0.3 && rn < 0.5)
			{
				item = FindItem("Invisibility");
				ent->classname = "item_invisibility";
			}
			else if (rn >= 0.5 && rn < 0.75)
			{
				item = FindItem("Jetpack");
				ent->classname = "item_jet";
			}
			else
			{
				item = it_grapple;
				ent->classname = "item_grapple";
			}
			newit = 1;
		}
		else if (strcmp(ent->classname, "ammo_grenades") == 0
			|| strcmp(ent->classname, "ammo_flashgrenades") == 0
			|| strcmp(ent->classname, "ammo_lasermines") == 0
			|| strcmp(ent->classname, "ammo_poisongrenades") == 0
			|| strcmp(ent->classname, "ammo_proxymines") == 0)
		{
			rn = random();

			if ((rn <= 0.2) && (numbots == 0))	//don't spawn laser mines in bot games
			{
				item = it_lasermines;
				ent->classname = "ammo_lasermines";
			}
			else if ( rn > 0.2 && rn <= 0.4)
			{
				item = it_flashgrenades;
				ent->classname = "ammo_flashgrenades";
			}
			else if ( rn > 0.4 && rn <= 0.6)
			{
				item = it_poisongrenades;
				ent->classname = "ammo_poisongrenades";
			}
			else if ( rn > 0.6 && rn <= 0.8)
			{
				item = it_proxymines;
				ent->classname = "ammo_proxymines";
			}
			else
			{
				item = it_grenades;
				ent->classname = "ammo_grenades";
			}
			newit = 1;
		}
		else if (strcmp(ent->classname, "ammo_rockets") == 0
			|| strcmp(ent->classname, "ammo_homing") == 0)
		{
			if (random() < 0.3)
			{
				item = it_homings;
				ent->classname = "ammo_homing";
			}
			else
			{
				item = it_rockets;
				ent->classname = "ammo_rockets";
			}
			newit = 1;
		}
		else if (strcmp(ent->classname, "weapon_sword") == 0
			|| strcmp(ent->classname, "weapon_chainsaw") == 0)
		{
			if (random() <= 0.5)
			{
				item = it_sword;
				ent->classname = "weapon_sword";
			}
			else
			{
				item = it_chainsaw;
				ent->classname = "weapon_chainsaw";
			}
			newit = 1;
		}
		else if (strcmp(ent->classname, "ammo_slugs") == 0
			|| strcmp(ent->classname, "ammo_buzzes") == 0)
		{
			if (random() < 0.5)
			{
				item = it_buzzes;
				ent->classname = "ammo_buzzes";
			}
			else
			{
				item = it_slugs;
				ent->classname = "ammo_slugs";
			}
			newit = 1;
		}
		else if (strcmp(ent->classname, "weapon_grenadelauncher") == 0
			|| strcmp(ent->classname, "weapon_proxyminelauncher") == 0)
		{
			if (random() < 0.3)
			{
				item = it_proxyminelauncher;
				ent->classname = "weapon_proxyminelauncher";
			}
			else
			{
				item = it_grenadelauncher;
				ent->classname = "weapon_grenadelauncher";
			}
			newit = 1;
		}
		else if (strcmp(ent->classname, "weapon_railgun") == 0
			|| strcmp(ent->classname, "weapon_buzzsaw") == 0)
		{
			if (random() < 0.5)
			{
				item = it_buzzsaw;
				ent->classname = "weapon_buzzsaw";
			}
			else
			{
				item = it_railgun;
				ent->classname = "weapon_railgun";
			}
			newit = 1;
		}
		else if (strcmp(ent->classname, "weapon_bfg") == 0
			|| strcmp(ent->classname, "ammo_vortex") == 0
			|| strcmp(ent->classname, "ammo_laserturret") == 0
			|| strcmp(ent->classname, "ammo_rocketturret") == 0)
		{
			rn = random();

			if ( rn <= 0.3)
			{
				item = it_vortex;
				ent->classname = "ammo_vortex";
			}
			else if ((rn > 0.3) && (rn <= 0.5))
			{
				item = it_rturret;
				ent->classname = "ammo_rocketturret";
			}
			else if ((rn > 0.5) && (rn <= 0.7))
			{
				item = it_lturret;
				ent->classname = "ammo_laserturret";
			}
			else
			{
				item = it_bfg;
				ent->classname = "weapon_bfg";
			}
			newit = 1;
		}
	//MATTHIAS - Weapon banning

		if (!Q_stricmp(ent->classname, "weapon_sword") && ban_sword->value)
		{
			if (ban_chainsaw->value) //banned,too
			{
				G_FreeEdict (ent);
				return;
			}
			else
			{
				item = it_chainsaw;
				ent->classname = "weapon_chainsaw";
			}
		}
		else if (strcmp(ent->classname, "weapon_chainsaw") == 0 && ban_chainsaw->value > 0)
		{
			if (ban_sword->value > 0) //banned,too
			{
				G_FreeEdict (ent);
				return;
			}
			else
			{
				item = it_sword;
				ent->classname = "weapon_sword";
			}
		}
		else if (strcmp(ent->classname, "weapon_supershotgun") == 0 && ban_supershotgun->value > 0)
		{
			G_FreeEdict (ent);
			return;
		}
		else if (strcmp(ent->classname, "weapon_crossbow") == 0 && ban_crossbow->value > 0)
		{
			G_FreeEdict (ent);
			return;
		}
		else if (strcmp(ent->classname, "weapon_airfist") == 0 && ban_airgun->value > 0)
		{
			G_FreeEdict (ent);
			return;
		}
		else if (strcmp(ent->classname, "weapon_grenadelauncher") == 0 && ban_grenadelauncher->value > 0)
		{
			if (ban_proxylauncher->value > 0) //banned,too
			{
				G_FreeEdict (ent);
				return;
			}
			else
			{
				item = it_proxyminelauncher;
				ent->classname = "weapon_proxyminelauncher";
			}
		}
		else if (strcmp(ent->classname, "weapon_proxyminelauncher") == 0 && ban_proxylauncher->value > 0)
		{
			if (ban_grenadelauncher->value > 0) //banned,too
			{
				G_FreeEdict (ent);
				return;
			}
			else
			{
				item = it_grenadelauncher;
				ent->classname = "weapon_grenadelauncher";
			}
		}
		else if (strcmp(ent->classname, "weapon_rocketlauncher") == 0 && ban_rocketlauncher->value > 0)
		{
			G_FreeEdict (ent);
			return;
		}
		else if (strcmp(ent->classname, "weapon_railgun") == 0 && ban_railgun->value > 0)
		{
			if (ban_buzzsaw->value > 0) //banned,too
			{
				G_FreeEdict (ent);
				return;
			}
			else
			{
				item = it_buzzsaw;
				ent->classname = "weapon_buzzsaw";
			}
		}
		else if (strcmp(ent->classname, "weapon_buzzsaw") == 0 && ban_buzzsaw->value > 0)
		{
			if (ban_railgun->value > 0) //banned,too
			{
				G_FreeEdict (ent);
				return;
			}
			else
			{
				item = it_railgun;
				ent->classname = "weapon_railgun";
			}
		}
		else if (strcmp(ent->classname, "weapon_bfg") == 0 && ban_bfg->value > 0)
		{
			if (ban_vortex->value == 0)
			{
				item = it_vortex;
				ent->classname = "ammo_vortex";
			}
			else if (ban_defenceturret->value == 0)
			{
				item = it_lturret;
				ent->classname = "ammo_laserturret";
			}
			else if (ban_rocketturret->value == 0)
			{
				item = it_rturret;
				ent->classname = "ammo_rocketturret";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "weapon_vortex") == 0 && ban_vortex->value > 0)
		{
			if (ban_bfg->value == 0)
			{
				item = it_bfg;
				ent->classname = "weapon_bfg";
			}
			else if (ban_defenceturret->value == 0)
			{
				item = it_lturret;
				ent->classname = "ammo_laserturret";
			}
			else if (ban_rocketturret->value == 0)
			{
				item = it_rturret;
				ent->classname = "ammo_rocketturret";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "ammo_laserturret") == 0 && ban_defenceturret->value > 0)
		{
			if (ban_bfg->value == 0)
			{
				item = it_bfg;
				ent->classname = "weapon_bfg";
			}
			else if (ban_vortex->value == 0)
			{
				item = it_vortex;
				ent->classname = "ammo_vortex";
			}
			else if (ban_rocketturret->value == 0)
			{
				item = it_rturret;
				ent->classname = "ammo_rocketturret";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "ammo_rocketturret") == 0 && ban_rocketturret->value > 0)
		{
			if (ban_bfg->value == 0)
			{
				item = it_bfg;
				ent->classname = "weapon_bfg";
			}
			else if (ban_defenceturret->value == 0)
			{
				item = it_lturret;
				ent->classname = "ammo_laserturret";
			}
			else if (ban_vortex->value == 0)
			{
				item = it_vortex;
				ent->classname = "ammo_vortex";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "ammo_grenades") == 0 && ban_ammo_grenades->value > 0)
		{
			if (ban_ammo_flashgrenades->value == 0)
			{
				item = it_flashgrenades;
				ent->classname = "ammo_flashgrenades";
			}
			else if (ban_ammo_lasergrenades->value == 0)
			{
				item = it_lasermines;
				ent->classname = "ammo_lasermines";
			}
			else if (ban_ammo_poisongrenades->value == 0)
			{
				item = it_poisongrenades;
				ent->classname = "ammo_poisongrenades";
			}
			else if (ban_ammo_proximitymines->value == 0)
			{
				item = it_proxymines;
				ent->classname = "ammo_proxymines";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "ammo_flashgrenades") == 0 && ban_ammo_flashgrenades->value > 0)
		{
			if (ban_ammo_lasergrenades->value == 0)
			{
				item = it_lasermines;
				ent->classname = "ammo_lasermines";
			}
			else if (ban_ammo_poisongrenades->value == 0)
			{
				item = it_poisongrenades;
				ent->classname = "ammo_poisongrenades";
			}
			else if (ban_ammo_proximitymines->value == 0)
			{
				item = it_proxymines;
				ent->classname = "ammo_proxymines";
			}
			else if (ban_ammo_grenades->value == 0)
			{
				item = it_grenades;
				ent->classname = "ammo_grenades";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "ammo_lasermines") == 0 && ban_ammo_lasergrenades->value > 0)
		{
			if (ban_ammo_poisongrenades->value == 0)
			{
				item = it_poisongrenades;
				ent->classname = "ammo_poisongrenades";
			}
			else if (ban_ammo_proximitymines->value == 0)
			{
				item = it_proxymines;
				ent->classname = "ammo_proxymines";
			}
			else if (ban_ammo_grenades->value == 0)
			{
				item = it_grenades;
				ent->classname = "ammo_grenades";
			}
			else if (ban_ammo_flashgrenades->value == 0)
			{
				item = it_flashgrenades;
				ent->classname = "ammo_flashgrenades";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "ammo_poisongrenades") == 0 && ban_ammo_poisongrenades->value > 0)
		{
			if (ban_ammo_proximitymines->value == 0)
			{
				item = it_proxymines;
				ent->classname = "ammo_proxymines";
			}
			else if (ban_ammo_grenades->value == 0)
			{
				item = it_grenades;
				ent->classname = "ammo_grenades";
			}
			else if (ban_ammo_flashgrenades->value == 0)
			{
				item = it_flashgrenades;
				ent->classname = "ammo_flashgrenades";
			}
			else if (ban_ammo_lasergrenades->value == 0)
			{
				item = it_lasermines;
				ent->classname = "ammo_lasermines";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "ammo_proxymines") == 0 && ban_ammo_proximitymines->value > 0)
		{
			
			if (ban_ammo_grenades->value == 0)
			{
				item = it_grenades;
				ent->classname = "ammo_grenades";
			}
			else if (ban_ammo_flashgrenades->value == 0)
			{
				item = it_flashgrenades;
				ent->classname = "ammo_flashgrenades";
			}
			else if (ban_ammo_lasergrenades->value == 0)
			{
				item = it_lasermines;
				ent->classname = "ammo_lasermines";
			}
			else if (ban_ammo_poisongrenades->value == 0)
			{
				item = it_poisongrenades;
				ent->classname = "ammo_poisongrenades";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "ammo_shells") == 0 && ban_ammo_shells->value > 0)
		{
			if (ban_ammo_explosiveshells->value > 0) //banned,too
			{
				G_FreeEdict (ent);
				return;
			}
			else
			{
				item = it_eshells;
				ent->classname = "ammo_explosiveshells";
			}
		}
		else if (strcmp(ent->classname, "ammo_explosiveshells") == 0 && ban_ammo_explosiveshells->value > 0)
		{
			if (ban_ammo_shells->value > 0) //banned,too
			{
				G_FreeEdict (ent);
				return;
			}
			else
			{
				item = it_shells;
				ent->classname = "ammo_shells";
			}
		}
		else if (strcmp(ent->classname, "ammo_rockets") == 0 && ban_ammo_rockets->value > 0)
		{
			if (ban_ammo_homingmissiles->value > 0) //banned,too
			{
				G_FreeEdict (ent);
				return;
			}
			else
			{
				item = it_homings;
				ent->classname = "ammo_homing";
			}
		}
		else if (strcmp(ent->classname, "ammo_homing") == 0 && ban_ammo_homingmissiles->value > 0)
		{
			if (ban_ammo_rockets->value > 0) //banned,too
			{
				G_FreeEdict (ent);
				return;
			}
			else
			{
				item = it_rockets;
				ent->classname = "ammo_rockets";
			}
		}
		else if (strcmp(ent->classname, "ammo_slugs") == 0 && ban_ammo_slugs->value > 0)
		{
			if (ban_ammo_buzzes->value > 0) //banned,too
			{
				G_FreeEdict (ent);
				return;
			}
			else
			{
				item = it_buzzes;
				ent->classname = "ammo_buzzes";
			}
		}
		else if (strcmp(ent->classname, "ammo_buzzes") == 0 && ban_ammo_buzzes->value > 0)
		{
			if (ban_ammo_slugs->value > 0) //banned,too
			{
				G_FreeEdict (ent);
				return;
			}
			else
			{
				item = it_slugs;
				ent->classname = "ammo_slugs";
			}
		}
		else if (strcmp(ent->classname, "ammo_arrows") == 0 && ban_ammo_arrows->value > 0)
		{
			if (ban_ammo_poisonarrows->value == 0)
			{
				item = it_poisonarrows;
				ent->classname = "ammo_poisonarrows";
			}
			else if (ban_ammo_explosivearrows->value == 0)
			{
				item = it_explosivearrows;
				ent->classname = "ammo_explosivearrows";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "ammo_poisonarrows") == 0 && ban_ammo_poisonarrows->value > 0)
		{
			
			if (ban_ammo_explosivearrows->value == 0)
			{
				item = it_explosivearrows;
				ent->classname = "ammo_explosivearrows";
			}
			else if (ban_ammo_arrows->value == 0)
			{
				item = it_arrows;
				ent->classname = "ammo_arrows";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "ammo_explosivearrows") == 0 && ban_ammo_explosivearrows->value > 0)
		{
			if (ban_ammo_arrows->value == 0)
			{
				item = it_arrows;
				ent->classname = "ammo_arrows";
			}
			else if (ban_ammo_poisonarrows->value == 0)
			{
				item = it_poisonarrows;
				ent->classname = "ammo_poisonarrows";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "item_quad") == 0 && ban_quaddamage->value > 0)
		{
			if (ban_jetpack->value == 0)
			{
				item = FindItem("Jetpack");
				ent->classname = "item_jet";
			}
			else if (ban_grapple->value == 0)
			{
				item = it_grapple;
				ent->classname = "item_grapple";
			}
			else if (ban_invisibility->value == 0)
			{
				item = FindItem("Invisibility");
				ent->classname = "item_invisibility";
			}
			else if (ban_adrenaline->value == 0)
			{
				item = FindItem("Adrenaline");
				ent->classname = "item_adrenaline";
			}
			else if (ban_silencer->value == 0)
			{
				item = FindItem("Silencer");
				ent->classname = "item_silencer";
			}		
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "item_jet") == 0 && ban_jetpack->value > 0)
		{
			if (ban_grapple->value == 0)
			{
				item = it_grapple;
				ent->classname = "item_grapple";
			}
			else if (ban_invisibility->value == 0)
			{
				item = FindItem("Invisibility");
				ent->classname = "item_invisibility";
			}
			else if (ban_adrenaline->value == 0)
			{
				item = FindItem("Adrenaline");
				ent->classname = "item_adrenaline";
			}
			else if (ban_silencer->value == 0)
			{
				item = FindItem("Silencer");
				ent->classname = "item_silencer";
			}
			else if (ban_quaddamage->value == 0)
			{
				item = FindItem("Quad Damage");
				ent->classname = "item_quad";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "item_grapple") == 0 && ban_grapple->value > 0)
		{
			if (ban_invisibility->value == 0)
			{
				item = FindItem("Invisibility");
				ent->classname = "item_invisibility";
			}
			else if (ban_adrenaline->value == 0)
			{
				item = FindItem("Adrenaline");
				ent->classname = "item_adrenaline";
			}
			else if (ban_silencer->value == 0)
			{
				item = FindItem("Silencer");
				ent->classname = "item_silencer";
			}
			else if (ban_quaddamage->value == 0)
			{
				item = FindItem("Quad Damage");
				ent->classname = "item_quad";
			}
			else if (ban_jetpack->value == 0)
			{
				item = FindItem("Jetpack");
				ent->classname = "item_jet";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "item_invisibility") == 0 && ban_invisibility->value > 0)
		{
			if (ban_adrenaline->value == 0)
			{
				item = FindItem("Adrenaline");
				ent->classname = "item_adrenaline";
			}
			else if (ban_silencer->value == 0)
			{
				item = FindItem("Silencer");
				ent->classname = "item_silencer";
			}
			else if (ban_quaddamage->value == 0)
			{
				item = FindItem("Quad Damage");
				ent->classname = "item_quad";
			}
			else if (ban_jetpack->value == 0)
			{
				item = FindItem("Jetpack");
				ent->classname = "item_jet";
			}
			else if (ban_invisibility->value == 0)
			{
				item = it_grapple;
				ent->classname = "item_grapple";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "item_adrenaline") == 0 && ban_adrenaline->value > 0)
		{
			if (ban_silencer->value == 0)
			{
				item = FindItem("Silencer");
				ent->classname = "item_silencer";
			}
			else if (ban_quaddamage->value == 0)
			{
				item = FindItem("Quad Damage");
				ent->classname = "item_quad";
			}
			else if (ban_jetpack->value == 0)
			{
				item = FindItem("Jetpack");
				ent->classname = "item_jet";
			}
			else if (ban_invisibility->value == 0)
			{
				item = it_grapple;
				ent->classname = "item_grapple";
			}
			else if (ban_adrenaline->value == 0)
			{
				item = FindItem("Invisibility");
				ent->classname = "item_invisibility";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		else if (strcmp(ent->classname, "item_silencer") == 0 && ban_silencer->value > 0)
		{
			if (ban_quaddamage->value == 0)
			{
				item = FindItem("Quad Damage");
				ent->classname = "item_quad";
			}
			else if (ban_jetpack->value == 0)
			{
				item = FindItem("Jetpack");
				ent->classname = "item_jet";
			}
			else if (ban_invisibility->value == 0)
			{
				item = it_grapple;
				ent->classname = "item_grapple";
			}
			else if (ban_adrenaline->value == 0)
			{
				item = FindItem("Invisibility");
				ent->classname = "item_invisibility";
			}
			else if (ban_adrenaline->value == 0)
			{
				item = FindItem("Adrenaline");
				ent->classname = "item_adrenaline";
			}
			else
			{
				G_FreeEdict (ent);
				return;
			}
		}
		
		VectorCopy(ent->spawnorigin, ent->s.origin);
		ent->nextthink = level.time + 0.1;
		ent->think = droptofloor;

		if (newit == 1)
		{
			ent->item = item;
			gi.setmodel (ent, ent->item->world_model);
		}
	}
	
	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
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

qboolean Pickup_Powerup (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	if ((skill->value == 1 && quantity >= 2) || (skill->value >= 2 && quantity >= 1))
		return false;

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;

	// LETHAL : start
	if ( ent->item == it_grapple )
	{
		if ( other->client->pers.inventory[ITEM_INDEX(ent->item)] > 0 )
			return false;
		else
		{
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
			return true;
		}
	}	

	if ( ent->item == it_jetpack )
	{
		if ( other->client->pers.inventory[ITEM_INDEX(ent->item)] > 0 
		    && other->client->jet_remaining == 600 )
		{
			return false; 

		}
		else 
		{
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
			other->client->jet_remaining = 600;
	
	 				
			if ( Jet_Active(other) || (int) dmflags->value & DF_INSTANT_JET )
				other->client->jet_framenum = level.framenum + other->client->jet_remaining;
			else
			  other->client->jet_framenum = 0;
			
			return true;
		}
       	}
	
	// LETHAL : end	

        other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
                                                        
	if (!(ent->spawnflags & DROPPED_ITEM) )
		SetRespawn (ent, ent->item->quantity);
	if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
	{
		if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
			quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
		{
			if (ent->item->use)//MATTHIAS
				if(!(ent->item->use == Use_Jet))
					ent->item->use (other, ent->item);
		}
	}

	return true;
}

void Drop_General (edict_t *ent, gitem_t *item)
{
	Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
}


//======================================================================

qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other)
{
	if (other->health < other->max_health)
		other->health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_AncientHead (edict_t *ent, edict_t *other)
{
	other->max_health += 2;

	if (!(ent->spawnflags & DROPPED_ITEM))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Bandolier (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;
	vec3_t	dir;
	vec_t	dist;

	if (other->client->pers.max_bullets < 250)
		other->client->pers.max_bullets = 250;
	if (other->client->pers.max_shells < 150)
		other->client->pers.max_shells = 150;
	if (other->client->pers.max_cells < 250)
		other->client->pers.max_cells = 250;
	if (other->client->pers.max_slugs < 75)
		other->client->pers.max_slugs = 75;
	if (other->client->pers.max_eshells < 150) //MATTHIAS
		other->client->pers.max_eshells = 150;
	if (other->client->pers.max_arrows < 100)
		other->client->pers.max_arrows = 100;
	if (other->client->pers.max_poisonarrows < 100)
		other->client->pers.max_poisonarrows = 100;
	if (other->client->pers.max_explosivearrows < 100)
		other->client->pers.max_explosivearrows = 100;

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	item = FindItem("Cells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_cells)
			other->client->pers.inventory[index] = other->client->pers.max_cells;
	}

	item = FindItem("Explosive Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_eshells)
			other->client->pers.inventory[index] = other->client->pers.max_eshells;
	}

	item = FindItem("Slugs");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_slugs)
			other->client->pers.inventory[index] = other->client->pers.max_slugs;
	}

	item = FindItem("Arrows");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_arrows)
			other->client->pers.inventory[index] = other->client->pers.max_arrows;
	}

	if (!(ent->spawnflags & DROPPED_ITEM))
		SetRespawn (ent, ent->item->quantity);

	if (strcmp(other->classname,"bot") == 0)	//MATTHIAS
	{
		if (other->enemy)
		{
			VectorSubtract(other->enemy->s.origin, other->s.origin, dir);
			dist = VectorLength(dir);
			if (dist > IDEAL_ENEMY_DIST)
				Bot_BestFarWeapon (other);
			else if (dist >= MELEE_DIST)
				Bot_BestMidWeapon (other);
			else
				Bot_BestCloseWeapon (other);
		}
		else
			Bot_BestFarWeapon (other);
	}

	return true;
}

qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;
	vec3_t	dir;
	vec_t	dist;

	if (other->client->pers.max_bullets < 300)
		other->client->pers.max_bullets = 300;
	if (other->client->pers.max_shells < 200)
		other->client->pers.max_shells = 200;
	if (other->client->pers.max_rockets < 100)
		other->client->pers.max_rockets = 100;
	if (other->client->pers.max_grenades < 100)
		other->client->pers.max_grenades = 100;
	if (other->client->pers.max_cells < 300)
		other->client->pers.max_cells = 300;
	if (other->client->pers.max_slugs < 100)
		other->client->pers.max_slugs = 100;
	if (other->client->pers.max_eshells < 200) //MATTHIAS
		other->client->pers.max_eshells = 200;
	if (other->client->pers.max_arrows < 150)
		other->client->pers.max_arrows = 150;
	if (other->client->pers.max_poisonarrows < 150)
		other->client->pers.max_poisonarrows = 150;
	if (other->client->pers.max_explosivearrows < 150)
		other->client->pers.max_explosivearrows = 150;
	if (other->client->pers.max_flashgrenades < 100)
		other->client->pers.max_flashgrenades = 100;
	if (other->client->pers.max_lasergrenades < 100)
		other->client->pers.max_lasergrenades = 100;
	if (other->client->pers.max_poisongrenades < 100)
		other->client->pers.max_poisongrenades = 100;
	if (other->client->pers.max_proxymines < 100)
		other->client->pers.max_proxymines = 100;
	if (other->client->pers.max_homing < 100)
		other->client->pers.max_homing = 100;
	if (other->client->pers.max_buzzes < 100)
		other->client->pers.max_buzzes = 100;

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	item = FindItem("Cells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_cells)
			other->client->pers.inventory[index] = other->client->pers.max_cells;
	}

	item = FindItem("Grenades");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
			other->client->pers.inventory[index] = other->client->pers.max_grenades;
	}

	item = FindItem("Rockets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
			other->client->pers.inventory[index] = other->client->pers.max_rockets;
	}

	item = FindItem("Slugs");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_slugs)
			other->client->pers.inventory[index] = other->client->pers.max_slugs;
	}

	item = FindItem("Explosive Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_eshells)
			other->client->pers.inventory[index] = other->client->pers.max_eshells;
	}

	item = FindItem("Arrows");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_arrows)
			other->client->pers.inventory[index] = other->client->pers.max_arrows;
	}

	item = FindItem("Flash Grenades");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_flashgrenades)
			other->client->pers.inventory[index] = other->client->pers.max_flashgrenades;
	}

	item = FindItem("Laser Mines");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_lasergrenades)
			other->client->pers.inventory[index] = other->client->pers.max_lasergrenades;
	}

	item = FindItem("Poison Grenades");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_poisongrenades)
			other->client->pers.inventory[index] = other->client->pers.max_poisongrenades;
	}

	item = FindItem("Proximity Mines");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_proxymines)
			other->client->pers.inventory[index] = other->client->pers.max_proxymines;
	}

	item = FindItem("Homing Missiles");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_homing)
			other->client->pers.inventory[index] = other->client->pers.max_homing;
	}

	item = FindItem("Buzzes");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_buzzes)
			other->client->pers.inventory[index] = other->client->pers.max_buzzes;
	}

	if (!(ent->spawnflags & DROPPED_ITEM))
		SetRespawn (ent, ent->item->quantity);

	if (strcmp(other->classname,"bot") == 0)	//MATTHIAS
	{
		if (other->enemy)
		{
			VectorSubtract(other->enemy->s.origin, other->s.origin, dir);
			dist = VectorLength(dir);
			if (dist > IDEAL_ENEMY_DIST)
				Bot_BestFarWeapon (other);
			else if (dist >= MELEE_DIST)
				Bot_BestMidWeapon (other);
			else
				Bot_BestCloseWeapon (other);
		}
		else
			Bot_BestFarWeapon (other);
	}

	return true;
}

//======================================================================

void Use_Quad (edict_t *ent, gitem_t *item)
{
	int		timeout;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (quad_drop_timeout_hack)
	{
		timeout = quad_drop_timeout_hack;
		quad_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->client->quad_framenum > level.framenum)
		ent->client->quad_framenum += timeout;
	else
		ent->client->quad_framenum = level.framenum + timeout;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Breather (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->breather_framenum > level.framenum)
		ent->client->breather_framenum += 300;
	else
		ent->client->breather_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Envirosuit (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->enviro_framenum > level.framenum)
		ent->client->enviro_framenum += 300;
	else
		ent->client->enviro_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void	Use_Invulnerability (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += 300;
	else
		ent->client->invincible_framenum = level.framenum + 300;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void	Use_Silencer (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	ent->client->silencer_shots += 30;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

qboolean Pickup_Key (edict_t *ent, edict_t *other)
{
	if (coop->value)
	{
		if (strcmp(ent->classname, "key_power_cube") == 0)
		{
			if (other->client->pers.power_cubes & ((ent->spawnflags & 0x0000ff00)>> 8))
				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
			other->client->pers.power_cubes |= ((ent->spawnflags & 0x0000ff00) >> 8);
		}
		else
		{
			if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
		}
		return true;
	}
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	return true;
}

//======================================================================

qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count)
{
	int			index;
	int			max;

	if (!ent->client)
		return false;

	if (item->tag == AMMO_BULLETS)
		max = ent->client->pers.max_bullets;
	else if (item->tag == AMMO_SHELLS)
		max = ent->client->pers.max_shells;
	else if (item->tag == AMMO_ROCKETS)
		max = ent->client->pers.max_rockets;
	else if (item->tag == AMMO_GRENADES)
		max = ent->client->pers.max_grenades;
	else if (item->tag == AMMO_CELLS)
		max = ent->client->pers.max_cells;
	else if (item->tag == AMMO_SLUGS)
		max = ent->client->pers.max_slugs;
	else if (item->tag == AMMO_EXPLOSIVESHELLS) //MATTHIAS
		max = ent->client->pers.max_eshells;
	else if (item->tag == AMMO_ARROWS)
		max = ent->client->pers.max_arrows;
	else if (item->tag == AMMO_POISONARROWS)
		max = ent->client->pers.max_poisonarrows;
	else if (item->tag == AMMO_EXPLOSIVEARROWS)
		max = ent->client->pers.max_explosivearrows;
	else if (item->tag == AMMO_FLASHGRENADES)
		max = ent->client->pers.max_flashgrenades;
	else if (item->tag == AMMO_LASERGRENADES)
		max = ent->client->pers.max_lasergrenades;
	else if (item->tag == AMMO_POISONGRENADES)
		max = ent->client->pers.max_poisongrenades;
	else if (item->tag == AMMO_PROXYMINES)
		max = ent->client->pers.max_proxymines;
	else if (item->tag == AMMO_HOMING)
		max = ent->client->pers.max_homing;
	else if (item->tag == AMMO_BUZZES)
		max = ent->client->pers.max_buzzes;
	else if (item->tag == AMMO_VORTEX)
		max = ent->client->pers.max_vortex;
	else if (item->tag == AMMO_LTURRET)
		max = ent->client->pers.max_lturret;
	else if (item->tag == AMMO_RTURRET)
		max = ent->client->pers.max_rturret;
	else
		return false;

	index = ITEM_INDEX(item);

	if (ent->client->pers.inventory[index] == max)
		return false;

	ent->client->pers.inventory[index] += count;

	if (ent->client->pers.inventory[index] > max)
		ent->client->pers.inventory[index] = max;

	return true;
}

qboolean Pickup_Ammo (edict_t *ent, edict_t *other)
{
	int			oldcount;
	int			count;
	qboolean	weapon;
	vec3_t	dir;
	vec_t	dist;

	weapon = (ent->item->flags & IT_WEAPON);
	if ( (weapon) && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		count = 1000;
	else if (ent->count)
		count = ent->count;
	else
		count = ent->item->quantity;

	oldcount = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (!Add_Ammo (other, ent->item, count))
		return false;

	if (weapon && !oldcount)
	{
		if (other->client->pers.weapon != ent->item && ( !deathmatch->value || other->client->pers.weapon == FindItem("AK42 Assault Pistol") ) )
			other->client->newweapon = ent->item;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 30);

	if (strcmp(other->classname,"bot") == 0)	//MATTHIAS
	{
		if (other->client->pers.weapon != it_rturret
			&& other->client->pers.weapon != it_lturret
			&& other->client->pers.weapon != it_proxyminelauncher)
		{
			if (other->enemy)
			{
				VectorSubtract(other->enemy->s.origin, other->s.origin, dir);
				dist = VectorLength(dir);
				if (dist > IDEAL_ENEMY_DIST)
					Bot_BestFarWeapon (other);
				else if (dist >= MELEE_DIST)
					Bot_BestMidWeapon (other);
				else
					Bot_BestCloseWeapon (other);
			}
			else
				Bot_BestFarWeapon (other);
		}
	}

	return true;
}

void Drop_Ammo (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	int		index;

	index = ITEM_INDEX(item);
	dropped = Drop_Item (ent, item);
	if (ent->client->pers.inventory[index] >= item->quantity)
		dropped->count = item->quantity;
	else
		dropped->count = ent->client->pers.inventory[index];
	ent->client->pers.inventory[index] -= dropped->count;
	ValidateSelectedItem (ent);
}


//======================================================================

void MegaHealth_think (edict_t *self)
{
	if ( ( self->owner->health > self->owner->max_health &&
	       !CTFHasRegeneration(self->owner) ) ||
	     ( CTFHasRegeneration(self->owner) && 
	       self->owner->health > 150 ) )
	{
		self->nextthink = level.time + 1;
		self->owner->health -= 1;
		return;
	}

	if (!(self->spawnflags & DROPPED_ITEM))
		SetRespawn (self, 20);
	else
		G_FreeEdict (self);
}

qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->health >= other->max_health)
			return false;

//ZOID
	if (other->health >= 250 && ent->count > 25)
		return false;
//ZOID

	other->health += ent->count;

//ZOID
	if (other->health > 250 && ent->count > 25)
		other->health = 250;
//ZOID

	if (ent->count == 2)
		ent->item->pickup_sound = "items/s_health.wav";
	else if (ent->count == 10)
		ent->item->pickup_sound = "items/n_health.wav";
	else if (ent->count == 25)
		ent->item->pickup_sound = "items/l_health.wav";
	else // (ent->count == 100)
		ent->item->pickup_sound = "items/m_health.wav";

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
			other->health = other->max_health;
	}

//ZOID
	if (ent->style & HEALTH_TIMED )
		// && !CTFHasRegeneration(other) )
//ZOID
	{
		ent->think = MegaHealth_think;
		ent->nextthink = level.time + 5;
		ent->owner = other;
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	else
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
			SetRespawn (ent, 30);
	}

	return true;
}

//======================================================================

int ArmorIndex (edict_t *ent)
{
	if (!ent->client)
		return 0;

	if (ent->client->pers.inventory[jacket_armor_index] > 0)
		return jacket_armor_index;

	if (ent->client->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

	if (ent->client->pers.inventory[body_armor_index] > 0)
		return body_armor_index;

	return 0;
}

qboolean Pickup_Armor (edict_t *ent, edict_t *other)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	float			salvage;
	int				salvagecount;

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;

	old_armor_index = ArmorIndex (other);

	// handle armor shards specially
	if (ent->item->tag == ARMOR_SHARD)
	{
		if (!old_armor_index)
			other->client->pers.inventory[jacket_armor_index] = 2;
		else
			other->client->pers.inventory[old_armor_index] += 2;
	}

	// if player has no armor, just use it
	else if (!old_armor_index)
	{
		other->client->pers.inventory[ITEM_INDEX(ent->item)] = newinfo->base_count;
	}

	// use the better armor
	else
	{
		// get info on old armor
		if (old_armor_index == jacket_armor_index)
			oldinfo = &jacketarmor_info;
		else if (old_armor_index == combat_armor_index)
			oldinfo = &combatarmor_info;
		else // (old_armor_index == body_armor_index)
			oldinfo = &bodyarmor_info;

		if (newinfo->normal_protection > oldinfo->normal_protection)
		{
			// calc new armor values
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount = salvage * other->client->pers.inventory[old_armor_index];
			newcount = newinfo->base_count + salvagecount;
			if (newcount > newinfo->max_count)
				newcount = newinfo->max_count;

			// zero count of old armor so it goes away
			other->client->pers.inventory[old_armor_index] = 0;

			// change armor to new item with computed value
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;
		}
		else
		{
			// calc new armor values
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = salvage * newinfo->base_count;
			newcount = other->client->pers.inventory[old_armor_index] + salvagecount;
			if (newcount > oldinfo->max_count)
				newcount = oldinfo->max_count;

			// if we're already maxed out then we don't need the new armor
			if (other->client->pers.inventory[old_armor_index] >= newcount)
				return false;

			// update current armor value
			other->client->pers.inventory[old_armor_index] = newcount;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM))
		SetRespawn (ent, 20);

	return true;
}

//======================================================================

int PowerArmorType (edict_t *ent)
{
	if (!ent->client)
		return POWER_ARMOR_NONE;

	if (!(ent->flags & FL_POWER_ARMOR))
		return POWER_ARMOR_NONE;

	if (ent->client->pers.inventory[power_shield_index] > 0)
		return POWER_ARMOR_SHIELD;

	if (ent->client->pers.inventory[power_screen_index] > 0)
		return POWER_ARMOR_SCREEN;

	return POWER_ARMOR_NONE;
}

void Use_PowerArmor (edict_t *ent, gitem_t *item)
{
	int		index;

	if (ent->flags & FL_POWER_ARMOR)
	{
		ent->flags &= ~FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		index = ITEM_INDEX(FindItem("cells"));
		if (!ent->client->pers.inventory[index])
		{
			cprintf2 (ent, PRINT_HIGH, "No cells for power armor.\n");
			return;
		}
		ent->flags |= FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
	}
}

qboolean Pickup_PowerArmor (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (!(ent->spawnflags & DROPPED_ITEM) )
		SetRespawn (ent, ent->item->quantity);
	// auto-use for DM only if we didn't already have one
	if (!quantity)
		ent->item->use (other, ent->item);

	return true;
}

void Drop_PowerArmor (edict_t *ent, gitem_t *item)
{
	if ((ent->flags & FL_POWER_ARMOR) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
		Use_PowerArmor (ent, item);
	Drop_General (ent, item);
}

//======================================================================

/*
===============
Touch_Item
===============
*/
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean	taken;

	if (!other->client)
		return;
	if (other->health < 1)
		return;		// dead people can't pickup
	if (!ent->item->pickup)
		return;		// not a grabbable item?

	taken = ent->item->pickup(ent, other);

	if (taken)
	{
		// flash the screen
		other->client->bonus_alpha = 0.25;	

		// show icon and name on status bar
		other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
		other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);
		other->client->pickup_msg_time = level.time + 3.0;

		// change selected item
		if (ent->item->use)
			other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);

		gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
	}

	if (!(ent->spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets (ent, other);
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

	if (!taken)
		return;

	if (!((coop->value) &&  (ent->item->flags & IT_STAY_COOP)) || (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
	{
		if (ent->flags & FL_RESPAWN)
			ent->flags &= ~FL_RESPAWN;
		else
			G_FreeEdict (ent);
	}
}

//======================================================================

static void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	Touch_Item (ent, other, plane, surf);
}

static void drop_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	ent->nextthink = level.time + 29;
	ent->think = G_FreeEdict;
}

edict_t *Drop_Item (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	vec3_t	forward, right;
	vec3_t	offset;

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;  
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
			dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}

	VectorScale (forward, 100, dropped->velocity);
	dropped->velocity[2] = 300;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

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

//======================================================================

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
		G_FreeEdict (ent);
		return;
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
		ent->use = Use_Item;
	}

	//MATTHIAS Autorespawn

	ent->nextthink = level.time + 60 + random() * 80;
	ent->think = DoRespawn;

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
	int		len;
	gitem_t	*ammo;

	if (!it)
		return;

	if (it->pickup_sound)
		gi.soundindex (it->pickup_sound);
	if (it->world_model)
		gi.modelindex (it->world_model);
	if (it->view_model)
		gi.modelindex (it->view_model);
	if (it->icon)
		gi.imageindex (it->icon);

	// parse everything for its ammo
	if (it->ammo && it->ammo[0])
	{
		ammo = FindItem (it->ammo);
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
	float  rn; //MATTHIAS

	it_lturret = FindItem("automatic defence turret");	//bugfix
	it_airfist = FindItem("airgun");	//bugfix


	PrecacheItem (item);
	

	if (ent->spawnflags)
	{
		if (strcmp(ent->classname, "key_power_cube") != 0)
		{
			ent->spawnflags = 0;
			gi.dprintf("%s at %s has invalid spawnflags set\n", ent->classname, vtos(ent->s.origin));
		}
	}

	// some items will be prevented in deathmatch
	
	if ( (int)dmflags->value & DF_NO_ARMOR )
	{
		if (item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor)
		{
			G_FreeEdict (ent);
			return;
		}
	}
	if ( (int)dmflags->value & DF_NO_ITEMS )
	{
		if (item->pickup == Pickup_Powerup)
		{
			G_FreeEdict (ent);
			return;
		}
	}
	if ( (int)dmflags->value & DF_NO_HEALTH )
	{
		if (item->pickup == Pickup_Health || item->pickup == Pickup_Adrenaline || item->pickup == Pickup_AncientHead)
		{
			G_FreeEdict (ent);
			return;
		}
	}
	if ( (int)dmflags->value & DF_INFINITE_AMMO )
	{
		if ( (item->flags == IT_AMMO) || (strcmp(ent->classname, "weapon_bfg") == 0) )
		{
			G_FreeEdict (ent);
			return;
		}
	}

//ZOID
//Don't spawn the flags unless enabled
	if (!ctf->value &&
		(strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0)) {
		G_FreeEdict(ent);
		return;
	}

	//MATTHIAS - place flag nodes
	if (ctf->value)
	{
		if (strcmp(ent->classname, "item_flag_team1") == 0
			&& red_base == -1
			&& dntg->value)
		{
			vec3_t spot;

			VectorCopy (ent->s.origin, spot);
			spot[2] += 5;

			Bot_PlaceNode(spot, REDFLAG_NODE, 0);
			red_base = numnodes;
		}
		else if(strcmp(ent->classname, "item_flag_team2") == 0
			&& blue_base == -1
			&& dntg->value)
		{
			vec3_t spot;

			VectorCopy (ent->s.origin, spot);
			spot[2] += 5;

			Bot_PlaceNode(spot, BLUEFLAG_NODE, 0);
			blue_base = numnodes;
		}
	}

//MATTHIAS - Weapon/Item banning

	if (strcmp(ent->classname, "item_armor_body") == 0 && ban_bodyarmor->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "item_armor_combat") == 0 && ban_combatarmor->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "item_armor_jacket") == 0 && ban_jacketarmor->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "item_armor_shard") == 0 && ban_armorshard->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "item_power_screen") == 0 && ban_powerscreen->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "item_power_shield") == 0 && ban_powershield->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "ammo_cells") == 0 && ban_ammo_cells->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "item_invulnerability") == 0 && ban_invulnerability->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "item_breather") == 0 && ban_rebreather->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "item_enviro") == 0 && ban_environmentsuit->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}

//MATTHIAS	- Weapon/Item exchange

	if (strcmp(ent->classname, "ammo_shells") == 0)
	{
		if (random() < 0.3)
		{
			item = it_eshells;
			ent->classname = "ammo_explosiveshells";
		}
	}
	else if (strcmp(ent->classname, "item_quad") == 0
		|| strcmp(ent->classname, "item_silencer") == 0
		|| strcmp(ent->classname, "item_adrenaline") == 0
		|| strcmp(ent->classname, "item_invisibility") == 0
		|| strcmp(ent->classname, "item_jet") == 0
		|| strcmp(ent->classname, "item_grapple") == 0)
	{
		rn = random();
		if (rn < 0.1)
		{
			item = FindItem("Quad Damage");
			ent->classname = "item_quad";
		}
		else if (rn >= 0.1 && rn < 0.2)
		{
			item = FindItem("Silencer");
			ent->classname = "item_silencer";
		}
		else if (rn >= 0.2 && rn < 0.3)
		{
			item = FindItem("Adrenaline");
			ent->classname = "item_adrenaline";
		}
		else if (rn >= 0.3 && rn < 0.5)
		{
			item = FindItem("Invisibility");
			ent->classname = "item_invisibility";
		}
		else if (rn >= 0.5 && rn < 0.75)
		{
			item = FindItem("Jetpack");
			ent->classname = "item_jet";
		}
		else
		{
			item = it_grapple;
			ent->classname = "item_grapple";
		}
	}
	else if (strcmp(ent->classname, "ammo_bullets") == 0)
	{
		rn = random();
		if (rn < 0.4)
		{
			item = it_arrows;
			ent->classname = "ammo_arrows";
		}
		else if (rn >= 0.4 && rn < 0.7)
		{
			item = it_poisonarrows;
			ent->classname = "ammo_poisonarrows";
		}
		else if (rn >= 0.7)
		{
			item = it_explosivearrows;
			ent->classname = "ammo_explosivearrows";
		}
	}
	else if (strcmp(ent->classname, "ammo_grenades") == 0)
	{
		rn = random();

		if ( rn <= 0.2)
		{
			item = it_lasermines;
			ent->classname = "ammo_lasermines";
		}
		else if ( rn > 0.2 && rn <= 0.4)
		{
			item = it_flashgrenades;
			ent->classname = "ammo_flashgrenades";
		}
		else if ( rn > 0.4 && rn <= 0.6)
		{
			item = it_poisongrenades;
			ent->classname = "ammo_poisongrenades";
		}
		else if ( rn > 0.6 && rn <= 0.8)
		{
			item = it_proxymines;
			ent->classname = "ammo_proxymines";
		}
	}
	else if (strcmp(ent->classname, "ammo_rockets") == 0)
	{
		if (random() < 0.3)
		{
			item = it_homings;
			ent->classname = "ammo_homing";
		}
	}
	else if (strcmp(ent->classname, "ammo_slugs") == 0)
	{
		if (random() < 0.5)
		{
			item = it_buzzes;
			ent->classname = "ammo_buzzes";
		}
	}
	else if (strcmp(ent->classname, "weapon_shotgun") == 0)
	{
		if ( random() <= 0.5)
		{
			item = it_sword;
			ent->classname = "weapon_sword";
		}
		else
		{
			item = it_chainsaw;
			ent->classname = "weapon_chainsaw";
		}
	}
	else if (strcmp(ent->classname, "weapon_machinegun") == 0)
	{
		item = it_crossbow;
		ent->classname = "weapon_crossbow";
	}
	else if (strcmp(ent->classname, "weapon_chaingun") == 0)
	{
		item = it_airfist;
		ent->classname = "weapon_airfist";
	}
	else if (strcmp(ent->classname, "weapon_grenadelauncher") == 0)
	{
		if (random() < 0.3)
		{
			item = it_proxyminelauncher;
			ent->classname = "weapon_proxyminelauncher";
		}
	}
	else if (strcmp(ent->classname, "weapon_railgun") == 0)
	{
		if (random() < 0.5)
		{
			item = it_buzzsaw;
			ent->classname = "weapon_buzzsaw";
		}
	}
	else if (strcmp(ent->classname, "weapon_bfg") == 0)
	{
		rn = random();

		if ( rn <= 0.4)
		{
			item = it_vortex;
			ent->classname = "ammo_vortex";
		}
	}

//MATTHIAS - Weapon banning

	if (strcmp(ent->classname, "weapon_sword") == 0 && ban_sword->value > 0)
	{
		if (ban_chainsaw->value > 0) //banned,too
		{
			G_FreeEdict (ent);
			return;
		}
		else
		{
			item = it_chainsaw;
			ent->classname = "weapon_chainsaw";
		}
	}
	else if (strcmp(ent->classname, "weapon_supershotgun") == 0 && ban_supershotgun->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "weapon_crossbow") == 0 && ban_crossbow->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "weapon_airfist") == 0 && ban_airgun->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "weapon_grenadelauncher") == 0 && ban_grenadelauncher->value > 0)
	{
		if (ban_proxylauncher->value > 0) //banned,too
		{
			G_FreeEdict (ent);
			return;
		}
		else
		{
			item = it_proxyminelauncher;
			ent->classname = "weapon_proxyminelauncher";
		}
	}
	else if (strcmp(ent->classname, "weapon_rocketlauncher") == 0 && ban_rocketlauncher->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "weapon_hyperblaster") == 0 && ban_hyperblaster->value > 0)
	{
		G_FreeEdict (ent);
		return;
	}
	else if (strcmp(ent->classname, "weapon_railgun") == 0 && ban_railgun->value > 0)
	{
		if (ban_buzzsaw->value > 0) //banned,too
		{
			G_FreeEdict (ent);
			return;
		}
		else
		{
			item = it_buzzsaw;
			ent->classname = "weapon_buzzsaw";
		}
	}
	else if (strcmp(ent->classname, "weapon_buzzsaw") == 0 && ban_buzzsaw->value > 0)
	{
		if (ban_railgun->value > 0) //banned,too
		{
			G_FreeEdict (ent);
			return;
		}
		else
		{
			item = it_railgun;
			ent->classname = "weapon_railgun";
		}
	}
	//BFG
	else if (strcmp(ent->classname, "weapon_bfg") == 0 && ban_bfg->value > 0)
	{
		if (ban_vortex->value == 0)
		{
			item = it_vortex;
			ent->classname = "ammo_vortex";
		}
		else if (ban_defenceturret->value == 0)
		{
			item = it_lturret;
			ent->classname = "ammo_laserturret";
		}
		else if (ban_rocketturret->value == 0)
		{
			item = it_rturret;
			ent->classname = "ammo_rocketturret";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	//Vortex
	else if (strcmp(ent->classname, "ammo_vortex") == 0 && ban_vortex->value > 0)
	{
		if (ban_defenceturret->value == 0)
		{
			item = it_lturret;
			ent->classname = "ammo_laserturret";
		}
		else if (ban_rocketturret->value == 0)
		{
			item = it_rturret;
			ent->classname = "ammo_rocketturret";
		}
		else if (ban_bfg->value == 0)
		{
			item = it_bfg;
			ent->classname = "weapon_bfg";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	//defence turret
	else if (strcmp(ent->classname, "ammo_laserturret") == 0 && ban_defenceturret->value > 0)
	{
		if (ban_rocketturret->value == 0)
		{
			item = it_rturret;
			ent->classname = "ammo_rocketturret";
		}
		else if (ban_bfg->value == 0)
		{
			item = it_bfg;
			ent->classname = "weapon_bfg";
		}
		else if (ban_vortex->value == 0)
		{
			item = it_vortex;
			ent->classname = "ammo_vortex";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	//rocket turret
	else if (strcmp(ent->classname, "ammo_rocketturret") == 0 && ban_rocketturret->value > 0)
	{
		if (ban_bfg->value == 0)
		{
			item = it_bfg;
			ent->classname = "weapon_bfg";
		}
		else if (ban_vortex->value == 0)
		{
			item = it_vortex;
			ent->classname = "ammo_vortex";
		}
		else if (ban_defenceturret->value == 0)
		{
			item = it_lturret;
			ent->classname = "ammo_laserturret";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "ammo_grenades") == 0 && ban_ammo_grenades->value > 0)
	{
		if (ban_ammo_flashgrenades->value == 0)
		{
			item = it_flashgrenades;
			ent->classname = "ammo_flashgrenades";
		}
		else if (ban_ammo_lasergrenades->value == 0)
		{
			item = it_lasermines;
			ent->classname = "ammo_lasermines";
		}
		else if (ban_ammo_poisongrenades->value == 0)
		{
			item = it_poisongrenades;
			ent->classname = "ammo_poisongrenades";
		}
		else if (ban_ammo_proximitymines->value == 0)
		{
			item = it_proxymines;
			ent->classname = "ammo_proxymines";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "ammo_flashgrenades") == 0 && ban_ammo_flashgrenades->value > 0)
	{
		if (ban_ammo_lasergrenades->value == 0)
		{
			item = it_lasermines;
			ent->classname = "ammo_lasermines";
		}
		else if (ban_ammo_poisongrenades->value == 0)
		{
			item = it_poisongrenades;
			ent->classname = "ammo_poisongrenades";
		}
		else if (ban_ammo_proximitymines->value == 0)
		{
			item = it_proxymines;
			ent->classname = "ammo_proxymines";
		}
		else if (ban_ammo_grenades->value == 0)
		{
			item = it_grenades;
			ent->classname = "ammo_grenades";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "ammo_lasermines") == 0 && ban_ammo_lasergrenades->value > 0)
	{
		if (ban_ammo_poisongrenades->value == 0)
		{
			item = it_poisongrenades;
			ent->classname = "ammo_poisongrenades";
		}
		else if (ban_ammo_proximitymines->value == 0)
		{
			item = it_proxymines;
			ent->classname = "ammo_proxymines";
		}
		else if (ban_ammo_grenades->value == 0)
		{
			item = it_grenades;
			ent->classname = "ammo_grenades";
		}
		else if (ban_ammo_flashgrenades->value == 0)
		{
			item = it_flashgrenades;
			ent->classname = "ammo_flashgrenades";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "ammo_poisongrenades") == 0 && ban_ammo_poisongrenades->value > 0)
	{
		if (ban_ammo_proximitymines->value == 0)
		{
			item = it_proxymines;
			ent->classname = "ammo_proxymines";
		}
		else if (ban_ammo_grenades->value == 0)
		{
			item = it_grenades;
			ent->classname = "ammo_grenades";
		}
		else if (ban_ammo_flashgrenades->value == 0)
		{
			item = it_flashgrenades;
			ent->classname = "ammo_flashgrenades";
		}
		else if (ban_ammo_lasergrenades->value == 0)
		{
			item = it_lasermines;
			ent->classname = "ammo_lasermines";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "ammo_proxymines") == 0 && ban_ammo_proximitymines->value > 0)
	{
		
		if (ban_ammo_grenades->value == 0)
		{
			item = it_grenades;
			ent->classname = "ammo_grenades";
		}
		else if (ban_ammo_flashgrenades->value == 0)
		{
			item = it_flashgrenades;
			ent->classname = "ammo_flashgrenades";
		}
		else if (ban_ammo_lasergrenades->value == 0)
		{
			item = it_lasermines;
			ent->classname = "ammo_lasermines";
		}
		else if (ban_ammo_poisongrenades->value == 0)
		{
			item = it_poisongrenades;
			ent->classname = "ammo_poisongrenades";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "ammo_shells") == 0 && ban_ammo_shells->value > 0)
	{
		if (ban_ammo_explosiveshells->value > 0) //banned,too
		{
			G_FreeEdict (ent);
			return;
		}
		else
		{
			item = it_eshells;
			ent->classname = "ammo_explosiveshells";
		}
	}
	else if (strcmp(ent->classname, "ammo_explosiveshells") == 0 && ban_ammo_explosiveshells->value > 0)
	{
		if (ban_ammo_shells->value > 0) //banned,too
		{
			G_FreeEdict (ent);
			return;
		}
		else
		{
			item = it_shells;
			ent->classname = "ammo_shells";
		}
	}
	else if (strcmp(ent->classname, "ammo_rockets") == 0 && ban_ammo_rockets->value > 0)
	{
		if (ban_ammo_homingmissiles->value > 0) //banned,too
		{
			G_FreeEdict (ent);
			return;
		}
		else
		{
			item = it_homings;
			ent->classname = "ammo_homing";
		}
	}
	else if (strcmp(ent->classname, "ammo_homing") == 0 && ban_ammo_homingmissiles->value > 0)
	{
		if (ban_ammo_rockets->value > 0) //banned,too
		{
			G_FreeEdict (ent);
			return;
		}
		else
		{
			item = it_rockets;
			ent->classname = "ammo_rockets";
		}
	}
	else if (strcmp(ent->classname, "ammo_slugs") == 0 && ban_ammo_slugs->value > 0)
	{
		if (ban_ammo_buzzes->value > 0) //banned,too
		{
			G_FreeEdict (ent);
			return;
		}
		else
		{
			item = it_buzzes;
			ent->classname = "ammo_buzzes";
		}
	}
	else if (strcmp(ent->classname, "ammo_buzzes") == 0 && ban_ammo_buzzes->value > 0)
	{
		if (ban_ammo_slugs->value > 0) //banned,too
		{
			G_FreeEdict (ent);
			return;
		}
		else
		{
			item = it_slugs;
			ent->classname = "ammo_slugs";
		}
	}
	else if (strcmp(ent->classname, "ammo_arrows") == 0 && ban_ammo_arrows->value > 0)
	{
		if (ban_ammo_poisonarrows->value == 0)
		{
			item = it_poisonarrows;
			ent->classname = "ammo_poisonarrows";
		}
		else if (ban_ammo_explosivearrows->value == 0)
		{
			item = it_explosivearrows;
			ent->classname = "ammo_explosivearrows";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "ammo_poisonarrows") == 0 && ban_ammo_poisonarrows->value > 0)
	{
		
		if (ban_ammo_explosivearrows->value == 0)
		{
			item = it_explosivearrows;
			ent->classname = "ammo_explosivearrows";
		}
		else if (ban_ammo_arrows->value == 0)
		{
			item = it_arrows;
			ent->classname = "ammo_arrows";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "ammo_explosivearrows") == 0 && ban_ammo_explosivearrows->value > 0)
	{
		if (ban_ammo_arrows->value == 0)
		{
			item = it_arrows;
			ent->classname = "ammo_arrows";
		}
		else if (ban_ammo_poisonarrows->value == 0)
		{
			item = it_poisonarrows;
			ent->classname = "ammo_poisonarrows";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "item_quad") == 0 && ban_quaddamage->value > 0)
	{
		if (ban_jetpack->value == 0)
		{
			item = FindItem("Jetpack");
			ent->classname = "item_jet";
		}
		else if (ban_grapple->value == 0)
		{
			item = it_grapple;
			ent->classname = "item_grapple";
		}
		else if (ban_invisibility->value == 0)
		{
			item = FindItem("Invisibility");
			ent->classname = "item_invisibility";
		}
		else if (ban_adrenaline->value == 0)
		{
			item = FindItem("Adrenaline");
			ent->classname = "item_adrenaline";
		}
		else if (ban_silencer->value == 0)
		{
			item = FindItem("Silencer");
			ent->classname = "item_silencer";
		}		
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "item_jet") == 0 && ban_jetpack->value > 0)
	{
		if (ban_grapple->value == 0)
		{
			item = it_grapple;
			ent->classname = "item_grapple";
		}
		else if (ban_invisibility->value == 0)
		{
			item = FindItem("Invisibility");
			ent->classname = "item_invisibility";
		}
		else if (ban_adrenaline->value == 0)
		{
			item = FindItem("Adrenaline");
			ent->classname = "item_adrenaline";
		}
		else if (ban_silencer->value == 0)
		{
			item = FindItem("Silencer");
			ent->classname = "item_silencer";
		}
		else if (ban_quaddamage->value == 0)
		{
			item = FindItem("Quad Damage");
			ent->classname = "item_quad";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "item_grapple") == 0 && ban_grapple->value > 0)
	{
		if (ban_invisibility->value == 0)
		{
			item = FindItem("Invisibility");
			ent->classname = "item_invisibility";
		}
		else if (ban_adrenaline->value == 0)
		{
			item = FindItem("Adrenaline");
			ent->classname = "item_adrenaline";
		}
		else if (ban_silencer->value == 0)
		{
			item = FindItem("Silencer");
			ent->classname = "item_silencer";
		}
		else if (ban_quaddamage->value == 0)
		{
			item = FindItem("Quad Damage");
			ent->classname = "item_quad";
		}
		else if (ban_jetpack->value == 0)
		{
			item = FindItem("Jetpack");
			ent->classname = "item_jet";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "item_invisibility") == 0 && ban_invisibility->value > 0)
	{
		if (ban_adrenaline->value == 0)
		{
			item = FindItem("Adrenaline");
			ent->classname = "item_adrenaline";
		}
		else if (ban_silencer->value == 0)
		{
			item = FindItem("Silencer");
			ent->classname = "item_silencer";
		}
		else if (ban_quaddamage->value == 0)
		{
			item = FindItem("Quad Damage");
			ent->classname = "item_quad";
		}
		else if (ban_jetpack->value == 0)
		{
			item = FindItem("Jetpack");
			ent->classname = "item_jet";
		}
		else if (ban_invisibility->value == 0)
		{
			item = it_grapple;
			ent->classname = "item_grapple";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "item_adrenaline") == 0 && ban_adrenaline->value > 0)
	{
		if (ban_silencer->value == 0)
		{
			item = FindItem("Silencer");
			ent->classname = "item_silencer";
		}
		else if (ban_quaddamage->value == 0)
		{
			item = FindItem("Quad Damage");
			ent->classname = "item_quad";
		}
		else if (ban_jetpack->value == 0)
		{
			item = FindItem("Jetpack");
			ent->classname = "item_jet";
		}
		else if (ban_invisibility->value == 0)
		{
			item = it_grapple;
			ent->classname = "item_grapple";
		}
		else if (ban_adrenaline->value == 0)
		{
			item = FindItem("Invisibility");
			ent->classname = "item_invisibility";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}
	else if (strcmp(ent->classname, "item_silencer") == 0 && ban_silencer->value > 0)
	{
		if (ban_quaddamage->value == 0)
		{
			item = FindItem("Quad Damage");
			ent->classname = "item_quad";
		}
		else if (ban_jetpack->value == 0)
		{
			item = FindItem("Jetpack");
			ent->classname = "item_jet";
		}
		else if (ban_invisibility->value == 0)
		{
			item = it_grapple;
			ent->classname = "item_grapple";
		}
		else if (ban_adrenaline->value == 0)
		{
			item = FindItem("Invisibility");
			ent->classname = "item_invisibility";
		}
		else if (ban_adrenaline->value == 0)
		{
			item = FindItem("Adrenaline");
			ent->classname = "item_adrenaline";
		}
		else
		{
			G_FreeEdict (ent);
			return;
		}
	}

	VectorCopy(ent->s.origin, ent->spawnorigin);

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;

	//MATTHIAS
	

	if (ent->model)
		gi.modelindex (ent->model);

	AddItemToList(ent);

//ZOID
//flags are server animated and have special handling
	if (strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0) {
		ent->think = CTFFlagSetup;
	}

}

//======================================================================

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
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/body/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_bodyarmor",
/* pickup */	"Body Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		&bodyarmor_info,
		ARMOR_BODY,
/* precache */ ""
	},

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_combat", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/combat/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_combatarmor",
/* pickup */	"Combat Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		&combatarmor_info,
		ARMOR_COMBAT,
/* precache */ ""
	},

/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_jacket", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/jacket/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_jacketarmor",
/* pickup */	"Jacket Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		&jacketarmor_info,
		ARMOR_JACKET,
/* precache */ ""
	},

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_shard", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar2_pkup.wav",
		"models/items/armor/shard/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_jacketarmor",
/* pickup */	"Armor Shard",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		NULL,
		ARMOR_SHARD,
/* precache */ ""
	},


/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_power_screen", 
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/screen/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_powerscreen",
/* pickup */	"Power Screen",
/* width */		0,
		60,
		NULL,
		IT_ARMOR,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_power_shield",
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/shield/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_powershield",
/* pickup */	"Power Shield",
/* width */		0,
		60,
		NULL,
		IT_ARMOR,
		NULL,
		0,
/* precache */ "misc/power2.wav misc/power1.wav"
	},


	//
	// WEAPONS 
	//

/* weapon_ak42 (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_ak42", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_AK42,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_ak42/tris.md2",
/* icon */		"w_ak42",
/* pickup */	"AK42 Assault Pistol",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */
	},

/*QUAKED weapon_sword (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_sword", 
		Pickup_NoAmmoWeapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Sword,
		"misc/w_pkup.wav",
		"models/weapons/g_sword/tris.md2", EF_ROTATE,
		"models/weapons/v_sword/tris.md2",
/* icon */		"w_sword",
/* pickup */	"Bastard Sword",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */
	},

/*QUAKED weapon_chainsaw (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chainsaw", 
		Pickup_NoAmmoWeapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Chainsaw,
		"misc/w_pkup.wav",
		"models/weapons/g_chsaw/tris.md2", EF_ROTATE,
		"models/weapons/v_chsaw/tris.md2",
/* icon */		"w_chainsaw",
/* pickup */	"Chainsaw",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */
	},

/*QUAKED weapon_crossbow (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_crossbow", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Crossbow,
		"misc/w_pkup.wav",
		"models/weapons/g_bow/tris.md2", EF_ROTATE,
		"models/weapons/v_bow/tris.md2",
/* icon */		"w_bow",
/* pickup */	"Crossbow",
		0,
		1,
		"Arrows",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "crossbow/fly.wav crossbow/release1.wav crossbow/hit1.wav crossbow/hit3.wav crossbow/catch.wav"
	},

/*QUAKED weapon_posioncrossbow (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_poisoncrossbow", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_PoisonCrossbow,
		"misc/w_pkup.wav",
		"models/weapons/g_pbow/tris.md2", EF_ROTATE,
		"models/weapons/v_pbow/tris.md2",
/* icon */		"w_pbow",
/* pickup */	"Poison Crossbow",
		0,
		1,
		"Poison Arrows",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "crossbow/fly.wav crossbow/release1.wav crossbow/hit1.wav crossbow/hit3.wav crossbow/catch.wav"
	},

/*QUAKED weapon_explosivecrossbow (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_explosivecrossbow", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ExplosiveCrossbow,
		"misc/w_pkup.wav",
		"models/weapons/g_ebow/tris.md2", EF_ROTATE,
		"models/weapons/v_ebow/tris.md2",
/* icon */		"w_ebow",
/* pickup */	"Explosive Crossbow",
		0,
		1,
		"Explosive Arrows",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "crossbow/fly.wav crossbow/release1.wav crossbow/hit1.wav crossbow/hit3.wav crossbow/catch.wav"
	},


/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Shotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
/* icon */		"w_shotgun",
/* pickup */	"Shotgun",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav"
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_supershotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_SuperShotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
/* icon */		"w_sshotgun",
/* pickup */	"Super Shotgun",
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */
	},

/*QUAKED weapon_explosivesupershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_explosivesupershotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ExplosiveSuperShotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_eshot2/tris.md2", EF_ROTATE,
		"models/weapons/v_eshot2/tris.md2",
/* icon */		"w_esshotgun",
/* pickup */	"Explosive Super Shotgun",
		0,
		2,
		"Explosive Shells",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_machinegun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Machinegun,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_machn/tris.md2",
/* icon */		"w_machinegun",
/* pickup */	"Machinegun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chaingun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Chaingun,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"Chaingun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav"
	},

/*QUAKED weapon_airfist (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_airfist", 
		Pickup_NoAmmoWeapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Airfist,
		"misc/w_pkup.wav",
		"models/weapons/g_air/tris.md2", EF_ROTATE,
		"models/weapons/v_air/tris.md2",
/* icon */		"w_airfist",
/* pickup */	"Airgun",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "air/agfail.wav air/agfire.wav air/agwater.wav air/fly.wav"
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades1",
/* pickup */	"Grenades",
/* width */		3,
		5,
		"grenades",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_GRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED ammo_flashgrenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_flashgrenades",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_FlashGrenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/flashg/tris.md2", 0,
		"models/weapons/v_flashg/tris.md2",
/* icon */		"a_fgrenades",
/* pickup */	"Flash Grenades",
/* width */		3,
		5,
		"flash grenades",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_FLASHGRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED ammo_lasermines (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_lasermines",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_LaserGrenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/laserg/tris.md2", 0,
		"models/weapons/v_laserg/tris.md2",
/* icon */		"a_lmines",
/* pickup */	"Laser Mines",
/* width */		3,
		5,
		"laser mines",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_LASERGRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

	/*QUAKED ammo_poisongrenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_poisongrenades",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_PoisonGrenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/poison/tris.md2", 0,
		"models/weapons/v_poison/tris.md2",
/* icon */		"a_pgrenades",
/* pickup */	"Poison Grenades",
/* width */		3,
		5,
		"poison grenades",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_POISONGRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

	/*QUAKED ammo_proxymines (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_proxymines",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/proxy/tris.md2", 0,
		NULL,
/* icon */		"a_xgrenades",
/* pickup */	"Proximity Mines",
/* width */		3,
		5,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_PROXYMINES,
/* precache */
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_GrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher1",
/* pickup */	"Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

/*QUAKED weapon_flashgrenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_flashgrenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_FlashGrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_flalau/tris.md2", EF_ROTATE,
		"models/weapons/v_flalau/tris.md2",
/* icon */		"w_flauncher",
/* pickup */	"Flash Grenade Launcher",
		0,
		1,
		"Flash Grenades",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

/*QUAKED weapon_poisongrenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_poisongrenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_PoisonGrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_poilau/tris.md2", EF_ROTATE,
		"models/weapons/v_poilau/tris.md2",
/* icon */		"w_plauncher",
/* pickup */	"Poison Grenade Launcher",
		0,
		1,
		"Poison Grenades",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

/*QUAKED weapon_proxyminelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_proxyminelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ProxyMineLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_proxyl/tris.md2", EF_ROTATE,
		"models/weapons/v_proxyl/tris.md2",
/* icon */		"w_xlauncher",
/* pickup */	"Proxy Mine Launcher",
		0,
		1,
		"Proximity Mines",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Rocket Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

/*QUAKED weapon_hominglauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hominglauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_HomingLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_guided/tris.md2", EF_ROTATE,
		"models/weapons/v_guided/tris.md2",
/* icon */		"w_grlauncher",
/* pickup */	"Homing Missile Launcher",
		0,
		1,
		"Homing Missiles",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hyperblaster", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_HyperBlaster,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */	"HyperBlaster",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Railgun,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Railgun",
		0,
		1,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav"
	},

/*QUAKED weapon_buzzsaw (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_buzzsaw", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Buzzsaw,
		"misc/w_pkup.wav",
		"models/weapons/g_buzzsw/tris.md2", EF_ROTATE,
		"models/weapons/v_buzzsw/tris.md2",
/* icon */		"w_buzzsaw",
/* pickup */	"Buzzsaw",
		0,
		1,
		"Buzzes",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BFG,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"BFG10K",
		0,
		50,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav"
	},

/*QUAKED ammo_vortex (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_vortex",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Vortex,
		"misc/am_pkup.wav",
		"models/objects/rings/tris.md2", EF_ROTATE,
		"models/weapons/v_vortx/tris.md2",
/* icon */		"a_vortex",
/* pickup */	"Gravity Vortex",
/* width */		3,
		1,
		"Gravity Vortex",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_VORTEX,
/* precache */
	},

/*QUAKED ammo_laserturret (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_laserturret",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_LaserTurret,
		"misc/am_pkup.wav",
		"models/items/ammo/lturret/tris.md2", 0,
		"models/weapons/v_ltur/tris.md2",
/* icon */		"a_lturret",
/* pickup */	"Automatic Defence Turret",
/* width */		3,
		1,
		"Automatic Defence Turret",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_LTURRET,
/* precache */
	},

/*QUAKED ammo_rocketturret (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rocketturret",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_RocketTurret,
		"misc/am_pkup.wav",
		"models/items/ammo/rturret/tris.md2", 0,
		"models/weapons/v_rtur/tris.md2",
/* icon */		"a_rturret",
/* pickup */	"Automatic Rocket Turret",
/* width */		3,
		1,
		"Automatic Rocket Turret",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_RTURRET,
/* precache */
	},

	//
	// AMMO ITEMS
	//

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_shells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/shells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_shells",
/* pickup */	"Shells",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},

/*QUAKED ammo_explosiveshells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_explosiveshells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/bullets/explo/tris.md2", 0,
		NULL,
/* icon */		"a_eshells",
/* pickup */	"Explosive Shells",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_EXPLOSIVESHELLS,
/* precache */ ""
	},

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bullets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/bullets/medium/tris.md2", 0,
		NULL,
/* icon */		"a_bullets",
/* pickup */	"Bullets",
/* width */		3,
		50,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

/*QUAKED ammo_arrows (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_arrows",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/arrows/tris.md2", 0,
		NULL,
/* icon */		"a_arrows",
/* pickup */	"Arrows",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_ARROWS,
/* precache */ ""
	},

/*QUAKED ammo_poisonarrows (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_poisonarrows",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/parrows/tris.md2", 0,
		NULL,
/* icon */		"a_parrows",
/* pickup */	"Poison Arrows",
/* width */		3,
		5,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_POISONARROWS,
/* precache */ ""
	},

/*QUAKED ammo_explosivearrows (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_explosivearrows",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/earrows/tris.md2", 0,
		NULL,
/* icon */		"a_earrows",
/* pickup */	"Explosive Arrows",
/* width */		3,
		5,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_EXPLOSIVEARROWS,
/* precache */ ""
	},

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_cells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/cells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_cells",
/* pickup */	"Cells",
/* width */		3,
		50,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rockets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/medium/tris.md2", 0,
		NULL,
/* icon */		"a_rockets",
/* pickup */	"Rockets",
/* width */		3,
		5,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

/*QUAKED ammo_homing (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_homing",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/guided/tris.md2", 0,
		NULL,
/* icon */		"a_grockets",
/* pickup */	"Homing Missiles",
/* width */		3,
		5,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_HOMING,
/* precache */ ""
	},

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_slugs",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/slugs/medium/tris.md2", 0,
		NULL,
/* icon */		"a_slugs",
/* pickup */	"Slugs",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SLUGS,
/* precache */ ""
	},

/*QUAKED ammo_buzzes (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_buzzes",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/buzz/tris.md2", 0,
		NULL,
/* icon */		"a_buzz",
/* pickup */	"Buzzes",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_BUZZES,
/* precache */ ""
	},


	//
	// POWERUP ITEMS
	//
/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_quad", 
		Pickup_Powerup,
		Use_Quad,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quaddama/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_quad",
/* pickup */	"Quad Damage",
/* width */		2,
		40,
		NULL,
		IT_POWERUP,
		NULL,
		0,
/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav"
	},

/*QUAKED item_jet (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
       {
         "item_jet",    
         Pickup_Powerup,
         Use_Jet,
         NULL,
         NULL,
         "items/pkup.wav",
         "models/items/jet/tris.md2", EF_ROTATE, 

         NULL,
       /* icon */   "p_jet",
       /* pickup */ "Jetpack",
       /* width */  2,
         40,
         NULL,
         IT_POWERUP,
         NULL,
         0,
       /* precache */ "hover/hovidle1.wav items/damage.wav items/damage2.wav items/damage3.wav"
       },

/*QUAKED item_grapple (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
       {
         "item_grapple",    
         Pickup_Powerup,
         NULL,
         NULL,
         NULL,
         "items/pkup.wav",
         "models/items/grapple/tris.md2", EF_ROTATE, 

         NULL,
       /* icon */   "i_grapple",
       /* pickup */ "Grappling Hook",
       /* width */  2,
         40,
         NULL,
         IT_POWERUP,
         NULL,
         0,
       /* precache */
       },

/*QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_invulnerability",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/invulner/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_invulnerability",
/* pickup */	"Invulnerability",
/* width */		2,
		300,
		NULL,
		IT_POWERUP,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
	},

/*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_silencer",
		Pickup_Powerup,
		Use_Silencer,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/silencer/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_silencer",
/* pickup */	"Silencer",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_invisibility (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_invisibility",
		Pickup_Powerup,
		Use_Invisibility,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/invis/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_invis",
/* pickup */	"Invisibility",
/* width */		2,
		40,
		NULL,
		IT_POWERUP,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_breather",
		Pickup_Powerup,
		Use_Breather,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/breather/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"Rebreather",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

/*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_enviro",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/enviro/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_envirosuit",
/* pickup */	"Environment Suit",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

/*QUAKED item_ancient_head (.3 .3 1) (-16 -16 -16) (16 16 16)
Special item that gives +2 to maximum health
*/
	{
		"item_ancient_head",
		Pickup_AncientHead,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/c_head/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_fixme",
/* pickup */	"Ancient Head",
/* width */		2,
		60,
		NULL,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16)
gives +1 to maximum health
*/
	{
		"item_adrenaline",
		Pickup_Adrenaline,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/adrenal/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_adrenaline",
/* pickup */	"Adrenaline",
/* width */		2,
		40,
		NULL,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_bandolier",
		Pickup_Bandolier,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/band/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_bandolier",
/* pickup */	"Bandolier",
/* width */		2,
		60,
		NULL,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_pack",
		Pickup_Pack,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/pack/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_pack",
/* pickup */	"Ammo Pack",
/* width */		2,
		180,
		NULL,
		0,
		NULL,
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
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/data_cd/tris.md2", EF_ROTATE,
		NULL,
		"k_datacd",
		"Data CD",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_power_cube (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN NO_TOUCH
warehouse circuits
*/
	{
		"key_power_cube",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/power/tris.md2", EF_ROTATE,
		NULL,
		"k_powercube",
		"Power Cube",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the entrance of jail3
*/
	{
		"key_pyramid",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
		"k_pyramid",
		"Pyramid Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the city computer
*/
	{
		"key_data_spinner",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/spinner/tris.md2", EF_ROTATE,
		NULL,
		"k_dataspin",
		"Data Spinner",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16)
security pass for the security level
*/
	{
		"key_pass",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"k_security",
		"Security Pass",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - blue
*/
	{
		"key_blue_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key/tris.md2", EF_ROTATE,
		NULL,
		"k_bluekey",
		"Blue Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - red
*/
	{
		"key_red_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/red_key/tris.md2", EF_ROTATE,
		NULL,
		"k_redkey",
		"Red Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
	{
		"key_commander_head",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/monsters/commandr/head/tris.md2", EF_GIB,
		NULL,
/* icon */		"k_comhead",
/* pickup */	"Commander's Head",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
	{
		"key_airstrike_target",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/target/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_airstrike",
/* pickup */	"Airstrike Marker",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
/* precache */ ""
	},

	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0,
		NULL,
/* icon */		"i_health",
/* pickup */	"Health",
/* width */		3,
		0,
		NULL,
		0,
		NULL,
		0,
/* precache */ ""
	},


//ZOID
/*QUAKED item_flag_team1 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flag_team1",
		CTFPickup_Flag,
		NULL,
		CTFDrop_Flag, //Should this be null if we don't want players to drop it manually?
		NULL,
		"ctf/flagtk.wav",
		"players/male/flag1.md2", EF_FLAG1,
		NULL,
/* icon */		"i_ctf1",
/* pickup */	"Red Flag",
/* width */		2,
		0,
		NULL,
		0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},

/*QUAKED item_flag_team2 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flag_team2",
		CTFPickup_Flag,
		NULL,
		CTFDrop_Flag, //Should this be null if we don't want players to drop it manually?
		NULL,
		"ctf/flagtk.wav",
		"players/male/flag2.md2", EF_FLAG2,
		NULL,
/* icon */		"i_ctf2",
/* pickup */	"Blue Flag",
/* width */		2,
		0,
		NULL,
		0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},

/* Resistance Tech */
	{
		"item_tech1",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, //Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/resistance/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech1",
/* pickup */	"Disruptor Shield",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		NULL,
		0,
/* precache */ "ctf/tech1.wav"
	},

/* Strength Tech */
	{
		"item_tech2",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, //Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/strength/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech2",
/* pickup */	"Power Amplifier",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		NULL,
		0,
/* precache */ "ctf/tech2.wav ctf/tech2x.wav"
	},

/* Haste Tech */
	{
		"item_tech3",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, //Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/haste/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech3",
/* pickup */	"Time Accel",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		NULL,
		0,
/* precache */ "ctf/tech3.wav"
	},

/* Regeneration Tech */
	{
		"item_tech4",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, //Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/regeneration/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech4",
/* pickup */	"AutoDoc",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		NULL,
		0,
/* precache */ "ctf/tech4.wav"
	},

//ZOID

	// end of list marker
	{NULL}
};


/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health (edict_t *self)
{
	if ((int)dmflags->value & DF_NO_HEALTH)
	{
		G_FreeEdict (self);
		return;
	}

	if (ban_health->value > 0)	//MATTHIAS
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/n_health.wav");
	AddItemToList(self);	//MATTHIAS
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_small (edict_t *self)
{
	if ((int)dmflags->value & DF_NO_HEALTH)
	{
		G_FreeEdict (self);
		return;
	}

	if (ban_health_small->value > 0)	//MATTHIAS
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/stimpack/tris.md2";
	self->count = 2;
	SpawnItem (self, FindItem ("Health"));
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex ("items/s_health.wav");
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_large (edict_t *self)
{
	if ((int)dmflags->value & DF_NO_HEALTH)
	{
		G_FreeEdict (self);
		return;
	}

	if (ban_health_large->value > 0)	//MATTHIAS
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/l_health.wav");
	AddItemToList(self);	//MATTHIAS
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_mega (edict_t *self)
{
	if ((int)dmflags->value & DF_NO_HEALTH)
	{
		G_FreeEdict (self);
		return;
	}

	if (ban_health_mega->value > 0)	//MATTHIAS
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;
	AddItemToList(self);	//MATTHIAS
}


void InitItems (void)
{
	game.num_items = sizeof(itemlist)/sizeof(itemlist[0]) - 1;
}

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

	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
	body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
	power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
	power_shield_index = ITEM_INDEX(FindItem("Power Shield"));

	//MATTHIAS

	it_shells = FindItem("shells");
	it_eshells = FindItem("explosive shells");
	it_cells = FindItem("cells");
	it_arrows = FindItem("arrows");
	it_poisonarrows = FindItem("poison arrows");
	it_explosivearrows = FindItem("explosive arrows");
	it_rockets = FindItem("rockets");
	it_homings = FindItem("homing missiles");
	it_slugs = FindItem("slugs");
	it_buzzes = FindItem("buzzes");
	it_grenades = FindItem("grenades");
	it_flashgrenades = FindItem("flash grenades");
	it_lasermines = FindItem("Laser Mines");
	it_poisongrenades = FindItem("poison grenades");
	it_proxymines = FindItem("proximity mines");

	it_ak42	= FindItem("AK42 Assault Pistol");
	it_sword	= FindItem("bastard sword");
	it_chainsaw	= FindItem("chainsaw");
	it_shotgun	= FindItem("shotgun");
	it_supershotgun	= FindItem("super shotgun");
	it_esupershotgun	= FindItem("explosive super shotgun");
	it_crossbow	= FindItem("crossbow");
	it_poisoncrossbow	= FindItem("poison crossbow");
	it_explosivecrossbow	= FindItem("explosive crossbow");
	it_airfist	= FindItem("airgun");
	it_grenadelauncher	= FindItem("grenade launcher");
	it_flashgrenadelauncher	= FindItem("flash grenade launcher");
	it_poisongrenadelauncher	= FindItem("poison grenade launcher");
	it_proxyminelauncher	= FindItem("proxy mine launcher");
	it_rocketlauncher	= FindItem("rocket launcher");
	it_hominglauncher	= FindItem("homing missile launcher");
	it_railgun	= FindItem("railgun");
	it_buzzsaw	= FindItem("buzzsaw");
	it_hyperblaster	= FindItem("hyperblaster");
	it_bfg	= FindItem("bfg10k");
	it_lturret	= FindItem("automatic defence turret");
	it_rturret	= FindItem("automatic rocket turret");
	it_vortex	= FindItem("gravity vortex");

	it_health		= FindItemByClassname("item_health");
	it_health_large	= FindItemByClassname("item_health_large");
	it_health_mega	= FindItemByClassname("item_health_mega");

	it_grapple	= FindItem("grappling hook");
	it_jetpack	= FindItem("jetpack");
}
