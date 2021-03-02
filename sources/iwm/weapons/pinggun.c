#include "../g_local.h"

void ping_l (edict_t *self)
{
	trace_t tr;
	vec3_t end, star;

	VectorCopy(self->s.origin, star);

	VectorMA(star, 0.1, tv(0, 0, -90), end);
	tr = gi.trace(star, NULL, NULL, end, self, MASK_SOLID);

	if (tr.fraction < 1.0)
	{
		//gi.dprintf ("Hit a plane: vel2 %f\n", self->velocity[2]);
		self->velocity[2] = 300;
		//gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/laserhit.wav"), 1, ATTN_NORM, 0);
	}

	self->nextthink2 = level.time + .1;
}

float AmountToLag (edict_t *self)
{
	int m = self->tempint;
	float amt = 0.1;

	if (m < 2)
		amt = 0.2;
	else if (m < 4)
		amt = 0.3;
	else if (m < 5)
		amt = 0.4;
	else if (m < 7)
		amt = 0.5;
	else if (m < 10)
		amt = 0.55;
	else
		amt = 0.6;

	return amt;
}

qboolean IsLagged (edict_t *self)
{
	if (!self->lagged)
		return false;
	else if (self->latency_time < level.time)
	{
		self->lagged = false;
		return false;
	}
	else
		return true;
}

void ping_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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

	if (other->client)
	{
		other->lagged = true;
		other->latency_time = level.time + (unsigned)10+(self->s.frame/2);
		other->tempint = self->s.frame;
		G_FreeEdict(self);
		return;
	}
	gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/laserhit.wav"), 1, ATTN_NORM, 0);
}

void fire_ping (edict_t *self, vec3_t start, vec3_t dir, int size)
{
	edict_t	*bolt;
	vec3_t up;
	vec3_t tmp;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, 650, bolt->velocity);
	bolt->velocity[2] += 100;
	bolt->movetype = MOVETYPE_BOUNCESTAY;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.frame = size;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = ModelIndex ("models/proj/pingball/tris.md2");
	bolt->owner = self;
	bolt->touch = ping_touch;
	bolt->nextthink = level.time + (8);
	bolt->think = G_FreeEdict;
	bolt->think2 = ping_l;
	bolt->nextthink2 = level.time + .1;
	bolt->classname = "pingpong";
	gi.linkentity (bolt);

	bolt->can_teleport_through_destinations = 1;
}	


void weapon_ping_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int frame = ent->tempint2;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-4);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	//gi.dprintf ("%i %i\n", frame, frame-17);
	fire_ping (ent, start, forward, frame-17);
	ent->tempint2 = 0;
	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_PingGun (edict_t *ent)
{
	static int	pause_frames[]	= {37, 49, 56, 0};
	static int	fire_frames[]	= {31, 0};

	if (ent->client->ps.gunframe < 31 && ent->client->weaponstate == WEAPON_FIRING)
	{
		if (!((ent->client->buttons|ent->client->latched_buttons) & BUTTON_ATTACK))
		{
			ent->tempint2 = ent->client->ps.gunframe;
			ent->client->ps.gunframe = 31;
		}
	}
	if (ent->client->ps.gunframe == 30)
	{
		if (((ent->client->buttons|ent->client->latched_buttons) & BUTTON_ATTACK))
			ent->client->ps.gunframe--;
	}

	Weapon_Generic (ent, 16, 36, 56, 62, pause_frames, fire_frames, weapon_ping_fire);
}