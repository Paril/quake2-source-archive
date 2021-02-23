#include "g_local.h"

int ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce);
qboolean CannotTeamDamage (edict_t *targ, edict_t *attacker);
void GQ_FreeHole(edict_t *ent);

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
		//antibot code
		if (tr.ent->flags & FL_ANTIBOT)
			self->client->pers.hit_antibot++;
		//end 

		if (!tr.ent->takedamage) {
			return false;
		}
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

void fire_melee (edict_t *self, vec3_t start, vec3_t aimdir, int range, int damage, int kick)
{
	vec3_t	from;
	vec3_t	end;
	trace_t	tr;
	
	if (self->client)
		self->client->resp.standard[GSTAT_SHOTS]++;
	VectorNormalize(aimdir);
	VectorCopy (start, from);
	VectorMA(from, (float)range, aimdir, end);
	tr = gi.trace (from, NULL, NULL, end, self, MASK_SHOT);

	if ((tr.ent != self) && (tr.fraction<1.0)) {
		if (tr.ent->takedamage) {
			T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_MELEE);
			if (self->client)
				self->client->resp.standard[GSTAT_HITS]++;
		} else {
			if (strncmp (tr.surface->name, "sky", 3) != 0) {
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_SPARKS);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.multicast (tr.endpos, MULTICAST_PVS);

//				gi.sound (self, CHAN_AUTO, gi.soundindex("weapons/hgrenb2a.wav") , 1, ATTN_NORM, 0);
				gi.sound (self, CHAN_AUTO, gi.soundindex("gunslinger/hatchetImpact.wav") , 1, ATTN_NORM, 0);
					//Sound if hit wall 
			}
		}
	}
}

/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
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
				if (self->client)
					self->client->resp.standard[GSTAT_HITS]++;
			//antibot code
			} else if (tr.ent->flags & FL_ANTIBOT)
				self->client->pers.hit_antibot ++;
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					// Ricochet -- Added by Stone
					// Deflection most likely at sharp angles, no ricochet off the sky (duh).
					// Damage is reduced for each bounce... so no infinite ricochet.
					ClipVelocity (aimdir, tr.plane.normal, dir, 2);
					ang=AngleOfVectors(tr.plane.normal, dir);
					if (bullet_ricochet->value && (damage>=2) && (random()<ang/135.0)) {
						// Create a temporary entity to give ricochets something to ignore. :)
						// Is this the best way to do it?  Don't know.  We can't just ignore
						// the tr.ent, because all walls are the "same" entity.  Thus, ricochets
						// would pass right through all walls! -- Stone
						temp=G_Spawn();
						VectorCopy (start, temp->s.origin);
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
					} else {
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
							} else {
//								debugmsg("Too close to another hole\n");
							}
						}
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
	
	// Through and throughs.  These look really sweet in slow-motion. -- Stone
//	debugmsg("Takedamage? %i\n", tr.ent->takedamage);
//	debugmsg("Classname: '%s'\n", tr.ent->classname);
//	debugmsg("Damage: %i\n", damage);
//	if (!tr.ent)
//		debugmsg("No ent!\n");
	if (tr.ent && (!tr.ent->inuse || (tr.ent->takedamage && (rand()%100 < damage)))) {
//		debugmsg("Bullet passed through!\n");
//		Fixme: Incorporate spread to simulate deflection?  Might ruin effect.
		fire_lead (self, tr.ent, tr.endpos, aimdir, (int)(damage/2.0), (int)kick/2, te_impact, 0, 0, mod);
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
	if (self->client)
		self->client->resp.standard[GSTAT_SHOTS]++;
	fire_lead (self, self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
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

	if (self->client)
		self->client->resp.standard[GSTAT_SHOTS]+=count;
	for (i = 0; i < count; i++)
		fire_lead (self, self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
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
		if (self->spawnflags & 1)
			mod = MOD_WINCHESTER_RIFLE;
		else
			mod = MOD_BOWIE_KNIFE;
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
fire_cocktail
=================
*/

void fireball_think (edict_t *self)
{
	int dmg;
	if (!self->enemy) {
		G_FreeEdict(self);
		return;
	}
//	debugmsg("Waterlevel: %i\n", self->enemy->waterlevel);
	if (self->enemy->waterlevel>1) {
		G_FreeEdict(self);
		return;
	}		
	if ((self->enemy->health < 1) || (!self->enemy->takedamage)) {
		G_FreeEdict(self);
		return;
	}
	VectorCopy(self->enemy->s.origin, self->s.origin);
	self->s.origin[2]=self->enemy->absmin[2]+64;
	if (deathmatch->value)
		dmg = 1;
	else
		dmg = 2;

	if (level.framenum % 2)
		T_Damage (self->enemy, self, self->owner, vec3_origin, self->s.origin, vec3_origin, dmg, 0, DAMAGE_RADIUS | DAMAGE_NO_ARMOR | DAMAGE_ENERGY | DAMAGE_NO_KNOCKBACK, MOD_FIRE);
	if (self->enemy->client && (self->enemy->pain_debounce_time <= level.time)
		&& (self->enemy->client->invincible_framenum < level.framenum))
	{
		if (rand()&1)
			gi.sound (self->enemy, CHAN_VOICE, gi.soundindex("player/burn1.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (self->enemy, CHAN_VOICE, gi.soundindex("player/burn2.wav"), 1, ATTN_NORM, 0);
		self->enemy->pain_debounce_time = level.time + 1;
	}
	self->delay--;
	if (self->delay<=0) {
		G_FreeEdict(self);
		return;
	}

	if (self->enemy->health < 1) {
		G_FreeEdict(self);
		return;
	}
	self->think = fireball_think;
	self->nextthink = level.time + FRAMETIME;
}


void create_fireball (edict_t *target, edict_t *owner)
{
	if (deathmatch->value && ((int)playmode->value == PM_LAST_MAN_STANDING) && (preptimeframe>level.framenum))
		return;
//	if (target->client)
//		debugmsg("Target: %s\n", target->client->pers.netname);
//	debugmsg("Owner: %s\n", owner->client->pers.netname);
	if (!CannotTeamDamage(target,owner)) {
		edict_t	*fire;

//		debugmsg("Creating fireball\n");
		fire = G_Spawn();
		VectorCopy(target->s.origin, fire->s.origin);
		fire->s.origin[2]=target->absmin[2]+64;
		fire->movetype = MOVETYPE_FLYMISSILE;
		fire->clipmask = MASK_SHOT;
		fire->solid = SOLID_NOT;
		fire->s.effects = EF_ROCKET | EF_ANIM_ALLFAST;
		fire->s.renderfx = RF_TRANSLUCENT;
		VectorClear (fire->mins);
		VectorClear (fire->maxs);
		fire->s.modelindex = gi.modelindex ("sprites/s_explo2.sp2");
		fire->enemy = target;
		fire->owner = owner;
		fire->think = fireball_think;
		fire->classname = "fireball";
		fire->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");
		fire->think = fireball_think;
		fire->nextthink = level.time + FRAMETIME;
		fire->delay = 50;
		gi.linkentity (fire);
	}
}



void Shrapnel_Touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (other->takedamage)
	{
//		debugmsg("Shrapnel hitting %s\n", other->classname);
		self->enemy=other;
		mod = MOD_HG_SPLASH;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GUNSHOT);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

static void Shrapnel_Think (edict_t *ent) {
	G_FreeEdict (ent);
}

static void Cocktail_Explode (edict_t *ent)
{
	edict_t		*shrapnel;
	vec3_t		origin, temp;
	int			mod, i;
	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	ent->takedamage=DAMAGE_NO;

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		vec3_t	dir;
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);

		mod = MOD_HANDGRENADE;
//		debugmsg("Molotov cocktail direct hit!  ");
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, ent->dmg, (int)ent->dmg/2, DAMAGE_RADIUS, mod);
		create_fireball (ent->enemy, ent->owner);
	}

	mod = MOD_HG_SPLASH;

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	if (ent->waterlevel && (ent->watertype&(CONTENTS_WATER|CONTENTS_SLIME))) {
		gi.sound (ent, CHAN_AUTO, gi.soundindex("world/brkglas.wav") , 1, ATTN_NORM, 0); 
		G_FreeEdict (ent);
		return;
	}
	gi.WriteByte (svc_temp_entity);
	if (ent->groundentity)
		gi.WriteByte (TE_GRENADE_EXPLOSION);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

//	debugmsg("Molotov cocktail splash!\n");
	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	for (i=0;i<8;i++) {
		shrapnel = G_Spawn();
		// Throw shrapnel in all eight quarter hemispheres.
		temp[0]=random() * ((2*(i & 1))-1);
		temp[1]=random() * ((2*(i & 2))-1);
		temp[2]=random() * ((2*(i & 4))-1);
		VectorSet(temp, crandom(), crandom(), crandom());
		VectorMA(ent->s.origin, 5, temp, shrapnel->s.origin);
		VectorNormalize(temp);
		VectorSet(shrapnel->velocity, temp[0]*500, temp[1]*500, temp[2]*500);
		VectorCopy (shrapnel->velocity, shrapnel->movedir);
		vectoangles (shrapnel->velocity, shrapnel->s.angles);
		shrapnel->movetype = MOVETYPE_FLYMISSILE;
		shrapnel->clipmask = MASK_SHOT;
		shrapnel->solid = SOLID_BBOX;
		//shrapnel->s.effects |= EF_ROCKET;
		VectorSet(shrapnel->mins, -4, -4, -4);	// Bounding box
		VectorSet(shrapnel->maxs, 4, 4, 4);
		shrapnel->monsterinfo.aiflags = AI_NOSTEP;
		shrapnel->s.modelindex = gi.modelindex ("models/objects/debris2/tris.md2");
		shrapnel->owner = ent->owner;
		shrapnel->touch = Shrapnel_Touch;
		shrapnel->nextthink = level.time + 5;
		shrapnel->think = Shrapnel_Think;
		shrapnel->dmg = 5;
		shrapnel->classname = "shrapnel";
		shrapnel->spawnflags = 1;
		gi.linkentity (shrapnel);
	}
	G_FreeEdict (ent);
}

static void Cocktail_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	debugmsg("Cocktail killed by: %s (%s)\n",inflictor->classname, attacker->classname);
//	if (self->enemy) {
//		debugmsg("Cocktail's enemy: %s\n",self->enemy->classname);
//	}
	self->enemy=NULL;
	Cocktail_Explode(self);
}

static void Cocktail_Think (edict_t *ent) {
	vec3_t	temp, temp2;

	if (ent->waterlevel && (ent->watertype&(CONTENTS_WATER|CONTENTS_SLIME))) {
		G_FreeEdict (ent);
		return;
	}
	ent->delay++;
	ent->nextthink = level.time + 0.1;
	ent->s.frame = ent->s.frame + 1;
	if (ent->s.frame>3) {
		ent->s.frame=0;
	}
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	VectorSet(temp,0,0,5);
	VectorAdd(temp,ent->s.origin,temp2);
	gi.WritePosition (temp2);
	VectorSet(temp,0,0,100);
	gi.WriteDir (temp);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	if (ent->delay>50) {
		Cocktail_Explode(ent);
	}
}

void Cocktail_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

//	debugmsg("Cocktail touch: %s\n",other->classname);
	if (other->takedamage)
	{
		if (ent->owner->client)
			ent->owner->client->resp.standard[GSTAT_HITS]++;
		ent->enemy = other;
	} else if (other->flags & FL_ANTIBOT)
		ent->owner->client->pers.hit_antibot ++;	//antibot code

	Cocktail_Explode (ent);
}



void Cocktail_Touch2 (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t temp;
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

//	debugmsg("Cocktail touch: %s\n",other->classname);
	if (other->takedamage)
	{
		if (ent->owner->client)
			ent->owner->client->resp.standard[GSTAT_HITS]++;
		ent->enemy = other;
		Cocktail_Explode (ent);
	}

	VectorSet(ent->velocity, 0, 0, 0);
	VectorSet(temp,0,0,5);
//	VectorAdd(ent->s.origin,temp,ent->s.origin);
//	VectorSet(temp,0,0,10);
}


void fire_cocktail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*cocktail;
	vec3_t	dir;
	vec3_t	forward, right, up;

	if (self->client)
		self->client->resp.standard[GSTAT_SHOTS]++;
	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	cocktail = G_Spawn();
	VectorCopy (start, cocktail->s.origin);
	VectorScale (aimdir, speed, cocktail->velocity);
	VectorMA (cocktail->velocity, 200 + crandom() * 10.0, up, cocktail->velocity);
	VectorMA (cocktail->velocity, crandom() * 10.0, right, cocktail->velocity);
	VectorSet (cocktail->avelocity, 300, 300, 300);
	cocktail->movetype = MOVETYPE_BOUNCE;
	cocktail->clipmask = MASK_SHOT;
	cocktail->solid = SOLID_BBOX;
	cocktail->s.effects |= EF_ROCKET;
	cocktail->takedamage = DAMAGE_AIM;
	cocktail->health = 5;
	VectorSet(cocktail->mins, -8, -8, -8);	// Bounding box
	VectorSet(cocktail->maxs, 8, 8, 8);
	cocktail->mass = 2;
	cocktail->die = Cocktail_Die;
	cocktail->monsterinfo.aiflags = AI_NOSTEP;
	cocktail->s.modelindex = gi.modelindex ("models/objects/bottle/tris.md2");
	cocktail->owner = self;
	cocktail->touch = Cocktail_Touch;
	cocktail->nextthink = level.time + timer;
	cocktail->think = Cocktail_Explode;
	cocktail->dmg = damage;
	cocktail->dmg_radius = damage_radius;
	cocktail->classname = "cocktail";
	if (held)
		cocktail->spawnflags = 3;
	else
		cocktail->spawnflags = 1;
	cocktail->s.sound = gi.soundindex("weapons/Bfg__l1a.wav");
	cocktail->delay=0;

	if (timer <= 0.0)
		Cocktail_Explode (cocktail);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (cocktail);
	}
}

void drop_cocktail (edict_t *self, vec3_t start, int damage, float damage_radius)
{
	edict_t	*cocktail;

	if (self->client)
		self->client->resp.standard[GSTAT_SHOTS]++;
	cocktail = G_Spawn();
	VectorCopy (start, cocktail->s.origin);
	VectorSet(cocktail->velocity, 0, 0, -100);
	cocktail->movetype = MOVETYPE_BOUNCE;
	cocktail->clipmask = MASK_SHOT;
	cocktail->solid = SOLID_BBOX;
	cocktail->s.effects |= EF_ROCKET;
	cocktail->takedamage = DAMAGE_AIM;
	cocktail->health = 5;
	VectorSet(cocktail->mins, -4, -4, -5);	// Bounding box
	VectorSet(cocktail->maxs, 4, 4, 10);
	cocktail->mass = 2;
	cocktail->monsterinfo.aiflags = AI_NOSTEP;
	cocktail->die = Cocktail_Die;
	cocktail->s.modelindex = gi.modelindex ("models/objects/bottle/tris.md2");
	cocktail->owner = self;
	cocktail->touch = Cocktail_Touch2;
	cocktail->nextthink = level.time + .1;
	cocktail->think = Cocktail_Think;
	cocktail->dmg = damage;
	cocktail->dmg_radius = damage_radius;
	cocktail->classname = "cocktail";
	cocktail->spawnflags = 1;
	cocktail->s.sound = gi.soundindex("weapons/Bfg__l1a.wav");
	cocktail->delay=0;

	gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
	gi.linkentity (cocktail);
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
	} else if (other->flags & FL_ANTIBOT) {
		//antibot code
		ent->owner->client->pers.hit_antibot ++;
	} else {
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
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

/*
=================
fire_hatchet
=================
*/
void hatchet_dissolve (edict_t *ent) {
	G_FreeEdict(ent);
}

void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void drop_make_touchable (edict_t *ent);

void Hatchet_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}
	gi.sound (ent, CHAN_AUTO, gi.soundindex("gunslinger/hatchetImpact.wav") , 1, ATTN_NORM, 0);
//	if (!strcmp(other->classname,"hatchet")) {
//		G_FreeEdict (ent);
//		G_FreeEdict (other);
//		return;
//	}
	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, ent->dmg, 0, MOD_HATCHET);
		if (ent->owner->client)
			ent->owner->client->resp.standard[GSTAT_HITS]++;
		G_FreeEdict (ent);
	} else if (other->flags & FL_ANTIBOT) {
		other->owner->client->pers.hit_antibot ++;	//antibot code
	} else {
		int aplane=0;
		int ahatchet=0;
		gitem_t *item;
		if (plane) {
			vec3_t norm;

			vectoangles(plane->normal, norm);
			aplane=((int)norm[0]+180)%360;
			while (aplane<0) {
				aplane+=360;
			}
			ahatchet=-((int)ent->s.angles[0]+180)%360;
			while (ahatchet<0) {
				ahatchet+=360;
			}
//			gi_bprintf(PRINT_HIGH, "Plane: %i   Hatchet: %i\n", aplane, ahatchet);
		}
		if ((ahatchet-aplane<30) && (ahatchet-aplane>-90) && !Q_strcasecmp(other->classname, "worldspawn")) {
			ent->movetype = MOVETYPE_NONE;
		}
		item=FindItem("Hatchet");
		Make_Dropped(ent, item);
		ent->clipmask=0;
		ent->dmg=0;
//		debugmsg("Set clipmask\n");
	}
	return;
}

void fire_hatchet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer)
{
	edict_t	*hatchet;
	vec3_t	forward, right, up;

	if (self->client)
		self->client->resp.standard[GSTAT_SHOTS]++;
	hatchet = G_Spawn();

	VectorCopy (start, hatchet->s.origin);
	VectorCopy (aimdir, hatchet->movedir);
	AngleVectors (aimdir, forward, right, up);
	vectoangles (aimdir, hatchet->s.angles);
	VectorScale (aimdir, speed, hatchet->velocity);
	VectorMA (hatchet->velocity, 150 + crandom() * 10.0, up, hatchet->velocity);
	VectorMA (hatchet->velocity, crandom() * 10.0, right, hatchet->velocity);
	VectorSet (hatchet->avelocity, speed*2, 0, 0);
	hatchet->movetype = MOVETYPE_TOSS;
	hatchet->clipmask = MASK_SHOT;
	hatchet->solid = SOLID_BBOX;
    VectorSet(hatchet->mins, -7, -7, -7);
	VectorSet(hatchet->maxs, 7, 7, 7);
	hatchet->monsterinfo.aiflags = AI_NOSTEP;
//	hatchet->s.modelindex = gi.modelindex ("models/objects/hatchet/tris.md2");
	gi.setmodel (hatchet, "models/objects/hatchet/tris.md2");
	hatchet->owner = self;
	hatchet->touch = Hatchet_Touch;
	hatchet->dmg = damage;
	hatchet->gravity = 0.5;
	hatchet->classname = "hatchet";

	gi.linkentity (hatchet);
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
			if (((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client) ||
				(tr.ent->solid == SOLID_BBOX)) && (!tr.ent->flags & FL_ANTIBOT))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_SHARPS_RIFLE);
			else if (tr.ent->flags & FL_ANTIBOT) {
				//antibot code
				self->client->pers.hit_antibot ++;
			}
		}

		VectorCopy (tr.endpos, from);
	}

	// send gun puff / flash
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_RAILTRAIL);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);
//	gi.multicast (start, MULTICAST_PHS);
	if (water)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_RAILTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PHS);
	}

	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}


/*
=================
fire_cannon
=================
*/
void cannonball_explode (edict_t *ent)
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
		mod = MOD_CANNON_EFFECT;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	mod = MOD_CANNON_BLAST;
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
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void cannonball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (other->takedamage)
	{
		if (ent->owner->client)
			ent->owner->client->resp.standard[GSTAT_HITS]++;
		ent->enemy = other;
	}

	cannonball_explode (ent);
}


void fire_cannon (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float damage_radius)
{
	edict_t	*cannon;
	vec3_t	dir;
	vec3_t	forward, right, up;

	if (self->client)
		self->client->resp.standard[GSTAT_SHOTS]++;
	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	cannon = G_Spawn();
	VectorCopy (start, cannon->s.origin);
	VectorScale (aimdir, speed, cannon->velocity);
	VectorMA (cannon->velocity, 200 + crandom() * 10.0, up, cannon->velocity);
//	VectorMA (cannon->velocity, crandom() * 10.0, right, cannon->velocity);
//	VectorSet (cannon->avelocity, 300, 300, 300);
	cannon->movetype = MOVETYPE_TOSS;
	cannon->clipmask = MASK_SHOT;
	cannon->solid = SOLID_BBOX;
	cannon->s.effects |= EF_GRENADE;
	VectorClear (cannon->mins);
	VectorClear (cannon->maxs);
	cannon->s.modelindex = gi.modelindex ("models/objects/cannonball/tris.md2");
	cannon->owner = self;
	cannon->touch = cannonball_touch;
	cannon->nextthink = level.time + 60;
	cannon->think = cannonball_explode;
	cannon->dmg = damage;
	cannon->dmg_radius = damage_radius;
	cannon->classname = "cannonball";
	cannon->s.sound = gi.soundindex("gunslinger/ball.wav");
	gi.linkentity (cannon);
}

void fire_cannon2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float damage_radius)
{
	edict_t	*cannon;
	vec3_t	dir, dir2;
	vec3_t	forward, right, up;
	int i;
	float r, u;
//	speed=500;

	if (self->client)
		self->client->resp.standard[GSTAT_SHOTS]+=3;
	vectoangles (aimdir, dir2);
	AngleVectors (dir2, forward, right, up);

	for (i=0;i<3;i++) {
		cannon = G_Spawn();

		r = crandom()/4;	// Horizontal spread
		u = crandom()/4;	// Vertical spread
		VectorMA (forward, r, right, dir);
		VectorMA (dir, u, up, dir);

		VectorNormalize(dir);
		VectorCopy (start, cannon->s.origin);
		VectorScale (dir, speed, cannon->velocity);
		VectorMA (cannon->velocity, 200 + crandom() * 10.0, up, cannon->velocity);

//		VectorScale (aimdir, speed, cannon->velocity);
//		VectorMA (cannon->velocity, 200 + crandom() * 10.0, up, cannon->velocity);
//		VectorMA (cannon->velocity, crandom() * 10.0, right, cannon->velocity);
//		VectorSet (cannon->avelocity, 300, 300, 300);
		cannon->movetype = MOVETYPE_TOSS;
		cannon->clipmask = MASK_SHOT;
		cannon->solid = SOLID_BBOX;
		cannon->s.effects |= EF_GRENADE;
		VectorClear (cannon->mins);
		VectorClear (cannon->maxs);
		cannon->s.modelindex = gi.modelindex ("models/objects/cannonball/tris.md2");
		cannon->owner = self;
		cannon->touch = cannonball_touch;
		cannon->nextthink = level.time + 60;
		cannon->think = cannonball_explode;
		cannon->dmg = damage;
		cannon->dmg_radius = damage_radius;
		cannon->classname = "cannonball";
		gi.linkentity (cannon);
	}

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

	//antibot code
	if (other->flags & FL_ANTIBOT)
		self->client->pers.hit_antibot++;

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
