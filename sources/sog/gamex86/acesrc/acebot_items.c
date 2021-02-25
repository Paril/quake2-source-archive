///////////////////////////////////////////////////////////////////////
//
//  ACE - Quake II Bot Base Code
//
//  Version 1.0
//
//  This file is Copyright(c), Steve Yeager 1998, All Rights Reserved
//
//
//	All other files are Copyright(c) Id Software, Inc.
//
//	Please see liscense.txt in the source directory for the copyright
//	information regarding those files belonging to Id Software, Inc.
//	
//	Should you decide to release a modified version of ACE, you MUST
//	include the following text (minus the BEGIN and END lines) in the 
//	documentation for your modification.
//
//	--- BEGIN ---
//
//	The ACE Bot is a product of Steve Yeager, and is available from
//	the ACE Bot homepage, at http://www.axionfx.com/ace.
//
//	This program is a modification of the ACE Bot, and is therefore
//	in NO WAY supported by Steve Yeager.

//	This program MUST NOT be sold in ANY form. If you have paid for 
//	this product, you should contact Steve Yeager immediately, via
//	the ACE Bot homepage.
//
//	--- END ---
//
//	I, Steve Yeager, hold no responsibility for any harm caused by the
//	use of this source code, especially to small children and animals.
//  It is provided as-is with no implied warranty or support.
//
//  I also wish to thank and acknowledge the great work of others
//  that has helped me to develop this code.
//
//  John Cricket    - For ideas and swapping code.
//  Ryan Feltrin    - For ideas and swapping code.
//  SABIN           - For showing how to do true client based movement.
//  BotEpidemic     - For keeping us up to date.
//  Telefragged.com - For giving ACE a home.
//  Microsoft       - For giving us such a wonderful crash free OS.
//  id              - Need I say more.
//  
//  And to all the other testers, pathers, and players and people
//  who I can't remember who the heck they were, but helped out.
//
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//
//  acebot_items.c - This file contains all of the 
//                   item handling routines for the 
//                   ACE bot, including fact table support
//           
///////////////////////////////////////////////////////////////////////

#include "..\g_local.h"
#include "..\g_sogutil.h"
#include "acebot.h"

int	num_players = 0;
int num_items = 0;
item_table_t item_table[MAX_EDICTS];
edict_t *players[MAX_CLIENTS];		// pointers to all players in the game

///////////////////////////////////////////////////////////////////////
// Add the player to our list
///////////////////////////////////////////////////////////////////////
void ACEIT_PlayerAdded(edict_t *ent)
{
	players[num_players++] = ent;
}

///////////////////////////////////////////////////////////////////////
// Remove player from list
///////////////////////////////////////////////////////////////////////
void ACEIT_PlayerRemoved(edict_t *ent)
{
	int i;
	int pos;

	// watch for 0 players
	if(num_players == 0)
		return;

	// special cas for only one player
	if(num_players == 1)
	{	
		num_players = 0;
		return;
	}

	// Find the player
	for(i=0;i<num_players;i++)
		if(ent == players[i])
			pos = i;

	// decrement
	for(i=pos;i<num_players-1;i++)
		players[i] = players[i+1];

	num_players--;
}

///////////////////////////////////////////////////////////////////////
// Can we get there?
///////////////////////////////////////////////////////////////////////
qboolean ACEIT_IsReachable(edict_t *self, vec3_t goal)
{
	trace_t trace;
	vec3_t v;

	VectorCopy(self->mins,v);
	v[2] += 18; // Stepsize

	trace = gi.trace (self->s.origin, v, self->maxs, goal, self, MASK_OPAQUE);
	
	// Yes we can see it
	if (trace.fraction == 1.0)
		return true;
	else
		return false;

}

///////////////////////////////////////////////////////////////////////
// Visiblilty check 
///////////////////////////////////////////////////////////////////////
qboolean ACEIT_IsVisible(edict_t *self, vec3_t goal)
{
	trace_t trace;
	
	trace = gi.trace (self->s.origin, vec3_origin, vec3_origin, goal, self, MASK_OPAQUE);
	
	// Yes we can see it
	if (trace.fraction == 1.0)
		return true;
	else
		return false;

}

///////////////////////////////////////////////////////////////////////
//  Weapon changing support
///////////////////////////////////////////////////////////////////////
qboolean ACEIT_ChangeWeapon (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;
		
	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return true; 

	// Has not picked up weapon yet
	if(!ent->client->pers.inventory[ITEM_INDEX(item)])
		return false;

	// Do we have ammo for it?
	if (item->ammo)
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (!ent->client->pers.inventory[ammo_index] && !g_select_empty->value)
			return false;
	}

	// Change to this weapon
	ent->client->newweapon = item;
	
	return true;
}


extern gitem_armor_t jacketarmor_info;
extern gitem_armor_t combatarmor_info;
extern gitem_armor_t bodyarmor_info;
extern gitem_armor_t greenarmor_info;
extern gitem_armor_t yellowarmor_info;
extern gitem_armor_t redarmor_info;
extern gitem_armor_t dgreenarmor_info;
extern gitem_armor_t dbluearmor_info;
extern gitem_armor_t wgreenvest_info;
extern gitem_armor_t wbluevest_info;

///////////////////////////////////////////////////////////////////////
// Check if we can use the armor
///////////////////////////////////////////////////////////////////////
qboolean ACEIT_CanUseArmor (gitem_t *item, edict_t *other)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	float			salvage;
	int				salvagecount;

	// get info on new armor
	newinfo = (gitem_armor_t *)item->info;

	old_armor_index = ArmorIndex (other);

	// handle armor shards specially
	if (item->tag == ARMOR_SHARD)
		return true;
	

	// get info on old armor
	switch(other->client->resp.player_class)
	{
	case CLASS_WOLF:
	{
		if(old_armor_index == ITEM_INDEX(FindItem("Green Vest")))
			oldinfo = &wgreenvest_info;
		else if(old_armor_index == ITEM_INDEX(FindItem("Blue Vest")))
			oldinfo = &wbluevest_info;
	}
	case CLASS_DOOM:
	{
		if(old_armor_index == ITEM_INDEX(FindItem("Green Armor")))
			oldinfo = &dgreenarmor_info;
		else if(old_armor_index == ITEM_INDEX(FindItem("Blue Armor")))
			oldinfo = &dbluearmor_info;
	}
	case CLASS_Q1:
	{
			if(old_armor_index == ITEM_INDEX(FindItem("Armor")))
				oldinfo = &greenarmor_info;
			else if(old_armor_index == ITEM_INDEX(FindItem("Yellow Armor")))
				oldinfo = &yellowarmor_info;
			else if(old_armor_index == ITEM_INDEX(FindItem("Red Armor")))
				oldinfo = &redarmor_info;
	}
	case CLASS_Q2:
	{
		if (old_armor_index == ITEM_INDEX(FindItem("Jacket Armor")))
			oldinfo = &jacketarmor_info;
		else if (old_armor_index == ITEM_INDEX(FindItem("Combat Armor")))
			oldinfo = &combatarmor_info;
		else // (old_armor_index == body_armor_index)
			oldinfo = &bodyarmor_info;
	}
	}
	
	
	if (newinfo->normal_protection <= oldinfo->normal_protection)
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

	}

	return true;
}


///////////////////////////////////////////////////////////////////////
// Determins the NEED for an item
//
// This function can be modified to support new items to pick up
// Any other logic that needs to be added for custom decision making
// can be added here. For now it is very simple.
///////////////////////////////////////////////////////////////////////
float ACEIT_ItemNeed(edict_t *self, int item)
{
	
	// Make sure item is at least close to being valid
	if(item < 0 || item > 138)
		return 0.0;

	switch(item)
	{
		// Health
		case ITEMLIST_HEALTH_SMALL:	
		case ITEMLIST_HEALTH_MEDIUM:
		case ITEMLIST_HEALTH_LARGE:	
		case ITEMLIST_HEALTH_MEGA:	
			if(self->health < 100)
				return 1.0 - (float)self->health/100.0f; // worse off, higher priority
			else
				return 0.0;

		case Q2_QUAD:
		case Q2_GOD:
		case Q2_SILENCER:
		case Q2_REBREATHER:			
		case Q2_ENVIROSUIT:
		case Q2_ADERNALINE:
		case Q2_BANDOLIER:		
		case Q2_BACKPACK:
		case Q1_QUAD:
		case Q1_RING:
		case Q1_PENT:
		case Q1_SUIT:
		case Q1_PACK:
		case D_BERSERK:
		case D_INVIS:
		case D_GOD:
		case D_SOULSPHERE:
		case D_RADSUIT:
		// case D_BACKPACK:
		case W_GOD:
			return 0.6;
		
		// Sgt. Payne Weapons
		case Q2_SG:
		case Q2_SSG:
		case Q2_MG:
		case Q2_CG:
		case Q2_GL:
		case Q2_RL:
		case Q2_HB:
		case Q2_BFG:
		if(!self->client->pers.inventory[
			  GiveNewWeapon(item,CLASS_Q2,self->client->resp.player_class)])
				return 0.7;
			else
				return 0.0;
				
		// Axe Weapons
		case Q1_SSG:
		case Q1_NG:
		case Q1_SNG:
		case Q1_GL:
		case Q1_RL:
		case Q1_LG:
			if(!self->client->pers.inventory[
			  GiveNewWeapon(item,CLASS_Q1,self->client->resp.player_class)])
				return 0.7;
			else
				return 0.0;	
		
		// Flynn weapons
		case D_CSAW:
		case D_SG:
		case D_SSG:
		case D_CG:
		case D_RL:
		case D_PG:
		case D_BFG:
			if(!self->client->pers.inventory[
			  GiveNewWeapon(item,CLASS_DOOM,self->client->resp.player_class)])
				return 0.7;
			else
				return 0.0;		
		
		// Blaze weapons
		case W_MG:
		case W_GG:
		case W_RL:
		case W_FT:
		case W_GG2:	
			if(!self->client->pers.inventory[
			  GiveNewWeapon(item,CLASS_WOLF,self->client->resp.player_class)])
				return 0.7;
			else
				return 0.0;
		
		// Ammo
		case AMMO_SLUGS_INDEX:			
			if(self->client->pers.inventory[AMMO_SLUGS_INDEX] < self->client->pers.max_slugs)
				return 0.4;  
			else
				return 0.0;
	
		case AMMO_BULLETS_INDEX:
			if(self->client->pers.inventory[AMMO_BULLETS_INDEX] < self->client->pers.max_bullets)
				return 0.3;  
			else
				return 0.0;
	
		case AMMO_SHELLS_INDEX:
		   if(self->client->pers.inventory[AMMO_SHELLS_INDEX] < self->client->pers.max_shells)
				return 0.3;  
			else
				return 0.0;
	
		case AMMO_CELLS_INDEX:
			if(self->client->pers.inventory[AMMO_CELLS_INDEX] <	self->client->pers.max_cells)
				return 0.3;  
			else
				return 0.0;
	
		case AMMO_ROCKETS_INDEX:
			if(self->client->pers.inventory[AMMO_ROCKETS_INDEX] < self->client->pers.max_rockets)
				return 1.5;  
			else
				return 0.0;
	
		case AMMO_GRENADES_INDEX:
			if(self->client->pers.inventory[AMMO_GRENADES_INDEX] < self->client->pers.max_grenades)
				return 0.3;  
			else
				return 0.0;
	
		// Axe ammo
		
		case INDEX_Q1_NL:
		case INDEX_Q1_NS:
		case INDEX_Q1_SL:
		case INDEX_Q1_SS:
		case INDEX_Q1_CL:
		case INDEX_Q1_CS:
		case INDEX_Q1_RL:
		case INDEX_Q1_RS:
			if(self->client->pers.inventory[
				GiveNewAmmo(item,CLASS_Q1,CLASS_Q2)		
			] < 50)
				return 0.4;  
			else
				return 0.0;		

		// Flynn Ammo
		case INDEX_D_BL:
		case INDEX_D_BS:
		case INDEX_D_SL:
		case INDEX_D_SS:
		case INDEX_D_CL:
		case INDEX_D_CS:
		case INDEX_D_RL:
		case INDEX_D_RS:
			if(self->client->pers.inventory[
				GiveNewAmmo(item,CLASS_DOOM,CLASS_Q2)		
			] < 50)
				return 0.4;  
			else
				return 0.0;	
				
		// Blaze Ammo
		case INDEX_W_BL:
		case INDEX_W_BM:
		case INDEX_W_BS:
		case INDEX_W_RL:
		case INDEX_W_RS:
		case INDEX_W_PL:
		case INDEX_W_PS:
			if(self->client->pers.inventory[
				GiveNewAmmo(item,CLASS_WOLF,CLASS_Q2)		
			] < 50)
				return 0.4;  
			else
				return 0.0;	

		case Q2_BODY:
			if(ACEIT_CanUseArmor (FindItem("Body Armor"), self))
				return 0.6;  
			else
				return 0.0;
	
		case Q2_COMBAT:
			if(ACEIT_CanUseArmor (FindItem("Combat Armor"), self))
				return 0.6;  
			else
				return 0.0;
	
		case Q2_JACKET:
			if(ACEIT_CanUseArmor (FindItem("Jacket Armor"), self))
				return 0.6;  
			else
				return 0.0;
	
		case Q2_POWERSCREEN:
		case Q2_POWERSHIELD:
			return 0.5;  

		case ITEMLIST_FLAG1:
			// If I am on team one, I want team two's flag
			if(!self->client->pers.inventory[item] && self->client->resp.team == CTF_TEAM2)
				return 10.0;  
			else 
				return 0.0;

		case ITEMLIST_FLAG2:
			if(!self->client->pers.inventory[item] && self->client->resp.team == CTF_TEAM1)
				return 10.0;  
			else
				return 0.0;
		
		case Q2CTF_TECH1:
		case Q2CTF_TECH2:
		case Q2CTF_TECH3:			
		case Q2CTF_TECH4:
			// Check for other tech
			if(!self->client->pers.inventory[Q2CTF_TECH1] &&
			   !self->client->pers.inventory[Q2CTF_TECH2] &&
			   !self->client->pers.inventory[Q2CTF_TECH3] &&
			   !self->client->pers.inventory[Q2CTF_TECH4])
			    return 0.4;  
			else
				return 0.0;
				
		default:
			return 0.0;
			
	}
		
}

///////////////////////////////////////////////////////////////////////
// Convert a classname to its index value
//
// I prefer to use integers/defines for simplicity sake. This routine
// can lead to some slowdowns I guess, but makes the rest of the code
// easier to deal with.
///////////////////////////////////////////////////////////////////////
int ACEIT_ClassnameToIndex(char *classname)
{
	if(strcmp(classname,"item_armor_body")==0) 
		return Q2_BODY;
	
	if(strcmp(classname,"item_armor_combat")==0)
		return Q2_COMBAT;

	if(strcmp(classname,"item_armor_jacket")==0)
		return Q2_JACKET;
	
	if(strcmp(classname,"item_armor_shard")==0)
		return Q2_SHARD;

	if(strcmp(classname,"item_power_screen")==0)
		return Q2_POWERSCREEN;

	if(strcmp(classname,"item_power_shield")==0)
		return Q2_POWERSHIELD;

	if(strcmp(classname,"item_idg3_armorR")==0)
		return Q1_RED;
	
	if(strcmp(classname,"item_idg3_armorY")==0)
		return Q1_YELLOW;
	
	if(strcmp(classname,"item_idg3_armorG")==0)
		return Q1_GREEN;
		
	if(strcmp(classname,"item_idg2_armorB")==0)
		return D_BLUE;
	
	if(strcmp(classname,"item_idg2_armorG")==0)
		return D_GREEN;

	if(strcmp(classname,"item_idg2_armorS")==0)
		return D_HELMET;
		
	if(strcmp(classname,"item_idg1_armorB")==0)
		return W_BLUE;
		
	if(strcmp(classname,"item_idg1_armorG")==0)
		return W_GREEN;
		
	if(strcmp(classname,"item_idg1_cross")==0)
		return W_CROSS;
		
	if(strcmp(classname,"item_idg1_chalice")==0)
		return W_CHALICE;
		
	if(strcmp(classname,"item_idg1_chest")==0)
		return W_CHEST;
		
	if(strcmp(classname,"item_idg1_crown")==0)
		return W_CROWN;
	if(strcmp(classname,"weapon_grapple")==0)
		return Q2_HOOK;

	if(strcmp(classname,"weapon_blaster")==0)
		return Q2_BLASTER;

	if(strcmp(classname,"weapon_shotgun")==0)
		return Q2_SG;
	
	if(strcmp(classname,"weapon_supershotgun")==0)
		return Q2_SSG;
	
	if(strcmp(classname,"weapon_machinegun")==0)
		return Q2_MG;
	
	if(strcmp(classname,"weapon_chaingun")==0)
		return Q2_CG;
	
	if(strcmp(classname,"ammo_grenades")==0)
		return Q2_HG;

	if(strcmp(classname,"weapon_grenadelauncher")==0)
		return Q2_GL;

	if(strcmp(classname,"weapon_rocketlauncher")==0)
		return Q2_RL;

	if(strcmp(classname,"weapon_hyperblaster")==0)
		return Q2_HB;

	if(strcmp(classname,"weapon_railgun")==0)
		return Q2_RL;

	if(strcmp(classname,"weapon_bfg10k")==0)
		return Q2_BFG;

	if(strcmp(classname,"weapon_idg3_dbshotg")==0)
		return Q1_SSG;
		
	if(strcmp(classname,"weapon_idg3_nailg")==0)
		return Q1_NG;
		
	if(strcmp(classname,"weapon_idg3_hnailg")==0)
		return Q1_SNG;
		
	if(strcmp(classname,"weapon_idg3_grenl")==0)
		return Q1_GL;

	if(strcmp(classname,"weapon_idg3_rocketl")==0)
		return Q1_RL;

	if(strcmp(classname,"weapon_idg3_discharger")==0)
		return Q1_LG;

	if(strcmp(classname,"weapon_idg2_saw")==0)
		return D_CSAW;

	if(strcmp(classname,"weapon_idg2_shotg")==0)
		return D_SG;
		
	if(strcmp(classname,"weapon_idg2_dbshotg")==0)
		return D_SSG;
		
	if(strcmp(classname,"weapon_idg2_chaing")==0)
		return D_CG;
		
	if(strcmp(classname,"weapon_idg2_rocketl")==0)
		return D_RL;
	
	if(strcmp(classname,"weapon_idg2_plasmag")==0)
		return D_PG;
		
	if(strcmp(classname,"weapon_idg2_gdbg")==0)
		return D_BFG;
		
	if(strcmp(classname,"weapon_idg1_machineg")==0)
		return W_MG;
		
	if(strcmp(classname,"weapon_idg1_gatlingg")==0)
		return W_GG;
		
	if(strcmp(classname,"weapon_idg1_bazooka")==0)
		return W_RL;
		
	if(strcmp(classname,"weapon_idg1_flamet")==0)
		return W_FT;
	
	if(strcmp(classname,"ammo_shells")==0)
		return AMMO_SHELLS_INDEX;
	
	if(strcmp(classname,"ammo_bullets")==0)
		return AMMO_BULLETS_INDEX;

	if(strcmp(classname,"ammo_cells")==0)
		return AMMO_CELLS_INDEX;

	if(strcmp(classname,"ammo_rockets")==0)
		return AMMO_ROCKETS_INDEX;

	if(strcmp(classname,"ammo_slugs")==0)
		return AMMO_SLUGS_INDEX;
	
	if(strcmp(classname,"weapon_idg3_shellsL")==0)
		return INDEX_Q1_SL;

	if(strcmp(classname,"weapon_idg3_shellsS")==0)
		return INDEX_Q1_SS;

	if(strcmp(classname,"weapon_idg3_nailsL")==0)
		return INDEX_Q1_NL;
		
	if(strcmp(classname,"weapon_idg3_nailsS")==0)
		return INDEX_Q1_NS;
		
	if(strcmp(classname,"weapon_idg3_cellsL")==0)
		return INDEX_Q1_CL;
		
	if(strcmp(classname,"weapon_idg3_cellsS")==0)
		return INDEX_Q1_CS;

	if(strcmp(classname,"weapon_idg3_rocketsL")==0)
		return INDEX_Q1_RL;
		
	if(strcmp(classname,"weapon_idg3_rocketsS")==0)
		return INDEX_Q1_RS;

	if(strcmp(classname,"weapon_idg2_shellsL")==0)
		return INDEX_D_SL;

	if(strcmp(classname,"weapon_idg2_shellsS")==0)
		return INDEX_D_SS;
		
	if(strcmp(classname,"weapon_idg2_bulletsL")==0)
		return INDEX_D_BL;
		
	if(strcmp(classname,"weapon_idg2_bulletsS")==0)
		return INDEX_D_BS;
		
	if(strcmp(classname,"weapon_idg2_plasmaL")==0)
		return INDEX_D_CL;
		
	if(strcmp(classname,"weapon_idg2_plasmaS")==0)
		return INDEX_D_CS;
		
	if(strcmp(classname,"weapon_idg2_rocketsL")==0)
		return INDEX_D_RL;
		
	if(strcmp(classname,"weapon_idg2_rocketsS")==0)
		return INDEX_D_RS;
		
	if(strcmp(classname,"weapon_idg1_bulletsL")==0)
		return INDEX_W_BL;
		
	if(strcmp(classname,"weapon_idg1_bulletsM")==0)
		return INDEX_W_BM;
		
	if(strcmp(classname,"weapon_idg1_bulletsS")==0)
		return INDEX_W_BS;		

	if(strcmp(classname,"weapon_idg1_rocketL")==0)
		return INDEX_W_RL;
		
	if(strcmp(classname,"weapon_idg1_rocketS")==0)
		return INDEX_W_RS;
		
	if(strcmp(classname,"weapon_idg1_petrolL")==0)
		return INDEX_W_PL;
		
	if(strcmp(classname,"weapon_idg1_petrolS")==0)
		return INDEX_W_PS;

	if(strcmp(classname,"item_quad")==0)
		return Q2_QUAD;

	if(strcmp(classname,"item_invulnerability")==0)
		return Q2_GOD;

	if(strcmp(classname,"item_silencer")==0)
		return Q2_SILENCER;

	if(strcmp(classname,"item_breather")==0)
		return Q2_REBREATHER;

	if(strcmp(classname,"item_enviro")==0)
		return Q2_ENVIROSUIT;

	if(strcmp(classname,"item_ancient_head")==0)
		return Q2_ANCIENTHEAD;

	if(strcmp(classname,"item_adrenaline")==0)
		return Q2_ADERNALINE;

	if(strcmp(classname,"item_bandolier")==0)
		return Q2_BANDOLIER;

	if(strcmp(classname,"item_pack")==0)
		return Q2_BACKPACK;

	if(strcmp(classname,"item_idg3_quad")==0)
		return Q1_QUAD;
		
	if(strcmp(classname,"item_idg3_invis")==0)
		return Q1_RING;
		
	if(strcmp(classname,"item_idg3_god")==0)
		return Q1_PENT;
		
	if(strcmp(classname,"item_idg3_envsuit")==0)
		return Q1_SUIT;
		
	if(strcmp(classname,"item_idg3_backpack")==0)
		return Q1_PACK;
		
	if(strcmp(classname,"item_idg2_quad")==0)
		return D_BERSERK;
		
	if(strcmp(classname,"item_idg2_invis")==0)
		return D_INVIS;
		
	if(strcmp(classname,"item_idg2_god")==0)
		return D_GOD;
		
	if(strcmp(classname,"item_idg2_envsuit")==0)
		return D_RADSUIT;
		
	if(strcmp(classname,"item_idg1_god")==0)
		return W_GOD;

	if(strcmp(classname,"item_datacd")==0)
		return ITEMLIST_DATACD;

	if(strcmp(classname,"item_powercube")==0)
		return ITEMLIST_POWERCUBE;

	if(strcmp(classname,"item_pyramidkey")==0)
		return ITEMLIST_PYRAMIDKEY;

	if(strcmp(classname,"item_dataspinner")==0)
		return ITEMLIST_DATASPINNER;

	if(strcmp(classname,"item_securitypass")==0)
		return ITEMLIST_SECURITYPASS;

	if(strcmp(classname,"item_bluekey")==0)
		return ITEMLIST_BLUEKEY;

	if(strcmp(classname,"item_redkey")==0)
		return ITEMLIST_REDKEY;

	if(strcmp(classname,"item_commandershead")==0)
		return ITEMLIST_COMMANDERSHEAD;

	if(strcmp(classname,"item_airstrikemarker")==0)
		return ITEMLIST_AIRSTRIKEMARKER;

	if(strcmp(classname,"item_health")==0) // ??
		return ITEMLIST_HEALTH;

	if(strcmp(classname,"item_flag_team1")==0)
		return ITEMLIST_FLAG1;

	if(strcmp(classname,"item_flag_team2")==0)
		return ITEMLIST_FLAG2;

	if(strcmp(classname,"item_tech1")==0)
		return Q2CTF_TECH1;

	if(strcmp(classname,"item_tech2")==0)
		return Q2CTF_TECH2;

	if(strcmp(classname,"item_tech3")==0)
		return Q2CTF_TECH3;

	if(strcmp(classname,"item_tech4")==0)
		return Q2CTF_TECH4;

	if(strcmp(classname,"item_health_small")==0)
		return ITEMLIST_HEALTH_SMALL;

	if(strcmp(classname,"item_idg3_healthS")==0)
		return ITEMLIST_HEALTH_SMALL;
		
	if(strcmp(classname,"item_idg2_healthS")==0)
		return ITEMLIST_HEALTH_SMALL;
		
	if(strcmp(classname,"item_idg1_healthS")==0)
		return ITEMLIST_HEALTH_SMALL;

	if(strcmp(classname,"item_health_medium")==0)
		return ITEMLIST_HEALTH_MEDIUM;

	if(strcmp(classname,"item_idg2_healthM")==0)
		return ITEMLIST_HEALTH_MEDIUM;
	
	if(strcmp(classname,"item_idg1_healthM")==0)
		return ITEMLIST_HEALTH_MEDIUM;
		
	if(strcmp(classname,"item_health_large")==0)
		return ITEMLIST_HEALTH_LARGE;
	
	if(strcmp(classname,"item_idg3_healthL")==0)
		return ITEMLIST_HEALTH_LARGE;

	if(strcmp(classname,"item_idg2_healthL")==0)
		return ITEMLIST_HEALTH_LARGE;
		
	if(strcmp(classname,"item_idg1_healthL")==0)
		return ITEMLIST_HEALTH_LARGE;

	if(strcmp(classname,"item_health_mega")==0)
		return ITEMLIST_HEALTH_MEGA;

	if(strcmp(classname,"item_idg3_megahealth")==0)
		return ITEMLIST_HEALTH_MEGA;

	if(strcmp(classname,"item_idg2_megahealth")==0)
		return ITEMLIST_HEALTH_MEGA;
		
	if(strcmp(classname,"item_idg1_megahealth")==0)
		return ITEMLIST_HEALTH_MEGA;

	return INVALID;
}


///////////////////////////////////////////////////////////////////////
// Only called once per level, when saved will not be called again
//
// Downside of the routine is that items can not move about. If the level
// has been saved before and reloaded, it could cause a problem if there
// are items that spawn at random locations.
//
//#define DEBUG // uncomment to write out items to a file.
///////////////////////////////////////////////////////////////////////
void ACEIT_BuildItemNodeTable (qboolean rebuild)
{
	edict_t *items;
	int i,item_index;
	vec3_t v,v1,v2;

#ifdef DEBUG
	FILE *pOut; // for testing
	if((pOut = fopen("items.txt","wt"))==NULL)
		return;
#endif
	
	num_items = 0;

	// Add game items
	for(items = g_edicts; items < &g_edicts[globals.num_edicts]; items++)
	{
		// filter out crap
		if(items->solid == SOLID_NOT)
			continue;
		
		if(!items->classname)
			continue;
		
		/////////////////////////////////////////////////////////////////
		// Items
		/////////////////////////////////////////////////////////////////
		item_index = ACEIT_ClassnameToIndex(items->classname);
		
		////////////////////////////////////////////////////////////////
		// SPECIAL NAV NODE DROPPING CODE
		////////////////////////////////////////////////////////////////
		// Special node dropping for platforms
		if(strcmp(items->classname,"func_plat")==0)
		{
			if(!rebuild)
				ACEND_AddNode(items,NODE_PLATFORM);
			item_index = 139; // to allow to pass the item index test
		}
		
		// Special node dropping for teleporters
		if(strcmp(items->classname,"misc_teleporter_dest")==0 || strcmp(items->classname,"misc_teleporter")==0)
		{
			if(!rebuild)
				ACEND_AddNode(items,NODE_TELEPORTER);
			item_index = 139;
		}
		
		#ifdef DEBUG
		if(item_index == INVALID)
			fprintf(pOut,"Rejected item: %s node: %d pos: %f %f %f\n",items->classname,item_table[num_items].node,items->s.origin[0],items->s.origin[1],items->s.origin[2]);
		else
			fprintf(pOut,"item: %s node: %d pos: %f %f %f\n",items->classname,item_table[num_items].node,items->s.origin[0],items->s.origin[1],items->s.origin[2]);
		#endif		
	
		if(item_index == INVALID)
			continue;

		// add a pointer to the item entity
		item_table[num_items].ent = items;
		item_table[num_items].item = item_index;
	
		// If new, add nodes for items
		if(!rebuild)
		{
			// Add a new node at the item's location.
			item_table[num_items].node = ACEND_AddNode(items,NODE_ITEM);
			num_items++;
		}
		else // Now if rebuilding, just relink ent structures 
		{
			// Find stored location
			for(i=0;i<numnodes;i++)
			{
				if(nodes[i].type == NODE_ITEM ||
				   nodes[i].type == NODE_PLATFORM ||
				   nodes[i].type == NODE_TELEPORTER) // valid types
				{
					VectorCopy(items->s.origin,v);
					
					// Add 16 to item type nodes
					if(nodes[i].type == NODE_ITEM)
						v[2] += 16;
					
					// Add 32 to teleporter
					if(nodes[i].type == NODE_TELEPORTER)
						v[2] += 32;
					
					if(nodes[i].type == NODE_PLATFORM)
					{
						VectorCopy(items->maxs,v1);
						VectorCopy(items->mins,v2);
		
						// To get the center
						v[0] = (v1[0] - v2[0]) / 2 + v2[0];
						v[1] = (v1[1] - v2[1]) / 2 + v2[1];
						v[2] = items->mins[2]+64;
					}

					if(v[0] == nodes[i].origin[0] &&
 					   v[1] == nodes[i].origin[1] &&
					   v[2] == nodes[i].origin[2])
					{
						// found a match now link to facts
						item_table[num_items].node = i;
			
#ifdef DEBUG
						fprintf(pOut,"Relink item: %s node: %d pos: %f %f %f\n",items->classname,item_table[num_items].node,items->s.origin[0],items->s.origin[1],items->s.origin[2]);
#endif							
						num_items++;
					}
				}
			}
		}
		

	}

#ifdef DEBUG
	fclose(pOut);
#endif

}
