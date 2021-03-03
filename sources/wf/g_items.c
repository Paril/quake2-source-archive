#include "g_local.h"

//ERASER START
#include "g_items.h"
#include "bot_procs.h"
#include "p_trail.h"
int	CanReach(edict_t *self, edict_t *targ);
//ERASER END


qboolean Pickup_Weapon (edict_t *ent, edict_t *other);
void Use_Weapon (edict_t *ent, gitem_t *inv);
void Use_Grapple (edict_t *ent, gitem_t *inv);//5/99
void Drop_Weapon (edict_t *ent, gitem_t *inv);
void Create_Home_Base(edict_t *flag, int team);

void Weapon_Blaster (edict_t *ent);
void Weapon_BoltedBlaster (edict_t *ent);
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

//STWF Weapons
void Weapon_BulletGrenadeLauncher (edict_t *ent);
void Weapon_FlareLauncher (edict_t *ent);
void Weapon_TimedNukeLauncher (edict_t *ent);
void Weapon_Nag (edict_t *ent);
void Weapon_PelletRocketLauncher (edict_t *ent);
void Weapon_DiseaseGrenadeLauncher (edict_t *ent);
void Weapon_FlameThrower (edict_t *ent);
void Weapon_SHCRifle (edict_t *ent);
void Weapon_FlareGun (edict_t *ent);
void Weapon_FlareGun2 (edict_t *ent);
void Weapon_NapalmGrenadeLauncher (edict_t *ent);
void Weapon_RocketNapalmLauncher (edict_t *ent);
void Weapon_NormalRocketLauncher (edict_t *ent);
void Weapon_NailGun (edict_t *ent);
void Weapon_Lightninggun (edict_t *ent);
void Weapon_Needler (edict_t *ent);
void Weapon_Telsa (edict_t *ent);
void Weapon_ChainMBPC (edict_t *ent);
void Weapon_InfectedDartLauncher (edict_t *ent);
void Weapon_ArmorDartLauncher (edict_t *ent);
void Weapon_TranquilizerRifle (edict_t *ent);
void Weapon_RocketClusterLauncher (edict_t *ent);
void Weapon_Snipe (edict_t *ent);
void Weapon_ComputerGuidedProjectileLauncher (edict_t *ent);
void Weapon_SentryRocketLauncher (edict_t *ent);
void Weapon_MegaChaingun (edict_t *ent);
void Weapon_PoisonDartLauncher (edict_t *ent);
void Weapon_Knife (edict_t *ent);
void Weapon_AK47 (edict_t *ent);
void Weapon_Pistol (edict_t *ent);
void Weapon_StingerLauncher (edict_t *ent);
void Weapon_Freezer (edict_t *ent);//acrid 3/99

//=========
//Rogue Weapons
void Weapon_Disintegrator (edict_t *ent);
void Weapon_ETF_Rifle (edict_t *ent);
void Weapon_Heatbeam (edict_t *ent);
//Rogue Weapons
//=========

// RAFAEL
void Weapon_Ionripper (edict_t *ent);
void Weapon_Phalanx (edict_t *ent);
// RAFAEL


//Red = body, combat = yellow, jacket = green
//	                      { base count, max, norm, energy prot, armor type }
gitem_armor_t jacketarmor_info	= { 25, 100,  .30,         .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 200,  .60,         .30, ARMOR_COMBAT};
//gitem_armor_t bodyarmor_info	= {100, 200,  .80,         .60, ARMOR_BODY};
gitem_armor_t bodyarmor_info	= {100, 400,  .80,         .60, ARMOR_BODY};

int	jacket_armor_index;
int	combat_armor_index;
int	body_armor_index;
int	power_screen_index;
int	power_shield_index;

#define HEALTH_IGNORE_MAX		1
#define HEALTH_TIMED			2
#define HEALTH_MEGAMAX_HEALTH	200

void Use_Quad (edict_t *ent, gitem_t *item);
static int	quad_drop_timeout_hack;

extern char *tnames[];
//======================================================================

//======================================================================
void WFAddArmor (edict_t *other, int armorval)
{
	int index;

	if (!other->client) return;

	//What armor do they have?
	index = ArmorIndex (other);
	if (index && other->client->pers.inventory[index] < other->client->player_armor)
	{
		other->client->pers.inventory[index] += armorval;
		gi.sound (other, CHAN_VOICE, gi.soundindex ("misc/ar2_pkup.wav"), 1, ATTN_NORM, 0);
	}

	//If they don't have any armor, give them something if they are allowed it
	if (index == 0 && other->client->player_items & ITEM_JACKETARMOR)
	{
		index = ITEM_INDEX(FindItem("Jacket Armor"));
		if (index)
		{
			other->client->pers.inventory[index] += armorval;
			gi.sound (other, CHAN_VOICE, gi.soundindex ("misc/ar2_pkup.wav"), 1, ATTN_NORM, 0);
		}
	}

	//Don't allow greater than max armor for each class
	if (index != 0 && other->client->pers.inventory[index] > other->client->player_armor)
	{
		other->client->pers.inventory[index] = other->client->player_armor;
	}

}

//ERASER START
/*
===============
AddToItemList

  Adds an item to the respective linked list, returns the head of that list
===============
*/
edict_t	*AddToItemList(edict_t *ent, edict_t	*head)
{
	edict_t *trav;

	// make sure this item isn't already in the list
	trav = head;
	while (trav)
	{
		if (trav == ent)		// already in there!
			return head;

		trav = trav->node_target;
	}

	if (head)
		head->last_goal = ent;

	ent->node_target = head;
	ent->last_goal = NULL;

	return ent;
};

void	RemoveFromItemList(edict_t *ent)
{
	if (ent->node_target)
		ent->node_target->last_goal = ent->last_goal;

	if (ent->last_goal)
	{
		ent->last_goal->node_target = ent->node_target;
//		ent->last_goal->last_goal = NULL;
	}
	else if (ent->node_target)	// ent must be head
	{
		if (ent->item->pickup == Pickup_Weapon)
			weapons_head = ent->node_target;
		else if (ent->item->pickup == Pickup_Health)
			health_head = ent->node_target;
		else if (ent->item->pickup == Pickup_Ammo)
			ammo_head = ent->node_target;
		else
			bonus_head = ent->node_target;
	}

	ent->node_target = NULL;
	ent->last_goal = NULL;
};

void	RemoveDroppedItem(edict_t	*ent)
{

	//Is this ammo?
	if (ent->item->pickup == Pickup_Ammo) 
	{
		//if (wfdebug) gi.dprintf("Remove ammo. Dropped total = %d\n", TOTALDROPPEDAMMO);
		--TOTALDROPPEDAMMO;
	}
		

	RemoveFromItemList(ent);
	G_FreeEdict(ent);
};
//ERASER END

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

void DoRespawn (edict_t *ent)
{
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

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		if ((other->bot_client) || ((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))//ERASER
//		if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		}
	}

	return true;
}

void Drop_General (edict_t *ent, gitem_t *item)
{
	if (item->flags & IT_KEY)
	{
		safe_cprintf (ent, PRINT_HIGH, "You can't drop keys\n");
		return;
	}

	//Don't allow observer to drop anything
	if (ent->solid == SOLID_NOT)
	{
		safe_cprintf (ent, PRINT_HIGH, "You can't drop anything as an observer\n");
		return;
	}

	if (strcmp(ent->classname, "item_breather") == 0)
	{
		safe_cprintf (ent, PRINT_HIGH, "You can't drop the rebreather\n");
		return;
	}


	Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
}


//======================================================================

qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other)
{
	if (!deathmatch->value)
		other->max_health += 1;

	if (other->health < other->max_health)
		other->health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_AncientHead (edict_t *ent, edict_t *other)
{
	other->max_health += 2;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Bandolier (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

/* This is now handled with class specific ammo limits
	if (other->client->pers.max_bullets < 250)
		other->client->pers.max_bullets = 250;
	if (other->client->pers.max_shells < 150)
		other->client->pers.max_shells = 150;
	if (other->client->pers.max_cells < 250)
		other->client->pers.max_cells = 250;
	if (other->client->pers.max_slugs < 75)
		other->client->pers.max_slugs = 75;
*/

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}
//////////////////////////bot specials////////////////////////////////////
void place_turret (edict_t *other);
void UpgradeSentry(edict_t *other);
void PlaceBiosentry (edict_t *other);
void SP_SupplyDepot(edict_t *other);
void SP_HealingDepot(edict_t *other);


qboolean Place_Special (edict_t *ent, edict_t *other)
{
	//this is the pickup it should always return false //42
	if (strcmp(ent->classname, "item_sentryspot") == 0)
		if (ent->owner && !ent->owner->sentry)
			ent->owner = NULL;

	if (strcmp(ent->classname, "item_depotspot") == 0)
		if (ent->owner && !ent->owner->supply)
			ent->owner = NULL;

	if (ent->owner)
	{
        botDebugPrint("%s owns me g items\n",ent->owner->client->pers.netname);
		return false;
	}
	else botDebugPrint("noone owns me g items\n");

	if (strcmp(ent->classname, "item_sentryspot") == 0)
	{
		//for classes with sentry special
		if (!ent->owner && other->client->player_special & SPECIAL_SENTRY_GUN)
		{
            //makes the bot own the nodeitem
			ent->owner = other;
			place_turret (other);
			UpgradeSentry(other);
			if (bot_melee->value)
				UpgradeSentry(other);//alt for L3
		}

		//for classes with bio special
		if (!ent->owner && other->client->player_special & SPECIAL_BIOSENTRY)
		{
			ent->owner = other;
			PlaceBiosentry (other);
		}
	}

	if (strcmp(ent->classname, "item_depotspot") == 0)
	{
		if (!ent->owner && other->client->player_special & SPECIAL_SUPPLY_DEPOT)
		{
			ent->owner = other;
			SP_SupplyDepot(other);
		}

		if (!ent->owner && other->client->player_special & SPECIAL_HEALING)
		{
			ent->owner = other;
			SP_HealingDepot(other);
		}
	}

	return false;
}


/////////////////////////////bot specials end/////////////////////////////

qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;
	int old_armor_index;
	old_armor_index = ArmorIndex (other);

	//See if there is a team specific indicator
        //Team=9 means that any team can pick it up, but it will respawn in 5 seconds
        if ((ent->wf_team) && (ent->wf_team != other->wf_team) && (ent->wf_team != 9))
		return false;

//Acrid pack pickup fixes 5/99 stop the real player hogs
  if (!other->bot_client)
	if ((other->client->pers.inventory[ITEM_INDEX(item_bullets)] != other->client->pers.max_bullets)||
	    (other->client->pers.inventory[ITEM_INDEX(item_shells)] != other->client->pers.max_shells) ||
	    (other->client->pers.inventory[ITEM_INDEX(item_rockets)] != other->client->pers.max_rockets)||
	    (other->client->pers.inventory[ITEM_INDEX(item_grenades)] != other->client->pers.max_grenades)||
	    (other->client->pers.inventory[ITEM_INDEX(item_cells)] != other->client->pers.max_cells )  ||
	    (other->client->pers.inventory[ITEM_INDEX(item_slugs)] != other->client->pers.max_slugs) ||
		(other->health != other->max_health) ||
		(other->client->pers.inventory[old_armor_index] != other->client->player_armor))
		botDebugPrint("test\n");
		else 
		return false;

//removed old code here

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

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

	//WF - Added large health
	item = FindItem("Health");
	if ((item) && (other->health < other->max_health))
	{
		index = ITEM_INDEX(item);
		other->health += 20;
		if (other->health > other->max_health)
			other->health = other->max_health;
	}

	//WF - Added armor
	WFAddArmor(other, 20);

	//Set the respawn time.  If no team specified, use default.  If it is
	//a team pack, respawn in 5 seconds.
	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
        {
        if (ent->wf_team)
            SetRespawn (ent, 5);
        else
            SetRespawn (ent, ent->item->quantity);
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
//ERASER START
	if (ent->bot_client)
	{	// select best weapon for Quad
		botPickBestWeapon(ent);
	}
//ERASER END
}

//======================================================================

void Use_Breather (edict_t *ent, gitem_t *item)
{
//ERASER START
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return;
//ERASER END	REMOVED ALL ENT->
	client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (client->breather_framenum > level.framenum)
		client->breather_framenum += 300;
	else
		client->breather_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Envirosuit (edict_t *ent, gitem_t *item)
{
//ERASER START
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return;
//ERASER END REMOVED ALL ENT->
	client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (client->enviro_framenum > level.framenum)
		client->enviro_framenum += 300;
	else
		client->enviro_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void	Use_Invulnerability (edict_t *ent, gitem_t *item)
{
//ERASER START
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return;
//ERASER END REMOVED ALL ENT->
	client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (client->invincible_framenum > level.framenum)
		client->invincible_framenum += 300;
	else
		client->invincible_framenum = level.framenum + 300;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void	Use_Silencer (edict_t *ent, gitem_t *item)
{
//ERASER START
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return;
//ERASER END REMOVED ALL ENT->
	client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	client->silencer_shots += 30;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

qboolean Pickup_Key (edict_t *ent, edict_t *other)
{
//gi.dprintf("Key pickup. team = %d\n", ent->wf_team);

	//If this is team specifc, check for correct team
	if ((ent->wf_team) && (ent->wf_team != other->wf_team))
	{
		return false;
	}

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

	if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
	{
		safe_cprintf (ent, PRINT_HIGH, "You already have one of those.\n");
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 60);

	return true;
}

//======================================================================
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count)
{
	int			index;
	int			max;
	gclient_t	*client;//ERASER
//ERASER START
	if (ent->client)
		client = ent->client;
	else
		return false;
//WF24	if (!ent->client)
//WF24		return false;
//ERASER END REMOVED ALL ENT->
	if (item->tag == AMMO_BULLETS)
		max = client->pers.max_bullets;
	else if (item->tag == AMMO_SHELLS)
		max = client->pers.max_shells;
	else if (item->tag == AMMO_ROCKETS)
		max = client->pers.max_rockets;
	else if (item->tag == AMMO_GRENADES)
		max = client->pers.max_grenades;
	else if (item->tag == AMMO_CELLS)
		max = client->pers.max_cells;
	else if (item->tag == AMMO_SLUGS)
		max = client->pers.max_slugs;
	else
		return false;

	index = ITEM_INDEX(item);

	if (client->pers.inventory[index] == max)
		return false;

	client->pers.inventory[index] += count;

	if (client->pers.inventory[index] > max)
		client->pers.inventory[index] = max;

	return true;
}

qboolean Pickup_Ammo (edict_t *ent, edict_t *other)
{
	int			oldcount;
	int			count;
	qboolean	weapon;
	qboolean		canuse;

	//WF - Only allow certain classes to use certain ammo
	if (other->client && !other->bot_client)//ACRID FIXME FIXME FIXME
	{
		canuse = wf_CanUse(other->client, ent);
		if (canuse == false) return false;
	}


	weapon = (ent->item->flags & IT_WEAPON);
	if ( (weapon) && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		count = 1000;
	else if (ent->count)
		count = ent->count;
	else
		count = ent->item->quantity;

	oldcount = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (!Add_Ammo (other, ent->item, count))
	{
//ERASER START
		if (other->bot_client && (other->movetarget == ent))
		{	// ignore this item for a while
			ent->ignore_time = level.time + 3;
			other->movetarget = NULL;
			bot_roam(other, false);
		}
//ERASER END
		return false;
	}//E

	if (weapon && !oldcount)
	{
		if (other->client->pers.weapon != ent->item && ( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
			other->client->newweapon = ent->item;
	}

	//WF - If we are picking up dropped ammo, decrease max dropped ammo counter
	if (ent->spawnflags & DROPPED_ITEM)
	{
		--TOTALDROPPEDAMMO;
		if (TOTALDROPPEDAMMO < 0) TOTALDROPPEDAMMO = 0;

		//if (wfdebug) gi.dprintf("Picked up dropped ammo. Dropped total = %d\n", TOTALDROPPEDAMMO);
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 30);
	return true;
}
void Drop_Ammo (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	int		index;
//ERASER START
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return;
//ERASER END REMOVED ALL ENT->

	//WF - limit the number of dropped ammo items
	if (TOTALDROPPEDAMMO >= MAX_DROPPEDAMMO)
	{
		safe_cprintf (ent, PRINT_HIGH, "Only %d dropped ammo allowed in world at a time\n",MAX_DROPPEDAMMO);
	}
	else
	{
		++TOTALDROPPEDAMMO;
		index = ITEM_INDEX(item);
		dropped = Drop_Item (ent, item);
		if (client->pers.inventory[index] >= item->quantity)
			dropped->count = item->quantity;
		else
			dropped->count = client->pers.inventory[index];

		if (client->pers.weapon &&
			client->pers.weapon->tag == AMMO_GRENADES &&
			item->tag == AMMO_GRENADES &&
			client->pers.inventory[index] - dropped->count <= 0)
		{
			safe_cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
			G_FreeEdict(dropped);
			return;
		}

		client->pers.inventory[index] -= dropped->count;
		ValidateSelectedItem (ent);
	}
}


//======================================================================

void MegaHealth_think (edict_t *self)
{
	if (self->owner->health > self->owner->max_health
//ZOID
		&& !CTFHasRegeneration(self->owner)
//ZOID
		)
	{
		self->nextthink = level.time + 1;
		self->owner->health -= 1;
		return;
	}

	if (self->health > MAX_HEALTH)	self->health = MAX_HEALTH;//WF24 USES 200

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (self, 20);
	else
		G_FreeEdict (self);
}

qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
//	char *s;
	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->health >= other->max_health)
//ERASER START
		{
			if (other->bot_client && (other->movetarget == ent))
			{	// ignore this item for a while
				ent->ignore_time = level.time + 1;
				other->movetarget = NULL;
				bot_roam(other, false);
			}
//ERASER END
			return false;
		}//E

	if (other->health >= 250 && ent->count > 25)//ERASER ADDED
		return false;

	other->health += ent->count;
	// set skin for damage
//	s = Info_ValueForKey (other->client->pers.userinfo, "skin");
	//CHeck to see if CTF is set
//	if (ctf->value)
//		CTFAssignSkin(other, s);
//ZOID//ERASER ADDED
	if (other->health > 250 && ent->count > 25)
		other->health = 250;
//ZOID//ERASER ADDED
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
		if (other->health > HEALTH_MEGAMAX_HEALTH)
			other->health = HEALTH_MEGAMAX_HEALTH;
	}

//ZOID
	if ((ent->style & HEALTH_TIMED)
		&& !CTFHasRegeneration(other)
//ZOID
	)
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
		if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
			SetRespawn (ent, 30);
	}

	return true;
}


//======================================================================

int ArmorIndex (edict_t *ent)
{
//ERASER START
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return false;
//WF	if (!ent->client)
//WF		return 0;
//ERASER START REMOVED ALL ENT->
	if (client->pers.inventory[jacket_armor_index] > 0)
		return jacket_armor_index;

	if (client->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

	if (client->pers.inventory[body_armor_index] > 0)
		return body_armor_index;

	return 0;
}

qboolean Pickup_Armor (edict_t *ent, edict_t *other)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	//float			salvage;
	//int			salvagecount;
	int				index;//WF34
	gclient_t	*client;//ERASER
	qboolean		canuse;//WF24
//ERASER START
	if (other->client)
		client = other->client;
	else
		return false;
//ERASER END
	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;

	old_armor_index = ArmorIndex (other);


	// handle armor shards specially
	if (ent->item->tag == ARMOR_SHARD)
	{
		if (!old_armor_index)
		{
			// Does class allow jacket armor?
			if (other->client->player_items & ITEM_JACKETARMOR)
				other->client->pers.inventory[jacket_armor_index] = 2;
			else
				return false;
		}
		else
		{
			client->pers.inventory[old_armor_index] += 2;

			//Don't allow greater than max armor for each class
			if (other->client->pers.inventory[old_armor_index] > other->client->player_armor)
				other->client->pers.inventory[old_armor_index] = other->client->player_armor;
		}
		return true;//WF24
	}
//ACRID FIXME FIXME FIXME CANUSE WILL HAVE TO BE FIXED FOR BOTS SOMEHOW

	//Only allow certain classes to use different armor//WF24 DIF
	canuse = wf_CanUse(other->client, ent);

	if (canuse == false)
	{
		//safe_cprintf (other, PRINT_HIGH, "Sorry, your class can't use that type of armor.\n");
		return false;
	}

//Acrid Armor pickup fixes 5/99 stop the real player hogs
	if (client->pers.inventory[combat_armor_index] && (ent->item->tag == ARMOR_COMBAT))
		return false;

	if (client->pers.inventory[body_armor_index] && (ent->item->tag == ARMOR_COMBAT || ent->item->tag == ARMOR_BODY)  )
		return false;


	// if player has no armor, just use it
	if (!old_armor_index)
	{
		client->pers.inventory[ITEM_INDEX(ent->item)] = newinfo->base_count;
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

			// ++TeT Just add it to the old armor
			//salvage =  oldinfo->normal_protection / newinfo->normal_protection;
			//salvagecount = salvage * other->client->pers.inventory[old_armor_index];
			//newcount = newinfo->base_count + salvagecount;
			newcount = newinfo->base_count + other->client->pers.inventory[old_armor_index];
			//--TeT

			if (newcount > newinfo->max_count)
				newcount = newinfo->max_count;

			// zero count of old armor so it goes away
			client->pers.inventory[old_armor_index] = 0;

			// change armor to new item with computed value//FIXME OTHER
			index = ITEM_INDEX(ent->item);
			other->client->pers.inventory[index] = newcount;

			//Don't allow greater than max armor for each class
			if (other->client->pers.inventory[index] > other->client->player_armor)
				other->client->pers.inventory[index] = other->client->player_armor;

		}
		else
		{
			// calc new armor values

			//++TeT Just add it to the old armor
			//salvage = newinfo->normal_protection;//TeT / oldinfo->normal_protection;
			//salvagecount = salvage * newinfo->base_count;
			//newcount = other->client->pers.inventory[old_armor_index] + salvagecount;
			newcount = other->client->pers.inventory[old_armor_index] + newinfo->base_count;
			//--TeT

			if (newcount > oldinfo->max_count)
				newcount = oldinfo->max_count;

			// if we're already maxed out then we don't need the new armor
			if (client->pers.inventory[old_armor_index] >= newcount)
			{
//ERASER START
				if (other->bot_client && (other->movetarget == ent))
				{
					other->movetarget = NULL;
				}
				ent->ignore_time = level.time + 2;
//ERASER END
				return false;
			}

			// update current armor value//FIXME OTHERS
			other->client->pers.inventory[old_armor_index] = newcount;

			//Don't allow greater than max armor for each class
			if (other->client->pers.inventory[old_armor_index] > other->client->player_armor)
				other->client->pers.inventory[old_armor_index] = other->client->player_armor;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 20);

	return true;
}

//======================================================================

int PowerArmorType (edict_t *ent)
{
//ERASER START
	gclient_t	*client;

	if (ent->client)
		client = ent->client;
	else
		return POWER_ARMOR_NONE;
//WF24	if (!ent->client)
//WF24		return POWER_ARMOR_NONE;
//ERASER END

	if (!(ent->flags & FL_POWER_ARMOR))
		return POWER_ARMOR_NONE;

	if (client->pers.inventory[power_shield_index] > 0)
		return POWER_ARMOR_SHIELD;

	if (client->pers.inventory[power_screen_index] > 0)
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
			safe_cprintf (ent, PRINT_HIGH, "No cells for power armor.\n");
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

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		// auto-use for DM only if we didn't already have one
		if (!quantity)
			ent->item->use (other, ent->item);
	}

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
	gclient_t	*client;//ERASER
//ERASER START
//////////////////////////////////special node////////////////////////////
if (strcmp(ent->classname, "item_sentryspot") == 0)
{
	//to stop accidental touching
	if (other->bot_client && other->sentry && ent->item->pickup == Place_Special)
	{   botDebugPrint("bot cant touchthis\n");
	    return;
	}
	//to stop real player from touching
	if (!bot_debug_nodes->value)
	if (!other->bot_client && ent->item->pickup == Place_Special)
	{	botDebugPrint("I cant touch this\n");
	    return;
	}
	if (bot_debug_nodes->value)
	if (other->client && other->sentry && ent->item->pickup == Place_Special)
	{	botDebugPrint("I cant touch this\n");
	return;
	}
}
if (strcmp(ent->classname, "item_depotspot") == 0)
{
	//to stop accidental touching
	if (other->bot_client && other->supply && ent->item->pickup == Place_Special)
	{   botDebugPrint("bot cant touchthis\n");
	    return;
	}
	//to stop real player from touching
	if (!bot_debug_nodes->value)
	if (!other->bot_client && ent->item->pickup == Place_Special)
	{	botDebugPrint("I cant touch this\n");
	    return;
	}
	if (bot_debug_nodes->value)
	if (other->client && other->supply && ent->item->pickup == Place_Special)
	{	botDebugPrint("I cant touch this\n");
	return;
	}
}
////////////////////////////////special node end//////////////////////////

	if (other->bot_client)
	{
		if	(!(	 (ent->item->pickup == Pickup_Weapon)
			  || (ent->item->pickup == Pickup_Health)
			  || (ent->item->pickup == Pickup_Ammo)
			  || (ent->item->pickup == Pickup_Powerup)
			  || (ent->item->pickup == Pickup_Armor)
			  || (ent->item->pickup == Pickup_Pack)
			  || (ent->item->pickup == Pickup_Bandolier)
			  || (ent->item->pickup == Place_Special)//acrid 42
			  || (ent->item->pickup == CTFPickup_Flag)
			  || (ent->item->pickup == CTFPickup_Tech)))
		{
			return;
		}
		else if (other->movetarget == ent)	// if this was their movetarget, then clear it
		{
			other->movetarget = other->goalentity = NULL;

			// force search for new item next think
			other->last_nopaths_roam = 0;
			other->last_roam_time   = 0;
		}

	}

	if (other->client)
		client = other->client;
	else
		return;
//ERASER END
//WF	if (!other->client)
//WF		return;
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
//ERASER START
		{
			if ((ent->item->pickup == Pickup_Weapon) ||
				(ent->item->pickup == Pickup_Health) ||
				(ent->item->pickup == Pickup_Ammo) ||
				(ent->item->pickup == Pickup_Powerup) ||
				(ent->item->pickup == Pickup_Armor) ||
				(ent->item->pickup == Place_Special) ||//acrid42
				(ent->item->pickup == CTFPickup_Flag) ||
				(ent->item->pickup == CTFPickup_Tech))
			{
				RemoveFromItemList(ent);
			}
//ERASER END
			G_FreeEdict (ent);
	}}//E
}

//======================================================================

void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	Touch_Item (ent, other, plane, surf);
}
//WF34 START
static void remove_ammo(edict_t *ent)
{
	//if (wfdebug) gi.dprintf("Remove ammo. Dropped total = %d\n", TOTALDROPPEDAMMO);
	--TOTALDROPPEDAMMO;
	ent->nextthink = level.time + .1;
	ent->think = G_FreeEdict;
}
//WF34 END
void drop_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	if (deathmatch->value)
	{
//ERASER START
		// Set visible nodes
		if ((ent->item->pickup == Pickup_Weapon) ||
			(ent->item->pickup == Pickup_Health) ||
			(ent->item->pickup == Pickup_Ammo)   ||
			(ent->item->pickup == Pickup_Armor)  ||
			(ent->item->pickup == Pickup_Powerup) ||
			(ent->item->pickup == Place_Special) ||//acrid42
			(ent->item->pickup == CTFPickup_Flag) ||
			(ent->item->pickup == CTFPickup_Tech))
		{
			CalcItemPaths(ent);

			ent->nextthink = level.time + 29;
			ent->think = RemoveDroppedItem;
		}
		else
		{
//ERASER END
		ent->nextthink = level.time + 29;

		//If this is ammo, reduce world ammo count//WF34
		if ( (ent->spawnflags & DROPPED_ITEM) && (ent->item->flags == IT_AMMO))
		{
			ent->think = remove_ammo;
		}
		else
		{
			ent->think = G_FreeEdict;//NORMAL
		}
	}}//E
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

	//if (wfdebug) gi.dprintf("Drop item: %s\n", item->classname);

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
		//G_FreeEdict (ent); //G.A.R.  Will this fix disappearing flags?
		BecomeExplosion1(ent);
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

//ERASER START
	if (((int)dmflags->value & DF_INFINITE_AMMO) && (ent->item->pickup == Pickup_Ammo))
	{	// no ammo, since infinite ammo is set
		G_FreeEdict(ent);
		return;
	}

	// Set visible nodes
	if ((ent->item->pickup == Pickup_Weapon) ||
		(ent->item->pickup == Pickup_Health) ||
		(ent->item->pickup == Pickup_Ammo)   ||
		(ent->item->pickup == Pickup_Armor) ||
		(ent->item->pickup == Pickup_Powerup) ||
		(ent->item->pickup == Pickup_Pack) ||
		(ent->item->pickup == Pickup_Bandolier) ||
		(ent->item->pickup == Place_Special) ||//acrid42
		(ent->item->pickup == CTFPickup_Flag) ||
		(ent->item->pickup == CTFPickup_Tech))
	{
		CalcItemPaths(ent);
	}
//ERASER END
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
	if (deathmatch->value)
	{
//WF	// If player classes are in effect, remove weapons from maps
		if (((int)wfflags->value & WF_NO_PLAYER_CLASSES) == 0)
		{
			if (wf_IsWeapon(ent->classname))
			{
			    //gi.dprintf ("WF removed: %s\n",ent->classname);
				G_FreeEdict (ent);
				return;
			}

		}
		if ((int)dmflags->value & DF_CTF_NO_TECH)
		{
			if (item->pickup == CTFPickup_Tech)
			{
			    gi.dprintf ("WF removed: %s\n",ent->classname);
				G_FreeEdict (ent);
				return;
			}

		}
//WF
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
	}

	if (coop->value && (strcmp(ent->classname, "key_power_cube") == 0))
	{
		ent->spawnflags |= (1 << (8 + level.power_cubes));
		level.power_cubes++;
	}

	// don't let them drop items that stay in a coop game
	if ((coop->value) && (item->flags & IT_STAY_COOP))
	{
		item->drop = NULL;
	}
//ZOID
//Don't spawn the flags unless enabled
	if (!ctf->value &&
		(strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0)) {
		G_FreeEdict(ent);
		return;
	}
//ZOID
	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	if (ent->model)
		gi.modelindex (ent->model);
///////////////////special node/////////////////////////////////////////
    //for making the special nodeitems visible
	if (bot_debug_nodes->value)
	{
		if (strcmp(ent->classname, "item_sentryspot") == 0)
		{
//			item->world_model = "models/objects/debris1/tris.md2";
			item->world_model = "models/items/snode/tris.md2";
//			item->world_model = "models/items/mega_h/tris.md2";
		}
		if (strcmp(ent->classname, "item_depotspot") == 0)
		{
//			item->world_model = "models/items/healing/stimpack/tris.md2";
//			item->world_model = "models/objects/debris1/tris.md2";
			item->world_model = "models/items/snode/tris.md2";
		}
	}
///////////////////////////////special nodes end//////////////////////////

//ZOID
//flags are server animated and have special handling
	if (strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0) {
		ent->think = CTFFlagSetup;
	}
//ZOID

//WF
	//Create an entity where flags are spawned to support
	//Team Fortress style flag captures
	if (((int)wfflags->value & WF_ZOID_FLAGCAP) == 0)
	{
		/*
		if (strcmp(ent->classname, "item_flag_team1") == 0)
			Create_Home_Base(ent, CTF_TEAM1);
		if (strcmp(ent->classname, "item_flag_team2") == 0)
			Create_Home_Base(ent, CTF_TEAM2);
		*/
	}

//WF
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
		NULL,
		0,
/* precache */ "misc/power2.wav misc/power1.wav"
	},
//////////////////////////////special//////////////////////////
    //
	//NEW BOT NODES
	//
	// Special
	//

/*QUAKED item_sentryspot (.3 .3 1) (-16 -16 -16) (16 16 16)42
*/
	{
		"item_sentryspot",
		Place_Special,
		NULL,
		NULL,
		NULL,
		NULL,
//		"models/objects/debris1/tris.md2", 0,
		"models/items/snode/tris.md2", 0,
		NULL,
/* icon */		NULL,
/* pickup */	"SentrySpot",
/* width */		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		0,		//3.20
		NULL,
		0,
/* precache */ ""
	},
/*QUAKED item_depotspot (.3 .3 1) (-16 -16 -16) (16 16 16)42
*/
	{
		"item_depotspot",
		Place_Special,
		NULL,
		NULL,
		NULL,
		NULL,
		"models/items/snode/tris.md2", 0,
//		"models/items/healing/stimpack/tris1.md2", 0,
		NULL,
/* icon */		NULL,
/* pickup */	"DepotSpot",
/* width */		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		0,		//3.20
		NULL,
		0,
/* precache */ ""
	},
////////////////////////////////////special end//////////////////////////
	//
	// WEAPONS
	//

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blaster",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Blaster,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Blaster",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,	//3.20
		NULL,
		WEAPON_BLASTER,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
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
		WEAP_SHOTGUN,	//3.20
		NULL,
		WEAPON_SHOTGUN,
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
		WEAP_SUPERSHOTGUN,	//3.20
		NULL,
		WEAPON_SUPERSHOTGUN,
/* precache */ "weapons/sshotf1b.wav"
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
		WEAP_MACHINEGUN,		//3.20
		NULL,
		WEAPON_MACHINEGUN,
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
		WEAP_CHAINGUN,		//3.20
		NULL,
		WEAPON_CHAINGUN,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav"
	},

/*QUAKED weapon_megachaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_megachaingun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_MegaChaingun,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
//		"models/weapons/v_mega/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"Mega Chaingun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,		//3.20
		NULL,
		WEAPON_MEGACHAINGUN,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav"
	},

/*QUAKED weapon_pulsecannon (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_pulsecannon",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ChainMBPC,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"Pulse Cannon",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,		//3.20
		NULL,
		WEAPON_PULSE,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav"
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
/* icon */		"a_grenades",
/* pickup */	"Grenades",
/* width */		3,
		5,
		"grenades",
		IT_AMMO|IT_WEAPON,
		WEAP_GRENADES,		//3.20
		NULL,
		AMMO_GRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED ammo_grenades2 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades2",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr2/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Grenades2",
/* width */		3,
		5,
		"grenades",
		IT_AMMO|IT_WEAPON,
		WEAP_GRENADES,		//3.20
		NULL,
		AMMO_GRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED ammo_grenades3 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades3",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr3/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Grenades3",
/* width */		3,
		5,
		"grenades",
		IT_AMMO|IT_WEAPON,
		WEAP_GRENADES,		//3.20
		NULL,
		AMMO_GRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED weapon_lrprojectile (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_lrprojectile",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_ComputerGuidedProjectileLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Projectile Launcher",
		0,
		10,
		"Grenades",
		IT_WEAPON,
		WEAP_GRENADELAUNCHER,		//3.20
		NULL,
		WEAPON_LRPROJECTILE,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
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
		WEAP_HYPERBLASTER,		//3.20
		NULL,
		WEAPON_HYPERBLASTER,
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
		WEAP_RAILGUN,		//3.20
		NULL,
		WEAPON_RAILGUN,
/* precache */ "weapons/rg_hum.wav"
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
		WEAP_BFG,		//3.20
		NULL,
		WEAPON_BFG,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav"
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
		0,		//3.20
		NULL,
		AMMO_SHELLS,
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
		0,		//3.20
		NULL,
		AMMO_BULLETS,
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
		0,		//3.20
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
		0,		//3.20
		NULL,
		AMMO_ROCKETS,
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
		0,		//3.20
		NULL,
		AMMO_SLUGS,
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
		60,
		NULL,
		IT_POWERUP,
		0,		//3.20
		NULL,
		0,
/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav"
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		60,
		NULL,
		0,
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
		NULL,
		0,
/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},
	//new weapons

	/* weapon_Nag (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_nag",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Nag,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Nervous Accelerator Gun",
		0,
		1,
		"Cells",
		IT_WEAPON,
		WEAP_BLASTER,	//3.20
		NULL,
		WEAPON_NAG,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},
	/* weapon_flaregun (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_flaregun",
		NULL,
		Use_Weapon,
		NULL,
//		Weapon_FlareGun,
		Weapon_FlareGun2,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/grapple/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Flare Gun",
		0,
		1,
		"Bullets",
		IT_WEAPON,
		WEAP_GRAPPLE,	//3.20
		NULL,
		WEAPON_FLAREGUN,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},
	/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shcrifle",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_SHCRifle,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
/* icon */		"w_shotgun",
/* pickup */	"SHC Rifle",
		0,
		1,
		"Shells",
		IT_WEAPON,
		WEAP_SHOTGUN,	//3.20
		NULL,
		WEAPON_SHC,
/* precache */ "weapons/v_shotg/flash2/tris.md2 weapons/shotgf1b.wav weapons/shotgr1b.wav"
	},


/*QUAKED weapon_nailgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_nailgun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_NailGun,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/nailgun/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"NailGun",
		0,
		1,
		"Bullets",
		IT_WEAPON,
		WEAP_CHAINGUN,		//3.20
		NULL,
		WEAPON_NAILGUN,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav"
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
/* icon */		"w_glauncher",
/* pickup */	"Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON,
		WEAP_GRENADELAUNCHER,		//3.20
		NULL,
		WEAPON_GRENADELAUNCHER,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

/*QUAKED weapon_flarelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_flarelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_FlareLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Flare Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON,
		WEAP_GRENADELAUNCHER,		//3.20
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},
/* weapon_bulletgrenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bulletgrenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BulletGrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Shrapnal Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON,
		WEAP_GRENADELAUNCHER,		//3.20
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

/* weapon_timednukelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_timednukelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_TimedNukeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Timed Nuke Launcher",
		0,
		25,
		"Grenades",
		IT_WEAPON,
		WEAP_GRENADELAUNCHER,		//3.20
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

	/* weapon_napalmgrenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_napalmgrenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_NapalmGrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Napalm Grenade Launcher",
		0,
		2,
		"Grenades",
		IT_WEAPON,
		WEAP_GRENADELAUNCHER,		//3.20
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

/* weapon_infected_dart (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_infected_dart",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_InfectedDartLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
//		"models/weapons/v_launch/tris.md2",
		"models/weapons/v_dartinfect/tris.md2",
//		NULL, 0,
//		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Infected Dart Launcher",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,	//3.20
		NULL,
		WEAPON_INFECTEDDART,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},
/* weapon_poison_dart (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_poison_dart",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_ArmorDartLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
//		"models/weapons/v_launch/tris.md2",
		"models/weapons/v_dartarmor/tris.md2",
//		NULL, 0,
//		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Poison Dart Launcher",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,	//3.20
		NULL,
		WEAPON_ARMORDART,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},
	/* weapon_poison_dart (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_poison_dartb",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_PoisonDartLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_darttranq/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Tranquilizer Dart Launcher",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,	//3.20
		NULL,
		WEAPON_TRANQUILDART,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},
/* weapon_tranquilizer (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_tranquilizer",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_TranquilizerRifle,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Tranquilizer",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,	//3.20
		NULL,
		WEAPON_TRANQUILIZER,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},


/*QUAKED weapon_sniper_rifle (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_sniper_rifle",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Snipe,
		"misc/w_pkup.wav",
		"models/weapons/v_sniper2/tris.md2", EF_ROTATE,
		"models/weapons/v_sniper2/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Sniper Rifle",
		0,
		1,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		0,		//3.20
		NULL,
		WEAPON_SNIPERRIFLE,
/* precache */ //"weapons/shotgf1b.wav weapons/shotgr1b.wav"
/* precache */ "tank\tnkatck5.wav weapons/shotgr1b.wav"
	},
/*QUAKED weapon_cluster_missiles (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_cluster_missiles",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketClusterLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Cluster Rocket Launcher",
		0,
		5,
		"Rockets",
		IT_WEAPON,
		WEAP_ROCKETLAUNCHER,		//3.20
		NULL,
		WEAPON_CLUSTERROCKET,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},


	/* weapon_diseasegrenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_diseasegrenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_DiseaseGrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Disease Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON,
		0,		//3.20
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},
/*QUAKED weapon_pelletrocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_pelletrocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_PelletRocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",

/* icon */		"w_rlauncher",
/* pickup */	"Pellet Rocket Launcher",
		0,
		2,
		"Rockets",
		IT_WEAPON,
		WEAP_ROCKETLAUNCHER,		//3.20
		NULL,
		WEAPON_PELLET,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},
/*QUAKED weapon_sentrykiller_missiles (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
 {
		"weapon_sentry_missiles",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_SentryRocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */  "w_rlauncher",
/* pickup */ "Sentry Killer",
		0,
		10,
		"Rockets",
		IT_WEAPON,
		WEAP_ROCKETLAUNCHER,		//3.20
		NULL,
		WEAPON_SENTRYKILLER,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
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
//		"models/weapons/v_rocket/tris.md2",
		"models/weapons/v_homing/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Rocket Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON,
		WEAP_ROCKETLAUNCHER,		//3.20
		NULL,
		WEAPON_ROCKETLAUNCHER,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},
	/*QUAKED weapon_rocketnapalmlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketnapalmlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketNapalmLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
//		"models/weapons/v_rocket/tris.md2",
		"models/weapons/v_napalm/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Rocket Napalm Launcher",
		0,
		2,
		"Rockets",
		IT_WEAPON,
		WEAP_ROCKETLAUNCHER,		//3.20
		NULL,
		WEAPON_NAPALMMISSLE,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},


	/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_flamethrower",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_FlameThrower,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_flamethrower/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"FlameThrower",
		0,
		1,
		"Cells",
		IT_WEAPON,
		WEAP_BFG,		//3.20
		NULL,
		WEAPON_FLAMETHROWER,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav"
	},

/* weapon_timednukelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
/*QUAKED weapon_lightninggun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_lightninggun",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Lightninggun,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_lightn/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Lightning Gun",
		0,
		1,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,		//3.20
		NULL,
		WEAPON_LIGHTNING,
/* precache */ "weapons/rg_hum.wav sounds/electric.wav sounds/lshoot.wav"
	},

/*QUAKED weapon_knife (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_knife",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Knife,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_knife/tris.md2",
/* icon */		"w_shotgun",
/* pickup */	"Knife",
		0,
		0,		//uses no ammo
		NULL,		//"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_KNIFE,	//3.20
		NULL,
		WEAPON_KNIFE,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav"
	},

/*QUAKED weapon_ak47 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_ak47",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_AK47,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_ak47/tris.md2",
/* icon */		"w_machinegun",
/* pickup */	"AK47",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,		//3.20
		NULL,
		WEAPON_AK47,
/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
	},

/*QUAKED weapon_pistol (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_pistol",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Pistol,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_pistol/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Pistol",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,	//3.20
		NULL,
		WEAPON_PISTOL,
/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
	},

/* weapon_boltedblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_boltedblaster",
//		Pickup_Weapon,
		NULL,
		Use_Weapon,
		NULL,
		Weapon_BoltedBlaster,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Mag Bolted",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,	//3.20
		NULL,
		WEAPON_MAGBOLTED,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},

/* weapon_needler (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_needler",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Needler,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_blast/tris.md2",

/* icon */		"w_blaster",
/* pickup */	"Needler",
		0,
		1,
		"Bullets",
		IT_WEAPON,
		WEAP_BLASTER,	//3.20
		NULL,
		WEAPON_NEEDLER,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
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
		0,		//3.20
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
		0,		//3.20
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},
/////////////$$$///////////Eraser Support Acrid////////$$$////////////////
/*QUAKED item_flagreturn_team1 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flagreturn_team1",
		CTFPickup_Flag,
		NULL,
		NULL,
		NULL,
		NULL,
//		"models/objects/flagbase1/tris.md2", 0,
		"models/objects/flagbase/tris.md2", 0,
		NULL,
/* icon */		NULL,
/* pickup */	NULL,
/* width */		0,
		0,
		NULL,
		0,
		0,		//3.20
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},
/*QUAKED item_flagreturn_team2 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flagreturn_team2",
		CTFPickup_Flag,
		NULL,
		NULL,
		NULL,
		NULL,
//		"models/objects/flagbase/tris1.md2", 0,
		"models/objects/flagbase/tris.md2", 0,
		NULL,
/* icon */		NULL,
/* pickup */	NULL,
/* width */		0,
		0,
		NULL,
		0,
		0,		//3.20
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},
/////////////$$$///////////END Eraser Support Acrid/////////$$$///////////////

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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
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
		0,		//3.20
		NULL,
		0,
/* precache */ "ctf/tech4.wav"
	},
/* weapon_grapple (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_grapple",
		NULL,
		//Use_Weapon,
		Use_Grapple,//5/99
		NULL,
//		CTFWeapon_Grapple,
		CTFWeapon_Grapple2,//newgrap 4/99
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/grapple/tris.md2",
/* icon */		"w_grapple",
/* pickup */	"Grapple",
		0,
		0,
		NULL,
		IT_WEAPON,
		WEAP_GRAPPLE,		//3.20
		NULL,
		0,
/* precache */ "weapons/grapple/grfire.wav weapons/grapple/grpull.wav weapons/grapple/grhang.wav weapons/grapple/grreset.wav weapons/grapple/grhit.wav"
	},
/* QUAKED weapon_freezer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
		{//acrid 3/99
			"weapon_freezer",
				NULL,
				Use_Weapon,
				NULL,
				Weapon_Freezer,
				"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_freeze/tris.md2",
				"w_railgun",
				"Freezer",
				0,
				1,
				"Slugs",
				IT_WEAPON,
		        WEAP_RAILGUN,//3.20
				NULL,
				WEAPON_FREEZER,
				"weapons/blastf1a.wav mics/lasfly.wav"
		},
	/*QUAKED weapon_telsa (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_telsa",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Telsa,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
//      "models/weapons/telsa/tris.md2", EF_ROTATE,
//      "models/weapons/telsa/tris.md2",
/* icon */		"w_hyperblaster",

/* pickup */	"Telsa Coil",
		0,
		4,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,		//3.20
		NULL,
		WEAPON_TELSA,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"
	},
	// end of list marker
	// ROGUE
/*QUAKED weapon_etf_rifle (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_etf_rifle",									// classname
		Pickup_Weapon,										// pickup function
		Use_Weapon,											// use function
		Drop_Weapon,										// drop function
		Weapon_ETF_Rifle,									// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_etf_rifle/tris.md2", EF_ROTATE,		// world model, world model flags
		"models/weapons/v_etf_rifle/tris.md2",					// view model
		"w_etf_rifle",										// icon
		"ETF Rifle",										// name printed when picked up
		0,													// number of digits for statusbar
		1,													// amount used / contained
		"SLUGS",										// ammo type used
		IT_WEAPON,											// inventory flags
//		WEAP_ETFRIFLE,										// visible weapon
		WEAP_BLASTER,
		NULL,												// info (void *)
		WEAPON_ETF_RIFLE,													// tag
		"weapons/nail1.wav models/proj/flechette/tris.md2",	// precaches
	},
	// ROGUE
/*QUAKED weapon_plasmabeam (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_plasmabeam",								// classname
		Pickup_Weapon,										// pickup function
		Use_Weapon,											// use function
		Drop_Weapon,										// drop function
		Weapon_Heatbeam,									// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_beamer/tris.md2", EF_ROTATE,		// world model, world model flags
		"models/weapons/v_beamer/tris.md2",					// view model
		"w_heatbeam",											// icon
		"Plasma Beam",											// name printed when picked up
		0,													// number of digits for statusbar
		// FIXME - if this changes, change it in NoAmmoWeaponChange as well
		2,													// amount used / contained
		"Cells",											// ammo type used
		IT_WEAPON,											// inventory flags
//		WEAP_PLASMA,										// visible weapon
		WEAP_BLASTER,
		NULL,												// info (void *)
		WEAPON_PLASMA_BEAM,													// tag
		"models/weapons/v_beamer2/tris.md2 weapons/bfg__l1a.wav",		// precaches
	},
        //Rogue weapon
/*QUAKED weapon_disintegrator (.3 .3 1) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN
*/
	{
		"weapon_disintegrator",								// classname
		Pickup_Weapon,										// pickup function
		Use_Weapon,											// use function
		Drop_Weapon,										// drop function
		Weapon_Disintegrator,								// weapon think function
		"misc/w_pkup.wav",									// pick up sound
		"models/weapons/g_dist/tris.md2", EF_ROTATE,		// world model, world model flags
		"models/weapons/v_dist/tris.md2",					// view model
		"w_disintegrator",									// icon
		"Disruptor",										// name printed when picked up
		0,													// number of digits for statusbar
		1,													// amount used / contained
//		"Rounds",											// ammo type used
		"Cells",											// ammo type used
#ifdef KILL_DISRUPTOR
		IT_NOT_GIVEABLE,
#else
		IT_WEAPON,											// inventory flags
#endif
//		WEAP_DISRUPTOR,										// visible weapon
		WEAP_BLASTER,
		NULL,												// info (void *)
		1,													// tag
		"models/items/spawngro/tris.md2 models/proj/disintegrator/tris.md2 weapons/disrupt.wav weapons/disint2.wav weapons/disrupthit.wav",	// precaches
	},
        //XATRIX
/*QUAKED weapon_ioripper (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_ionripper",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Ionripper,
		"misc/w_pkup.wav",
           "models/weapons/g_boom/tris.md2", EF_ROTATE,
           "models/weapons/v_boomer/tris.md2",
/* icon */	"w_ripper",
/* pickup */ "Ionripper",
		0,
		2,
		"Cells",
		IT_WEAPON,
//		WEAP_BOOMER,
		WEAP_BLASTER,
		NULL,
		WEAPON_ION_RIPPER,
/* precache */ "weapons/rg_hum.wav weapons/rippfire.wav"
	},
// RAFAEL 14-APR-98     - XATRIX
/*QUAKED weapon_phalanx (.3 .3 1) (-16 -16 -16) (16 16 16)
*/

	{
		"weapon_phalanx",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Phalanx,
		"misc/w_pkup.wav",
		"models/weapons/g_shotx/tris.md2", EF_ROTATE,
		"models/weapons/v_shotx/tris.md2",
/* icon */	"w_phallanx",
/* pickup */ "Phalanx",
		0,
		1,
//		"Mag Slug",
		"Slugs",
		IT_WEAPON,
//		WEAP_PHALANX,
		WEAP_BLASTER,
		NULL,
		WEAPON_PHALANX,
/* precache */ "weapons/plasshot.wav"
	},

	/*QUAKED weapon_stingerlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_stingerlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_StingerLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_stinger/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Stinger Launcher",
		0,
		5,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,		//3.20
		NULL,
		WEAPON_STINGER,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},



	{NULL}
};


/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/n_health.wav");
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_small (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
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
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/l_health.wav");
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_mega (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;
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
}
int JacketArmor (void)
{
	return jacket_armor_index;
}