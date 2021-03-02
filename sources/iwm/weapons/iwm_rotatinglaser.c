#include "../g_local.h"
#include "../m_player.h"

static void RotLaser_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
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

void Grenade_LaserProduce (edict_t *en)
{
	vec3_t  start;
	vec3_t  forward;
	vec3_t  end;
	trace_t tr;
	
	VectorCopy(en->s.origin, start);
	AngleVectors(en->s.angles, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);

	tr = gi.trace(start, NULL, NULL, end, en, MASK_SHOT);

	if ((tr.ent != en->owner) && (tr.ent->takedamage))
		T_Damage (tr.ent, en, en->owner, forward, tr.endpos, tr.plane.normal, 10, 0, 0, MOD_GRENADELASER);
    else if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
    {
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (tr.plane.normal);
		gi.multicast (tr.endpos, MULTICAST_PVS);

// 		gi.sound (en, CHAN_AUTO, SoundIndex ("weapons/laserhit.wav"), 1, ATTN_NORM, 0);
   }

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (en->s.origin);
	gi.WritePosition (tr.endpos);
	gi.multicast (en->s.origin, MULTICAST_PHS);

	en->think = Grenade_LaserProduce;
	en->nextthink = 0.1;

	en->s.angles[YAW] += 5;

	en->times++;

	if (en->times == 144)
		G_FreeEdict (en);
}

void Grenade_Raise (edict_t *self)
{
	VectorSet (self->s.angles, 0, 0, 0);
	self->movetype = MOVETYPE_NONE;
	self->s.origin[ROLL] += 6;
	gi.linkentity (self);
	self->times--;

	if (!self->times)
	{
		Grenade_LaserProduce(self);
		return;
	}

	self->nextthink = 0.1;
}

void fire_rotatelas (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int timer, float damage_radius, qboolean held)
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
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	if (!ISGL(self))
		grenade->s.modelindex = ModelIndex ("models/objects/grenade2/tris.md2");
	else
		grenade->s.modelindex = ModelIndex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = RotLaser_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Raise;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	grenade->times = 5;
	grenade->can_teleport_through_destinations = 1;
	grenade->s.renderfx |= RF_IR_VISIBLE;
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;

	if (ISGL(self))
		return;

	grenade->s.sound = SoundIndex("weapons/hgrenc1b.wav");

	gi.sound (self, CHAN_WEAPON, SoundIndex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
	gi.linkentity (grenade);
}



void Weapon_RotLas_Launcher (edict_t *ent, qboolean held)
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
	fire_rotatelas (ent, start, forward, damage, speed, timer, radius, held);

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

void Weapon_RotLas (edict_t *ent)
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
				ent->client->weapon_sound = SoundIndex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				Weapon_RotLas_Launcher (ent, true);
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
			Weapon_RotLas_Launcher (ent, false);
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
