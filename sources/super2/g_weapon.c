#include "g_local.h"

// SH
static void Grenade_Explode (edict_t *);
void SpawnDamage (int, vec3_t, vec3_t, int);
// \SH

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
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick, int mod)
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
		if (((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client)))
			tr.ent = self->enemy;
	}

	AngleVectors(self->s.angles, forward, right, up);
	VectorMA (self->s.origin, range, forward, point);
	VectorMA (point, aim[1], right, point);
	VectorMA (point, aim[2], up, point);
	VectorSubtract (point, self->enemy->s.origin, dir);

	// do the damage
	T_Damage (tr.ent, self, self, dir, point, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK|DAMAGE_BOOT, mod);

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
// SH
	if (self->client && (self->client->pers.special == SNIPER || self->client->pers.combo == C_ASS))
	{
		hspread = 0;
		vspread = 0;
		if (VectorLength (self->velocity) == 0)
		{
			damage *= 2.5;
		}
		else
		{
			damage *= 1.8;
		}
	}
// \SH
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
}


// SH
void fire_fastbop (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod, int effect)
{
	int		i;

	for (i = 0; i < count; i++)
		fire_lead (self, start, aimdir, damage, kick, effect, hspread, vspread, mod);
}

void fire_explobop (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int mod)
{
	trace_t	tr;
	vec3_t	end;

	VectorMA (start, 2000, aimdir, end);

	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);

	blast_fire (self, tr.endpos, damage, mod);
}

// \SH

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
}


/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int mod;
	
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);
// SH
	if (self->flags & FL_OPTIC)
	{
		T_RadiusDamage (self, self->owner, self->radius_dmg, self->owner, 50, DAMAGE_RADIO, MOD_BOPGUN);
	}
// \SH
	if (other->takedamage)
	{
		if (self->spawnflags & 1)
			mod = MOD_HYPERBLASTER;
		else
			mod = MOD_BLASTER;

		if (self->owner->client && self->owner->client->pers.passive == MAGE)
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ARMOR_ONLY, mod);
		else
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
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->classname = "bolt";
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);

	if (!(self->client && self->client->pers.special == ISHOTS))  // SH
	{
		bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	}
// SH	
	if (self->client && self->client->pers.passive == MAGE)
	{
		int ran = rand()%12;

		VectorAdd (start, bolt->velocity, bolt->s.old_origin);
		bolt->s.renderfx |= RF_BEAM;
		bolt->s.modelindex = 1;      
		bolt->s.frame = 3;
// set the color
		if (ran == 0)
			bolt->s.skinnum = 0xf2f2f0f0;
		else if (ran == 1)                                                                                           // on blue team
			bolt->s.skinnum = 0xf3f3f1f1;
		else if (ran == 2)		// green
			bolt->s.skinnum = 0xd0d1d2d3;
		else if (ran == 3)		// blue
			bolt->s.skinnum = 0xf3f3f1f1;
		else if (ran == 4)		// yellow
			bolt->s.skinnum = 0xdcdddedf;
		else if (ran == 5)		// orange
			bolt->s.skinnum = 0xe0e1e2e3;
		else if (ran == 6)		// another orange
			bolt->s.skinnum = 0xe2e5e3e6;
		else if (ran == 7)		// color mix
			bolt->s.skinnum = 0xd0f1d3f3;
		else if (ran == 8)		// red outer, blue inner
			bolt->s.skinnum = 0xf2f3f0f1;
		else if (ran == 9)		// blue outer, red inner
			bolt->s.skinnum = 0xf3f2f1f0;
		else if (ran == 10)		// yellow/green
			bolt->s.skinnum = 0xdad0dcd2;
		else if (ran == 11)		// green/yellow
			bolt->s.skinnum = 0xd0dad2dc;
	}
	else
// \SH
	{
		bolt->s.effects |= effect;  //SH
		;
	}

	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	if (hyper)
		bolt->spawnflags = 1;
// SH
	if (self->client && self->client->pers.special == BOPGUN)
	{
		bolt->flags |= FL_OPTIC;
		bolt->radius_dmg = (int)(damage / 100);
		bolt->dmg = damage%100;
	}

// \SH
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

// SH
void Freeze_Explode (edict_t *ent)
{
	vec3_t		origin;
	edict_t		*cur;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/xpld_wat.wav"), 1, ATTN_NORM, 0);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	cur = world;

	while (cur != NULL) //findradius (cur, origin, ent->dmg_radius) != NULL)
	{
//		if (cur && cur->client)
//		{
//			gi.centerprintf (cur, "You are frozen!\n");
//		}

		if (cur && (cur->client || cur->svflags & SVF_MONSTER))
		{
			if (cur->client && cur->client->pers.passive == LFORM)
			{
				cur->freezetime = level.time + 3;
			}
//			else if (cur->client && cur->client->pers.passive == FFORM)
//			{
//				T_Damage (cur, ent, ent->owner, ent->s.origin, ent->s.origin, cur->s.origin, 80, 100, DAMAGE_NO_PROTECTION);
//			}
			else if (cur == ent->owner)
			{
				cur->freezetime = level.time + 1;
			}
			else if (cur->client && cur->client->pers.passive == DENSE)
			{
				cur->freezetime = 0;
			}
			else
			{
				cur->freezetime = level.time + 1.5;
			}
			gi.sound(cur, CHAN_AUTO, gi.soundindex("powers/freezegren.wav"), 1, ATTN_NORM, 0);
		}
		cur = findradius (cur, origin, ent->dmg_radius);
	}
/*
	gi.WriteByte (svc_temp_entity);
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
*/
	make_ball (B_FREEZE, ent->s.origin, ent);

//	G_FreeEdict (ent);
}

void Freeze_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	Freeze_Explode (ent);
}


void Prox_Think (edict_t *ent)
{
	int detect = 0;
	edict_t	*cur = NULL;
	vec3_t	dir = {0,0,0};

	ent->nextthink = level.time + 0.1;

	cur = world;
//	cur = findradius (cur, ent->s.origin, ent->dmg_radius);
	while (cur != NULL)//findradius (cur, ent->s.origin, ent->dmg_radius) != NULL)
	{
		if (cur && ((cur->client && !(cur->flags & FL_OBSERVER) && !(CTFSameTeam (cur, ent->owner))) 
			|| cur->svflags & SVF_MONSTER 
			|| !strcmp(cur->classname, "misc_teleporter_dest") 
			|| !strcmp(cur->classname, "info_player_deathmatch") 
			|| !strcmp(cur->classname, "item_flag_team1") 
			|| !strcmp(cur->classname, "item_flag_team2")))
		{
			detect = 1;
			VectorSubtract (cur->s.origin, ent->s.origin, dir);
			break;
		}
		cur = findradius (cur, ent->s.origin, ent->dmg_radius);
	}

	if ((ent->wait < level.time || detect))
	{
		ent->flags |= FL_OPTIC;
		SpawnDamage (TE_SPARKS, ent->s.origin, dir, 200);
		gi.sound(ent, CHAN_AUTO, gi.soundindex("powers/prox1.wav"), 1, ATTN_NORM, 0);

		ent->think = Grenade_Explode;
		ent->nextthink = level.time + 0.4;
		return;
	}
	VectorScale (ent->avelocity, 1.05, ent->avelocity);
}

void Prox_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	dir;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	VectorSubtract (ent->owner->s.origin, ent->s.origin, dir);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("world/spark1.wav"), 1, ATTN_NORM, 0);
	SpawnDamage (TE_SPARKS, ent->s.origin, dir, 200);
	ent->think = Prox_Think;
	ent->nextthink = level.time + 0.2;
	VectorClear (ent->velocity);
	ent->movetype = MOVETYPE_FLY;
	gi.linkentity (ent);
}
// \SH

/*
=================
fire_grenade
=================
*/
static void Grenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod = MOD_GRENADE;
// SH
	if (ent->style == 2)
		mod = MOD_PROX;
// \SH	
	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, 0, mod);

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
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
}

static void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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

	Grenade_Explode (ent);
}

void BecomeExplosion1_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	BecomeExplosion1(self);
}

void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, int type)
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
//	grenade->s.effects |= EF_GRENADE;  // SH
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	if (!(self->client && self->client->pers.special == ISHOTS))  // SH
	{
		if (type == 2)
		{
			grenade->s.modelindex = gi.modelindex ("models/super2/prox/tris.md2");
			grenade->style = 2;
		}
		else
		{
			grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
		}
	}
	grenade->owner = self;
	grenade->nextthink = level.time + timer;
// SH	
	if (type == 0)
	{
		grenade->touch = Grenade_Touch;
		grenade->think = Grenade_Explode;
	}
	else if (type == 1)
	{
		grenade->touch = Freeze_Touch;
		grenade->think = Freeze_Explode;
	}
	else if (type == 2)
	{
		grenade->touch = Prox_Touch;
		grenade->think = Prox_Think;
		grenade->nextthink = 0;
		grenade->wait = level.time + timer;
		VectorSet (grenade->mins, -3, -3, -3);
		VectorSet (grenade->maxs, 3, 3, 3);
		grenade->takedamage = DAMAGE_AIM;
		grenade->health = 100;
		grenade->die = BecomeExplosion1_die;
	}
// \SH
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

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
//	grenade->s.effects |= EF_GRENADE;  // SH
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	if (!(self->client && self->client->pers.special == ISHOTS))  // SH
	{
		grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	}
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
	int			mod;

	if (!strcmp(ent->classname, "super shot"))
		mod = MOD_HAPPY;
	else
		mod = MOD_ROCKET;

	if (other == ent->owner)
		return;

	if (!strcmp (other->classname, "super shot"))
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

	if (other->takedamage && ent->dmg)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, 0, mod);
	}
	else if (ent->dmg)
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
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, 0, mod);
	}
	else if (!ent->dmg)
	{
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, 0, mod);
	}

// SH
	if (!strcmp (ent->classname, "robo rocket"))
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (ent->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	else
// \SH
	{
		gi.WriteByte (svc_temp_entity);
		if (ent->waterlevel)
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
		gi.WritePosition (origin);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (ent);
}
// SH

void robo_rocket_think (edict_t *ent)
{
	edict_t	*cur;
	trace_t	tr;
	vec3_t	dir;

	if (ent->goalentity == NULL)
	{
//gi.cprintf (ent->owner, PRINT_HIGH, "robo looking for target\n");
		cur = findradius (world, ent->s.origin, 200);
	
		while (cur != NULL)
		{
			if (cur->takedamage && cur != ent->owner && cur->solid != SOLID_BSP && cur->health > 0 && (cur->client || cur->svflags & SVF_MONSTER))
			{
//gi.cprintf (ent->owner, PRINT_HIGH, "robo found a target, %s\n", cur->classname);
				tr = gi.trace (cur->s.origin, ent->mins, ent->maxs, ent->s.origin, ent, MASK_SHOT);
				if (tr.fraction == 1.0)
				{
//gi.cprintf (ent->owner, PRINT_HIGH, "robo target aquired: %s", cur->classname);
					ent->goalentity = cur;
					break;
				}
			}
			cur = findradius (cur, ent->s.origin, 200);
		}
	}
	
	if (ent->goalentity)
	{
//gi.cprintf (ent->owner, PRINT_HIGH, "robo seek target\n");
		VectorSubtract (ent->s.origin, ent->goalentity->s.origin, dir);
		VectorMA (ent->velocity, -2, dir, ent->velocity);
	}

	vectoangles (ent->velocity, ent->s.angles);

	ent->nextthink = level.time + FRAMETIME;
}

// \SH
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
// SH
	if (self->client && self->client->pers.passive == ROBOT)
	{
		if (self->wait == 1)
			rocket->s.renderfx |= RF_SHELL_RED;
		else
			rocket->s.renderfx |= (RF_SHELL_GREEN);

		rocket->classname = "robo rocket";
		rocket->s.effects |= EF_COLOR_SHELL;
//		rocket->s.renderfx |= (RF_SHELL_GREEN);  // SH
		rocket->think = robo_rocket_think;
		rocket->nextthink = level.time + FRAMETIME;
		speed *= 0.8;
	}
	else
// \SH
	{
		rocket->s.effects |= EF_ROCKET;  // SH
		rocket->classname = "rocket";
		rocket->nextthink = level.time + 8000/speed;
		rocket->think = G_FreeEdict;
	}

	VectorScale (dir, speed, rocket->velocity);
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	if (!(self->client && self->client->pers.special == ISHOTS))  // SH
	{
		rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	}
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");

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
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
		}

		VectorCopy (tr.endpos, from);
	}

	// send gun puff / flash
	if (!(self->client && self->client->pers.special == ISHOTS))  // SH
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_RAILTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);
//		gi.multicast (start, MULTICAST_PHS);
	}

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
			gi.multicast (ent->s.origin, MULTICAST_PVS);
			T_Damage (ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_BFG_BLAST);
		}
	}

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 5)
		self->think = G_FreeEdict;
}

void bfg_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*ent;
	edict_t	*next;

	if (other == self->owner)
		return;

	// clean up laser entities
	for (ent = self->teamchain; ent; ent = next)
	{
		next = ent->teamchain;
		G_FreeEdict (ent);
	}

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
	T_RadiusDamage(self, self->owner, 200, other, 100, 0, MOD_BFG_BLAST);

	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorMA (self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
	VectorClear (self->velocity);
	if (!(self->client && self->client->pers.special == ISHOTS))  // SH
	{
		self->s.modelindex = gi.modelindex ("sprites/s_bfg3.sp2");
	}
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


extern void target_laser_start (edict_t *self);

void bfg_think (edict_t *self)
{
	edict_t	*ent;
	edict_t	*next;
//	edict_t	*check;
//	edict_t	*beam;
	vec3_t	v;
	float	distance;

	ent = self->teamchain;
	self->teamchain = NULL;

	// go through the old list; re-add good ones and free one the have gone out of range
	while (ent)
	{
		next = ent->teamchain;

		VectorSubtract(ent->enemy->s.origin, self->s.origin, v);
		distance = VectorLength(v);
		if (distance <= 256)
		{
			ent->teamchain = self->teamchain;
			self->teamchain = ent;
		}
		else
		{
			G_FreeEdict (ent);
		}

		ent = next;
	}
/*
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

		// see if this is already on the list
		for (check = self->teamchain; check; check = check->teamchain)
			if (check->enemy == ent)
				break;
		if (check)
			continue;

		// create a new laser
		beam = G_Spawn();
		beam->flags |= FL_TEAMSLAVE;
		beam->teamchain = self->teamchain;
		beam->teammaster = self;
		self->teamchain = beam;
		beam->owner = self->owner;
		beam->spawnflags = 1 | 4;
		beam->enemy = ent;
		if (deathmatch->value)
			beam->dmg = 5;
		else
			beam->dmg = 10;
		beam->classname = "bfg laser";
		target_laser_start (beam);
		beam->movetype = MOVETYPE_FLYMISSILE;
		gi.linkentity (beam);
	}
*/
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
	if (!(self->client && self->client->pers.special == ISHOTS))  // SH
	{
		bfg->s.modelindex = gi.modelindex ("sprites/s_bfg1.sp2");
	}
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
