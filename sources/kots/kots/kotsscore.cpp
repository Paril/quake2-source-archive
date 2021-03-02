//*************************************************************************************
//*************************************************************************************
// File: kotsscore.cpp
//*************************************************************************************
//*************************************************************************************

//#include "stdafx.h"
#include "kots.h"
#include <stdio.h>

#include "user.h"
#include "shared.h"

extern "C"
{
#include "g_local.h"
#include "exports.h"
#include "p_menu.h"
#include "super.h"
#include "superdef.h"
}
#include "kotscpp.h"

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSaveDamage
//*************************************************************************************
//*************************************************************************************

void KOTSSaveDamage( edict_t *attacker, edict_t *targ, int take, int high )
{
	CUser *user = KOTSGetUser( attacker );

	if ( !targ || !attacker || !user )
		return;

	if ( !targ->client || !attacker->client )
		return;

	if ( user->Level() < KOTS_LEVEL_DAMAGE && high )
		return;

	if ( user->Level() >= KOTS_LEVEL_DAMAGE && !high )
		return;

	if ( targ->client->kots_last == attacker->client )
		targ->client->kots_damage += take;
	else
	{
		targ->client->kots_last   = attacker->client;
		targ->client->kots_damage = take;
	}
	KOTSKnock( attacker, targ, take );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSPowerDamage
//*************************************************************************************
//*************************************************************************************

float KOTSPowerDamage( edict_t *ent, float damagepercell )
{
	float x     = 1.0;
	CUser *user = KOTSGetUser( ent );

	if ( !user )
		return damagepercell;

	x = user->GetAmmoMulti();

	x = damagepercell / x;
		
	return x;			
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSChangeScore
//*************************************************************************************
//*************************************************************************************

void KOTSChangeScore( CUser *user, int amount, int type )
{
	bool blevel;

	blevel = user->ModScore( amount, type );

	if ( blevel )
	{
		gi.bprintf( PRINT_MEDIUM, "***************%s GAINED A LEVEL!!!!***************\n", user->m_ent->client->pers.netname );

		KOTSHelp( user, true );
	}
	else
		KOTSHelp( user, false );

	if ( type == USER_KILLED || type == USER_SUICIDE )
		KOTSPlayerDie( user );
}

//*************************************************************************************
//*************************************************************************************
// Function: PtsForKill
//*************************************************************************************
//*************************************************************************************

static void PtsForKill( edict_t *ekiller, edict_t *evict, long &lkiller, CUser *ukiller, 
                        CUser *uvict )
{
	long  lvict = 0;
	long  diff;
	long  damage  = evict->client->kots_damage;
	float ldiv;

	// I just got killed so erase my last killed info
	evict->client->kots_lastkilled = 0;
	evict->client->kots_lasttimes  = 0;

	if ( evict->client->kots_last != ekiller->client )
	{
		lkiller = 0;
		return;
	}

	if ( damage > evict->client->pers.max_health )
		damage = evict->client->pers.max_health;

	// vicitim is the same level or higher than the killer
	if ( uvict->Level() >= ukiller->Level() )
	{
		diff = uvict->Level() - ukiller->Level() + 1;

		// no losing points if you are level 0
		if ( uvict->Level() > 0 )
			lvict = 2 * diff;

		diff -= 1;
		ldiv  = 7.5 - (float)diff;
 	}
	else
	{
		diff = ukiller->Level() - uvict->Level();

		ldiv = 7.5 + (float)diff;
	}

	if ( ldiv < 1 )
		ldiv = 1.0;

	lkiller = (float)damage / ldiv;

	if ( lvict > USER_MAX_LOSS )
		lvict = USER_MAX_LOSS;

	KOTSChangeScore( uvict, -lvict, USER_KILLED );
}

//*************************************************************************************
//*************************************************************************************
// Function: OnASpree
//*************************************************************************************
//*************************************************************************************

static void OnASpree( edict_t *ekiller, CUser *ukiller, long &lkiller )
{
	char str[ 100 ];

	// Killing spree stuff
	ekiller->client->pers.kots_streak_count++;

	if ( ukiller->m_spree < ekiller->client->pers.kots_streak_count )
		ukiller->m_spree = ekiller->client->pers.kots_streak_count;

	if ( ekiller->client->pers.kots_streak_count <= KOTS_STREAK_COUNT )
	{
		ekiller->client->pers.kots_streak_points = 0;

		return;
	}

	if ( ekiller->client->pers.kots_streak_count == KOTS_STREAK_COUNT + 1 )
		ukiller->m_spreecount++;

	if (ekiller->client->pers.kots_streak_count < 10)
	{
		gi.bprintf( PRINT_MEDIUM, "%s is on a %d frag spree!\n", ekiller->client->pers.netname, ekiller->client->pers.kots_streak_count );
		ekiller->client->pers.kots_streak_points = lkiller * 2;
	}
	else if(ekiller->client->pers.kots_streak_count < 15)
	{
		gi.bprintf( PRINT_MEDIUM, "%s is on a rampage: %d frag spree!\n", ekiller->client->pers.netname, ekiller->client->pers.kots_streak_count );
		ekiller->client->pers.kots_streak_points = lkiller * 2.5;
	}
	else if(ekiller->client->pers.kots_streak_count < 25)
	{
		gi.bprintf( PRINT_MEDIUM, "%s is god-like: %d frag spree!\n", ekiller->client->pers.netname, ekiller->client->pers.kots_streak_count );
		ekiller->client->pers.kots_streak_points = lkiller * 3;
	}
	//25 isnt defined anywhere as spreewar number so it was hardcoded it 
	//in come back and change if needed
	else if( ekiller->client->pers.kots_streak_count == 25 )
	{
		//Mother added for special_items
		//drop any special items
		if(ekiller->client->pers.inventory[ ITEM_INDEX( FindItem( "Damage Amp" ) ) ])
		{	
			Drop_Item(ekiller,FindItemByClassname ("kots_damage_item"));
			ekiller->client->pers.inventory[ ITEM_INDEX( FindItem( "Damage Amp" ) ) ]--;
		}
		if(ekiller->client->pers.inventory[ ITEM_INDEX( FindItem( "boomerang" ) ) ])
		{	
			Drop_Item(ekiller,FindItemByClassname ("kots_boomerang"));
			ekiller->client->pers.inventory[ ITEM_INDEX( FindItem( "boomerang" ) ) ]--;
		}
		if(ekiller->client->pers.inventory[ ITEM_INDEX( FindItem( "resist" ) ) ])
		{	
			Drop_Item(ekiller,FindItemByClassname ("kots_resist_item"));
			ekiller->client->pers.inventory[ ITEM_INDEX( FindItem( "resist" ) ) ]--;
		}
		//end Mother added for special_items

		ukiller->m_spreewar++;

		sprintf( str, "SPREE WAR %s: %d frag spree!\n", ekiller->client->pers.netname, ekiller->client->pers.kots_streak_count );

		KOTSSendAll( false, str );

		ekiller->client->pers.kots_streak_points = lkiller * 5;
	}
	else
	{
		gi.bprintf( PRINT_MEDIUM, "SPREE WAR %s: %d frag spree!\n", ekiller->client->pers.netname, ekiller->client->pers.kots_streak_count );
		ekiller->client->pers.kots_streak_points = lkiller * 5;
	}
	lkiller = ekiller->client->pers.kots_streak_points;
}

//*************************************************************************************
//*************************************************************************************
// Function: EndASpree
//*************************************************************************************
//*************************************************************************************

static void EndASpree( edict_t *ekiller, edict_t *evict, CUser *ukiller, long &lkiller )
{
	char str[ 100 ];

	if ( evict->client->pers.kots_streak_count <= KOTS_STREAK_COUNT )
		return;

	if ( evict->client->pers.kots_streak_count >= 25 )
	{
		ukiller->m_spreewarbroke++;

		sprintf( str, "%s's SPREE WAR BROKEN by %s: %d frag spree!\n", evict->client->pers.netname,
		         ekiller->client->pers.netname, evict->client->pers.kots_streak_count );

		KOTSSendAll( false, str );

		lkiller = USER_MAX_GAIN;
	}
	else
	{
		ukiller->m_spreebroke++;

		gi.bprintf( PRINT_MEDIUM, "%s broke %s's %d frag spree!\n", ekiller->client->pers.netname,
		            evict->client->pers.netname, evict->client->pers.kots_streak_count );
	
		lkiller *= 5;
	}
}

//*************************************************************************************
//*************************************************************************************
// Function: Got2Fer
//*************************************************************************************
//*************************************************************************************

static void Got2Fer( edict_t *ekiller, CUser *ukiller, long &lkiller )
{
	if ( ekiller->client->pers.kots_lastkillframe > 0 )
	{
		if ( ekiller->client->pers.kots_lastkillframe + KOTS_2FER_FRAME >= level.framenum )
		{
			gi.bprintf( PRINT_MEDIUM, "%s got a 2fer!\n", ekiller->client->pers.netname );

			lkiller += KOTS_2FER_BONUS;

			ukiller->m_2fer++;
		}
	}
	ekiller->client->pers.kots_lastkillframe = level.framenum;
}

//*************************************************************************************
//*************************************************************************************
// Function: TeamplayScoring
//*************************************************************************************
//*************************************************************************************

static void TeamplayScoring( edict_t *ekiller, long lkiller )
{
	int     i;
	long    score;
	CUser   *user;
	edict_t *ent;

	score = lkiller / 5;

 	for ( i = 0; i < game.maxclients; i++ )
	{
		ent = g_edicts + 1 + i;

		if ( ent->health < 1 )
			continue;

		if ( !ent->inuse || !ent->client )
			continue;

		if ( ent == ekiller || !ent->client->resp.kots_ingame )
			continue;

		if ( ent->client->resp.kots_team != ekiller->client->resp.kots_team )
			continue;

		if ( ( user = KOTSGetUser( ent ) ) == NULL )
			continue;

		KOTSChangeScore( user, score, USER_NONE );
	}
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSScoring
//*************************************************************************************
//*************************************************************************************

int KOTSScoring( edict_t *evict, edict_t *ekiller, int type )
{
	long  lvict;
	long  lkiller;
	CUser *uvict;
	CUser *ukiller;
 
 	uvict = KOTSGetUser( evict );

	if ( !uvict )
		return 1;

	if ( kots_lives->value )
		evict->kots_lives--;

	evict->kotssave++;

	switch ( type )
	{
		case KOTS_SELFKILL:
			lvict = 3 * uvict->Level();

			KOTSChangeScore( uvict, -lvict, USER_SUICIDE );

			break;
		case KOTS_NORMAL:
		 	ukiller = KOTSGetUser( ekiller );

			if ( !ukiller )
				return 1;

			ekiller->kotssave++;
			ekiller->kots_kills++;

			PtsForKill( ekiller, evict, lkiller, ukiller, uvict );

			OnASpree( ekiller, ukiller, lkiller );

			EndASpree( ekiller, evict, ukiller, lkiller );

			Got2Fer( ekiller, ukiller, lkiller );

			if ( kots_teamplay->value )
				TeamplayScoring( ekiller, lkiller );
 
 			// I just got killed so erase my last killed info
			if ( ekiller->client->kots_lastkilled == evict )
				ekiller->client->kots_lasttimes++;
			else
			{
				ekiller->client->kots_lastkilled = evict;
				ekiller->client->kots_lasttimes  = 1;
			}

			if ( ekiller->client->kots_lasttimes >= KOTS_MAX_SAMEKILL )
				lkiller = 0;

			// Set max points allowed
			if ( lkiller > USER_MAX_GAIN )
				lkiller = USER_MAX_GAIN;

			// special boss scoring
			if ( uvict->m_bBossFlag > 0 )
				lkiller = USER_BOSS_SCORE;

			KOTSChangeScore( ukiller, lkiller, USER_KILL );

 			break;	
	}
	evict->client->pers.kots_streak_count  = 0;
	evict->client->pers.kots_streak_points = 0;

	evict->client->kots_last   = 0;
	evict->client->kots_damage = 0;

	return 1;
}

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************


