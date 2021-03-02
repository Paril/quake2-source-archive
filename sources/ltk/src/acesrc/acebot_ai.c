///////////////////////////////////////////////////////////////////////
//
//  ACE - Quake II Bot Base Code
//
//  Version 1.0
//
//  Original file is Copyright(c), Steve Yeager 1998, All Rights Reserved
//
//
//	All other files are Copyright(c) Id Software, Inc.
////////////////////////////////////////////////////////////////////////
/*
 * $Header: /LicenseToKill/src/acesrc/acebot_ai.c 25    2/11/99 17:02 Riever $
 *
 * $Log: /LicenseToKill/src/acesrc/acebot_ai.c $
 * 
 * 25    2/11/99 17:02 Riever
 * Fog support added for bot vision and target selection.
 * 
 * 24    27/10/99 18:49 Riever
 * Added lightlevel check for enemy selection
 * 
 * 23    22/10/99 7:36 Riever
 * Commented checkshot for future development.
 * 
 * 22    22/10/99 7:08 Riever
 * Removed distance from weightings.
 * 
 * 21    22/10/99 6:22 Riever
 * Modified LR goal item selection weightings.
 * 
 * 20    16/10/99 12:06 Riever
 * Changed checkshot code to use a single pixel beam for now.
 * 
 * 19    11/10/99 7:47 Riever
 * Fixed SP respawn bug (There was no respawn!)
 * 
 * 18    6/10/99 20:28 Riever
 * Stopped targetting spectators for long range goals.
 * 
 * 17    6/10/99 18:15 Riever
 * ACEAI_Cmd_Choose() for Teamplay equipment.
 * 
 * 16    6/10/99 17:51 Riever
 * Teamplay: Bot spectators don't think.
 * 
 * 15    6/10/99 17:40 Riever
 * Added TeamPlay state STATE_POSITION to enable bots to seperate and
 * avoid centipede formations.
 * 
 * 14    27/09/99 20:47 Riever
 * Removed "current enemy" check
 * Added line of sight check for "hurt by enemy"
 * 
 * 13    27/09/99 14:27 Riever
 * Removed cost weighting from item selection on long range goals
 * 
 * 12    27/09/99 7:05 Riever
 * Added more weapon selection groups based on range
 * 
 * 11    26/09/99 15:34 Riever
 * Look for nearby enemies even behind us
 * 
 * 10    26/09/99 7:34 Riever
 * Bot vision restricted to FOV
 * Enemey selection now chooses closest target
 * 
 * 9     21/09/99 20:20 Riever
 * Take teamPauseTime into account when choosing long range goals.
 * 
 * 8     21/09/99 17:22 Riever
 * Put a timer in to stop the bots going centipede style in
 * TeamPlay.(teamPauseTime)
 * 
 * 7     21/09/99 12:20 Riever
 * 
 * 6     18/09/99 19:47 Riever
 * If range is over 300, HC and Knife will not be chosen
 * 
 * 5     18/09/99 19:23 Riever
 * Increased random door opening time to 5 seconds
 * 
 * 4     18/09/99 10:16 Riever
 * changed door opening attempt time to be 3 + random
 * 
 * 3     18/09/99 9:49 Riever
 * Stopped the centipede effect in teamplay
 * 
 * 2     13/09/99 19:52 Riever
 * Added headers
 *
 */
///////////////////////////////////////////////////////////////////////
//
//  acebot_ai.c -      This file contains all of the 
//                     AI routines for the ACE II bot.
//
//
// NOTE: I went back and pulled out most of the brains from
//       a number of these functions. They can be expanded on 
//       to provide a "higher" level of AI. 
////////////////////////////////////////////////////////////////////////

#include "g_local.h"
#include "m_player.h"

#include "acebot.h"
// CGF_FOG ADD
#include "cgf_sfx_fog.h"
// CGF_FOG END
void	ACEAI_Cmd_Choose( edict_t *ent, char *s);

///////////////////////////////////////////////////////////////////////
// Main Think function for bot
///////////////////////////////////////////////////////////////////////
void ACEAI_Think (edict_t *self)
{
	usercmd_t	ucmd;

	// Set up client movement
	VectorCopy(self->client->ps.viewangles,self->s.angles);
	VectorSet (self->client->ps.pmove.delta_angles, 0, 0, 0);
	memset (&ucmd, 0, sizeof (ucmd));
	self->enemy = NULL;
	self->movetarget = NULL;
	
	// Force respawn 
	if (self->deadflag)
	{
		self->client->buttons = 0;
		ucmd.buttons = BUTTON_ATTACK;
	}

	// Teamplay spectator?
//	if( self->solid == SOLID_NOT)
//		return;

	if(self->state == STATE_WANDER && 
		(self->wander_timeout < level.time)
		)
	  ACEAI_PickLongRangeGoal(self); // pick a new long range goal

	// In teamplay pick a random node
	if( self->state == STATE_POSITION )
	{
		if( level.time >= self->teamPauseTime)
		{
			// We've waited long enough - let's go kick some ass!
			self->state = STATE_WANDER;
		}
		// Don't go here too often
		if( self->goal_node == INVALID || self->wander_timeout < level.time )
			ACEAI_PickLongRangeGoal(self);
	}

	// Kill the bot if completely stuck somewhere
	if(VectorLength(self->velocity) > 37) //
		self->suicide_timeout = level.time + 10.0;

	if(self->suicide_timeout < level.time && !teamplay->value)
	{
		self->health = 0;
		player_die (self, self, self, 100000, vec3_origin);
	}

	// Find any short range goal
	ACEAI_PickShortRangeGoal(self);
	
	// Look for enemies
	if(ACEAI_FindEnemy(self))
	{	
		ACEAI_ChooseWeapon(self);
		ACEMV_Attack (self, &ucmd);
	}
	else
	{
		// Are we hurt?
		if( self->health < 100)
		{
			Cmd_Bandage_f ( self );
		}
		// Execute the move, or wander
		if(self->state == STATE_WANDER)
			ACEMV_Wander(self,&ucmd);
		else if( (self->state == STATE_MOVE) || (self->state == STATE_POSITION) )
			ACEMV_Move(self,&ucmd);
	}

//AQ2 ADD
	if(self->last_door_time < (level.time - 5.0 - random()) )
	{
		// Toggle any door that may be nearby
		//@@ Temporary until I get better code in! Needs to trace for the door
		Cmd_OpenDoor_f ( self );
		self->last_door_time = level.time;
	}
//AQ2 END

	//debug_printf("State: %d\n",self->state);

	// set approximate ping
	ucmd.msec = 75 + floor (random () * 25) + 1;

	// show random ping values in scoreboard
	self->client->ping = ucmd.msec;

	// set bot's view angle
	ucmd.angles[PITCH] = ANGLE2SHORT(self->s.angles[PITCH]);
	ucmd.angles[YAW] = ANGLE2SHORT(self->s.angles[YAW]);
	ucmd.angles[ROLL] = ANGLE2SHORT(self->s.angles[ROLL]);
	
	// send command through id's code
	ClientThink (self, &ucmd);
	
	self->nextthink = level.time + FRAMETIME;
}

///////////////////////////////////////////////////////////////////////
// Evaluate the best long range goal and send the bot on
// its way. This is a good time waster, so use it sparingly. 
// Do not call it for every think cycle.
///////////////////////////////////////////////////////////////////////
void ACEAI_PickLongRangeGoal(edict_t *self)
{

	int i;
	int node;
	float weight,best_weight=0.0;
	int current_node,goal_node;
	edict_t *goal_ent;
	float cost;
	
	// look for a target 
	current_node = ACEND_FindClosestReachableNode(self,NODE_DENSITY,NODE_ALL);

	self->current_node = current_node;

	// Even in teamplay, we wander if no valid node
	if(current_node == -1)
	{
		self->state = STATE_WANDER;
		self->wander_timeout = level.time + 1.0;
		self->goal_node = -1;
		return;
	}

	//======================
	// Teamplay POSITION state
	//======================
	if( self->state == STATE_POSITION )
	{
		int counter = 0;
		cost = INVALID;
		self->goal_node = INVALID;

		// Pick a random node to go to
		while( cost == INVALID && counter < 10) // Don't look for too many
		{
			counter++;
			i = (int)(random() * numnodes -1);	// Any of the current nodes will do
			cost = ACEND_FindCost(current_node, i);

			if(cost == INVALID || cost < 2) // ignore invalid and very short hops
			{
				cost = INVALID;
				i = INVALID;
				continue;
			}
		}
		// We have a target node - just go there!
		if( i != INVALID )
		{
			self->state = STATE_MOVE;
			self->tries = 0; // Reset the count of how many times we tried this goal
			ACEND_SetGoal(self,i);
			self->wander_timeout = level.time + 1.0;
			return;
		}
	}

	///////////////////////////////////////////////////////
	// Items
	///////////////////////////////////////////////////////
	for(i=0;i<num_items;i++)
	{
		if(item_table[i].ent == NULL || item_table[i].ent->solid == SOLID_NOT) // ignore items that are not there.
			continue;
		
		cost = ACEND_FindCost(current_node,item_table[i].node);
		
		if(cost == INVALID || cost < 2) // ignore invalid and very short hops
			continue;
	
		weight = ACEIT_ItemNeed(self, item_table[i].item);

/*		// If I am on team one and I have the flag for the other team....return it
		if(ctf->value && (item_table[i].item == ITEMLIST_FLAG2 || item_table[i].item == ITEMLIST_FLAG1) &&
		  (self->client->resp.ctf_team == CTF_TEAM1 && self->client->pers.inventory[ITEMLIST_FLAG2] ||
		   self->client->resp.ctf_team == CTF_TEAM2 && self->client->pers.inventory[ITEMLIST_FLAG1]))
			weight = 10.0;*/

		weight *= ( (rand()%5) +1 ); // Allow random variations
//		weight /= cost; // Check against cost of getting there
				
		if(weight > best_weight && item_table[i].node != INVALID)
		{
			best_weight = weight;
			goal_node = item_table[i].node;
			goal_ent = item_table[i].ent;
		}
	}

	///////////////////////////////////////////////////////
	// Players
	///////////////////////////////////////////////////////
	// This should be its own function and is for now just
	// finds a player to set as the goal.
	for(i=0;i<num_players;i++)
	{
		if( (players[i] == self) || (players[i]->solid == SOLID_NOT) )
			continue;

		// If it's dark and he's not already our enemy, ignore him
		if( self->enemy && players[i] != self->enemy)
		{
			if( players[i]->light_level < 30)
				continue;
// CGF_FOG ADD
			// Check for FOG!
			if( CGF_SFX_IsFogEnabled() )
			{
				vec3_t	v;
				float	range;
				
				// Get distance to enemy
				VectorSubtract (self->s.origin, players[i]->s.origin, v);
				range = VectorLength(v);
				// If fog index is < 0.1 we can't see him
				if( CGF_SFX_GetFogForDistance(range) < 0.1)
					continue;
			}
// CGF_FOG END
		}

		node = ACEND_FindClosestReachableNode(players[i],NODE_DENSITY,NODE_ALL);
		// RiEvEr - bug fixing
		if( node == INVALID)
			cost = INVALID;
		else
			cost = ACEND_FindCost(current_node, node);

		if(cost == INVALID || cost < 3) // ignore invalid and very short hops
			continue;

/*		// Player carrying the flag?
		if(ctf->value && (players[i]->client->pers.inventory[ITEMLIST_FLAG2] || players[i]->client->pers.inventory[ITEMLIST_FLAG1]))
		  weight = 2.0;
		else*/
		// Stop the centipede effect in teamplay
		if( teamplay->value )
		{
			// Check it's an enemy
			// If not an enemy, don't follow him
			if( OnSameTeam( self, players[i]))
				weight = 0.0;
			else
				weight = 0.3;
		}
		else
		  weight = 0.3; 
		
		weight *= ( (rand()%5) +1 ); // Allow random variations
//		weight /= cost; // Check against cost of getting there
		
		if(weight > best_weight && node != INVALID)
		{		
			best_weight = weight;
			goal_node = node;
			goal_ent = players[i];
		}	
	}

	// If do not find a goal, go wandering....
	if(best_weight == 0.0 || goal_node == INVALID )
	{
		self->goal_node = INVALID;
		self->state = STATE_WANDER;
		self->wander_timeout = level.time + 1.0;
		if(debug_mode)
			debug_printf("%s did not find a LR goal, wandering.\n",self->client->pers.netname);
		return; // no path? 
	}
	
	// OK, everything valid, let's start moving to our goal.
	self->state = STATE_MOVE;
	self->tries = 0; // Reset the count of how many times we tried this goal
	 
	if(goal_ent != NULL && debug_mode)
		debug_printf("%s selected a %s at node %d for LR goal.\n",self->client->pers.netname, goal_ent->classname, goal_node);

	ACEND_SetGoal(self,goal_node);

}

///////////////////////////////////////////////////////////////////////
// Pick best goal based on importance and range. This function
// overrides the long range goal selection for items that
// are very close to the bot and are reachable.
///////////////////////////////////////////////////////////////////////
void ACEAI_PickShortRangeGoal(edict_t *self)
{
	edict_t *target;
	float weight,best_weight=0.0;
	edict_t *best;
	int index;
	
	// look for a target (should make more efficient later)
	target = findradius(NULL, self->s.origin, 200);
	
	while(target)
	{
		if(target->classname == NULL)
			return;
		
		// Missle avoidance code
		// Set our movetarget to be the rocket or grenade fired at us. 
		if(strcmp(target->classname,"rocket")==0 || strcmp(target->classname,"grenade")==0)
		{
			if(debug_mode) 
				debug_printf("ROCKET ALERT!\n");

			self->movetarget = target;
			return;
		}
	
		if (ACEIT_IsReachable(self,target->s.origin))
		{
			if (infront(self, target))
			{
				index = ACEIT_ClassnameToIndex(target->classname);
				weight = ACEIT_ItemNeed(self, index);
				
				if(weight > best_weight)
				{
					best_weight = weight;
					best = target;
				}
			}
		}

		// next target
		target = findradius(target, self->s.origin, 200);
	}

	if(best_weight)
	{
		self->movetarget = best;
		
		if(debug_mode && self->goalentity != self->movetarget)
			debug_printf("%s selected a %s for SR goal.\n",self->client->pers.netname, self->movetarget->classname);
		
		self->goalentity = best;

	}

}

///////////////////////////////////////////////////////////////////////
// Scan for enemy (simplifed for now to just pick any visible enemy)
///////////////////////////////////////////////////////////////////////
// Modified by RiEvEr
// Chooses nearest enemy or last person to shoot us
//
qboolean ACEAI_FindEnemy(edict_t *self)
{
	int i;
	edict_t		*bestenemy = NULL;
	float		bestweight = 99999;
	float		weight;
	vec3_t		dist;

/*	// If we already have an enemy and it is the last enemy to hurt us
	if (self->enemy && 
		(self->enemy == self->client->attacker) &&
		(!self->enemy->deadflag) &&
		(self->enemy->solid != SOLID_NOT)
		)
	{
		return true;
	}
*/
	for(i=0;i<=num_players;i++)
	{
		if(players[i] == NULL || players[i] == self || 
		   players[i]->solid == SOLID_NOT)
		   continue;
	
		// If it's dark and he's not already our enemy, ignore him
		if( self->enemy && players[i] != self->enemy)
		{
			if( players[i]->light_level < 30)
				continue;
// CGF_FOG ADD
			// Check for FOG!
			if( CGF_SFX_IsFogEnabled() )
			{
				vec3_t	v;
				float	range;
				
				// Get distance to enemy
				VectorSubtract (self->s.origin, players[i]->s.origin, v);
				range = VectorLength(v);
				// If fog index is < 0.1 we can't see him
				if( CGF_SFX_GetFogForDistance(range) < 0.1)
					continue;
			}
// CGF_FOG END
		}

/*		if(ctf->value && 
		   self->client->resp.ctf_team == players[i]->client->resp.ctf_team)
		   continue;*/
// AQ2 ADD
		if(teamplay->value && OnSameTeam( self, players[i]) )
		   continue;
// AQ2 END

		if(!players[i]->deadflag && visible(self, players[i]) && 
			gi.inPVS(self->s.origin, players[i]->s.origin)	)
		{
// RiEvEr
			// Now we assess this enemy
			VectorSubtract(self->s.origin, players[i]->s.origin, dist);
			weight = VectorLength( dist );

			// Can we see this enemy, or is it so close that we should not ignore it!
			if( infront( self, players[i] ) ||
				(weight < 300 )	)
			{
				// See if it's better than what we have already
				if (weight < bestweight)
				{
					bestweight = weight;
					bestenemy = players[i];
				}
			}
		}
	}
	// If we found a good enemy set it up
	if( bestenemy)
	{
		self->enemy = bestenemy;
		return true;
	}
	// Check if we've been shot from behind or out of view
	if( self->client->attacker )
	{
		// Check if it was recent
		if( self->client->push_timeout > 0)
		{
			if(!self->client->attacker->deadflag && visible(self, self->client->attacker) && 
				gi.inPVS(self->s.origin, self->client->attacker->s.origin)	)
			{
				self->enemy = self->client->attacker;
				return true;
			}
		}
	}
//R
	// Otherwise signal "no enemy available"
	return false;
  
}

///////////////////////////////////////////////////////////////////////
// Hold fire with RL/BFG?
///////////////////////////////////////////////////////////////////////
//@@ Modify this to check for hitting teammates in teamplay games.
qboolean ACEAI_CheckShot(edict_t *self)
{
	trace_t tr;

//AQ2	tr = gi.trace (self->s.origin, tv(-8,-8,-8), tv(8,8,8), self->enemy->s.origin, self, MASK_OPAQUE);
//	tr = gi.trace (self->s.origin, tv(-8,-8,-8), tv(8,8,8), self->enemy->s.origin, self, MASK_SOLID|MASK_OPAQUE);
	tr = gi.trace (self->s.origin, vec3_origin, vec3_origin, self->enemy->s.origin, self, MASK_SOLID|MASK_OPAQUE);
	
	// Blocked, do not shoot
	if (tr.fraction < 0.9)
		return false; 
	
	return true;
}

///////////////////////////////////////////////////////////////////////
// Choose the best weapon for bot (simplified)
///////////////////////////////////////////////////////////////////////
void ACEAI_ChooseWeapon(edict_t *self)
{	
	float range;
	vec3_t v;
	
	// if no enemy, then what are we doing here?
	if(!self->enemy)
		return;
//AQ2 CHANGE	
	// Currently always favor the dual pistols!
	//@@ This will become the "bot choice" weapon
//	if(ACEIT_ChangeDualSpecialWeapon(self,FindItem(DUAL_NAME)))
//   	   return;
//AQ2 END

	// Base selection on distance.
	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	range = VectorLength(v);
		
	// Longer range 
	if(range > 1000)
	{
		if(ACEIT_ChangeSniperSpecialWeapon(self,FindItem(SNIPER_NAME)))
			return;
		
		if(ACEIT_ChangeM3SpecialWeapon(self,FindItem(M3_NAME)))
			return;
		
		if(ACEIT_ChangeM4SpecialWeapon(self,FindItem(M4_NAME)))
			return;
		
		if(ACEIT_ChangeMP5SpecialWeapon(self,FindItem(MP5_NAME)))
   		   return;

		if(ACEIT_ChangeMK23SpecialWeapon(self,FindItem(MK23_NAME)))
   		   return;
	}
	
	// Longer range 
	if(range > 700)
	{		
		if(ACEIT_ChangeM3SpecialWeapon(self,FindItem(M3_NAME)))
			return;
		
		if(ACEIT_ChangeM4SpecialWeapon(self,FindItem(M4_NAME)))
			return;
		
		if(ACEIT_ChangeMP5SpecialWeapon(self,FindItem(MP5_NAME)))
   		   return;

	if(ACEIT_ChangeSniperSpecialWeapon(self,FindItem(SNIPER_NAME)))
		return;
	
		if(ACEIT_ChangeMK23SpecialWeapon(self,FindItem(MK23_NAME)))
   		   return;
	}
	
	// Longer range 
	if(range > 500)
	{		
		if(ACEIT_ChangeMP5SpecialWeapon(self,FindItem(MP5_NAME)))
   		   return;

		if(ACEIT_ChangeM3SpecialWeapon(self,FindItem(M3_NAME)))
			return;
		
		if(ACEIT_ChangeM4SpecialWeapon(self,FindItem(M4_NAME)))
			return;
		
	if(ACEIT_ChangeSniperSpecialWeapon(self,FindItem(SNIPER_NAME)))
		return;
	
		if(ACEIT_ChangeMK23SpecialWeapon(self,FindItem(MK23_NAME)))
   		   return;
	}
	
	// Longer range 
	if(range > 300)
	{		
		if(ACEIT_ChangeM4SpecialWeapon(self,FindItem(M4_NAME)))
			return;
		
		if(ACEIT_ChangeMP5SpecialWeapon(self,FindItem(MP5_NAME)))
   		   return;

		if(ACEIT_ChangeM3SpecialWeapon(self,FindItem(M3_NAME)))
			return;
		
	if(ACEIT_ChangeSniperSpecialWeapon(self,FindItem(SNIPER_NAME)))
		return;
	
		if(ACEIT_ChangeMK23SpecialWeapon(self,FindItem(MK23_NAME)))
   		   return;
	}
	
	// Short range	   
	if(ACEIT_ChangeHCSpecialWeapon(self,FindItem(HC_NAME)))
		return;
	
	if(ACEIT_ChangeSniperSpecialWeapon(self,FindItem(SNIPER_NAME)))
		return;
	
	if(ACEIT_ChangeM3SpecialWeapon(self,FindItem(M3_NAME)))
		return;
	
	if(ACEIT_ChangeM4SpecialWeapon(self,FindItem(M4_NAME)))
		return;
	
	if(ACEIT_ChangeMP5SpecialWeapon(self,FindItem(MP5_NAME)))
   	   return;
	
	if(ACEIT_ChangeDualSpecialWeapon(self,FindItem(DUAL_NAME)))
   	   return;

	if(ACEIT_ChangeMK23SpecialWeapon(self,FindItem(MK23_NAME)))
   	   return;
	
	if(ACEIT_ChangeWeapon(self,FindItem(KNIFE_NAME)))
   	   return;
	
	return;

}

void ACEAI_Cmd_Choose (edict_t *ent, char *s)
{
    // only works in teamplay
    if (!teamplay->value)
            return;
    
    if ( stricmp(s, MP5_NAME) == 0 )
            ent->client->resp.weapon = FindItem(MP5_NAME);
    else if ( stricmp(s, M3_NAME) == 0 )
            ent->client->resp.weapon = FindItem(M3_NAME);
    else if ( stricmp(s, M4_NAME) == 0 )
            ent->client->resp.weapon = FindItem(M4_NAME);
    else if ( stricmp(s, HC_NAME) == 0 )
            ent->client->resp.weapon = FindItem(HC_NAME);
    else if ( stricmp(s, SNIPER_NAME) == 0 )
            ent->client->resp.weapon = FindItem(SNIPER_NAME);
    else if ( stricmp(s, KNIFE_NAME) == 0 )
            ent->client->resp.weapon = FindItem(KNIFE_NAME);
    else if ( stricmp(s, DUAL_NAME) == 0 )
            ent->client->resp.weapon = FindItem(DUAL_NAME);
    else if ( stricmp(s, KEV_NAME) == 0 )
            ent->client->resp.item = FindItem(KEV_NAME);
    else if ( stricmp(s, LASER_NAME) == 0 )
            ent->client->resp.item = FindItem(LASER_NAME);
    else if ( stricmp(s, SLIP_NAME) == 0 )
            ent->client->resp.item = FindItem(SLIP_NAME);
    else if ( stricmp(s, SIL_NAME) == 0 )
            ent->client->resp.item = FindItem(SIL_NAME);
    else if ( stricmp(s, BAND_NAME) == 0 )
            ent->client->resp.item = FindItem(BAND_NAME);
}
