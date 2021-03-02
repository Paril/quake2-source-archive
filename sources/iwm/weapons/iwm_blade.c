#include "../g_local.h"

void blade_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	int		mod;
//	vec3_t forward;
//	vec3_t kvel;

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
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_BLADE);
	
		//G_FreeEdict(self);
		return;
	}

	//self->movetype = MOVETYPE_FLYMISSILE;
	G_FreeEdict(self);
}

void Blade_Think (edict_t *s)
{
	vec3_t forward;
//	vec3_t kvel;
	trace_t	tr;
	vec3_t up = {0, 0, -90};
	vec3_t forw;
//	vec3_t vec;
	vec3_t star, end, st;
//	float dist;
	trace_t hurt;
	
	VectorCopy(s->s.origin, star);

	VectorMA(star, 8192, up, end);
	tr = gi.trace(star, NULL, NULL, end, s, MASK_SOLID);

	if ( gi.pointcontents(tr.endpos) == (CONTENTS_SOLID || CONTENTS_WINDOW || CONTENTS_TRANSLUCENT || CONTENTS_LADDER))
	{
		gi.dprintf ("Somehow, the destination went inside a wall. Tell Paril to fix it.\n");
		return;
	}

	VectorCopy (tr.endpos, st);

	st[2] += 24;

	VectorCopy (st, s->s.origin);

	s->s.angles[0] -= 100;

	AngleVectors (s->s.angles, forward, NULL, NULL);

//	s->velocity[2] += 10;
//	ai_stand(s, 10);

	VectorCopy(s->s.origin, star);
	VectorCopy (s->s.angles, forw);

	VectorMA(star, 60, forw, end);
	hurt = gi.trace(star, NULL, NULL, end, s, MASK_SHOT);
	if (tr.ent->takedamage)
		T_Damage (tr.ent, s, s->owner, vec3_origin, tr.ent->s.origin, vec3_origin, 5, 0, 0, MOD_BLADE);

	/*gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (star);
	gi.WritePosition (end);
	gi.multicast (star, MULTICAST_PVS);*/

	s->nextthink = level.time + .1;
}

void fire_blade (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*bolt;
	trace_t	tr;
	vec3_t up = {0, 0, -90};
//	vec3_t vec;
	vec3_t star, end, st;
//	float dist;

	dir[2] = 0;
	
	VectorCopy(self->s.origin, star);

	VectorMA(star, 8192, up, end);
	tr = gi.trace(star, NULL, NULL, end, self, MASK_SOLID);

	if ( gi.pointcontents(tr.endpos) == (CONTENTS_SOLID || CONTENTS_WINDOW || CONTENTS_TRANSLUCENT || CONTENTS_LADDER))
	{
		gi.dprintf ("Somehow, the destination went inside a wall. Tell Paril to fix it.\n");
		return;
	}

	VectorCopy (tr.endpos, st);

	st[2] += 24;


	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE;
	//VectorCopy (start, bolt->s.origin);
	//VectorCopy (start, bolt->s.old_origin);
	dir[2] = 0;
	VectorCopy (st, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, 1000, bolt->velocity);
	bolt->velocity[2] = 0;
	bolt->movetype = MOVETYPE_FLYMISSILE;
	//bolt->clipmask = MASK_SHOT;
	bolt->s.angles[1] += 180;
	//VectorClear(bolt->s.angles);
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	//VectorClear (bolt->mins);
	//VectorClear (bolt->maxs);
	VectorSet (bolt->mins, -18, -12, -22);
	VectorSet (bolt->maxs, 18, 12, 22);
	bolt->s.modelindex = ModelIndex ("models/proj/blade/tris.md2");
	bolt->owner = self;
	bolt->touch = blade_touch;
	bolt->think = Blade_Think;
	bolt->nextthink = level.time + .1;
	bolt->think2 = G_FreeEdict;
	bolt->nextthink2 = level.time + 4;
	bolt->dmg = damage;
	bolt->classname = "blade";
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);
	bolt->can_teleport_through_destinations = 1;
}	

void Weapon_Blade_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 4;
	int			kick = 8;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_blade (ent, start, forward, 20, 60, 0);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SSHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_Blade (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, Weapon_Blade_Fire);
}
