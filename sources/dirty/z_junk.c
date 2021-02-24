// File that handles gibs, heads, blood etc...
#include "g_local.h"

// Dirty - *ugh* Action Quake code!! :(
        // we'll just use ->points instead of ->classnum
/*
==================
FindEdictByClassnum
==================
*/
edict_t *FindEdictByClassnum (char *classname, int classnum)
{
        edict_t *ent;
        int     i;

        for (i=0 ; i<globals.num_edicts ; i++)
        {
                ent = &g_edicts[i];
                if (!ent->classname)
                        continue;
                if (!ent->points)
                        continue;
                if (Q_stricmp(ent->classname, classname) == 0)
                {
                        if (ent->points == classnum)
                                return ent;
                }
        }

        return NULL;
}
// Dirty

/*
=================
ShitAvailable - Global limiter.
=================
*/
qboolean ShitAvailable (void)
{
        if (total_shit > (int)junk->value)
                return false;
        total_shit++;
        return true;
}

/*
=================
Misc functions
=================
*/
void VelocityForDamage (int damage, vec3_t v)
{
	v[0] = 100.0 * crandom();
	v[1] = 100.0 * crandom();
	v[2] = 200.0 + 100.0 * random();

	if (damage < 50)
		VectorScale (v, 0.7, v);
	else 
		VectorScale (v, 1.2, v);
}

void ClipGibVelocity (edict_t *ent)
{
	if (ent->velocity[0] < -300)
		ent->velocity[0] = -300;
	else if (ent->velocity[0] > 300)
		ent->velocity[0] = 300;
	if (ent->velocity[1] < -300)
		ent->velocity[1] = -300;
	else if (ent->velocity[1] > 300)
		ent->velocity[1] = 300;
	if (ent->velocity[2] < 200)
		ent->velocity[2] = 200;	// always some upwards
	else if (ent->velocity[2] > 500)
		ent->velocity[2] = 500;
}


/*
=================
gibs
=================
*/
void gib_free (edict_t *self)
{
        total_shit--;
        total_gibs--;
        G_FreeEdict(self);
}

void gib_think (edict_t *self)
{
	self->s.frame++;
	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == 10)
	{
                self->think = gib_free;
		self->nextthink = level.time + 8 + random()*10;
	}
}

void gib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	normal_angles, right;

        if (gi.pointcontents (self->s.origin) & (CONTENTS_LAVA | CONTENTS_SLIME))
        {
                gib_free (self);
                return;
        }

	if (!self->groundentity)
		return;

	self->touch = NULL;

	if (plane)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/fhit3.wav"), 1, ATTN_NORM, 0);

		vectoangles (plane->normal, normal_angles);
		AngleVectors (normal_angles, NULL, right, NULL);
		vectoangles (right, self->s.angles);

		if (self->s.modelindex == sm_meat_index)
		{
			self->s.frame++;
			self->think = gib_think;
			self->nextthink = level.time + FRAMETIME;
		}
	}
}

void gib_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        if (self->attack_time > level.time)
                return;
        
        gib_free (self);
}

void ThrowGib (edict_t *self, char *gibname, int damage, int type)
{
	edict_t *gib;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	float	vscale;

        if (total_gibs > (int)gibs->value)
                return;
        if (!ShitAvailable())
                return;
        total_gibs++;

	gib = G_Spawn();

	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	gib->s.origin[0] = origin[0] + crandom() * size[0];
	gib->s.origin[1] = origin[1] + crandom() * size[1];
	gib->s.origin[2] = origin[2] + crandom() * size[2];

	gi.setmodel (gib, gibname);
        gib->solid = SOLID_NOT;
	gib->s.effects |= EF_GIB;
	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_YES;
	gib->die = gib_die;

	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_TOSS;
		gib->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		gib->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage (damage, vd);
	VectorMA (self->velocity, vscale, vd, gib->velocity);
	ClipGibVelocity (gib);
	gib->avelocity[0] = random()*600;
	gib->avelocity[1] = random()*600;
	gib->avelocity[2] = random()*600;

        gib->think = gib_free;
	gib->nextthink = level.time + 10 + random()*10;

        gib->deadflag = DEAD_DEAD;
        gib->attack_time = level.time + 0.1;

	gi.linkentity (gib);
}


/*
=================
skulls - Client skulls are allow the client to view the mayhem as a skull
        Other skulls are treated like gibs. Spawned then removed.
=================
*/
void head_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        vec3_t          forward, right, up, vel;

        if ((!ent->client) && gi.pointcontents (ent->s.origin) & (CONTENTS_LAVA | CONTENTS_SLIME))
        {
                gib_free (ent);
                return;
        }

        if ((!other->groundentity) || (other->groundentity == ent))
                return;

        if (other->client)
        {
                AngleVectors (other->s.angles, forward, right, up);
                ent->groundentity = NULL;
                ent->velocity[2] += 20;
                VectorScale (forward, 300, vel);

                if (vel[2] > 100)
                        vel[2] = 100;
                else if (vel[2] < 10)
                        vel[2] = 10;

                VectorSet (ent->avelocity, 100, 100, 100);

                VectorMA (vel, 200 + crandom() * 10.0, up, vel);
                VectorMA (vel, crandom() * 10.0, right, vel);
                VectorAdd (ent->velocity, vel, ent->velocity);
                ClipGibVelocity (ent);
        }
        else if (random() < 0.3)
                gi.sound (ent, CHAN_VOICE, gi.soundindex ("misc/fhit3.wav"), 1, ATTN_NORM, 0);
}

void ThrowHead (edict_t *self, char *gibname, int damage, int type)
{
        edict_t *head;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	float	vscale;

        if (total_gibs > (int)gibs->value)
                return;
        if (!ShitAvailable())
                return;
        total_gibs++;

        head = G_Spawn();

        VectorSet (head->mins, -16, -16, 0);
        VectorSet (head->maxs, 16, 16, 32);

        if (damage < 0)
                VectorCopy (self->s.origin, head->s.origin);
        else
        {
                VectorScale (self->size, 0.5, size);
                VectorAdd (self->absmin, size, origin);
                head->s.origin[0] = origin[0] + crandom() * size[0];
                head->s.origin[1] = origin[1] + crandom() * size[1];
                head->s.origin[2] = origin[2] + crandom() * size[2];
                head->s.origin[2] += 16;
        }
        gi.setmodel (head, gibname);

        head->svflags &= ~SVF_MONSTER;

        head->movetype = MOVETYPE_BOUNCE;
	if (type == GIB_ORGANIC)
	{
                head->solid = SOLID_BBOX;
                head->svflags = SVF_DEADMONSTER;
                head->touch = head_touch;
		vscale = 0.5;
	}
	else
	{
                head->solid = SOLID_NOT;
		vscale = 1.0;
	}

        if (damage < 0)
                VectorCopy (self->s.angles, head->s.angles);
        else
        {
                VelocityForDamage (damage, vd);
                VectorMA (head->velocity, vscale, vd, head->velocity);
                ClipGibVelocity (head);
                head->avelocity[YAW] = crandom()*600;
        }

        head->s.effects |= EF_GIB;
        head->s.effects &= ~EF_FLIES;
        head->flags |= FL_NO_KNOCKBACK;
        head->s.sound = 0;
        head->takedamage = DAMAGE_YES;
        head->die = gib_die;

        head->think = G_FreeEdict;
        head->nextthink = level.time + 10 + random()*10;

        gi.linkentity (head);
}

void ThrowClientHead (edict_t *self, int damage)
{
	vec3_t	vd;
	char	*gibname;

        // FIX ME - Add something clever for Paranoid

	if (rand()&1)
	{
		gibname = "models/objects/gibs/head2/tris.md2";
		self->s.skinnum = 1;		// second skin is player
	}
	else
	{
		gibname = "models/objects/gibs/skull/tris.md2";
		self->s.skinnum = 0;
	}

	self->s.origin[2] += 32;
	self->s.frame = 0;
	gi.setmodel (self, gibname);
	VectorSet (self->mins, -16, -16, 0);
        VectorSet (self->maxs, 16, 16, 32);

        self->takedamage = DAMAGE_YES;
        self->solid = SOLID_BBOX;
        self->svflags = SVF_DEADMONSTER;
        self->touch = head_touch;
	self->s.effects = EF_GIB;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;

	self->movetype = MOVETYPE_BOUNCE;
	VelocityForDamage (damage, vd);
	VectorAdd (self->velocity, vd, self->velocity);

	if (self->client)	// bodies in the queue don't have a client anymore
	{
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = self->s.frame;
	}
	else
	{
		self->think = NULL;
		self->nextthink = 0;
	}

	gi.linkentity (self);
}


/*
=================
debris
=================
*/
void debris_free (edict_t *self)
{
        total_shit--;
        total_debris--;
        G_FreeEdict(self);
}

void debris_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        debris_free (self);
}

void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin)
{
	edict_t	*chunk;
	vec3_t	v;

        if (total_debris > (int)debris->value)
                return;
        if (!ShitAvailable())
                return;
        total_debris++;

	chunk = G_Spawn();
	VectorCopy (origin, chunk->s.origin);
	gi.setmodel (chunk, modelname);
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA (self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_BOUNCE;
	chunk->solid = SOLID_NOT;
	chunk->avelocity[0] = random()*600;
	chunk->avelocity[1] = random()*600;
	chunk->avelocity[2] = random()*600;
        chunk->deadflag = DEAD_DEAD;
        chunk->think = debris_free;
	chunk->nextthink = level.time + 5 + random()*5;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "debris";
	chunk->takedamage = DAMAGE_YES;
	chunk->die = debris_die;
	gi.linkentity (chunk);
}



/*
==============

QUE STUFF

==============
*/
/*
==============
InitOtherQues - Blood and bullet holes.
==============
*/

void InitOtherQues (void)
{
	int		i;
	edict_t	*ent;

        return; // Disabled

        // Only for deathmatch so far.
        if (!deathmatch->value)
                return;

        if (bhole->value)
        {
                level.bhole_que = 0;
                for (i=0; i<((int)bhole->value); i++)
                {               
                        ent = G_Spawn();
                        ent->classname = "bholeque";
                }
        }

        if (blood->value)
        {
                level.blood_que = 0;
                for (i=0; i< (int)blood->value ; i++)
                {
                        ent = G_Spawn();
                        ent->classname = "bloodque";
                }
        }
}
/*
==============
TakeFromBHoleQue - Grab a bhole from the que and place it where you want it.
==============
*/
void TakeFromBHoleQue (edict_t *ent, char *model, vec3_t pos, vec3_t angle)
{
        edict_t *hole;

        return; // Disabled

        // Only for deathmatch so far.
        if (!deathmatch->value)
                return;

        if (!bhole->value)
                return;

        // grab a bhole que and cycle to the next one
        hole = &g_edicts[((int)maxclients->value + BODY_QUEUE_SIZE) + level.bhole_que + 1];
        level.bhole_que = (level.bhole_que + 1) % (int)bhole->value;

        gi.unlinkentity (hole);

        hole->s.number = hole - g_edicts;

        vectoangles (angle, hole->s.angles);
        VectorCopy (pos, hole->s.origin);

        hole->movetype = MOVETYPE_NONE;
        hole->solid = SOLID_NOT;
        VectorClear (hole->mins);
        VectorClear (hole->maxs);

        gi.setmodel (hole, model);

        gi.linkentity (hole);
}

/*
==============
BloodSpread - Think routine for blood pools.
==============
*/
void BloodSpread (edict_t *ent)
{
        vec3_t          angle, pos, pos2, forward, up;
        trace_t         tr;
        qboolean        nospread = false;
        VectorCopy (ent->s.angles, angle);

        while (angle[1] < 360)
        {
                AngleVectors (angle, forward, NULL, up);
                VectorMA (ent->s.origin, ((ent->s.frame + 1) * 4), forward, pos);
                VectorMA (pos, -4, up, pos2);

                tr = gi.trace (ent->s.origin, NULL, NULL, pos, ent, CONTENTS_SOLID);
                if (tr.fraction < 1.0)
                        nospread = true;
                if (!(gi.pointcontents(pos2) & CONTENTS_SOLID))
                        nospread = true;

                if (nospread)
                {
                        ent->nextthink = level.time + 2;
                        return;
                }

                angle[1] += 10;
        }

        if (ent->s.frame < ent->count)
                ent->s.frame++;

        ent->nextthink = level.time + (ent->s.frame / 4);
}

/*
==============
TakeFromBloodQue - Grab a blood from the que and place it where you want it.
==============
*/
void TakeFromBloodQue (edict_t *ent, char *model, vec3_t pos, vec3_t angle, int count)
{
        edict_t *bpool;

        return; // Disabled

        // Only for deathmatch so far.
        if (!deathmatch->value)
                return;

        if (!blood->value)
                return;

        // grab a bhole que and cycle to the next one
        bpool = &g_edicts[((int)maxclients->value + BODY_QUEUE_SIZE + (int)bhole->value) + level.blood_que + 1];
        level.blood_que = (level.blood_que + 1) % (int)blood->value;

        gi.unlinkentity (bpool);

        VectorCopy (pos, bpool->s.origin);
        VectorCopy (pos, bpool->s.old_origin);
        vectoangles (angle, bpool->s.angles);

        bpool->s.angles[1] = (rand() % 360);

        bpool->movetype = MOVETYPE_BOUNCE;
        bpool->clipmask = CONTENTS_SOLID;
        bpool->solid = SOLID_NOT;

        VectorClear (bpool->mins);
        VectorClear (bpool->maxs);

        gi.setmodel (bpool, model);

        if (count)
        {
                bpool->owner = ent;
                bpool->count = count;
                bpool->nextthink = level.time + 0.4;
                bpool->think = BloodSpread;
                bpool->s.frame = 1;
        }
        gi.linkentity (bpool);
        //gi.dprintf ("TakeFromBloodQue\n");
}

/*
==============
SpawnBloodPool - Place a pool of blood where 'ent' is
==============
*/
void SpawnBloodPool (edict_t *ent)
{
        vec3_t  end, up, angles;
        vec3_t  pos, angle;
	trace_t	tr;

        // Only for deathmatch so far.
        if (!deathmatch->value)
                return;

        if (!blood->value)
                return;

        VectorCopy (ent->s.angles, angles);
        angles[2] = 0;

        AngleVectors (angles, NULL, NULL, up);
        VectorMA (ent->s.origin, (ent->mins[2] - 50), up, end);

        tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);

        if (tr.fraction < 1.0)
        {
                VectorCopy (tr.endpos, pos);
                vectoangles (tr.plane.normal, angle);
        }
        else
                return;

        TakeFromBloodQue (ent, "models/lag/blood/bpool.md2", pos, angle, (ent->max_health/10));
}	

/*
=================
fire_blood - Blood spray
=================
*/
void blood_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
       // char    *splat_name;

        /*
        if (!(other->takedamage))
        {
                splat_name = va("models/lag/blood/splat%i.md2", (rand()%3)+1);
                TakeFromBloodQue (self, splat_name, self->s.origin, plane->normal, 0);
        }
        */
        G_FreeEdict (self);
}

void fire_blood (edict_t *self, vec3_t point, vec3_t dir, int speed)
{
        edict_t *blood;
        vec3_t  start;
        int     i;

        return; // Paranoid temp

        VectorMA (point, 1, dir, start);

        // No blood leaking out underwater...
        //if (gi.pointcontents(start) & MASK_WATER)
        //        return;

        blood = G_Spawn();
                
        //VectorNormalize (dir);

        VectorCopy (start, blood->s.origin);
        VectorCopy (start, blood->s.old_origin);

        i = speed;

        vectoangles (dir, blood->s.angles);
        VectorScale (dir, i, blood->velocity);

        if (i < 300)
                blood->velocity[2] = 30;

        VectorClear (blood->mins);
        VectorClear (blood->maxs);

        // Even though it noclips bboxs, it still can be shot!
        blood->svflags |= SVF_DEADMONSTER;

        //blood->movetype = MOVETYPE_FLYMISSILE;
        blood->movetype = MOVETYPE_TOSS;

        blood->solid = SOLID_BBOX;
        blood->clipmask = CONTENTS_SOLID;
        blood->s.effects |= EF_GIB;
        blood->flags |= FL_NO_KNOCKBACK;

        blood->s.modelindex = gi.modelindex ("sprites/null.sp2");
        blood->owner = self;
        blood->touch = blood_touch;
        blood->nextthink = level.time + 0.5;
        blood->think = G_FreeEdict;
        blood->classname = "blood";
        blood->waterlevel = -1;
        gi.linkentity (blood);
}

//========================================================================

/*
=================
GibClient - Throw out the right amount of arms, legs etc
=================
*/
void GibClient (edict_t *ent, int damage)
{
        int n;

        for (n= 0; n < 2; n++)
                ThrowGib (ent, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        for (n= 0; n < 2; n++)
                ThrowGib (ent, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
        for (n= 0; n < 2; n++)
                ThrowGib (ent, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);

        ThrowGib (ent, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);

        if (ent->client)
                ThrowClientHead (ent, damage);
        else
                ThrowHead (ent, "models/objects/gibs/skull/tris.md2", damage, GIB_ORGANIC);
}




