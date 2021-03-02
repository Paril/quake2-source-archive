#include "../g_local.h"

void neutron_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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

	G_FreeEdict (ent);
}
edict_t *findradius2 (edict_t *from, vec3_t org, float rad);	//find all entities

// UNUSED
// FUBAR
// LOLGSJKG.
void ShotTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->takedamage)
		T_Damage (other, ent, ent->owner, vec3_origin, other->s.origin, vec3_origin, 15, 0, 0, MOD_BLASTER); // FIXME MOD

	G_FreeEdict (ent);
}

void SpewShot (edict_t *star)
{
	edict_t *shot;

	shot = G_Spawn();

	VectorCopy (star->s.origin, shot->s.origin);
	shot->s.angles[0] = shot->velocity[0] = shot->movedir[0] += (rand() % 360 - rand() % 360);
	shot->s.angles[1] = shot->velocity[1] = shot->movedir[1] += (rand() % 360 - rand() % 360);
	shot->s.angles[2] = shot->velocity[2] = shot->movedir[2] += (rand() % 360 - rand() % 360);
	//shot->s.angles[0] = shot->movedir[0];
	//shot->s.angles[1] = shot->movedir[1];
//	shot->s.angles[2] = shot->movedir[2];

	shot->owner = star->owner;
	shot->s.modelindex = ModelIndex("models/proj/neutron/shot.md2");
	shot->thinks[0] = G_FreeEdict;
	shot->nextthinks[0] = level.time + 3;
	shot->solid = SOLID_BBOX;
	shot->movetype = MOVETYPE_FLYMISSILE;
	VectorClear (shot->mins);
	VectorClear (shot->maxs);
	shot->touch = ShotTouch;
	shot->classname = "nshot";

	gi.linkentity(shot);
}
edict_t *SpawnTheActualItem (edict_t *ent, gitem_t *item);
void fire_neutronstar (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);

void NeutronThink (edict_t *ent)
{
	edict_t *blip = NULL; 
	vec3_t blipdir;
	float dist;
	vec3_t h;
	vec3_t add = {rand()%60-rand()%60, rand()%60-rand()%60, rand()%60-rand()%60};

	VectorCopy (ent->s.origin, h);
	VectorAdd (h, add, h);

	while ((blip = findradius2(blip, ent->s.origin, 1200)) != NULL)
	{
		if (blip == ent)
			continue;
		if (blip == ent->owner)
			continue;
		if (blip->item)
			continue;
		//if (Q_stricmp(blip->classname, "nshot") == 0)
		//	continue;

		if (blip->client)
			blip->client->pers.temp_nofall = 1;

		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		dist = VectorLength(blipdir);
		VectorNormalize(blipdir);
		

		VectorMA(blip->velocity, 1.2*(800 - 1200), blipdir, blip->velocity);
		blip->velocity[0] += crandom() * 30;
		blip->velocity[1] += crandom() * 30;
		blip->velocity[2] += random() * 30;
	}

	//ent->s.angles[0] += (rand() % 90 - rand() % 90);
	//ent->s.angles[1] += (rand() % 90 - rand() % 90);
	//ent->s.angles[2] += (rand() % 90 - rand() % 90);

	//SpewShot (ent);
	
	fire_blaster2 (ent->owner, h, tv(0, 0, 0), 15, 2, EF_IONRIPPER, true);
	//fire_blaster2 (ent, h, tv(0, 0, 0), 15, 2, EF_BLASTER, true);
	//fire_blaster2 (ent->owner, h, tv(0, 0, 0), 15, 2, EF_BLASTER, true);
	//fire_rail (ent->owner, h, tv(rand()%600-rand()%600, rand()%600-rand()%600, rand()%600-rand()%600), 50, 50, 5);
	//fire_beachball (ent->owner, h, vec3_origin, 50, 100);
	//if (random() < 0.5)

	//fire_gravrocket (ent->owner, h, tv(0, 0, 0), 55, 355, 435, 43);
	ent->nextthinks[0] = level.time + .1;
}

void AddAnotherParticle (edict_t *ent)
{
	vec3_t h;

	VectorCopy (ent->s.origin, h);

	//fire_blaster (ent->owner, h, tv(0, 0, 0), 45, 2, EF_BLASTER, false);

	ent->nextthinks[3] = level.time + .3;
}

void DestroyIt (edict_t *s)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_EXPLOSION);
	gi.WritePosition (s->s.origin);
	gi.multicast (s->s.origin, MULTICAST_PVS);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (s->s.origin);
	gi.multicast (s->s.origin, MULTICAST_PVS);

	G_FreeEdict (s);
}

void FolOwn (edict_t *ent)
{
	if (ent->owner == NULL || !ent->owner || !ent->owner->inuse)
	{
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (ent->owner->s.origin, ent->s.origin);
	ent->nextthinks[0] = level.time + .1;
}

void FollowingThingy (edict_t *ent)
{
	edict_t *fol;

	fol = G_Spawn();
	fol->owner = ent;
	fol->movetype = MOVETYPE_NONE;
	fol->s.modelindex = ModelIndex("sprites/null.sp2");
	fol->s.solid = SOLID_NOT;
	fol->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
	VectorCopy (ent->s.origin, fol->s.origin);
	fol->thinks[0] = FolOwn;
	fol->nextthinks[0] = level.time + .1;

	gi.linkentity (fol);
}

void fire_neutronstar (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->avelocity[0] += 400;
	rocket->avelocity[1] += 400;
	//rocket->avelocity[2] += 400;
	rocket->movetype = MOVETYPE_WALLBOUNCE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_NOT;
	rocket->s.effects |= EF_BFG;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = ModelIndex ("models/proj/neutron/star.md2");
	rocket->owner = self;
	rocket->touch = neutron_touch;
	rocket->nextthinks[0] = level.time + .1;
	rocket->thinks[0] = NeutronThink;
	rocket->thinks[3] = AddAnotherParticle;
	rocket->nextthinks[3] = level.time + .4;
	rocket->s.frame = 0;
	rocket->thinks[1] = FollowingThingy;
	rocket->nextthinks[1] = level.time + .1;
//	rocket->s.frame = rocket->s.skinnum = 8;
	
	rocket->can_teleport_through_destinations = 1;
	rocket->s.renderfx |= RF_IR_VISIBLE;

	rocket->nextthinks[2] = level.time + 8;
	rocket->thinks[2] = DestroyIt;

	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->classname = "neutronstar";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void Weapon_NeutronStar_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BFG | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		ent->client->ps.gunframe++;

		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);
	fire_neutronstar (ent, start, forward, 0, 250, 0, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_NeutronStar (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, Weapon_NeutronStar_Fire);
}
