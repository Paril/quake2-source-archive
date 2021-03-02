#include "../g_local.h"
#include "../m_player.h"

void FireBombSpike (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, vec3_t angles)
{
	edict_t	*rocket;

	speed = 1;
	damage = 30;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	VectorAdd (rocket->s.angles, angles, rocket->s.angles);
	VectorAdd (rocket->velocity, angles, rocket->velocity);

	VectorCopy (rocket->s.angles, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = ModelIndex ("models/proj/bomb_spike/tris.md2");
	rocket->owner = self;
//	rocket->touch = BombSpike_Touch;
	rocket->dmg = damage;
//	rocket->radius_dmg = radius_damage;
//	rocket->dmg_radius = damage_radius;
	rocket->classname = "shard";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void SpreadSpikes (edict_t *ent)
{
	vec3_t ag, forward, right, up;
	int loop;
	vec3_t or;
//	vec3_t spike1, spike2, spike3, spike4, spike5, spike6, spike7, spike8, spike9, spike10, spike11, spike12, spike13, spike14, spike15, spike16, spike17, spike18, spike19, spike20, spike21, spike22, spike23, spike24, spike25, spike26, spike27, spike28, spike29, spike30, spike31, spike32;
//	vec3_t ang;

	VectorCopy (ent->s.origin, or);
	or[2] += 25;

	/*VectorSet(spike1,20,-20,0);
	VectorSet(spike2,20,20,0);
	VectorSet(spike3,-20,20,0);
	VectorSet(spike4,-20,-20,0);

	VectorSet(spike5,20,-20,20);
	VectorSet(spike6,20,20,20);
	VectorSet(spike7,-20,20,20);
	VectorSet(spike8,-20,-20,20);

	VectorSet(spike9,20,-20,10);
	VectorSet(spike10,20,20,10);
	VectorSet(spike11,-20,20,10);
	VectorSet(spike12,-20,-20,10);

	VectorSet(spike13,20,-20,60);
	VectorSet(spike14,20,20,60);
	VectorSet(spike15,-20,20,60);
	VectorSet(spike16,-20,-20,60);

	VectorSet(spike17,-20,0,20);
	VectorSet(spike18,20,0,-20);
	VectorSet(spike19,20,0,20);
	VectorSet(spike20,-20,0,-20);

	VectorSet(spike21,-20,20,20);
	VectorSet(spike22,20,20,-20);
	VectorSet(spike23,20,20,20);
	VectorSet(spike24,-20,20,-20);

	VectorSet(spike25,-20,10,20);
	VectorSet(spike26,20,10,-20);
	VectorSet(spike27,20,10,20);
	VectorSet(spike28,-20,10,-20);

	VectorSet(spike29,-20,60,20);
	VectorSet(spike30,20,60,-20);
	VectorSet(spike31,20,60,20);
	VectorSet(spike32,-20,60,-20);
	//VectorSet(spike17,0,-20,20);
	//VectorSet(spike18,0,20,-20);
	//VectorSet(spike19,0,20,20);
	//VectorSet(spike20,0,-20,-20);

	//VectorSet(spike21,20,-20,20);
	//VectorSet(spike22,20,20,-20);
//	VectorSet(spike23,20,20,20);
	//VectorSet(spike24,20,-20,-20);

	//VectorSet(spike25,10,-20,20);
	//VectorSet(spike26,10,20,-20);
	//VectorSet(spike27,10,20,20);
	//VectorSet(spike28,10,-20,-20);

	//VectorSet(spike29,60,-20,20);
	//VectorSet(spike30,60,20,-20);
	//VectorSet(spike31,60,20,20);
	//VectorSet(spike32,60,-20,-20);

	FireBombSpike (ent->owner, or, spike1, 10, 30);
	FireBombSpike (ent->owner, or, spike2, 10, 30);
	FireBombSpike (ent->owner, or, spike3, 10, 30);
	FireBombSpike (ent->owner, or, spike4, 10, 30);
	FireBombSpike (ent->owner, or, spike5, 10, 30);
	FireBombSpike (ent->owner, or, spike6, 10, 30);
	FireBombSpike (ent->owner, or, spike7, 10, 30);
	FireBombSpike (ent->owner, or, spike8, 10, 30);
	FireBombSpike (ent->owner, or, spike9, 10, 30);
	FireBombSpike (ent->owner, or, spike10, 10, 30);
	FireBombSpike (ent->owner, or, spike11, 10, 30);
	FireBombSpike (ent->owner, or, spike12, 10, 30);
	FireBombSpike (ent->owner, or, spike13, 10, 30);
	FireBombSpike (ent->owner, or, spike14, 10, 30);
	FireBombSpike (ent->owner, or, spike15, 10, 30);
	FireBombSpike (ent->owner, or, spike16, 10, 30);
	FireBombSpike (ent->owner, or, spike17, 10, 30);
	FireBombSpike (ent->owner, or, spike18, 10, 30);
	FireBombSpike (ent->owner, or, spike19, 10, 30);
	FireBombSpike (ent->owner, or, spike20, 10, 30);
	FireBombSpike (ent->owner, or, spike21, 10, 30);
	FireBombSpike (ent->owner, or, spike22, 10, 30);
	FireBombSpike (ent->owner, or, spike23, 10, 30);
	FireBombSpike (ent->owner, or, spike24, 10, 30);
	FireBombSpike (ent->owner, or, spike25, 10, 30);
	FireBombSpike (ent->owner, or, spike26, 10, 30);
	FireBombSpike (ent->owner, or, spike27, 10, 30);
	FireBombSpike (ent->owner, or, spike28, 10, 30);
	FireBombSpike (ent->owner, or, spike29, 10, 30);
	FireBombSpike (ent->owner, or, spike30, 10, 30);
	FireBombSpike (ent->owner, or, spike31, 10, 30);
	FireBombSpike (ent->owner, or, spike32, 10, 30);*/

	for (loop = 0; loop < 24; loop++)
	{
		vec3_t angle;
		ag[0] += 15;
		ag[1] = 0;
		ag[2] = 0;

		VectorCopy (ag, angle);

		AngleVectors (ag, forward, right, up);


		FireBombSpike(ent->owner, or, forward, 15, 10, angle);
	}


	//24
}

void Touch_SpikeBomb (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (Q_stricmp(other->classname, "spikebomb") == 0)
		return;

	if (other == ent->owner)
		return;

	if (other->takedamage)
		T_Damage(other, ent, ent->owner, vec3_origin, other->s.origin, vec3_origin, 15, 0, DAMAGE_NO_ARMOR, MOD_SPIKEBOMB_HIT); // MOD FIXME
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GUNSHOT);
		gi.WritePosition (ent->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}


	G_FreeEdict(ent);
}
void SpikeBombExplode2 (edict_t *ent)
{
	int n/*, mod*/;
	edict_t *flame;
	vec3_t origin, grenade_angs;
	vec3_t forward, right, up;
	// Thanks again Wabbit.

	// ///------>> Shrapnel:
	
	// Set up the means of death.
//	mod = MOD_SPIKE; // MOD FIXME
	
	// Big explosion effect:
	//for(n = 0; n < 128; n+=32)
	//{
		VectorMA (ent->s.origin, -0.02f, ent->velocity, origin);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GRENADE_EXPLOSION);
		gi.WritePosition (origin);
		gi.multicast (origin, MULTICAST_PVS);
	//}
	
	VectorMA (ent->s.origin, -0.02f, ent->velocity, origin);
	origin[2] = origin[2] + 32;
	
	for (n = 0; n < 30; n++)
	{
		grenade_angs[0] = 0;
		grenade_angs[1] = n * 12.0f;
		grenade_angs[2] = 0;
		AngleVectors (grenade_angs, forward, right, up);
		flame = G_Spawn();
		VectorCopy (origin, flame->s.origin);
		flame->svflags = SVF_PROJECTILE;
		
		VectorClear (flame->velocity);
		VectorMA (flame->velocity, 1200, forward, flame->velocity);

		flame->s.angles[1] = n * 12.0f;

		//VectorCopy (flame->velocity, flame->s.angles);
		flame->movetype = MOVETYPE_FLYMISSILE;
		flame->clipmask = MASK_SHOT;
		flame->solid = SOLID_BBOX;
		flame->dmg = 80;
		
		flame->s.modelindex = ModelIndex ("models/proj/bomb_spike/tris.md2");
		flame->s.frame = 0;
		
		//VectorSet (flame->mins, -3, -3, -3);
		//VectorSet (flame->maxs, 3, 3, 3);
		VectorClear (flame->mins);
		VectorClear (flame->maxs);
		flame->owner = ent->owner;
		flame->touch = Touch_SpikeBomb;		
		
		flame->classname = "spikebomb";
	}

	for (n = 0; n < 30; n++)
	{
		grenade_angs[0] = n * 12.0f;
		grenade_angs[1] = 0;
		grenade_angs[2] = 0;
		AngleVectors (grenade_angs, forward, right, up);
		flame = G_Spawn();
		VectorCopy (origin, flame->s.origin);
		
		VectorClear (flame->velocity);
		VectorMA (flame->velocity, 1200, up, flame->velocity);

		flame->s.angles[0] = n * 12.0f + 270;

		//VectorCopy (flame->velocity, flame->s.angles);
		flame->movetype = MOVETYPE_FLYMISSILE;
		flame->clipmask = MASK_SHOT;
		flame->solid = SOLID_BBOX;
		flame->dmg = 80;
		
		flame->s.modelindex = ModelIndex ("models/proj/bomb_spike/tris.md2");
		flame->s.frame = 0;
		
		//VectorSet (flame->mins, -3, -3, -3);
		//VectorSet (flame->maxs, 3, 3, 3);
		VectorClear (flame->mins);
		VectorClear (flame->maxs);
		flame->owner = ent->owner;
		flame->touch = Touch_SpikeBomb;	
		flame->think = G_FreeEdict;
		flame->nextthink = level.time + 3;
		
		flame->classname = "spikebomb";
	}

	G_FreeEdict (ent);
}

void SpikeBomb_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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

	ent->enemy = other;
	SpikeBombExplode2 (ent);
}

void fire_spikebomb (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
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
	grenade->s.modelindex = ModelIndex ("models/proj/bomb/tris.md2");
	grenade->owner = self;
	grenade->touch = SpikeBomb_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = SpikeBombExplode2;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	grenade->can_teleport_through_destinations = 1;
	grenade->s.renderfx |= RF_IR_VISIBLE;
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = SoundIndex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		SpikeBombExplode2 (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, SoundIndex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}

void Weapon_SpikeBomb_Fire (edict_t *ent, qboolean held)
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
	//fire_concussiongrenade (ent, start, forward, damage, speed, timer, radius, held);
	fire_spikebomb (ent, start, forward, damage, speed, timer, radius, held);

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

void Weapon_SpikeBomb (edict_t *ent)
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
				Weapon_SpikeBomb_Fire (ent, true);
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
			Weapon_SpikeBomb_Fire (ent, false);
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
