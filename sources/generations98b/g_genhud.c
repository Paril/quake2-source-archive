#include "g_local.h"
#include "g_genutil.h"

//doom/wolfie status bar

char *wolfdm =
"yb	-24 "

//armor
"if 4 "
"	xv	2 "
"	rnum "
"	xv	52 "
"	pic 4 "
"endif "

// health
//"yb	-24 "
"xv	98 "
"hnum "
"xv	148 "
"yb	-32 "
"pic 0 "
"yb	-24 "

// ammo
"if 2 "
"	xv	200 "
"	anum "
"	xv	250 "
"	pic 2 "
"endif "

// selected item
"if 6 "
"	yb	-32 "			//-----------------
"	xv	288 " 
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

/*
// picked up item2
"if 30 "
"	xv	0 "
"	pic 30 "
"	
"endif "
*/

// timer
"if 9 "
"	xv	246 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"   yb  -70 "
"	pic	11 "
"   yb  -50 "
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 14"

// id view state
/*"if 27 "
  "xv 0 "
  "yb -58 "
  "string \"Viewing\" "
  "xv 64 "
  "stat_string 27 "
"endif "
*/
;


char *wolfsp =
"yb	-24 "

//armor
"if 4 "
"	xv	0 "
"	rnum "
"	xv	50 "
"	pic 4 "
"endif "

// health
//"yb	-24 "
"xv	98 "
"hnum "
"xv	148 "
"yb	-32 "
"pic 0 "
"yb	-24 "

// ammo
"if 2 "
"	xv	200 "
"	anum "
"	xv	250 "
"	pic 2 "
"endif "

// selected item
"if 6 "
"	yb	-32 "			//-----------------
"	xv	288 " 
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
"	xv	246 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"   yb  -64 "
"	pic	11 "
"   yb  -50 "
"endif "
;


//======================================================================
// Common function used by the HUD functions
//======================================================================

static int HealthLevel(int health)
{
	if(health > 80)
		return 1;
	else if(health > 60)
		return 2;
	else if(health > 40)
		return 3;
	else if(health > 20)
		return 4;
	else 
		return 5;
}

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
	
	index=HealthLevel(ent->health);
//
// Health	
//
	if(ent->flags & FL_INVIS) 
	{
		if(ent->client->invincible_framenum > level.framenum)
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex("q1hud/q1_pentinvf");
		else
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex("q1hud/q1_invf");
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		if (ent->client->quad_framenum > level.framenum)
			 ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex("q1hud/q1_pentquadf");
		else
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex("q1hud/q1_pentf");
	}
	else if (ent->client->quad_framenum > level.framenum)
	{
		 ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex("q1hud/q1_quadf");
	}
	else if((ent->pain_debounce_time > level.time ||
			 ent->client->next_drown_time > level.time)
			 && ent->health)
	{
		ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("q1hud/q1_hf%i",index));
	}
	else
	{
		ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("q1hud/q1_face%i",index));
	}
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
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("q1hud/items/q1_shell");
				break;
			case AMMO_BULLETS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("q1hud/items/q1_nails");
				break;
			case AMMO_CELLS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("q1hud/items/q1_cells");
				break;
			case AMMO_ROCKETS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("q1hud/items/q1_rocks");
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
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("q1hud/items/SbQuad");
		else if ((ent->client->invincible_framenum > level.framenum) ||
				(ent->flags & FL_GODMODE))
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("q1hud/items/Sb666");
		else if (ent->flags & FL_INVIS) 
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("q1hud/items/SbRing");
		else if (ent->client->enviro_framenum > level.framenum ||
			     ent->client->breather_framenum > level.framenum)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("q1hud/items/SbSuit");
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

//======================================================================
//======================================================================
// Doomguy looks where he's been shot at
//======================================================================
//======================================================================

void DoomFaceTheMusic(gclient_t *client, float side, int health)
{
	int index = HealthLevel(health);

	if(!health)
		return;
		
	if(side < -0.35)
		client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("doomhud/d_Lface%i",index));
	else if(side > 0.35)
		client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("doomhud/d_Rface%i",index));
	else
		client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("doomhud/d_Ouch%i",index));
	client->v_dmg_time = level.time + 1.5;
}


//======================================================
// DOOM STATUS BAR
//======================================================

void G_DoomSetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index=0;

	index=HealthLevel(ent->health);
//	
// HEALTH
//
	if(ent->health < 0)
		ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex("doomhud/d_dead");
	else if ((ent->client->invincible_framenum > level.framenum) ||
			 (ent->flags & FL_GODMODE))
	{
		ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex("doomhud/d_God");
	}
	else if (ent->client->lastfiretime > level.time || ent->client->weaponstate == WEAPON_FIRING) 
		ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("doomhud/d_Fire%i",index));
	else 
	{
		if ((level.framenum & 32) || (level.framenum & 8))
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("doomhud/d_face%i",index));
		else if (level.framenum & 16)
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("doomhud/d_face%iL",index));
		else
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("doomhud/d_face%iR",index));
	}
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
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("doomhud/items/d_shells");	
				ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[AMMO_SHELLS_INDEX];
				break;
			case AMMO_BULLETS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("doomhud/items/d_bullet");
				ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[AMMO_BULLETS_INDEX];
				break;
			case AMMO_CELLS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("doomhud/items/d_cells");
				ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[AMMO_CELLS_INDEX];
				break;
			case AMMO_ROCKETS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("doomhud/items/d_rocks");
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
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("doomhud/items/dInvis");
		else if (ent->client->invincible_framenum > level.framenum)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("doomhud/items/dGod");
		else if ((ent->client->enviro_framenum > level.framenum) ||
				 (ent->client->breather_framenum > level.framenum))
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("doomhud/items/dSuit");
		else if (ent->flags & FL_D_BERSERK)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("doomhud/items/d_ader");
		else
			ent->client->ps.stats[STAT_SELECTED_ICON]=0;
		//else if (ent->client->pers.weapon)
		//	ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (ent->client->pers.weapon->icon);
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

static int WHealthLevel(int health)
{
	if(health > 80)
		return 1;
	else if(health > 60)
		return 2;
	else if(health > 30)
		return 3;
	else 
		return 4;
}

void WolfieFaceTheMusic(gclient_t *client, float side, int health)
{
	int index = WHealthLevel(health);

	if(!health)
		return;
	
	if(side < -0.35)
		client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("wolfhud/wLouch%i",index));
	else if(side > 0.35)
		client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("wolfhud/wRouch%i",index));
	else
		client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("wolfhud/wOuch%i",index));
	client->v_dmg_time = level.time + 1.5;
}

void G_WolfSetStats(edict_t *ent)
{
	gitem_t	*item;
	int		index=0;
	
	index= WHealthLevel(ent->health);
//
// Health	
//
	if(ent->health < 0)
	{
		ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex("wolfhud/wdead");
	}
	else if ((ent->client->invincible_framenum > level.framenum) ||
			(ent->flags & FL_GODMODE))
	{
		ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex("wolfhud/wgod");
	}
	//else if (ent->client->lastfiretime > level.time) 
	else if (ent->client->lastfiretime > level.time || ent->client->weaponstate == WEAPON_FIRING) 
	{
		ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("wolfhud/wfire%i",index));
	}
	else //if((ent->client->v_dmg_time < level.time) && ent->health)
	{
		if ((level.framenum & 32) || (level.framenum & 8))
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("wolfhud/wface%i",index));
		else if (level.framenum & 16)
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("wolfhud/wface%il",index));
		else
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("wolfhud/wface%ir",index));
	}
	//else
	//	ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex("wolfhud/wdead");
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
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("wolfhud/items/wAmmo");
				break;
			case AMMO_CELLS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("wolfhud/items/wPetrol");
				break;
			case AMMO_ROCKETS_INDEX:
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex ("wolfhud/items/wRocks");
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
//FIXME
		if (ent->client->quad_framenum > level.framenum)
			//ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("q1hud/items/SbQuad");
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("wolfhud/items/wPowerup");
		else if (ent->client->invincible_framenum > level.framenum)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("wolfhud/items/wPowerup");
		else if (ent->client->invis_framenum > level.framenum)
			//ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("q1hud/items/SbRing");
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("wolfhud/items/wPowerup");
		else if (ent->client->enviro_framenum > level.framenum ||
			     ent->client->breather_framenum > level.framenum)
			ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("doomhud/items/dSuit");
		//else if (ent->client->pers.weapon)
		//	ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (ent->client->pers.weapon->icon);
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