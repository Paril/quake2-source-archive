#include "../g_local.h"
#include "../m_player.h"


void DecFree (edict_t *ent)
{
	ent->owner->client->pers.flies--;
	G_FreeEdict (ent);
}

void FlyFly (edict_t *drinker)
{
	int p = (rand() % 2);
	int o = (rand() % 2)/*(rand() % 360)*/;
	int i = (rand() % 2);
	int p2 = (rand() % 2);
	int o2 = (rand() % 2)/*(rand() % 360)*/;
	int i2 = (rand() % 2);

	drinker->s.angles[PITCH] += (p - p2);
	drinker->s.angles[YAW] += (o - o2);
	drinker->s.angles[PITCH] += (i - i2);

	drinker->velocity[PITCH] += (p - p2);
	drinker->velocity[YAW] += (o - o2);
	drinker->s.angles[PITCH] += (i - i2);

	//drinker->nextthink = level.time + 0.1;
}

// CCH: New think function for homing missiles
// Paril: Thanks CCH. HOMINGWHOAMG.
void homing_think (edict_t *ent)
{
	edict_t *target = NULL;
	edict_t *blip = NULL;
	vec3_t  targetdir, blipdir;
	vec_t   speed;

	while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
	{
        if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			continue;
        if (blip == ent->owner)
			continue;
        if (!blip->takedamage)
			continue;
        if (blip->health <= 0)
			continue;
        if (!visible(ent, blip))
			continue;
        if (!infront(ent, blip))
			continue;

        VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
        blipdir[2] += 16;
        if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
        {
			target = blip;
			VectorCopy(blipdir, targetdir);
        }
	}
	
	if (target != NULL)
	{
		if (!ent->tempint)
		{
			vec3_t	forward;
			vec3_t dir;

			AngleVectors (ent->s.angles, forward, NULL, NULL);

			VectorCopy (forward, dir);

			vectoangles (dir, ent->s.angles);
			VectorScale (dir, 350, ent->velocity);
			ent->tempint = 1;

			ent->think2 = NULL;
			ent->nextthink2 = 0;
		}

        // target acquired, nudge our direction toward it
        VectorNormalize(targetdir);
        VectorScale(targetdir, 20, targetdir);
        VectorAdd(targetdir, ent->movedir, targetdir);
        VectorNormalize(targetdir);
        VectorCopy(targetdir, ent->movedir);
        vectoangles(targetdir, ent->s.angles);
        speed = VectorLength(ent->velocity);
        VectorScale(targetdir, speed, ent->velocity);
	}
	
	FlyFly(ent);
	ent->nextthink = level.time + .1;
}


void Firefly_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	vec3_t		origin;
//	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		DecFree (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_FIREFLY);
	}

	DecFree (ent);
}

void StopMoving (edict_t *self)
{
	VectorScale (self->s.origin, 0, self->velocity);

	self->think2 = DecFree;
	self->nextthink2 = level.time + 7;

	self->think = homing_think;
	self->nextthink = level.time + .1;
}

void fire_firefly (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*rocket;

	if (self->client->pers.flies > 29)
	{
		self->client->pers.inventory[self->client->ammo_index] += 1;
		return;
	}

	rocket = G_Spawn();
	rocket->tempspeed = speed;
	rocket->tempint = 0;
	speed = 100;
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);

	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = ModelIndex ("models/proj/firefly/tris.md2");
	rocket->owner = self;
	rocket->touch = Firefly_Touch;
	rocket->s.effects |= EF_HYPERBLASTER;
	rocket->think = StopMoving;
	rocket->nextthink = level.time + 0.7;
	rocket->s.effects |= EF_COLOR_SHELL;
	rocket->s.renderfx |= RF_SHELL_RED;
	rocket->dmg = damage;
//	rocket->radius_dmg = radius_damage;
//	rocket->dmg_radius = damage_radius;
	rocket->classname = "firefly";
	rocket->can_teleport_through_destinations = 1;
	rocket->s.renderfx |= RF_IR_VISIBLE;
//	rocket->s.frame = rocket->s.skinnum = 2;

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);

	self->client->pers.flies++;
}

void Weapon_FireFly_Fire (edict_t *ent)
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

	//for (i=0 ; i<shots ; i++)
	//{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

		//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
		fire_firefly (ent, start, forward, 8, 1800);
	//}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		//if (flies < 29)
			ent->client->pers.inventory[ent->client->ammo_index] -= 1;
	}
}


void Weapon_FireFly (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Weapon_FireFly_Fire);
}
