#include "../g_local.h"
#include "../m_player.h"

void Flare_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (!other->takedamage && Q_stricmp(ent->classname, "glowstick"))
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
	}
}

void Illuminate (edict_t *self)
{
//	int colors[2] = {EF_TRACKER | EF_BLASTER | EF_BFG, EF_BLUEHYPERBLASTER, EF_HYPERBLASTER};
//	int rfx[2] = {RF_SHELL_GREEN, RF_SHELL_BLUE, RF_SHELL_BLUE | RF_SHELL_GREEN};

	self->s.effects |= EF_COLOR_SHELL;

	if (self->tempint < 2)
	{
		self->s.effects |= EF_TRACKER | EF_BLASTER | EF_BFG;//colors[theone];
		self->s.renderfx |= RF_SHELL_GREEN;//rfx[theone];
	}
	else if (self->tempint < 4)
	{
		self->s.effects |= EF_FLAG2;
		self->s.renderfx |= RF_SHELL_BLUE;//rfx[theone];
	}
	else if (self->tempint < 6)
	{
		self->s.effects |= EF_FLAG1;
		self->s.renderfx |= RF_SHELL_RED;//rfx[theone];
	}
	else
	{
		self->s.effects |= EF_BLASTER;
		self->s.renderfx |= RF_SHELL_DOUBLE;//rfx[theone];
	}

	//gi.WriteByte (svc_temp_entity);
	//gi.WriteByte (TE_FLASHLIGHT);
	//gi.WritePosition (self->s.origin);
	//gi.WriteShort (self - g_edicts);
	//gi.multicast (self->s.origin, MULTICAST_PVS);

	if (self->think != G_FreeEdict)
	{
		self->nextthink = level.time + 12;
		self->think = G_FreeEdict;
	}
	self->nextthink2 = level.time + .1;
	self->think2 = Illuminate;
}

void ThrowThemOut2 (edict_t *self, float speed, vec3_t origin)
{
	edict_t	*chunk;
	vec3_t	v;

	origin[2] += 8;

	chunk = G_Spawn();
	VectorCopy (origin, chunk->s.origin);
	chunk->s.modelindex = ModelIndex ("models/proj/glowstick/tris.md2");
	chunk->s.skinnum = rand()%5;
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA (self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_BOUNCENOSTOP;
	chunk->solid = SOLID_BBOX;
	chunk->owner = self->owner;
	VectorSet (chunk->mins, -6, -6, -2);
	VectorSet (chunk->maxs, 6, 6, 2);
	chunk->avelocity[0] = random()*600;
	chunk->avelocity[1] = random()*600;
	chunk->avelocity[2] = random()*600;
	chunk->tempint = rand()%8;
	chunk->think = Illuminate;
	chunk->nextthink = level.time + .1;
	chunk->touch = Flare_Touch;
	chunk->think2 = G_FreeEdict;
	chunk->nextthink2 = level.time + 6;
	chunk->s.frame = 0;
	chunk->classname = "glowstick";
	chunk->dmg = 50;
	chunk->radius_dmg = 65;
	chunk->dmg_radius = 65;
	chunk->s.effects |= EF_SPHERETRANS;
//	chunk->s.frame = chunk->s.skinnum = 0;
	gi.linkentity (chunk);
}

void ThrowThemOut (edict_t *ent)
{
	ThrowThemOut2(ent, 3, ent->s.origin);
	ThrowThemOut2(ent, 3, ent->s.origin);
	ThrowThemOut2(ent, 3, ent->s.origin);
	ThrowThemOut2(ent, 3, ent->s.origin);

	G_FreeEdict (ent);
}

void fire_flare (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
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
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->tempint = rand()%6;
	if (!ISGL(self))
		grenade->s.modelindex = ModelIndex ("models/objects/grenade2/tris.md2");
	else
		grenade->s.modelindex = ModelIndex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = Flare_Touch;
	grenade->nextthink = level.time + 1;
	grenade->think = ThrowThemOut;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;

	grenade->can_teleport_through_destinations = 1;
	grenade->s.renderfx |= RF_IR_VISIBLE;

	if (ISGL(self))
		return;
	//grenade->s.sound = SoundIndex("weapons/hgrenc1b.wav");

	//if (timer <= 0.0)
	//	Grenade_Explode (grenade);
	//else
	//{
		gi.sound (self, CHAN_WEAPON, SoundIndex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	//}
}


void Weapon_Flare_Fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	//fire_flare (ent, start, forward, damage, speed, timer, radius, held);
	fire_flare (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_Flare (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			gi.sound(ent, CHAN_WEAPON, SoundIndex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
//				ent->client->weapon_sound = SoundIndex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				Weapon_Flare_Fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			Weapon_Flare_Fire (ent, false);
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}
