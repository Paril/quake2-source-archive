#include "g_local.h"

//Other weapon functions

void fire_timednuke (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	grenade->wf_team = self->wf_team;
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = TimedNuke_Touch; //Stuff for cluster grenades when they explode
	grenade->nextthink = level.time + 4;
	grenade->think = Grenade_Explode; //stuff for cluster grenades exploding
	grenade->dmg = damage +500;
	grenade->dmg_radius = damage_radius + 500;
	grenade->classname = "grenade";
			// CCH: a few more attributes to let the grenade 'die'
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->mass = 2;
	grenade->health = 10;
	grenade->die = GenericGrenade_Die;
	grenade->takedamage = DAMAGE_YES;
	grenade->monsterinfo.aiflags = AI_NOSTEP;

	gi.linkentity (grenade);
}


void TimedNuke_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// ignore if on same team
	if (other->wf_team == ent->wf_team)
		return;

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

	return;
}


void weapon_timednukelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 1200;
	float	radius;

	radius = damage+400;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_timednuke (ent, start, forward, damage, 400, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}

void Weapon_TimedNukeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_timednukelauncher_fire);
}

void gib_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void gib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

void ThrowVomit (edict_t *ent, vec3_t mouth_pos, vec3_t forward, vec3_t right, vec3_t player_vel)
{	
	edict_t *gib;
	gib = G_Spawn();
	gi.setmodel (gib, "models/objects/gibs/sm_meat/tris.md2");
	gib->solid = SOLID_NOT;	gib->s.effects |= EF_GIB;
	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_YES;
	gib->die = gib_die;	
	gib->movetype = MOVETYPE_TOSS;
	gib->touch = gib_touch;
	// start the gib from out mouth, moving at a forwards velocity
	VectorCopy ( mouth_pos, gib->s.origin);
	VectorScale (forward, 120 + crandom()*40, gib->velocity);
	VectorAdd (player_vel, gib->velocity, gib->velocity);
	// add a random left-right component to the vomit velocity
	VectorScale (right, crandom()*20, right);
	VectorAdd (right, gib->velocity, gib->velocity);
	gib->avelocity[0] = random()*500;	
	gib->avelocity[1] = random()*500;
	gib->avelocity[2] = random()*500;
	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 10 + random()*5;	
	gi.linkentity (gib);
}
