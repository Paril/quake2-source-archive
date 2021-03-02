#include "g_local.h"

qboolean CheckBounds (edict_t *owner, int rangeup, int rangeforward, int cost)
{
	vec3_t		forward,
				wallp;

	trace_t		tr;
	gitem_t		*item;
//        edict_t *self;

        if (!Q_stricmp(owner->classname, "doomspawn"))
        if ((!owner->client) || (owner->health<=0))
           return false;

	// Setup "little look" to close wall
        VectorCopy(owner->s.origin,wallp);         

	// Cast along view angle
        AngleVectors (owner->client->v_angle, forward, NULL, NULL);

	// Setup end point
        wallp[0]=owner->s.origin[0]+forward[0]*rangeforward;
        wallp[1]=owner->s.origin[1]+forward[1]*rangeforward;
        wallp[2]=owner->s.origin[2]+forward[2]*rangeforward;  

	// trace
        tr = gi.trace (owner->s.origin, NULL, NULL, wallp, owner, MASK_SOLID);

	// Line complete ? (ie. no collision)
        if (tr.fraction != 1.0)
	{
                gi.cprintf (owner, PRINT_HIGH, "Too close to wall.\n");
        item = FindItem("Cells");
        //owner->client->pers.selected_item = ITEM_INDEX(item);
        owner->client->pers.inventory[ITEM_INDEX(item)] = owner->client->pers.inventory[ITEM_INDEX(item)] + cost;
                return false;
	}

	// Hit sky ?
	if (tr.surface)
		if (tr.surface->flags & SURF_SKY)
                        return false;


	// Setup "little look" to close wall
        VectorCopy(owner->s.origin,wallp);         

	// Cast along view angle
        AngleVectors (owner->client->v_angle, forward, NULL, NULL);

        //check upward

        wallp[2]=owner->s.origin[2]+rangeup;

        tr = gi.trace (owner->s.origin, vec3_origin, vec3_origin, wallp, owner, MASK_SOLID);

	// Line complete ? (ie. no collision)
        if (tr.fraction != 1.0)
	{
                gi.cprintf (owner, PRINT_HIGH, "Too close to ceiling.\n");
        item = FindItem("Cells");
       // owner->client->pers.selected_item = ITEM_INDEX(item);
        owner->client->pers.inventory[ITEM_INDEX(item)] = owner->client->pers.inventory[ITEM_INDEX(item)] + cost;
                return false;
	}

        //check downward

	// Setup "little look" to close wall
        VectorCopy(owner->s.origin,wallp);         

	// Cast along view angle
        AngleVectors (owner->client->v_angle, forward, NULL, NULL);


        wallp[2]=owner->s.origin[2] - 16;

        tr = gi.trace (owner->s.origin, vec3_origin, vec3_origin, wallp, owner, MASK_SOLID);

	// Line complete ? (ie. no collision)
        if (tr.fraction != 1.0)
	{
                gi.cprintf (owner, PRINT_HIGH, "Too close to floor.\n");
        item = FindItem("Cells");
       // owner->client->pers.selected_item = ITEM_INDEX(item);
        owner->client->pers.inventory[ITEM_INDEX(item)] = owner->client->pers.inventory[ITEM_INDEX(item)] + cost;
                return false;
	}
     


      //it's ok to spawn it now

      return true;

}
