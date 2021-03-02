//*************************************************************************************
//*************************************************************************************
// File: kcombat.cpp
//*************************************************************************************
//*************************************************************************************

//#include "stdafx.h"
#include "kots.h"
#include <stdlib.h>
#include <string.h>

#include "user.h"
#include "shared.h"

extern "C"
{
#include "g_local.h"
#include "exports.h"
#include "super.h"
#include "superdef.h"

qboolean IsFemale (edict_t *ent);
}
#include "kotscpp.h"

#define HEAD_HEIGHT 16.0

//*************************************************************************************
//*************************************************************************************
// Function: TeamDamage
//*************************************************************************************
//*************************************************************************************

static int TeamDamage( edict_t *targ, edict_t *attacker, int damage )
{
	int     i;
	int     teams[2];
	int     diff;
	CUser   *user;
	edict_t *cl_ent;

	memset( teams, 0, sizeof teams );

	if ( !targ->client || !attacker->client )
		return damage;

 	for ( i = 0; i < game.maxclients; i++ )
	{
		cl_ent = g_edicts + 1 + i;

		if ( !cl_ent->inuse )
			continue;

		user = KOTSGetUser( cl_ent );

		if ( !user )
			continue;

		if ( !cl_ent->client->resp.kots_ingame )
			continue;

		teams[ cl_ent->client->resp.kots_team ] += user->Level();
	}
	diff = abs( teams[0] - teams[1] );

	if ( teams[targ->client->resp.kots_team] > teams[attacker->client->resp.kots_team] )
		damage +=	diff;

	if ( damage <= 0 )
		damage = 1;

	return damage;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSWeirdRules
//*************************************************************************************
//*************************************************************************************

int KOTSWeirdRules( edict_t *targ, edict_t *attacker, int damage )
{
	if ( targ != attacker && kots_teamplay->value )
	{
		if ( OnSameTeam( targ, attacker ) )
			damage = 0;
		else
			damage = TeamDamage( targ, attacker, damage );
	}

	if ( level.kots_spreewar && targ != attacker && targ->client && attacker->client )
	{
		if ( attacker->client->pers.kots_streak_count < 25 && 
		     targ->client->pers.kots_streak_count < 25 ) 
		{
			damage = 0;
		}
	}
	return damage;
}

//*************************************************************************************
//*************************************************************************************
// Function: VerifyHeadShot
// Code for checking head shot, got from action quake mod but modified this heavily
// Their code was pretty sloppy and allowed shots over a players head to be a headshot
//*************************************************************************************
//*************************************************************************************

void VerifyHeadShot( vec3_t point, vec3_t dir, float height, vec3_t newpoint )
{
	/*called in this form:
		vec3_t new_point;
		VerifyHeadShot( point, dir, HEAD_HEIGHT, new_point );
	  */
	vec3_t normdir;
	vec3_t normdir2;
	
	//create a unit vector (unit vector means length of 1) in the direction the projectile 
	//was facing (dir) when it hit the BBOX and save in normdir
	VectorNormalize2(dir, normdir);

	//scale this vector to be the length defined by HEAD_HEIGHT and save in normdir2
	//Ok this is kinda funky vector stuff basically 'point' was the point on the BBOX
	//the projectile hit, and normdir is currently a unit vector that points in the direction
	//the projectile was going when it hit the BBOX, now let's scale this vector by 
	//a scalar 'height' (multiply) so it will now have an end point somewhere in the BBOX
	VectorScale( normdir, height, normdir2 );

	//add the vectors 'point' and normdir2 by superposition and save in newpoint
	//which was passed in.
	//What this does is it creates a new vector that points to the spot in the BBOX this 
	//projectile would have hit if it could go into the BBOX by 'HEAD_HEIGHT' distance
	//this vector has an absolute point IE it is relative to the world's origin (0,0,0)
	VectorAdd( point, normdir2, newpoint );


}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSHeadShot
//*************************************************************************************
//*************************************************************************************

int KOTSHeadShot( edict_t *attacker, edict_t *targ, vec3_t dir, vec3_t point, int damage, 
                  int mod )
{
	float	height, lower_bound, upper_bound;//height will be distance from the top of BBOX
	int		head_success = 0;
	float	z_rel;
	float	from_top;

	if ( damage < 1 )
		return 0;

	if ( strcmp( attacker->classname, "player" ) || strcmp( targ->classname, "player" ) )
		return damage;

	if ( !targ->client || targ->health <= 0 )
		return damage;

	if ( targ->client->pers.kots_helmet || attacker->client->pers.kots_helmet )
		return damage;

	if ( kots_teamplay->value && targ->client->resp.kots_team == attacker->client->resp.kots_team )
		return damage;

	switch ( mod )
	{
		case MOD_BLASTER:		
		case MOD_MACHINEGUN:	
		case MOD_GRENADE:		
		case MOD_ROCKET:			
		case MOD_HYPERBLASTER:	
		case MOD_RAILGUN:			
		case MOD_BFG_LASER:		
		case MOD_HANDGRENADE:	
			break;
		default:
			return damage;
	}

//shrink height a bit for female, height will represent the distance from the top of the 
//target players BBOX, so for shorter models (female) make this greater
	if (IsFemale(targ))
	 height = 8;//was 4, Mother changed 2-11-00
//Mother added 2-10-00
	else//this is for male to shrink the bounding box down from the top thus keeping
		//players from being able to shoot over someone's head to get a headshot
	 height = 7;//was 4

//set z_rel to be the difference between the z component (height) of the projectile hitting the player
//and the players origin (center of player)
	z_rel = point[2] - targ->s.origin[2]; 
	//find relative z from point where object hit the player and the targ players origin
	//z component

//ok this next function sets from_top to be the difference between the top of the players bounding 
//box and what we just found was the heigth of the projectile (z component), in other words this
//is the height distance (z component only) between the top of the bounding box, and where the 
//the projectile hit
	from_top = targ->maxs[2] - z_rel;

//This check is to see if the distance between the top of the bounding box and the projectiles
//height (z-component) is less than 2*HEAD_HEIGHT why 2*? Because since the projectile is coming
//at an angle it is possible that the attacking player's shot could be coming from below the 
//target player, the projectile would hit the BBOX below HEAD_HEIGHT, but it could have ended 
//up hitting the player in the head.
//This is just a quick check to rule out shots that are clearly not a headshot right off the bat
//detailed checking will done later if it looks like a candidate for a headshot
	if ( from_top < 2 * HEAD_HEIGHT )
	{
		vec3_t new_point;

//point is the vec3_t where the projectile touched the bounding box, dir is the direction including
//magnitude the projectile is traveling at when it hit the bounding box, therefore this next function
//creates a new vector that represents where the projectile would have stopped if we allowed it
//to go a distance of 'HEAD_HEIGHT' into the BBOX of the player, this new vector points to the exact 
//spot
		VerifyHeadShot( point, dir, HEAD_HEIGHT, new_point );

//subtracting the player's origin from the point the projectile would have gone in, gives a vector
//that runs from the players origin to the new_point, it like drawing the third side of a triangle 
//when you already have 2 sides drawn, and we save this vector, in new_point
	  VectorSubtract( new_point, targ->s.origin, new_point );
         
//Mother added 2-10-2000
	  //check for crouch!
	 if (targ->client->ps.pmove.pm_flags & PMF_DUCKED)
	{//if crouched use these
		upper_bound = targ->maxs[2];
		lower_bound = targ->maxs[2] - 7;
	}
	 else
	 {//if not crouched then use these
		upper_bound = targ->maxs[2] - height;
		lower_bound = targ->maxs[2] - (height +7);
	 }

	  //Mother added: this checks to see if the point is over the players head	  
	  //set upperbound for head
	  if((upper_bound - new_point[2]) < 0)
		return damage;//if negative difference then the shot is too high

	  //set lower bound for head (height + 7), make sure proj point is above this line 
	  if((lower_bound - new_point[2]) > 0)
		  return damage;//if positive difference then the shot is too low
		  

//if the distance between the top of the players bounding box 
//and this new point is less than HEAD_HEIGHT, AND the y-component of this distance is less than 
//HEAD_HEIGHT*.8...
//This checks for correct heigth and width, if it fits, then its a head shot, they consider
//the head width to be 80% of the BBOX, it was .8, Mother shrank this to .5 (2-10-2000)
	  if ( (upper_bound - new_point[2]) < HEAD_HEIGHT 
		  && (abs(new_point[1])) < HEAD_HEIGHT*.5 
		  && (abs(new_point[0])) < HEAD_HEIGHT*.5 )
	  {
		  head_success = 1;
	  }
	}
    
	if ( head_success )
	{
    //since KOTS has so much built in to protection, we will need to add a bit more here
		//this was originally meant to kill in a shot no matter what
		damage = damage * 2 + 1;

		if (attacker->kots_justhit < 1)
			gi.cprintf(targ, PRINT_HIGH, "Head damage\n"); 

		if (attacker->client && attacker->kots_justhit < 1 ) 
		{
			gi.cprintf(attacker, PRINT_HIGH, "You hit %s in the head\n", targ->client->pers.netname); 
		
			gi.sound(targ, CHAN_VOICE, gi.soundindex("makron/brain1.wav"), 1, ATTN_NORM, 0);                
		}
		targ->locOfDeath = LOC_HDAM;
	}
	return damage;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSRunes
//*************************************************************************************
//*************************************************************************************

int KOTSRunes( edict_t *targ, edict_t *attacker, int damage )
{
	//Mother added for resist rune
	if(strcmp(targ->classname,"player") == 0)
	{
		if (targ->client->pers.inventory[ ITEM_INDEX( FindItem( "resist" ) ) ]>0)
		{	//.571 evens out damage amp so if damage amp does 1.75 then to protect against it
			//fully you need to use .571
			damage *= 0.571;
			if (!damage)
				damage = 1;

			//only allow it to play every 2 seconds
			if (targ->pain_debounce_time < level.time)
			{
				gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect3.wav"), 1, ATTN_NORM, 0);
				targ->pain_debounce_time = level.time + 2;
			}//end only play every 2 seconds
	
		}
	}
//Mother added for 2x damage, noises are taken care of for each weapon otherwise they only
//sound quad when they hit
	if((strcmp(attacker->classname,"player") == 0) && (attacker->client->quad_framenum <= level.framenum))
	{
		if (attacker->client->pers.inventory[ ITEM_INDEX( FindItem( "Damage Amp" ) ) ]>0)
		{
			damage *= 1.75;
			if (!damage)
				damage = 1;
		}
	}
	return damage;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSHits
//*************************************************************************************
//*************************************************************************************

void KOTSHits( edict_t *attacker, edict_t *inflictor, edict_t *targ, int take, int mod, 
               int damage )
{
	//these next 2 lines only affects instant hit weapons
	if ( take > 0 )
		attacker->kots_justhit = 1;
	
	//this next section is for all non-instant hit weapons: 
	//It must appear before any reductions to the damage is done, IE armor checking.
//Mother
	//if it is a rocket, and it is a direct HIT not splash damage then count as hit
	if((take > 0) && (mod == MOD_ROCKET))//if it were splash damage (like from rocket jump) it would not be MOD_ROCKET
		attacker->kots_hits++;

	//else if it was a grenade from GL and it isn't the person who shot it, and it is doing at least 90% ot total possible damage then it is a hit
	//I used the % so that it is only a hit if the person was within a certain range of the grenade, this will simulate a direct hit
	//since all grenades only do splash damage. inflictor->dmg is set in the p_weapon.c file and already takes into account quad, and training.
	else if((take > 0) && (strcmp(inflictor->classname,"grenade" ) == 0) && (targ != attacker) && (damage > (.87*inflictor->dmg)))
		attacker->kots_hits++;

	//else if it was a hand grenade and it isn't the person who shot it, and it is doing at least 90% ot total possible damage then it is a hit
	//I used the % so that it is only a hit if the person was within a certain range of the grenade, this will simulate a direct hit
	//since all grenades only do splash damage. inflictor->dmg is set in the p_weapon.c file and already takes into account quad, and training.
	else if((take > 0) && (strcmp(inflictor->classname,"hgrenade" ) == 0) && (targ != attacker) && (damage > (.87*inflictor->dmg)))
		attacker->kots_hits++;

	//Hyperblaster support here:
	else if((take > 0) && (mod == MOD_HYPERBLASTER))
		attacker->kots_hits++;
//End Mother
}

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************

