#include "g_local.h"

//Skid added
#include "g_sogutil.h"

//Knightmare added
#include "g_sogcvar.h"

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

void Weapon_Keen_Raygun(edict_t *ent);		

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

static int	w_green_vest_index;
static int	w_blue_vest_index;
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
		if (sog_ctf->value &&
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
		gi.sound(ent, CHAN_AUTO, gi.soundindex("idg2items/itemrespawn.wav"), 1, ATTN_NORM, 0);
		break;
	case CLASS_Q1:
		gi.sound(ent, CHAN_AUTO, gi.soundindex("idg3world/teleport.wav"), 1, ATTN_NORM, 0);
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
	//Knightmare- powerup capacity cvar
	if ((skill->value == 1 && quantity >= powerup_max->value) || (skill->value >= 2 && quantity >= (powerup_max->value - 1)))
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
	else if(!deathmatch->value)
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


/*
===============
Pickup_Bandolier

by Gavino
===============
*/
qboolean Pickup_Bandolier (edict_t *ent, edict_t *other)
{
        gitem_t *item;
        int             index;

//Skid added
        if (coop->value)
        {
          int quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
          
          if((ent->item->flags & IT_STAY_COOP) && (quantity > 0))
                return false;
          
          other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
        }
//end Skid

// This code was bad, replaced with better code (hopefully) - Gavino
        switch (other->client->resp.player_class)
        {
		//Knightmare- cvar ammo capacities
                case CLASS_DOOM:
                        if (other->client->pers.max_bullets < idg2_pack_bullets->value)
                                other->client->pers.max_bullets = idg2_pack_bullets->value;
                        if (other->client->pers.max_shells < idg2_pack_shells->value)
                                other->client->pers.max_shells = idg2_pack_shells->value;
                        if (other->client->pers.max_cells < idg2_pack_cells->value)
                                other->client->pers.max_cells = idg2_pack_cells->value;
                        if (other->client->pers.max_rockets < idg2_pack_rockets->value)
                                other->client->pers.max_rockets = idg2_pack_rockets->value;
                        break;
		//Knightmare- cvar ammo capacities
                case CLASS_Q2:
                        if (other->client->pers.max_bullets < bando_bullets->value)
                                other->client->pers.max_bullets = bando_bullets->value;
                        if (other->client->pers.max_shells < bando_shells->value)
                                other->client->pers.max_shells = bando_shells->value;
                        if (other->client->pers.max_cells < bando_cells->value)
                                other->client->pers.max_cells = bando_cells->value;
                        if (other->client->pers.max_slugs < bando_slugs->value)  // Forgot these 2 lines!
                                other->client->pers.max_slugs = bando_slugs->value;
                        break;
                        // Do not add ammo capacity for Axe, Blaze classes
				//Knightmare- do add ammo capacity if old guys' pack increases are enabled
                case CLASS_WOLF:
					if (idg1_pack_increase->value)
					{
						if (other->client->pers.max_bullets < bando_bullets->value)
							other->client->pers.max_bullets = bando_bullets->value;
						if (other->client->pers.max_shells < bando_shells->value)
							other->client->pers.max_shells = bando_shells->value;
						if (other->client->pers.max_cells < bando_cells->value)
							other->client->pers.max_cells = bando_cells->value;
						if (other->client->pers.max_slugs < bando_rockets->value)
							other->client->pers.max_slugs = bando_rockets->value;
					}
                    break;
				case CLASS_Q1:
					if (idg3_pack_increase->value)
					{
						if (other->client->pers.max_bullets < bando_bullets->value)
							other->client->pers.max_bullets = bando_bullets->value;
						if (other->client->pers.max_shells < bando_shells->value)
							other->client->pers.max_shells = bando_shells->value;
						if (other->client->pers.max_cells < bando_cells->value)
							other->client->pers.max_cells = bando_cells->value;
						if (other->client->pers.max_rockets < bando_rockets->value)
							other->client->pers.max_rockets = bando_rockets->value;
					}
                    break;
				default:
					break;


        }       

        // Fixed Ammo code. Axe, Blaze get large ammos
        // Flynn gets exactly the Doom backpack
        // Payne recieves standard bandoleer.

        item = FindItem("Bullets");
        if (item)
        {
                index = ITEM_INDEX(item);

                if (other->client->resp.player_class == CLASS_DOOM)
                        other->client->pers.inventory[index] += 10;
                else
                        other->client->pers.inventory[index] += item->quantity;

                if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
                        other->client->pers.inventory[index] = other->client->pers.max_bullets;
        }

        item = FindItem("Shells");
        if (item)
        {
                index = ITEM_INDEX(item);

                if (other->client->resp.player_class == CLASS_WOLF)
                        other->client->pers.inventory[index] += 0;              
                if (other->client->resp.player_class == CLASS_DOOM)
                        other->client->pers.inventory[index] += 4;
                if (other->client->resp.player_class == CLASS_Q1)
                        other->client->pers.inventory[index] += 40;                     
                if (other->client->resp.player_class == CLASS_Q2)
                        other->client->pers.inventory[index] += item->quantity;

                if (other->client->pers.inventory[index] > other->client->pers.max_shells)
                        other->client->pers.inventory[index] = other->client->pers.max_shells;
        }

        if(other->client->resp.player_class != CLASS_Q2)
        {
                item = FindItem("Cells");
                if (item)
                {
                        index = ITEM_INDEX(item);

                        if (other->client->resp.player_class == CLASS_WOLF)
                                other->client->pers.inventory[index] += 50;             
                        if (other->client->resp.player_class == CLASS_DOOM)
                                other->client->pers.inventory[index] += 20;
                        if (other->client->resp.player_class == CLASS_Q1)
                                other->client->pers.inventory[index] += 12;                     
                        if (other->client->resp.player_class == CLASS_Q2)
                                other->client->pers.inventory[index] += item->quantity;

                        if (other->client->pers.inventory[index] > other->client->pers.max_cells)
                                other->client->pers.inventory[index] = other->client->pers.max_cells;
                }
                item = FindItem("Rockets");
                if (item)
                {
                        index = ITEM_INDEX(item);

                        if (other->client->resp.player_class == CLASS_WOLF)
                                other->client->pers.inventory[index] += 5;              
                        if (other->client->resp.player_class == CLASS_DOOM)
                                other->client->pers.inventory[index] += 1;
                        if (other->client->resp.player_class == CLASS_Q1)
                                other->client->pers.inventory[index] += 10;             
                        if (other->client->resp.player_class == CLASS_Q2)
                                other->client->pers.inventory[index] += item->quantity;

                        if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
                                other->client->pers.inventory[index] = other->client->pers.max_rockets;
                }
        }

        if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
                SetRespawn (ent, ent->item->quantity);

        return true;
}

/*
===============
Pickup_Pack

by Gavino
===============
*/
qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
        gitem_t *item;
        int             index;

        //Skid added
        if (coop->value)
        {
          int quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
          
          if((ent->item->flags & IT_STAY_COOP) && (quantity > 0))
                return false;
          
          other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
        }
//end Skid

        // basically same code as with bandoleer, only dif is Payne - Gavino
        switch (other->client->resp.player_class)
        {
		//Knightmare- cvar ammo capacities
                case CLASS_DOOM:
                        if (other->client->pers.max_bullets < idg2_pack_bullets->value)
                                other->client->pers.max_bullets = idg2_pack_bullets->value;
                        if (other->client->pers.max_shells < idg2_pack_shells->value)
                                other->client->pers.max_shells = idg2_pack_shells->value;
                        if (other->client->pers.max_cells < idg2_pack_cells->value)
                                other->client->pers.max_cells = idg2_pack_cells->value;
                        if (other->client->pers.max_rockets < idg2_pack_rockets->value)
                                other->client->pers.max_rockets = idg2_pack_rockets->value;
                        break;
		//Knightmare- cvar ammo capacities
                case CLASS_Q2:
                        if (other->client->pers.max_bullets < pack_bullets->value)
                                other->client->pers.max_bullets = pack_bullets->value;
                        if (other->client->pers.max_shells < pack_shells->value)
                                other->client->pers.max_shells = pack_shells->value;
                        if (other->client->pers.max_rockets < pack_rockets->value)
                                other->client->pers.max_rockets = pack_rockets->value;
                        if (other->client->pers.max_cells < pack_cells->value)
                                other->client->pers.max_cells = pack_cells->value;
                        if (other->client->pers.max_slugs < pack_slugs->value)
                                other->client->pers.max_slugs = pack_slugs->value;
                        if (other->client->pers.max_grenades < pack_grenades->value)
                                other->client->pers.max_grenades = pack_grenades->value;
                        break;
                        // Do not add ammo capacity for Axe, Blaze classes
				//Knightmare- do add ammo capacity if old guys' pack increases are enabled
                case CLASS_WOLF:
					if (idg1_pack_increase->value)
					{
						if (other->client->pers.max_bullets < pack_bullets->value)
                        	 other->client->pers.max_bullets = pack_bullets->value;
                        if (other->client->pers.max_shells < pack_shells->value)
                             other->client->pers.max_shells = pack_shells->value;
                        if (other->client->pers.max_cells < pack_cells->value)
                             other->client->pers.max_cells = pack_cells->value;
                        if (other->client->pers.max_slugs < pack_rockets->value)
                             other->client->pers.max_slugs = pack_rockets->value;
					}
                    break;
				case CLASS_Q1:
					if (idg3_pack_increase->value)
					{
						if (other->client->pers.max_bullets < pack_bullets->value)
                        	  other->client->pers.max_bullets = pack_bullets->value;
						if (other->client->pers.max_shells < pack_shells->value)
                                other->client->pers.max_shells = pack_shells->value;
						if (other->client->pers.max_cells < pack_cells->value)
                                other->client->pers.max_cells = pack_cells->value;
						if (other->client->pers.max_rockets < pack_rockets->value)
							other->client->pers.max_rockets = pack_rockets->value;
					}
                    break;
				default:
					break;
        }       

        item = FindItem("Bullets");
        if (item)
        {
                index = ITEM_INDEX(item);

                if (other->client->resp.player_class == CLASS_DOOM)
                        other->client->pers.inventory[index] += 10;
                else
                        other->client->pers.inventory[index] += item->quantity;

                if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
                        other->client->pers.inventory[index] = other->client->pers.max_bullets;
        }

        item = FindItem("Shells");
        if (item)
        {
                index = ITEM_INDEX(item);

                if (other->client->resp.player_class == CLASS_WOLF)
                        other->client->pers.inventory[index] += 0;              
                if (other->client->resp.player_class == CLASS_DOOM)
                        other->client->pers.inventory[index] += 4;
                if (other->client->resp.player_class == CLASS_Q1)
                        other->client->pers.inventory[index] += 40;                     
                if (other->client->resp.player_class == CLASS_Q2)
                        other->client->pers.inventory[index] += item->quantity;

                if (other->client->pers.inventory[index] > other->client->pers.max_shells)
                        other->client->pers.inventory[index] = other->client->pers.max_shells;
        }

        item = FindItem("Cells");
        if (item)
        {
                index = ITEM_INDEX(item);

                if (other->client->resp.player_class == CLASS_WOLF)
                        other->client->pers.inventory[index] += 50;             
                if (other->client->resp.player_class == CLASS_DOOM)
                        other->client->pers.inventory[index] += 20;
                if (other->client->resp.player_class == CLASS_Q1)
                        other->client->pers.inventory[index] += 12;                     
                if (other->client->resp.player_class == CLASS_Q2)
                        other->client->pers.inventory[index] += item->quantity;

                if (other->client->pers.inventory[index] > other->client->pers.max_cells)
                        other->client->pers.inventory[index] = other->client->pers.max_cells;
        }

//Skid added
//No grenades or slugs for old guys
        if(other->client->resp.player_class == CLASS_Q2)
        {
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

                if (other->client->resp.player_class == CLASS_WOLF)
                        other->client->pers.inventory[index] += 5;              
                if (other->client->resp.player_class == CLASS_DOOM)
                        other->client->pers.inventory[index] += 1;
                if (other->client->resp.player_class == CLASS_Q1)
                        other->client->pers.inventory[index] += 10;                     
                if (other->client->resp.player_class == CLASS_Q2)
                        other->client->pers.inventory[index] += item->quantity;

                if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
                        other->client->pers.inventory[index] = other->client->pers.max_rockets;
        }

        //if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
        //        SetRespawn (ent, ent->item->quantity);

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
		timeout = (quad_time->value * 10);  //Knightmare- variable quad duration
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
		ent->client->breather_framenum += (breather_time->value * 10);  //Knightmare- variable breather duration
	else
		ent->client->breather_framenum = level.framenum + (breather_time->value * 10);  //Knightmare- variable breather duration
}

//======================================================================

void Use_Envirosuit (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->enviro_framenum > level.framenum)
		ent->client->enviro_framenum += (enviro_time->value * 10);  //Knightmare- variable enviro duration
	else
		ent->client->enviro_framenum = level.framenum + (enviro_time->value * 10);  //Knightmare- variable enviro duration
}

//======================================================================

void Use_Invulnerability (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += (inv_time->value * 10);  //Knightmare- variable invulnerability duration
	else
		ent->client->invincible_framenum = level.framenum + (inv_time->value * 10);  //Knightmare- variable invulnerability duration

	if(ent->client->resp.player_class == CLASS_DOOM)		
		gi.sound(ent, CHAN_ITEM, gi.soundindex("idg2items/powerups/god.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Silencer (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	ent->client->silencer_shots += silencer_shots->value;  //Knightmare- variable silencer shots

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
	case INDEX_W_BM:
	case INDEX_W_BS:
		{
			index = AMMO_BULLETS_INDEX;
			break;
		}
	case INDEX_Q1_CL:
	case INDEX_Q1_CS:
	case INDEX_D_CL:
	case INDEX_D_CS:
	case INDEX_W_PL:
	case INDEX_W_PS:
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
	case INDEX_W_RL:
	case INDEX_W_RS:
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
			{
			// Arno added - Blaze can't pickup Shells either...
				if(ent->client->resp.player_class == CLASS_WOLF)
					index = AMMO_CELLS_INDEX; 	
			// Arno end
				else	
					index = AMMO_SHELLS_INDEX;
			}
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
    case INDEX_W_BM:
	case INDEX_W_BS:
		{
			index = AMMO_BULLETS_INDEX;
			break;
		}
	case INDEX_Q1_CL:
	case INDEX_Q1_CS:
	case INDEX_D_CL:
	case INDEX_D_CS:
	case INDEX_W_PL:
	case INDEX_W_PS:
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
	case INDEX_W_RL:
	case INDEX_W_RS:
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
			{
			// Arno added - Blaze can't pickup Shells either...
				if(ent->client->resp.player_class == CLASS_WOLF)
					index = AMMO_CELLS_INDEX; 	
			// Arno end
				else	
					index = AMMO_SHELLS_INDEX;
			}
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
//Knightmare- changed to Doom's max soulsphere health
	if(player_class == CLASS_DOOM && other->health >= idg2_soul_health->value)
		return false;
//end Skid

//ZOID
	if (other->health >= 250 && ent->count > 25)
		return false;
//ZOID

//Skid added - quakeguy cannot increase health to 101 by small packs
//Knightmare- added cvar to make this switchable
	if((player_class == CLASS_Q1) && (ent->count < 10)  && (other->health >= other->max_health) && (!idg3_allow_health_bonus->value))
	   return false;
//end Skid

	other->health += ent->count;

//Skid	
	if(player_class == CLASS_DOOM && other->health > idg2_soul_health->value)
		other->health = idg2_soul_health->value;
	else
//end Skid

//ZOID
	if (other->health > 250 && ent->count > 25)
		other->health = 250;
//ZOID
	
	if(ent->count == 1)
	{
//Skid added

        ent->item->icon = "i_health";
		switch(player_class)
		{
		case CLASS_Q1:
			ent->item->pickup_sound = "idg3items/health/small.wav";
			break;
		case CLASS_WOLF:
			ent->item->pickup_sound = "idg1items/health/small.wav";
			break;
		case CLASS_DOOM:
			ent->item->pickup_sound = "idg2items/health/small.wav";
			break;
		case CLASS_Q2:
		default:
			ent->item->pickup_sound = "items/s_health.wav";
			break;
		}
	}
	else if (ent->count == 2 || ent->count == 5)
	{
        ent->item->icon = "i_health";
		switch(player_class)
		{
			case CLASS_Q1:
				ent->item->pickup_sound = "idg3items/health/small.wav";
				break;
			case CLASS_WOLF:
				ent->item->pickup_sound = "idg1items/health/small.wav";
				break;
			case CLASS_DOOM:
				ent->item->pickup_sound = "idg2items/health/small.wav";
				break;
			case CLASS_Q2:
			default:
				ent->item->pickup_sound = "items/s_health.wav";
				break;
		}
//end Skid
	}
	else if (ent->count == 10 || ent->count == 15)
	{
//Skid added
//DIFF HEALTH SOUNDS
		ent->item->icon = "i_health";
		switch(player_class)
		{
			case CLASS_Q1:
				ent->item->pickup_sound = "idg3items/health/small.wav";
				break;
			case CLASS_WOLF:
				ent->item->pickup_sound = "idg1items/health/medium.wav";
				break;
			case CLASS_DOOM:
				ent->item->pickup_sound = "idg2items/health/medium.wav";
				break;
			case CLASS_Q2:
			default:
				ent->item->pickup_sound = "items/n_health.wav";
				break;
		}
	}
	else if (ent->count == 25)
	{
		ent->item->icon = "i_health";
		switch(player_class)
		{
			case CLASS_Q1:
				ent->item->pickup_sound = "idg3items/health/medium.wav";
				break;
			case CLASS_WOLF:
				ent->item->pickup_sound = "idg1items/health/large.wav";
				break;
			case CLASS_DOOM:
				ent->item->pickup_sound = "idg2items/health/medium.wav";
				break;
			case CLASS_Q2:
			default:
				ent->item->pickup_sound = "items/l_health.wav";
				break;
		}
	}
	else // (ent->count == 100)
	{
		switch(player_class)
		{
			case CLASS_Q1:
				ent->item->pickup_sound = "idg3items/health/large.wav";
				ent->item->icon = "idg3/p_mega";
				break;
			case CLASS_WOLF:
				ent->item->pickup_sound = "idg1items/powerups/megah.wav";		
                ent->item->icon = "idg1/p_mega";
				break;
			case CLASS_DOOM:
				ent->item->pickup_sound = "idg2items/powerups/megah.wav";
				ent->item->icon = "idg2/p_mega";
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
			if (strcmp(ent->classname, "item_idg1_chalice") == 0)
			{	// chalice-treasure : +2 armor by default
				//Knightmare- added variable value
				if (!old_armor_index)
					other->client->pers.inventory[w_green_vest_index] = idg1_chalice_armor_bonus_value->value;
				else if(other->client->pers.inventory[old_armor_index] < idg1_max_armor->value)
				{ 
					other->client->pers.inventory[old_armor_index] += idg1_chalice_armor_bonus_value->value;
					if(other->client->pers.inventory[old_armor_index] > idg1_max_armor->value)
						other->client->pers.inventory[old_armor_index] = idg1_max_armor->value;	
				}
				else
					return false;
			}
			else if (strcmp(ent->classname, "item_idg1_chest") == 0)
			{	// chest-treasure : +5 armor by default
				//Knightmare- added variable value
				if (!old_armor_index)
					other->client->pers.inventory[w_green_vest_index] = idg1_chest_armor_bonus_value->value;
				else if(other->client->pers.inventory[old_armor_index] < idg1_max_armor->value)
				{ 
					other->client->pers.inventory[old_armor_index] += idg1_chest_armor_bonus_value->value;
					if(other->client->pers.inventory[old_armor_index] > idg1_max_armor->value)
						other->client->pers.inventory[old_armor_index] = idg1_max_armor->value;	
				}
				else
					return false;
			}
			else if (strcmp(ent->classname, "item_idg1_crown") == 0)
			{	// crown-treasure : +10 armor by default
				//Knightmare- added variable value
				if (!old_armor_index)
					other->client->pers.inventory[w_green_vest_index] = idg1_crown_armor_bonus_value->value;
				else if(other->client->pers.inventory[old_armor_index] < idg1_max_armor->value)
				{ 
					other->client->pers.inventory[old_armor_index] += idg1_crown_armor_bonus_value->value;
					if(other->client->pers.inventory[old_armor_index] > idg1_max_armor->value)
						other->client->pers.inventory[old_armor_index] = idg1_max_armor->value;	
				}
				else
					return false;
			}
			else
			{   // cross-treasure or armor_shard from other class
			//Knightmare- same value as q2 shard option
				if (old_guys_same_armor_bonus->value)
				{
					if (!old_armor_index)
						other->client->pers.inventory[w_green_vest_index] = armor_bonus_value->value;
					else if(other->client->pers.inventory[old_armor_index] < idg1_max_armor->value)
						other->client->pers.inventory[old_armor_index] += armor_bonus_value->value;
					else
						return false;
 				}
				else //if no armor stacking-  +1 armor 
				{
					if (!old_armor_index)
						other->client->pers.inventory[w_green_vest_index] = 1;
					else if(other->client->pers.inventory[old_armor_index] < 200)
						other->client->pers.inventory[old_armor_index] += 1;
					else
						return false;
				}
			}
		}
		else //regular armor pickups
		{
			if (!old_armor_index)
				other->client->pers.inventory[new_armor_index] = newinfo->base_count;
			else  //if already have armor
			{
				//Knighmare- optional armor stacking mode
				if (old_guys_allow_armor_stacking->value)
				{
					if (newinfo->normal_protection > oldinfo->normal_protection)
					{
						// calc new armor values
						salvage = oldinfo->normal_protection / newinfo->normal_protection;
						salvagecount = salvage * other->client->pers.inventory[old_armor_index];
						newcount = newinfo->base_count + salvagecount;
						if (newcount > idg1_max_armor->value)
							newcount = idg1_max_armor->value;

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
						if (newcount > idg1_max_armor->value)
							newcount = idg1_max_armor->value;

						// if we're already maxed out then we don't need the new armor
						if (other->client->pers.inventory[old_armor_index] >= newcount)
							return false;

						// update current armor value
						other->client->pers.inventory[old_armor_index] = newcount;
					}
				}
				else	//if no armor stacking
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
		//Knightmare- same vaule as q2 shard option
			if (old_guys_same_armor_bonus->value)
			{
				if (!old_armor_index)
					other->client->pers.inventory[d_green_armor_index] = armor_bonus_value->value;
				else if(other->client->pers.inventory[old_armor_index] < idg2_max_armor->value)
					other->client->pers.inventory[old_armor_index] += armor_bonus_value->value;
				else
					return false;
			}
			else //if no armor stacking - 1 point each
			{
				if (!old_armor_index)
					other->client->pers.inventory[d_green_armor_index] = 1;
				else if(other->client->pers.inventory[old_armor_index] < 200)
					other->client->pers.inventory[old_armor_index] += 1;
				else
					return false;
			}
		}
		else  //regular armor pickups
		{
			if (!old_armor_index)
				other->client->pers.inventory[new_armor_index] = newinfo->base_count;
			else //if already have armor
			{
				//Knighmare- optional armor stacking mode
				if (old_guys_allow_armor_stacking->value)
				{
					if (newinfo->normal_protection > oldinfo->normal_protection)
					{
						// calc new armor values
						salvage = oldinfo->normal_protection / newinfo->normal_protection;
						salvagecount = salvage * other->client->pers.inventory[old_armor_index];
						newcount = newinfo->base_count + salvagecount;
						if (newcount > idg2_max_armor->value)
							newcount = idg2_max_armor->value;

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
						if (newcount > idg2_max_armor->value)
							newcount = idg2_max_armor->value;

						// if we're already maxed out then we don't need the new armor
						if (other->client->pers.inventory[old_armor_index] >= newcount)
							return false;

						// update current armor value
						other->client->pers.inventory[old_armor_index] = newcount;
					}
				}
				else	//if no armor stacking
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
		//Knightmare- same value as Q2 shard option
			if (old_guys_same_armor_bonus->value)
			{
				if (!old_armor_index)
					other->client->pers.inventory[q1_green_armor_index] = armor_bonus_value->value;
				else if(other->client->pers.inventory[old_armor_index] < idg3_max_armor->value)
					other->client->pers.inventory[old_armor_index] += armor_bonus_value->value;
				else
					return false;
			}
			else //if no armor stacking
			{
				if (!old_armor_index)
					other->client->pers.inventory[q1_green_armor_index] = 1;
				else if(other->client->pers.inventory[old_armor_index] < oldinfo->max_count)
					other->client->pers.inventory[old_armor_index] += 1;
				else
					return false;
			}
		}
		else //regular armor pickups
		{
			if (!old_armor_index)
				other->client->pers.inventory[new_armor_index] = newinfo->base_count;
			else //if already have armor
			{
				//Knighmare- optional armor stacking mode
				if (old_guys_allow_armor_stacking->value)
				{
					if (newinfo->normal_protection > oldinfo->normal_protection)
					{
						// calc new armor values
						salvage = oldinfo->normal_protection / newinfo->normal_protection;
						salvagecount = salvage * other->client->pers.inventory[old_armor_index];
						newcount = newinfo->base_count + salvagecount;
						if (newcount > idg3_max_armor->value)
							newcount = idg3_max_armor->value;

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
						if (newcount > idg3_max_armor->value)
							newcount = idg3_max_armor->value;

						// if we're already maxed out then we don't need the new armor
						if (other->client->pers.inventory[old_armor_index] >= newcount)
							return false;

						// update current armor value
						other->client->pers.inventory[old_armor_index] = newcount;
					}
				}
				else	//if no armor stacking
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
		//Knighmare- variable armor shard values
			if (!old_armor_index)
				other->client->pers.inventory[jacket_armor_index] = armor_bonus_value->value;
			else
				other->client->pers.inventory[old_armor_index] += armor_bonus_value->value;
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

			//if picking up stronger armor than already have
			if (newinfo->normal_protection > oldinfo->normal_protection)
			{
				// calc new armor values
				salvage = oldinfo->normal_protection / newinfo->normal_protection;
				salvagecount = salvage * other->client->pers.inventory[old_armor_index];
				newcount = newinfo->base_count + salvagecount;

				//Knightmare- if all classes stacking is enabled, just stack 
				//up to maximum for that class, ignoring limit for each type
				if (old_guys_allow_armor_stacking->value)
				{
					if (newcount > idg4_max_armor->value)
						newcount = idg4_max_armor->value;
				}
				else  //regular armor limit
 				{ //Knightmare- this allows extra stacking for Q2 class beyond the maximum for each type
     			  //when old guys stacking isn't enabled, but the idg4 maximum is > 200
     				if (newinfo->armor == ARMOR_JACKET)
    				{
      					if (newcount > (idg4_max_armor->value / 4))
      						newcount = (idg4_max_armor->value / 4);
     				}
     				if (newinfo->armor == ARMOR_COMBAT)
     				{
      					if (newcount > (idg4_max_armor->value / 2))
       						newcount = (idg4_max_armor->value / 2);
     				}
     				if (newinfo->armor == ARMOR_BODY)
     				{
      					if (newcount > idg4_max_armor->value)
       						newcount = idg4_max_armor->value;
     				}

    			}
				// zero count of old armor so it goes away
				other->client->pers.inventory[old_armor_index] = 0;

				// change armor to new item with computed value
				other->client->pers.inventory[new_armor_index] = newcount; 
			}
			else //if new armor is weaker
			{
				// calc new armor values
				salvage = newinfo->normal_protection / oldinfo->normal_protection;
				salvagecount = salvage * newinfo->base_count;
				newcount = other->client->pers.inventory[old_armor_index] + salvagecount;

				//Knightmare- if all classes stacking is enabled, just stack
				//up to maximum for that class, ignoring limit for each type
				if (old_guys_allow_armor_stacking->value)
				{
					if (newcount > idg4_max_armor->value)
						newcount = idg4_max_armor->value;
				}
				else  //regular armor limit
				{//Knightmare- this allows extra stacking for Q2 class beyond the maximum for each type
					//when old guys stacking isn't enabled, but the idg4 maximum is > 200
					if (oldinfo->armor == ARMOR_JACKET)
					{
						if (newcount > (idg4_max_armor->value / 4))
						newcount = (idg4_max_armor->value / 4);
					}
					if (oldinfo->armor == ARMOR_JACKET)
					{
						if (newcount > (idg4_max_armor->value / 2))
							newcount = (idg4_max_armor->value / 2);
					}
					if (oldinfo->armor == ARMOR_BODY)
					{
						if (newcount > idg4_max_armor->value)
							newcount = idg4_max_armor->value;
					}
				}

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
	if (strcmp(classname, "weapon_idg3_dbshotg") == 0)
		return true;
	if (strcmp(classname, "weapon_idg3_nailg") == 0)
		return true;
	if (strcmp(classname, "weapon_idg3_hnailg") == 0)
		return true;
	if (strcmp(classname, "weapon_idg3_grenl") == 0)
		return true;
	if (strcmp(classname, "weapon_idg3_rocketl") == 0)
		return true;
	if (strcmp(classname, "weapon_idg3_discharger") == 0)
		return true;
	if (strcmp(classname, "weapon_idg2_shotg") == 0)
		return true;
	if (strcmp(classname, "weapon_idg2_dshotg") == 0)
		return true;
	if (strcmp(classname, "weapon_idg2_chaing") == 0)
		return true;
	if (strcmp(classname, "weapon_idg2_rocketl") == 0)
		return true;
	if (strcmp(classname, "weapon_idg2_plasmag") == 0)
		return true;
	if (strcmp(classname, "weapon_idg2_gdbg") == 0)
		return true;
	if (strcmp(classname, "weapon_idg1_machineg") == 0)
		return true;
	if (strcmp(classname, "weapon_idg1_gatling") == 0)
		return true;
	if (strcmp(classname, "weapon_idg1_bazooka") == 0)
		return true;
	if (strcmp(classname, "weapon_idg1_flamet") == 0)
		return true;
	if (strcmp(classname, "weapon_idg1_dgatlingg") == 0)
		return true;
	if (strcmp(classname, "weapon_keen_raygun") == 0)
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
//if(deathmatch->value)  <- Arno commented out, item substitution also in SP and COOP.
//{	//check

	if(((int)sogflags->value & SOG_IDG3_ONLY) && 
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

	else if(((int)sogflags->value & SOG_IDG2_ONLY) && 
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
	else if(((int)sogflags->value & SOG_IDG1_ONLY) && 
			(level.game != CLASS_WOLF) &&
			!(item->flags & IT_WOLF))
	{	
		newitem = GiveNewItem(item,CLASS_WOLF);
		if(newitem)
		{
			ent->classname = newitem->classname;
			ent->s.effects = newitem->world_model_flags;
			item = newitem;
		}	
			//final Precache will be Wolf item
	}
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
//	}
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
		
		if (ItemWeaponCheck(ent->classname) && (int)sogflags->value & SOG_FULLYLOADED )
		{
		    G_FreeEdict (ent);
			return;
		}

		if ( (int)dmflags->value & DF_NO_ARMOR || (int)sogflags->value & SOG_FULLYLOADED  )
		{
			if (item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_ITEMS || (int)sogflags->value & SOG_FULLYLOADED  )
		{
			if (item->pickup == Pickup_Powerup)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_HEALTH || (int)sogflags->value & SOG_FULLYLOADED )
		{
			if (item->pickup == Pickup_Health || item->pickup == Pickup_Adrenaline || item->pickup == Pickup_AncientHead)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_INFINITE_AMMO || (int)sogflags->value & SOG_FULLYLOADED )
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
		if(!sog_ctf->value)
		{
			gi.cvar_forceset("sog_ctf","1");
			gi.dprintf("==== Init SOG-CTF ====\n");
			
			if(sog_team->value)
				gi.cvar_forceset("sog_team", "0");
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
		"weapon_idg3_axe", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Q1_Axe,
		"misc/w_pkup.wav",
		NULL,0,
		"models/weapons/v_idg3axe/tris.md2",
/* icon */		"idg3/w_axe",
/* pickup */	"Axe",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "idg3weapons/axe/ax1.wav idg3weapons/axe/axhit1.wav idg3weapons/axe/axhit2.wav"
	},

///////////////////////////////////
///// Boomstick ///////////////////
///////////////////////////////////

//INDEX = 19 
	{
		"weapon_idg3_shotg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_SG,
		"idg3items/weapon.wav",
		NULL, 0,
		"models/weapons/v_idg3shotg/tris.md2",
/* icon */		"idg3/w_shotg",
/* pickup */	"Boomstick",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "idg3weapons/sshotgun/shotgn2.wav"
	},

///////////////////////////////////
///// Double Barrelled ShotGun ////
/////////////////////////////////// 

//INDEX = 20
	{
		"weapon_idg3_dbshotg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_SSG,
		"idg3items/weapon.wav",
		"models/weapons/g_idg3dbshotg/tris.md2", EF_ROTATE,
		"models/weapons/v_idg3dbshotg/tris.md2",
/* icon */		"idg3/w_dbshotg",
/* pickup */	"Sawed-Off Shotgun",
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "idg3weapons/sshotgun/shotgn2.wav"
	},

///////////////////////////////////
///// NailGun /////////////////////
/////////////////////////////////// 

//INDEX = 21
	{
		"weapon_idg3_nailg", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_NG,
		"idg3items/weapon.wav",
		"models/weapons/g_idg3nailg/tris.md2", EF_ROTATE,
		"models/weapons/v_idg3nailg/tris.md2",
/* icon */		"idg3/w_nailg",
/* pickup */	"Spiker",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "models/objects/idg3nail/tris.md2 idg3weapons/nails/r_end.wav idg3weapons/nails/ric1.wav idg3weapons/nails/ric2.wav idg3weapons/nails/ric3.wav idg3weapons/nails/tink1.wav"
	},


///////////////////////////////////
///// Super NailGun ///////////////
///////////////////////////////////

//INDEX = 22
	{
		"weapon_idg3_hnailg", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_SNG,
		"idg3items/weapon.wav",
		"models/weapons/g_idg3hnailg/tris.md2", EF_ROTATE,
		"models/weapons/v_idg3hnailg/tris.md2",
/* icon */		"idg3/w_hnailg",
/* pickup */	"Heavy Spiker",
		0,
		2,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_CHAINGUN,
		NULL,
		0,		
/* precache */ "models/objects/idg3nail/tris.md2 weapons/plasshot.wav idg3weapons/nails/s_end.wav idg3weapons/nails/ric1.wav idg3weapons/nails/ric2.wav idg3weapons/nails/ric3.wav idg3weapons/nails/tink1.wav"
	},


///////////////////////////////////
///// q1 grenadelauncher //////////
/////////////////////////////////// 

//INDEX = 23
	{
		"weapon_idg3_grenl",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_GL,
		"idg3items/weapon.wav",
		"models/weapons/g_idg3grenl/tris.md2", EF_ROTATE,
		"models/weapons/v_idg3grenl/tris.md2",
/* icon */		"idg3/w_grenadel",
/* pickup */	"Pineapple Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache*/ "sprites/s_explod.sp2 models/objects/idg3grenade/tris.md2 idg3weapons/grenades/bounce.wav idg3weapons/grenades/grenade.wav"
	},


///////////////////////////////////
///// q1 rocket ///////////////////
/////////////////////////////////// 
	
//INDEX = 24		
	{
		"weapon_idg3_rocketl",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_RL,
		"idg3items/weapon.wav",
		"models/weapons/g_idg3rocketl/tris.md2", EF_ROTATE,
		"models/weapons/v_idg3rocketl/tris.md2",
/* icon */		"idg3/w_rocketl",
/* pickup */	"RPG",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,		
/* precache*/ "sprites/s_explod.sp2 models/objects/rocket/tris.md2 weapons/rippfire.wav weapons/rockfly.wav idg3weapons/rocket/r_exp3.wav"
	},


////////////////////////////////////
///// q1 LIGHTNING GUN /////////////
/////////////////////////////////// 
	
//INDEX = 25			
	{
		"weapon_idg3_discharger",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Q1_LG,
		"idg3items/weapon.wav",
		"models/weapons/g_idg3discharger/tris.md2", EF_ROTATE,
		"models/weapons/v_idg3discharger/tris.md2",
/* icon */		"idg3/w_discharger",
/* pickup */	"Discharger",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP|IT_Q1,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "models/monsters/parasite/segment/tris.md2 idg3weapons/lightning/lhit.wav idg3weapons/lightning/lstart.wav "
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
		"weapon_idg2_fists", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_D_Fists,
		"misc/w_pkup.wav",
		NULL,0,
		"models/weapons/v_idg2fists/tris.md2",			
/* icon */		"idg2/w_fist",
/* pickup */	"Fists",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "idg2weapons/punch.wav"
	},		
		
///////////////////////////////////
///// Doom Chainsaw ///////////////
///////////////////////////////////

//INDEX = 27
	{
		"weapon_idg2_saw", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_Saw,
		"idg2items/weapon.wav",
		"models/weapons/g_idg2saw/tris.md2", EF_ROTATE,
		"models/weapons/v_idg2saw/tris.md2",
/* icon */		"idg2/w_chainsaw",
/* pickup */	"Chainsaw",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "idg2weapons/sawidle.wav idg2weapons/sawhit.wav idg2weapons/sawup.wav idg2weapons/sawon.wav"
	},

///////////////////////////////////
///// Doom Pistol /////////////////
///////////////////////////////////

//INDEX = 28
	{
		"weapon_idg2_pistol",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_Pistol,
		"idg2items/weapon.wav",
		NULL,0,
		"models/weapons/v_idg2pistol/tris.md2",
/* icon */		"idg2/w_pistol",
/* pickup */	"Pistol",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "idg2weapons/pistol.wav"
	},

///////////////////////////////////
///// Doom ShotGun ////////////////
///////////////////////////////////

//INDEX = 29
	{
		"weapon_idg2_shotg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_SG,
		"idg2items/weapon.wav",
		"models/weapons/g_idg2shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_idg2shotg/tris.md2", 
/* icon */		"idg2/w_shotgun",
/* pickup */	"Combat Shotgun",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "idg2weapons/sg.wav"
	},

///////////////////////////////////
///// Doom Super ShotGun //////////
///////////////////////////////////

//INDEX = 30
	{
		"weapon_idg2_dshotg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_SSG,
		"idg2items/weapon.wav",
		"models/weapons/g_idg2dbshotg/tris.md2", EF_ROTATE,
		"models/weapons/v_idg2dbshotg/tris.md2",
/* icon */		"idg2/w_sshotgun",
/* pickup */	"Double Barreled Shotgun",
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "idg2weapons/ssg.wav"
	},

///////////////////////////////////
///// Doom ChainGun ///////////////
///////////////////////////////////

//INDEX = 31
	{
		"weapon_idg2_chaing", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_CG,
		"idg2items/weapon.wav",
		"models/weapons/g_idg2chaing/tris.md2", EF_ROTATE,
		"models/weapons/v_idg2chaing/tris.md2",
/* icon */		"idg2/w_chaingun",
/* pickup */	"Mini Gun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "idg2weapons/pistol.wav"
	},


////////////////////////////////////
///// Doom RL //////////////////////
////////////////////////////////////
	
//INDEX = 32		
	{
		"weapon_idg2_rocketl",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_RL,
		"idg2items/weapon.wav",
		"models/weapons/g_idg2rocketl/tris.md2", EF_ROTATE,
		"models/weapons/v_idg2rocketl/tris.md2",
/* icon */		"idg2/w_rlauncher",
/* pickup */	"RL3000",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache*/ "sprites/Drexp.sp2 models/objects/idg2rocket/tris.md2 weapons/rockfly.wav idg2weapons/rocket.wav idg2weapons/rockexp.wav"
	},

///////////////////////////////////
///// Doom PlasmaGun //////////////
///////////////////////////////////

//INDEX = 33	
	{
		"weapon_idg2_plasmag", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_PG,
		"idg2items/weapon.wav",
		"models/weapons/g_idg2plasmag/tris.md2", EF_ROTATE,
		"models/weapons/v_idg2plasmag/tris.md2",
/* icon */		"idg2/w_plasmagun",
/* pickup */	"Plasma Rifle",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "sprites/plasma.sp2 idg2weapons/plasma.wav idg2weapons/phit.wav"
	},

////////////////////////////////////
///// Doom BFG /////////////////////
////////////////////////////////////
	
//INDEX = 34		
	{
		"weapon_idg2_gdbg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_D_BFG,
		"idg2items/weapon.wav",
		"models/weapons/g_idg2gdbg/tris.md2", EF_ROTATE,
		"models/weapons/v_idg2gdbg/tris.md2",
/* icon */		"idg2/w_bfg",
/* pickup */	"Goddamn Big Gun",
		0,
		40,
		"Cells",
		IT_WEAPON|IT_STAY_COOP|IT_DOOM,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 idg2weapons/bfg.wav weapons/bfg__f1y.wav weapons/bfg__l1a.wav idg2weapons/bfgexp.wav"
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
		"weapon_idg1_knife", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_W_Knife,
		"idg1items/weapon.wav",
		NULL,0,
		"models/weapons/v_idg1knife/tris.md2",
/* icon */		"idg1/w_knife",
/* pickup */	"Knife",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "idg1weapons/knifeair.wav idg1weapons/knifehit.wav idg3weapons/nails/tink1.wav"
	},
		
///////////////////////////////////
///// Wolf Pistol /////////////////
///////////////////////////////////

//INDEX = 36				
	{
		"weapon_idg1_pistol",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_W_Pistol,
		"idg1items/weapon.wav",
		NULL,0,								
		"models/weapons/v_idg1pistol/tris.md2",	
/* icon */		"idg1/w_pistol",
/* pickup */	"Walter P-38",					
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "idg1weapons/pshot.wav"
	},


///////////////////////////////////
///// Wolf Machine Gun ////////////
///////////////////////////////////

//INDEX = 37		
	{
		"weapon_idg1_machineg", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_W_MG,
		"idg1items/weapon.wav",
		"models/weapons/g_idg1machineg/tris.md2", EF_ROTATE,	  
		"models/weapons/v_idg1machineg/tris.md2",                 
/* icon */		"idg1/w_machinegun",
/* pickup */	"Machine Pistol 40",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "idg1weapons/mgshot.wav"
	},


///////////////////////////////////
///// Wolf Gatling Gun ////////////
///////////////////////////////////

//INDEX = 38		
	{
		"weapon_idg1_gatlingg", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_W_GG,
		"idg1items/weapon.wav",
		"models/weapons/g_idg1gatlingg/tris.md2", EF_ROTATE,		
		"models/weapons/v_idg1gatlingg/tris.md2",				
/* icon */		"idg1/w_chaingun",
/* pickup */	"Gatling Gun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "idg1weapons/ggshot.wav"
	},

///////////////////////////////////
///// Wolf Bazooka ////////////////
///////////////////////////////////
	
//INDEX = 39
	{
		"weapon_idg1_bazooka",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_W_RL,   
		"idg1items/weapon.wav",
		"models/weapons/g_idg1bazooka/tris.md2", EF_ROTATE,		
		"models/weapons/v_idg1bazooka/tris.md2",				
/* icon */		"idg1/w_rlauncher",
/* pickup */	"Bazooka",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,		
/* precache*/ "sprites/wrexp.sp2 models/objects/idg1rocket/tris.md2 weapons/rockfly.wav idg2weapons/rocket.wav idg1weapons/explode.wav"
	},


///////////////////////////////////
///// Wolf FlameThrower ///////////
///////////////////////////////////

//INDEX = 40
	{
		"weapon_idg1_flamet", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_W_FT,
		"idg1items/weapon.wav",
		"models/weapons/g_idg1flamet/tris.md2", EF_ROTATE,		
		"models/weapons/v_idg1flamet/tris.md2",				
/* icon */		"idg1/w_flame",
/* pickup */	"FlameThrower",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_HYPERBLASTER,
		NULL,
		0,		
/* precache */ "sprites/wflame.sp2 idg1weapons/ftshot.wav idg1weapons/fthit.wav"
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
		"ammo_idg3_shellsL",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg3items/ammo.wav",
		"models/items/idg3ammo/shellsL/tris.md2", 0,
		NULL,
/* icon */		"idg3/a_shells", 
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
		"ammo_idg3_shellsS",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg3items/ammo.wav",
		"models/items/idg3ammo/shellsS/tris.md2", 0,
		NULL,
/* icon */		"idg3/a_shells", 
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
		"ammo_idg3_nailsL",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg3items/ammo.wav",
		"models/items/idg3ammo/nailsL/tris.md2", 0,
		NULL,
/* icon */		"idg3/a_nails", 
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
		"ammo_idg3_nailsS",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg3items/ammo.wav",
		"models/items/idg3ammo/nailsS/tris.md2", 0,
		NULL,
/* icon */		"idg3/a_nails", 
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
		"ammo_idg3_cellsL",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg3items/ammo.wav",
		"models/items/idg3ammo/cellsL/tris.md2", 0,
		NULL,
/* icon */		"idg3/a_cells", 
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
		"ammo_idg3_cellsS",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg3items/ammo.wav",
		"models/items/idg3ammo/cellsS/tris.md2", 0,
		NULL,
/* icon */		"idg3/a_cells", 
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
		"ammo_idg3_rocketsL",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg3items/ammo.wav",
		"models/items/idg3ammo/rocketsL/tris.md2", 0,
		NULL,
/* icon */		"idg3/a_rockets", 
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
		"ammo_idg3_rocketsS",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg3items/ammo.wav",
		"models/items/idg3ammo/rocketsS/tris.md2", 0,
		NULL,
/* icon */		"idg3/a_rockets", 
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
		"ammo_idg2_shellsL",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg2items/ammo.wav",
		"models/items/idg2ammo/shellsL/tris.md2", 0, 
		NULL,
/* icon */		"idg2/a_shells",
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
		"ammo_idg2_shellsS",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg2items/ammo.wav",
		"models/items/idg2ammo/shellsS/tris.md2", 0, 
		NULL,
/* icon */		"idg2/a_shells",
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
		"ammo_idg2_bulletsL",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg2items/ammo.wav",
		"models/items/idg2ammo/bulletsL/tris.md2", 0,
		NULL,
/* icon */		"idg2/a_bullets", 
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
		"ammo_idg2_bulletsS",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg2items/ammo.wav",
		"models/items/idg2ammo/bulletsS/tris.md2", 0,
		NULL,
/* icon */		"idg2/a_bullets", 
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
		"ammo_idg2_plasmaL",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg2items/ammo.wav",
		"models/items/idg2ammo/plasmaL/tris.md2", 0, 
		NULL,
/* icon */		"idg2/a_cells", 
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
		"ammo_idg2_plasmaS",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg2items/ammo.wav",
		"models/items/idg2ammo/plasmaS/tris.md2", 0, 
		NULL,
/* icon */		"idg2/a_cells", 
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
		"ammo_idg2_rocketsL",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg2items/ammo.wav",
		"models/items/idg2ammo/rocketsL/tris.md2", 0, 
		NULL,
/* icon */		"idg2/a_rockets", 
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
		"ammo_idg2_rocketsS",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg2items/ammo.wav",
		"models/items/idg2ammo/rocketsS/tris.md2", 0, 
		NULL,
/* icon */		"idg2/a_rockets", 
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
		"item_idg3_armorR", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"idg3items/armor.wav",
		"models/items/idg3armor/red/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"idg3/i_redarmor", 
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
		"item_idg3_armorY", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"idg3items/armor.wav",
		"models/items/idg3armor/yellow/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"idg3/i_yellowarmor", 
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
		"item_idg3_armorG", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"idg3items/armor.wav",
		"models/items/idg3armor/green/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"idg3/i_greenarmor", 
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
		"item_idg2_armorB", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"idg2items/armor.wav",
		"models/items/idg2armor/blue/tris.md2", (EF_ROTATE), 
		NULL,
/* icon */		"idg2/i_bluearmor", 
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
		"item_idg2_armorG", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"idg2items/armor.wav",
		"models/items/idg2armor/green/tris.md2", (EF_ROTATE),
		NULL,
/* icon */		"idg2/i_greenarmor", 
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
		"item_idg2_armorS", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"idg2items/health/small.wav",
		"models/items/idg2armor/helmet/tris.md2", 0,
		NULL,
/* icon */		"idg2/i_greenarmor", 
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
		"item_idg3_quad",
		Pickup_Powerup,
		Use_Quad,
		Drop_General,
		NULL,
		"items/damage.wav",
		"models/items/idg3powerups/quad/tris.md2", (EF_ROTATE|EF_FLAG2), 
		NULL,
/* icon */		"idg3/p_quad", 
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
		"item_idg3_invis",
		Pickup_Powerup,
		Use_Invis,
		Drop_General,
		NULL,
		"idg3items/powerups/invis.wav",
		"models/items/idg3powerups/invis/tris.md2", EF_ROTATE, 
		NULL,
/* icon */		"idg3/p_invis", 
/* pickup */	"Ring of Shadows",
/* width */		2,
		300,
		NULL,
		IT_POWERUP|IT_Q1,
		0,
		NULL,
		0,
/* precache */ "models/items/idg3powerups/invis/eyes/tris.md2 idg3items/powerups/invisfade.wav"
	},

//=================================================================

// Quake1 PENT												

//INDEX = 71
	{
		"item_idg3_god",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"items/protect.wav",
		"models/items/idg3powerups/god/tris.md2", (EF_ROTATE|EF_FLAG1),
		NULL,
/* icon */		"idg3/p_god", 
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
		"item_idg3_envsuit",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"idg3items/powerups/envsuit.wav",
		"models/items/idg3powerups/envsuit/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"idg3/p_suit",						
/* pickup */	"Biosuit",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP|IT_Q1,
		0,
		NULL,
		0,
/* precache */ "idg3items/powerups/envsuitfade.wav"
	},

//=================================================================

// Quake1 Player Backpack							

//INDEX = 73
	{
		"item_idg3_backpack",
		Pickup_Pack,
		NULL,
		Drop_General,
		NULL,
		"idg3items/weapon.wav",
		"models/items/idg3powerups/backpack/tris.md2", EF_ROTATE, 
		NULL,
/* icon */ 		"idg3/i_pack",
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
		"item_idg2_quad",
		Pickup_Powerup,
		Use_Berserk,
		Drop_General,
		NULL,
		"idg2items/powerups/berserk.wav",
		"models/items/idg2powerups/quad/tris.md2", 0,  
		NULL,
/* icon */		"idg2/p_berserk",
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
		"item_idg2_invis",
		Pickup_Powerup,
		Use_Invis,
		Drop_General,
		NULL,
		"idg2items/powerups/invis.wav",
		"models/items/idg2powerups/invis/tris.md2", EF_ROTATE|EF_ANIM01, 
		NULL,
/* icon */		"idg2/p_invis",
/* pickup */	"Invisibility",
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

// Doom GOD MODE											

//INDEX = 76
	{
		"item_idg2_god",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"idg2items/powerups/god.wav",
		"models/items/idg2powerups/god/tris.md2", EF_ROTATE, 
		NULL,
/* icon */		"idg2/p_god",
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

// Doom Mega Sphere												

//INDEX = 77
	{  //Knightmare- this acts like the original megasphere from Doom
	   // so I changed its name, model and pic accordingly
		"item_idg2_soulsphere",
		Pickup_Powerup,
		Use_MegaSphere,
		Drop_General,
		NULL,
		"idg2items/powerups/megah.wav",
		"models/items/idg2powerups/megasphere/tris.md2", EF_ROTATE|EF_ANIM01,
		NULL,
/* icon */		"idg2/p_mega",
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
		"item_idg2_envsuit",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"idg2items/powerups/envsuit.wav",
		"models/items/idg2powerups/envsuit/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"idg2/p_suit",
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
		"item_idg2_backpack",
		Pickup_Bandolier,		// FIXME
		NULL,
		Drop_General,
		NULL,
		"idg2items/ammo.wav",
		"models/items/idg3powerups/backpack/tris.md2", 0, 
		NULL,
/* icon */		"idg2/i_pack",
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
		"key_idg3_gold",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"idg3items/key.wav",
		"models/items/idg3keys/gold/tris.md2", EF_ROTATE,
		NULL,
		"idg3/k_gold",						
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
		"key_idg3_silver",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"idg3items/key.wav",
		"models/items/idg3keys/silver/tris.md2", EF_ROTATE,
		NULL,
		"idg3/k_silver",								
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
//
// Arno - Quake1-RUNEKEYS changed into WOLF KEYS


//INDEX = 91
	{
		"key_idg1_gold",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"idg1items/key.wav",
		"models/items/idg1keys/gold/tris.md2",EF_ROTATE,
		NULL,
		"idg1/k_gold",						
		"Gold NAZI Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_WOLF,
		0,
		NULL,
		0,
/* precache */ ""
	},

//======================================================================================

//WOLF Silver Key

//INDEX = 92
	{
		"key_idg1_silver",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"idg1items/key.wav",
		"models/items/idg1keys/silver/tris.md2", EF_ROTATE,
		NULL,
		"idg1/k_silver",						
		"Silver NAZI Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_WOLF,
		0,
		NULL,
		0,
/* precache */ ""
	},

//======================================================================================
//======================================================================================
// RUNES 
//
// Arno - changed into WOLF-TREASURES
//
//======================================================================================
//======================================================================================

// cross

//INDEX = 93
	{
		"item_idg1_cross",
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"idg1items/treasures.wav",
		"models/items/idg1treasures/cross/tris.md2", 0,
		NULL,
		"idg1/i_cross",						
		"Cross",
		2,
		0,
		NULL,
		IT_ARMOR|IT_WOLF,
		0,
		NULL,
		ARMOR_SHARD,

/* precache */ ""
		},

//======================================================================================

// Chalice

//INDEX = 94
	{
		"item_idg1_chalice",
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"idg1items/treasures.wav",
		"models/items/idg1treasures/chalice/tris.md2", 0,
		NULL,
		"idg1/i_chalice",						
		"Chalice",
		2,
		0,
		NULL,
		IT_ARMOR|IT_WOLF,
		0,
		NULL,
		ARMOR_SHARD,
/* precache */ ""
		},

//======================================================================================

// Chest

//INDEX = 95
	{
		"item_idg1_chest",
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"idg1items/treasures.wav",
		"models/items/idg1treasures/chest/tris.md2", 0,
		NULL,
		"idg1/i_chest",						
		"Chest",
		2,
		0,
		NULL,
		IT_ARMOR|IT_WOLF,
		0,
		NULL,
		ARMOR_SHARD,
/* precache */ ""
		},

//======================================================================================

// Crown

//INDEX = 96
	{
		"item_idg1_crown",
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"idg1items/treasures.wav",
		"models/items/idg1treasures/crown/tris.md2", 0,
		NULL,
		"idg1/i_crown",						
		"Crown",
		2,
		0,
		NULL,
		IT_ARMOR|IT_WOLF,
		0,
		NULL,
		ARMOR_SHARD,
/* precache */ ""
		},

//======================================================================================
//======================================================================================

// weapon_idg3_grapple (.3 .3 1) (-16 -16 -16) (16 16 16)
//always owned by Quakeguys, never in the world

// Arno - No longer in use

//INDEX = 97
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
		"idg3items/ammo.wav",
		"models/items/idg3ammo/rocketsS/tris.md2", 0,
		NULL,
/* icon */		"idg3/a_rockets", 
/* pickup */	"Rockets",
/* width */		3,
		5, //Knightmare- was 1, changed value to 5 to be fair
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
		"idg3items/ammo.wav",
		"models/items/idg3armor/green/tris.md2", 0,
		NULL,
/* icon */		"idg3/i_greenarmor", 
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
		"ammo_idg1_bulletsL",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg1items/weapon.wav",
		"models/items/idg1ammo/bulletsL/tris.md2", 0,
		NULL,
/* icon */		"idg1/a_bullets", 
/* pickup */	"Large Ammobox",
/* width */		3,
		100,
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
		"ammo_idg1_bulletsS",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg1items/weapon.wav",
		"models/items/idg1ammo/bulletsS/tris.md2", 0,
		NULL,
/* icon */		"idg1/a_bullets", 
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

//Wolfie Rockets Large
//INDEX = 118
	{
		"ammo_idg1_rocketL",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg1items/weapon.wav",
		"models/items/idg1ammo/rocketsL/tris.md2", 0,
		NULL,
/* icon */		"idg1/a_rockets", 
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
	
//Wolfie Petrol Large
//INDEX = 119
	{
		"ammo_idg1_petrolL",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg1items/weapon.wav",
		"models/items/idg1ammo/petrolL/tris.md2", 0,
		NULL,
/* icon */		"idg1/a_petrol", 
/* pickup */	"Petrol",
/* width */		3,
		50,
		NULL,
		IT_AMMO|IT_WOLF,
		0,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},

//=================================================================

///////////////////////////////////
///// Wolf Double Gatling Gun /////
///////////////////////////////////

//INDEX = 120
	{
		"weapon_idg1_dgatlingg", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_W_GG2,
		"idg1items/weapon.wav",
		"models/weapons/g_idg1gatlingg/tris.md2", EF_ROTATE,		
		"models/weapons/v_idg1dgatlingg/tris.md2",				
/* icon */		"idg1/w_doublechaingun",
/* pickup */	"Gatling Guns",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP|IT_WOLF,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "idg1weapons/ggshot.wav"
	},

//=================================================================
//=================================================================
//=================================================================
// WOLF

//INDEX = 121
	{
		"item_idg1_armorB", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"idg1items/armor.wav",
		"models/items/idg1armor/blue/tris.md2", EF_ROTATE, 
		NULL,
/* icon */		"idg1/i_bluearmor", 
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
		"item_idg1_armorG", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"idg1items/armor.wav",
		"models/items/idg1armor/green/tris.md2", EF_ROTATE, 
		NULL,
/* icon */		"idg1/i_greenarmor", 
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

// Wolfie Petrol Small
//INDEX = 123
	{
		"ammo_idg1_petrolS",     
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg1items/weapon.wav",
		"models/items/idg1ammo/petrolS/tris.md2", 0,
		NULL,
/* icon */		"idg1/a_petrol",
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

//Wolfie Rockets Small
//INDEX = 124
	{
		"ammo_idg1_rocketS",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg1items/weapon.wav",
		"models/items/idg1ammo/rocketsS/tris.md2", 0,
		NULL,
/* icon */		"idg1/a_rockets", 
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


//=================================================================
									
//Wolfie Bullets Medium
//INDEX = 125
	{
		"ammo_idg1_bulletsM",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg1items/ammo.wav",
		"models/items/idg1ammo/bulletsM/tris.md2", 0,
		NULL,
/* icon */		"idg1/a_bullets", 
/* pickup */	"Ammo Box",
/* width */		3,
		50,
		NULL,
		IT_AMMO|IT_WOLF,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},



//======================================================================================

// Wolfie Spear of Destiny

/*QUAKED item_idg1_god (.3 .3 1) (-16 -16 -16) (16 16 16) */

//INDEX = 126
	{
		"item_idg1_god",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"idg1items/powerups/god.wav",
		"models/items/idg1powerups/god/tris.md2", (EF_PENT),
		NULL,
/* icon */		"idg1/p_god",
/* pickup */	"Invulnerability",
/* width */		2,
		300,
		NULL,
		IT_POWERUP|IT_WOLF,
		0,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
	},

//INDEX = 127
// Rockets to Grenades substitute
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
//INDEX = 128
//Rockets to Grenades substitute
	{
		"ammo_w_rocksG",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"idg1items/ammo.wav",
		"models/items/idg1ammo/rocketss/tris.md2", 0,
		NULL,
		"idg1/a_rockets", 
		"Bazooka Rockets",
		3,
		1,
		NULL,
		IT_AMMO|IT_WOLF,
		0,
		NULL,
		AMMO_ROCKETS,
		""
	},



// DOOM - KEYS

//INDEX = 129
	{
		"key_idg2_card_blue",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"idg2items/key.wav",
		"models/items/idg2keys/bluecard/tris.md2",EF_ROTATE,
		NULL,
		"idg2/k_bluecard",						
		"Blue Key Card",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_DOOM,
		0,
		NULL,
		0,
		""
	},


//INDEX = 130
	{
		"key_idg2_card_yellow",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"idg2items/key.wav",
		"models/items/idg2keys/yellowcard/tris.md2",EF_ROTATE,
		NULL,
		"idg2/k_yellowcard",						
		"Yellow Key Card",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_DOOM,
		0,
		NULL,
		0,
		""
	},

//INDEX = 131
	{
		"key_idg2_card_red",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"idg2items/key.wav",
		"models/items/idg2keys/redcard/tris.md2",EF_ROTATE,
		NULL,
		"idg2/k_redcard",						
		"Red Key Card",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY|IT_DOOM,
		0,
		NULL,
		0,

		""
	},
// INDEX = 132
	{
		"weapon_keen_raygun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Keen_Raygun,
		"misc/w_pkup.wav",
		"models/weapons/g_raygun/tris.md2", EF_ROTATE,
		"models/weapons/v_raygun/tris.md2",
/* icon */		"w_raygun",
/* pickup */	"Commander Keen Raygun",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "idg3weapons/lightning/lstart.wav"
	},
//	 end of list marker

	{NULL}
};



//======================================
// QUAKE 1 Health items
//======================================

void SP_item_d_health (edict_t *self);
void SP_item_d_health_small (edict_t *self);
void SP_item_d_health_large (edict_t *self);
void SP_item_d_health_mega (edict_t *self);


void SP_item_idg1_healthS (edict_t *self);
void SP_item_idg1_healthM (edict_t *self);
void SP_item_idg1_healthL (edict_t *self);
void SP_item_idg1_megahealth (edict_t *self);


void SP_item_q1_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		SP_item_d_health (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_item_idg1_healthM (self);
		return;
	}

	self->model = "models/items/idg3health/small/tris.md2"; 
	self->count = 15;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("idg3items/health/small.wav");
	gi.imageindex ("i_health");
}


void SP_item_q1_health_large (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		SP_item_d_health_large (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_item_idg1_healthL (self);
		return;
	}
	self->model = "models/items/idg3health/large/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("idg3items/health/large.wav");
	gi.imageindex ("i_health");
}


void SP_item_q1_health_mega (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		SP_item_d_health_mega (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_item_idg1_megahealth (self);
		return;
	}
	
	self->model = "models/items/idg3powerups/megahealth/tris.md2";
	self->count = 100;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("idg3items/powerups/megah.wav");
	gi.imageindex ("i_health");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED; 
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

//======================================
// DOOM Health items
//======================================


void SP_item_d_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG3_ONLY)
	{
		SP_item_q1_health (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_item_idg1_healthM (self);
		return;
	}

	self->model = "models/items/idg2health/medium/tris.md2"; 
	self->count = 10;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("idg2items/health/medium.wav");
}


void SP_item_d_health_small (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG3_ONLY)
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_item_idg1_healthS (self);
		return;
	}
 
	self->model = "models/items/idg2health/small/tris.md2";
	//Knightmare- if old guys armor stacking, same value as q2 stimpack
	if (old_guys_same_health_bonus->value)
		self->count = health_bonus_value->value;
	else
		self->count = 1;
	SpawnItem (self, FindItem ("Health"));
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex ("idg2items/health/small.wav");
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
	else if ((int)sogflags->value & SOG_IDG3_ONLY)
	{
		SP_item_q1_health_large (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_item_idg1_healthL (self);
		return;
	}
	
	self->model = "models/items/idg2health/large/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("idg2items/health/large.wav");
}

/*
Doom MegaSphere
SSDwellah
*/

void animmegasphere(edict_t *ent)
{
	ent->s.frame = (ent->s.frame<1)?ent->s.frame+1:0;
}

void SP_item_d_health_mega (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG3_ONLY)
	{
		SP_item_q1_health_mega (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_item_idg1_megahealth (self);
		return;
	}
	
	self->model = "models/items/idg2powerups/megasphere/tris.md2";
	self->count = 100;

	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("idg2items/powerups/megah.wav");
	self->style = HEALTH_IGNORE_MAX;
	self->s.effects |= EF_ROTATE;
	self->prethink = animmegasphere;
}

/*
======================================

SSDwellah's patented half-assed animation support

======================================
*/

//Doom God powerup


void animgod(edict_t *ent)
{
	ent->s.frame= (ent->s.frame<3)?ent->s.frame+1:0;
}

void SP_item_idg2_god (edict_t *self)
{

	self->model = "models/items/idg2powerups/god/tris.md2";

	SpawnItem (self, FindItem ("item_d_god"));
	gi.soundindex ("idg2items/powerups/god.wav");
	self->s.effects |= EF_ROTATE;
	self->prethink = animgod;
}

//======================================
//
// Arno added - WOLF Health items
//
//======================================


// Dog Food

void SP_item_idg1_healthS (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		SP_item_d_health_small (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG3_ONLY)
	{
		SP_item_q1_health (self);
		return;
	}
	self->model = "models/items/idg1health/small/tris.md2"; 
	self->count = 5;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("idg1items/health/small.wav");
	gi.imageindex ("i_health");
}


// Dinner

void SP_item_idg1_healthM (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		SP_item_d_health (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG3_ONLY)
	{
		SP_item_q1_health (self);
		return;
	}

	self->model = "models/items/idg1health/medium/tris.md2";
	self->count = 10;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("idg1items/health/medium.wav");
	gi.imageindex ("i_health");
}


// Wolf Medikit

void SP_item_idg1_healthL (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		SP_item_d_health_large (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG3_ONLY)
	{
		SP_item_q1_health_large (self);
		return;
	}

	self->model = "models/items/idg1health/large/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("idg1items/health/large.wav");
	gi.imageindex ("i_health");
}

// Wolf Megasphere

void SP_item_idg1_megahealth (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		SP_item_d_health_mega (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG3_ONLY)
	{
		SP_item_q1_health_mega (self);
		return;
	}
	self->model = "models/items/idg1powerups/megahealth/tris.md2";
	self->count = 100;
	SpawnItem (self, FindItem ("Health"));
	self->style = HEALTH_IGNORE_MAX;
	self->s.effects |= EF_ROTATE;
	self->s.renderfx |= RF_FULLBRIGHT;
	gi.soundindex ("idg1items/powerups/megah.wav");
	gi.imageindex ("idg1/p_mega");
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
	else if ((int)sogflags->value & SOG_IDG3_ONLY)
	{
		SP_item_q1_health (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		SP_item_d_health (self);
		return;
	}
//End Skid
	else if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_item_idg1_healthM (self);
		return;
	}
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
		((int)sogflags->value & SOG_IDG3_ONLY))
	{
		G_FreeEdict (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		SP_item_d_health_small(self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_item_idg1_healthS (self);
		return;
	}
	
	self->model = "models/items/healing/stimpack/tris.md2";
	//Knightmare- variable value
	self->count = health_bonus_value->value;
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
	else if ((int)sogflags->value & SOG_IDG3_ONLY)
	{
		SP_item_q1_health_large (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		SP_item_d_health_large (self);
		return;
	}
//End Skid
	else if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_item_idg1_healthL (self);
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

//Skid added - For Quake1-only mode
	else if ((int)sogflags->value & SOG_IDG3_ONLY)
	{
		SP_item_q1_health_mega (self);
		return;
	}
	else if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		SP_item_d_health_mega (self);
		return;
	}
//end Skid
	else if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_item_idg1_megahealth (self);
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