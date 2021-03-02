#include "../g_local.h"

void FollowFFF (edict_t *s)
{
	VectorCopy (s->owner->s.origin, s->s.origin);
	gi.linkentity (s);
	s->nextthink2 = level.time + .1;
}

void SpawnStickyIce (edict_t *ent)
{
	edict_t *freezah;

	freezah = G_Spawn();
	freezah->movetype = MOVETYPE_NONE;
	freezah->solid = SOLID_NOT;
	freezah->s.effects |= EF_COLOR_SHELL;
	freezah->s.renderfx |= RF_SHELL_BLUE | RF_TRANSLUCENT;
	VectorCopy (ent->s.origin, freezah->s.origin);
	freezah->owner = ent;
	freezah->s.modelindex = ModelIndex ("models/other/icec/tris.md2");
	ent->freeze_ent = freezah;
	freezah->nextthink = level.time + 4;
	freezah->think = G_FreeEdict;
	freezah->think2 = FollowFFF;
	freezah->nextthink2 = level.time + .1;

	gi.linkentity (freezah);
}

void freeze_player(edict_t *ent)
{
    if (ent->health <= 0)//5/99
        return;

	// make em frozen
	ent->frozen = 1;

	// set the time till thaw
	ent->frozentime = level.time + 4;	//4 seconds of freeze (was 6)
	gi.sound (ent, CHAN_WEAPON, SoundIndex("weapons/freezer/freeze.wav"), 1, ATTN_NORM, 0);

	// Stick majiggy
	SpawnStickyIce (ent);
}

void unfreeze_player(edict_t *ent)
{
	int i;

	ent->frozen = 0;
	ent->frozentime = level.time - 1;
	gi.sound (ent, CHAN_WEAPON, SoundIndex("world/brkglas.wav"), 1, ATTN_NORM, 0);
	//G_FreeEdict (ent->freeze_ent);
	ent->freeze_ent = NULL;

	// Paril Revision 2.00: throw glass gibs!

	for (i = 0; i < 3+rand()%8; i++)
	{
		ThrowDebris (ent, va("models/objects/glass_gibs/gib%i.md2", 1+rand()%4), 1, ent->s.origin);
	}
}

void T_RadiusFreeze (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
	float	points;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;

	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		points = damage - 0.5 * VectorLength (v);
		if (ent == attacker)
			points = points * 0.5;
		if (points > 0)
		{
			if (CanDamage (ent, inflictor))
			{
				VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
				safe_bprintf (PRINT_HIGH, "Player %s is found.\n", ent->client->pers.netname);
					if (!ent->frozen)
					{
						safe_bprintf (PRINT_HIGH, "Player %s is frozen.. I think\n", ent->client->pers.netname);
						freeze_player(ent);
					}
					else
						safe_bprintf (PRINT_HIGH, "Player %s is frozen already!\n", ent->client->pers.netname);
					return;
			}
		}
	}
}

void FreezeNukeThink (edict_t *fr)
{
//	int i;
	int f = 0;
//	edict_t *freezer;
	// Done.
//	float	points;
//	vec3_t	v;
//	vec3_t	dir;

	T_RadiusDamage (fr, fr->owner, 200, NULL, 700, MOD_BLASTER);

	if (fr->s.frame == 14)
	{
		G_FreeEdict(fr);
		return;
	}
	switch (fr->s.frame)
	{
	case 0:
		f = 0;
		break;
	case 1:
		f = 0;
		break;
	case 2:
		f = 1;
		break;
	case 3:
		f = 1;
		break;
	case 4:
		f = 2;
		break;
	case 5:
		f = 2;
		break;
	case 6:
		f = 3;
		break;
	case 7:
		f = 3;
		break;
	case 8:
		f = 4;
		break;
	case 9:
		f = 4;
		break;
	case 10:
		f = 5;
		break;
	case 11:
		f = 5;
		break;
	case 12:
		f = 6;
		break;
	case 13:
		f = 6;
		break;
	case 14:
		f = 7;
		break;
	case 15:
		f = 7;
		break;
	default:
		f = 0;
		break;
	}

	fr->s.skinnum = f;
	fr->s.frame++;
	fr->nextthink = level.time + .1;
}

void MakeFreezeExplosion (edict_t *owner, vec3_t origin)
{
	edict_t *fr;

	fr = G_Spawn();

	VectorCopy (origin, fr->s.origin);

	VectorCopy (origin, fr->tempvector);
	fr->movetype = MOVETYPE_NONE;
	fr->s.frame = 0;
	fr->solid = SOLID_NOT;
	fr->s.modelindex = ModelIndex("models/other/nuke/tris.md2");
	fr->s.skinnum = 0;
	fr->owner = owner;
	// Causes freeze and skinnumber change.
	fr->think = FreezeNukeThink;
	fr->nextthink = level.time + .1;
	fr->s.renderfx |= (RF_TRANSLUCENT | RF_SHELL_BLUE);
	fr->s.effects |= (EF_BLUEHYPERBLASTER | EF_COLOR_SHELL);
	fr->classname = "freeze";

	gi.linkentity (fr);
}


void freeze_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
//	int			n;

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

	MakeFreezeExplosion(ent->owner, origin);
	gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/rocklx1a.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict (ent);
}

void fire_freeze (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
	rocket->s.effects |= EF_COLOR_SHELL | EF_BLUEHYPERBLASTER | EF_FLAG2;
	rocket->s.renderfx |= RF_SHELL_BLUE;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = ModelIndex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = freeze_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = SoundIndex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	rocket->can_teleport_through_destinations = 1;
	rocket->s.renderfx |= RF_IR_VISIBLE;

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}
