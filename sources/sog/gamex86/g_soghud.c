#include "g_local.h"
#include "g_sogutil.h"


//======================================================================
//======================================================================
// Skid
// Quake / Doom HUDS
//======================================================================

// Quake 

void G_Q1SetStats (edict_t *ent)
{
	gitem_t	*item;
	int		index;
	
//
// Health	
//

	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	
	index = 0;
//
// AMMO
//
	if (!ent->client->ammo_index)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		//icons for different ammo types -Skid
		switch(ent->client->ammo_index)
		{
			case AMMO_SHELLS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("idg3/a_shells");
				break;
			case AMMO_BULLETS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("idg3/a_nails");
				break;
			case AMMO_CELLS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("idg3/a_cells");
				break;
			case AMMO_ROCKETS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("idg3/a_rockets");
				break;
			default:
				ent->client->ps.stats[STAT_AMMO_ICON] = 0; 
				break;
		}
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}
//
// Armor
//
	index = ArmorIndex (ent);
	if (index)
	{
		item = GetItemByIndex (index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
		
		if(ent->client->invincible_framenum > level.framenum)
			ent->client->ps.stats[STAT_ARMOR] = 666;
		else
			ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}
//
// pickup message
//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}
//
// timers, didnt have any in Quake
//
	ent->client->ps.stats[STAT_TIMER_ICON] = 0;
	ent->client->ps.stats[STAT_TIMER] = 0;
	
//
// selected item, or weapon if none active
//
	if(!ent->deadflag)
	{
		if (ent->client->quad_framenum > level.framenum)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("idg3/p_quad");
		else if ((ent->client->invincible_framenum > level.framenum) ||
				(ent->flags & FL_GODMODE))
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("idg3/p_god");
		else if (ent->flags & FL_INVIS) 
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("idg3/p_invis");
		else if (ent->client->enviro_framenum > level.framenum ||
			     ent->client->breather_framenum > level.framenum)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("idg3/p_suit");
		else if (ent->client->pers.weapon)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (ent->client->pers.weapon->icon);
	}
	else
		ent->client->ps.stats[STAT_SELECTED_ICON]=0;
//
// layouts
//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
			if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores )
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;

		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
//
// frags
//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

//
// help icon / current weapon if not shown
//
	if (ent->client->pers.helpchanged && (level.framenum & 8))
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if (((ent->client->pers.hand == CENTER_HANDED) || 
			  (ent->client->ps.fov > 91) || 
			  (ent->client->chasetoggle))
				&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;
}


//======================================================
// DOOM STATUS BAR
//======================================================

void G_DoomSetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index=0;

//	
// HEALTH
//

	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;


//	
// AMMO
//
	if (!ent->client->ammo_index)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		switch(ent->client->ammo_index)
		{
			case AMMO_SHELLS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("idg2/a_shells");	
				ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[AMMO_SHELLS_INDEX];
				break;
			case AMMO_BULLETS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("idg2/a_bullets");
				ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[AMMO_BULLETS_INDEX];
				break;
			case AMMO_CELLS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("idg2/a_cells");
				ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[AMMO_CELLS_INDEX];
				break;
			case AMMO_ROCKETS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("idg2/a_rockets");
				ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[AMMO_ROCKETS_INDEX];
				break;
			default:
				ent->client->ps.stats[STAT_AMMO_ICON] = 0; 
				ent->client->ps.stats[STAT_AMMO] = 0;
				break;
		}
		
	}
//	
// ARMOR
//	
	index = ArmorIndex (ent);
	if (index)
	{
		item = GetItemByIndex (index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}
//
// pickup message
//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}
//
// timers
//
	ent->client->ps.stats[STAT_TIMER_ICON] = 0;
	ent->client->ps.stats[STAT_TIMER] = 0;
//
// SELECTED ICON FOR CURRENT POWERUP OR WEAPON	
// selected item, or weapon if none active
//
	if(!ent->deadflag)
	{
		if (ent->flags & FL_INVIS)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("idg2/p_invis");
		else if (ent->client->invincible_framenum > level.framenum)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("idg2/p_god");
		else if ((ent->client->enviro_framenum > level.framenum) ||
				 (ent->client->breather_framenum > level.framenum))
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("idg2/p_suit");
		else if (ent->flags & FL_D_BERSERK)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("idg2/p_berserk");
		else if (ent->client->pers.weapon)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (ent->client->pers.weapon->icon);
		else
			ent->client->ps.stats[STAT_SELECTED_ICON]=0;

	}
	else
		ent->client->ps.stats[STAT_SELECTED_ICON]=0;
//
// layouts
//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
			if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores )
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;

		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
//
// frags
//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;
//
// help icon / current weapon if not shown
//
	if (ent->client->pers.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if (((ent->client->pers.hand == CENTER_HANDED) || 
			  (ent->client->ps.fov > 91) || 
			  (ent->client->chasetoggle)) &&
			  (ent->client->pers.weapon))
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;
}

//======================================================================
// WOLFY STATUS BAR
//======================================================================


void G_WolfSetStats(edict_t *ent)
{
	gitem_t	*item;
	int		index=0;
	
//
// Health	
//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;
	
	index = 0;
//
// AMMO
//
	if (!ent->client->ammo_index)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
//icons for different ammo types -Skid
		switch(ent->client->ammo_index)
		{
			case AMMO_SHELLS_INDEX:
			case AMMO_BULLETS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("idg1/a_bullets");
				break;
			case AMMO_CELLS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("idg1/a_petrol");
				break;
			case AMMO_ROCKETS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("idg1/a_rockets");
				break;
			default:
				ent->client->ps.stats[STAT_AMMO_ICON] = 0; 
				break;
		}
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}
//
// Armor
//
	index = ArmorIndex (ent);
	if (index)
	{
		item = GetItemByIndex (index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}
//
// pickup message
//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}
//
// timers, didnt have any in Quake
//
	ent->client->ps.stats[STAT_TIMER_ICON] = 0;
	ent->client->ps.stats[STAT_TIMER] = 0;
	
//
// selected item, or weapon if none active
//
	if(!ent->deadflag)
	{
		if (ent->client->quad_framenum > level.framenum)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("idg1/p_mega");
		else if (ent->client->invincible_framenum > level.framenum)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("idg1/p_god");
		else if (ent->client->invis_framenum > level.framenum)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("idg1/p_mega");
		else if (ent->client->enviro_framenum > level.framenum ||
			     ent->client->breather_framenum > level.framenum)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("idg2/p_suit");
		else if (ent->client->pers.weapon)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (ent->client->pers.weapon->icon);
		else 
			ent->client->ps.stats[STAT_SELECTED_ICON]=0;
	}
	else
		ent->client->ps.stats[STAT_SELECTED_ICON]=0;
//
// layouts
//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
			if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores )
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;

		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
//
// frags
//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;
//
// help icon / current weapon if not shown
//
	if (ent->client->pers.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ((ent->client->pers.hand == CENTER_HANDED || (ent->client->ps.fov > 91) || (ent->client->chasetoggle))
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;
}