#include "../g_local.h"

void NukeThink (edict_t *fr)
{
//	int i;
//	int f;
//	edict_t *freezer;
	// Done.
//	float	points;
//	vec3_t	v;
//	vec3_t	dir;

	T_RadiusDamage (fr, fr->owner, 1000, NULL, 1500, MOD_WOMD);

	if (fr->s.frame == 16)
	{
		G_FreeEdict(fr);
		return;
	}

	fr->s.frame++;
	fr->nextthink = level.time + .1;
}

void MakeNukeExplosionAndCloudBecauseItsAVeryBigWeaponAndDoesALotofDamage (edict_t *owner, vec3_t origin, int Backwards)
{
	edict_t *fr;
	vec3_t r;

	fr = G_Spawn();

	VectorCopy (origin, fr->s.origin);

	VectorCopy (origin, fr->tempvector);
	
	/*gi.dprintf ("%i\n", Backwards);

	if (Backwards == 1)
	{
		fr->s.angles[2] = 180;
	}
	else if (Backwards == 2)
	{
		fr->s.angles[0] = 90;
	}
	else if (Backwards == 3)
	{
		fr->s.angles[0] = -90;
	}
	else if (Backwards == 4)
	{
		safe_bprintf (PRINT_HIGH, "Forward\n");
		fr->s.angles[1] = 90;
	}
	else if (Backwards == 5)
	{
		safe_bprintf (PRINT_HIGH, "Backward\n");
		fr->s.angles[1] = -90;
	}*/


	fr->movetype = MOVETYPE_NONE;
	fr->s.frame = 0;
	fr->solid = SOLID_NOT;
	fr->s.modelindex = ModelIndex("models/other/womd/tris.md2");
	fr->owner = owner;
	fr->think = NukeThink;
	fr->nextthink = level.time + .1;
	fr->s.effects |= EF_HYPERBLASTER;
	fr->classname = "womdexpl";

	gi.linkentity (fr);

	VectorCopy (fr->s.origin, r);

	r[2] += 145;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_NUKEBLAST);
	gi.WritePosition (r);
	gi.multicast (r, MULTICAST_ALL);
}

void womd_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
//	int			n;
	//int Backwards;

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

	//gi.dprintf ("%i\n", (int)plane->normal[0]);
	//gi.dprintf ("%i\n", (int)plane->normal[1]);
	//gi.dprintf ("%i\n", (int)plane->normal[2]);
	/*if ((int)plane->normal[0] == 1)
	{
	//	gi.dprintf ("Left\n");
		Backwards = 2;
	}
	else if ((int)plane->normal[0] == -1)
	{
	//	gi.dprintf ("Right\n");
		Backwards = 3;
	}
	else if ((int)plane->normal[1] == 1)
	{
	//	gi.dprintf ("Forward Wall\n");
		Backwards = 4;
	}
	else if ((int)plane->normal[1] == -1)
	{
	//	gi.dprintf ("Backward Wall\n");
		Backwards = 5;
	}
	else if ((int)plane->normal[2] == 0)
	{
	//	gi.dprintf ("Not Backwards\n");
		Backwards = 0;
	}
	else if ((int)plane->normal[2] == -1)
	{
	//	gi.dprintf ("Backwards\n");
		Backwards = 1;
	}*/



	MakeNukeExplosionAndCloudBecauseItsAVeryBigWeaponAndDoesALotofDamage (ent->owner, origin, 0);
	gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/rocklx1a.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict (ent);
}

void fire_womd (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	edict_t	*rocket;

	self->client->pers.womded = 1;
	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = ModelIndex ("models/proj/womd/tris.md2");
	rocket->owner = self;
	rocket->touch = womd_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->s.sound = SoundIndex ("weapons/rockfly.wav");
	rocket->classname = "rocket";
	rocket->can_teleport_through_destinations = 1;
	rocket->s.renderfx |= RF_IR_VISIBLE;

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}
