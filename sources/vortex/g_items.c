#include "g_local.h"
#include "teamplay.h"

qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Use_Weapon2 (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);

void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
//K03 End
gitem_armor_t jacketarmor_info	= { 25, 200, .80, .60, ARMOR_BODY};//K03
gitem_armor_t combatarmor_info	= { 50, 200, .80, .60, ARMOR_BODY};//K03
gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};

int	jacket_armor_index;
int	combat_armor_index;
int resistance_index;
int	strength_index;
int regeneration_index;
int	haste_index;
int	body_armor_index;
int	power_cube_index;
int flag_index;
int	red_flag_index;
int blue_flag_index;
int halo_index;

//ammo
int	bullet_index;
int	shell_index;
int	grenade_index;
int	rocket_index;
int	slug_index;
int	cell_index;

static int	power_screen_index;
static int	power_shield_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

void Use_Quad (edict_t *ent, gitem_t *item);
// RAFAEL
void Use_QuadFire (edict_t *ent, gitem_t *item);

static int	quad_drop_timeout_hack;
// RAFAEL
static int	quad_fire_drop_timeout_hack;

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

		for (count = 0, ent = master; ent; ent = ent->chain, count++)
			;

		choice = rand() % count;

		for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
			;
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	gi.linkentity (ent);

	if(ent->classname[0] == 'R') return;

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
		if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		}
		// RAFAEL
		else if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_QuadFire) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_QuadFire) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_fire_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		}
	}

	return true;
}

void Drop_General (edict_t *ent, gitem_t *item)
{
	Drop_Item (ent, item);
	if (item->quantity > 0) // Chamooze
		ent->client->pers.inventory[ITEM_INDEX(item)] -= item->quantity;
	else
		ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
}

float Get_yaw (vec3_t vec);

qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other)
{
//	int i;
	//que_t *slot;

	if (!deathmatch->value)
		other->max_health += 1;

	if (other->health < other->max_health)
		other->health = other->max_health;
//GHz START
	// adrenaline heals all curses
	CurseRemove(other, 0);
	//RemoveAllCurses(other);
//GHz END
	

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

	if (other->client->pers.max_bullets < 250)
		other->client->pers.max_bullets = 250;
	if (other->client->pers.max_shells < 150)
		other->client->pers.max_shells = 150;
	if (other->client->pers.max_cells < 250)
		other->client->pers.max_cells = 250;
	if (other->client->pers.max_slugs < 75)
		other->client->pers.max_slugs = 75;
	// RAFAEL
	if (other->client->pers.max_magslug < 75)
		other->client->pers.max_magslug = 75;

	item = Fdi_BULLETS;//FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = Fdi_SHELLS;//FindItem("Shells");
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

qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	//3.02 Give them some ammo for each ammo type
    V_GiveAmmoClip(other, 2, AMMO_BULLETS);
	V_GiveAmmoClip(other, 2, AMMO_SHELLS);
	V_GiveAmmoClip(other, 2, AMMO_GRENADES);
	V_GiveAmmoClip(other, 2, AMMO_ROCKETS);
	V_GiveAmmoClip(other, 2, AMMO_CELLS);
	V_GiveAmmoClip(other, 2, AMMO_SLUGS);

	// RAFAEL
	item = Fdi_MAGSLUGS;//FindItem ("Mag Slug");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_magslug)
			other->client->pers.inventory[index] = other->client->pers.max_magslug;
	}
	//K03 Begin
	item = Fdi_POWERCUBE;
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity*4;
		other->myskills.inventory[index] = other->client->pers.inventory[index];
	}
	item = Fdi_TBALL;
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		other->myskills.inventory[index] = other->client->pers.inventory[index];
	}
	Check_full(other);
	//K03 End

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
// =====================================================================

// RAFAEL
void Use_QuadFire (edict_t *ent, gitem_t *item)
{
	int		timeout;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (quad_fire_drop_timeout_hack)
	{
		timeout = quad_fire_drop_timeout_hack;
		quad_fire_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->client->quadfire_framenum > level.framenum)
		ent->client->quadfire_framenum += timeout;
	else
		ent->client->quadfire_framenum = level.framenum + timeout;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/quadfire1.wav"), 1, ATTN_NORM, 0);
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
#ifdef PRINT_DEBUGINFO
gi.dprintf("%s is picking up a %s in Pickup_Key()\n", ent->client->pers.netname, other->classname);
#endif

	if (ent->count > 0)
		other->client->pers.inventory[ITEM_INDEX(ent->item)] += ent->count;
	else
		other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	return true;
}

//======================================================================

qboolean Add_Ammo (edict_t *ent, gitem_t *item, float count)  
{ 
     float qty; 
     if (!ent->client)  
          return false; 
       
     //How much of a pack do we have?  
	 if (item->quantity)
		qty = count / (float)item->quantity; 
	 else
		 qty = 1;
  
     //Give them the ammo  
     return V_GiveAmmoClip(ent, qty, item->tag);  
}

qboolean Pickup_Ammo (edict_t *ent, edict_t *other) 
{  
     int          count;  
     //float qty;  
 
	 if ((other->client || other->mtype) &&  // knights and polts can't get ammo in pvm modes
		 (pvm->value || invasion->value) &&
		 // polts and kn only if the ammo is not cells
		 ((isMorphingPolt(ent) || other->myskills.class_num == CLASS_PALADIN) && strcmp(ent->classname, "ammo_cells")))
		 return false;

     if (ent->count)
		 count = ent->count; 
     else
	 {
		 gitem_t *item = ent->item;
		 if (item->tag == AMMO_SHELLS)
			item->quantity = SHELLS_PICKUP;
		else if (item->tag == AMMO_CELLS)
			item->quantity = CELLS_PICKUP;
		else if (item->tag == AMMO_SLUGS)
			item->quantity = SLUGS_PICKUP;
		else if (item->tag == AMMO_ROCKETS)
			item->quantity = ROCKETS_PICKUP;
		else if (item->tag == AMMO_BULLETS)
			item->quantity = BULLETS_PICKUP;
		else if (item->tag == AMMO_GRENADES)
			item->quantity = GRENADES_PICKUP;

		 count = ent->item->quantity; 
	 }
 
     //Give them the ammo 
    if (!Add_Ammo(other, ent->item, count))  
          return false;  
  
     //Respawn!  
     if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))  
          SetRespawn (ent, 30); 
     return true; 
}

void Drop_Ammo (edict_t *ent, gitem_t *item) 
{ 
     edict_t    *dropped; 
     int        index;  
     int		amount; 
 
     index = ITEM_INDEX(item); 
     dropped = Drop_Item (ent, item); 
 
     //Figure out what the pack's quantity should be 
     amount = (float)ent->client->pers.inventory[index]; 
     if (ent->client->pers.inventory[index] >= item->quantity) 
          amount = (float)item->quantity; 
	 //Talent: Improved HA Pickup
     else if (getTalentSlot(ent, TALENT_BASIC_HA) != -1)  
          amount /= 1.0 + (0.2 * getTalentLevel(ent, TALENT_BASIC_HA));       
     dropped->count = amount;//(int)ceil(amount); 
	
     //Take their ammo away 
     V_GiveAmmoClip(ent, -1.0, item->tag); 
      
     //Done 
     ValidateSelectedItem (ent); 
}


//======================================================================

void MegaHealth_think (edict_t *self)
{
	if (!self->owner)
	{
		SetRespawn (self, 20);
		return;
	}

	if (self->owner->health > self->owner->max_health && self->owner->megahealth)
	{
		self->nextthink = level.time + 1;
		self->owner->health -= 1;

		if (PM_PlayerHasMonster(self->owner))
			self->owner->owner->health = self->owner->health;
		return;
	}

	self->owner->megahealth = NULL;	//4.0

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (self, 20);
	else
		G_FreeEdict (self);
}

qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
	int count;
	float temp=1.0;

	//3.0 cursed players can't pick up health
	if (que_findtype(other->curses, NULL, CURSE) != NULL)
		return false;

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other && other->health >= other->max_health)
			return false;
	}
	
	// special rules disable flag carrier abilities
	if(!(ctf->value && ctf_enable_balanced_fc->value && HasFlag(other)))
	{
		if(!other->myskills.abilities[HA_PICKUP].disable)
			temp += 0.3*other->myskills.abilities[HA_PICKUP].current_level;

		//Talent: Basic HA Pickup
		//if(getTalentSlot(other, TALENT_BASIC_HA) != -1)
		//	temp += 0.2 * getTalentLevel(other, TALENT_BASIC_HA);
	}
		
	count = ent->count;
	//K03 Begin
	if (other->health < other->max_health * 2)
		other->health += count * temp;
	else return false;
	//K03 End

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
	if (ent->style & HEALTH_TIMED)
	{
		other->megahealth = ent;	//4.0
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
		{
			if (!invasion->value)
				SetRespawn (ent, 30);
			else
				SetRespawn (ent, 20);
		}
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
	int				armor, current_armor, max_armor, delta;
	gitem_armor_t	*newinfo=(gitem_armor_t *)ent->item->info;
	qboolean		shard=false;
	float			temp = 1.0;

	// cursed players can't pick up armor
	if (que_findtype(other->curses, NULL, CURSE) != NULL)
		return false;

	// how much armor we already have
	current_armor = other->client->pers.inventory[body_armor_index];
	// maximum armor we can hold
	max_armor = MAX_ARMOR(other);

	// handle armor shards specially
	if (ent->item->tag == ARMOR_SHARD)
	{
		armor = 2;
		// we can hold double our max with shards
		max_armor *= 2;
		shard = true;
	}
	else
	{
		// amount of armor to be picked up
		armor = newinfo->base_count;
	}

	//Talent: Improved Supply Station
	//Items dropped with the talent will have a count > 0
	//Add this count to the armor base amount.
	armor += ent->count;

	// special rules disable flag carrier abilities
	if(!(ctf->value && ctf_enable_balanced_fc->value && HasFlag(other)))
	{
		if(!other->myskills.abilities[HA_PICKUP].disable)
			temp += 0.3*other->myskills.abilities[HA_PICKUP].current_level;

		//Talent: Basic HA Pickup
		//if(getTalentSlot(other, TALENT_BASIC_HA) != -1)
		//	temp += 0.2 * getTalentLevel(other, TALENT_BASIC_HA);
	}

	armor *= temp;

	// don't pick up armor if we are already at or beyond our max
	if (current_armor >= max_armor)
	{
		// let them pick up shards for power cubes even when full
		if (shard)
		{
			other->client->pers.inventory[power_cube_index] += 5;
			return true;
		}
		return false;
	}

	// calculate exactly how much armor we need
	delta = max_armor - current_armor;
	// don't add more than we need
	if (armor > delta)
		armor = delta;

	// add the armor
	other->client->pers.inventory[body_armor_index] += armor;

	if (shard)
		other->client->pers.inventory[power_cube_index] += 5;

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
	int		pslevel, index;

	if (ent->myskills.abilities[POWER_SHIELD].disable)
		return;

	//3.0 amnesia disables power screen
	if (que_findtype(ent->curses, NULL, AMNESIA) != NULL)
		return;

	// use power shield level or brain level, whichever is highest
	pslevel = ent->myskills.abilities[POWER_SHIELD].current_level;
	if (ent->mtype == MORPH_BRAIN && ent->myskills.abilities[BRAIN].current_level > pslevel)
		pslevel = ent->myskills.abilities[BRAIN].current_level;

	if (pslevel < 1)
	{
		safe_cprintf (ent, PRINT_HIGH, "You need to upgrade Power Armor!\n");
		return;
	}

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
	//K03 End
}

qboolean Pickup_PowerArmor (edict_t *ent, edict_t *other)
{
	int		quantity;

	//K03 Begin
	if (other->client->pers.inventory[ITEM_INDEX(FindItem("Body Armor"))] >= MAX_ARMOR(other))
		return false;
	//K03 End

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

//K03 Begin
edict_t *SelectFarthestDeathmatchSpawnPoint (edict_t *ent);
void Teleport_them(edict_t *ent)
{
	vec3_t	spawn_origin, spawn_angles, start;
	edict_t	*spot = NULL;

	ent->client->tball_delay = level.time + 4;
	
	//They just got teleported, increment their counter. :)
	if (ent->client)
	{
		ent->myskills.teleports++;
		hook_reset(ent->client->hook);
		V_RestoreMorphed(ent, 50);
	}

	ent->s.event = EV_PLAYER_TELEPORT;
	
	SelectSpawnPoint(ent, spawn_origin, spawn_angles);
	
	VectorCopy(spawn_origin, start);
	start[2] += 9;
	VectorCopy(start, ent->s.origin);
	VectorCopy(spawn_angles, ent->s.angles);
	
	//3.0 You get some invincibility when you spawn, but you can't shoot
	ent->client->respawn_time = ent->client->ability_delay = level.time + (RESPAWN_INVIN_TIME / 10);
	ent->client->invincible_framenum = level.framenum + RESPAWN_INVIN_TIME;

	// 3.68 don't allow morphs to immediately attack
	if (ent->mtype)
		ent->monsterinfo.attack_finished = ent->client->respawn_time;

	KickPlayerBack(ent);//Kicks all campers away!
	KillBox (ent);
}

qboolean CanTball (edict_t *ent, qboolean print)
{
	// must be alive/valid
	if (!G_EntIsAlive(ent))
		return false;

	// must be a player/client
	if (!ent->client)
		return false;

	if (ctf->value)
	{
		if (print)
			safe_cprintf(ent, PRINT_HIGH, "Tball is disabled in CTF mode.\n");
		return false;
	}

	if (level.time < pregame_time->value) 
	{
		if (print)
			safe_cprintf(ent, PRINT_HIGH, "You can't use this ability in pre-game!\n");
		return false;
	}

	if (que_typeexists(ent->curses, 0))
	{
		safe_cprintf(ent, PRINT_HIGH, "You can't use tballs while cursed!\n");
		return false;
	}

	if (level.time < ent->client->tball_delay) 
	{
		if (print)
			safe_cprintf(ent, PRINT_HIGH, "You can't use tballs for another %2.1f seconds\n", 
				(ent->client->tball_delay - level.time));
		return false;
	}

	if (!pvm->value && !invasion->value && (ent->myskills.streak >= SPREE_START))
	{
		if (print)
			safe_cprintf (ent, PRINT_HIGH, "You can't use tballs when you're on a spree\n");
		return false;
	}

	if (HasFlag(ent))
	{
		if (print)
			safe_cprintf(ent, PRINT_HIGH, "You can't use tballs while carrying the flag\n");
		return false;
	}

	// can't tball while taking damage
	if (ent->lasthurt+DAMAGE_ESCAPE_DELAY >= level.time)
		return false;

	// can't tball while summoning/building
	if (ent->holdtime > level.time)
		return false;

	return true;
}

void Tball_Aura(edict_t *owner, vec3_t origin)
{
	edict_t *other=NULL;
	int i=0;
	int radius = 160;

	//3.0 new algorithm for tball code (faster)
	for (i = 1; i <= game.maxclients; i++)
	{
		other = &g_edicts[i];

		if (!CanTball(other, false))
			continue;

		//Make sure they are close enough
		if(distance(origin, other->s.origin) > radius)
			continue;

		//Do the teleporting
		Teleport_them(owner);

		if (other == owner)
		{
			gi.bprintf(PRINT_MEDIUM, "%s teleported away.\n", owner->client->pers.netname);
			VortexRemovePlayerSummonables(other);
		}
		else
		{
			//Give the tball owner some xp
			if (other->client && !G_IsSpectator(other)) // spectators being "teleported away" lawd -az
			{
				owner->client->pers.score += other->myskills.level;
				owner->myskills.experience += other->myskills.level;

				// if the other wasn't teleported away... why put this at all?
				// gi.bprintf(PRINT_MEDIUM, "%s was teleported away by %s.\n", other->client->pers.netname, owner->client->pers.netname);
			}
		}
	}		
}

void Tball_Explode (edict_t *ent)
{
	Tball_Aura(ent->owner, ent->s.origin);
	
	G_FreeEdict (ent);
}

void Tball_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}
	if(!CanTball(other, false))
		return;

	ent->enemy = other;

	//Tball_Explode (ent);
	Teleport_them(other);
}

void fire_tball (edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/items/ammo/grenades/medium/tris.md2");
	grenade->owner = self;
	grenade->touch = Tball_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Tball_Explode;
	grenade->classname = "grenade";

	gi.linkentity (grenade);
}

void Use_Tball_Self(edict_t *ent, gitem_t *item)
{
	if (!CanTball(ent, true))
		return;

	if (ent->client->pers.inventory[ITEM_INDEX(Fdi_TBALL)] < 1)
	{
		if (!(ent->svflags & SVF_MONSTER))
			safe_cprintf (ent, PRINT_HIGH, "Out of item: tball\n");
		return;
	}

	if (ent->holdtime > level.time)
		return; // can't tball while building

	
	ent->client->pers.inventory[ITEM_INDEX(Fdi_TBALL)]--;
	ent->myskills.inventory[ITEM_INDEX(Fdi_TBALL)] = ent->client->pers.inventory[ITEM_INDEX(Fdi_TBALL)];
	
	if (!(ent->svflags & SVF_MONSTER))
		safe_cprintf(ent, PRINT_HIGH, "You have %d tballs left.\n", ent->client->pers.inventory[ITEM_INDEX(Fdi_TBALL)]);

	//ent->client->tball_delay = level.time + 4.0;
	Tball_Aura(ent, ent->s.origin);
}

void Use_Tball(edict_t *ent, gitem_t *item)
{
    vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;

	if (!CanTball(ent, true))
		return;

	if (ent->client->pers.inventory[ITEM_INDEX(Fdi_TBALL)] < 1)
	{
		if (!(ent->svflags & SVF_MONSTER))
			safe_cprintf (ent, PRINT_HIGH, "Out of item: tball\n");
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ent->myskills.inventory[ITEM_INDEX(Fdi_TBALL)] = ent->client->pers.inventory[ITEM_INDEX(Fdi_TBALL)];

	safe_cprintf(ent, PRINT_HIGH, "You have %d tballs left.\n", ent->client->pers.inventory[ITEM_INDEX(Fdi_TBALL)]);

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	ent->client->tball_delay = level.time + 1;
	fire_tball (ent, start, forward, 600, 2.5);
}
//K03 End of tball stuff

/*
===============
Touch_Item
===============
*/
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//GHz START
	// if this is a player-controlled monster, then the player should
	// be able to pick up the items that the monster touches
	int pm = PM_MonsterHasPilot(other);

	if (pm && (other->mtype != BOSS_TANK) && (other->mtype != BOSS_MAKRON))
		other = other->activator;
//GHz END
	if (strcmp(other->classname, "player") && strcmp(other->classname, "dmbot"))
		return;
	if (ent->classname[0] == 'R')
	{
		if(!(other->svflags & SVF_MONSTER))	return;
		if(ent->classname[6] == 'F'&& other->target_ent != NULL)
		{
			if(other->target_ent != ent) return;
//			else if(other->moveinfo.state == SUPPORTER) return;
		}
	}
	if (other->health < 1)
		return;		// dead people can't pickup
	if (!ent->item->pickup)
		return;		// not a grabbable item?
	if (!ent->item->pickup(ent, other))
		return;		// player can't hold it

	// flash the screen
	other->client->bonus_alpha = 0.25;	
	
	// show icon and name on status bar
	other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
	other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);
	other->client->pickup_msg_time = level.time + 3.0;

	// change selected item
	if (ent->item->use)
		other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);

	if(ent->classname[0] != 'R')
	{
		if ((other->client) || !other->client)
		{
			gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF); //ponko
		}
		G_UseTargets (ent, other);
	}
//	else gi.bprintf(PRINT_HIGH,"get %s %x inv %i!\n",ent->classname,ent->spawnflags,other->client->pers.inventory[ITEM_INDEX(ent->item)]);


	//respawn set
	if (ent->flags & FL_RESPAWN)
		ent->flags &= ~FL_RESPAWN;
	else if(ent->classname[6] != 'F') G_FreeEdict (ent);
// GHz START
	// the player may have picked up something useful for the monster
	// make sure the player still has a monster to pilot, since picking up
	// some items (flag) causes the monster to be removed!
	if (pm && PM_PlayerHasMonster(other))
		// sync the player's health with the monster's health
		other->owner->health = other->health;
//GHz END
}

//======================================================================

/*static*/ void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	Touch_Item (ent, other, plane, surf);
}

/*static*/ void drop_make_touchable (edict_t *ent)
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

	if (item->quantity > 0)	// Chamooze
		dropped->count = item->quantity;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 48, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
			dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		//GHz START
		VectorCopy(ent->s.angles, forward);
		forward[YAW] = GetRandom(0, 360);
		//GHz END
		AngleVectors (forward, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}
	//GHz START
	dropped->s.angles[YAW] = GetRandom(0, 360);
	VectorScale (forward, GetRandom(50, 150), dropped->velocity);
	//GHz END
	dropped->velocity[2] = 300;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

void it_think (edict_t *self)
{
	self->touch = Touch_Item;
}

edict_t *Spawn_Item (gitem_t *item)
{
	edict_t	*dropped;

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

	if (item->quantity > 0)	// Chamooze
		dropped->count = item->quantity;

	//GHz START
	dropped->s.angles[YAW] = GetRandom(0, 360);
	//GHz END
	dropped->velocity[2] = 300;

	dropped->think = it_think;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

void Use_Item (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->svflags &= ~SVF_NOCLIENT;
	ent->use = NULL;

	if (ent->spawnflags & 2)	// NO_TOUCH
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
		if (debuginfo->value)
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

void tech_think (edict_t *self)
{
	// let previous holder pick-up this item again
	if (self->owner) 
		self->owner = NULL;

	// remove self after 60 seconds
	if (self->count >= 60)
	{
		if(!FindValidSpawnPoint(self, false))
		{
			BecomeTE(self);
			return;
		}
		self->count = 0; // reset timer
	}

	self->count++;
	self->nextthink = level.time + 1.0;
}

void tech_drop (edict_t *ent, gitem_t *item)
{
	int		index;
	edict_t *tech;

	index = ITEM_INDEX(item);

	// can't drop something we don't have
	if (!ent->client->pers.inventory[index])
		return;

	tech = Drop_Item (ent, item);
	tech->think = tech_think;
	tech->count = 0; // reset timer
	tech->nextthink = level.time + 1.0;

	// clear inventory
	ent->client->pers.inventory[index]--;

	ValidateSelectedItem (ent);
}

qboolean tech_spawn (int index)
{
	edict_t *tech;
	
	if (index == resistance_index)
		tech = Spawn_Item(FindItem("Resistance"));
	else if (index == strength_index)
		tech = Spawn_Item(FindItem("Strength"));
	else if (index == regeneration_index)
		tech = Spawn_Item(FindItem("Regeneration"));
	else if (index == haste_index)
		tech = Spawn_Item(FindItem("Haste"));
	else
		return false;

	tech->think = tech_think;
	tech->nextthink = level.time + FRAMETIME;
	if(!FindValidSpawnPoint(tech, false))
	{
		gi.dprintf("Failed to spawn %s\n", tech->item->pickup_name);
		G_FreeEdict(tech);
		return false;
	}
	gi.dprintf("Spawned %s\n", tech->item->pickup_name);
	return true;
}

void tech_checkrespawn (edict_t *ent)
{
	int index;

	if (!ent || !ent->inuse || !ent->item)
		return;

	// get item index
	index = ITEM_INDEX(ent->item);

	if (!index)
		return;

	// try to respawn tech
	tech_spawn(index);
}

void tech_spawnall (void)
{
	gi.dprintf("Spawning techs...\n");
	tech_spawn(resistance_index);
	tech_spawn(strength_index);
	tech_spawn(regeneration_index);
	tech_spawn(haste_index);
}

qboolean tech_pickup (edict_t *ent, edict_t *other)
{
	int index;
	int	maxLevel = 1.7*AveragePlayerLevel();

	// can't pick-up more than 1 tech
	if (other->client->pers.inventory[resistance_index] 
		|| other->client->pers.inventory[strength_index] 
		|| other->client->pers.inventory[regeneration_index]
		|| other->client->pers.inventory[haste_index]
		|| (other->myskills.streak > 15 && (V_IsPVP() || ffa->value) ) ) // don't allow to pick up techs
		return false;

    // vrc 2.32: allow players to get techs unless there's a player significantly lower
	// than himself instead of limiting techs to level 15.
	
	// can't pick-up if 1.7x higher than average player level
	if (other->myskills.level > maxLevel)
		return false;
	

	index = ITEM_INDEX(ent->item);

	if (index == resistance_index)
		gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/tech1.wav"), 1, ATTN_STATIC, 0);
	else if (index == strength_index)
		gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/tech2.wav"), 1, ATTN_STATIC, 0);
	else if (index == haste_index)
		gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/tech3.wav"), 1, ATTN_STATIC, 0);
	else if (index == regeneration_index)
		gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/tech4.wav"), 1, ATTN_STATIC, 0);

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	return true;
}

void tech_dropall (edict_t *ent)
{
	if (ent->client->pers.inventory[resistance_index])
		tech_drop(ent, FindItem("Resistance"));
	if (ent->client->pers.inventory[strength_index])
		tech_drop(ent, FindItem("Strength"));
	if (ent->client->pers.inventory[regeneration_index])
		tech_drop(ent, FindItem("Regeneration"));
	if (ent->client->pers.inventory[haste_index])
		tech_drop(ent, FindItem("Haste"));
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
void SetBotFlag1(edict_t *ent);	//チーム1の旗
void SetBotFlag2(edict_t *ent);  //チーム2の旗

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
	// Dont spawn quad, invuln, or power armor
	if ((strcmp(ent->classname, "item_power_shield") == 0) 
		|| (strcmp(ent->classname, "item_power_shield") == 0) 
		/*|| (strcmp(ent->classname, "item_quad") == 0)*/
		|| (strcmp(ent->classname, "item_invulnerability") == 0))
	{
		G_FreeEdict (ent);
		return;
	}

	ent->s.renderfx = RF_GLOW|RF_IR_VISIBLE;

	// some items will be prevented in deathmatch
	if (deathmatch->value)
	{
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

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW|RF_IR_VISIBLE; //GHz: Make items visible to lowlight vision
	if (ent->model)
		gi.modelindex (ent->model);

	VectorCopy(ent->s.origin,ent->monsterinfo.last_sighting);
}

qboolean dom_pickupflag (edict_t *ent, edict_t *other);
void dom_dropflag (edict_t *ent, gitem_t *item);
//======================================================================
void Weapon_20mm (edict_t *ent);

gitem_t	itemlist[] = 
{
	{
		NULL
	},	// leave index 0 alone

	//
	// NAVI
	//

/*QUAKED r_navi (.3 .3 1) (-16 -16 -16) (16 16 16)	0
*/

	//
	// ARMOR
	//

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16)	0
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

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16)	1
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

/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)	2
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

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16)	3
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


/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)	4
*/
	{
		"item_power_screen",
		Pickup_Powerup,
		Use_PowerArmor,
		NULL,//Drop_PowerArmor,
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

/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16)	5
*/
	{
		"item_power_shield",
		Pickup_Powerup,
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
/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)	6
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
		IT_WEAPON,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav a_blaster_hud",
		WEAP_BLASTER
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)	7
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
		IT_WEAPON,
		NULL,
		0,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav a_shells_hud",
		WEAP_SHOTGUN
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)	8
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
		IT_WEAPON,
		NULL,
		0,
/* precache */ "weapons/sshotf1b.wav a_shells_hud",
		WEAP_SUPERSHOTGUN
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)	9
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
		IT_WEAPON,
		NULL,
		0,
/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav a_bullets_hud",
		WEAP_MACHINEGUN
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)	10
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
		IT_WEAPON,
		NULL,
		0,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav a_bullets_hud",
		WEAP_CHAINGUN
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)	11
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
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav a_grenades_hud",
		WEAP_GRENADES
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)	12
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
		IT_WEAPON,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2 a_rockets_hud"
	},

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)	13
*/
	{
		"weapon_hyperblaster", 
		Pickup_Weapon,
		// RAFAEL
		Use_Weapon2,
/*		Use_Weapon,*/
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
		IT_WEAPON,
		NULL,
		0,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav a_cells_hud",
		WEAP_HYPERBLASTER
	},
// END 14-APR-98

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)	14
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		// RAFAEL
		Use_Weapon2,
/*		Use_Weapon,*/
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
		IT_WEAPON,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav a_slugs_hud",
		WEAP_RAILGUN
	},

	{
		"weapon_20mm", 
		Pickup_Weapon,
		Use_Weapon2,
		Drop_Weapon,
		Weapon_20mm,
		"misc/w_pkup.wav",
		"models/espada/g_sword.md2", EF_ROTATE,
		"models/espada/v_sword.md2",
		"w_railgun",
		"20mm Cannon",
		0,
		1,
		"Shells",
		IT_WEAPON,
		NULL,
		0,
		"weapons/sgun1.wav a_shells_hud",
		WEAP_20MM
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)	15
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
		20,
		"Cells",
		IT_WEAPON,

		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav a_cells_hud",
		WEAP_BFG
	},

	//K03 Begin
	/* weapon_sword
     always owned, never in the world
     */
     {
		 


     "weapon_sword", 
     NULL,
     Use_Weapon2,                             //How to use
     NULL,
     Weapon_Sword,                           //What the function is
     "misc/w_pkup.wav",
	 "models/espada/g_sword.md2",EF_ROTATE,
     "models/espada/v_sword.md2",      //The models stuff
     "w_blaster",                                    //Icon to be used
     "Sword",                                        //Pickup name
     0,
     0,
     NULL,
     IT_WEAPON,
     NULL,
     0,
      "espada/arm.wav", //The sound of the blaster
	WEAP_SWORD                         //This is precached
     },
	 //K03 End

	//
	// AMMO ITEMS
	//

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)	16
*/
{
		NULL, // ammo to spawn on map
		NULL, // ammo to pickup
		Use_Lasers, // ammo to use
		NULL, // drop ammo
		NULL, // weapon ammo
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		NULL,
/* icon */		"a_grenades",
/* pickup */	"Lasers",
/* width */		3,
		0, // func timer or something
		NULL,
		0,
		NULL,
		0,
/* precache */ ""
	},
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
		0,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)	17
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
		0,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16)	18
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
		0,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_CELLS,
/* precache */ ""
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
		0,
		"grenades",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_GRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav a_grenades_hud"
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
		0,
		NULL,
		IT_AMMO,
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
		0,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SLUGS,
/* precache */ ""
	},
	/*QUAKED RUNE*/
	{
		"item_rune", 
		Pickup_Rune,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/data_cd/tris.md2", EF_ROTATE,
		NULL,
/* icon */		NULL,
/* pickup */	"Rune",
/* width */		2,
		60,
		NULL,
		0,
		NULL,
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
		0,
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
		0,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect3.wav"
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
		0,
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
		0,
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
		0,
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
		45, // az- 45 seconds to respawn.
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
#if 0
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
		0,
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
		0,
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
		0,
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
		0,
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
		0,
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
		0,
		NULL,
		0,
/* precache */ ""
	},

// RAFAEL
/*QUAKED key_green_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - blue
*/
	{
		"key_green_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/green_key/tris.md2", EF_ROTATE,
		NULL,
		"k_green",
		"Green Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
//		0,
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
		0,
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
		0,
		NULL,
		0,
/* precache */ ""
	},
#endif

//K03 Begin
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
		5,//K03
		NULL,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_tball (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"item_tball",
		Pickup_Key,
		Use_Tball,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
		"k_pyramid",
		"tballs",
		2,
		1, //was 5
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
/* precache */ ""
	},
/*QUAKED item_tball (0 .5 .8) (-16 -16 -16) (16 16 16)
*/
	{
		"item_fireball",
		Pickup_Key,
		Use_Tball_Self,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
		"k_pyramid",
		"tball self",
		2,
		1,//was 4
		NULL,
		IT_STAY_COOP|IT_KEY,
		NULL,
		0,
/* precache */ ""
	},
//K03 End
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
		IT_HEALTH,
		NULL,
		0,
/* precache */ ""
	},
	//K03 Begin
	/* Helmet */
	{
		"item_helmet",
		NULL,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"k_pyramid",
/* pickup */	"Helmet",
/* width */		2,
		0,
		NULL,
		0,
		NULL,
		0,
/* precache */ "ctf/tech4.wav"
	},
	/* Stealth Boots */
	{
		"item_stealthboots",
		NULL,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"k_pyramid",
/* pickup */	"Stealth Boots",
/* width */		2,
		0,
		NULL,
		0,
		NULL,
		0,
/* precache */ "ctf/tech4.wav"
	},
	/* Fire Resistant Armor */
	{
		"item_fire_resistant_armor",
		NULL,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"k_pyramid",
/* pickup */	"Fire Resistant Armor",
/* width */		2,
		0,
		NULL,
		0,
		NULL,
		0,
/* precache */ "ctf/tech4.wav"
	},
	{
		"item_redflag",
		CTF_PickupFlag,
		NULL,
		CTF_DropFlag,
		NULL,
		"world/klaxon2.wav",
		"models/items/keys/red_key/tris.md2", EF_FLAG1,
		NULL,
/* icon */		"k_redkey",
/* pickup */	"Red Flag",
/* width */		2,
		0,
		NULL,
		IT_FLAG,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},
	{
		"item_blueflag",
		CTF_PickupFlag,
		NULL,
		CTF_DropFlag,
		NULL,
		NULL,//"world/klaxon2.wav",
		"models/items/keys/key/tris.md2", EF_FLAG2,
		NULL,
/* icon */		"k_bluekey",
/* pickup */	"Blue Flag",
/* width */		2,
		0,
		NULL,
		IT_FLAG,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},
	{
		"item_flag",
		dom_pickupflag,
		NULL,
		dom_dropflag, //Should this be null if we don't want players to drop it manually?
		NULL,
		NULL,//"world/klaxon2.wav",
		"models/items/keys/red_key/tris.md2", EF_FLAG1,
		NULL,
/* icon */		"k_redkey",
/* pickup */	"Flag",
/* width */		2,
		0,
		NULL,
		IT_FLAG,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},
	{
		"item_flaghw",
		hw_pickupflag,
		NULL,
		hw_dropflag,
		NULL,
		NULL,//"world/klaxon2.wav",
		"models/halo/tris.md2", EF_PLASMA,
		NULL,
/* icon */		"k_redkey",
/* pickup */	"Halo",
/* width */		2,
		0,
		NULL,
		IT_FLAG,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},
	{
		"tech_resistance",								// classname
		tech_pickup,									// pick-up function
		NULL,											// use function
		tech_drop,										// drop function
		NULL,
		NULL,											// sound
		"models/ctf/resistance/tris.md2", EF_ROTATE,	// model and effects
		NULL,
		"tech1",										// icon
		"Resistance",									// pickup name
		2,												// width
		0,												// respawn delay
		NULL,
		IT_TECH,
		NULL,
		0,
		"ctf/tech1.wav"									// precache sound
	},

	{
		"tech_strength",								// classname
		tech_pickup,									// pick-up function
		NULL,											// use function
		tech_drop,										// drop function
		NULL,
		NULL,											// sound
		"models/ctf/strength/tris.md2", EF_ROTATE,	// model and effects
		NULL,
		"tech2",										// icon
		"Strength",										// pickup name
		2,												// width
		0,												// respawn delay
		NULL,
		IT_TECH,
		NULL,
		0,
		"ctf/tech2.wav"									// precache sound
	},

	{
		"tech_regeneration",							// classname
		tech_pickup,									// pick-up function
		NULL,											// use function
		tech_drop,										// drop function
		NULL,
		NULL,											// sound
		"models/ctf/regeneration/tris.md2", EF_ROTATE,	// model and effects
		NULL,
		"tech4",										// icon
		"Regeneration",									// pickup name
		2,												// width
		0,												// respawn delay
		NULL,
		IT_TECH,
		NULL,
		0,
		"ctf/tech4.wav"									// precache sound
	},

	{
		"tech_haste",									// classname
		tech_pickup,									// pick-up function
		NULL,											// use function
		tech_drop,										// drop function
		NULL,
		NULL,											// sound
		"models/ctf/haste/tris.md2", EF_ROTATE,			// model and effects
		NULL,
		"tech3",										// icon
		"Haste",										// pickup name
		2,												// width
		0,												// respawn delay
		NULL,
		IT_TECH,
		NULL,
		0,
		"ctf/tech3.wav"									// precache sound
	},

	// end of list marker
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

// RAFAEL
void SP_item_foodcube (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/objects/trapfx/tris.md2";
	SpawnItem (self, FindItem ("Health"));
	self->spawnflags |= DROPPED_ITEM;
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex ("items/s_health.wav");
	self->classname = "foodcube";
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

	for (i=1 ; i<game.num_items ; i++)
	{
		it = &itemlist[i];
		gi.configstring (CS_ITEMS+i, it->pickup_name);
	}

	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
	body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
	power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
	power_shield_index = ITEM_INDEX(FindItem("Power Shield"));
	power_cube_index   = ITEM_INDEX(FindItem("Power Cube"));
	flag_index         = ITEM_INDEX(FindItem("Flag"));
	red_flag_index     = ITEM_INDEX(FindItem("Red Flag"));
	blue_flag_index    = ITEM_INDEX(FindItem("Blue Flag"));
	halo_index		   = ITEM_INDEX(FindItem("Halo"));
	resistance_index   = ITEM_INDEX(FindItem("Resistance"));
	strength_index     = ITEM_INDEX(FindItem("Strength"));
	regeneration_index = ITEM_INDEX(FindItem("Regeneration"));
	haste_index        = ITEM_INDEX(FindItem("Haste"));
	
	//Ammo
	bullet_index		= ITEM_INDEX(FindItem("Bullets"));
	shell_index			= ITEM_INDEX(FindItem("Shells"));
	grenade_index		= ITEM_INDEX(FindItem("Grenades"));
	rocket_index		= ITEM_INDEX(FindItem("Rockets"));
	slug_index			= ITEM_INDEX(FindItem("Slugs"));
	cell_index			= ITEM_INDEX(FindItem("Cells"));
}

int GetWorldAmmoCount (char *pickupName)
{
	int		count=0;
	edict_t *e=NULL;
	gitem_t *it=FindItem(pickupName);

	while((e = G_Find(e, FOFS(classname), it->classname)) != NULL)
	{
		if (e->inuse && !(e->spawnflags & DROPPED_ITEM))
			count++;
	}
	return count;
}

void SpawnWorldAmmoType (char *pickupName, int count)
{
	int		i;
	edict_t	*e;

	for (i=0; i<count; i++)
	{
		e = Spawn_Item(FindItem(pickupName));
		if (!FindValidSpawnPoint(e, false))
			G_FreeEdict(e);
		e->spawnflags &= ~DROPPED_ITEM;
	}
}

//
//#define WORLD_MINIMUM_SHELLS		1
//#define WORLD_MINIMUM_BULLETS		1
//#define WORLD_MINIMUM_GRENADES		1
//#define WORLD_MINIMUM_ROCKETS		1
//#define WORLD_MINIMUM_SLUGS			1
//#define WORLD_MINIMUM_CELLS			1

void SpawnWorldAmmo (void)
{
	int count, need;

	if (trading->value)
		return;

	if ((count = GetWorldAmmoCount("Shells")) < world_min_shells->value)
	{
		need = world_min_shells->value-count;
		SpawnWorldAmmoType("Shells", need);
		gi.dprintf("World spawned %d shell packs\n", need);
	}
	if ((count = GetWorldAmmoCount("Bullets")) < world_min_bullets->value)
	{
		need = world_min_bullets->value-count;
		SpawnWorldAmmoType("Bullets", need);
		gi.dprintf("World spawned %d bullet packs\n", need);
	}
	if ((count = GetWorldAmmoCount("grenades")) < world_min_grenades->value)
	{
		need = world_min_grenades->value-count;
		SpawnWorldAmmoType("Grenades", need);
		gi.dprintf("World spawned %d grenade packs\n", need);
	}
	if ((count = GetWorldAmmoCount("Rockets")) < world_min_rockets->value)
	{
		need = world_min_rockets->value-count;
		SpawnWorldAmmoType("Rockets", need);
		gi.dprintf("World spawned %d rocket packs\n", need);
	}
	if ((count = GetWorldAmmoCount("Slugs")) < world_min_slugs->value)
	{
		need = world_min_slugs->value-count;
		SpawnWorldAmmoType("Slugs", need);
		gi.dprintf("World spawned %d slug packs\n", need);
	}
	if ((count = GetWorldAmmoCount("Cells")) < world_min_cells->value)
	{
		need = world_min_cells->value-count;
		SpawnWorldAmmoType("Cells", need);
		gi.dprintf("World spawned %d cell packs\n", need);
	}
}

