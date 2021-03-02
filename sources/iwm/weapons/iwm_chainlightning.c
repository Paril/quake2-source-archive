#include "../g_local.h"
#include "../m_player.h"

// Chain Lightning Grenade File
// Thesis: G_RunEntity will have a check that if "lightning_chain" in edict_t
// is true, then it will find non-lightning_chain entities in the area
// and zap it, continuing a chain until no more entities are to be seen.

edict_t *grenade_owner;

// Search for a victim!
// This will not search for closest ent, just a
// random one it finds.
// FIXME: Closest ent maybe?
void CheckLightningChains (edict_t *ent)
{
	edict_t *t = NULL;
	qboolean found = false;
	edict_t *chains;
	vec3_t end;
	vec3_t start;

	while (((t = findradius(t, ent->s.origin, 800)) != NULL))
	{
		if (!t->takedamage)
			continue;
		if (t->lightning_chain)
			continue;
		if (t == grenade_owner)
			continue;

		VectorCopy (t->s.origin, end);
		VectorCopy (ent->s.origin, start);

		start[2] += 12;
		end[2] += 12;

		// Zap!
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_LIGHTNING);
		gi.WriteShort (t - g_edicts);			// destination entity
		gi.WriteShort (ent - g_edicts);		// source entity
		gi.WritePosition (end);
		gi.WritePosition (start);
		gi.multicast (start, MULTICAST_PVS);

		// Give kills to the owner of the grenade.
		T_Damage (t, ent, grenade_owner, vec3_origin, t->s.origin, vec3_origin, 50, 0, 0, MOD_LZAP);
		found = true;
		t->lightning_chain = true;
		break;
	}

	// None found in area, remove everyone's lightning_chain flag;
	if (found)
		return;

	for (chains = g_edicts+1; chains < &g_edicts[game.maxentities]; chains++) 
	{
		if (!chains->inuse)
			continue;
		if (chains->lightning_chain) 
		{
			chains->lightning_chain = false;
		}
	}
}

void ChainLightning_Explode (edict_t *ent)
{
	vec3_t      offset;//,v; 
    // Move it off the ground so people are sure to see it
    VectorSet(offset, 0, 0, 10);    
    VectorAdd(ent->s.origin, offset, ent->s.origin);
	
    if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	
	ent->lightning_chain = true;
	grenade_owner = ent->owner;
	// Let the chains begin!
	CheckLightningChains (ent);
	
	// Blow up the grenade
	BecomeExplosion1(ent);
}

void ChainLightning_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
	ChainLightning_Explode (ent);
}

void fire_chainlightning (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
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
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	if (!ISGL(self))
		grenade->s.modelindex = ModelIndex ("models/objects/grenade2/tris.md2");
	else
		grenade->s.modelindex = ModelIndex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = ChainLightning_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = ChainLightning_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "chainlightning";
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
		ChainLightning_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, SoundIndex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}

	gi.linkentity (grenade);
}


void weapon_chainlightning_fire (edict_t *ent, qboolean held)
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
	fire_chainlightning (ent, start, forward, damage, speed, timer, radius, held);

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

void Weapon_ChainLightning (edict_t *ent)
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
				weapon_chainlightning_fire (ent, true);
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
			weapon_chainlightning_fire (ent, false);
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
