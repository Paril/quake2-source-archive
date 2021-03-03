/*========================================================================
	Neuralbot v0.6						23/November/99
	--------------
		
Neuralbot is made by Nicholas Chapman (c)1999

Feel free to slice and dice this code as you see fit.
	If you want to use any of the code for any of your projects, feel free.
If you release this project, drop me a line as I would like to see what 
this code has been used for.
	If you wish to use any of this code in a commercial release, you must gain 
my written consent first.

The area of bot AI is large, and way more experimentation needs to be done
with neural-networks and g.a.s in bots than I could ever do.  So I encourage 
you all to use this code as a base or inspiration or whatever and see if you 
can squeeze some learning out of a bot.

  Nick Chapman/Ono-Sendai		nickamy@paradise.net.nz		
								onosendai@botepidemic.com
								icq no# 19054613

==========================================================================*/

#include "g_local.h"

/*============================================================================
bot input functions
-------------------
Each function is used to charge up a certain input neuron.
Each function will be called for each bot every server frame ( 0.1 secs )

============================================================================*/

#define ACTIVATION_NUMERATOR 10
//used in calculating what the input fractions return if they do not return binary values



/* ===========================================================================
special function: choose_target
---------------------------------------------------------------------------
This chooses what entity the bot considers to be it's current enemy.
This will be called every bot think ( 100ms )
I am not going to implement any tactical criteria, ie. how much health 
possible target has.  That would be a bit too interferist.


With enough computing power, it would not be neccesary for me to program
this in.  For instance, tracelines could be used to represent what each retina
receptor sees.  For instance, if the bot found that a lot of the tracelines to 
the right of the screen where hitting an entity, the bot might decide that
this might be it's enemy and to move in that direction.  
At an even lower level would be setting up an environment for visual
pattern recognition based on the way the human eye works - there could be 
receptors for each different color at each different 'pixel' of the bot's
'screen'.
This will have to wait for future generations of bots though.
QuakeXVII perhaps? 


for now I will manually program in how the bot chooses it's target.
the target must be:

* in front of  the bot

* not obstructed by any bit of the level

* another bot or player client
  


* alive 

* not dead

* still fighting

* a real fighter

* a real go-getter


////////// note: //////////
  G_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

///////////////////////////

===========================================================================*/
qboolean in90FOV (edict_t *self, edict_t *other);

edict_t* choose_target(edict_t* self)
{
	
	
	edict_t* entity =NULL;		// entity currently being considered
	edict_t* bestent=NULL;		// best candidate so far

	float	best_suitability = 0;	// bestent's suitability

	vec3_t	difference;		// vector between self and entities origin
	float 	distance;		// length of above vector

	

	while ((entity = G_Find (entity, FOFS(classname), "player")) != NULL)
	{	
		if (visible (self, entity))
			if (infront (self, entity))
				if(entity->client)//NOTE: probably unessecary? all humans have clients..
				if(entity->client->botinfo.invisible == false)//not invisible to bots
					if (entity->health > 0)
					{
						VectorSubtract (self->s.origin, entity->s.origin, difference); 
						// find a vector that runs from self origin to entity origin ( or vice.versa)
						
						distance = VectorLength(difference); // find it's length
						
						
						// A close bot with say distance 30 would have 
						// suitability 1/30 = 0.0333
						// A far bot with distance 200 would have
						// suitability 1/200 = 0.005 so closer bot is more suitable
						if (distance == 0) // avoid divide by 0
						{
							safe_bprintf (PRINT_HIGH,"possible enemy with distance == 0\n");
							bestent = entity;
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
								bestent = entity;	// chose this entity for now
								best_suitability = 1/distance; // use this entity's suitability
							}
						}
					}	
	} 

	return bestent;

}

/*===================================================================================
in90FOV
-------
returns true if other is within a 90 degree field of view of self
===================================================================================*/
qboolean in90FOV (edict_t *self, edict_t *other)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;
	
	AngleVectors (self->client->v_angle, forward, NULL, NULL);
	VectorSubtract (other->s.origin, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);

	//if(!self->client->botinfo.isbot)
		//safe_bprintf (PRINT_HIGH,"%s's dotproduct: %i\n", self->client->pers.netname, (int)(dot * 1000));
	
	if (dot > 0.67)
		return true;
	return false;
}



/*==========================================================================
choose_targetitem
----------------------------------------------------------------------------
chooses an item for the bot to lock onto with the same criteria as above.
==========================================================================*/
edict_t* choose_targetitem(edict_t* self)
{
	
	edict_t* candidate = NULL;
	edict_t* bestcandidate = NULL;						

	float	best_suitability = 0;

	vec3_t	difference;
	float 	distance;

	candidate = g_edicts; //start at beginning of edict_t pointer array

	while(candidate <= &g_edicts[globals.num_edicts])	// while the whole array has not been searched
	{
		if(candidate->item != NULL)	// if candidate points to an item
			if(candidate->solid != SOLID_NOT)//if item has respawned
				if (visible (self, candidate))
					if (infront (self, candidate))
					{
						VectorSubtract (self->s.origin, candidate->s.origin, difference); 
						// find a vector that runs from self origin to candidate origin ( or vice.versa)
						
						distance = VectorLength(difference); // find it's length						
							
						if (distance == 0) // avoid divide by 0
						{
							bestcandidate = candidate; // we have found the perfect item... probably means we can't get it.. 
							break;
						}
						else // else preceed as normal
						{						// Closer the target, the more suitable it is. ( roughly)
						// Equivalent to 
						//		suitability = 1/distance;
						//		if (suitability > best_suitability ) etc...
							if ( (1/distance) > best_suitability )
							{								
								bestcandidate = candidate;	// chose this entity for now
								best_suitability = 1/distance; // use this entity's suitability
							}
						}
					}
		candidate++; // point at next entity	
	} 

	return bestcandidate;
}








/* ===========================================================================
check_self_pain
---------------------------------------------------------------------------
returns 1 if bot took damage in the last 100 ms

  lastpaintime is set to leveltime in the bot's pain function

===========================================================================*/
float check_self_pain (edict_t* self)
{
		
	if  ( (self->client->botinfo.lastpaintime + 0.1) >= level.time )
		return 1.0;
	else
		return 0.0;
}

/* ===========================================================================
check_inflicted_pain
---------------------------------------------------------------------------
returns 1 if bot inflicted pain on anything in the last 100 ms

  lastpaintime is set to leveltime in the T_Damage() in g_combat.c if the bot inflicts pain

===========================================================================*/
float check_inflicted_pain(edict_t* self)
{
	
	if  ( (self->client->botinfo.lastinflictedpaintime + 0.1) >= level.time )
		return 1.0;
	else
		return 0.0;
}
/* ===========================================================================
check_inflicted_death
---------------------------------------------------------------------------
returns 1 if bot killed anything in the last 100 ms

  lastkilltime is set to leveltime in the T_Damage() in g_combat.c if the bot kills something
===========================================================================*/
float check_inflicted_death(edict_t* self)
{

		
	if  ( (self->client->botinfo.lastinflicteddeathtime + 0.1) >= level.time )
		return 1.0;
	else
		return 0.0;
}



/* ===========================================================================
check_just_fired
---------------------------------------------------------------------------
returns 1 if bot just fired
client->last_fire_time is set to level.time when bot fires in bot playanim_attack()

===========================================================================*/
float check_just_fired (edict_t* self)
{
		// if bot fired in the last 100 milliseconds
	if  ( (self->client->last_fire_time + 0.1) >= level.time )
		return 1.0;
	else
		return 0.0;
}





/* ===========================================================================
allwaysfire
---------------------------------------------------------------------------
allways returns 1
a bias neuron if you will
===========================================================================*/

float allwaysfire (edict_t* self)
{

	return 1.0; 
}


/* ===========================================================================
boxtraceahead_close
---------------------------------------------------------------------------
does a bounding box trace, returns 1 if bot could get xxx units ahead
along current view vector.
===========================================================================*/
float boxtraceahead_close (edict_t* self)
{
	//float y;
	vec3_t forward,end;
	trace_t trace_results;	// The structure that will contain the results
							// from the traceline function.
	vec3_t groundvec;

	groundvec[ROLL]		= 0;
	groundvec[YAW]		= self->client->v_angle[YAW];
	groundvec[PITCH]	= 0;

	AngleVectors (groundvec, forward, NULL, NULL);
	// make a forwards pointing vector out of the bot's view angles

	VectorMA (self->s.origin, 60, forward, end);
	// Make end equal to 60* the forward pointing vector

	trace_results = gi.trace (self->s.origin, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;

	//NOTE: make this pow(1 - trace_results.fraction, 0.5)?

}


/* ===========================================================================
boxtraceahead_far
---------------------------------------------------------------------------
does a bounding box trace, returns 1 if bot could get xxx units ahead
along current view vector.
===========================================================================*/
float boxtraceahead_far (edict_t* self)
{
	//float y;
	vec3_t forward,end;
	trace_t trace_results;	// The structure that will contain the results
							// from the traceline function.
	vec3_t groundvec;

	groundvec[ROLL]		= 0;
	groundvec[YAW]		= self->client->v_angle[YAW];
	groundvec[PITCH]	= 0;

	AngleVectors (groundvec, forward, NULL, NULL);
	// make a forwards pointing vector out of the bot's view angles

	VectorMA (self->s.origin, 200, forward, end);
	// Make end equal to 200* the forward pointing vector

	trace_results = gi.trace (self->s.origin, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;

}




#define LOOK_DIST 300
#define LOOKDIAG_DIST 300
#define LOOKDIAG_ANGLE 30
#define LOOKHALF_DIST 270
#define LOOKHALF_ANGLE 30

/* ===========================================================================
traceahead_lava;
---------------------------------------------------------------------------
returns 1 if there is lava just ahead.
===========================================================================*/

float traceahead_lava(edict_t* self)
{
	//float y;
	vec3_t	forward;// forwards poiting vector
	trace_t trace_results;	// The structure that will contain the results
							// from the traceline function.
	vec3_t  eyepos;	// eye position
	vec3_t	end;	// endpoint of the line



	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;
	// eye position = origin ( about middle of model i think) + viewheight

	AngleVectors (self->client->v_angle, forward, NULL, NULL);
	// make a forwards pointing vector out of the bot's view angles

	VectorMA (eyepos, LOOK_DIST, forward, end);
	// Make end equal to LOOK_DIST * the forward pointing vector, all offset
	// by the original eye position

	trace_results = gi.trace (eyepos, NULL, NULL, end, self, CONTENTS_LAVA );
	// Trace a line from the bot's eye position to end, ignoring
	// bot's own model, and only stopping when it hits a brush with 
	// CONTENTS_LAVA
	/*
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (eyepos);
	gi.WritePosition (trace_results.endpos);//client->botinfo.targetitem->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);
	*/
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;
}

/* ===========================================================================
traceahead_wall;
---------------------------------------------------------------------------
returns 1 if there is wall just ahead 

===========================================================================*/

float traceahead_wall(edict_t* self)
{
	float y;
	vec3_t	forward;// forwards poiting vector
	trace_t trace_results;	// The structure that will contain the results
							// form the traceline function.
	vec3_t  eyepos;	// eye position
	vec3_t	end;	// endpoint of the line

	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;
	// eye position = origin ( about middle of model i think) + viewheight

	AngleVectors (self->client->v_angle, forward, NULL, NULL);
	// make a forwards pointing vector out of the bot's view angles

	VectorMA (eyepos, LOOK_DIST, forward, end);
	// Make end equal to LOOK_DIST * the forward pointing vector, all offset
	// by the original eye position
	trace_results = gi.trace (eyepos, NULL, NULL, end, self, CONTENTS_SOLID ); 
	// Trace a line from the bot's eye position to end, ignoring
	// bot's own model, and only stopping when it hits a brush with 
	// CONTENTS_SOLID

	
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;
}
/* ===========================================================================
traceright_wall
---------------
returns 1 if there is a wall within LOOK_DIST units 90 degrees to the bot's right
===========================================================================*/
float traceright_wall(edict_t* self)
{
	float y;
	vec3_t	right;// right pointing vector
	trace_t trace_results;	// The structure that will contain the results
							// form the traceline function.
	vec3_t  eyepos;	// eye position
	vec3_t	end;	// endpoint of the line


	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;
	// eye position = origin ( about middle of model i think) + viewheight

	AngleVectors (self->client->v_angle, NULL, right, NULL);
	// make a right pointing vector out of the bot's view angles

	VectorMA (eyepos, LOOK_DIST, right, end);
	// Make end equal to LOOK_DIST * the right pointing vector, all offset
	// by the original eye position
	trace_results = gi.trace (eyepos, NULL, NULL, end, self, CONTENTS_SOLID ); 
	// Trace a line from the bot's eye position to end, ignoring
	// bot's own model, and only stopping when it hits a brush with 
	// CONTENTS_SOLID

	
	//gi.WriteByte (svc_temp_entity);
	//gi.WriteByte (TE_BFG_LASER);
	//gi.WritePosition (eyepos);
	//gi.WritePosition (trace_results.endpos);//client->botinfo.targetitem->s.origin);
	//gi.multicast (self->s.origin, MULTICAST_PHS);
	
	
	//if ( trace_results.fraction < 1.0 )// if trace hit a wall
	//	return 1;					// time to fire up neuron
	//else
	//	return 0;

	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;

}

/* ===========================================================================
traceleft_wall
---------------
returns 1 if there is a wall within LOOK_DIST units 90 degrees to the bot's left
===========================================================================*/
float traceleft_wall(edict_t* self)
{
	float y;
	vec3_t	right;// right pointing vector
	trace_t trace_results;	// The structure that will contain the results
							// form the traceline function.
	vec3_t  eyepos;	// eye position
	vec3_t	end;	// endpoint of the line

	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;
	// eye position = origin ( about middle of model i think) + viewheight

	AngleVectors (self->client->v_angle, NULL, right, NULL);
	// make a right pointing vector out of the bot's view angles

	VectorMA (eyepos, -1*LOOK_DIST, right, end);
	// Make end equal to -LOOK_DIST * the right pointing vector, all offset
	// by the original eye position
	trace_results = gi.trace (eyepos, NULL, NULL, end, self, CONTENTS_SOLID ); 
	// Trace a line from the bot's eye position to end, ignoring
	// bot's own model, and only stopping when it hits a brush with 
	// CONTENTS_SOLID

	
	//gi.WriteByte (svc_temp_entity);
	//gi.WriteByte (TE_BFG_LASER);
	//gi.WritePosition (eyepos);
	//gi.WritePosition (trace_results.endpos);//client->botinfo.targetitem->s.origin);
	//gi.multicast (self->s.origin, MULTICAST_PHS);
	

	
	//if ( trace_results.fraction < 1.0 )// if trace hit a wall
	//	return 1;					// time to fire up neuron
	//else
	//	return 0;
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;
}
/* ===========================================================================
traceahead_enemy;
---------------------------------------------------------------------------
returns 1 if thebot's enemy is exactly in front of the bot

===========================================================================*/
float traceahead_enemy(edict_t* self)
{


	vec3_t	forward;// forwards pointing vector
	trace_t trace_results;	// The structure that will contain the results
							// form the traceline function.
	vec3_t  eyepos;	// eye position
	vec3_t	end;	// endpoint of the line

	if (self->enemy == NULL)
		return 0.0;

	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;
	// eye position = origin ( about middle of model i think) + viewheight

	AngleVectors (self->client->v_angle, forward, NULL, NULL);
	// make a forwards pointing vector out of the bot's view angles

	VectorMA (eyepos, 8192, forward, end);
	// Make end equal to 8192 * the forward pointing vector, all offset
	// by the original eye position
	trace_results = gi.trace (eyepos, NULL, NULL, end, self, MASK_SHOT );
	// Trace a line from the bot's eye position to end, ignoring
	// bot's own model. traceline can be stopped by walls and entities

	if (trace_results.ent && trace_results.ent == self->enemy)// if trace hit enemy
	{
	//	safe_bprintf (PRINT_HIGH,"bing\n");
		return 1.0;					// time to fire up neuron
	}
	else
		return 0.0;

}
	
/* ===========================================================================
traceahead_entity;
---------------------------------------------------------------------------
just like above but will return 1 even if the entity is not the bot's enemy
even walls are entities so the criteria for returning one is that the thing
the trace hits must have health > 1
===========================================================================*/
float traceahead_entity(edict_t* self)
{

	vec3_t	forward;// forwards poiting vector
	trace_t trace_results;	// The structure that will contain the results
							// form the traceline function.
	vec3_t  eyepos;	// eye position
	vec3_t	end;	// endpoint of the line

	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;
	// eye position = origin ( about middle of model i think) + viewheight

	AngleVectors (self->client->v_angle, forward, NULL, NULL);
	// make a forwards pointing vector out of the bot's view angles

	VectorMA (eyepos, 8192, forward, end);
	// Make end equal to 8192 * the forward pointing vector, all offset
	// by the original eye position
	trace_results = gi.trace (eyepos, NULL, NULL, end, self, MASK_SHOT );
	// Trace a line from the bot's eye position to end, ignoring
	// bot's own model. traceline can be stopped by walls and entities

	if (trace_results.ent != NULL)// if trace hit an entity (could be wall)
	{
		if (trace_results.ent->health > 0)
		{
		//	safe_bprintf (PRINT_HIGH,"bing\n");
			return 1.0;					// time to fire up neuron
		}
	}
		
	return 0.0;

}
/* ===========================================================================
traceahead_wall;
---------------------------------------------------------------------------
returns 1 if there is water just ahead 

===========================================================================*/

float traceahead_water(edict_t* self)
{
	float y;
	vec3_t	forward;
	trace_t trace_results;	
						
	vec3_t  eyepos;	
	vec3_t	end;	

	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;

	AngleVectors (self->client->v_angle, forward, NULL, NULL);

	VectorMA (eyepos, LOOK_DIST, forward, end);
	// Make end equal to LOOK_DIST * the forward pointing vector, all offset
	// by the original eye position
	trace_results = gi.trace (eyepos, NULL, NULL, end, self, CONTENTS_WATER );
	// Trace a line from the bot's eye position to end, ignoring
	// bot's own model, and only stopping when it hits a brush with 
	// CONTENTS_WATER

	//if ( trace_results.fraction < 1.0 )// if trace hit water
	//	return 1;				
	//else
	//	return 0;
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;
}
	

/* ===========================================================================
tracediag_lava
---------------------------------------------------------------------------
this is just like traceahead, but this time the traceline heads diaginally 
down a bit. This would be the human equivalent of looking at the centre 
bottom of the screen while playing.

===========================================================================*/

float tracediag_lava(edict_t* self)
{
	float y;
	vec3_t	diag; // forwards and down a bit angle array
	vec3_t	direction; // vector poitning in same directions as diag
	trace_t trace_results;	// gi.trace return structure
						
	vec3_t  eyepos;		// bot's eye position
	vec3_t	end;		// end of the trace

	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;

	VectorCopy ( self->client->v_angle, diag );

	diag[0] = diag[0] + LOOKDIAG_ANGLE; // tilt down 40? degrees. 
										// NOTE: check this works

	AngleVectors (diag, direction, NULL, NULL);

	VectorMA (eyepos, LOOKDIAG_DIST, direction, end);
	
	trace_results = gi.trace (eyepos, NULL, NULL, end, self, CONTENTS_LAVA );
	/*
	gi.WriteByte (svc_temp_entity);//bfg laser for testing
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (eyepos);
	gi.WritePosition (trace_results.endpos);//client->botinfo.targetitem->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);
	*/	
	//if ( trace_results.fraction < 1.0 )
	//	return 1;				
	//else
	//	return 0;
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;
}


/* ===========================================================================
tracediag_wall
---------------------------------------------------------------------------
this is just like traceahead, but this time the traceline heads diaginally 
down a bit. This would be the human equivalent of looking at the centre 
bottom of the screen while playing.

===========================================================================*/

float tracediag_wall(edict_t* self)
{
	float y;
	vec3_t	diag; // forwards and down a bit angle array
	vec3_t	direction; // resulting vector
	trace_t trace_results;	
						
	vec3_t  eyepos;	
	vec3_t	end;	

	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;

	VectorCopy ( self->client->v_angle, diag );

	diag[0] = diag[0] + LOOKDIAG_ANGLE; // tilt down 40 degrees. 
							// NOTE: check this works

	AngleVectors (diag, direction, NULL, NULL);

	VectorMA (eyepos, LOOKDIAG_DIST, direction, end);
	
	trace_results = gi.trace (eyepos, NULL, NULL, end, self, CONTENTS_SOLID ); 
	
	//if ( trace_results.fraction < 1.0 )
	//	return 1;				
	//else
	//	return 0;
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;;
}


/* ===========================================================================
tracediag_water
---------------------------------------------------------------------------
this is just like traceahead, but this time the traceline heads diaginally 
down a bit. This would be the human equivalent of looking at the centre 
bottom of the screen while playing.

===========================================================================*/

float tracediag_water(edict_t* self)
{
	float y;
	vec3_t	diag; // forwards and down a bit angle array
	vec3_t	direction; // resulting vector
	trace_t trace_results;	
						
	vec3_t  eyepos;	
	vec3_t	end;	

	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;

	VectorCopy ( self->client->v_angle, diag );

	diag[0] = diag[0] + LOOKDIAG_ANGLE; // tilt down 40 degrees. 
							// NOTE: check this works

	AngleVectors (diag, direction, NULL, NULL);

	VectorMA (eyepos, LOOKDIAG_DIST, direction, end);
	
	trace_results = gi.trace (eyepos, NULL, NULL, end, self, CONTENTS_WATER ); 
	
	//if ( trace_results.fraction < 1.0 )
	//	return 1;				
	//else
	//	return 0;
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;
}

/* ===========================================================================
tracediag_entity
---------------------------------------------------------------------------
this is just like traceahead, but this time the traceline heads diaginally 
down a bit. This would be the human equivalent of looking at the centre 
bottom of the screen while playing.

Again a MASK_SHOT is used ( bot can't see through walls), 
8192 is used as the distance ( might be useful for bot railgunning plebs
a long way away ), 
and... yeah that's it actually
===========================================================================*/

float tracediag_entity(edict_t* self)
{

	vec3_t	diag; // forwards and down a bit angle array
	vec3_t	direction; // resulting vector
	trace_t trace_results;	
						
	vec3_t  eyepos;	
	vec3_t	end;	

	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;

	VectorCopy ( self->client->v_angle, diag );

	diag[0] = diag[0] + LOOKDIAG_ANGLE; // tilt down 40 degrees. 
							// NOTE: check this works

	AngleVectors (diag, direction, NULL, NULL);

	VectorMA (eyepos, 8192, direction, end);
	
	trace_results = gi.trace (eyepos, NULL, NULL, end, self, MASK_SHOT );
	
	//if (self->enemy == NULL)
	//	return 0;

	if (trace_results.ent != NULL)// if trace hit an entity
		return 1.0;					// time to fire up neuron
	else
		return 0.0;


}











/* ===========================================================================
tracehalfleft_wall
---------------------------------------------------------------------------
does a trace forwards and to the left, returns 1 if it hits anything.
roughly equivalent to looking at the left edge of your screen with a 90 degree FOV
and seeing if there is a wall there.
===========================================================================*/
float tracehalfleft_wall(edict_t* self)
{
	float y;
	vec3_t	halfleft; // forwards and left a bit angle 
	vec3_t	direction; 
	trace_t trace_results;	
						
	vec3_t  eyepos;	
	vec3_t	end;	

	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;

	VectorCopy (self->client->v_angle, halfleft);

	halfleft[YAW] = halfleft[YAW] + LOOKHALF_ANGLE;
	
	halfleft[PITCH] = 0;//make vector run parallel to ground plane					

	AngleVectors (halfleft, direction, NULL, NULL);

	VectorMA (eyepos, LOOKHALF_DIST, direction, end);
	

	trace_results = gi.trace (eyepos, NULL, NULL, end, self, CONTENTS_SOLID ); 

	//gi.WriteByte (svc_temp_entity);//bfg laser for testing
	//gi.WriteByte (TE_BFG_LASER);
	//gi.WritePosition (eyepos);
	//gi.WritePosition (trace_results.endpos);//client->botinfo.targetitem->s.origin);
	//gi.multicast (self->s.origin, MULTICAST_PHS);
	
	//if ( trace_results.fraction < 1.0 )
	//	return 1;				
	//else
	//	return 0;
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;
}











/* ===========================================================================
tracehalfright_wall
---------------------------------------------------------------------------

===========================================================================*/
float tracehalfright_wall(edict_t* self)
{
	float y;
	vec3_t	halfright; // forwards and right a bit angle 
	vec3_t	direction; 
	trace_t trace_results;	
						
	vec3_t  eyepos;	
	vec3_t	end;	

	VectorCopy (self->s.origin, eyepos );
	eyepos[2] += self->viewheight;

	VectorCopy (self->client->v_angle, halfright);

	halfright[YAW] = halfright[YAW] - LOOKHALF_ANGLE; 
						
	halfright[PITCH] = 0;//make vector run parallel to ground plane	

	AngleVectors (halfright, direction, NULL, NULL);

	VectorMA (eyepos, LOOKHALF_DIST, direction, end);
	
	trace_results = gi.trace (eyepos, NULL, NULL, end, self, CONTENTS_SOLID ); 

	//gi.WriteByte (svc_temp_entity);//bfg laser for testing
	//gi.WriteByte (TE_BFG_LASER);
	//gi.WritePosition (eyepos);
	//gi.WritePosition (trace_results.endpos);//client->botinfo.targetitem->s.origin);
	//gi.multicast (self->s.origin, MULTICAST_PHS);

	
	//if ( trace_results.fraction < 1.0 )
	//	return 1;				
	//else
	//	return 0;	
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;
}













/* ===========================================================================
boxtraceleft_wall
---------------------------------------------------------------------------
traces a bounding box the size of a bot 90 degrees to the left of where the bot
is looking, returns 1 if the trace is blocked.
===========================================================================*/
float boxtraceleft_wall(edict_t* self)
{
	float y;
	vec3_t left;
	vec3_t end;
	trace_t trace_results;	// The structure that will contain the results
							// from the traceline function.
	vec3_t groundvec;

	groundvec[ROLL]		= 0;
	groundvec[YAW]		= self->client->v_angle[YAW];
	groundvec[PITCH]	= 0;


	AngleVectors (groundvec, NULL, left, NULL);
	// make a right pointing vector out of the bot's view angles
	VectorInverse(left);	//turn right pointing vector into left pointing vector

	VectorMA (self->s.origin, 60, left, end);
	// Make end equal to self origin + 60 * the left pointing vector

	trace_results = gi.trace (self->s.origin, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

	//if ( trace_results.fraction < 1.0 )// if trace hit something
	//{
	//	return 1;	
	//}
	//else
	//	return 0;
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;
}







float boxtraceright_wall(edict_t* self)
{
	float y;
	vec3_t right;
	vec3_t end;
	trace_t trace_results;	// The structure that will contain the results
							// from the traceline function.
	vec3_t groundvec;

	groundvec[ROLL]		= 0;
	groundvec[YAW]		= self->client->v_angle[YAW];
	groundvec[PITCH]	= 0;


	AngleVectors (groundvec, NULL, right, NULL);

	VectorMA (self->s.origin, 60, right, end);

	trace_results = gi.trace (self->s.origin, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

	//if ( trace_results.fraction < 1.0 )// if trace hit something
	//{
	//	return 1;	
	//}
	//else
	//	return 0;
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;

}








float boxtracehalfleft_wall(edict_t* self)
{
	float y;
	vec3_t	halfleft; // forwards and left a bit angle 
	vec3_t	direction; //vector pointing forwards and left a bit
	trace_t trace_results;	
						
	vec3_t	end;
	
	vec3_t groundvec;

	groundvec[ROLL]		= 0;
	groundvec[YAW]		= self->client->v_angle[YAW];
	groundvec[PITCH]	= 0;


	VectorCopy (groundvec, halfleft);

	halfleft[YAW] = halfleft[YAW] + LOOKHALF_ANGLE; 
						

	AngleVectors (halfleft, direction, NULL, NULL);

	VectorMA (self->s.origin, LOOKHALF_DIST, direction, end);

	trace_results = gi.trace (self->s.origin, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

	//if ( trace_results.fraction < 1.0 )// if trace hit something
	//{
	//	return 1;	
	//}
	//else
	//	return 0;
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;
}

float boxtracehalfright_wall(edict_t* self)
{
	float y;
	vec3_t	halfright; // forwards and right a bit angle 
	vec3_t	direction; //vector pointing forwards and right a bit
	trace_t trace_results;	
						
	vec3_t	end;
	
	vec3_t groundvec;

	groundvec[ROLL]		= 0;
	groundvec[YAW]		= self->client->v_angle[YAW];
	groundvec[PITCH]	= 0;


	VectorCopy (groundvec, halfright);

	halfright[YAW] = halfright[YAW] - LOOKHALF_ANGLE; 
						

	AngleVectors (halfright, direction, NULL, NULL);

	VectorMA (self->s.origin, LOOKHALF_DIST, direction, end);

	trace_results = gi.trace (self->s.origin, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

	//if ( trace_results.fraction < 1.0 )// if trace hit something
	//{
	//	return 1;	
	//}
	//else
	//	return 0;
	//y = ACTIVATION_NUMERATOR / trace_results.fraction;
	//if(y > 1)
		//y = 1;

	return 1 - trace_results.fraction;
}





















/* ===========================================================================
enemy_jump
---------------------------------------------------------------------------
checks if bot's enemy has jumped in the last 100ms.
If the bot doesn't have an enemy the function returns 0.

===========================================================================*/

float enemy_jump (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if ( (self->enemy->client->last_jump_time + 0.1) >= level.time )
		return 1.0;
	else
		return 0.0;
}

/* ===========================================================================
enemy_duck
---------------------------------------------------------------------------
checks if bot's enemy has ducked in the last 100ms.
If the bot doesn't have an enemy the function returns 0.

===========================================================================*/

float enemy_duck (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if ( (self->enemy->client->last_duck_time + 0.1) >= level.time )
		return 1.0;
	else
		return 0.0;
}



/* ===========================================================================
check_f_blaster

---------------------------------------------------------------------------
checks if bot's enemy has fired their blaster in the last 100ms.
If the bot doesn't have an enemy the function returns 0.

NOTE: this doesn't strictly check if enemy has fire blaster in last 0.1 secs, but
checks if they have fired in the last 0.1 secs and are holding a blaster.
Shouldn't matter though.
===========================================================================*/

float check_f_blaster (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if(self->enemy->client->pers.weapon == NULL)//if enemy doesn't have a weapon
		return 0.0;

	if ( ( (self->enemy->client->last_fire_time + 0.1) >= level.time )
		&& ( self->enemy->client->pers.weapon->weapmodel == WEAP_BLASTER ))
		return 1.0;
	else
		return 0.0;
}


/* ===========================================================================
check_f_shotgun

---------------------------------------------------------------------------
checks if bot's enemy has fired their shotgun in the last 100ms.
If the bot doesn't have an enemy the function returns 0.

===========================================================================*/

float check_f_shotgun (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if(self->enemy->client->pers.weapon == NULL)
		return 0.0;

	if (( (self->enemy->client->last_fire_time + 0.1) >= level.time )
		&& ( self->enemy->client->pers.weapon->weapmodel == WEAP_SHOTGUN ))
		return 1.0;
	else
		return 0.0;
}

/* ===========================================================================
check_f_sshotgun

---------------------------------------------------------------------------
checks if bot's enemy has fired their super shotgun in the last 100ms.
If the bot doesn't have an enemy the function returns 0.

===========================================================================*/

float check_f_sshotgun (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if(self->enemy->client->pers.weapon == NULL)
		return 0.0;

	if (( (self->enemy->client->last_fire_time + 0.1) >= level.time )
		&& ( self->enemy->client->pers.weapon->weapmodel == WEAP_SUPERSHOTGUN ))
		return 1.0;
	else
		return 0.0;
}


/* ===========================================================================
check_f_mgun

---------------------------------------------------------------------------
checks if bot's enemy has fired their machinegun in the last 100ms.
If the bot doesn't have an enemy the function returns 0.

===========================================================================*/

float check_f_mgun (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if(self->enemy->client->pers.weapon == NULL)
		return 0.0;

	if (( (self->enemy->client->last_fire_time + 0.1) >= level.time )
		&& ( self->enemy->client->pers.weapon->weapmodel == WEAP_MACHINEGUN ))
		return 1.0;
	else
		return 0.0;
}




float check_f_chaingun (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if(self->enemy->client->pers.weapon == NULL)
		return 0.0;

	if (( (self->enemy->client->last_fire_time + 0.1) >= level.time ) 
		&& ( self->enemy->client->pers.weapon->weapmodel == WEAP_CHAINGUN ))
		return 1.0;
	else
		return 0.0;
}



float check_f_grenadel (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if(self->enemy->client->pers.weapon == NULL)
		return 0.0;

	if (( (self->enemy->client->last_fire_time + 0.1) >= level.time )
		&& ( self->enemy->client->pers.weapon->weapmodel == WEAP_GRENADELAUNCHER ))
		return 1.0;
	else
		return 0.0;
}



float check_f_rocketl (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if(self->enemy->client->pers.weapon == NULL)
		return 0.0;

	if (( (self->enemy->client->last_fire_time + 0.1) >= level.time )
		&& ( self->enemy->client->pers.weapon->weapmodel == WEAP_ROCKETLAUNCHER ))
		return 1.0;
	else
		return 0.0;
}



float check_f_hyperb (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if(self->enemy->client->pers.weapon == NULL)
		return 0.0;

	if (( (self->enemy->client->last_fire_time + 0.1) >= level.time )
		&& ( self->enemy->client->pers.weapon->weapmodel == WEAP_HYPERBLASTER ))
		return 1.0;
	else
		return 0.0;
}



float check_f_railgun (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if(self->enemy->client->pers.weapon == NULL)
		return 0.0;

	if (( (self->enemy->client->last_fire_time + 0.1) >= level.time )
		&& ( self->enemy->client->pers.weapon->weapmodel == WEAP_RAILGUN ))
		return 1.0;
	else
		return 0.0;
}



float check_f_bfg10k (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if(self->enemy->client->pers.weapon == NULL)
		return 0.0;

	if (( (self->enemy->client->last_fire_time + 0.1) >= level.time )
		&& ( self->enemy->client->pers.weapon->weapmodel == WEAP_BFG ))
		return 1.0;
	else
		return 0.0;
}

/* ===========================================================================
check_f_grenade

---------------------------------------------------------------------------
checks if bot's enemy has chucked a hand grenade in the last 100ms.
If the bot doesn't have an enemy the function returns 0.

===========================================================================*/

float check_f_grenade (edict_t* self)
{
	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 0.0;

	if(self->enemy->client->pers.weapon == NULL)
		return 0.0;

	if (( (self->enemy->client->last_fire_time + 0.1) >= level.time )
		&& ( self->enemy->client->pers.weapon->weapmodel == WEAP_GRENADES ))
		return 1.0;
	else
		return 0.0;
}




/* ===========================================================================
check_far_left
---------------------------------------------------------------------------

===========================================================================*/

float check_far_left (edict_t* self, float difference)
{

	if ( ( difference < -25 )&&(difference >= -90) ) //uuuuuugh horribly low resolution
	{
		//safe_bprintf (PRINT_HIGH,"check_far_left\n");
		return 1.0;
	}
	else
		return 0.0;	

}

/* ===========================================================================
check_far_right
---------------------------------------------------------------------------
===========================================================================*/

float check_far_right (edict_t* self, float difference)
{

	if ( ( difference > 25 )&&(difference <= 90) ) 
	{
		//safe_bprintf (PRINT_HIGH, "check_far_right\n");
		return 1.0;
	}
	else
		return 0.0;	

}



/* ===========================================================================
check_at_left
---------------------------------------------------------------------------
This function checks to see if the bot's target is just to the left of the 
centre of the bot's 'screen'.


  P.S the 4 and 25 are somewhat arbitrary. This could do with some experimentation.

===========================================================================*/

float check_at_left (edict_t* self, float difference)
{

	if ( ( difference < -4 )&&(difference >= -25) ) //uuuuuugh horribly low resolution
	{
		//safe_bprintf (PRINT_HIGH,"check_at_left\n");
		return 1.0;
	}
	else
		return 0.0;	

}

/* ===========================================================================
check_at_right
---------------------------------------------------------------------------
same as above, but swap all instances of left for right
===========================================================================*/

float check_at_right (edict_t* self, float difference)
{

	if ( ( difference > 4 )&&(difference <= 25) ) 
	{
		//safe_bprintf (PRINT_HIGH,"check_at_right\n");
		return 1.0;
	}
	else
		return 0.0;	

}

/* ===========================================================================
check_justat_left
---------------------------------------------------------------------------
This function checks to see if the bot's target is just to the left of the 
centre of the bot's 'screen'.

===========================================================================*/

float check_justat_left (edict_t* self, float difference)
{

	if ( ( difference < -1 )&&(difference >= -4) ) //uuuuuugh horribly low resolution
	{
		//safe_bprintf (PRINT_HIGH,"check_justat_left\n");
		return 1.0;
	}
	else
		return 0.0;	

}
/* ===========================================================================
check_justat_right
---------------------------------------------------------------------------

===========================================================================*/

float check_justat_right (edict_t* self, float difference)
{

	if ( ( difference > 1 )&&(difference <= 4) ) 
	{
		//safe_bprintf (PRINT_HIGH,"check_justat_right\n");
		return 1.0;
	}
	else
		return 0.0;	

}

/* ===========================================================================
check_ahead_enemy
---------------------------------------------------------------------------
checks if enemy is +- 1 degree from the center of view
===========================================================================*/
float check_ahead_enemy (edict_t* self, float difference)
{

	if ( ( difference >=-1 )&&(difference <= 1) ) 
	{
		//safe_bprintf (PRINT_HIGH,"check_ahead_enemy\n");
		return 1.0;
	}
	else
		return 0.0;
}




/* ===========================================================================
check_feet
---------------------------------------------------------------------------
checks if the bot's feet are in water

waterlevel        int               height of liquid : 
                                    0 - not in liquid
                                    1 - feet in liquid
                                    2 - on surface of liquid
                                    3 - surrounded by liquid

===========================================================================*/

float check_feet (edict_t* self)

{
	if ( self->waterlevel == 1 )
		return 1.0;
	else 
		return 0.0;
}

/* ===========================================================================
check_waist
---------------------------------------------------------------------------
checks if the bot's waist is in the water

  I presume this is waterlevel 2

===========================================================================*/

float check_waist (edict_t* self)

{
	if ( self->waterlevel == 2 )
		return 1.0;
	else 
		return 0.0;
}

/* ===========================================================================
check_head
----------
checks if the bot is totally in the water
===========================================================================*/

float check_head (edict_t* self)

{
	if ( self->waterlevel == 3 )
		return 1.0;
	else 
		return 0.0;
}


/*==========================================================================================
check_item_xxxxxxxxxxxxxxxx
---------------------------
More sight simulation functions.  These functions tell the bot whereabouts in it's field of view
the item it is locked onto is.
==========================================================================================*/
float check_item_justatleft(edict_t* self, float difference)
{

	if ( ( difference < -4 )&&(difference >= -25) ) 
		return 1.0;
	else
		return 0.0;	

}
float check_item_justatright(edict_t* self, float difference)
{
	if ( ( difference > 4 )&&(difference <= 25) ) 
		return 1.0;
	else
		return 0.0;	

}
float check_item_atleft(edict_t* self, float difference)
{

	if ( ( difference < -25 )&&(difference >= -90) )
		return 1.0;
	else
		return 0.0;	

}
float check_item_atright(edict_t* self, float difference)
{

	if ( ( difference > 25 )&&(difference <= 90) ) 
		return 1.0;
	else
		return 0.0;	

}

float check_item_ahead(edict_t* self, float difference)
{

	if ( ( difference >= -4 )&&(difference <= 4) ) 
		return 1.0;
	else
		return 0.0;	

}


/*===========================================================================================
holding_xxxx
------------

input functions that return 1 if the calling bot is holding a specific weapon.
self->client->pers.weapon is a pointer to the weapon that the bot is holding.
===========================================================================================*/
float holding_blaster(edict_t* self)
{
	if(self->client->pers.weapon == NULL)
		return 0.0;

	if(self->client->pers.weapon->weapmodel == WEAP_BLASTER)
		return 1.0;
	else
		return 0.0;
}
float holding_shotgun(edict_t* self)
{
	if(self->client->pers.weapon == NULL)
		return 0.0;

	if(self->client->pers.weapon->weapmodel == WEAP_SHOTGUN)
		return 1.0;
	else
		return 0.0;
}
float holding_sshotgun(edict_t* self)
{
	if(self->client->pers.weapon == NULL)
		return 0.0;

	if(self->client->pers.weapon->weapmodel == WEAP_SUPERSHOTGUN)
		return 1.0;
	else
		return 0.0;
}
float holding_mgun(edict_t* self)
{
	if(self->client->pers.weapon == NULL)
		return 0.0;

	if(self->client->pers.weapon->weapmodel == WEAP_MACHINEGUN)
		return 1.0;
	else
		return 0.0;
}
float holding_chaingun(edict_t* self)
{
	if(self->client->pers.weapon == NULL)
		return 0.0;

	if(self->client->pers.weapon->weapmodel == WEAP_CHAINGUN)
		return 1.0;
	else
		return 0.0;
}
float holding_grenadel(edict_t* self)
{
	if(self->client->pers.weapon == NULL)
		return 0.0;

	if(self->client->pers.weapon->weapmodel == WEAP_GRENADELAUNCHER)
		return 1.0;
	else
		return 0.0;
}
float holding_rocketl(edict_t* self)
{
	if(self->client->pers.weapon == NULL)
		return 0.0;

	if(self->client->pers.weapon->weapmodel == WEAP_ROCKETLAUNCHER)
		return 1.0;
	else
		return 0.0;
}
float holding_hyperb(edict_t* self)
{
	if(self->client->pers.weapon == NULL)
		return 0.0;

	if(self->client->pers.weapon->weapmodel == WEAP_HYPERBLASTER)
		return 1.0;
	else
		return 0.0;
}
float holding_railgun(edict_t* self)
{
	if(self->client->pers.weapon == NULL)
		return 0.0;

	if(self->client->pers.weapon->weapmodel == WEAP_RAILGUN)
		return 1.0;
	else
		return 0.0;
}
float holding_bfg10k(edict_t* self)
{
	if(self->client->pers.weapon == NULL)
		return 0.0;

	if(self->client->pers.weapon->weapmodel == WEAP_BFG)
		return 1.0;
	else
		return 0.0;
}
float holding_grenade(edict_t* self)
{
	if(self->client->pers.weapon == NULL)
		return 0.0;

	if(self->client->pers.weapon->weapmodel == WEAP_GRENADES)
		return 1.0;
	else
		return 0.0;
}
/*=======================================================================
was_blocked
-----------
returns true if the bot hit an obstacle and could not make the move
=======================================================================*/
float was_blocked(edict_t* self)
{

	if(self->client->botinfo.wasblocked == true)
	{
		//safe_bprintf (PRINT_HIGH,"was_blocked:blocked\n");
		return 1.0;
	}
	else
		return 0.0;
}

/*=======================================================================
health_low
----------
returns true if the bot has health <= 25

NOTE: work out some continous sloped function here?
=======================================================================*/
float health_low(edict_t* self)
{
	if(self->health <= 25)
		return 1.0;
	else
		return 0.0;
}

/*=======================================================================
health_medium
-------------
returns true if the bot has health >25 and <=75
=======================================================================*/
float health_medium(edict_t* self)
{
	if(self->health > 25 && self->health <=75)
		return 1.0;
	else
		return 0.0;
}

/*=======================================================================
check_enemy_close
-----------------
=======================================================================*/
float check_enemy_close(edict_t* self)
{
	float y;	
	vec3_t	difference;		// vector between self and entities origin
	
	if(self->enemy == NULL) //if don't have an enemy
		return 0.0;

	VectorSubtract (self->s.origin, self->enemy->s.origin, difference); 
						// find a vector that runs from self origin to enemy origin ( or vice.versa)					
						
	y = 50 / VectorLength(difference);
	if(y > 1)
		y = 1;

	return y;


}




float check_item_close(edict_t* self)
{
	float y;	
	vec3_t	difference;		// vector between self and entities origin
	
	if(self->client->botinfo.targetitem == NULL)
		return 0.0;

	VectorSubtract (self->s.origin, self->client->botinfo.targetitem->s.origin, difference); 
						// find a vector that runs from self origin to targetitem origin ( or vice.versa)											

	y = 50 / VectorLength(difference);
	if(y > 1)
		y = 1;

	return y;
}


float enemy_moving_left(edict_t* self, float heading)
{
	float dyaw;//change in heading from self to enemy since last server frame

	if(heading == 666)//sentinel value for no enemy :)
		return 0.0;

	dyaw = self->client->botinfo.oldEnemyHeading - heading;

	dyaw /= 20;

	if(dyaw > 1)
		dyaw = 1;
	if(dyaw < 0)
		dyaw = 0;

	return dyaw;
}


float enemy_moving_right(edict_t* self, float heading)
{
	float dyaw;

	if(heading == 666)
		return 0.0;

	dyaw = heading - self->client->botinfo.oldEnemyHeading; //get negative dyaw

	dyaw /= 20;

	if(dyaw > 1)
		dyaw = 1;
	if(dyaw < 0)
		dyaw = 0;

	return dyaw;
}




/*==============================================================================================
input functions for enemies' pitch relative to where bot is looking
==============================================================================================*/
float enemypitch_near0(edict_t* self, float pitchdif)
{
	if(pitchdif >= -2 && pitchdif <= 2)
		return 1.0;
	else
		return 0.0;
}


float enemyjust_up(edict_t* self, float pitchdif)
{
	if(pitchdif >= -5 && pitchdif < 2)
		return 1.0;
	else
		return 0.0;
}
float enemyjust_down(edict_t* self, float pitchdif)
{
	if(pitchdif > 2 && pitchdif <= 5)
		return 1.0;
	else
		return 0.0;
}



float enemy_up(edict_t* self, float pitchdif)
{
	if(pitchdif >= -30 && pitchdif < -5)
		return 1.0;
	else
		return 0.0;
}
float enemy_down(edict_t* self, float pitchdif)
{
	if(pitchdif > 5 && pitchdif <= 30)
		return 1.0;
	else
		return 0.0;
}



float enemylots_up(edict_t* self, float pitchdif)
{
	if(pitchdif >= -90 && pitchdif < -30)
		return 1.0;
	else
		return 0.0;
}
float enemylots_down(edict_t* self, float pitchdif)
{
	if(pitchdif > 30  && pitchdif <= 90)
		return 1.0;
	else
		return 0.0;
}





/*=======================================================================
calcEnemyYawDif
-------------
returns the difference in yaw from the line of sight to ray to enemy.
negative num = enemy to the left
if enemy==null, returns 666
=======================================================================*/
float calcEnemyYawDif(edict_t* self)
{
	
	float	difference;		// Difference in terms of yaw between the facing
							// of the bot and the line to it's target

	vec3_t	vector_to_target;
	vec3_t	eye_pos;		// bot eye position

	VectorCopy (self->s.origin, eye_pos );
	eye_pos[2] += self->viewheight;

	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 666;

	VectorSubtract (self->enemy->s.origin, eye_pos, vector_to_target);
		// find vector_to_target
	
	difference = self->client->v_angle[1] - vectoyaw(vector_to_target);
		

	if (difference < -180)
		difference += 360;
	else if (difference > 180)
		difference -= 360;

	return difference;
}






float calcItemYawDif(edict_t* self)
{
	
	float	difference;		// Difference in terms of yaw between the facing
							// of the bot and the line to it's target

	vec3_t	vector_to_target;
	vec3_t	eye_pos;		// bot eye position

	VectorCopy (self->s.origin, eye_pos );
	eye_pos[2] += self->viewheight;

	if ( self->client->botinfo.targetitem == NULL ) // if choose_target couldn't find an enemy
		return 666;

	VectorSubtract (self->client->botinfo.targetitem->s.origin, eye_pos, vector_to_target);
		// find vector_to_target
	
	difference = self->client->v_angle[1] - vectoyaw(vector_to_target);
		
	//if(difference > 180)//if we took the long way round the circle
		//difference = 360 - difference;

	if (difference < -180)
		difference += 360;
	else if (difference > 180)
		difference -= 360;

	//safe_bprintf (PRINT_HIGH,"item difference: %i\n", difference);

	return difference;
}


float calcEnemyPitchDif(edict_t* self)
{
	
	float	difference;		// Difference in terms of pitch between the facing
							// of the bot and the line to it's target

	vec3_t	vector_to_target;
	vec3_t	angles_to_target;
	vec3_t	eye_pos;		// bot eye position

	VectorCopy (self->s.origin, eye_pos );
	eye_pos[2] += self->viewheight;

	if ( self->enemy == NULL ) // if choose_target couldn't find an enemy
		return 666;

	VectorSubtract (self->enemy->s.origin, eye_pos, vector_to_target);
		// find vector_to_target
		
	vectoangles (vector_to_target, angles_to_target);//overkill?

	difference = self->client->v_angle[PITCH] - angles_to_target[PITCH];

	if (difference < -180)
		difference += 360;
	else if (difference > 180)
		difference -= 360;

	return difference;
}