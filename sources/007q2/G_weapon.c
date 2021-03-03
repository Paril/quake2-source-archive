#include "g_local.h"
/*void GQ_FreeHole(edict_t *ent) {
	// If the chain has become broken, panic.  Just destroy this bullethole
	// and reinitialize the chain.  This should never happen, but this is
	// just in case.  Better safe than sorry.  A bullethole may at sometime
	// get destroyed by an opening door or something.
	if (!firsthole) {
		gi.dprintf("What?  No firsthole?  Panic!\n");
		lasthole=NULL;
		holes=0;
		G_FreeEdict(ent);
		return;
	}
	gi.unlinkentity(firsthole);
	// Somehow a bullthole is dying before another hole that should have
	// died first.  Kill this entity and then proceed at the beginning of
	// the chain as normal.  This should never happen, but this is just in
	// case.  Better safe than sorry.  A bullethole may at sometime get
	// destroyed by an opening door or something.
	if (ent!=firsthole) {
		gi.dprintf("Uh oh, that's strange.  Firsthole isn't the one dying\n");
//		G_FreeEdict(old);
		firsthole->s.effects |= EF_SPHERETRANS;
		firsthole->nextthink = level.time+5;
		firsthole->think = G_FreeEdict;
	}
	// Now we kill off the first hole, the next hole takes its place
	firsthole=firsthole->owner;
//	G_FreeEdict(ent);
	ent->s.effects |= EF_SPHERETRANS;
	ent->nextthink = level.time+5;
	ent->think = G_FreeEdict;
	holes--;
//	debugmsg("Erasing hole.  Now %i\n", holes);
//	debugmsg("Erasing hole.  Now %i\n", holes);
}
*/
/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
static void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	vec3_t	end;
	vec3_t	v;
	trace_t	tr;
	float	eta;

	// easy mode only ducks one quarter the time
	if (skill->value == 0)
	{
		if (random() > 0.25)
			return;
	}
	VectorMA (start, 8192, dir, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) && infront(tr.ent, self))
	{
		VectorSubtract (tr.endpos, start, v);
		eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
		tr.ent->monsterinfo.dodge (tr.ent, self, eta);
	}
}


/*
=================
fire_hit

Used for all impact (hit/punch/slash) attacks
=================
*/
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick)
{
	trace_t		tr;
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	float		range;
	vec3_t		dir;

	//see if enemy is in range
	VectorSubtract (self->enemy->s.origin, self->s.origin, dir);
	range = VectorLength(dir);
	if (range > aim[0])
		return false;

	if (aim[1] > self->mins[0] && aim[1] < self->maxs[0])
	{
		// the hit is straight on so back the range up to the edge of their bbox
		range -= self->enemy->maxs[0];
	}
	else
	{
		// this is a side hit so adjust the "right" value out to the edge of their bbox
		if (aim[1] < 0)
			aim[1] = self->enemy->mins[0];
		else
			aim[1] = self->enemy->maxs[0];
	}

	VectorMA (self->s.origin, range, dir, point);

	tr = gi.trace (self->s.origin, NULL, NULL, point, self, MASK_SHOT);
	if (tr.fraction < 1)
	{
		if (!tr.ent->takedamage)
			return false;
		// if it will hit any client/monster then hit the one we wanted to hit
		if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
			tr.ent = self->enemy;
	}

	AngleVectors(self->s.angles, forward, right, up);
	VectorMA (self->s.origin, range, forward, point);
	VectorMA (point, aim[1], right, point);
	VectorMA (point, aim[2], up, point);
	VectorSubtract (point, self->enemy->s.origin, dir);

	// do the damage
	T_Damage (tr.ent, self, self, dir, point, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, MOD_HIT);

	if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		return false;

	// do our special form of knockback here
	VectorMA (self->enemy->absmin, 0.5, self->enemy->size, v);
	VectorSubtract (v, point, v);
	VectorNormalize (v);
	VectorMA (self->enemy->velocity, kick, v, self->enemy->velocity);
	if (self->enemy->velocity[2] > 0)
		self->enemy->groundentity = NULL;
	return true;
}


/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
/
static void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;
	float		ang;
	edict_t		*temp;
    
	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, self, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;

			water = true;
			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{

//}else{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (te_impact);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);

						if (!Q_strcasecmp(tr.ent->classname, "worldspawn") && ((int)max_holes->value>0)) {
							float closest=99;
							if (holes) {
								int i=1;
								float range;
								vec3_t dist;
								edict_t *current=firsthole;
								while (current) {
									if (!current->inuse) {
										current=current->owner;
										continue;
									}
									VectorSubtract(current->s.origin, tr.endpos, dist);
									range=VectorLength(dist);
//									debugmsg("Length to hole %i: %.2f\n", i, range);
									i++;
									current=current->owner;
									if (range<closest)
										closest=range;
								}
							}
							if (closest>2) {
//								debugmsg("Creating hole\n");
								temp=G_Spawn();
								VectorCopy (start, temp->s.origin);
								VectorMA(tr.endpos, 0.03, tr.plane.normal, temp->s.origin);
								vectoangles (tr.plane.normal, temp->s.angles);
								temp->s.modelindex = gi.modelindex ("models/objects/hole/tris.md2");
								temp->clipmask = 0;
								temp->solid = SOLID_NOT;
								temp->classname = "bullethole";
								temp->nextthink = level.time+5;
								temp->think = GQ_FreeHole;
								gi.linkentity(temp);
								if (!firsthole) {
									firsthole=temp;
//									debugmsg("First hole.\n");
									firsthole->owner=NULL;
									lasthole=firsthole;
									holes=1;
								} else {
									if (holes>=(int)max_holes->value) {
										edict_t *old;
//	 									debugmsg("Reached our limit\n");
										gi.unlinkentity(firsthole);
										old=firsthole;
										firsthole=firsthole->owner;
										G_FreeEdict(old);
									} else {
										holes++;
									}
//									debugmsg("Hole number %i\n", holes);
									lasthole->owner=temp;
									lasthole=temp;
									lasthole->owner=NULL;
								}
							} else {         */
/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
/
void fire_lead (edict_t *self, edict_t *ignore, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;
	float		ang;
	edict_t		*temp;
	qboolean	ricochet = false;

	tr = gi.trace (self->s.origin, NULL, NULL, start, ignore, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
		VectorNormalize (aimdir);				// For correct spread
		VectorMA (start, 8192, aimdir, end);	// More precise
//		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, ignore, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;

			water = true;
			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.trace (water_start, NULL, NULL, end, ignore, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
//				if (self->client)
//					self->client->resp.standard[GSTAT_HITS]++;
			//antibot code
//			} else if (tr.ent->flags & FL_ANTIBOT)
//				self->client->pers.hit_antibot ++;
//			else
		   //	{ //   }
             }
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					// Ricochet -- Added by Stone
					// Deflection most likely at sharp angles, no ricochet off the sky (duh).
					// Damage is reduced for each bounce... so no infinite ricochet.
//					ClipVelocity (aimdir, tr.plane.normal, dir, 2);
//					ang=AngleOfVectors(tr.plane.normal, dir);
//					if (bullet_ricochet->value && (damage>=2) && (random()<ang/135.0)) {
						// Create a temporary entity to give ricochets something to ignore. :)
						// Is this the best way to do it?  Don't know.  We can't just ignore
						// the tr.ent, because all walls are the "same" entity.  Thus, ricochets
						// would pass right through all walls! -- Stone
//						temp=G_Spawn();
 /*						VectorCopy (start, temp->s.origin);
						temp->clipmask = MASK_SHOT;
						temp->solid = SOLID_BBOX;
						temp->classname = "tempent";
						//gi_bprintf(PRINT_HIGH,"Ent: %s\n", tr.ent->classname);
						//gi_bprintf(PRINT_HIGH, "Angle: %f\n", ang);
						// Fixme: Incorporate spread to simulate imperfect surfaces?
						fire_lead (self, temp, tr.endpos, dir, (int)(damage/2.0), (int)kick/2, te_impact, 0, 0, mod);
						// Some sparks for effect
						gi.WriteByte (svc_temp_entity);
						gi.WriteByte (TE_BULLET_SPARKS);
//						gi.WriteByte (10);
						gi.WritePosition (tr.endpos);
						gi.WriteDir (tr.plane.normal);
						gi.multicast (tr.endpos, MULTICAST_PVS);
						gi.sound (temp, CHAN_AUTO, gi.soundindex(va("gunslinger/ric%i.wav",(rand()%4)+1)) , 1, ATTN_STATIC, 0);
						G_FreeEdict(temp);
  // /		  //			} else
//   {
						gi.WriteByte (svc_temp_entity);
						gi.WriteByte (te_impact);
						gi.WritePosition (tr.endpos);
						gi.WriteDir (tr.plane.normal);
						gi.multicast (tr.endpos, MULTICAST_PVS);
						if (!Q_strcasecmp(tr.ent->classname, "worldspawn") && ((int)max_holes->value>0)) {
							float closest=99;
							if (holes) {
								int i=1;
								float range;
								vec3_t dist;
								edict_t *current=firsthole;
								while (current) {
									if (!current->inuse) {
										current=current->owner;
										continue;
									}
									VectorSubtract(current->s.origin, tr.endpos, dist);
									range=VectorLength(dist);
//									debugmsg("Length to hole %i: %.2f\n", i, range);
									i++;
									current=current->owner;
									if (range<closest)
										closest=range;
								}
							}
							if (closest>2) {
//								gi.dprintf("Creating hole\n");
//								debugmsg("Creating hole\n");
								temp=G_Spawn();
								VectorCopy (start, temp->s.origin);
								VectorMA(tr.endpos, 0.03, tr.plane.normal, temp->s.origin);
								vectoangles (tr.plane.normal, temp->s.angles);
								temp->s.modelindex = gi.modelindex ("models/objects/hole/tris.md2");
								temp->clipmask = 0;
								temp->solid = SOLID_NOT;
								temp->classname = "bullethole";
								temp->nextthink = level.time+5;
								temp->think = GQ_FreeHole;
								gi.linkentity(temp);
								if (!firsthole) {
									firsthole=temp;
//									gi.dprintf("First hole.\n");
//									debugmsg("First hole.\n");
									firsthole->owner=NULL;
									lasthole=firsthole;
									holes=1;
								} else {
									if (holes>=(int)max_holes->value) {
										edict_t *old;
//	 									gi.dprintf("Reached our limit\n");
//	 									debugmsg("Reached our limit\n");
										gi.unlinkentity(firsthole);
										old=firsthole;
										firsthole=firsthole->owner;
										G_FreeEdict(old);
									} else {
										holes++;
									}
//									gi.dprintf("Hole number %i\n", holes);
//									debugmsg("Hole number %i\n", holes);
									lasthole->owner=temp;
									lasthole=temp;
									lasthole->owner=NULL;
								}
							} else {
//								gi.dprintf("Too close to another hole\n");
//								debugmsg("Too close to another hole\n");

         //   }
			   }
               		if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}
}
*/
/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
static void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;

	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, self, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;

			water = true;
			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (te_impact);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);

					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}
}
/*
=================
fire_bullet

Fires a single round.  Used for machinegun and chaingun.  Would be fine for
pistols, rifles, etc....
=================
*/
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
	fire_lead (self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
//	fire_lead (self, self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
}


/*
=================
fire_shotgun

Shoots shotgun pellets.  Used by shotgun and super shotgun.
=================
*/
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
	int		i;

	for (i = 0; i < count; i++)
		fire_lead (self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
//		fire_lead (self, self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
}


/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
//		if (self->spawnflags & 1)
//			mod = MOD_HYPERBLASTER;
//		else
			mod = MOD_PP7;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;
	// yes, I know it looks weird that projectiles are deadmonsters
	// what this means is that when prediction is used against the object
	// (blaster/hyperblaster shots), the player won't be solid clipped against
	// the object.  Right now trying to run into a firing hyperblaster
	// is very jerky since you are predicted 'against' the shots.
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	if (hyper)
		bolt->spawnflags = 1;
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}


/*
=================
fire_grenade
=================
*/
/* CCH: New think function for proximity grenades
static void proxim_think (edict_t *ent)
{
	edict_t *blip = NULL;

	if (level.time > ent->delay)
	{
		Grenade_Explode(ent);
		return;
	}

	ent->think = proxim_think;
	while ((blip = findradius(blip, ent->s.origin, 100)) != NULL)
	{
		if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			continue;
		if (blip == ent->owner)
			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health <= 0)
			continue;
		if (!visible(ent, blip))
			continue;
		ent->think = Grenade_Explode;
		break;
	}

	ent->nextthink = level.time + .1;
}                      */

static void Grenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
//   	points = ent->dmg + 1.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;
//    	T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
//	    T_RadiusDamage (ent, ent->activator, ent->dmg, NULL, ent->dmg+40, mod);
    	T_Damage (ent, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else if (ent->spawnflags & 1)
		mod = MOD_HG_SPLASH;
	else
		mod = MOD_G_SPLASH;
	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

static void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

// ion
//    if (ent->groundentity) Grenade_Explode (ent);
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}
// ions stupid idea =)
//    if (ent->groundentity) return;
	ent->enemy = other;
	Grenade_Explode (ent);
}
           // added ION
static void Grenade_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	self->takedamage = DAMAGE_NO;
	self->takedamage = DAMAGE_YES;
	self->nextthink = level.time + .1;
	self->think = Grenade_Explode;
}
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
//  grenade->movetype = MOVETYPE_MINETOSS;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
//  grenade->nextthink = Grenade_Touch;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
    // Added lines ION
    VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->mass = 2;
	grenade->health = 1;
	grenade->die = Grenade_Die;
	grenade->takedamage = DAMAGE_YES;
	grenade->monsterinfo.aiflags = AI_NOSTEP;

// another ion idea
//    if (ent->groundentity) Grenade_Die;
	gi.linkentity (grenade);
}

void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
    // Added ION
    VectorSet(grenade->mins, -3, -3, 0);
//	VectorSet(grenade->maxs, 3, 3, 6);
	VectorSet(grenade->maxs, 6, 6, 12);
	grenade->mass = 2;
	grenade->health = 1;
	grenade->die = Grenade_Die;
	grenade->takedamage = DAMAGE_YES;
	grenade->monsterinfo.aiflags = AI_NOSTEP;

	if (timer <= 0.0)
		Grenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
 }


/*
=================
fire_rocket
=================
*/
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}
       //start of long add ION
static void Rocket_Explode (edict_t *ent)
{
	vec3_t		origin;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_R_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
}

// CCH: When a rocket 'dies', it blows up next frame
static void Rocket_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
//	self->nextthink = level.time + 111.1;
	self->think = Rocket_Explode;
}
     //end of long add
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
//	rocket->nextthink = level.time + 1600000000/speed;
//	rocket->nextthink = level.time + 1600000000;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";
    // added ION
//    VectorSet(rocket->mins, -10, -3, 0);
//	VectorSet(rocket->maxs, 10, 3, 6);
//	VectorSet(rocket->maxs, 20, 6, 12);
	rocket->mass = 10;
	rocket->health = 10;
	rocket->die = Rocket_Die;
	rocket->takedamage = DAMAGE_YES;
	rocket->monsterinfo.aiflags = AI_NOSTEP;

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}


/*
=================
fire_rail
=================
*/
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;
	qboolean	water;

	VectorMA (start, 8192, aimdir, end);
	VectorCopy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	while (ignore)
	{
		tr = gi.trace (from, NULL, NULL, end, ignore, mask);

		if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
			water = true;
		}
		else
		{
			//ZOID--added so rail goes through SOLID_BBOX entities (gibs, etc)
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client) ||
				(tr.ent->solid == SOLID_BBOX))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
		}

		VectorCopy (tr.endpos, from);
	}

	// send gun puff / flash
	gi.WriteByte (svc_temp_entity);
//	gi.WriteByte (TE_RAILTRAIL);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);
//	gi.multicast (start, MULTICAST_PHS);
	if (water)
	{
		gi.WriteByte (svc_temp_entity);
//		gi.WriteByte (TE_RAILTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PHS);
	}

	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}


/*
=================
fire_bfg
=================
*/
void bfg_explode (edict_t *self)
{
	edict_t	*ent;
	float	points;
	vec3_t	v;
	float	dist;

	if (self->s.frame == 0)
	{
		// the BFG effect
		ent = NULL;
		while ((ent = findradius(ent, self->s.origin, self->dmg_radius)) != NULL)
		{
			if (!ent->takedamage)
				continue;
			if (ent == self->owner)
				continue;
			if (!CanDamage (ent, self))
				continue;
			if (!CanDamage (ent, self->owner))
				continue;

			VectorAdd (ent->mins, ent->maxs, v);
			VectorMA (ent->s.origin, 0.5, v, v);
			VectorSubtract (self->s.origin, v, v);
			dist = VectorLength(v);
			points = self->radius_dmg * (1.0 - sqrt(dist/self->dmg_radius));
			if (ent == self->owner)
				points = points * 0.5;

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_EXPLOSION);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PHS);
			T_Damage (ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_BFG_EFFECT);
		}
	}

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 5)
		self->think = G_FreeEdict;
}

void bfg_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	// core explosion - prevents firing it into the wall/floor
	if (other->takedamage)
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 200, 0, 0, MOD_BFG_BLAST);
	T_RadiusDamage(self, self->owner, 200, other, 100, MOD_BFG_BLAST);

	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorMA (self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/s_bfg3.sp2");
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST;
	self->think = bfg_explode;
	self->nextthink = level.time + FRAMETIME;
	self->enemy = other;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}


void bfg_think (edict_t *self)
{
	edict_t	*ent;
	edict_t	*ignore;
	vec3_t	point;
	vec3_t	dir;
	vec3_t	start;
	vec3_t	end;
	int		dmg;
	trace_t	tr;

	if (deathmatch->value)
		dmg = 5;
	else
		dmg = 10;

	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 256)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->owner)
			continue;

		if (!ent->takedamage)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;
//ZOID
/*		//don't target players in CTF
		if (ctf->value && ent->client &&
			self->owner->client &&
			ent->client->resp.ctf_team == self->owner->client->resp.ctf_team)
			continue;*/
//ZOID


		VectorMA (ent->absmin, 0.5, ent->size, point);

		VectorSubtract (point, self->s.origin, dir);
		VectorNormalize (dir);

		ignore = self;
		VectorCopy (self->s.origin, start);
		VectorMA (start, 2048, dir, end);
		while(1)
		{
			tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

			if (!tr.ent)
				break;

			// hurt it if we can
			if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
				T_Damage (tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);

			// if we hit something that's not a monster or player we're done
			if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (4);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
				break;
			}

			ignore = tr.ent;
			VectorCopy (tr.endpos, start);
		}

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}

	self->nextthink = level.time + FRAMETIME;
}


void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t	*bfg;

	bfg = G_Spawn();
	VectorCopy (start, bfg->s.origin);
	VectorCopy (dir, bfg->movedir);
	vectoangles (dir, bfg->s.angles);
	VectorScale (dir, speed, bfg->velocity);
	bfg->movetype = MOVETYPE_FLYMISSILE;
	bfg->clipmask = MASK_SHOT;
	bfg->solid = SOLID_BBOX;
	bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
	VectorClear (bfg->mins);
	VectorClear (bfg->maxs);
	bfg->s.modelindex = gi.modelindex ("sprites/s_bfg1.sp2");
	bfg->owner = self;
	bfg->touch = bfg_touch;
	bfg->nextthink = level.time + 8000/speed;
	bfg->think = G_FreeEdict;
	bfg->radius_dmg = damage;
	bfg->dmg_radius = damage_radius;
	bfg->classname = "bfg blast";
	bfg->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");

	bfg->think = bfg_think;
	bfg->nextthink = level.time + FRAMETIME;
	bfg->teammaster = bfg;
	bfg->teamchain = NULL;

	if (self->client)
		check_dodge (self, bfg->s.origin, dir, speed);

	gi.linkentity (bfg);
}

/*
=================
fire_proxmine (ION)
=================
*/


static void Proxmine_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
//		if (ent->spawnflags & 1)
//			mod = MOD_HELDPROXMINE;
//		else
			mod = MOD_PROXMINE;
//		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
//		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
//	    T_RadiusDamage (ent, ent->activator, ent->dmg, NULL, ent->dmg+40, mod);
    	T_Damage (ent, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

//	if (ent->spawnflags & 2)
//		mod = MOD_HELD_GRENADE;
	if (ent->spawnflags & 1)
		mod = MOD_HP_SPLASH;
	else
		mod = MOD_P_SPLASH;
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, mod);
//	T_RadiusDamage (ent, ent->activator, ent->dmg, NULL, ent->dmg+40, mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

static void Proxmine_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
// ion
//	if (ent->groundentity)
//		BecomeExplosion2 (ent);
//	else
//		BecomeExplosion1 (ent);
// ion
	Proxmine_Explode (ent);
}
           // added ION
static void Proxmine_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	self->takedamage = DAMAGE_NO;
	self->takedamage = DAMAGE_YES;
	self->nextthink = level.time + .1;
//	self->think = Proxmine_Explode;
	self->think = Proxmine_Explode;
}


// CCH: New think function for proximity grenades
static void proxim_think (edict_t *ent)
{
	edict_t *blip = NULL;

	if (level.time > ent->delay)
	{
		Proxmine_Explode(ent);
		return;
	}

	ent->think = proxim_think;
	while ((blip = findradius(blip, ent->s.origin, 140)) != NULL)
	{
		if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			continue;
//		if (blip == ent->owner)
//			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health <= 0)
			continue;
		if (!visible(ent, blip))
			continue;
		ent->think = Proxmine_Explode;
		break;
	}

	ent->nextthink = level.time + .1;
}

void fire_proxmine (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*proxmine;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	proxmine = G_Spawn();
	VectorCopy (start, proxmine->s.origin);
	VectorScale (aimdir, speed, proxmine->velocity);
	VectorMA (proxmine->velocity, 200 + crandom() * 10.0, up, proxmine->velocity);
	VectorMA (proxmine->velocity, crandom() * 10.0, right, proxmine->velocity);
	VectorSet (proxmine->avelocity, 300, 300, 300);
//	grenade->movetype = MOVETYPE_BOUNCE;
	proxmine->movetype = MOVETYPE_MINETOSS;
    proxmine->clipmask = MASK_SHOT;
	proxmine->solid = SOLID_BBOX;
//	proxmine->s.effects |= EF_GRENADE;
	VectorClear (proxmine->mins);
	VectorClear (proxmine->maxs);
	proxmine->s.modelindex = gi.modelindex ("models/objects/proxmine/tris.md2");
	proxmine->owner = self;
	proxmine->touch = Proxmine_Touch;
//	grenade->nextthink = level.time + timer;
//	grenade->think = Mine_Explode;

	// CCH: use our proximity think instead
//	grenade->nextthink = level.time + .1;
	proxmine->nextthink = level.time + 3.1;
	proxmine->think = proxim_think;
	proxmine->delay = level.time + 60;

	proxmine->dmg = damage;
	proxmine->dmg_radius = damage_radius;
//	proxmine->classname = "hgrenade";
	if (held)
		proxmine->spawnflags = 3;
	else
		proxmine->spawnflags = 1;
//	proxmine->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
    // Added ION
    VectorSet(proxmine->mins, -4, -4, 0);
	VectorSet(proxmine->maxs, 4, 4, 4);
	proxmine->mass = 2;
	proxmine->health = 1;
	proxmine->die = Proxmine_Die;
	proxmine->takedamage = DAMAGE_YES;
	proxmine->monsterinfo.aiflags = AI_NOSTEP;

	if (timer <= 0.0)
		Proxmine_Explode (proxmine);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (proxmine);
	}
}
// End ION

/*
=================
fire_remote (ION)
=================
*/


void Remote_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
//		if (ent->spawnflags & 1)
//			mod = MOD_HANDGRENADE;
//		else
			mod = MOD_REMOTE;
//		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
//		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
//	    T_RadiusDamage (ent, ent->activator, ent->dmg, NULL, ent->dmg+40, mod);
    	T_Damage (ent, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

//	if (ent->spawnflags & 2)
//		mod = MOD_HELD_GRENADE;
/*	if (ent->spawnflags & 1)
		mod = MOD_HR_SPLASH;
	else*/
		mod = MOD_RP_SPLASH;
//		mod = MOD_HG_SPLASH;
// hmmm	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);
//	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, mod);

//	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);
	T_RadiusDamage (ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, mod);
//	T_RadiusDamage (ent, ent->activator, ent->dmg, NULL, ent->dmg+40, mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

static void Remote_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;


	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}


	if (other->takedamage)
	{

//		ent->movetype = MOVETYPE_FLY;
//		VectorCopy (other->s.origin, ent->s.origin);
//		VectorCopy (other->s.origin, ent->s.origin);
		
//		motion = Sticky_FindPlayer(motion, ent->s.origin, 175) ;
		return;
	}

	if (!other->takedamage)
	{
/*		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{*/
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
//		}
		return;
	}
//	ent->s.angles = plane->normal;
//	vectoangles(plane->normal,	ent -> s.angles);

	ent->enemy = other;
// ion
//	if (ent->groundentity)
//		BecomeExplosion2 (ent);
//	else
//		BecomeExplosion1 (ent);
// ion
	Remote_Explode (ent);
}

           // added ION
static void Remote_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	self->takedamage = DAMAGE_NO;
	self->takedamage = DAMAGE_YES;
	self->nextthink = level.time + .1;
//	self->think = Proxmine_Explode;
	self->think = Remote_Explode;
}

/* CCH: New think function for proximity grenades
static void proxim_think (edict_t *ent)
{
	edict_t *blip = NULL;

	if (level.time > ent->delay)
	{
		Proxmine_Explode(ent);
		return;
	}

	ent->think = proxim_think;
	while ((blip = findradius(blip, ent->s.origin, 140)) != NULL)
	{
		if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			continue;
//		if (blip == ent->owner)
//			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health <= 0)
			continue;
		if (!visible(ent, blip))
			continue;
		ent->think = Proxmine_Explode;
		break;
	}

	ent->nextthink = level.time + .1;
}                                          */

void fire_remote (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*remote;
	vec3_t	dir;
	vec3_t	forward, right, up;
	trace_t		tr;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	remote = G_Spawn();
	VectorCopy (start, remote->s.origin);
	VectorScale (aimdir, speed, remote->velocity);
	VectorMA (remote->velocity, 200 + crandom() * 10.0, up, remote->velocity);
	VectorMA (remote->velocity, crandom() * 10.0, right, remote->velocity);
	VectorSet (remote->avelocity, 300, 300, 300);

//	grenade->movetype = MOVETYPE_BOUNCE;
	remote->movetype = MOVETYPE_MINETOSS;
    remote->clipmask = MASK_SHOT;
	remote->solid = SOLID_BBOX;
//	remote->s.effects |= EF_GRENADE;
	VectorClear (remote->mins);
	VectorClear (remote->maxs);
	remote->s.modelindex = gi.modelindex ("models/objects/proxmine/tris.md2");
/*	remote->owner = self;
	remote->touch = Remote_Touch;
//	grenade->nextthink = level.time + timer;
//	grenade->think = Mine_Explode;

	// CCH: use our proximity think instead
//	grenade->nextthink = level.time + .1;
	remote->nextthink = level.time + 10000;
	remote->think = Remote_Explode;
	remote->delay = level.time + 60;

	remote->dmg = damage;
	remote->dmg_radius = damage_radius;      */
// start
	remote->owner = self;
	remote->touch = Remote_Touch;
	remote->nextthink = level.time + 10000;
	remote->think = Remote_Explode;
	remote->dmg = damage;
	remote->dmg_radius = damage_radius;
// end
	remote->classname = "remote";
/*	if (held)
		remote->spawnflags = 3;
	else
		remote->spawnflags = 1;*/
//	proxmine->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
    // Added ION
    VectorSet(remote->mins, -4, -4, 0);
	VectorSet(remote->maxs, 4, 4, 4);
	remote->mass = 2;
	remote->health = 1;
	remote->die = Remote_Die;
	remote->takedamage = DAMAGE_YES;
	remote->monsterinfo.aiflags = AI_NOSTEP;

/*	if (timer <= 0.0)
		Remote_Explode (remote);
	else
	{*/
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (remote);
//	}
 }
// End ION

// Start ION
/*
=================
fire_laser

Laser by IoN_PuLse
=================
*/
void laser_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
//		if (self->spawnflags & 1)
			mod = MOD_LASER;
//		else
//			mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_laser (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
        edict_t *laserbolt;
        trace_t tr;
//        char lasercolour;

        VectorNormalize (dir);


        // Only change hand blaster effect
        if (effect & EF_BLASTER)
        {
                laserbolt = G_Spawn();
                VectorCopy (start, laserbolt->s.origin);
                vectoangles (dir, laserbolt->s.angles);
                VectorScale (dir, speed, laserbolt->velocity);
                VectorAdd (start, laserbolt->velocity, laserbolt->s.old_origin);
                laserbolt->clipmask = MASK_SHOT;

                laserbolt->movetype = MOVETYPE_FLYMISSILE;
                laserbolt->solid = SOLID_BBOX;
                laserbolt->s.renderfx |= RF_BEAM;
                laserbolt->s.modelindex = 1;
                laserbolt->owner = self;

//                laserbolt->speed = 1000;
                laserbolt->s.frame = 3;
                // set the color
//                if (self->client && self->client->teamnum == 1)         // on red team
//                        bolt->s.skinnum = 0xf2f2f0f0;
//                else                                                                                            // on blue team
//                laserbolt->s.skinnum = 0xf3f3f1f1;
    if (lasercolour->value == 0)
                laserbolt->s.skinnum = Laser_LtBlue;
    else if (lasercolour->value == 1)
                laserbolt->s.skinnum = Laser_Red;
    else if (lasercolour->value == 2)
                laserbolt->s.skinnum = Laser_Green;
    else if (lasercolour->value == 3)
                laserbolt->s.skinnum = Laser_Blue;
    else if (lasercolour->value == 4)
                laserbolt->s.skinnum = Laser_Yellow;
    else if (lasercolour->value == 5)
                laserbolt->s.skinnum = Laser_YellowS;
    else if (lasercolour->value == 6)
                laserbolt->s.skinnum = Laser_DkPurple;
    else if (lasercolour->value == 7)
                laserbolt->s.skinnum = Laser_Green2;
    else if (lasercolour->value == 8)
                laserbolt->s.skinnum = Laser_Purple;
    else if (lasercolour->value == 9)
                laserbolt->s.skinnum = Laser_Red2;
    else if (lasercolour->value == 10)
                laserbolt->s.skinnum = Laser_Orange;
    else if (lasercolour->value == 11)
                laserbolt->s.skinnum = Laser_Mix;
    else if (lasercolour->value == 12)
                laserbolt->s.skinnum = Laser_RedBlue;
    else if (lasercolour->value == 13)
                laserbolt->s.skinnum = Laser_BlueRed;
    else if (lasercolour->value == 14)
                laserbolt->s.skinnum = Laser_GreenY;
    else if (lasercolour->value == 15)
                laserbolt->s.skinnum = Laser_YellowG;

//                laserbolt->s.skinnum = lasercolour;


                VectorSet (laserbolt->mins, -8, -8, -8);
                VectorSet (laserbolt->maxs, 8, 8, 8);

                laserbolt->touch = laser_touch;
                laserbolt->nextthink = level.time + 4;
                laserbolt->think = G_FreeEdict;
                laserbolt->dmg = damage;
                
                gi.linkentity (laserbolt);

                if (self->client)
                        check_dodge (self, laserbolt->s.origin, dir, speed);

                tr = gi.trace (self->s.origin, NULL, NULL, laserbolt->s.origin, laserbolt, MASK_SHOT);
                if (tr.fraction < 1.0)
                {
                        VectorMA (laserbolt->s.origin, -10, dir, laserbolt->s.origin);
                        laserbolt->touch (laserbolt, tr.ent, NULL, NULL);
                }

                return;
        }

        laserbolt = G_Spawn();
        VectorCopy (start, laserbolt->s.origin);
        VectorCopy (start, laserbolt->s.old_origin);
        vectoangles (dir, laserbolt->s.angles);
        VectorScale (dir, speed, laserbolt->velocity);
        laserbolt->movetype = MOVETYPE_FLYMISSILE;
        laserbolt->clipmask = MASK_SHOT;
        laserbolt->solid = SOLID_BBOX;
        laserbolt->s.effects |= effect;
        VectorClear (laserbolt->mins);
        VectorClear (laserbolt->maxs);
        laserbolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
        laserbolt->s.sound = gi.soundindex ("misc/lasfly.wav");
        laserbolt->owner = self;
        laserbolt->touch = laser_touch;
        laserbolt->nextthink = level.time + 2;
        laserbolt->think = G_FreeEdict;
        laserbolt->dmg = damage;
        gi.linkentity (laserbolt);

        if (self->client)
                check_dodge (self, laserbolt->s.origin, dir, speed);

        tr = gi.trace (self->s.origin, NULL, NULL, laserbolt->s.origin, laserbolt, MASK_SHOT);
        if (tr.fraction < 1.0)
        {
                VectorMA (laserbolt->s.origin, -10, dir, laserbolt->s.origin);
                laserbolt->touch (laserbolt, tr.ent, NULL, NULL);
        }
}

// ENd IOn
