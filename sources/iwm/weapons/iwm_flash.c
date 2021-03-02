#include "../g_local.h"
#include "../m_player.h"

#define         FLASH_RADIUS                    400
#define         BLIND_FLASH                     50      // Time of blindness in FRAMES

void Flash_Explode (edict_t *ent)
{
	vec3_t      offset, v;//, origin;
	edict_t *target;
	float Distance, BlindTimeAdd;
		
	// Move it off the ground so people are sure to see it	
	VectorSet(offset, 0, 0, 10);    	
	VectorAdd(ent->s.origin, offset, ent->s.origin);	
	
	if (ent->owner->client)		
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
			
	target = NULL;	
	while ((target = findradius(target, ent->s.origin, FLASH_RADIUS)) != NULL)		
	{		
		//if (target == ent->owner)			
			//continue;       // You know when to close your eyes, don't you?		
		if (!target->client)			
			continue;       // It's not a player		
		if (!visible(ent, target))			
			continue;       // The grenade can't see it		
               // Find distance

		VectorSubtract(ent->s.origin, target->s.origin, v);		
		Distance = VectorLength(v);				
		
		// Calculate blindness factor		
		if ( Distance < FLASH_RADIUS/10 )			
			BlindTimeAdd = BLIND_FLASH; // Blind completely		
		else			
			BlindTimeAdd = 1.5 * BLIND_FLASH * ( 1 / ( ( Distance - FLASH_RADIUS*2 ) / (FLASH_RADIUS*2) - 2 ) + 1 ); // Blind partially		
		if ( BlindTimeAdd < 0 )
			BlindTimeAdd = 0; // Do not blind at all.				
		
		// Not facing it, but still blinded a little		
		if (!infront(target, ent))			
			BlindTimeAdd *= .5;
		
		// You know when to close your eyes, don't you? Doesn't quite do the job. :)		
		//if (target == ent->owner)			
		//{			
		//	target->client->blindTime += BlindTimeAdd * .3;
		//	target->client->blindBase = BLIND_FLASH;			
		//	continue;			
		//}		
		
		// Increment the blindness counter		
		target->client->blindTime = 40 + BlindTimeAdd;
		target->client->blindBase = BLIND_FLASH;
		target->s.angles[YAW] = (rand() % 360); // Whee!
		
		//gi.dprintf ("%f\n", target->client->blindTime);

		// Let the player know what just happened		
		// (It's just as well, he won't see the message immediately!)		
		safe_cprintf(target, PRINT_HIGH, "You are blinded by a flash grenade!\n");
						
		// Let the owner of the grenade know it worked	
		safe_cprintf(ent->owner, PRINT_HIGH, "%s is blinded by your flash grenade!\n", target->client->pers.netname);
	}
			
	// Blow up the grenade	
	BecomeExplosion1(ent);	
}



void Flash_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{	
	if (other == ent->owner)		
		return;	
	
	// If it goes in to orbit, it's gone...	
	if (surf && (surf->flags & SURF_SKY))		
	{		
		G_FreeEdict (ent);		
		return;		
	}	
	
	// All this does is make the bouncing noises when it hits something...	
	if (!other->takedamage)		
	{		
		if (ent->spawnflags & 1)			
		{			
			if (random() > 0.5)				
				gi.sound (ent, CHAN_VOICE, SoundIndex("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);	
			else			
				gi.sound (ent, CHAN_VOICE, SoundIndex("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);			
		}		
		else			
			gi.sound (ent, CHAN_VOICE, SoundIndex("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);		
	}	
	return;	

	// The ONLY DIFFERENCE between this and "Grenade_Touch"!!
	Flash_Explode (ent);    
}

void fire_flash (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
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
	grenade->nextthink = level.time + timer;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->touch = Flash_Touch;
	grenade->think = Flash_Explode;
	grenade->classname = "flash_grenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;

	grenade->can_teleport_through_destinations = 1;
	grenade->s.renderfx |= RF_IR_VISIBLE;

	if (ISGL(self))
		return;

	grenade->s.sound = SoundIndex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		Grenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, SoundIndex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}

void weapon_flashgrenade_fire (edict_t *ent, qboolean held)
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
	/*fire_grenade2*/fire_flash (ent, start, forward, damage, speed, timer, radius, held);

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

void Weapon_FlashGrenade (edict_t *ent)
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
				weapon_flashgrenade_fire (ent, true);
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
			weapon_flashgrenade_fire (ent, false);
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
