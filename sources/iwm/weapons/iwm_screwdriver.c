#include "../g_local.h"
#include "../m_player.h"

void Stick(edict_t *ent, edict_t *other)
{
	if (ent->movetype == MOVETYPE_STUCK)
		return;
	
	ent->movetype = MOVETYPE_STUCK;
	VectorClear(ent->velocity);
	VectorClear(ent->avelocity);
	ent->stuckentity = other;
	VectorCopy(other->s.origin,ent->oldstuckorigin);
	VectorCopy(other->s.angles,ent->oldstuckangles);
	// Paril: Fixes the impeding movement and jump bug.
	ent->solid = SOLID_NOT;

	ent->nextthink = level.time + 3;
	ent->think = G_FreeEdict;
}

void screwdriver_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		mod = MOD_SCREWDRIVER;
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

	Stick (self, other);
}

void Calc_Arc (edict_t *ent)
{
        vec3_t move;

        vectoangles(ent->velocity, move);
        VectorSubtract(move, ent->s.angles, move);

        move[0] = fmod((move[0] + 180), 360) - 180;
        move[1] = fmod((move[1] + 180), 360) - 180;
        move[2] = fmod((move[2] + 180), 360) - 180;
        VectorScale(move, 1/FRAMETIME, ent->avelocity);
}

void KnifeArc (edict_t *ent)
{
        ent->nextthink = 0;
        //ent->owner = NULL;
        ent->think = NULL;
        ent->movetype = MOVETYPE_BOUNCE;
        ent->prethink = Calc_Arc;
        gi.linkentity (ent);
}

void fire_screwdriver (edict_t *attacker, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*arrow;
	trace_t	tr;

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
	arrow->s.modelindex = ModelIndex ("models/proj/screwdriver/tris.md2");
	arrow->owner = attacker;
	arrow->touch = screwdriver_touch;
	arrow->nextthink = level.time + .1;
	arrow->think = KnifeArc;
	arrow->dmg = damage;
	arrow->classname = "screwdriver";
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


void Weapon_ScrewDriver_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	vec3_t		offset;
	int damage;
	vec3_t		v;

	damage = 20;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	if (is_quad)
	{
		damage *= 4;
	}

	v[PITCH] = ent->client->v_angle[PITCH] - 10;
	v[YAW]   = ent->client->v_angle[YAW];
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_screwdriver (ent, start, forward, damage, 1200);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->client->ps.gunframe++;

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

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
}

void Weapon_ScrewDriver (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_ScrewDriver_Fire);
}
