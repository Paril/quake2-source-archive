
#include "g_local.h"

qboolean sbit;
/*=================================	
Standard Bot Intelligence Test


	Tests the speed a bot learns
	
	NOTE: scrapped for now
=================================*/
void Cmd_Sbit_f(edict_t* self)
{
	edict_t* tankguy;


	sbit = true;
	creategame();	// create a normal deathmatch game

	//self->client->botinfo.mygame.sbit = true;	// mark the game as an sbit



	
	
	
	
	
	tankguy = G_Spawn();

	tankguy->s.origin[0] = 1226;
	tankguy->s.origin[1] = 692;
	tankguy->s.origin[2] = 490;
	SP_monster_supertank(tankguy);
	
	
}
	
	
	
	
edict_t* sbit_choose_target(edict_t* self)
{
	
		// NOTE: might be a good idea to make this 17
		// BOTNUMDEPENDENT: // might have to change this when changing number of bots
		// array should be big enough to hold all bots and clients
	edict_t* possible_targets[20];	// An array of pointers to possible
									// targets
	int		i, best;
	float	best_suitability = 0;

	vec3_t	difference;
	float 	distance;

	// NOTE: Fix up
	edict_t* sbittarget;

	best = 666;
	i = 1;
	/* A note from g_utils.c:
	=============
	G_Find

	Searches all active entities for the next one that holds
	the matching string at fieldofs (use the FOFS() macro) in the structure.

	Searches beginning at the edict after from, or the beginning if NULL
	NULL will be returned if the end of the list is reached.

	=============
	*/

	possible_targets[0] = NULL;
	// NOTE: searchs for supertanks now
	while ((possible_targets[i] = G_Find (possible_targets[i-1],FOFS(classname), "monster_supertank")) != NULL)
	{
		
		if (visible (self, possible_targets[i]))
			if (infront (self, possible_targets[i]))
				if (possible_targets[i]->health > 0)
				{
					VectorSubtract (self->s.origin, possible_targets[i]->s.origin, difference); 
					// find a vector that runs from self origin to poss[] origin ( or vice.versa)
					
					distance = VectorLength(difference); // find it's length
					
					

					// A close bot with say distance 30 would have 
					// suitability 1/30 = 0.0333
					// A far bot with distance 200 would have
					// suitability 1/200 = 0.005 so closer bot is more suitable
					if (distance == 0) // avoid divide by 0
					{
						safe_bprintf (PRINT_HIGH,"possible enemy with distance == 0\n");
						best = i;
						best_suitability = 1/distance; // use this entity's suitability
					}
					else // else preceed as normal
					{
					// Closer the target, the more suitable it is. ( roughly)
					// Equivalent to 
					//		suitability = 1/distance;
					//		if (suitability > best_suitability ) etc...
						if ( (1/distance) > best_suitability )
						{
							best = i;	// chose this entity for now
							best_suitability = 1/distance; // use this entity's suitability
						}
					}
				}

		// BOTNUMDEPENDENT: // might have to change this when changing number of bots
		if ( i>18)	// I hope this is right
		{
			safe_bprintf (PRINT_HIGH,"Out of spaces in possible_targets Array\n");
			return NULL;
		}
		i++; // go through while loop again, search edicts starting from current i
	
	} // end of while loop

	if ( best == 666 ) // if no target was found
		return NULL;
	else
		return possible_targets[best];//returns a pointer to an edict_t structure
	
}







void Cmd_Saypos_f(edict_t* self)
{
	long printnum;

	printnum = self->s.origin[0];
	safe_bprintf(PRINT_HIGH, "x: %i ",(long)printnum);

	printnum = self->s.origin[1];
	safe_bprintf(PRINT_HIGH, "y: %i ",(long)printnum);

	printnum = self->s.origin[2];
	safe_bprintf(PRINT_HIGH, "z: %i \n",(long)printnum);

}
void sayvelocity(edict_t* self)
{
	long printnum;

	printnum = self->velocity[0];
	safe_bprintf(PRINT_HIGH, "x: %i ",(long)printnum);

	printnum = self->velocity[1];
	safe_bprintf(PRINT_HIGH, "y: %i ",(long)printnum);

	printnum = self->velocity[2];
	safe_bprintf(PRINT_HIGH, "z: %i \n",(long)printnum);

}