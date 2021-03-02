#include "g_local.h"



void FL_make(edict_t *self) 
{
	vec3_t	start,forward,right,end;

        if (self->client->pers.spectator > 0) return;
        if ( self->flashlight )
	{
		G_FreeEdict(self->flashlight);
		self->flashlight = NULL;
       gi.sound (self, CHAN_BODY, gi.soundindex ("weapons/noammo.wav"), 1, ATTN_NORM, 0);
		return;
	}

	AngleVectors (self->client->v_angle, forward, right, NULL);

	VectorSet(end,100 , 0, 0);
	G_ProjectSource (self->s.origin, end, forward, right, start);

	self->flashlight = G_Spawn ();
	self->flashlight->owner = self;
	self->flashlight->movetype = MOVETYPE_NOCLIP;
	self->flashlight->solid = SOLID_NOT;
	self->flashlight->classname = "flashlight";
        self->flashlight->s.modelindex = gi.modelindex ("sprites/s_fl.sp2");  // HEY KIDDYS NOTE THIS
	self->flashlight->s.skinnum = 0;
        self->flashlight->s.effects |= EF_HYPERBLASTER;          
                                                                
	self->flashlight->think = FL_think;
	self->flashlight->nextthink = level.time + 0.1;
       gi.sound (self, CHAN_BODY, gi.soundindex ("weapons/noammo.wav"), 1, ATTN_NORM, 0);
}

void FL_think (edict_t *self)
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

	if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
	{
		if ((tr.ent->takedamage) && (tr.ent != self->owner))
		{
			self->s.skinnum = 1;
		}
	}
	else
		self->s.skinnum = 0;

	vectoangles(tr.plane.normal,self->s.angles);
	VectorCopy(tr.endpos,self->s.origin);

	gi.linkentity (self);
	self->nextthink = level.time + 0.1;
}
