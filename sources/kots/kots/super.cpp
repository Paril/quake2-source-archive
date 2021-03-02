//*************************************************************************************
//*************************************************************************************
// File: super.cpp
//*************************************************************************************
//*************************************************************************************

//#include "stdafx.h"
#include "kots.h"

extern "C"
{
#include "g_local.h"
#include "super.h"
#include "superdef.h"
#include "exports.h"

void Cmd_WeapNext_f( edict_t *ent );
}
#include "user.h"
#include "kotscpp.h"

//*************************************************************************************
//*************************************************************************************
// Function: KOTSStopCloak
//*************************************************************************************
//*************************************************************************************

void KOTSStopCloak( edict_t *ent )
{
	if ( !ent->client->pers.kots_cloaking || ent->deadflag )
		return;

	ent->svflags &= ~SVF_NOCLIENT;

	ent->client->pers.kots_cloaking = false;

	gi.cprintf( ent, PRINT_HIGH, "Cloaking Disabled\n" );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCloak
//*************************************************************************************
//*************************************************************************************

int KOTSCloak( edict_t *ent, usercmd_t *ucmd )
{
	if ( !ent->client->pers.kots_cloaking || ent->deadflag )
		return 0;

	if ( ucmd->buttons & BUTTON_ATTACK )
	{
		KOTSStopCloak( ent );

		return 0;
	}
	
	if ( ent->svflags & SVF_NOCLIENT )
	{
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Power Cube"))] >= KOTS_WISDOM_CLOAK_AMMO )
		{
			ent->client->pers.kots_cloakdrain++;

			if ( ent->client->pers.kots_cloakdrain == KOTS_WISDOM_CLOAK_DRAIN )
			{
				ent->client->pers.inventory[ITEM_INDEX(FindItem("Power Cube"))] -= KOTS_WISDOM_CLOAK_AMMO;
				ent->client->pers.kots_cloakdrain = 0;
			}
		}
		else
		{
			gi.cprintf( ent, PRINT_HIGH, "Cloaking Disabled\n" );
	
			ent->svflags   &= ~SVF_NOCLIENT;
			ent->client->pers.kots_cloaking = false;
		}
	}
	return 1;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSFly
//*************************************************************************************
//*************************************************************************************

int KOTSFly( edict_t *ent )
{
	if ( !ent->client->pers.kots_flying || ent->deadflag )
		return false;

	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Power Cube"))] >= KOTS_WISDOM_FLY_AMMO )
	{
		ent->client->pers.kots_flydrain++;

		if ( ent->client->pers.kots_flydrain == KOTS_WISDOM_FLY_DRAIN )
		{
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Power Cube"))] -= KOTS_WISDOM_FLY_AMMO;
			ent->client->pers.kots_flydrain = 0;

			gi.sound (ent, CHAN_BODY, gi.soundindex("weapons/rockfly.wav"), 1, ATTN_NORM, 0);
		}
	}
	else
	{
		gi.cprintf( ent, PRINT_HIGH, "Low Gravity Disabled\n" );

		ent->client->pers.kots_flying = false;

		return false;
	}

	ent->velocity[2] = 200;

//	if ( ent->velocity[2] < -500 )
//		ent->velocity[2] += ((ent->velocity[2])/(-5));
//	else if (ent->velocity[2] < 0)
//		ent->velocity[2] += 100; 
//	else
//		ent->velocity[2] += ((1000-ent->velocity[2])/8);

	return true;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSResist
//*************************************************************************************
//*************************************************************************************

int	KOTSResist( edict_t *ent, int take )
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return take;

	if ( user->Level() >= KOTS_WISDOM_RESIST )
		take *= KOTS_WISDOM_RESIST_BASE;

	return take;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSStrength
//*************************************************************************************
//*************************************************************************************
//
//int KOTSStrength( edict_t *ent, int take )
//{
//	CUser *user = KOTSGetUser( ent );
//
////	if ( !user || ent->deadflag )
////		return take;
//
////	if ( user->Level() >= USER_STRENGTH )
////		take *= 1.5;
//
//	return take;
//}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSilentJump
//*************************************************************************************
//*************************************************************************************

int KOTSSilentJump( edict_t *ent )
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return false;

	return ( user->Level() >= KOTS_DEXTERITY_SJUMP	);
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSilentWalk
//*************************************************************************************
//*************************************************************************************

int KOTSSilentWalk( edict_t *ent )
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return false;

	return ( user->Level() >= KOTS_DEXTERITY_SWALK );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSilentPickup
//*************************************************************************************
//*************************************************************************************

int KOTSSilentPickup( edict_t *ent )
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return false;

	return ( user->Level() >= KOTS_DEXTERITY_SPICKUP	);
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSHighJump
//*************************************************************************************
//*************************************************************************************

int KOTSHighJump( edict_t *ent )
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return false;

	return ( user->Level() >= KOTS_STRENGTH_HJUMP );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSArmorProtection
//*************************************************************************************
//*************************************************************************************

int KOTSArmorProtection( edict_t *ent, int *damage )
{
	int   index;
	int   armor;
	int   count;
	CUser *user;

	user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return 0;

	index = ArmorIndex( ent );

	if ( !index )
		return 0;

	if ( user->Level() >= KOTS_TECHNO_3XARMOR )
		armor = KOTS_TECHNO_ARMOR_3X;  
	else if ( user->Level() >= KOTS_TECHNO_2XARMOR )
		armor = KOTS_TECHNO_ARMOR_2X;	 
	else
		armor = KOTS_TECHNO_ARMOR_BASE;

	count = *damage / armor;

	if ( count >= ent->client->pers.inventory[index] )
	{
		count   = ent->client->pers.inventory[index];
		*damage = count * armor;
	}
	return count;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSwitch
//*************************************************************************************
//*************************************************************************************

int KOTSSwitch( edict_t *ent )
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return false;

	return ( user->Level() >= KOTS_DEXTERITY_SWITCHING );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSVampire
//*************************************************************************************
//*************************************************************************************

void KOTSVampire( edict_t *ent, edict_t *targ, int take )
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag || targ->health < 1 )
		return;

	if ( user->Level() < KOTS_WISDOM_VAMPIRE )
		return;

	if ( ent->health >= ent->max_health )
		return;

	if ( take > targ->health )
		take = targ->health;

	ent->health += ( take * KOTS_WISDOM_VAMPIRE_BASE );

	if ( ent->health > ent->max_health )
		ent->health = ent->max_health;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSRegen
//*************************************************************************************
//*************************************************************************************

void KOTSRegen( edict_t *ent )
{
	int   x;
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return;

	if ( user->Level() >= KOTS_KARMA_2XREGEN )
		x = 4;
	else if ( user->Level() >= KOTS_KARMA_REGEN )
		x = 2;
	else
		return;

	if ( ent->client->pers.kots_regentime >= level.time ) 
		return;

	ent->client->pers.kots_regentime = level.time;

	if ( ent->health < ent->max_health ) 
	{
		ent->health += x;
	
		if ( ent->health > ent->max_health )
			ent->health = ent->max_health;

		ent->client->pers.kots_regentime += 1;
	}
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSKnock
//*************************************************************************************
//*************************************************************************************

void KOTSKnock( edict_t *ent, edict_t *other, int take )
{
	int     index;
	CUser   *user = KOTSGetUser( ent );
	gitem_t *item;

	if ( take < 200 )
		return;

	if ( !ent || !other || !user )
		return;

	if ( ent == other )
		return;

	if ( ent->deadflag || other->deadflag )
		return;

	if ( user->Level() < KOTS_STRENGTH_KNOCK )
		return;

	if ( !ent->client || !other->client )
		return;

	if ( take < ( other->health * .5 ) )
		return;

	item = other->client->pers.weapon;

	if ( !item )
		return;

	index = ITEM_INDEX( item );

	if ( index == ITEM_INDEX( FindItem( "blaster" ) ) )
		return;

	Cmd_WeapNext_f( other );

	Drop_Item( other, item );

	other->client->pers.inventory[ index ] = 0;

	return;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSDamage
//*************************************************************************************
//*************************************************************************************

int KOTSDamage( edict_t *ent, int mod )
{
	int   damage = 0;
	float x      = 1.0;
	CUser *user  = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return 0;

	switch ( mod )
	{
		case MOD_BLASTER:
			damage = KOTS_SWORD_BASE_DAMAGE;

			if ( user->Level() >= KOTS_SWORD_4X_DAMAGE )
				x = 2.5;
			else if ( user->Level() >= KOTS_SWORD_3X_DAMAGE )
				x = 2.0;
			else if ( user->Level() >= KOTS_SWORD_2X_DAMAGE )
				x = 1.5;
			break;

		case MOD_SHOTGUN:			
			damage = KOTS_SHOTGUN_BASE_DAMAGE;

			if ( user->Level() >= KOTS_SHOTGUN_5X_DAMAGE )
				x = 3.5;
			else if ( user->Level() >= KOTS_SHOTGUN_4X_DAMAGE )
				x = 3.0;
			else if ( user->Level() >= KOTS_SHOTGUN_3X_DAMAGE )
				x = 2.5;
			else if ( user->Level() >= KOTS_SHOTGUN_2X_DAMAGE )
				x = 2.0;
			break;

		case MOD_SSHOTGUN:		
			damage = KOTS_SSHOTGUN_BASE_DAMAGE;

			if ( user->Level() >= KOTS_SSHOTGUN_4X_DAMAGE )
				x = 2.5;
			else if ( user->Level() >= KOTS_SSHOTGUN_3X_DAMAGE )
				x = 2.0;
			else if ( user->Level() >= KOTS_SSHOTGUN_2X_DAMAGE )
				x = 1.5;
			break;

		case MOD_MACHINEGUN:	
			damage = KOTS_MGUN_BASE_DAMAGE;

			if ( user->Level() >= KOTS_MGUN_5X_DAMAGE )
				x = 3.5;
			else if ( user->Level() >= KOTS_MGUN_4X_DAMAGE )
				x = 3.0;
			else if ( user->Level() >= KOTS_MGUN_3X_DAMAGE )
				x = 2.5;
			else if ( user->Level() >= KOTS_MGUN_2X_DAMAGE )
				x = 2.0;
			break;

		case MOD_CHAINGUN:		
			damage = KOTS_CGUN_BASE_DAMAGE;

			if ( user->Level() >= KOTS_CGUN_4X_DAMAGE )
				x = 2.5;
			else if ( user->Level() >= KOTS_CGUN_3X_DAMAGE )
				x = 2.0;
			else if ( user->Level() >= KOTS_CGUN_2X_DAMAGE )
				x = 1.5;
			break;

		case MOD_GRENADE:			
			damage = KOTS_GLAUNCHER_BASE_DAMAGE;

			if ( user->Level() >= KOTS_GLAUNCHER_3X_DAMAGE )
				x = 2.5;
			else if ( user->Level() >= KOTS_GLAUNCHER_2X_DAMAGE )
				x = 2.0;
			else if ( user->Level() >= KOTS_GLAUNCHER_15X_DAMAGE )
				x = 1.5;
			break;

		case MOD_ROCKET:			
			damage = KOTS_RLAUNCHER_BASE_DAMAGE;

			if ( user->Level() >= KOTS_RLAUNCHER_3X_DAMAGE )
				x = 2.5;
			else if ( user->Level() >= KOTS_RLAUNCHER_2X_DAMAGE )
				x = 2.0;
			break;

		case MOD_HYPERBLASTER:
			damage = KOTS_HYPER_BASE_DAMAGE;

			if ( user->Level() >= KOTS_HYPER_4X_DAMAGE )
				x = 3.5;
			else if ( user->Level() >= KOTS_HYPER_3X_DAMAGE )
				x = 3.0;
			else if ( user->Level() >= KOTS_HYPER_2X_DAMAGE )
				x = 2.0;
			else if ( user->Level() >= KOTS_HYPER_15X_DAMAGE )
				x = 1.5;
			break;
	
		case MOD_RAILGUN:			
			damage = KOTS_RAILGUN_BASE_DAMAGE;

			if ( user->Level() >= KOTS_RAILGUN_4X_DAMAGE )
				x = 3.0;
			else if ( user->Level() >= KOTS_RAILGUN_3X_DAMAGE )
				x = 2.5;
			else if ( user->Level() >= KOTS_RAILGUN_2X_DAMAGE )
				x = 2.0;
			else if ( user->Level() >= KOTS_RAILGUN_15X_DAMAGE )
				x = 1.5;
			break;

		case MOD_BFG_LASER:		
			damage = KOTS_BFG_BASE_DAMAGE;

			if ( user->Level() >= KOTS_BFG_4X_DAMAGE )
				x = 2.5;
			else if ( user->Level() >= KOTS_BFG_3X_DAMAGE )
				x = 2.0;
			else if ( user->Level() >= KOTS_BFG_2X_DAMAGE )
				x = 1.5;
			else if ( user->Level() >= KOTS_BFG_15X_DAMAGE )
				x = 1.25;
			break;

		case MOD_HANDGRENADE:
			damage = KOTS_HGRENADE_BASE_DAMAGE;

			if ( user->Level() >= KOTS_HGRENADE_DAMAGE4 )
				x = 2.5;
			else if ( user->Level() >= KOTS_HGRENADE_DAMAGE3 )
				x = 2.0;
			else if ( user->Level() >= KOTS_HGRENADE_DAMAGE2 )
				x = 1.5;
			else if ( user->Level() >= KOTS_HGRENADE_DAMAGE1 )
				x = 1.25;
			break;
	};
	damage = damage * x;

	return damage;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSHaste
//*************************************************************************************
//*************************************************************************************

int KOTSHaste( edict_t *ent )
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return false;

	return false;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSilent
//*************************************************************************************
//*************************************************************************************

int KOTSSilent( edict_t *ent, int mod )
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return false;

	switch ( mod )
	{
		case MOD_BLASTER:			
			return ( user->Level() >= KOTS_SWORD_SILENT     );
			break;
		case MOD_SHOTGUN:			
			return ( user->Level() >= KOTS_SHOTGUN_SILENT   );
			break;
		case MOD_SSHOTGUN:		
			return ( user->Level() >= KOTS_SSHOTGUN_SILENT  );
			break;
		case MOD_MACHINEGUN:	
			return ( user->Level() >= KOTS_MGUN_SILENT      );
			break;
		case MOD_CHAINGUN:		
			return ( user->Level() >= KOTS_CGUN_SILENT      );
			break;
		case MOD_GRENADE:			
			return ( user->Level() >= KOTS_GLAUNCHER_SILENT );
			break;
		case MOD_ROCKET:			
			return ( user->Level() >= KOTS_RLAUNCHER_SILENT );
			break;
		case MOD_HYPERBLASTER:
			return ( user->Level() >= KOTS_HYPER_SILENT     );
			break;
		case MOD_RAILGUN:			
			return ( user->Level() >= KOTS_RAILGUN_SILENT   );
			break;
		case MOD_BFG_LASER:		
			return ( user->Level() >= KOTS_BFG_SILENT       );
			break;
	};
	return false;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSpeed
//*************************************************************************************
//*************************************************************************************

int KOTSSpeed( edict_t *ent, int mod )
{
	int   speed = 0;
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return 1;

	switch ( mod )
	{
		case MOD_GRENADE:
			if ( user->Level() >= KOTS_GLAUNCHER_2X_SPEED )
				speed = KOTS_GLAUNCHER_BASE_SPEED2;
			else
				speed = KOTS_GLAUNCHER_BASE_SPEED; 
			break;
	
		case MOD_ROCKET:			
			if ( user->Level() >= KOTS_RLAUNCHER_2X_SPEED )
				speed = KOTS_RLAUNCHER_BASE_SPEED2;
			else
				speed = KOTS_RLAUNCHER_BASE_SPEED; 
			break;
		
		case MOD_HYPERBLASTER:

			if ( user->Level() >= KOTS_HYPER_2X_SPEED )
				speed = KOTS_HYPER_BASE_SPEED2;
			else
				speed = KOTS_HYPER_BASE_SPEED;
		
			break;

	};
	return speed;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSModKarma
//*************************************************************************************
//*************************************************************************************

int KOTSModKarma( edict_t *ent )
{
	CUser *user = KOTSGetUser( ent );
	
	if ( !user || ent->deadflag )
		return 1;

	if ( user->Level() >= KOTS_KARMA_BOOST )
		return 3;

	return 1;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSpecial
//*************************************************************************************
//*************************************************************************************

int KOTSSpecial( edict_t *ent, int mod, int type )
{
	int   length;
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return false;

	switch ( mod )
	{
		case MOD_BLASTER:
			length = KOTS_SWORD_BASE_LENGTH;

			if ( user->Level() >= KOTS_SWORD_2X_LENGTH )
				length *= 1.5;
			else if ( user->Level() >= KOTS_SWORD_15X_LENGTH )
				length *= 1.25;

			return length;
			break;

		case MOD_BFG_LASER:
			if ( user->Level() >= KOTS_BFG_LONGER )
				return true;
			else
				return false;
			break;

		case MOD_HANDGRENADE:
			if ( user->Level() >= type )
				return true;
			else
				return false;
			break;

		case MOD_RAILGUN:
			if ( user->Level() >= KOTS_RAILGUN_HASTE )
				return true;
			else
				return false;
			break;

		case MOD_CHAINGUN:
			if ( type == KOTS_CGUN_TRACERS )
			{
				if ( user->Level() >= KOTS_CGUN_TRACERS )
					return true;
				else
					return false;
			}
			else if ( type == KOTS_CGUN_NOSPIN )
			{
				if ( user->Level() >= KOTS_CGUN_NOSPIN )
					return true;
				else
					return false;
			}
			break;

		case MOD_MACHINEGUN:
			if ( user->Level() >= KOTS_MGUN_TRACERS )
				return true;
			else
				return false;
			break;

		case MOD_SHOTGUN:
			if ( user->Level() >= KOTS_SHOTGUN_2X_SHOT )
				return true;
			else
				return false;
			break;

		case MOD_SSHOTGUN:
			if ( type == KOTS_SSHOTGUN_1X_RADIUS )
			{
				if ( user->Level() >= KOTS_SSHOTGUN_1X_RADIUS )
					return true;
				else
					return false;
			}
			else if ( type == KOTS_SSHOTGUN_2X_RADIUS )
			{
				if ( user->Level() >= KOTS_SSHOTGUN_2X_RADIUS )
					return true;
				else
					return false;
			}
			break;

		case MOD_ROCKET:
			if ( user->Level() >= KOTS_RLAUNCHER_TRAILS )
				return true;
			else
				return false;

		case MOD_GRENADE:
			if ( user->Level() >= KOTS_GLAUNCHER_TRAILS )
				return true;
			else
				return false;
	}
	return false;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSModDamage
//*************************************************************************************
//*************************************************************************************

int KOTSModDamage( edict_t *ent, int damage, int mod )
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return damage;
	
	switch ( mod )
	{
		case MOD_G_SPLASH:		 
		case MOD_HG_SPLASH:	 
		case MOD_HELD_GRENADE:
			if ( user->Level() >= KOTS_STRENGTH_25XGREN )
				damage *= .50;

			break;
		case MOD_SPLASH:		
		case MOD_R_SPLASH:	
			if ( user->Level() >= KOTS_WISDOM_25XROCKET )
				damage *= .50;

			break;
		case MOD_WATER:		
			if ( user->Level() >= KOTS_DEXTERITY_25XWATER )
				damage *= .25;

			break;
		case MOD_SLIME:		
			if ( user->Level() >= KOTS_TECHNO_25XSLIME )
				damage *= .25;

			break;
		case MOD_LAVA:			
			if ( user->Level() >= KOTS_TECHNO_25XLAVA	)
				damage *= .25;

			break;
		case MOD_FALLING:	
			if ( user->Level() >= KOTS_DEXTERITY_25XFALLING )
				damage *= .25;

			break;
	};
	return damage;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSMakeMega
//*************************************************************************************
//*************************************************************************************

void KOTSMakeMega( edict_t *ent )
{
	CUser   *user = KOTSGetUser( ent );
	edict_t *health;
	gitem_t *item;
	gitem_t mega;

	if ( !user || ent->deadflag )
		return;

	if ( user->Level() < KOTS_KARMA_MEGA )
		return;

	if ( ent->client->pers.inventory[ ITEM_INDEX( FindItem( "Power Cube" ) ) ] < KOTS_KARMA_MEGA_CELLS )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "you need 40 power cubes to create a mega health!!!\n" );      
		return;
	}
	item = FindItem( "Health" );
	
	if ( !item )
		return;

	memcpy( &mega, item, sizeof(gitem_t) );

	mega.world_model = "models/items/mega_h/tris.md2";
	mega.classname   = "item_health_mega";
	health = Drop_Item( ent, &mega );

	if ( !health )
		return;

	health->item  = item;
	health->count = 50;
//	health->style = 1 | 2;
	health->style = 1;

	ent->client->pers.inventory[ ITEM_INDEX( FindItem( "Power Cube" ) ) ] -= KOTS_KARMA_MEGA_CELLS;
}

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************

