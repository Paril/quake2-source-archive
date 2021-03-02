#include "../g_local.h"
#include "../m_player.h"

void plasma_explode (edict_t *self)
{
	int mod;
	vec3_t origin;

	// calculate position for the explosion entity
	VectorMA (self->s.origin, -0.02, self->velocity, origin);

	if (self->enemy && self->enemy->takedamage)
	{
		// Paril, Fix!
		mod = MOD_PLASMA;
		T_Damage (self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, self->dmg, 1, DAMAGE_ENERGY, mod);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_FLECHETTE);
		gi.WritePosition (origin);
	//	if (!plane)
			gi.WriteDir (vec3_origin);
		//else
		//	gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PHS);

		gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/plasma_expl.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_FLECHETTE);
		gi.WritePosition (origin);
	//	if (!plane)
			gi.WriteDir (vec3_origin);
		//else
		//	gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PHS);

		gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/plasma_expl.wav"), 1, ATTN_NORM, 0);
	}

	if (gi.pointcontents(self->s.origin) == CONTENTS_WATER || self->waterlevel > 0)
	{
		// Water explode, radius damage!

		edict_t *bl = NULL;

		while (bl = findradius(bl, self->s.origin, 400))
		{
			if (!bl->inuse)
				continue;
			if (!bl->takedamage)
				continue;
			if (bl->solid != SOLID_BBOX)
				continue;
			if (!bl->waterlevel > 0)
				continue;

			T_Damage (bl, self, self->owner, vec3_origin, self->s.origin, vec3_origin, 5, 7, DAMAGE_ENERGY, MOD_PLASMA_SPLASH); // FIXME Mod
		}
	}

	G_FreeEdict(self);
}

void plasma_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	int		mod;
//	vec3_t origin;

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
		self->enemy = other;
	else
		self->enemy = NULL;

	plasma_explode(self);

	// Paril: Water damage from hit?
}

void FindWater (edict_t *self)
{
	//gi.dprintf ("%i\n", gi.pointcontents(self->s.origin));

	if (gi.pointcontents(self->s.origin) == CONTENTS_WATER)
	{
		plasma_explode(self);
		return;
	}	
	if (self->waterlevel > 0)
	{
		plasma_explode(self);
		return;
	}

	self->nextthink = level.time + 0.1;
}

void fire_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bolt;
	trace_t	tr;

	damage /= 3;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= EF_BLUEHYPERBLASTER;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = ModelIndex ("models/proj/plasma/tris.md2");
	bolt->owner = self;
	bolt->touch = plasma_touch;
	bolt->nextthink = level.time + 0.1;
	bolt->think = FindWater;
	// Paril: Water damage think?
	bolt->dmg = damage;
	bolt->classname = "plasma";
	bolt->can_teleport_through_destinations = 1;
	gi.linkentity (bolt);
	bolt->s.renderfx |= RF_IR_VISIBLE;

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	


void Plasma_Fire (edict_t *ent, vec3_t g_offset, int damage)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	vec3_t v;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;


	v[0] = ent->client->v_angle[PITCH] += (rndnum (0, 2) - rndnum (0, 2));
	v[1]   = ent->client->v_angle[YAW] += (rndnum (0, 2) - rndnum (0, 2));
	v[2]  = ent->client->v_angle[ROLL] += (rndnum (0, 2) - rndnum (0, 2));
	AngleVectors (v, forward, NULL, NULL);
	fire_plasma (ent, start, forward, damage/2, 1000);
	/*v[0] = ent->client->v_angle[PITCH] += (rndnum (0, 6) - rndnum (0, 6));
	v[1]   = ent->client->v_angle[YAW] += (rndnum (0, 6) - rndnum (0, 6));
	v[2]  = ent->client->v_angle[ROLL] += (rndnum (0, 6) - rndnum (0, 6));
	AngleVectors (v, forward, NULL, NULL);
	fire_plasma (ent, start, forward, damage, 800);*/

	gi.sound (ent, CHAN_WEAPON, SoundIndex ("weapons/plasma.wav"), 1, ATTN_NORM, 0);
//	gi.sound (ent, CHAN_WEAPON, SoundIndex ("weapons/plasma.wav"), 1, ATTN_NORM, 0.06);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_Plasma_Fire (edict_t *ent)
{
//	vec3_t		start;
//	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	float rotation;

	if (deathmatch->value)
		damage = 30;
	else
		damage = 25;

	rotation = (ent->client->ps.gunframe - 5) * 2*M_PI/6;
	offset[0] = -4 * sin(rotation);
	offset[1] = 0;
	offset[2] = 4 * cos(rotation);

	Plasma_Fire (ent, offset, damage);
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
	
	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - 1;
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - 1;
		ent->client->anim_end = FRAME_attack8;
	}

		ent->client->ps.gunframe++;
}

void Weapon_Plasma (edict_t *ent)
{
	static int	pause_frames[]	= {9, 10, 0};
	static int	fire_frames[]	= {3, 4, 0};

	Weapon_Generic (ent, 2, 8, 11, 14, pause_frames, fire_frames, Weapon_Plasma_Fire);

	if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
	{
		if (ent->client->ps.gunframe == 5)
			ent->client->ps.gunframe = 3;
	}
}
