#include "../g_local.h"
#include "../m_player.h"

qboolean findspawnpoint (edict_t *ent);

void Tele_Explode (edict_t *ent)
{
	edict_t *target;
    // Move it off the ground so people are sure to see it
	vec3_t offset;
    VectorSet(offset, 0, 0, 10);    
    VectorAdd(ent->s.origin, offset, ent->s.origin);
	
    if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	
    target = NULL;
    while ((target = findradius(target, ent->s.origin, 300)) != NULL)
    {
        //if (!target->client)
        //   continue;       // It's not a player
        if (!visible(ent, target))
            continue;       // The grenade can't see it
	//	if (!target->takedamage)
	//		continue;
		if (!target->can_teleport_through_destinations)
		{
			if (target->takedamage)
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_TELEPORT_EFFECT);
				gi.WritePosition (target->s.origin);
				gi.multicast (target->s.origin, MULTICAST_PVS);

				gi.sound (target, CHAN_AUTO, SoundIndex("misc/tele1.wav"), 1, ATTN_IDLE, 0);

				gi.unlinkentity(target);
				findspawnpoint (target);
				gi.linkentity(target);

				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_TELEPORT_EFFECT);
				gi.WritePosition (target->s.origin);
				gi.multicast (target->s.origin, MULTICAST_PVS);

				gi.sound (target, CHAN_AUTO, SoundIndex("misc/tele1.wav"), 1, ATTN_IDLE, 0);
			}
			continue;
		}
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_TELEPORT_EFFECT);
		gi.WritePosition (target->s.origin);
		gi.multicast (target->s.origin, MULTICAST_PVS);
		gi.sound (target, CHAN_AUTO, SoundIndex("misc/tele1.wav"), 1, ATTN_IDLE, 0);

		gi.unlinkentity(target);
		findspawnpoint (target);
		gi.linkentity(target);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_TELEPORT_EFFECT);
		gi.WritePosition (target->s.origin);
		gi.multicast (target->s.origin, MULTICAST_PVS);
		gi.sound (target, CHAN_AUTO, SoundIndex("misc/tele1.wav"), 1, ATTN_IDLE, 0);
	}
	
	// Blow up the grenade
	BecomeExplosion1(ent);
}

void TeleStick(edict_t *ent, edict_t *other)
{
	if (ent->movetype == MOVETYPE_STUCK)
		return;
	
	ent->movetype = MOVETYPE_STUCK;
	VectorClear(ent->velocity);
	VectorClear(ent->avelocity);
	ent->stuckentity = other;
	VectorCopy(other->s.origin,ent->oldstuckorigin);
	VectorCopy(other->s.angles,ent->oldstuckangles);
	// Paril: Fixes the impeding movement and jump bug.
	ent->solid = SOLID_NOT;
}

void Tele_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	TeleStick (ent, other);
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

	ent->enemy = other;
	//Tele_Explode (ent);
}

void fire_telegrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t *grenade;
	vec3_t  dir;
	vec3_t  forward, right, up;
	
	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);
	
	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 30.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE | EF_COLOR_SHELL;
	grenade->s.renderfx |= RF_SHELL_BLUE | RF_SHELL_RED;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	if (!ISGL(self))
		grenade->s.modelindex = ModelIndex ("models/objects/grenade2/tris.md2");
	else
		grenade->s.modelindex = ModelIndex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = Tele_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Tele_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "telegrenade";
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->can_teleport_through_destinations = 1;
	grenade->s.renderfx |= RF_IR_VISIBLE;
//	grenade->mass = 2;
//	grenade->health = 10;
//	grenade->die = Grenade_Die;
//	grenade->takedamage = DAMAGE_YES;
//	grenade->monsterinfo.aiflags = AI_NOSTEP;

	if (ISGL(self))
		return;

	grenade->s.sound = SoundIndex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		Tele_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, SoundIndex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}

	gi.linkentity (grenade);
}


void weapon_telegrenade_fire (edict_t *ent, qboolean held)
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
	fire_telegrenade (ent, start, forward, damage, speed, timer, radius, held);

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

void Weapon_TeleGrenade (edict_t *ent)
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
				weapon_telegrenade_fire (ent, true);
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
			weapon_telegrenade_fire (ent, false);
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
