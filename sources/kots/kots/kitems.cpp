//*************************************************************************************
//*************************************************************************************
// File: kitems.cpp
//*************************************************************************************
//*************************************************************************************

//#include "stdafx.h"
#include "kots.h"

extern "C"
{
#include "g_local.h"
#include "exports.h"
#include "super.h"
}
#include "user.h"
#include "shared.h"
#include "kotscpp.h"

//*************************************************************************************
//*************************************************************************************
// Function: kotsdrop_make_touchable
//*************************************************************************************
//*************************************************************************************

static void kotsdrop_make_touchable( edict_t *ent )
{
	ent->touch = Touch_Item;

	ent->nextthink = level.time + 15;
	ent->think     = G_FreeEdict;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSPlayerDie
//*************************************************************************************
//*************************************************************************************

void KOTSPlayerDie( CUser *user )
{
	CUser   *pack;
	edict_t *ent = user->m_ent;
	edict_t *dropped;
	gitem_t *item;

	if ( !ent )
		return;

	item = FindItem( "KOTS Pack" );

	if ( !item )
		return;

	ent->client->v_angle[YAW] += 22.5;
	dropped = Drop_Item( ent, item );
	ent->client->v_angle[YAW] -= 22.5;

	dropped->think = kotsdrop_make_touchable;
	dropped->owner = 0;

	pack = new CUser;

	theApp.m_packs.Add( pack );

	dropped->kotsdata = pack;

	// This is actually very important here, because it saves the inventory of the user
	user->Uninit( true );

	memcpy( pack, user, sizeof( CUser) );

	pack->m_ent   = 0;

	if ( kots_lives->value && ent->kots_lives < 1 )
	{
		KOTSLeave( user->m_ent );

		return;
	}

	if ( ent->kotssave < KOTS_CHECKIN_PLAYER )
		return;

	ent->kotssave = 0;

	user->GameSave( theApp.GetDataDir() );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTS_SetItem
//*************************************************************************************
//*************************************************************************************

int KOTS_SetItem( char *sItem, edict_t *other, int quantity, int max )
{
	int     index;
	gitem_t	*item;
 
 	item = FindItem( sItem );
 
 	if ( item )
	{
		index = ITEM_INDEX( item );

		if ( max > 0 && quantity > 0 )
			other->client->pers.inventory[index] += quantity;
		
		if ( other->client->pers.inventory[index] > max )
			other->client->pers.inventory[index] = max;
	
		return other->client->pers.inventory[index];
	}
	return 0;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSPickup_KOTSPack
//*************************************************************************************
//*************************************************************************************

int KOTSPickup_KOTSPack( edict_t *ent, edict_t *other )
{
	int   x;
	CUser *pack;
	CUser *user;

	pack = (CUser *)ent->kotsdata;

	user = KOTSGetUser( other );

	if ( !pack || !user )
		return true;

	for ( x = 0; x < theApp.m_packs.GetSize(); x++ )
	{
		if ( pack == theApp.m_packs[x] )
		{
			theApp.m_packs.RemoveAt( x );
			break;
		}
	}
	KOTS_SetItem( "Bullets" , other, pack->m_bullets , other->client->pers.max_bullets  );
	KOTS_SetItem( "Shells"  , other, pack->m_shells  , other->client->pers.max_shells   );
	KOTS_SetItem( "Cells"   , other, pack->m_cells   , other->client->pers.max_cells    );
	KOTS_SetItem( "Grenades", other, pack->m_grenades, other->client->pers.max_grenades );
	KOTS_SetItem( "Rockets" , other, pack->m_rockets , other->client->pers.max_rockets  );
	KOTS_SetItem( "Slugs"   , other, pack->m_slugs   , other->client->pers.max_slugs    );

	KOTS_SetItem( "Blaster"         , other, pack->m_blaster     , 1 );      
	KOTS_SetItem( "Shotgun"         , other, pack->m_shotgun     , 1 );      
	KOTS_SetItem( "Super Shotgun"   , other, pack->m_sshotgun    , 1 );     
	KOTS_SetItem( "Machinegun"      , other, pack->m_mgun        , 1 );         
	KOTS_SetItem( "Chaingun"        , other, pack->m_cgun        , 1 );         
	KOTS_SetItem( "Grenade Launcher", other, pack->m_glauncher   , 1 );     
	KOTS_SetItem( "Rocket Launcher" , other, pack->m_rlauncher   , 1 );     
	KOTS_SetItem( "HyperBlaster"    , other, pack->m_hyperblaster, 1 );  
	KOTS_SetItem( "Railgun"         , other, pack->m_rgun        , 1 );          
	KOTS_SetItem( "BFG10K"          , other, pack->m_bfg         , 1 );           

	delete pack;											

	return true;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSPickup_Pack
//*************************************************************************************
//*************************************************************************************

void KOTSPickup_Pack( edict_t *ent, int )
{
	int		  index;
	gitem_t	*item;

	item = FindItem( "tball" );

	if ( item )
	{
		index = ITEM_INDEX( item );

		ent->client->pers.inventory[index] += 4;
	}
	item = FindItem( "Power Cube" );

	if ( item )
	{
		index = ITEM_INDEX( item );

		ent->client->pers.inventory[index] += 20;
	}
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSPickup_Armor
//*************************************************************************************
//*************************************************************************************

int KOTSPickup_Armor( edict_t *ent, edict_t *other )
{
	int x;
	int	index;
	int value;
	int	old_armor_index;
	int	body_armor_index;

	CUser   *user = KOTSGetUser( other );
	gitem_t	*item;

	if ( !user )
		return false;

	gitem_armor_t	*newinfo;

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;

	old_armor_index = ArmorIndex (other);

	body_armor_index = ITEM_INDEX( FindItem( "Body Armor" ) );

	// handle armor shards specially
	if ( ent->item->tag == ARMOR_SHARD )
	{
		value = 2 * KOTSModKarma( other );

		if ( !old_armor_index )
			other->client->pers.inventory[body_armor_index] = value;
		else
			other->client->pers.inventory[body_armor_index] += value;

		item = FindItem( "Power Cube" );

		if ( item )
		{
			index = ITEM_INDEX( item );

			other->client->pers.inventory[index] += 5;
		}
	}
	// if player has no armor, just use it
	else if (!old_armor_index)
	{
		other->client->pers.inventory[body_armor_index] = newinfo->base_count;
	}
	else
	{
		x = other->client->pers.inventory[body_armor_index];

		if ( x >= user->GetMaxArmor() )
			return false;

		x += newinfo->base_count;

		if ( x > user->GetMaxArmor() )
			x = user->GetMaxArmor();

		other->client->pers.inventory[body_armor_index] = x;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 20);

	return true;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSPickup_PowerArmor
//*************************************************************************************
//*************************************************************************************

int KOTSPickup_PowerArmor( edict_t *ent, edict_t *other )
{
	int     max;
	int     index;
	CUser   *user = KOTSGetUser( other );
	gitem_t *item;

	if ( user )
	{
		max = user->GetMaxArmor();

		item = FindItem( "Body Armor" );

		if ( item )
		{
			index = ITEM_INDEX( item );

			if ( other->client->pers.inventory[index] < max )
			{
				other->client->pers.inventory[index] = max;

				if ( !( ent->spawnflags & DROPPED_ITEM ) )
					SetRespawn( ent, 180 );

				return true;
			}
		}
	}
	return false;
}

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************

