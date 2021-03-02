#include "../g_local.h"
#include "../m_player.h"

void Stick (edict_t *ent, edict_t *other);

void stapler_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		if (!other->die)
			return;
		mod = MOD_STAPLE;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 17, 1, DAMAGE_ENERGY, mod);
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

	Stick (self, other);
}

void Calc_Arc (edict_t *ent);

void KnifeArc (edict_t *ent);

void fire_stapler (edict_t *attacker, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*arrow;
	trace_t	tr;

	start[2] += 8;

	arrow = G_Spawn();
	arrow->svflags = SVF_PROJECTILE;
	VectorNormalize (dir);
	VectorCopy (start, arrow->s.origin);
	VectorCopy (start, arrow->s.old_origin);
	vectoangles (dir, arrow->s.angles);
	VectorScale (dir, speed, arrow->velocity);
	arrow->movetype = MOVETYPE_BOUNCE;
	arrow->clipmask = MASK_SHOT;
	arrow->solid = SOLID_BBOX;
	//VectorClear (arrow->mins);
	//VectorClear (arrow->maxs);
	VectorSet(arrow->mins, -3, -3, -6);
	VectorSet(arrow->maxs, 3, 3, 6);
	arrow->mass = 1;
	arrow->takedamage = DAMAGE_YES;
	arrow->monsterinfo.aiflags = AI_NOSTEP;
	arrow->s.modelindex = ModelIndex ("models/proj/staple/tris.md2");
	arrow->owner = attacker;
	arrow->touch = stapler_touch;
	arrow->nextthink = level.time + .1;
	arrow->think = KnifeArc;
	arrow->dmg = damage;
	arrow->classname = "stapler";
//	arrow->s.frame = arrow->s.skinnum = 1;
	gi.linkentity (arrow);
	arrow->can_teleport_through_destinations = 1;
	arrow->s.renderfx |= RF_IR_VISIBLE;

	if (attacker->client)
		check_dodge (attacker, arrow->s.origin, dir, speed);
	tr = gi.trace (attacker->s.origin, NULL, NULL, arrow->s.origin, arrow, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (arrow->s.origin, -10, dir, arrow->s.origin);
		arrow->touch (arrow, tr.ent, NULL, NULL);
	}
}

void Stapler_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 8;
	int			kick = 2;
	vec3_t		offset;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);
	fire_stapler (ent, start, forward, 12, 1200);

	gi.sound (ent, CHAN_AUTO, SoundIndex("weapons/stapler.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1;
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1;
		ent->client->anim_end = FRAME_attack8;
	}

	ent->client->ps.gunframe++;
}

void Weapon_Stapler (edict_t *ent)
{
	static int	pause_frames[]	= {10, 13, 0};
	static int	fire_frames[]	= {2, 0};

	Weapon_Generic (ent, 1, 5, 10, 13, pause_frames, fire_frames, Stapler_Fire);
}
