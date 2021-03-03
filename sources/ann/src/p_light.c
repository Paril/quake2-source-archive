#include "g_local.h"
#include "g_brrr.h"

void FL_make (edict_t *self);
void FL_think (edict_t *self);

void Cmd_Sight_f(edict_t *ent)
{
    if (ent->solid==SOLID_NOT)  // Spectator
        return;

	if (strlen(gi.argv(1))==0)
	{
		ent->client->resp.fltype++;
		if (ent->client->resp.fltype>2)
			ent->client->resp.fltype=0;
	}
	else
	{
		ent->client->resp.fltype = atoi (gi.argv(1));
	}

	if (ent->client->resp.flashlight)
	{
		G_FreeEdict(ent->client->resp.flashlight);
		ent->client->resp.flashlight = NULL;
	}

	switch (ent->client->resp.fltype)
	{
		default:
			ent->client->resp.fltype=0;
		case 0:
			gi.cprintf(ent,PRINT_HIGH,"Laser sight deactivated.\n");
			break;
		case 1:
			gi.cprintf(ent,PRINT_HIGH,"Laser sight activated.\n");
			FL_make(ent);
			break;
		case 2:
			if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_FLASHLIGHT))
			{
				gi.cprintf(ent,PRINT_HIGH,"Flashlights are disallowed on this server!\n");
				ent->client->resp.fltype=0;
				return;
			}
			if (ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]<FLASHLIGHT_CELLS)
			{
				gi.cprintf(ent,PRINT_HIGH,"Out of cells for the flashlight!\n");
				ent->client->resp.fltype=0;
				return;
			}
			gi.cprintf(ent,PRINT_HIGH,"Flashlight activated.\n");
			FL_make(ent);
			break;
	}
}

/*

===============
FL_make
===============

*/
void FL_make(edict_t *self)
{
	vec3_t    start,forward,right,end;
	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorSet(end,100 , 0, 0);
	G_ProjectSource (self->s.origin, end, forward, right, start);
	self->client->resp.flashlight = G_Spawn ();
	self->client->resp.flashlight->owner = self;
	self->client->resp.flashlight->movetype = MOVETYPE_NOCLIP;
	self->client->resp.flashlight->solid = SOLID_NOT;
	self->client->resp.flashlight->classname = "flashlight";
	self->client->resp.flashlight->s.modelindex = gi.modelindex ("models/objects/sight/tris.md2");
	self->client->resp.flashlight->s.skinnum = 0;
//	self->client->resp.flashlight->s.effects |= 0x10000000;//transparency
	if (self->client->resp.fltype==2)
		self->client->resp.flashlight->s.effects |= EF_HYPERBLASTER;
	self->client->resp.flashlight->s.renderfx |= RF_FULLBRIGHT;

	self->client->resp.flashlight->think = FL_think;
	self->client->resp.flashlight->nextthink = level.time + 0.1;
}
/*

===============>
FL_think
===============

*/
void FL_think (edict_t *self)
{
	vec3_t start,end,endp,offset;
	vec3_t forward,right,up;
	trace_t tr;

/*	if (self->owner->deadflag!=DEAD_NO)
	{
		self->owner->client->resp.fltype=0;
		self->owner->client->resp.flashlight=NULL;
		G_FreeEdict(self);
	}*/

	AngleVectors (self->owner->client->v_angle, forward, right, up);
	VectorSet(offset,0 , 0, self->owner->viewheight/*-10*/);
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

	if (self->owner->client->resp.fltype==2)
	{
		if (self->owner->client->flframe==FLASHLIGHT_FRAMES)
		{
			self->owner->client->flframe=0;

			if (self->owner->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]<FLASHLIGHT_CELLS)
			{
				gi.cprintf(self->owner,PRINT_HIGH,"Out of cells for the flashlight!\n");
				self->owner->client->resp.flashlight = NULL;
				self->owner->client->resp.fltype=0;
				G_FreeEdict(self);
				return;
			}
			else
			{
				self->owner->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]-=FLASHLIGHT_CELLS;
			}
		}
		else
		{
			self->owner->client->flframe++;
		}
	}
	else
	{
		self->owner->client->flframe=0;
	}
}

