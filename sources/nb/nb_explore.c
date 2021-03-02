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

/*=========================================================================
exploreThink
------------
this should be called for each entity whose explorenss will be measured
every frame
=========================================================================*/
void exploreThink(edict_t* self, exploreinfo_t* einfo)
{
	if(level.time > einfo->nextwaypointdroptime)
	{
		einfo->nextwaypointdroptime = level.time + einfo->waypointdropperiod;
		dropWaypoint(self);
	}
}


/*=========================================================================
getExploreness
--------------
returns a float which is how well the bot is exploring at this instant
=========================================================================*/
float getExploreness(edict_t* self)
{
	exploreinfo_t* einfo = &self->client->botinfo.exploreinfo;//shortcut
	float exploreness = 0;
	float distanceweight = 0.000001;
	float outofsightweight = 0.0001;

	int i;
	float distance;
	vec3_t difference;
	trace_t trace;

	for(i=0; i<einfo->NUM_WAYPOINTS; i++)
	{
		
		//VectorSubtract(einfo->waypoints[i], self->s.origin, difference);	

		VectorSubtract(einfo->waypoints[i], self->s.origin, difference);
		distance = VectorLength(difference);

		exploreness += distance * distanceweight;


		trace = gi.trace (self->s.origin, NULL, NULL, einfo->waypoints[i], self, MASK_OPAQUE);
	
		if (trace.fraction != 1.0) //if trace was interrupted
			exploreness += outofsightweight;
		
	}
	return exploreness;
}


void initializeExploreinfo(edict_t* self, exploreinfo_t* ex)
{
	int i;

	ex->NUM_WAYPOINTS = 5;	
	ex->index = 0;
	ex->waypointdropperiod = 2.0;
	ex->nextwaypointdroptime = level.time + ex->waypointdropperiod;

	for(i=0; i<ex->NUM_WAYPOINTS; i++)
		VectorCopy(self->s.origin, ex->waypoints[i]);	//just put waypoints at player origin

	//safe_bprintf (PRINT_HIGH,"waypoints initialized\n");

}

int getIndex(exploreinfo_t* einfo)
{
	einfo->index = (einfo->index + 1) % einfo->NUM_WAYPOINTS;

	//safe_bprintf (PRINT_HIGH,"einfo->index: %i\n", einfo->index);
	return einfo->index;
}

void dropWaypoint(edict_t* self)
{
	exploreinfo_t* einfo = &self->client->botinfo.exploreinfo;
	int index = getIndex(einfo);

	VectorCopy(self->s.origin, einfo->waypoints[index] );

	//safe_bprintf (PRINT_HIGH,"dropped waypoint\n");

}