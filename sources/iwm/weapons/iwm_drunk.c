#include "../g_local.h"
#include "../m_player.h"

void drunk_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		if (self->spawnflags & 1)
			mod = MOD_HYPERBLASTER;
		else
			mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_FLECHETTE);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
//		G_Spawn_Sparks (TE_TUNNEL_SPARKS, self->s.origin, vec3_origin, self->s.origin);
		//G_Spawn_Splash(TE_TUNNEL_SPARKS, 6, COLOR_RED, self->s.origin, vec3_origin, self->s.origin);

//		gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/laserhit.wav"), 1, ATTN_NORM, 0);
	}

	G_FreeEdict (self);
}

void DrunkBolt (edict_t *drinker)
{
	int p = (rand() % 30 - rand() % 30);
	int o = (rand() % 30 - rand() % 30);
	int i = (rand() % 30 - rand() % 30);
	int a = (rand() % 30 - rand() % 30);
	int b = (rand() % 30 - rand() % 30);
	int c = (rand() % 30 - rand() % 30);

	drinker->s.angles[0] += (p -= a);
	drinker->s.angles[1] += (o -= b);
	drinker->s.angles[2] += (i -= c);

	drinker->velocity[0] += (p -= a);
	drinker->velocity[1] += (o -= b);
	drinker->velocity[2] += (i -= c);

	drinker->nextthink = level.time + 0.1;
}
void fire_drunk (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int hy)
{
	edict_t	*bolt;
	trace_t	tr;
	int all = 0;
	edict_t *cur;
	int i;

	damage = 15;
	speed = 1250;

	for (i=1, cur=g_edicts+i; i < globals.num_edicts; i++,cur++)
	{
		if (!cur->inuse)
			continue;
		if (cur->client)
			continue;
		if (!cur->classname)
			continue;

		if (Q_stricmp(cur->classname, "drunk") == 0)
			all++;
	}

	if (all > 55)
	{
		self->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))]++;
		return;
	}

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
	bolt->s.effects |= EF_COLOR_SHELL|EF_BLASTER|EF_IONRIPPER;
	bolt->s.renderfx |= RF_SHELL_GREEN;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = ModelIndex ("models/proj/firefly/tris.md2");
	bolt->s.sound = SoundIndex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = drunk_touch;
	bolt->nextthink = level.time + 0.1;
	bolt->think = DrunkBolt;
	bolt->dmg = damage;
	bolt->classname = "drunk";
	if (hy)
		bolt->spawnflags = 1;
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);
	bolt->can_teleport_through_destinations = 1;

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	

void Weapon_Drunk_Fire (edict_t *ent)
{
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = 2;

	if (deathmatch->value)
		damage = 6;
	else
		damage = 8;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, SoundIndex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&& ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, SoundIndex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = SoundIndex("weapons/chngnl1a.wav");
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}

	if (ent->client->ps.gunframe <= 9)
		shots = 1;
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTON_ATTACK)
			shots = 2;
		else
			shots = 1;
	}
	else
		shots = 3;

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

		fire_drunk (ent, start, forward, 5, 1500, EF_BLASTER, 0);
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}


void Weapon_Drunk (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Weapon_Drunk_Fire);
}
