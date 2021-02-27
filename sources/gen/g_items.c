#include "g_local.h"

//Skid added
#include "g_genutil.h"

qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);

void Weapon_Blaster (edict_t *ent);
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

//Skid added
/*================================================================
==================================
 Lots of new Weapons
 QUAKE Weapons
================================*/
void Weapon_Q1_Axe(edict_t *ent);
void Weapon_Q1_SG(edict_t *ent);
void Weapon_Q1_SSG(edict_t *ent);
void Weapon_Q1_NG(edict_t *ent);
void Weapon_Q1_SNG(edict_t *ent);
void Weapon_Q1_GL(edict_t *ent);
void Weapon_Q1_RL(edict_t *ent);
void Weapon_Q1_LG(edict_t *ent);
void Weapon_Q1_Grapple(edict_t *ent);	
/*================================
 DOOM Weapons
================================*/
void Weapon_D_Fists(edict_t *ent);     
void Weapon_D_Saw(edict_t *ent);		// cleanup
void Weapon_D_Pistol(edict_t *ent);
void Weapon_D_SG(edict_t *ent);
void Weapon_D_SSG(edict_t *ent);
void Weapon_D_CG(edict_t *ent);
void Weapon_D_PG(edict_t *ent);
void Weapon_D_RL(edict_t *ent);
void Weapon_D_BFG(edict_t *ent);		// cleanup
/*================================
 WOLF Weapons
================================*/
void Weapon_W_Knife(edict_t *ent);		
void Weapon_W_Pistol(edict_t *ent);		
void Weapon_W_MG(edict_t *ent);			
void Weapon_W_GG(edict_t *ent);			
void Weapon_W_RL(edict_t *ent);			
void Weapon_W_FT(edict_t *ent);	
void Weapon_W_GG2(edict_t *ent);		

/*================================
 NEW ARMOR 
================================*/
gitem_armor_t greenarmor_info   = {100, 100,  .30, .20, ARMOR_Q1_GREEN};
gitem_armor_t yellowarmor_info  = {150, 150, .60, .40, ARMOR_Q1_YELLOW};
gitem_armor_t redarmor_info     = {200, 200, .80, .60, ARMOR_Q1_RED};

gitem_armor_t dgreenarmor_info = {100,100, .33, .16, ARMOR_D_GREEN};
gitem_armor_t dbluearmor_info  = {200,200, .50, .60, ARMOR_D_BLUE};

gitem_armor_t wgreenvest_info = {100,100, .40, .25, ARMOR_W_GREEN};
gitem_armor_t wbluevest_info  = {200,200, .60, .60, ARMOR_W_BLUE};
 
int	q1_green_armor_index;
int	q1_yellow_armor_index;
int	q1_red_armor_index;

int	d_green_armor_index;
int	d_blue_armor_index;

int	w_green_vest_index;
int	w_blue_vest_index;
//================================================================
//End Skid

gitem_armor_t jacketarmor_info	= { 50,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= {100, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {200, 200, .80, .60, ARMOR_BODY};

int	jacket_armor_index;
int	combat_armor_index;
int	body_armor_index;
static int	power_screen_index;
static int	power_shield_index;

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
		if (gen_ctf->value &&
			((int)dmflags->value & DF_WEAPONS_STAY) &&
			master->item && (master->item->flags & IT_WEAPON))
			ent = master;
		else 
		{
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

//Skid added	
	switch(level.game)
	{
	case CLASS_DOOM:
	case CLASS_WOLF:
		gi.sound(ent, CHAN_AUTO, gi.soundindex("ditems/resp.wav"), 1, ATTN_NORM, 0);
		break;
	case CLASS_Q1:
		gi.sound(ent, CHAN_AUTO, gi.soundindex("q1items/itembk2.wav"), 1, ATTN_NORM, 0);
		break;
	case CLASS_Q2:
	default:
		// send an effect
		ent->s.event = EV_ITEM_RESPAWN;
		break;
	}
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

	if (coop->value)
	{
	  if(other->client->resp.player_class != CLASS_Q2 && (quantity > 0))
		  return false;

	  if((ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;
	}

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		
		if (((int)dmflags->value & DF_INSTANT_ITEMS) || 
		   ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
//Skid added - instant powerups for other players
			|| (other->client->resp.player_class != CLASS_Q2))
//End Skid
		{
			if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		}
	}
//Skid added - no inventory for other dudes in single player	
	else if(gen_sp->value || coop->value)
	{
		if(other->client->resp.player_class !=CLASS_Q2)
//End Skid
			ent->item->use (other, ent->item);
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
//Skid added
	if (coop->value)
	{
	  int quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	  
	  if((ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;
	  
	  other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	}
//end Skid

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

//Skid added
	if (coop->value)
	{
	  int quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	  
	  if((ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;
	  
	  other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	}
//end Skid

	other->max_health += 2;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Bandolier (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

//Skid added
	if (coop->value)
	{
	  int quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	  
	  if((ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;
	  
	  other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	}
//end Skid


	if(other->client->resp.player_class == CLASS_DOOM)
	{
		if (other->client->pers.max_bullets < 400)
			other->client->pers.max_bullets = 400;
		if (other->client->pers.max_shells < 100)
			other->client->pers.max_shells = 100;
		if (other->client->pers.max_cells < 300)
			other->client->pers.max_cells = 600;
		if (other->client->pers.max_rockets < 100)
			other->client->pers.max_rockets = 100;
	}
	else
	{
		if (other->client->pers.max_bullets < 250)
			other->client->pers.max_bullets = 250;
		if (other->client->pers.max_shells < 150)
			other->client->pers.max_shells = 150;
		if (other->client->pers.max_cells < 250)
			other->client->pers.max_cells = 250;
		
		//Skid added
		//no slugs for other dudes	
		if(other->client->resp.player_class == CLASS_Q2)
		{
			if (other->client->pers.max_slugs < 75)
				other->client->pers.max_slugs = 75;
		}
	}

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

	if(other->client->resp.player_class == CLASS_DOOM)
	{
		item = FindItem("Cells");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_cells)
				other->client->pers.inventory[index] = other->client->pers.max_cells;
		}
		item = FindItem("Rockets");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
				other->client->pers.inventory[index] = other->client->pers.max_rockets;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}


qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	//Skid added
	if (coop->value)
	{
	  int quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	  
	  if((ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;
	  
	  other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	}
//end Skid

	
	if(other->client->resp.player_class == CLASS_DOOM)
	{
		if (other->client->pers.max_bullets < 400)
			other->client->pers.max_bullets = 400;
		if (other->client->pers.max_shells < 100)
			other->client->pers.max_shells = 100;
		if (other->client->pers.max_cells < 300)
			other->client->pers.max_cells = 600;
		if (other->client->pers.max_rockets < 100)
			other->client->pers.max_rockets = 100;
	}
	else
	{
		if (other->client->pers.max_bullets < 300)
			other->client->pers.max_bullets = 300;
		if (other->client->pers.max_shells < 200)
			other->client->pers.max_shells = 200;
		if (other->client->pers.max_rockets < 100)
			other->client->pers.max_rockets = 100;
		if (other->client->pers.max_cells < 300)
			other->client->pers.max_cells = 300;
	}
	
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

//Skid added
//No grenades or slugs for old guys
	if(other->client->resp.player_class == CLASS_Q2)
	{
		if (other->client->pers.max_slugs < 100)
			other->client->pers.max_slugs = 100;

		if (other->client->pers.max_grenades < 100)
			other->client->pers.max_grenades = 100;

		item = FindItem("Grenades");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
				other->client->pers.inventory[index] = other->client->pers.max_grenades;
		}

		item = FindItem("Slugs");
		if (item)
		{
			index = ITEM_INDEX(item);
			other->client->pers.inventory[index] += item->quantity;
			if (other->client->pers.inventory[index] > other->client->pers.max_slugs)
				other->client->pers.inventory[index] = other->client->pers.max_slugs;
		}
	}
//End Skid
	
	item = FindItem("Rockets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
			other->client->pers.inventory[index] = other->client->pers.max_rockets;
	}


	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

//======================================================================

void Use_Quad (edict_t *ent, gitem_t *item)
{
	int		timeout;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (quad_drop_timeout_hack 
//Skid added
		&& (ent->client->resp.player_class == CLASS_Q2))
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
}

//======================================================================

void Use_Invulnerability (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += 300;
	else
		ent->client->invincible_framenum = level.framenum + 300;

	if(ent->client->resp.player_class == CLASS_DOOM)		
		gi.sound(ent, CHAN_ITEM, gi.soundindex("ditems/getpow.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Silencer (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	ent->client->silencer_shots += 30;

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
//======================================================================
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
	{
		if(ent->client->resp.player_class == CLASS_Q2)
			max = ent->client->pers.max_grenades;
//Skid - use max rockets for old guys		
		else
			max = ent->client->pers.max_rockets;
	}
	else if (item->tag == AMMO_CELLS)
		max = ent->client->pers.max_cells;
	else if (item->tag == AMMO_SLUGS)
	{
		if(ent->client->resp.player_class == CLASS_Q2)
			max = ent->client->pers.max_slugs;	
//Skid - slugs will be substituted with Large Shells		
		else
			max = ent->client->pers.max_shells;
	}
	else
		return false;

	index = ITEM_INDEX(item);

//=================================================================
// Ammo substitution - Skid
// UGLY
//=================================================================
	switch(index)
	{
	case INDEX_Q1_SL:
	case INDEX_Q1_SS:
	case INDEX_D_SL:
	case INDEX_D_SS:
		{
			index = AMMO_SHELLS_INDEX;
			break;
		}
	case INDEX_Q1_NL:
	case INDEX_Q1_NS:
	case INDEX_D_BL:
	case INDEX_D_BS:
	case INDEX_W_BL:
	case INDEX_W_BS:
		{
			index = AMMO_BULLETS_INDEX;
			break;
		}
	case INDEX_Q1_CL:
	case INDEX_Q1_CS:
	case INDEX_D_CL:
	case INDEX_D_CS:
	case INDEX_W_PETROL:
		{
			index=AMMO_CELLS_INDEX;
			break;
		}
	case INDEX_D_RS:
		{
			index = AMMO_ROCKETS_INDEX;
			break;
		}
	case INDEX_Q1_RL:
	case INDEX_Q1_RS:
	case INDEX_D_RL:
	case INDEX_Q1_RG:
	case INDEX_Q2_RG:
	case INDEX_W_RG:
	case INDEX_W_ROCKS:
		{
			index = AMMO_ROCKETS_INDEX;
			break;
		}
	case AMMO_GRENADES_INDEX:
		{
			if(ent->client->resp.player_class != CLASS_Q2)
				index = AMMO_ROCKETS_INDEX;
			break;
		}
	case AMMO_SLUGS_INDEX:
		{
			if(ent->client->resp.player_class != CLASS_Q2)
				index = AMMO_SHELLS_INDEX;
			break;
		}
	}
//Q2 guys get GRENADES AND ROCKETS if they pickup Rocks in older maps			 
	if((ent->client->resp.player_class == CLASS_Q2)
	   && (level.game != CLASS_Q2))
	{
		if(index == AMMO_ROCKETS_INDEX)
		{
			if((ent->client->pers.inventory[AMMO_GRENADES_INDEX] + 2) <=
				 ent->client->pers.max_grenades)
	 			ent->client->pers.inventory[AMMO_GRENADES_INDEX] += 2;
			else
				ent->client->pers.inventory[AMMO_GRENADES_INDEX] = ent->client->pers.max_grenades;
		}
	}
//=================================================================
// - end Skid
	
	if (ent->client->pers.inventory[index] == max)
		return false;

	ent->client->pers.inventory[index] += count;

	if (ent->client->pers.inventory[index] > max)
		ent->client->pers.inventory[index] = max;

	return true;
}




qboolean Add_SafeAmmo (edict_t *ent, gitem_t *item, int count)
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
	{
		if(ent->client->resp.player_class == CLASS_Q2)
			max = ent->client->pers.max_grenades;
//Skid - use max rockets for old guys		
		else
			max = ent->client->pers.max_rockets;
	}
	else if (item->tag == AMMO_CELLS)
		max = ent->client->pers.max_cells;
	else if (item->tag == AMMO_SLUGS)
	{
		if(ent->client->resp.player_class == CLASS_Q2)
			max = ent->client->pers.max_slugs;	
//Skid - slugs will be substituted with Large Shells		
		else
			max = ent->client->pers.max_shells;
	}
	else
		return false;

	index = ITEM_INDEX(item);

//=================================================================
// Ammo substitution - Skid
// UGLY
//=================================================================
	switch(index)
	{
	case INDEX_Q1_SL:
	case INDEX_Q1_SS:
	case INDEX_D_SL:
	case INDEX_D_SS:
		{
			index = AMMO_SHELLS_INDEX;
			break;
		}
	case INDEX_Q1_NL:
	case INDEX_Q1_NS:
	case INDEX_D_BL:
	case INDEX_D_BS:
	case INDEX_W_BL:
	case INDEX_W_BS:
		{
			index = AMMO_BULLETS_INDEX;
			break;
		}
	case INDEX_Q1_CL:
	case INDEX_Q1_CS:
	case INDEX_D_CL:
	case INDEX_D_CS:
	case INDEX_W_PETROL:
		{
			index=AMMO_CELLS_INDEX;
			break;
		}
	case INDEX_D_RS:
		{
			index = AMMO_ROCKETS_INDEX;
			break;
		}
	case INDEX_Q1_RL:
	case INDEX_Q1_RS:
	case INDEX_D_RL:
	case INDEX_Q1_RG:
	case INDEX_Q2_RG:
	case INDEX_W_RG:
	case INDEX_W_ROCKS:
		{
			index = AMMO_ROCKETS_INDEX;
			break;
		}
	case AMMO_GRENADES_INDEX:
		{
			if(ent->client->resp.player_class != CLASS_Q2)
				index = AMMO_ROCKETS_INDEX;
			break;
		}
	case AMMO_SLUGS_INDEX:
		{
			if(ent->client->resp.player_class != CLASS_Q2)
				index = AMMO_SHELLS_INDEX;
			break;
		}
	}
//=================================================================
// - end Skid
	
	if (ent->client->pers.inventory[index] == max)
		return false;

	ent->client->pers.inventory[index] += count;

//	if(index == AMMO_SLUGS_INDEX)
//		gi.dprintf("Added %d Railgun slugs\n",count);

	if (ent->client->pers.inventory[index] > max)
		ent->client->pers.inventory[index] = max;

	return true;
}



//======================================================================


qboolean Pickup_Ammo (edict_t *ent, edict_t *other)
{
	int			count;
	qboolean	weapon;

	weapon = (ent->item->flags & IT_WEAPON);
	if ( (weapon) && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		count = 1000;
	else if (ent->count)
		count = ent->count;
	else
		count = ent->item->quantity;
	
	
	if(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))
	{
		if(!Add_SafeAmmo (other,ent->item,count))
			return false;
	}
	else
	{
		if (!Add_Ammo (other, ent->item, count))
			return false;
	}

	if (weapon && !count)
	{
// FIXME - Weapon should be changed if Better for old guys for BOTH DM and SP
		if (other->client->pers.weapon != ent->item && 
			( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
			other->client->newweapon = ent->item;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 30);
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

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (self, 20);
	else
		G_FreeEdict (self);
}


qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
//SKID ADDED	
	int player_class = other->client->resp.player_class;
	
	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->health >= other->max_health)
			return false;

//Skid
	if(player_class == CLASS_DOOM && other->health >=200)
		return false;
//end Skid

//ZOID
	if (other->health >= 250 && ent->count > 25)
		return false;
//ZOID

//Skid added - quakeguy cannot increase health to 101 by small packs
	if((player_class == CLASS_Q1) && 
	   (ent->count < 10)  && 
	   (other->health >= other->max_health))
	   return false;
//end Skid

	other->health += ent->count;

//Skid	
	if(player_class == CLASS_DOOM && other->health > 200)
		other->health = 200;
	else
//end Skid

//ZOID
	if (other->health > 250 && ent->count > 25)
		other->health = 250;
//ZOID
	
	if(ent->count == 1)
	{
//Skid added
		switch(player_class)
		{
		case CLASS_Q1:
			ent->item->pickup_sound = "ditems/itemup.wav";
			ent->item->icon = "q1hud/items/q1_h10";
			break;
		case CLASS_WOLF:
			ent->item->pickup_sound = "wolfguy/slurp.wav";
			ent->item->icon = "wolfhud/items/wDogfood";
			break;
		case CLASS_DOOM:
			ent->item->pickup_sound = "ditems/itemup.wav";
			ent->item->icon = "doomhud/items/dNmed";
			break;
		case CLASS_Q2:
		default:
			ent->item->pickup_sound = "items/s_health.wav";
			ent->item->icon = "i_health";
			break;
		}
	}
	else if (ent->count == 2)
	{
		switch(player_class)
		{
			case CLASS_Q1:
				ent->item->pickup_sound = "items/s_health.wav";
				ent->item->icon = "q1hud/items/q1_h10";
				break;
			case CLASS_WOLF:
				ent->item->pickup_sound = "witems/item.wav";
				ent->item->icon = "wolfhud/items/wDogfood";
				break;
			case CLASS_DOOM:
				ent->item->pickup_sound = "ditems/itemup.wav";
				ent->item->icon = "doomhud/items/dNmed";
				break;
			case CLASS_Q2:
			default:
				ent->item->pickup_sound = "items/s_health.wav";
				ent->item->icon = "i_health";
				break;
		}
//end Skid
	}
	else if (ent->count == 10 || ent->count == 15)
	{
//Skid added
//DIFF HEALTH SOUNDS
		switch(player_class)
		{
			case CLASS_Q1:
				ent->item->pickup_sound = "q1items/healthS.wav";
				ent->item->icon = "q1hud/items/q1_h10";
				break;
			case CLASS_WOLF:
				ent->item->pickup_sound = "witems/item.wav";
				ent->item->icon = "wolfhud/items/wDinner";
				break;
			case CLASS_DOOM:
				ent->item->pickup_sound = "ditems/itemup.wav";
				ent->item->icon = "doomhud/items/dNmed";
				break;
			case CLASS_Q2:
			default:
				ent->item->pickup_sound = "items/n_health.wav";
				ent->item->icon = "i_health";
				break;
		}
	}
	else if (ent->count == 25)
	{
		switch(player_class)
		{
			case CLASS_Q1:
				ent->item->pickup_sound = "q1items/healthL.wav";
				ent->item->icon = "q1hud/items/q1_h25";
				break;
			case CLASS_WOLF:
				ent->item->pickup_sound = "witems/item.wav";
				ent->item->icon = "wolfhud/items/wMed";
				break;
			case CLASS_DOOM:
				ent->item->pickup_sound = "ditems/itemup.wav";
				ent->item->icon = "doomhud/items/dLmed";
				break;
			case CLASS_Q2:
			default:
				ent->item->pickup_sound = "items/l_health.wav";
				ent->item->icon = "i_health";
				break;
		}
	}
	else // (ent->count == 100)
	{
		switch(player_class)
		{
			case CLASS_Q1:
				ent->item->pickup_sound = "q1items/healthM.wav";
				ent->item->icon = "q1hud/items/q1_h100";
				break;
			case CLASS_WOLF:
				ent->item->pickup_sound = "wolfguy/yeah.wav";		
				break;
			case CLASS_DOOM:
				ent->item->pickup_sound = "ditems/getpow.wav";
				ent->item->icon = "doomhud/items/dSoul";
				break;
			case CLASS_Q2:
			default:
				ent->item->pickup_sound = "items/m_health.wav";
				ent->item->icon = "i_health";
				break;
		}
//End Skid
	}

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
			other->health = other->max_health;
	}

	if (((ent->style & HEALTH_TIMED) 
//Skid
		|| (ent->count == 100))
//Skid
//ZOID		
		&& !CTFHasRegeneration(other)
//ZOID
//Skid	
		&& (player_class!= CLASS_DOOM))
//Skid
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
	if (!ent->client)
		return 0;

	if (ent->client->pers.inventory[jacket_armor_index] > 0)
		return jacket_armor_index;

	if (ent->client->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

	if (ent->client->pers.inventory[body_armor_index] > 0)
		return body_armor_index;

// ADD Q1 ARMOR - Skid
	if (ent->client->pers.inventory[q1_green_armor_index] > 0)
		return q1_green_armor_index;

	if (ent->client->pers.inventory[q1_yellow_armor_index] > 0)
		return q1_yellow_armor_index;
	
	if (ent->client->pers.inventory[q1_red_armor_index] > 0)
		return q1_red_armor_index;

// ADD DOOM ARMOR - Skid
	if (ent->client->pers.inventory[d_green_armor_index] > 0)
		return d_green_armor_index;
	
	if (ent->client->pers.inventory[d_blue_armor_index] > 0)
		return d_blue_armor_index;

	if (ent->client->pers.inventory[w_green_vest_index] > 0)
		return w_green_vest_index;
	
	if (ent->client->pers.inventory[w_blue_vest_index] > 0)
		return w_blue_vest_index;
	
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
	
	int           new_armor_index;

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;
	new_armor_index = ITEM_INDEX (ent->item);
	old_armor_index = ArmorIndex (other);

//Skid Added
/*======================================================================
	Armor Pickups, Quake Doom and Wolfie
======================================================================*/

	switch(other->client->resp.player_class)
	{
	case CLASS_WOLF:
	{
		if(old_armor_index == w_green_vest_index)
			oldinfo = &wgreenvest_info;
		else if(old_armor_index == w_blue_vest_index)
			oldinfo = &wbluevest_info;

		// handle armor shards specially
		if (ent->item->tag == ARMOR_SHARD)
		{
			if (!old_armor_index)
				other->client->pers.inventory[w_green_vest_index] = 1;
			else if(other->client->pers.inventory[old_armor_index] < 200)
				other->client->pers.inventory[old_armor_index] += 1;
			else
				return false;
		}
		else 
		{
			if (!old_armor_index)
				other->client->pers.inventory[new_armor_index] = newinfo->base_count;
			else
			{
				if(other->client->pers.inventory[old_armor_index] >= newinfo->base_count)
				//if((other->client->pers.inventory[old_armor_index] * oldinfo->normal_protection) >=
				   //(newinfo->base_count * newinfo->normal_protection))
					return false;
				else
				{
					other->client->pers.inventory[old_armor_index] = 0;
					other->client->pers.inventory[new_armor_index] = newinfo->base_count;
				}
			}
		}
		break;
	}
	case CLASS_DOOM:
	{
		if(old_armor_index == d_green_armor_index)
			oldinfo = &dgreenarmor_info;
		else if(old_armor_index == d_blue_armor_index)
			oldinfo = &dbluearmor_info;

		// handle armor shards specially
		if (ent->item->tag == ARMOR_SHARD)
		{
			if (!old_armor_index)
				other->client->pers.inventory[d_green_armor_index] = 1;
			else if(other->client->pers.inventory[old_armor_index] < 200)
				other->client->pers.inventory[old_armor_index] += 1;
			else
				return false;
		}
		else 
		{
			if (!old_armor_index)
				other->client->pers.inventory[new_armor_index] = newinfo->base_count;
			else
			{
				if(other->client->pers.inventory[old_armor_index] >= newinfo->base_count)
				//if((other->client->pers.inventory[old_armor_index] * oldinfo->normal_protection) >=
				   //(newinfo->base_count * newinfo->normal_protection))
					return false;
				else
				{
					other->client->pers.inventory[old_armor_index] = 0;
					other->client->pers.inventory[new_armor_index] = newinfo->base_count;
				}
			}
		}
		break;
	}
	case CLASS_Q1:
	{
			if(old_armor_index == q1_green_armor_index)
				oldinfo = &greenarmor_info;
			else if(old_armor_index == q1_yellow_armor_index)
				oldinfo = &yellowarmor_info;
			else if(old_armor_index == q1_red_armor_index)
				oldinfo = &redarmor_info;

		// handle armor shards specially
		if (ent->item->tag == ARMOR_SHARD)
		{
			if (!old_armor_index)
				other->client->pers.inventory[q1_green_armor_index] = 1;
			else if(other->client->pers.inventory[old_armor_index] < oldinfo->max_count)
				other->client->pers.inventory[old_armor_index] += 1;
			else
				return false;
		}
		else 
		{
			if (!old_armor_index)
				other->client->pers.inventory[new_armor_index] = newinfo->base_count;
			else
			{
				if((other->client->pers.inventory[old_armor_index] * oldinfo->normal_protection) >=
				   (newinfo->base_count * newinfo->normal_protection))
					return false;
				else
				{
					other->client->pers.inventory[old_armor_index] = 0;
					other->client->pers.inventory[new_armor_index] = newinfo->base_count;
				}
			}
		}
		break;
	}
	case CLASS_Q2:
	default:
	{
//======================================================================
//End Skid
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
			if(ent->item->tag == ARMOR_JACKET)
			{
				other->client->pers.inventory[jacket_armor_index] = 50;
			}
			else if (ent->item->tag == ARMOR_COMBAT)
			{
				other->client->pers.inventory[combat_armor_index] = 100;
			}
			else
				other->client->pers.inventory[body_armor_index] = 200;
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
				other->client->pers.inventory[new_armor_index] = newcount; 
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
		break;
//Skid added	
	}
	}
//end

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
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
			gi.cprintf (ent, PRINT_HIGH, "No cells for power armor.\n");
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



/*
===============
Touch_Item
===============
*/

void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean	taken;
	
//Skid added 
//Temp substitution Items
	gitem_t *newitem=NULL;
	gitem_t *olditem=NULL;
//

	if (!other->client)
		return;
	if (other->health < 1)
		return;		// dead people can't pickup
	if (!ent->item->pickup)
		return;		// not a grabbable item?

// Skid added
// switch item if it doesnt belong to your class
	switch(other->client->resp.player_class)
	{
		case CLASS_Q1:
		{
			if(!(ent->item->flags & IT_Q1))
				newitem= GiveNewItem(ent->item,CLASS_Q1);
			break;
		}
		case CLASS_DOOM:
		{
			if(!(ent->item->flags & IT_DOOM))
				newitem= GiveNewItem(ent->item,CLASS_DOOM);
			break;
		}
		case CLASS_WOLF:
		{
			if(!(ent->item->flags & IT_WOLF))
				newitem= GiveNewItem(ent->item,CLASS_WOLF);
			break;
		}
		case CLASS_Q2:
		default:
		{
			if((ent->item->flags & IT_Q1) || 
			   (ent->item->flags & IT_DOOM) ||
			   (ent->item->flags & IT_WOLF))
				newitem= GiveNewItem(ent->item,CLASS_Q2);
			break;
		}
	}
	
	if(newitem != NULL)
	{
		olditem = ent->item;
		ent->item = newitem;
	}

// end substitution
	taken = ent->item->pickup(ent, other);

	if((olditem) && (taken))
	{
		int i = ITEM_INDEX(olditem);
		switch(other->client->resp.player_class)
		{
		case CLASS_Q2:
			{
				if(olditem->flags & IT_WEAPON)
				{
					if(olditem->flags & IT_Q1)
					{
						switch(i)
						{
						case Q1_RL:
							{
								GiveWeapon(other,Q2_RG,true);
								break;
							}
						case Q1_LG: 
							{
								GiveWeapon(other,Q2_BFG,false);
								break;
							}
						}
					}
					else if(olditem->flags & IT_DOOM)
					{
						if(i==D_RL)
						{
							GiveWeapon(other,Q2_RG,true);
						}
					}
				}
				break;
			}
		case CLASS_DOOM:
			{
				if(olditem->flags & IT_WEAPON)
				{
					if(olditem->flags & IT_Q1)
					{
						if(i==Q1_LG)
						{
							GiveWeapon(other,D_BFG,false);
						}
					}
				}
				break;
			}
		}
	}

	if(taken)
	{
		
// flash the screen
	other->client->bonus_alpha = 0.25;
		
		// show icon and name on status bar
		if(other->client->resp.player_class == CLASS_WOLF &&
		   (ent->item->flags &IT_POWERUP))
		    other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex("wolfhud/items/wPowerup");
		else
			other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
		other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);

// Higher in Doom - Skid
		if(other->client->resp.player_class == CLASS_DOOM)
			other->client->pickup_msg_time = level.time + 4.0;
		else
			other->client->pickup_msg_time = level.time + 3.0;
// end Skid

		// change selected item
		if (ent->item->use)
				other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);
		
//Skid added		
		if((other->client->resp.player_class == CLASS_WOLF) &&
		   (ent->item->flags & IT_POWERUP))
		{
			float r =random();
			if(r < 0.3)
				gi.sound(other, CHAN_ITEM, gi.soundindex("wolfguy/yeah.wav"), 1, ATTN_NORM, 0);
			else if(r < 0.6)
				gi.sound(other, CHAN_ITEM, gi.soundindex("wolfguy/allright.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(other, CHAN_ITEM, gi.soundindex("wolfguy/yeah1.wav"), 1, ATTN_NORM, 0);
		}
		else
//end
		gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
	}

	if (!(ent->spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets (ent, other);
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

//Relink Old Item to Entitiy
	if(newitem)
		ent->item = olditem;

	if (!taken)
		return;

	if (!((coop->value) &&  
		 (ent->item->flags & IT_STAY_COOP)) || 
		 (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
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
	if (deathmatch->value)
	{
		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
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
//======================================================================
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

	gi.linkentity (ent);
}



static qboolean ItemWeaponCheck(char *classname)
{
	if (strcmp(classname, "weapon_shotgun") == 0) 
		return true;
	if (strcmp(classname, "weapon_supershotgun") == 0) 
		return true;
	if (strcmp(classname, "weapon_machinegun") == 0) 
		return true;
	if (strcmp(classname, "weapon_chaingun") == 0) 
		return true;
	if (strcmp(classname, "weapon_grenadelauncher") == 0) 
		return true;
	if (strcmp(classname, "weapon_rocketlauncher") == 0) 
		return true;
	if (strcmp(classname, "weapon_hyperblaster") == 0) 
		return true;
	if (strcmp(classname, "weapon_railgun") == 0) 
		return true;
////////////////
// Skid - added	
////////////////
	if (strcmp(classname, "ammo_grenades") == 0) 
		return true;							
	if (strcmp(classname, "weapon_q1_ssg") == 0)
		return true;
	if (strcmp(classname, "weapon_q1_ng") == 0)
		return true;
	if (strcmp(classname, "weapon_q1_sng") == 0)
		return true;
	if (strcmp(classname, "weapon_q1_gl") == 0)
		return true;
	if (strcmp(classname, "weapon_q1_rl") == 0)
		return true;
	if (strcmp(classname, "weapon_q1_lg") == 0)
		return true;
	if (strcmp(classname, "weapon_d_sg") == 0)
		return true;
	if (strcmp(classname, "weapon_d_ssg") == 0)
		return true;
	if (strcmp(classname, "weapon_d_cg") == 0)
		return true;
	if (strcmp(classname, "weapon_d_rl") == 0)
		return true;
	if (strcmp(classname, "weapon_d_pg") == 0)
		return true;
	if (strcmp(classname, "weapon_d_bfg") == 0)
		return true;
	if (strcmp(classname, "weapon_w_mg") == 0)
		return true;
	if (strcmp(classname, "weapon_w_gg") == 0)
		return true;
	if (strcmp(classname, "weapon_w_rl") == 0)
		return true;
	if (strcmp(classname, "weapon_w_ft") == 0)
		return true;
	if (strcmp(classname, "weapon_w_gg2") == 0)
		return true;
////////////////
// end Skid
////////////////
	return false;
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
//===================================
// Skid Ent Substitution - Precaches
//===================================
	gitem_t *newitem;

//Substitute Item in map
if(deathmatch->value)  
{	//check

	if(((int)genflags->value & GEN_Q1_ONLY) && 
		(level.game != CLASS_Q1) &&
		 !(item->flags & IT_Q1))
	{
		int i = ITEM_INDEX(item);
		
		//no shards or grenades
		if(((item->flags & IT_ARMOR) && (item->tag == ARMOR_SHARD)) ||
		   (i==Q2_HG) ||
		   (i==Q2_SG))
		{
			G_FreeEdict (ent);
			return;
		}

//gi.dprintf("old item : %s\n",item->classname);
		newitem = GiveNewItem(item,CLASS_Q1);
		if(newitem)
		{
			ent->classname = newitem->classname;
			ent->s.effects = newitem->world_model_flags;
			item = newitem;
//gi.dprintf("new item : %s\n",newitem->classname);
		}	
		//final Precache will be Q1 item
	}

	else if(((int)genflags->value & GEN_DOOM_ONLY) && 
			(level.game != CLASS_DOOM) &&
			!(item->flags & IT_DOOM))
	{	
		newitem = GiveNewItem(item,CLASS_DOOM);
		if(newitem)
		{
			ent->classname = newitem->classname;
			ent->s.effects = newitem->world_model_flags;
			item = newitem;
		}	
			//final Precache will be Doom item
	}
		//TODO : add Wolfie mode ?
	else
	{
//Regular Precaches		
		if(item->flags & IT_Q1)
		{
			newitem = GiveNewItem(item,CLASS_Q2);
			if(newitem)
			{
				PrecacheItem(newitem);
			}
			newitem = GiveNewItem(item,CLASS_DOOM);
			if(newitem)
			{
				PrecacheItem(newitem);
			}

			newitem = GiveNewItem(item,CLASS_WOLF);
			if(newitem)
			{
				PrecacheItem(newitem);
			}
		}
		else if(item->flags & IT_DOOM)
		{
			newitem = GiveNewItem(item,CLASS_Q2);
			if(newitem)
			{
				PrecacheItem(newitem);
			}
			newitem = GiveNewItem(item,CLASS_Q1);
			if(newitem)
			{
				PrecacheItem(newitem);
			}
			newitem = GiveNewItem(item,CLASS_WOLF);
			if(newitem)
			{
				PrecacheItem(newitem);
			}
		}
		else if(item->flags & IT_WOLF)
		{
			newitem = GiveNewItem(item,CLASS_Q2);
			if(newitem)
			{
				PrecacheItem(newitem);
			}
			newitem = GiveNewItem(item,CLASS_Q1);
			if(newitem)
			{
				PrecacheItem(newitem);
			}
			newitem = GiveNewItem(item,CLASS_WOLF);
			if(newitem)
			{
				PrecacheItem(newitem);
			}
		}
		else
		{
			newitem = GiveNewItem(item,CLASS_Q1);
			if(newitem)
			{
				PrecacheItem(newitem);
			}
			newitem = GiveNewItem(item,CLASS_DOOM);
			if(newitem)
			{
				PrecacheItem(newitem);
			}
			newitem = GiveNewItem(item,CLASS_WOLF);
			if(newitem)
			{
				PrecacheItem(newitem);
			}
		}
	}
}
//===================================
// End Skid
//===================================
	PrecacheItem (item);
		
	if (ent->spawnflags)
	{
		if (strcmp(ent->classname, "key_power_cube") != 0)
		{
			ent->spawnflags = 0;
			gi.dprintf("%s at %s has invalid spawnflags set\n", 
				ent->classname, vtos(ent->s.origin));
		}
	}

//===================================
// New Changes for Pickup mode - Skid
//===================================
	// some items will be prevented in deathmatch
	if (deathmatch->value)
	{
// dont spawn items if its a pickup(fully loaded teamplay) game - Skid
		
		if (ItemWeaponCheck(ent->classname) && (int)genflags->value & GEN_FULLYLOADED )
		{
		    G_FreeEdict (ent);
			return;
		}

		if ( (int)dmflags->value & DF_NO_ARMOR || (int)genflags->value & GEN_FULLYLOADED  )
		{
			if (item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_ITEMS || (int)genflags->value & GEN_FULLYLOADED  )
		{
			if (item->pickup == Pickup_Powerup)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_HEALTH || (int)genflags->value & GEN_FULLYLOADED )
		{
			if (item->pickup == Pickup_Health || item->pickup == Pickup_Adrenaline || item->pickup == Pickup_AncientHead)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_INFINITE_AMMO || (int)genflags->value & GEN_FULLYLOADED )
		{
			if ( (item->flags & IT_AMMO) || (strcmp(ent->classname, "weapon_bfg") == 0) )
			{
				G_FreeEdict (ent);
				return;
			}
		}
	}

////////////////////////////////
// end -Skid
////////////////////////////////
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

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	
	if (ent->model)
		gi.modelindex (ent->model);

////////////////////////////////////////////////
// start a CTF game if map has a flags -Skid
////////////////////////////////////////////////
	if ((Q_stricmp(ent->classname, "item_flag_team1") == 0) ||
	    (Q_stricmp(ent->classname, "item_flag_team2") == 0))
	{
		if(!gen_ctf->value)
		{
			gi.cvar_forceset("gen_ctf","1");
			gi.dprintf("==== Init GenCTF ====\n");
			
			if(gen_team->value)
				gi.cvar_forceset("gen_team", "0");
		}
		ent->think = CTFFlagSetup;
	}
////////////////////////////////////////////////
// end - Skid
////////////////////////////////////////////////
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

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16)*/		
//INDEX = 1
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
		0,
		&bodyarmor_info,
		ARMOR_BODY,
/* precache */ ""
	},

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16) */	
//INDEX = 2
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
		0,
		&combatarmor_info,
		ARMOR_COMBAT,
/* precache */ ""
	},

/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16) */
//INDEX = 3
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
		0,
		&jacketarmor_info,
		ARMOR_JACKET,
/* precache */ ""
	},

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16) */
//INDEX = 4
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
		0,
		NULL,
		ARMOR_SHARD,
/* precache */ ""
	},


/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16) */
//INDEX = 5
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
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16) */
//INDEX = 6
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
		0,
		NULL,
		0,
/* precache */ "misc/power2.wav misc/power1.wav"
	},


// ==============
// WEAPONS 
// ==============

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world */         

//INDEX = 7
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
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)*/

//INDEX = 8
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
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav"
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)*/

//INDEX = 9
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
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "weapons/sshotf1b.wav"
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 10
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
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 11
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
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav"
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 12
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
		WEAP_GRENADES,
		NULL,
		AMMO_GRENADES,
/* precache */ "models/objects/q2exp/tris.md2 weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav"
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 13
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
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 models/objects/q2exp/tris.md2 weapons/q2rockex.wav weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 14
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
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/q2rockex.wav models/objects/debris2/tris.md2 models/objects/q2exp/tris.md2"
	},  

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 15
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
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 16
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
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav"
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 17
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
		WEAP_BFG,
		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav"
	},

//=======================================================	
/////////////////////////////////////////////////////////
//  QUAKE WEAPONS	/////////////////////////////////////
/////////////////////////////////////////////////////////
//=======================================================	

///////////////////////////////////
///// AXE /////////////////////////
///////////////////////////////////  

//INDEX = 18
	{
		"weapon_q1_axe", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Q1_Axe,
		"misc/w_pkup.wav",
		NULL,0,
		"models/weapons/v_q1axe/tris.md2",
/* icon */		"q1hud/items/w_q1ax",
/* pickup */	"Axe",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "q1weap/axe/ax1.wav q1weap/axe/axhit1.wav q1weap/axe/axhit2.wav"
	},

///////////////////////////////////
///// Boomstick ///////////////////
///////////////////////////////////

//INDEX = 19 
	{
		"weapon_q1_sg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_SG,
		"q1items/pickweap.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_q1sg/tris.md2",
/* icon */		"q1hud/items/w_q1sg",
/* pickup */	"Boomstick",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "weapons/shotg2.wav"
	},

///////////////////////////////////
///// Double Barrelled ShotGun ////
/////////////////////////////////// 

//INDEX = 20
	{
		"weapon_q1_ssg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_SSG,
		"q1items/pickweap.wav",
		"models/weapons/g_q1ssg/tris.md2", EF_ROTATE,
		"models/weapons/v_q1ssg/tris.md2",
/* icon */		"q1hud/items/w_q1ssg",
/* pickup */	"Double Barrelled Shotgun",
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "q1weap/sshotgun/shotgn2.wav"
	},

///////////////////////////////////
///// NailGun /////////////////////
/////////////////////////////////// 

//INDEX = 21
	{
		"weapon_q1_ng", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_NG,
		"q1items/pickweap.wav",
		"models/weapons/g_q1ng/tris.md2", EF_ROTATE,
		"models/weapons/v_q1ng/tris.md2",
/* icon */		"q1hud/items/w_q1ng",
/* pickup */	"Nailgun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_MACHINEGUN,
		NULL,
		0,			//q1weap/nails/rocket1i.wav
/* precache */ "models/objects/q1nail/tris.md2 weapons/nail1.wav q1weap/nails/r_end.wav q1weap/nails/ric1.wav q1weap/nails/ric2.wav q1weap/nails/ric3.wav q1weap/nails/tink1.wav"
	},


///////////////////////////////////
///// Super NailGun ///////////////
///////////////////////////////////

//INDEX = 22
	{
		"weapon_q1_sng", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_SNG,
		"q1items/pickweap.wav",
		"models/weapons/g_q1sng/tris.md2", EF_ROTATE,
		"models/weapons/v_q1sng/tris.md2",
/* icon */		"q1hud/items/w_q1sng",
/* pickup */	"Super Nailgun",
		0,
		2,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_CHAINGUN,
		NULL,
		0,		
/* precache */ "models/objects/q1nail/tris.md2 weapons/plasshot.wav q1weap/nails/s_end.wav q1weap/nails/ric1.wav q1weap/nails/ric2.wav q1weap/nails/ric3.wav q1weap/nails/tink1.wav"
	},


///////////////////////////////////
///// q1 grenadelauncher //////////
/////////////////////////////////// 

//INDEX = 23
	{
		"weapon_q1_gl",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_GL,
		"q1items/pickweap.wav",
		"models/weapons/g_q1gl/tris.md2", EF_ROTATE,
		"models/weapons/v_q1gl/tris.md2",
/* icon */		"q1hud/items/w_q1gl",
/* pickup */	"GrenadeLauncher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache*/ "sprites/s_explod.sp2 models/objects/q1gren/tris.md2 q1weap/grenades/bounce.wav q1weap/grenades/grenade.wav"
	},


///////////////////////////////////
///// q1 rocket ///////////////////
/////////////////////////////////// 
	
//INDEX = 24		
	{
		"weapon_q1_rl",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_RL,
		"q1items/pickweap.wav",
		"models/weapons/g_q1rl/tris.md2", EF_ROTATE,
		"models/weapons/v_q1rl/tris.md2",
/* icon */		"q1hud/items/w_q1rl",
/* pickup */	"RocketLauncher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,		
/* precache*/ "sprites/s_explod.sp2 models/objects/rocket/tris.md2 weapons/rippfire.wav weapons/rockfly.wav q1weap/rocket/r_exp3.wav"
	},


////////////////////////////////////
///// q1 LIGHTNING GUN /////////////
/////////////////////////////////// 
	
//INDEX = 25			
	{
		"weapon_q1_lg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_LG,
		"q1items/pickweap.wav",
		"models/weapons/g_q1lg/tris.md2", EF_ROTATE,
		"models/weapons/v_q1lg/tris.md2",
/* icon */		"q1hud/items/w_q1lg",
/* pickup */	"Thunderbolt",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "models/monsters/parasite/segment/tris.md2 q1weap/lightning/lhit.wav q1weap/lightning/lstart.wav "
	},


//=======================================================	
/////////////////////////////////////////////////////////
//   DOOM WEAPONS	/////////////////////////////////////
/////////////////////////////////////////////////////////
//=======================================================	

///////////////////////////////////
///// Doom Fists //////////////////
/////////////////////////////////// 

//INDEX = 26
{
		"weapon_d_fists", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_D_Fists,
		"misc/w_pkup.wav",
		NULL,0,
		"models/weapons/v_dfists/tris.md2",			
/* icon */		"doomhud/items/w_dfist",
/* pickup */	"Fists",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "dweap/punch.wav"
	},		
		
///////////////////////////////////
///// Doom Chainsaw ///////////////
///////////////////////////////////

//INDEX = 27
	{
		"weapon_d_saw", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_Saw,
		"ditems/weapup.wav",
		"models/weapons/g_dsaw/tris.md2", EF_ROTATE,
		"models/weapons/v_dsaw/tris.md2",
/* icon */		"doomhud/items/w_dcsaw",
/* pickup */	"Chainsaw",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "dweap/sawidle.wav dweap/sawhit.wav dweap/sawup.wav dweap/sawon.wav"
	},

///////////////////////////////////
///// Doom Pistol /////////////////
///////////////////////////////////

//INDEX = 28
	{
		"weapon_d_pistol",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_Pistol,
		"ditems/weapup.wav",
		NULL,0,
		"models/weapons/v_dpistol/tris.md2",
/* icon */		"doomhud/items/w_dpistol",
/* pickup */	"Pistol",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "dweap/pistol.wav"
	},

///////////////////////////////////
///// Doom ShotGun ////////////////
///////////////////////////////////

//INDEX = 29
	{
		"weapon_d_sg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_SG,
		"ditems/weapup.wav",
		"models/weapons/g_dsg/tris.md2", EF_ROTATE,
		"models/weapons/v_dsg/tris.md2", 
/* icon */		"doomhud/items/w_dsg",
/* pickup */	"Doom Shotgun",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "dweap/sg.wav"
	},

///////////////////////////////////
///// Doom Super ShotGun //////////
///////////////////////////////////

//INDEX = 30
	{
		"weapon_d_ssg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_SSG,
		"ditems/weapup.wav",
		"models/weapons/g_dssg/tris.md2", EF_ROTATE,
		"models/weapons/v_dssg/tris.md2",
/* icon */		"doomhud/items/w_dssg",
/* pickup */	"Doom Super Shotgun",
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "dweap/ssg.wav"
	},

///////////////////////////////////
///// Doom ChainGun ///////////////
///////////////////////////////////

//INDEX = 31
	{
		"weapon_d_cg", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_CG,
		"ditems/weapup.wav",
		"models/weapons/g_dcg/tris.md2", EF_ROTATE,
		"models/weapons/v_dcg/tris.md2",
/* icon */		"doomhud/items/w_dcg",
/* pickup */	"Doom Chaingun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "dweap/pistol.wav"
	},


////////////////////////////////////
///// Doom RL //////////////////////
////////////////////////////////////
	
//INDEX = 32		
	{
		"weapon_d_rl",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_RL,
		"ditems/weapup.wav",
		"models/weapons/g_drl/tris.md2", EF_ROTATE,
		"models/weapons/v_drl/tris.md2",
/* icon */		"doomhud/items/w_drl",
/* pickup */	"Doom Rocket Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache*/ "sprites/Drexp.sp2 models/objects/drocket/tris.md2 weapons/rockfly.wav dweap/rocket.wav dweap/rockexp.wav"
	},

///////////////////////////////////
///// Doom PlasmaGun //////////////
///////////////////////////////////

//INDEX = 33	
	{
		"weapon_d_pg", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_PG,
		"ditems/weapup.wav",
		"models/weapons/g_dpg/tris.md2", EF_ROTATE,
		"models/weapons/v_dpg/tris.md2",
/* icon */		"doomhud/items/w_dpg",
/* pickup */	"Plasma Rifle",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "sprites/plasma.sp2 dweap/plasma.wav dweap/phit.wav"
	},

////////////////////////////////////
///// Doom BFG /////////////////////
////////////////////////////////////
	
//INDEX = 34		
	{
		"weapon_d_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_BFG,
		"ditems/weapup.wav",
		"models/weapons/g_dbfg/tris.md2", EF_ROTATE,
		"models/weapons/v_dbfg/tris.md2",
/* icon */		"doomhud/items/w_dbfg",
/* pickup */	"BFG9000",
		0,
		40,
		"Cells",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 dweap/bfg.wav weapons/bfg__f1y.wav weapons/bfg__l1a.wav dweap/bfgexp.wav"
	},



//=================================================================
//=================================================================
//=================================================================
// WOLF 3d WEAPONS
//=================================================================
//=================================================================

//////////////////////////////
///// Knife	 /////////////////
//////////////////////////////
		
//INDEX = 35	
	{
		"weapon_w_knife", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_W_Knife,
		"witems/weapon.wav",
		NULL,0,
		"models/weapons/v_wknife/tris.md2",
/* icon */		"wolfhud/items/w_Wknife",
/* pickup */	"Knife",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "q1weap/axe/ax1.wav q1weap/axe/axhit1.wav q1weap/nails/tink1.wav"
	},
		
///////////////////////////////////
///// Wolf Pistol /////////////////
///////////////////////////////////

//INDEX = 36				
	{
		"weapon_w_pistol",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_W_Pistol,
		"witems/weapon.wav",
		NULL,0,								
		"models/weapons/v_wpistol/tris.md2",	
/* icon */		"wolfhud/items/w_Wpistol",
/* pickup */	"W Pistol",					
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "wweap/pshot.wav"
	},


///////////////////////////////////
///// Wolf Machine Gun ////////////
///////////////////////////////////

//INDEX = 37		
	{
		"weapon_w_mg", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_W_MG,
		"witems/weapon.wav",
		"models/weapons/g_wmg/tris.md2", EF_ROTATE,	  
		"models/weapons/v_wmg/tris.md2",                 
/* icon */		"wolfhud/items/w_Wmgun",
/* pickup */	"W Machinegun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "wweap/mshot.wav"
	},


///////////////////////////////////
///// Wolf Gatling Gun ////////////
///////////////////////////////////

//INDEX = 38		
	{
		"weapon_w_gg", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_W_GG,
		"witems/weapon.wav",
		"models/weapons/g_wcg/tris.md2", EF_ROTATE,		
		"models/weapons/v_wcg/tris.md2",				
/* icon */		"wolfhud/items/w_Wcgun",
/* pickup */	"Gatling Gun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "wweap/cshot.wav"
	},

///////////////////////////////////
///// Wolf Bazooka ////////////////
///////////////////////////////////
	
//INDEX = 39
	{
		"weapon_w_rl",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_W_RL,   
		"witems/weapon.wav",
		"models/weapons/g_wrl/tris.md2", EF_ROTATE,		
		"models/weapons/v_wrl/tris.md2",				
/* icon */		"wolfhud/items/w_Wrl",
/* pickup */	"Bazooka",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,		
/* precache*/ "sprites/wrexp.sp2 models/objects/wrocket/tris.md2 weapons/rockfly.wav dweap/rocket.wav wweap/explode.wav"
	},


///////////////////////////////////
///// Wolf FlameThrower ///////////
///////////////////////////////////

//INDEX = 40
	{
		"weapon_w_ft", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_W_FT,
		"witems/weapon.wav",
		"models/weapons/g_wft/tris.md2", EF_ROTATE,		
		"models/weapons/v_wft/tris.md2",				
/* icon */		"wolfhud/items/w_Wflame",
/* pickup */	"FlameThrower",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_HYPERBLASTER,
		NULL,
		0,		
/* precache */ "sprites/wflame.sp2 wweap/ffire.wav wweap/fhit.wav"
	},


//////////////////////////////////
///// GRAPPLE ////////////////////
//////////////////////////////////
/* weapon_grapple (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/

//INDEX = 41
	{
		"weapon_grapple", 
		NULL,
		Use_Weapon,
		NULL,
		CTFWeapon_Grapple,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/grapple/tris.md2",
/* icon */		"w_grapple",
/* pickup */	"Grapple",
		0,
		0,
		NULL,
		IT_WEAPON,
		WEAP_GRAPPLE,
		NULL,
		0,
/* precache */ "weapons/grapple/grfire.wav weapons/grapple/grpull.wav weapons/grapple/grhang.wav weapons/grapple/grreset.wav weapons/grapple/grhit.wav"
	},
//=======================================================	
//=======================================================	
//=======================================================	
// end - Skid

//
// AMMO ITEMS
//

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)*/

//INDEX = 42
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
		0,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},


/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)*/

//INDEX = 43
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
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 44
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
		0,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 45
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
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 46
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
		0,
		NULL,
		AMMO_SLUGS,
/* precache */ ""
	},


//=================================================================
//=================================================================
//=================================================================
//    New Ammo Models - Skid
//=================================================================
//=================================================================
//=================================================================


// Quake Shells Large						

//INDEX = 47	
	{
		"ammo_q1_shells_L",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"q1items/pickammo.wav",
		"models/items/q1ammobx/shellsL/tris.md2", 0,
		NULL,
/* icon */		"q1hud/items/q1_shell", 
/* pickup */	"Shells",
/* width */		3,
		40,
		NULL,
		IT_AMMO|IT_Q1,
		0,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},

//=================================================================
									
// Quake Shells Small							

//INDEX = 48
	{
		"ammo_q1_shells_S",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"q1items/pickammo.wav",
		"models/items/q1ammobx/shellsS/tris.md2", 0,
		NULL,
/* icon */		"q1hud/items/q1_shell", 
/* pickup */	"Shells",
/* width */		3,
		20,
		NULL,
		IT_AMMO|IT_Q1,
		0,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},

//=================================================================
//=================================================================

// Quake Nails Large							 

//INDEX = 49	
	{
		"ammo_q1_nails_L",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"q1items/pickammo.wav",
		"models/items/q1ammobx/nailsL/tris.md2", 0,
		NULL,
/* icon */		"q1hud/items/q1_nails", 
/* pickup */	"Nails",
/* width */		3,
		50,
		NULL,
		IT_AMMO|IT_Q1,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

//=================================================================
	
// Quake Nails Small							
	
//INDEX = 50
	{
		"ammo_q1_nails_S",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"q1items/pickammo.wav",
		"models/items/q1ammobx/nailsS/tris.md2", 0,
		NULL,
/* icon */		"q1hud/items/q1_nails", 
/* pickup */	"Nails",
/* width */		3,
		25,
		NULL,
		IT_AMMO|IT_Q1,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

//=================================================================
//=================================================================

// Quake Cells Large							 

//INDEX = 51	
	{
		"ammo_q1_cells_L",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"q1items/pickammo.wav",
		"models/items/q1ammobx/cellsL/tris.md2", 0,
		NULL,
/* icon */		"q1hud/items/q1_cells", 
/* pickup */	"Cells",
/* width */		3,
		12,
		NULL,
		IT_AMMO|IT_Q1,
		0,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},
													
//=================================================================

// Quake Cells Small							
		
//INDEX = 52	
	{
		"ammo_q1_cells_S",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"q1items/pickammo.wav",
		"models/items/q1ammobx/cellsS/tris.md2", 0,
		NULL,
/* icon */		"q1hud/items/q1_cells", 
/* pickup */	"Cells",
/* width */		3,
		6,
		NULL,
		IT_AMMO|IT_Q1,
		0,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},

//=================================================================
//=================================================================

// Quake Rockets Large							

//INDEX = 53
	{
		"ammo_q1_rockets_L",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"q1items/pickammo.wav",
		"models/items/q1ammobx/rocksL/tris.md2", 0,
		NULL,
/* icon */		"q1hud/items/q1_rocks", 
/* pickup */	"Rockets",
/* width */		3,
		10,
		NULL,
		IT_AMMO|IT_Q1,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

//=================================================================
	
// Quake Rockets Small							
		
//INDEX = 54	
	{
		"ammo_q1_rockets_S",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"q1items/pickammo.wav",
		"models/items/q1ammobx/rocksS/tris.md2", 0,
		NULL,
/* icon */		"q1hud/items/q1_rocks", 
/* pickup */	"Rockets",
/* width */		3,
		5,
		NULL,
		IT_AMMO|IT_Q1,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

//=================================================================
//=================================================================
// DOOM
//=================================================================
//=================================================================
	
// Doom Shells LARGE							

//INDEX = 55
	{
		"ammo_d_shells_L",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"ditems/ammoL.wav",
		"models/items/dammobx/shellsL/tris.md2", 0, 
		NULL,
/* icon */		"doomhud/items/dLshell",
/* pickup */	"Shells",
/* width */		3,
		20,
		NULL,
		IT_AMMO|IT_DOOM,
		0,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},

//=================================================================

// Doom Shells Small								
	
//INDEX = 56	
	{												
		"ammo_d_shells_S",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"ditems/ammoL.wav",
		"models/items/dammobx/shellsS/tris.md2", 0, 
		NULL,
/* icon */		"doomhud/items/dSshell",
/* pickup */	"Shells",
/* width */		3,
		4,
		NULL,
		IT_AMMO|IT_DOOM,
		0,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},

//=================================================================
//=================================================================

//Doom Bullets Large								 

//INDEX = 57
	{
		"ammo_d_bullets_L",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"ditems/ammoL.wav",
		"models/items/dammobx/bulletL/tris.md2", 0,
		NULL,
/* icon */		"doomhud/items/dLbull", 
/* pickup */	"Bullets",
/* width */		3,
		50,
		NULL,
		IT_AMMO|IT_DOOM,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

//=================================================================

//Doom Bullets Small								

//INDEX = 58
	{
		"ammo_d_bullets_S",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"ditems/ammoL.wav",
		"models/items/dammobx/bulletS/tris.md2", 0,
		NULL,
/* icon */		"doomhud/items/dSbull", 
/* pickup */	"Bullets",
/* width */		3,
		10,
		NULL,
		IT_AMMO|IT_DOOM,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

//=================================================================
//=================================================================

// Doom Cells Large									 

//INDEX = 59
	{
		"ammo_d_cells_L",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"ditems/ammoL.wav",
		"models/items/dammobx/cellsL/tris.md2", 0, 
		NULL,
/* icon */		"doomhud/items/dLcell", 
/* pickup */	"Cells",
/* width */		3,
		100,
		NULL,
		IT_AMMO|IT_DOOM,
		0,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},

//=================================================================

// Doom Cells Small									 

//INDEX = 60
	{
		"ammo_d_cells_S",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"ditems/ammoL.wav",
		"models/items/dammobx/cellsS/tris.md2", 0, 
		NULL,
/* icon */		"doomhud/items/dScell", 
/* pickup */	"Cells",
/* width */		3,
		20,
		NULL,
		IT_AMMO|IT_DOOM,
		0,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},

//=================================================================
//=================================================================

// Doom Rockets Large										 

//INDEX = 61
	{
		"ammo_d_rockets_L",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"ditems/ammoL.wav",
		"models/items/dammobx/rocksL/tris.md2", 0, 
		NULL,
/* icon */		"doomhud/items/dLrocks", 
/* pickup */	"Rockets",
/* width */		3,
		5,
		NULL,
		IT_AMMO|IT_DOOM,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

//=================================================================

// Doom Rockets Small										

//INDEX = 62
	{
		"ammo_d_rockets_S",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"ditems/ammoL.wav",
		"models/items/dammobx/rocksS/tris.md2", 0, 
		NULL,
/* icon */		"doomhud/items/dSrocks", 
/* pickup */	"Rockets",
/* width */		3,
		1,
		NULL,
		IT_AMMO|IT_DOOM,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},


//=================================================================
//=================================================================
//=================================================================
//
// QUAKE1 ARMOR
//

//QUAKE1 RED ARMOR									

//INDEX = 63
	{
		"item_q1_red_armor", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"q1items/armor.wav",
		"models/items/q1armor/armor_R/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"q1hud/items/SbRarmor", 
/* pickup */	"Red Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR|IT_Q1,
		0,
		&redarmor_info,
		ARMOR_Q1_RED,
/* precache */ ""
	},

//=================================================================

//QUAKE1 YELLOW ARMOR								  

//INDEX = 64
	{
		"item_q1_yellow_armor", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"q1items/armor.wav",
		"models/items/q1armor/armor_Y/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"q1hud/items/SbYarmor", 
/* pickup */	"Yellow Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR|IT_Q1,
		0,
		&yellowarmor_info,
		ARMOR_Q1_YELLOW,
/* precache */ ""
	},

//=================================================================

//QUAKE1 GREEN ARMOR									 

//INDEX = 65
	{
		"item_q1_green_armor", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"q1items/armor.wav",
		"models/items/q1armor/armor_G/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"q1hud/items/SbGarmor", 
/* pickup */	"Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR|IT_Q1,
		0,
		&greenarmor_info,
		ARMOR_Q1_GREEN,
/* precache */ ""
	},

//=================================================================
//=================================================================
//=================================================================
// DOOM BLUE ARMOR									  

//INDEX = 66
	{
		"item_d_blue_armor", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"ditems/getpow.wav",
		"models/items/darmor/blue/tris.md2", 0, 
		NULL,
/* icon */		"doomhud/items/d_armorb", 
/* pickup */	"Blue Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR|IT_DOOM,
		0,
		&dbluearmor_info,
		ARMOR_D_BLUE,
/* precache */ ""
	},

//=================================================================

// DOOM GREEN ARMOR										

//INDEX = 67
	{
		"item_d_green_armor", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"ditems/getpow.wav",
		"models/items/darmor/green/tris.md2", 0, 
		NULL,
/* icon */		"doomhud/items/d_armorg", 
/* pickup */	"Green Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR|IT_DOOM,
		0,
		&dgreenarmor_info,
		ARMOR_D_GREEN,
/* precache */ ""
	},

//=================================================================

// DOOM HELMET ARMOR									 

//INDEX = 68
	{
		"item_d_helmet", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"ditems/itemup.wav",
		"sprites/dA1.sp2", EF_ANIM_ALLFAST,
		NULL,
/* icon */		"doomhud/items/d_armorh", 
/* pickup */	"Armor Shards",
/* width */		3,
		0,
		NULL,
		IT_ARMOR|IT_DOOM,
		0,
		NULL,
		ARMOR_SHARD,
/* precache */ ""
	},


//=================================================================
//=================================================================
//=================================================================
//
// POWERUP ITEMS
//
//=================================================================
//=================================================================
//QUAKE

// Quake1 Quad

//INDEX = 69
	{
		"item_q1_quad",
		Pickup_Powerup,
		Use_Quad,
		Drop_General,
		NULL,
		"items/damage.wav",
		"models/items/q1pwrups/quad/tris.md2", (EF_ROTATE|EF_FLAG2), 
		NULL,
/* icon */		"q1hud/items/SbQuad", 
/* pickup */	"Quad",
/* width */		2,
		60,
		NULL,
		IT_POWERUP|IT_Q1,
		0,
		NULL,
		0,
/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav"
	},

//=================================================================

// Quake1 Ring of Invisibility								

//INDEX = 70
	{
		"item_q1_invisibility",
		Pickup_Powerup,
		Use_Invis,
		Drop_General,
		NULL,
		"q1items/inv1.wav",
		"models/items/q1pwrups/ring/tris.md2", EF_ROTATE, 
		NULL,
/* icon */		"q1hud/items/SbRing", 
/* pickup */	"Ring of Shadows",
/* width */		2,
		300,
		NULL,
		IT_POWERUP|IT_Q1,
		0,
		NULL,
		0,
/* precache */ "models/items/q1pwrups/ring/q1eyes/tris.md2 q1items/inv1.wav q1items/inv2.wav q1items/inv3.wav"
	},

//=================================================================

// Quake1 PENT												

//INDEX = 71
	{
		"item_q1_invulnerability",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"items/protect.wav",
		"models/items/q1pwrups/pent/tris.md2", (EF_ROTATE|EF_FLAG1),
		NULL,
/* icon */		"q1hud/items/Sb666", 
/* pickup */	"Pentagram of Protection",
/* width */		2,
		300,
		NULL,
		IT_POWERUP|IT_Q1,
		0,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
	},

//=================================================================

// Quake Envirosuit										

//INDEX = 72
	{
		"item_q1_biosuit",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"q1items/suit1.wav",
		"models/items/q1pwrups/suit/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"q1hud/items/SbSuit",						
/* pickup */	"Biosuit",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP|IT_Q1,
		0,
		NULL,
		0,
/* precache */ "q1items/suit1.wav q1items/suit2.wav"
	},

//=================================================================

// Quake1 Player Backpack							

//INDEX = 73
	{
		"item_q1_backpack",
		Pickup_q1_backpack,
		NULL,
		Drop_General,
		NULL,
		"q1items/pickweap.wav",
		"models/items/q1pwrups/backpack/tris.md2", EF_ROTATE, 
		NULL,
/* icon */ 		"doomhud/items/d_pack",
/* pickup */	"Backpack",
/* width */		2,
		0,
		NULL,
		IT_Q1, //IT_POWERUP|
		0,
		NULL,
		0,
/* precache */ ""
	},

//=================================================================
//=================================================================
//DOOM

// Doom BERSERK												

//INDEX = 74
	{
		"item_d_berserk",
		Pickup_Powerup,
		Use_Berserk,
		Drop_General,
		NULL,
		"ditems/getpow.wav",
		"models/items/dhealth/ader/tris.md2", 0,  
		NULL,
/* icon */		"doomhud/items/d_ader",
/* pickup */	"Berserk",
/* width */		2,
		60,
		NULL,
		IT_POWERUP|IT_DOOM,
		0,
		NULL,
		0,
/* precache */ ""
	},

//=================================================================

//Doom invisibility											

//INDEX = 75
	{
		"item_d_invisibility",
		Pickup_Powerup,
		Use_Invis,
		Drop_General,
		NULL,
		"ditems/getpow.wav",
		"sprites/dInvis.sp2", (EF_ANIM_ALLFAST),//|EF_SPHERETRANS), 
		NULL,
/* icon */		"doomhud/items/dInvis",
/* pickup */	"Invisibility",
/* width */		2,
		300,
		NULL,
		IT_POWERUP|IT_DOOM,
		0,
		NULL,
		0,
/* precache */ "models/objects/dinvis/tris.md2"
	},

//=================================================================

// Doom GOD MODE											

//INDEX = 76
	{
		"item_d_invurnability",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"ditems/getpow.wav",
		"sprites/dGod.sp2", (EF_ANIM_ALLFAST),//|EF_SPHERETRANS), 
		NULL,
/* icon */		"doomhud/items/dGod",
/* pickup */	"God",
/* width */		2,
		300,
		NULL,
		IT_POWERUP|IT_DOOM,
		0,
		NULL,
		0,
/* precache */ ""
	},

//=================================================================

// Doom Soul Sphere												

//INDEX = 77
	{
		"item_d_health_mega",//"item_d_soulsphere",
		Pickup_Powerup,
		Use_MegaSphere,
		Drop_General,
		NULL,
		"ditems/getpow.wav",
		"sprites/dMega.sp2", (EF_ANIM_ALLFAST),//|EF_SPHERETRANS),
		NULL,
/* icon */		"doomhud/items/dMega",
/* pickup */	"Mega Sphere",
/* width */		2,
		60,
		NULL,
		IT_POWERUP|IT_DOOM,
		0,
		NULL,
		0,
/* precache */ ""
	},


//=================================================================

// Doom radiation Suit											

//INDEX = 78
	{
		"item_d_radsuit",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"ditems/getpow.wav",
		"sprites/dSuit.sp2", 0,
		NULL,
/* icon */		"doomhud/items/dSuit",
/* pickup */	"Radiation Suit",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP|IT_DOOM,
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

//=================================================================

// Doom backpack												 

//INDEX = 79
	{
		"item_d_backpack",
		Pickup_Bandolier,		// FIXME
		NULL,
		Drop_General,
		NULL,
		"ditems/getpow.wav",
		"models/items/q1pwrups/backpack/tris.md2", 0, 
		NULL,
/* icon */		"doomhud/items/d_pack",
/* pickup */	"Backpack full of ammo",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP|IT_DOOM,
		0,
		NULL,
		0,
/* precache */ ""
	},

//======================================================================================
//======================================================================================
//======================================================================================
//======================================================================================
//
// POWERUP ITEMS
//

/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 80
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
		0,
		NULL,
		0,
/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav"
	},

//======================================================================================

/*QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 81
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
		0,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
	},

//======================================================================================

/*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 82
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
		0,
		NULL,
		0,
/* precache */ ""
	},

//======================================================================================

/*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 83
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
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

//======================================================================================

/*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16)	*/

//INDEX = 84
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
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

//======================================================================================

/*QUAKED item_ancient_head (.3 .3 1) (-16 -16 -16) (16 16 16) 
Special item that gives +2 to maximum health */

//INDEX = 85
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
		0,
		NULL,
		0,
/* precache */ ""
	},

//======================================================================================

/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16) 
gives +1 to maximum health
*/

//INDEX = 86
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
		IT_STAY_COOP|IT_POWERUP,//0,
		0,
		NULL,
		0,
/* precache */ ""
	},

//======================================================================================

/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)*/

//INDEX = 87
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
		IT_STAY_COOP|IT_POWERUP,//0,
		0,
		NULL,
		0,
/* precache */ ""
	},

//======================================================================================

/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)*/

//INDEX = 88
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
		IT_STAY_COOP|IT_POWERUP,//0,
		0,
		NULL,
		0,
/* precache */ ""
	},

//======================================================================================
//======================================================================================
// KEYS
//======================================================================================
//======================================================================================
// QUAKE 1

//key_q1_gold

//INDEX = 89
	{
		"key_q1_gold",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"q1items/medkey.wav",
		"models/items/q1keys/gold/rune/tris.md2", 0,
		NULL,
		"q1hud/items/SbKeyG",						
		"Gold Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_Q1,
		0,
		NULL,
		0,
/* precache */ ""
	},

//======================================================================================

//key_q1_silver

//INDEX = 90
	{
		"key_q1_silver",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"q1items/medkey.wav",
		"models/items/q1keys/silver/rune/tris.md2", 0,
		NULL,
		"q1hud/items/SbKeyS",								
		"Silver Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_Q1,
		0,
		NULL,
		0,
/* precache */ ""
	},

//======================================================================================
//======================================================================================
//runekey_q1_gold

//INDEX = 91
	{
		"runekey_q1_gold",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"q1items/runekey.wav",
		"models/items/q1keys/gold/tris.md2", 0,//EF_ROTATE
		NULL,
		"q1hud/items/SbKeyG",						
		"Gold Rune Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_Q1,
		0,
		NULL,
		0,
/* precache */ ""
	},

//======================================================================================

//runekey_q1_silver

//INDEX = 92
	{
		"runekey_q1_silver",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"q1items/runekey.wav",
		"models/items/q1keys/silver/tris.md2", 0,
		NULL,
		"q1hud/items/SbKeyS",						
		"Silver Rune Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_Q1,
		0,
		NULL,
		0,
/* precache */ ""
	},

//======================================================================================
//======================================================================================
// RUNES 
//======================================================================================
//======================================================================================

//INDEX = 93
	{
		"item_q1_rune1",
		Pickup_Rune,
		NULL,
		Drop_General,
		NULL,
		"q1items/runekey.wav",
		"models/items/q1runes/earth/tris.md2", 0,
		NULL,
		"q1hud/items/SbRune1",						
		"Rune of Earth Magic",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_TECH|IT_Q1,
		0,
		NULL,
		0,
/* precache */ "ctf/tech1.wav"
		},

//======================================================================================

//INDEX = 94
	{
		"item_q1_rune2",
		Pickup_Rune,
		NULL,
		Drop_General,
		NULL,
		"q1items/runekey.wav",
		"models/items/q1runes/black/tris.md2", 0,
		NULL,
		"q1hud/items/SbRune2",						
		"Rune of Black Magic",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_TECH|IT_Q1,
		0,
		NULL,
		0,
/* precache */ "ctf/tech2.wav ctf/tech2x.wav"
		},

//======================================================================================

//INDEX = 95
	{
		"item_q1_rune3",
		Pickup_Rune,
		NULL,
		Drop_General,
		NULL,
		"q1items/runekey.wav",
		"models/items/q1runes/hell/tris.md2", 0,
		NULL,
		"q1hud/items/SbRune3",						
		"Rune of Hell Magic",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_TECH|IT_Q1,
		0,
		NULL,
		0,
/* precache */ "ctf/tech3.wav"
		},

//======================================================================================

//INDEX = 96
	{
		"item_q1_rune4",
		Pickup_Rune,
		NULL,
		Drop_General,
		NULL,
		"q1items/runekey.wav",
		"models/items/q1runes/elder/tris.md2", 0,
		NULL,
		"q1hud/items/SbRune4",						
		"Rune of Elder Magic",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_TECH|IT_Q1,
		0,
		NULL,
		0,
/* precache */ "ctf/tech4.wav"
		},

//======================================================================================
//======================================================================================

// weapon_q1_grapple (.3 .3 1) (-16 -16 -16) (16 16 16)
//always owned by Quakeguys, never in the world

//INDEX = 97
	{
		"weapon_q1_grapple", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Q1_Grapple,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_q1hook/tris.md2",
/* icon */		"q1hud/items/w_q1gh",
/* pickup */	"Morning Star",
		0,
		0,
		NULL,
		IT_WEAPON|IT_Q1,
		WEAP_GRAPPLE,
		NULL,
		0,
/* precache */ "models/weapons/v_q1hook/star/tris.md2 q1weap/hook/chain1.wav q1weap/hook/chain2.wav q1weap/hook/chain3.wav q1weap/hook/bounce2.wav"
	},	

//======================================================================================
//======================================================================================


/*QUAKED key_data_cd (0 .5 .8) (-16 -16 -16) (16 16 16)
key for computer centers
*/
//INDEX = 98	
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
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_power_cube (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN NO_TOUCH
warehouse circuits
*/
//INDEX = 99
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
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the entrance of jail3
*/
//INDEX = 100
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
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the city computer
*/
//INDEX = 101
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
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16)
security pass for the security level
*/
//INDEX = 102
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
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - blue
*/
//INDEX = 103
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
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - red
*/
//INDEX = 104
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
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
//INDEX = 105
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
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
//INDEX = 106
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
		0,
		NULL,
		0,
/* precache */ ""
	},

//INDEX = 107
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
		0,
		NULL,
		0,
/* precache */ ""
	},

//////////////////////////////////////////
//    CTF - FLAGS
//////////////////////////////////////////

/*QUAKED item_flag_team1 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
//INDEX = 108
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
		0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},

/*QUAKED item_flag_team2 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
//INDEX = 109
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
		0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},

// TECHS

/* Resistance Tech */
//INDEX = 110
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
/* pickup */	"Disruptor Shield", // Earth Magic
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech1.wav"
	},

/* Strength Tech */
//INDEX = 111
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
/* pickup */	"Power Amplifier", // Black Magic
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech2.wav ctf/tech2x.wav"
	},

/* Haste Tech */
//INDEX = 112
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
/* pickup */	"Time Accel",	// Hell magic
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech3.wav"
	},

/* Regeneration Tech */
//INDEX = 113
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
/* pickup */	"AutoDoc", // Elder Magic
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech4.wav"
	},
//ZOID

//=================================================================
	
//HACK NUMBER 1
//Grenade to Rocket Substitute
		
//INDEX = 114	
	{
		"ammo_q1_rockets_G",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"q1items/pickammo.wav",
		"models/items/q1ammobx/rocksS/tris.md2", 0,
		NULL,
/* icon */		"q1hud/items/q1_rocks", 
/* pickup */	"Rockets",
/* width */		3,
		1,
		NULL,
		IT_AMMO|IT_Q1,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

//HACK NO 2							 

//INDEX = 115
	{
		"item_q1_armor_shards", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"q1items/pickammo.wav",
		"models/items/q1armor/armor_G/tris.md2", 0,
		NULL,
/* icon */		"q1hud/items/SbGarmor", 
/* pickup */	"Crushed Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR|IT_Q1,
		0,
		NULL,
		ARMOR_SHARD,
/* precache */ ""
	},

//=================================================================
//=================================================================

//Wolfie AMMO ITEMS
//AMMO

// Wolfie Bullets Large
//INDEX = 116
	{
		"ammo_w_bullets_L",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"witems/weapon.wav",
		"models/items/q1ammobx/shellsL/tris.md2", 0,
		NULL,
/* icon */		"wolfhud/items/wAmmoL", 
/* pickup */	"Large Ammobox",
/* width */		3,
		50,
		NULL,
		IT_AMMO|IT_WOLF,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

//=================================================================
									
//Wolfie Bullets Small
//INDEX = 117
	{
		"ammo_w_bullets_S",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"witems/weapon.wav",
		"models/items/q1ammobx/shellsS/tris.md2", 0,
		NULL,
/* icon */		"wolfhud/items/wAmmo", 
/* pickup */	"Small Clip",
/* width */		3,
		25,
		NULL,
		IT_AMMO|IT_WOLF,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

//=================================================================
//=================================================================

//Wolfie Rockets
//INDEX = 118
	{
		"ammo_w_rocks",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"witems/weapon.wav",
		"models/items/q1ammobx/nailsL/tris.md2", 0,
		NULL,
/* icon */		"wolfhud/items/wRocks", 
/* pickup */	"Bazooka Rockets",
/* width */		3,
		5,
		NULL,
		IT_AMMO|IT_WOLF,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

//=================================================================
	
//Wolfie Petrol
//INDEX = 119
	{
		"ammo_w_petrol",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"witems/weapon.wav",
		"models/items/q1ammobx/nailsS/tris.md2", 0,
		NULL,
/* icon */		"wolfhud/items/wPetrol", 
/* pickup */	"Petrol",
/* width */		3,
		10,
		NULL,
		IT_AMMO|IT_WOLF,
		0,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},

//=================================================================

///////////////////////////////////
///// Wolf Gatling Gun ////////////
///////////////////////////////////

//INDEX = 120
	{
		"weapon_w_gg2", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_W_GG2,
		"witems/weapon.wav",
		"models/weapons/g_wcg/tris.md2", EF_ROTATE,		
		"models/weapons/v_wcg2/tris.md2",				
/* icon */		"wolfhud/items/w_Wcgun",
/* pickup */	"Gatling Guns",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "wweap/cshot.wav"
	},

//=================================================================
//=================================================================
//=================================================================
// WOLF

//INDEX = 121
	{
		"item_w_blue_vest", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"witems/item.wav",
		"models/items/warmor/blue/tris.md2", 0, 
		NULL,
/* icon */		"wolfhud/items/w_armorb", 
/* pickup */	"Blue Vest",
/* width */		3,
		0,
		NULL,
		IT_ARMOR|IT_WOLF,
		0,
		&wbluevest_info,
		ARMOR_W_BLUE,
/* precache */ ""
	},

//=================================================================

// WOLF

//INDEX = 122
	{
		"item_w_green_vest", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"witems/item.wav",
		"models/items/warmor/green/tris.md2", 0, 
		NULL,
/* icon */		"wolfhud/items/w_armorg", 
/* pickup */	"Green Vest",
/* width */		3,
		0,
		NULL,
		IT_ARMOR|IT_WOLF,
		0,
		&wgreenvest_info,
		ARMOR_W_GREEN,
/* precache */ ""
	},

//INDEX = 123
	{
		"ammo_rocketsG",
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
		1,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

//Wolfie Rockets
//INDEX = 124
	{
		"ammo_w_rocksG",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"witems/weapon.wav",
		"models/items/q1ammobx/nailsL/tris.md2", 0,
		NULL,
/* icon */		"wolfhud/items/wRocks", 
/* pickup */	"Bazooka Rockets",
/* width */		3,
		1,
		NULL,
		IT_AMMO|IT_WOLF,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},


//	 end of list marker
	{NULL}
};





//======================================
// QUAKE 1 and Doom Health items
//======================================

void SP_item_d_health (edict_t *self);
void SP_item_d_health_small (edict_t *self);
void SP_item_d_health_large (edict_t *self);
void SP_item_d_health_mega (edict_t *self);


void SP_item_q1_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)genflags->value & GEN_DOOM_ONLY)
	{
		SP_item_d_health (self);
		return;
	}

	self->model = "models/items/q1health/h10/tris.md2"; 
	self->count = 15;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("q1items/healthS.wav");
	gi.imageindex ("q1hud/items/q1_h10");
}


void SP_item_q1_health_large (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)genflags->value & GEN_DOOM_ONLY)
	{
		SP_item_d_health_large (self);
		return;
	}

	self->model = "models/items/q1health/h25/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("q1items/healthL.wav");
	gi.imageindex ("q1hud/items/q1_h25");
}


void SP_item_q1_health_mega (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)genflags->value & GEN_DOOM_ONLY)
	{
		SP_item_d_health_mega (self);
		return;
	}

	self->model = "models/items/q1health/h100/tris.md2";
	self->count = 100;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("q1items/healthM.wav");
	gi.imageindex ("q1hud/items/q1_h100");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED; 
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void SP_item_d_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)genflags->value & GEN_Q1_ONLY)
	{
		SP_item_q1_health (self);
		return;
	}

	self->model = "models/items/dhealth/small/tris.md2"; 
	self->count = 10;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("ditems/itemup.wav");
}


void SP_item_d_health_small (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)genflags->value & GEN_Q1_ONLY)
	{
		G_FreeEdict (self);
		return;
	}

//	self->model = "sprites/dSmed.sp2"; 
	self->model = "models/items/dhealth/potion/tris.md2"; 
	self->count = 1;
	SpawnItem (self, FindItem ("Health"));
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex ("ditems/itemup.wav");
	self->s.effects |= EF_ROTATE;
	self->s.renderfx |= RF_FULLBRIGHT;
}


void SP_item_d_health_large (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)genflags->value & GEN_Q1_ONLY)
	{
		SP_item_q1_health_large (self);
		return;
	}

	self->model = "models/items/dhealth/large/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("ditems/itemup.wav");
}

// MegaSphere

void SP_item_d_health_mega (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)genflags->value & GEN_Q1_ONLY)
	{
		SP_item_q1_health_mega (self);
		return;
	}

	self->model = "sprites/dSoul.sp2";
	self->count = 100;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("ditems/getpow.wav");
	self->style = HEALTH_IGNORE_MAX;
	self->s.effects |= (EF_ANIM_ALLFAST);
}



////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////



/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

//Skid added - For Quake1-only mode
	else if ((int)genflags->value & GEN_Q1_ONLY)
	{
		SP_item_q1_health (self);
		return;
	}
	else if ((int)genflags->value & GEN_DOOM_ONLY)
	{
		SP_item_d_health (self);
		return;
	}
//End Skid

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 15;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/n_health.wav");
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_small (edict_t *self)
{
	if (( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) ) ||
		((int)genflags->value & GEN_Q1_ONLY))
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)genflags->value & GEN_DOOM_ONLY)
	{
		SP_item_d_health_small(self);
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

//Skid added - For Quake1-only mode
	else if ((int)genflags->value & GEN_Q1_ONLY)
	{
		SP_item_q1_health_large (self);
		return;
	}
	else if ((int)genflags->value & GEN_DOOM_ONLY)
	{
		SP_item_d_health_large (self);
		return;
	}
//End Skid

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

//Skid added - For Quake1-only mode
	else if ((int)genflags->value & GEN_Q1_ONLY)
	{
		SP_item_q1_health_mega (self);
		return;
	}
	else if ((int)genflags->value & GEN_DOOM_ONLY)
	{
		SP_item_d_health_mega (self);
		return;
	}
//end Skid

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

//Skid added - new armor indexes
	q1_green_armor_index = ITEM_INDEX(FindItem("Armor"));
	q1_yellow_armor_index = ITEM_INDEX(FindItem("Yellow Armor"));
	q1_red_armor_index   = ITEM_INDEX(FindItem("Red Armor"));
	d_green_armor_index = ITEM_INDEX(FindItem("Green Armor"));
	d_blue_armor_index = ITEM_INDEX(FindItem("Blue Armor"));
	w_green_vest_index = ITEM_INDEX(FindItem("Green Vest"));
	w_blue_vest_index = ITEM_INDEX(FindItem("Blue Vest"));
//End Skid
}