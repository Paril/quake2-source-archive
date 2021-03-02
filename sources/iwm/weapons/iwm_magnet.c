#include "../g_local.h"
#include "../m_player.h"

void Magnet_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	/*if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}*/
}
void Calc_Arc (edict_t *ent);

void Repel (edict_t *ent)
{
	edict_t *blip = NULL; 
	vec3_t blipdir;
	float dist;

	while ((blip = findradius(blip, ent->s.origin, 350)) != NULL)
	{
		if (blip == ent)
			continue;
		//if (blip == ent->owner)
		//	continue;
		if (blip->item)
			continue;
		if (Q_stricmp(blip->classname, "block") == 0)
			continue;
		if (!blip->can_teleport_through_destinations && !blip->client && !blip->svflags & SVF_MONSTER)
			continue;

		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		dist = VectorLength(blipdir);
		VectorNormalize(blipdir);
		

		VectorMA(blip->velocity, -(1.2*(80 - 160)), blipdir, blip->velocity);
		blip->velocity[0] -= crandom() * 5;
		blip->velocity[1] -= crandom() * 5;
		blip->velocity[2] -= random() * 5;
		blip->prethink = Calc_Arc;
	}

	ent->nextthink = level.time + .1;
}

void Attract (edict_t *ent)
{
	edict_t *blip = NULL; 
	vec3_t blipdir;
	float dist;

	while ((blip = findradius(blip, ent->s.origin, 350)) != NULL)
	{
		if (blip == ent)
			continue;
		//if (blip == ent->owner)
		//	continue;
		if (blip->item)
			continue;
		if (Q_stricmp(blip->classname, "block") == 0)
			continue;
		if (!blip->can_teleport_through_destinations && !blip->client && !blip->svflags & SVF_MONSTER)
			continue;

		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		dist = VectorLength(blipdir);
		VectorNormalize(blipdir);
		

		VectorMA(blip->velocity, (1.2*(80 - 160)), blipdir, blip->velocity);
		blip->velocity[0] += crandom() * 5;
		blip->velocity[1] += crandom() * 5;
		blip->velocity[2] += random() * 5;
		blip->prethink = Calc_Arc;
	}

	ent->nextthink = level.time + .1;
}

void DestroyMagnets (edict_t *self)
{
	edict_t *h;
	
	for (h = g_edicts+1; h < &g_edicts[game.maxentities]; h++) 
	{
		if (!h->inuse)
			continue;
		if (Q_stricmp(h->classname, "magnet"))
			continue;
		if (h->owner != self)
			continue;

		G_FreeEdict (h);
	}
}

void RepelMagnets (edict_t *self)
{
	edict_t *h;
	
	for (h = g_edicts+1; h < &g_edicts[game.maxentities]; h++) 
	{
		if (!h->inuse)
			continue;
		if (Q_stricmp(h->classname, "magnet"))
			continue;
		if (h->owner != self)
			continue;

		h->s.effects = EF_COLOR_SHELL;
		h->s.renderfx = RF_SHELL_BLUE;

		h->think = Repel;
		h->nextthink = level.time + .1;
	}
}

void AttractMagnets (edict_t *self)
{
	edict_t *h;
	
	for (h = g_edicts+1; h < &g_edicts[game.maxentities]; h++) 
	{
		if (!h->inuse)
			continue;
		if (Q_stricmp(h->classname, "magnet"))
			continue;
		if (h->owner != self)
			continue;

		h->s.effects = EF_COLOR_SHELL;
		h->s.renderfx = RF_SHELL_RED;

		h->think = Attract;
		h->nextthink = level.time + .1;
	}
}

void PushAbov (edict_t *s)
{
	return;

	if (s->groundentity)
	{
		s->groundentity = NULL;
		s->s.origin[2] += 4;
	}
	//else
	//	s->s.origin[2] -= 4;

	//SV_AddGravity (s);

	if (gi.pointcontents(s->s.origin) & CONTENTS_SOLID)
		s->s.origin[2] += 24;

	gi.linkentity(s);

	s->nextthink2 = level.time + .1;
}

void Stop_Moving_Magnet (edict_t *s)
{
	VectorClear (s->velocity);
}

void MoveTheMagnetUp (edict_t *ent)
{
	if (ent->groundentity)
	{
		ent->s.origin[2] += 1;
		gi.linkentity(ent);
		ent->groundentity = NULL;
	}

	ent->nextthink4 = level.time + .1;
}

// The magnets don't do much when sitting still...
// Basically it's a map object until activated
void fire_magnet (edict_t *self, vec3_t start, vec3_t aimdir, int speed)
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
	grenade->movetype = MOVETYPE_FLYMISSILE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
//	VectorClear (grenade->mins);
//	VectorClear (grenade->maxs);
	VectorSet (grenade->mins, -6, -6, -6);
	VectorSet (grenade->maxs, 6, 6, 6);
	grenade->s.modelindex = ModelIndex ("models/proj/magnet/tris.md2");
	grenade->owner = self;
	grenade->touch = Magnet_Touch;
	grenade->classname = "magnet";
	grenade->can_teleport_through_destinations = 1;
	grenade->think2 = PushAbov;
	grenade->nextthink2 = level.time + .1;
	grenade->think3 = Stop_Moving_Magnet;
	grenade->nextthink3 = level.time + 0.4;
	grenade->think4 = MoveTheMagnetUp;
	grenade->nextthink4 = level.time + 0.1;

	gi.linkentity (grenade);
}

void Weapon_Magnet_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 4;
	int			kick = 8;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

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

	fire_magnet (ent, start, forward, 450);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Magnet (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 9, 0};

	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, Weapon_Magnet_Fire);
}
