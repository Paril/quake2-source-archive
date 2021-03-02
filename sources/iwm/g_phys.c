/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// g_phys.c

#include "g_local.h"

/*


pushmove objects do not obey gravity, and do not interact with each other or trigger fields, but block normal movement and push normal objects when they move.

onground is set for toss objects when they come to a complete rest.  it is set for steping or walking objects 

doors, plats, etc are SOLID_BSP, and MOVETYPE_PUSH
bonus items are SOLID_TRIGGER touch, and MOVETYPE_TOSS
corpses are SOLID_NOT and MOVETYPE_TOSS
crates are SOLID_BBOX and MOVETYPE_TOSS
walking monsters are SOLID_SLIDEBOX and MOVETYPE_STEP
flying/floating monsters are SOLID_SLIDEBOX and MOVETYPE_FLY

solid_edge items only clip against bsp models.

*/


/*
============
SV_TestEntityPosition

============
*/
edict_t	*SV_TestEntityPosition (edict_t *ent)
{
	trace_t	trace;
	int		mask;

	if (ent->clipmask)
		mask = ent->clipmask;
	else
		mask = MASK_SOLID;
	trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, ent->s.origin, ent, mask);
	
	if (trace.startsolid)
		return g_edicts;
		
	return NULL;
}


/*
================
SV_CheckVelocity
================
*/
void SV_CheckVelocity (edict_t *ent)
{
	// Lazarus: This is a pretty goofy way to bound velocity. This has the effect
	//          of changing directions, which makes no sense at all. Maybe they
	//          were just avoiding a sqrt?
/*
	int		i;

//
// bound velocity
//
	for (i=0 ; i<3 ; i++)
	{
		if (ent->velocity[i] > sv_maxvelocity->value)
			ent->velocity[i] = sv_maxvelocity->value;
		else if (ent->velocity[i] < -sv_maxvelocity->value)
			ent->velocity[i] = -sv_maxvelocity->value;
	} */
	if (VectorLength(ent->velocity) > sv_maxvelocity->value)
	{
		VectorNormalize(ent->velocity);
		VectorScale(ent->velocity, sv_maxvelocity->value, ent->velocity);
	}
}

void RunTheThink (edict_t *ent, int num)
{
	float	thinktime;
	float t = 0;

	if (num == 0)
		t = ent->nextthink;
	else if (num == 1)
		t = ent->nextthink2;
	else if (num == 2)
		t = ent->nextthink3;
	else if (num == 3)
		t = ent->nextthink4;

	thinktime = ent->nextthinks[num];
	if (thinktime <= 0)
		return;
	if (thinktime > level.time+0.001)
		return;
	
	ent->nextthinks[num] = 0;
	if (!ent->thinks[num])
	{
		gi.dprintf ("========================================\n");
		// Paril: No more erring on null thinks
		gi.dprintf ("Warning: Entity has a NULL thinks[%i]: %s", num, ent->classname);
		if (ent->owner)
		{
			if (ent->owner->client)
				gi.dprintf (", owner is %s\n", ent->owner->client->pers.netname);
			else
				gi.dprintf (", owner is %s\n", ent->classname);
		}
		gi.dprintf ("Something might not behave correctly!\n");
		gi.dprintf ("========================================\n");
		return;
	}
	ent->thinks[num] (ent);
	//if (t > -1)
	//{
	//	ent->nextthinks[num] = t;
	//}
}

// New thinkings
void RunThinks (edict_t *ent)
{
	int i = 0;

	// Remove old-style thinks.
	if (ent->think)
	{
		ent->thinks[0] = ent->think;
		ent->nextthinks[0] = ent->nextthink;
		ent->think = NULL;
	}
	if (ent->think2)
	{
		ent->thinks[1] = ent->think2;
		ent->nextthinks[1] = ent->nextthink2;
		ent->think2 = NULL;
	}
	if (ent->think3)
	{
		ent->thinks[2] = ent->think3;
		ent->nextthinks[2] = ent->nextthink3;
		ent->think3 = NULL;
	}
	if (ent->think4)
	{
		ent->thinks[3] = ent->think4;
		ent->nextthinks[3] = ent->nextthink4;
		ent->think4 = NULL;
	}

	for (; i < MAX_THINKS; i++)
	{
		if (ent->thinks[i])
		RunTheThink(ent, i);
	}
}

/*
=============
SV_RunThink

Runs thinking code for this frame if necessary
=============
*/
qboolean SV_RunThink2 (edict_t *ent)
{
	float	thinktime2;

	thinktime2 = ent->nextthink2;
	if (thinktime2 <= 0)
		return true;
	if (thinktime2 > level.time+0.001)
		return true;
	
	ent->nextthink2 = 0;
	if (!ent->think2)
	{
		gi.dprintf ("========================================\n");
		// Paril: No more erring on null thinks
		gi.dprintf ("Warning: Entity has a NULL think2: %s", ent->classname);
		if (ent->owner)
		{
			if (ent->owner->client)
				gi.dprintf (", owner is %s\n", ent->owner->client->pers.netname);
			else
				gi.dprintf (", owner is %s\n", ent->classname);
		}
		gi.dprintf ("Something might not behave correctly!\n");
		gi.dprintf ("========================================\n");
		return true;
	}
	ent->think2 (ent);

	return false;
}

qboolean SV_RunThink3 (edict_t *ent)
{
	float	thinktime3;

	thinktime3 = ent->nextthink3;
	if (thinktime3 <= 0)
		return true;
	if (thinktime3 > level.time+0.001)
		return true;
	
	ent->nextthink3 = 0;
	if (!ent->think3)
	{
		gi.dprintf ("========================================\n");
		// Paril: No more erring on null thinks
		gi.dprintf ("Warning: Entity has a NULL think3: %s", ent->classname);
		if (ent->owner)
		{
			if (ent->owner->client)
				gi.dprintf (", owner is %s\n", ent->owner->client->pers.netname);
			else
				gi.dprintf (", owner is %s\n", ent->classname);
		}
		gi.dprintf ("Something might not behave correctly!\n");
		gi.dprintf ("========================================\n");
		return true;
	}
	ent->think3 (ent);

	return false;
}
qboolean SV_RunThink4 (edict_t *ent)
{
	float	thinktime4;

	thinktime4 = ent->nextthink4;
	if (thinktime4 <= 0)
		return true;
	if (thinktime4 > level.time+0.001)
		return true;
	
	ent->nextthink4 = 0;
	if (!ent->think4)
	{
		gi.dprintf ("========================================\n");
		// Paril: No more erring on null thinks
		gi.dprintf ("Warning: Entity has a NULL think4: %s", ent->classname);
		if (ent->owner)
		{
			if (ent->owner->client)
				gi.dprintf (", owner is %s\n", ent->owner->client->pers.netname);
			else
				gi.dprintf (", owner is %s\n", ent->classname);
		}
		gi.dprintf ("Something might not behave correctly!\n");
		gi.dprintf ("========================================\n");
		return true;
	}
	ent->think4 (ent);

	return false;
}

qboolean SV_RunThink (edict_t *ent)
{
	float	thinktime;

	thinktime = ent->nextthink;
	if (thinktime <= 0)
		return true;
	if (thinktime > level.time+0.001)
		return true;
	
	ent->nextthink = 0;
	if (!ent->think)
	{
		gi.dprintf ("========================================\n");
		// Paril: No more erring on null thinks
		gi.dprintf ("Warning: Entity has a NULL think: %s", ent->classname);
		if (ent->owner)
		{
			if (ent->owner->client)
				gi.dprintf (", owner is %s\n", ent->owner->client->pers.netname);
			else
				gi.dprintf (", owner is %s\n", ent->classname);
		}
		else
			gi.dprintf ("\n");
		gi.dprintf ("Something might not behave correctly!\n");
		gi.dprintf ("========================================\n");
		return true;
	}
	ent->think (ent);

	return false;
}

/*
==================
SV_Impact

Two entities have touched, so run their touch functions
==================
*/
void SV_Impact (edict_t *e1, trace_t *trace)
{
	edict_t		*e2;
//	cplane_t	backplane;

	e2 = trace->ent;

	//if (Q_stricmp(e1->classname, "pacman") == 0 && Q_stricmp(e2->classname, "player") == 0)
	//	return;

	if (e1->touch && e1->solid != SOLID_NOT)
		e1->touch (e1, e2, &trace->plane, trace->surface);
	
	if (e2->touch && e2->solid != SOLID_NOT)
		e2->touch (e2, e1, NULL, NULL);
}


/*
==================
ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
#define	STOP_EPSILON	0.1

int ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float	backoff;
	float	change;
	int		i, blocked;
	
	blocked = 0;
	if (normal[2] > 0)
		blocked |= 1;		// floor
	if (!normal[2])
		blocked |= 2;		// step
	
	backoff = DotProduct (in, normal) * overbounce;

	for (i=0 ; i<3 ; i++)
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}

	return blocked;
}


/*
============
SV_FlyMove

The basic solid body movement clip that slides along multiple planes
Returns the clipflags if the velocity was modified (hit something solid)
1 = floor
2 = wall / step
4 = dead stop
============
*/
#define	MAX_CLIP_PLANES	5
int SV_FlyMove (edict_t *ent, float time, int mask)
{
	edict_t		*hit;
	int			bumpcount, numbumps;
	vec3_t		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity, original_velocity, new_velocity;
	int			i, j;
	trace_t		trace;
	vec3_t		end;
	float		time_left;
	int			blocked;
	
	numbumps = 4;
	
	blocked = 0;
	VectorCopy (ent->velocity, original_velocity);
	VectorCopy (ent->velocity, primal_velocity);
	numplanes = 0;
	
	time_left = time;

	ent->groundentity = NULL;

	for (bumpcount=0 ; bumpcount<numbumps ; bumpcount++)
	{
		for (i=0 ; i<3 ; i++)
			end[i] = ent->s.origin[i] + time_left * ent->velocity[i];
		
		trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, end, ent, mask);
		
		if (trace.allsolid)
		{	// entity is trapped in another solid
			VectorCopy (vec3_origin, ent->velocity);
			return 3;
		}
		
		if (trace.fraction > 0)
		{	// actually covered some distance
			VectorCopy (trace.endpos, ent->s.origin);
			VectorCopy (ent->velocity, original_velocity);
			numplanes = 0;
		}
		
		if (trace.fraction == 1)
			break;		// moved the entire distance
		
		hit = trace.ent;
		
		if (trace.plane.normal[2] > 0.7)
		{
			blocked |= 1;		// floor
			if ( hit->solid == SOLID_BSP)
			{
				ent->groundentity = hit;
				ent->groundentity_linkcount = hit->linkcount;
			}
		}
		if (!trace.plane.normal[2])
		{
			blocked |= 2;		// step
		}
		
		//
		// run the impact function
		//
		SV_Impact (ent, &trace);
		if (!ent->inuse)
			break;		// removed by the impact function
		
		
		time_left -= time_left * trace.fraction;
		
		// cliped to another plane
		if (numplanes >= MAX_CLIP_PLANES)
		{	// this shouldn't really happen
			VectorCopy (vec3_origin, ent->velocity);
			return 3;
		}
		
		VectorCopy (trace.plane.normal, planes[numplanes]);
		numplanes++;
		
		//
		// modify original_velocity so it parallels all of the clip planes
		//
		for (i=0 ; i<numplanes ; i++)
		{
			ClipVelocity (original_velocity, planes[i], new_velocity, 1);
			
			for (j=0 ; j<numplanes ; j++)
				if ((j != i) && !VectorCompare (planes[i], planes[j]))
				{
					if (DotProduct (new_velocity, planes[j]) < 0)
						break;	// not ok
				}
				if (j == numplanes)
					break;
		}
		
		if (i != numplanes)
		{	// go along this plane
			VectorCopy (new_velocity, ent->velocity);
		}
		else
		{	// go along the crease
			if (numplanes != 2)
			{
				//				gi.dprintf ("clip velocity, numplanes == %i\n",numplanes);
				VectorCopy (vec3_origin, ent->velocity);
				return 7;
			}
			CrossProduct (planes[0], planes[1], dir);
			d = DotProduct (dir, ent->velocity);
			VectorScale (dir, d, ent->velocity);
		}
		
		//
		// if original velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		//
		if (DotProduct (ent->velocity, primal_velocity) <= 0)
		{
			VectorCopy (vec3_origin, ent->velocity);
			return blocked;
		}
	}

	return blocked;
}


/*
============
SV_AddGravity

============
*/
void SV_AddGravity (edict_t *ent)
{
	if(level.time > ent->gravity_debounce_time)
		ent->velocity[2] -= ent->gravity * sv_gravity->value * FRAMETIME;
}

/*
===============================================================================

PUSHMOVE

===============================================================================
*/

/*
============
SV_PushEntity

Does not change the entities velocity at all
============
*/
trace_t SV_PushEntity (edict_t *ent, vec3_t push)
{
	trace_t	trace;
	vec3_t	start;
	vec3_t	end;
	int		mask;

	VectorCopy (ent->s.origin, start);
	VectorAdd (start, push, end);

retry:
	if (ent->clipmask)
		mask = ent->clipmask;
	else
		mask = MASK_SOLID;

	trace = gi.trace (start, ent->mins, ent->maxs, end, ent, mask);
	
	VectorCopy (trace.endpos, ent->s.origin);
	gi.linkentity (ent);

	if (trace.fraction != 1.0)
	{
		SV_Impact (ent, &trace);

		// if the pushed entity went away and the pusher is still there
		if (!trace.ent->inuse && ent->inuse)
		{
			// move the pusher back and try again
			VectorCopy (start, ent->s.origin);
			gi.linkentity (ent);
			goto retry;
		}
	}

	if (ent->inuse)
		G_TouchTriggers (ent);

	return trace;
}					


typedef struct
{
	edict_t	*ent;
	vec3_t	origin;
	vec3_t	angles;
	float	deltayaw;
} pushed_t;
pushed_t	pushed[MAX_EDICTS], *pushed_p;

edict_t	*obstacle;

/*
============
SV_Push

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
============
*/
qboolean SV_Push (edict_t *pusher, vec3_t move, vec3_t amove)
{
	int			i, e;
	edict_t		*check, *block;
	vec3_t		mins, maxs;
	pushed_t	*p;
	vec3_t		org, org2, move2, forward, right, up;

	// clamp the move to 1/8 units, so the position will
	// be accurate for client side prediction
	for (i=0 ; i<3 ; i++)
	{
		float	temp;
		temp = move[i]*8.0;
		if (temp > 0.0)
			temp += 0.5;
		else
			temp -= 0.5;
		move[i] = 0.125 * (int)temp;
	}

	// find the bounding box
	for (i=0 ; i<3 ; i++)
	{
		mins[i] = pusher->absmin[i] + move[i];
		maxs[i] = pusher->absmax[i] + move[i];
	}

// we need this for pushing things later
	VectorSubtract (vec3_origin, amove, org);
	AngleVectors (org, forward, right, up);

// save the pusher's original position
	pushed_p->ent = pusher;
	VectorCopy (pusher->s.origin, pushed_p->origin);
	VectorCopy (pusher->s.angles, pushed_p->angles);
	if (pusher->client)
		pushed_p->deltayaw = pusher->client->ps.pmove.delta_angles[YAW];
	pushed_p++;

// move the pusher to it's final position
	VectorAdd (pusher->s.origin, move, pusher->s.origin);
	VectorAdd (pusher->s.angles, amove, pusher->s.angles);
	gi.linkentity (pusher);

// see if any solid entities are inside the final position
	check = g_edicts+1;
	for (e = 1; e < globals.num_edicts; e++, check++)
	{
		if (!check->inuse)
			continue;
		if (check->movetype == MOVETYPE_PUSH
		|| check->movetype == MOVETYPE_STOP
		|| check->movetype == MOVETYPE_NONE
		|| check->movetype == MOVETYPE_NOCLIP)
			continue;

		if (!check->area.prev)
			continue;		// not linked in anywhere

	// if the entity is standing on the pusher, it will definitely be moved
		if (check->groundentity != pusher)
		{
			// see if the ent needs to be tested
			if ( check->absmin[0] >= maxs[0]
			|| check->absmin[1] >= maxs[1]
			|| check->absmin[2] >= maxs[2]
			|| check->absmax[0] <= mins[0]
			|| check->absmax[1] <= mins[1]
			|| check->absmax[2] <= mins[2] )
				continue;

			// see if the ent's bbox is inside the pusher's final position
			if (!SV_TestEntityPosition (check))
				continue;
		}

		if ((pusher->movetype == MOVETYPE_PUSH) || (check->groundentity == pusher))
		{
			// move this entity
			pushed_p->ent = check;
			VectorCopy (check->s.origin, pushed_p->origin);
			VectorCopy (check->s.angles, pushed_p->angles);
			pushed_p++;

			// try moving the contacted entity 
			VectorAdd (check->s.origin, move, check->s.origin);
			if (check->client)
			{	// FIXME: doesn't rotate monsters?
				check->client->ps.pmove.delta_angles[YAW] += amove[YAW];
			}

			// figure movement due to the pusher's amove
			VectorSubtract (check->s.origin, pusher->s.origin, org);
			org2[0] = DotProduct (org, forward);
			org2[1] = -DotProduct (org, right);
			org2[2] = DotProduct (org, up);
			VectorSubtract (org2, org, move2);
			VectorAdd (check->s.origin, move2, check->s.origin);

			// may have pushed them off an edge
			if (check->groundentity != pusher)
				check->groundentity = NULL;

			block = SV_TestEntityPosition (check);
			if (!block)
			{	// pushed ok
				gi.linkentity (check);
				// impact?
				continue;
			}

			// if it is ok to leave in the old position, do it
			// this is only relevent for riding entities, not pushed
			// FIXME: this doesn't acount for rotation
			VectorSubtract (check->s.origin, move, check->s.origin);
			block = SV_TestEntityPosition (check);
			if (!block)
			{
				pushed_p--;
				continue;
			}
		}
		
		// save off the obstacle so we can call the block function
		obstacle = check;

		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for (p=pushed_p-1 ; p>=pushed ; p--)
		{
			VectorCopy (p->origin, p->ent->s.origin);
			VectorCopy (p->angles, p->ent->s.angles);
			if (p->ent->client)
			{
				p->ent->client->ps.pmove.delta_angles[YAW] = p->deltayaw;
			}
			gi.linkentity (p->ent);
		}
		return false;
	}

//FIXME: is there a better way to handle this?
	// see if anything we moved has touched a trigger
	for (p=pushed_p-1 ; p>=pushed ; p--)
		G_TouchTriggers (p->ent);

	return true;
}

/*
================
SV_Physics_Pusher

Bmodel objects don't interact with each other, but
push all box objects
================
*/
void SV_Physics_Pusher (edict_t *ent)
{
	vec3_t		move, amove;
	edict_t		*part, *mv;

	// if not a team captain, so movement will be handled elsewhere
	if ( ent->flags & FL_TEAMSLAVE)
		return;

	// make sure all team slaves can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
//retry:
	pushed_p = pushed;
	for (part = ent ; part ; part=part->teamchain)
	{
		if (part->velocity[0] || part->velocity[1] || part->velocity[2] ||
			part->avelocity[0] || part->avelocity[1] || part->avelocity[2]
			)
		{	// object is moving
			VectorScale (part->velocity, FRAMETIME, move);
			VectorScale (part->avelocity, FRAMETIME, amove);

			if (!SV_Push (part, move, amove))
				break;	// move was blocked
		}
	}
	if (pushed_p > &pushed[MAX_EDICTS])
		gi.error (ERR_FATAL, "pushed_p > &pushed[MAX_EDICTS], memory corrupted");

	if (part)
	{
		// the move failed, bump all nextthink times and back out moves
		for (mv = ent ; mv ; mv=mv->teamchain)
		{
			if (mv->nextthink > 0)
				mv->nextthink += FRAMETIME;
		}

		// if the pusher has a "blocked" function, call it
		// otherwise, just stay in place until the obstacle is gone
		if (part->blocked)
		{
			// Lazarus: Func_pushables with health < 0 & vehicles ALWAYS block pushers
			if(obstacle->movetype == MOVETYPE_VEHICLE)
			{
				part->moveinfo.is_blocked = true;
				if(part->s.sound)
				{
					if (part->moveinfo.sound_end)
						gi.sound (part, CHAN_NO_PHS_ADD+CHAN_VOICE, part->moveinfo.sound_end, 1, ATTN_STATIC, 0);
					part->s.sound = 0;
				}
			}
			else
			{
				part->blocked (part, obstacle);
				part->moveinfo.is_blocked = true;
			}
		}

		#if 0
		// if the pushed entity went away and the pusher is still there
		if (!obstacle->inuse && part->inuse)
			goto retry;
#endif
	}
	else
	{
		// the move succeeded, so call all think functions
		for (part = ent ; part ; part=part->teamchain)
		{
//			SV_RunThink (part);
//			SV_RunThink2 (part);
//			SV_RunThink3 (part);
//			SV_RunThink4 (part);
			RunThinks(part);
		}
	}
}


//
//============
//SV_VehicleMove
//============
//
#define	MAX_CLIP_PLANES	5
int SV_VehicleMove (edict_t *ent, float time, int mask)
{
	edict_t		*hit;
	edict_t		*ignore;
	trace_t		trace;
	vec3_t		dir;
	vec3_t		end;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity, original_velocity, new_velocity;
	vec3_t		start;
	vec3_t		move, amove;
	vec3_t		xy_velocity;
	vec_t		xy_speed;
	float		d;
	float		e, m, v11, v12, v21, v22;
	float		time_left;
	int			bumpcount, numbumps;
	int			numplanes;
	int			i, j;
	int			blocked;

	// Corrective stuff added for bmodels with no origin brush
	vec3_t		mins, maxs;
	vec3_t      origin;

	numbumps = 4;
	
	blocked = 0;
	VectorCopy (ent->velocity, original_velocity);
	VectorCopy (ent->velocity, primal_velocity);
	numplanes = 0;
	
	VectorCopy(ent->velocity,xy_velocity);
	xy_velocity[2] = 0;
	xy_speed = VectorLength(xy_velocity);

	time_left = time;

	VectorAdd(ent->s.origin,ent->origin_offset,origin);
	VectorCopy(ent->size,maxs);
	VectorScale(maxs,0.5,maxs);
	VectorNegate(maxs,mins);
	mins[2] += 1;

	ent->groundentity = NULL;

	ignore = ent;
	VectorCopy(origin,start);

	for (bumpcount=0 ; bumpcount<numbumps ; bumpcount++)
	{
		for (i=0 ; i<3 ; i++)
			end[i] = origin[i] + time_left * ent->velocity[i];

//retry:
		trace = gi.trace (start, mins, maxs, end, ignore, mask);
/*		if(trace.ent && (trace.ent->movewith_ent == ent) )
		{
			ignore = trace.ent;
			VectorCopy(trace.endpos,start);
			goto retry;
		}*/

		if (trace.allsolid)
		{
			// entity is trapped in another solid 
			if(trace.ent && (trace.ent->svflags & SVF_MONSTER)) {
				// Monster stuck in vehicle. No matter how screwed up this is,
				// we've gotta get him out of there.
				// Give him a light-speed nudge and a velocity
				trace_t	tr;
				vec3_t	new_origin, new_velocity;

				VectorSubtract(trace.ent->s.origin,ent->s.origin,dir);
				dir[2] = 0;
				VectorNormalize(dir);
				dir[2] = 0.2;
				VectorMA(trace.ent->velocity,32,dir,new_velocity);
				VectorMA(trace.ent->s.origin,FRAMETIME,new_velocity,new_origin);
				tr = gi.trace(trace.ent->s.origin,trace.ent->mins,trace.ent->maxs,new_origin,trace.ent,MASK_MONSTERSOLID);
				if(tr.fraction == 1) {
					VectorCopy(new_origin,trace.ent->s.origin);
					VectorCopy(new_velocity,trace.ent->velocity);
					gi.linkentity(trace.ent);
				}
			}
			else if(trace.ent->client && xy_speed > 0 )
			{
				// If player is relatively close to the vehicle move_origin, AND the 
				// vehicle is still moving, then most likely the player just disengaged
				// the vehicle and isn't really trapped. Move player along with
				// vehicle
				vec3_t	forward, left, f1, l1, drive, offset;

				AngleVectors(ent->s.angles, forward, left, NULL);
				VectorScale(forward,ent->move_origin[0],f1);
				VectorScale(left,ent->move_origin[1],l1);
				VectorAdd(ent->s.origin,f1,drive);
				VectorAdd(drive,l1,drive);
				VectorSubtract(drive,trace.ent->s.origin,offset);
				if (fabs(offset[2]) < 64)
					offset[2] = 0;
				if (VectorLength(offset) < 16)
				{
					VectorAdd(trace.ent->s.origin,end,trace.ent->s.origin);
					VectorSubtract(trace.ent->s.origin,origin,trace.ent->s.origin);
					gi.linkentity(trace.ent);
					goto not_allsolid;
				}
			}
			VectorCopy (vec3_origin, ent->velocity);
			VectorCopy (vec3_origin, ent->avelocity);
			return 3;
		}

not_allsolid:

		if (trace.fraction > 0)
		{	// actually covered some distance
			VectorCopy (trace.endpos, origin);
			VectorSubtract (origin, ent->origin_offset, ent->s.origin);
			VectorCopy (ent->velocity, original_velocity);
			numplanes = 0;
		}

		if (trace.fraction == 1)
			break;		// moved the entire distance

		hit = trace.ent;

		if (trace.plane.normal[2] > 0.7)
		{
			blocked |= 1;		// floor
			if ( hit->solid == SOLID_BSP)
			{
				ent->groundentity = hit;
				ent->groundentity_linkcount = hit->linkcount;
			}
		}
		if (trace.plane.normal[0] > 0 || trace.plane.normal[1] > 0)
			blocked |= 1;
		if (!trace.plane.normal[2])
			blocked |= 2;		// step

//
// run the impact function
//
		SV_Impact (ent, &trace);
		if (!ent->inuse)
			break;		// vehicle destroyed
		if (!trace.ent->inuse)
		{
			blocked = 0;
			break;
		}

		if(trace.ent->classname)
		{
			if(ent->owner && (trace.ent->svflags & (SVF_MONSTER | SVF_DEADMONSTER)))
			{
				continue; // handled in vehicle_touch
			}
			else if(trace.ent->movetype != MOVETYPE_PUSHABLE)
			{
				// if not a func_pushable, match speeds...
				VectorCopy(trace.ent->velocity,ent->velocity);
			}
			else if(ent->mass && VectorLength(ent->velocity))
			{
				// otherwise push func_pushable (if vehicle has mass & is moving)
				e = 0.0; // coefficient of restitution
				m = (float)(ent->mass)/(float)(trace.ent->mass);
				for(i=0; i<2; i++) {
					v11 = ent->velocity[i];
					v21 = trace.ent->velocity[i];
					v22 = ( e*m*(v11-v21) + m*v11 + v21 ) / (1.0 + m);
					v12 = v22 - e*(v11-v21);
					ent->velocity[i] = v12;
					trace.ent->velocity[i] = v22;
					trace.ent->oldvelocity[i] = v22;
				}
				gi.linkentity(trace.ent);
			}
		}

		time_left -= time_left * trace.fraction;

		// cliped to another plane
		if (numplanes >= MAX_CLIP_PLANES)
		{
			VectorCopy (vec3_origin, ent->velocity);
			VectorCopy (vec3_origin, ent->avelocity);
			return 3;
		}

		// players, monsters and func_pushables don't block us
		if(trace.ent->client) {
			blocked = 0;
			continue;
		}
		if(trace.ent->svflags & SVF_MONSTER) {
			blocked = 0;
			continue;
		}
		if(trace.ent->movetype == MOVETYPE_PUSHABLE)
		{
			blocked = 0;
			continue;
		}

		VectorCopy (trace.plane.normal, planes[numplanes]);
		numplanes++;

//
// modify original_velocity so it parallels all of the clip planes
//
		for (i=0 ; i<numplanes ; i++)
		{
			ClipVelocity (original_velocity, planes[i], new_velocity, 2);

			for (j=0 ; j<numplanes ; j++)
				if ((j != i) && !VectorCompare (planes[i], planes[j]))
				{
					if (DotProduct (new_velocity, planes[j]) < 0)
						break;	// not ok
				}
			if (j == numplanes)
				break;
		}

		if (i != numplanes)
		{	// go along this plane
			VectorCopy (new_velocity, ent->velocity);
			VectorCopy (new_velocity, ent->oldvelocity);
		}
		else
		{	// go along the crease
			// DWH: What the hell does this do?
			if (numplanes != 2)
			{
				ent->moveinfo.state = 0;
				ent->moveinfo.next_speed = 0;
				VectorCopy (vec3_origin, ent->velocity);
				VectorCopy (vec3_origin, ent->oldvelocity);
				VectorCopy (vec3_origin, ent->avelocity);
				return 7;
			}
			CrossProduct (planes[0], planes[1], dir);
			d = DotProduct (dir, ent->velocity);
			VectorScale (dir, d, ent->velocity);
		}
//
// if original velocity is against the original velocity, stop dead
// to avoid tiny occilations in sloping corners
//
		if (DotProduct (ent->velocity, primal_velocity) <= 0)
		{
			ent->moveinfo.state = 0;
			ent->moveinfo.next_speed = 0;
			VectorCopy (vec3_origin, ent->velocity);
			VectorCopy (vec3_origin, ent->oldvelocity);
			VectorCopy (vec3_origin, ent->avelocity);
			return blocked;
		}
	}
	VectorScale(ent->velocity,FRAMETIME,move);
	VectorScale(ent->avelocity,FRAMETIME,amove);
	return blocked;
}

//==================================================================
qboolean	wasonground;
void SV_Physics_Vehicle (edict_t *ent)
{
	edict_t		*ground;
	int			mask;

//  see if we're on the ground
	if (!ent->groundentity)
		M_CheckGround (ent);

	ground = ent->groundentity;
	SV_CheckVelocity (ent);
	if (ground)
		wasonground = true;

//  move angles
	VectorMA (ent->s.angles, FRAMETIME, ent->avelocity, ent->s.angles);

	if (ent->velocity[2] || ent->velocity[1] || ent->velocity[0])
	{
		if(ent->org_size[0])
		{
			float		ca, sa, yaw;
			vec3_t		p[2][2];
			vec3_t		mins, maxs;
			vec3_t		s2;
			// Adjust bounding box for yaw
			yaw = ent->s.angles[YAW] * M_PI / 180.;
			ca  = cos(yaw);
			sa  = sin(yaw);
			VectorCopy(ent->org_size,s2);
			VectorScale(s2,1,s2);
			p[0][0][0] = -s2[0]*ca + s2[1]*sa;
			p[0][0][1] = -s2[1]*ca - s2[0]*sa;
			p[0][1][0] =  s2[0]*ca + s2[1]*sa;
			p[0][1][1] = -s2[1]*ca + s2[0]*sa;
			p[1][0][0] = -s2[0]*ca - s2[1]*sa;
			p[1][0][1] =  s2[1]*ca - s2[0]*sa;
			p[1][1][0] =  s2[0]*ca - s2[1]*sa;
			p[1][1][1] =  s2[1]*ca + s2[0]*sa;
			mins[0] = min(p[0][0][0],p[0][1][0]);
			mins[0] = min(mins[0],p[1][0][0]);
			mins[0] = min(mins[0],p[1][1][0]);
			mins[1] = min(p[0][0][1],p[0][1][1]);
			mins[1] = min(mins[1],p[1][0][1]);
			mins[1] = min(mins[1],p[1][1][1]);
			maxs[0] = max(p[0][0][0],p[0][1][0]);
			maxs[0] = max(maxs[0],p[1][0][0]);
			maxs[0] = max(maxs[0],p[1][1][0]);
			maxs[1] = max(p[0][0][1],p[0][1][1]);
			maxs[1] = max(maxs[1],p[1][0][1]);
			maxs[1] = max(maxs[1],p[1][1][1]);
			ent->size[0] = maxs[0] - mins[0];
			ent->size[1] = maxs[1] - mins[1];
			ent->mins[0] = -ent->size[0]/2;
			ent->mins[1] = -ent->size[1]/2;
			ent->maxs[0] =  ent->size[0]/2;
			ent->maxs[1] =  ent->size[1]/2;
			gi.linkentity(ent);
		}
		mask = MASK_ALL;
		SV_VehicleMove (ent, FRAMETIME, mask);
		gi.linkentity (ent);
		G_TouchTriggers (ent);
		if (!ent->inuse)
			return;
	}
//  regular thinking
	SV_RunThink (ent);
	VectorCopy(ent->velocity,ent->oldvelocity);
}

/*
=============
SV_Physics_None

Non moving objects can only think
=============
*/
void SV_Physics_None (edict_t *ent)
{
// regular thinking
//	SV_RunThink (ent);
//	SV_RunThink2 (ent);
//	SV_RunThink3 (ent);
//	SV_RunThink4 (ent);
	RunThinks(ent);
}

/*
=============
SV_Physics_Noclip

A moving object that doesn't obey physics
=============
*/
void SV_Physics_Noclip (edict_t *ent)
{
// regular thinking
	if (!SV_RunThink (ent))
		return;

	VectorMA (ent->s.angles, FRAMETIME, ent->avelocity, ent->s.angles);
	VectorMA (ent->s.origin, FRAMETIME, ent->velocity, ent->s.origin);

	gi.linkentity (ent);
}

/*
==============================================================================

TOSS / BOUNCE

==============================================================================
*/

/*
=============
SV_Physics_Toss

Toss, bounce, and fly movement.  When onground, do nothing.
=============
*/
void SV_Physics_Toss (edict_t *ent)
{
	trace_t		trace;
	vec3_t		move;
	float		backoff;
	edict_t		*slave;
	qboolean	wasinwater;
	qboolean	isinwater;
	vec3_t		old_origin;

// regular thinking
//	SV_RunThink (ent);
//	SV_RunThink2 (ent);
//	SV_RunThink3 (ent);
//	SV_RunThink4 (ent);
	RunThinks(ent);
	// if not a team captain, so movement will be handled elsewhere
	if ( ent->flags & FL_TEAMSLAVE)
		return;

	if (ent->velocity[2] > 0)
		ent->groundentity = NULL;

// check for the groundentity going away
	if (ent->groundentity)
		if (!ent->groundentity->inuse)
			ent->groundentity = NULL;

// if onground, return without moving
	if ( ent->groundentity )
		return;

	VectorCopy (ent->s.origin, old_origin);

	SV_CheckVelocity (ent);

// add gravity
	if (ent->movetype != MOVETYPE_FLY
	&& ent->movetype != MOVETYPE_VEHICLE
	&& ent->movetype != MOVETYPE_FLYMISSILE
	&& ent->movetype != MOVETYPE_WALLBOUNCE)
		SV_AddGravity (ent);

// move angles
	VectorMA (ent->s.angles, FRAMETIME, ent->avelocity, ent->s.angles);

// move origin
	VectorScale (ent->velocity, FRAMETIME, move);
	trace = SV_PushEntity (ent, move);
	if (!ent->inuse)
		return;

	if (trace.fraction < 1)
	{
		// RAFAEL
		if (ent->movetype == MOVETYPE_WALLBOUNCE)
			backoff = 2.0;
		// RAFAEL ( else )		
		else if (ent->movetype == MOVETYPE_BOUNCE)
			backoff = 1.5;
		else if (ent->movetype == MOVETYPE_RUBBER)
			backoff = 2.3;
		else if (ent->movetype == MOVETYPE_BOUNCENOSTOP)
			backoff = 1.9;
		else if (ent->movetype == MOVETYPE_BOUNCESTAY)
			backoff = 2.0;
		else if (ent->movetype == MOVETYPE_FLYMISSILE && Q_stricmp(ent->classname, "pacman") == 0)
			backoff = 2;
		else
			backoff = 1;

		ClipVelocity (ent->velocity, trace.plane.normal, ent->velocity, backoff);

		// RAFAEL
		if (ent->movetype == MOVETYPE_WALLBOUNCE)
			vectoangles (ent->velocity, ent->s.angles);
		if (ent->movetype == MOVETYPE_BOUNCESTAY)
			vectoangles (ent->velocity, ent->s.angles);

	// stop if on ground
		if (trace.plane.normal[2] > 0.7 && ent->movetype != MOVETYPE_WALLBOUNCE && ent->movetype != MOVETYPE_BOUNCESTAY)
		{		
			if (ent->velocity[2] < 60 || (ent->movetype != MOVETYPE_BOUNCE && ent->movetype != MOVETYPE_RUBBER && ent->movetype != MOVETYPE_BOUNCENOSTOP && ent->movetype != MOVETYPE_BOUNCESTAY))
			{
				ent->groundentity = trace.ent;
				ent->groundentity_linkcount = trace.ent->linkcount;
				VectorCopy (vec3_origin, ent->velocity);
				VectorCopy (vec3_origin, ent->avelocity);
			}
		}

//		if (ent->touch)
//			ent->touch (ent, trace.ent, &trace.plane, trace.surface);
	}
	
// check for water transition
	wasinwater = (ent->watertype & MASK_WATER);
	ent->watertype = gi.pointcontents (ent->s.origin);
	isinwater = ent->watertype & MASK_WATER;

	if (isinwater)
		ent->waterlevel = 1;
	else
		ent->waterlevel = 0;

	if (!wasinwater && isinwater)
		gi.positioned_sound (old_origin, g_edicts, CHAN_AUTO, SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);
	else if (wasinwater && !isinwater)
		gi.positioned_sound (ent->s.origin, g_edicts, CHAN_AUTO, SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);

// move teamslaves
	for (slave = ent->teamchain; slave; slave = slave->teamchain)
	{
		VectorCopy (ent->s.origin, slave->s.origin);
		gi.linkentity (slave);
	}
}

//void Link_All_Together(edict_t *bodpad);
// STICKING MOVEMENT LOL
void SV_Physics_Stuck (edict_t *ent)
{
        if (!ent->stuckentity || !ent->stuckentity->inuse)
        {
                ent->movetype = MOVETYPE_TOSS;
                goto end;
        }

		if (ent->stuckentity->takedamage && (ent->stuckentity->deadflag || ent->stuckentity->health <= 0))
		{
			G_FreeEdict (ent);
			return;
		}

        if (ent->stuckentity->movetype == MOVETYPE_NONE)
                goto end;

        if (!VectorCompare(ent->oldstuckorigin, ent->stuckentity->s.origin))
        {
                vec3_t resultat;

                VectorSubtract(ent->stuckentity->s.origin, ent->oldstuckorigin, resultat);
                VectorAdd(resultat, ent->s.origin, ent->s.origin);
        }

        if (!VectorCompare(ent->oldstuckangles, ent->stuckentity->s.angles))
        {
                vec3_t resultat, resultat2;
                float llargada;


                VectorSubtract(ent->stuckentity->s.angles, ent->oldstuckangles, resultat);
                VectorSubtract(ent->s.origin, ent->stuckentity->s.origin, resultat2);
                VectorAdd(resultat, ent->s.angles, ent->s.angles);

                llargada = VectorLength(resultat2);

                vectoangles(resultat2, resultat2);
                VectorAdd(resultat, resultat2, resultat);
                AngleVectors(resultat, resultat, NULL, NULL);
                VectorMA(ent->stuckentity->s.origin, llargada, resultat, ent->s.origin);
        }

        VectorCopy(ent->stuckentity->velocity, ent->velocity);
        VectorCopy(ent->stuckentity->s.angles, ent->oldstuckangles);
        VectorCopy(ent->stuckentity->s.origin, ent->oldstuckorigin);

	// relink
	gi.linkentity(ent);
end:
	// regular thinking
//	SV_RunThink (ent);
//	SV_RunThink2 (ent);
//	SV_RunThink3 (ent);
//	SV_RunThink4 (ent);
	RunThinks(ent);
	
	// Reposition botpad's linked parts
//	if (ent->mynoise2!=NULL)
//		Link_All_Together(ent);
}

/*
===============================================================================

STEPPING MOVEMENT

===============================================================================
*/

/*
=============
SV_Physics_Step

Monsters freefall when they don't have a ground entity, otherwise
all movement is done with discrete steps.

This is also used for objects that have become still on the ground, but
will fall if the floor is pulled out from under them.
FIXME: is this true?
=============
*/

//FIXME: hacked in for E3 demo
#define	sv_stopspeed		100
#define sv_friction			6
#define sv_waterfriction	1

void SV_AddRotationalFriction (edict_t *ent)
{
	int		n;
	float	adjustment;

	VectorMA (ent->s.angles, FRAMETIME, ent->avelocity, ent->s.angles);
	adjustment = FRAMETIME * sv_stopspeed * sv_friction;
	for (n = 0; n < 3; n++)
	{
		if (ent->avelocity[n] > 0)
		{
			ent->avelocity[n] -= adjustment;
			if (ent->avelocity[n] < 0)
				ent->avelocity[n] = 0;
		}
		else
		{
			ent->avelocity[n] += adjustment;
			if (ent->avelocity[n] > 0)
				ent->avelocity[n] = 0;
		}
	}
}

void SV_Physics_Step (edict_t *ent)
{
	qboolean	wasonground;
	qboolean	hitsound = false;
	float		*vel;
	float		speed, newspeed, control;
	float		friction;
	edict_t		*groundentity;
	int			mask;

	// airborn monsters should always check for ground
	if (!ent->groundentity)
		M_CheckGround (ent);

	groundentity = ent->groundentity;

	SV_CheckVelocity (ent);

	if (groundentity)
		wasonground = true;
	else
		wasonground = false;
		
	if (ent->avelocity[0] || ent->avelocity[1] || ent->avelocity[2])
		SV_AddRotationalFriction (ent);

	// add gravity except:
	//   flying monsters
	//   swimming monsters who are in the water
	if (! wasonground)
		if (!(ent->flags & FL_FLY))
			if (!((ent->flags & FL_SWIM) && (ent->waterlevel > 2)))
			{
				if (ent->velocity[2] < sv_gravity->value*-0.1)
					hitsound = true;
				if (ent->waterlevel == 0)
					SV_AddGravity (ent);
			}

	// friction for flying monsters that have been given vertical velocity
	if ((ent->flags & FL_FLY) && (ent->velocity[2] != 0))
	{
		speed = fabs(ent->velocity[2]);
		control = speed < sv_stopspeed ? sv_stopspeed : speed;
		friction = sv_friction/3;
		newspeed = speed - (FRAMETIME * control * friction);
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;
		ent->velocity[2] *= newspeed;
	}

	// friction for flying monsters that have been given vertical velocity
	if ((ent->flags & FL_SWIM) && (ent->velocity[2] != 0))
	{
		speed = fabs(ent->velocity[2]);
		control = speed < sv_stopspeed ? sv_stopspeed : speed;
		newspeed = speed - (FRAMETIME * control * sv_waterfriction * ent->waterlevel);
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;
		ent->velocity[2] *= newspeed;
	}

	if (ent->velocity[2] || ent->velocity[1] || ent->velocity[0])
	{
		// apply friction
		// let dead monsters who aren't completely onground slide
		if ((wasonground) || (ent->flags & (FL_SWIM|FL_FLY)))
			if (!(ent->health <= 0.0 && !M_CheckBottom(ent)))
			{
				vel = ent->velocity;
				speed = sqrt(vel[0]*vel[0] +vel[1]*vel[1]);
				if (speed)
				{
					friction = sv_friction;

					control = speed < sv_stopspeed ? sv_stopspeed : speed;
					newspeed = speed - FRAMETIME*control*friction;

					if (newspeed < 0)
						newspeed = 0;
					newspeed /= speed;

					vel[0] *= newspeed;
					vel[1] *= newspeed;
				}
			}

		if (ent->svflags & SVF_MONSTER)
			mask = MASK_MONSTERSOLID;
		else
			mask = MASK_SOLID;
		SV_FlyMove (ent, FRAMETIME, mask);

		gi.linkentity (ent);
		G_TouchTriggers (ent);
		if (!ent->inuse)
			return;

		if (ent->groundentity)
			if (!wasonground)
				if (hitsound)
					gi.sound (ent, 0, SoundIndex("world/land.wav"), 1, 1, 0);
	}

// regular thinking
//	SV_RunThink (ent);
//	SV_RunThink2 (ent);
//	SV_RunThink3 (ent);
//	SV_RunThink4 (ent);
	RunThinks(ent);
}

//============================================================================
/*
================
G_RunEntity

================
*/
void G_RunEntity (edict_t *ent)
{
	if (ent->prethink)
		ent->prethink (ent);

	switch ( (int)ent->movetype)
	{
	case MOVETYPE_PUSH:
	case MOVETYPE_STOP:
		SV_Physics_Pusher (ent);
		break;
	case MOVETYPE_NONE:
		SV_Physics_None (ent);
		break;
	case MOVETYPE_NOCLIP:
		SV_Physics_Noclip (ent);
		break;
	case MOVETYPE_STEP:
		SV_Physics_Step (ent);
		break;
	case MOVETYPE_TOSS:
	case MOVETYPE_BOUNCE:
	case MOVETYPE_FLY:
	case MOVETYPE_FLYMISSILE:
	case MOVETYPE_RUBBER:
	case MOVETYPE_BOUNCENOSTOP:
	case MOVETYPE_BOUNCESTAY:
	// RAFAEL
	case MOVETYPE_WALLBOUNCE:
		SV_Physics_Toss (ent);
		break;
    case MOVETYPE_STUCK:
        SV_Physics_Stuck(ent);
        break;
	case MOVETYPE_VEHICLE:
		SV_Physics_Vehicle (ent);
		break;
	case MOVETYPE_WALK:
	//	SV_RunThink(ent);
	//	SV_RunThink2(ent);
	//	SV_RunThink3(ent);
//		SV_RunThink4 (ent);
		RunThinks(ent);
		break;
	default:
		gi.error ("SV_Physics: bad movetype %i", (int)ent->movetype);			
	}
}
