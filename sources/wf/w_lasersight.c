//This code just handles the laser sight
#include "g_local.h"

void LaserSightThink (edict_t *self);

#define lss ent->lasersight

void lasersight_on (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		end;

//if (wfdebug) gi.dprintf("  LASERON - ");

	if (!lss)	// Create it
	{
//if (wfdebug) gi.dprintf("  CREATE\n");
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(end,100 , 0, 0);
		G_ProjectSource (ent->s.origin, end, forward, right, start);
		lss = G_Spawn ();
		lss->owner = ent;
		lss->movetype = MOVETYPE_NOCLIP;
		lss->solid = SOLID_NOT;
		lss->classname = "lasersight";
		lss->s.modelindex = gi.modelindex ("models/sight/tris.md2");
		lss->s.skinnum = 0;   
		lss->s.renderfx |= RF_FULLBRIGHT;
		lss->think = LaserSightThink;  
		lss->nextthink = level.time + 0.1;
	}
	else
	{
//if (wfdebug) gi.dprintf("  NOTHING\n");
	}
}

void lasersight_off (edict_t *ent)
{
//if (wfdebug) gi.dprintf(" LASEROFF -");
	if (lss)	//Remove it
	{
	   G_FreeEdict(lss);   
	   lss = NULL;
//if (wfdebug) gi.dprintf("  CREATE\n");
	}
	else
	{
//if (wfdebug) gi.dprintf("  NOTHING\n");
	}
}

void LaserSightThink (edict_t *self)
{   
	vec3_t start,end,endp,offset;
	vec3_t forward,right,up;   
	trace_t tr;

	AngleVectors (self->owner->client->v_angle, forward, right, up);
	VectorSet(offset,24 , 6, self->owner->viewheight-7);
	G_ProjectSource (self->owner->s.origin, offset, forward, right, start);
	VectorMA(start,8192,forward,end);
	tr = gi.trace (start,NULL,NULL, end,self->owner,CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
	if (tr.fraction != 1) 
	{     
		VectorMA(tr.endpos,-4,forward,endp);
		VectorCopy(endp,tr.endpos);
	}
//if (tr.ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
	{
		if ((tr.ent->takedamage) && (tr.ent != self->owner))
		{ 
			if (tr.endpos[2] > ((tr.ent->s.origin[2] + 20)))
			{
			// Headshot Confirmed: Opted to go with switching skins instead of models
			self->s.skinnum = 1; 
			}

           /*Acrid 5/99 , note: BBox on crouched players differs 
		   from standing so +8 won't work, even though it 
		   should be correct ie, look how high you can aim
		   over a standing players head. I also suggest + 17-18
		   for standing players above and g_weapon.c:remove this note */
		   else if ( (tr.ent->client && tr.ent->client->ps.pmove.pm_flags & PMF_DUCKED) && (tr.endpos[2] > ((tr.ent->s.origin[2] + 1 ))) )
			{ 	
			self->s.skinnum = 1; 	
			}
		   //else your not aiming at the head anymore //Acrid
			else self->s.skinnum = 0;
		}
	} 
	else  //else this isnt a valid object //Acrid    
		self->s.skinnum = 0; 

	//The following line will make the meatball change angles
	//based on the the meatball is touching
		//vectoangles(tr.plane.normal,self->s.angles);

	//Instead, these line will face the meatball the same 
	//direction as the player
	self->s.angles[PITCH] = self->owner->s.angles[PITCH];
	self->s.angles[YAW] = self->owner->s.angles[YAW];
	self->s.angles[ROLL] = self->owner->s.angles[ROLL];


	VectorCopy(tr.endpos,self->s.origin);   
	gi.linkentity (self);
	self->nextthink = level.time + 0.1;
}
