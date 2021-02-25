//===========================================================================
// g_flare.c
//
// Marine flares.
// Originally coded by DingBat.
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//===========================================================================

#include "g_local.h"

static void Flare_End(edict_t *ent) {

	vec3_t VEC_UP		= {0, -1, 0};
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WELDING_SPARKS);
	gi.WriteByte (100);
	gi.WritePosition (ent->s.origin);
	gi.WriteDir (VEC_UP);
	gi.WriteByte (1);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// Simple cleanup.
	// Flare disappears.
	G_FreeEdict(ent) ;

	return ;
}

static void Flare_Burnout(edict_t *ent)
{ 
    vec3_t VEC_UP		= {0, -1, 0};

	// Try to lower the intensity of the flare as it burns out.
	// Doesn't work very well right now.
	// Need to find a way to make the area of effect smaller.
    ent->count-- ;
    if (ent->count == 0) {

      if (ent->s.effects & EF_BFG) {
    
	ent->s.renderfx = RF_SHELL_RED ;
	ent->s.effects ^= EF_BFG ;
	ent->s.effects |= EF_BLASTER ;

	//ent->health = flare_dim_time->value * 10 ;
	ent->count = getFlareDimTime() * 10 ;
      }
      else if (ent->s.effects & EF_BLASTER) {
	ent->s.effects ^= EF_BLASTER ; 
		
	//ent->health = flare_die_time->value * 10 ;
	ent->count = getFlareDieTime() * 10 ;
      }
      else {
	ent->think = Flare_End ;
      }
	
    }

    ent->nextthink += 1 ;

    gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_WELDING_SPARKS);
    gi.WriteByte (10);
    gi.WritePosition (ent->s.origin);
    gi.WriteDir (VEC_UP);
    gi.WriteByte (1);
    gi.multicast (ent->s.origin, MULTICAST_PVS);

    gi.linkentity(ent) ;

    return ;

}

static void Flare_Die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point) {

	// Simple cleanup.
	// Flare disappears.
        vec3_t VEC_UP           = {0, -1, 0};
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_WELDING_SPARKS);
        gi.WriteByte (100);
        gi.WritePosition (self->s.origin);
        gi.WriteDir (VEC_UP);
        gi.WriteByte (1);
        gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict(self) ;

	return ;
}

static void Flare_Explode (edict_t *ent)
{

	int			mod;

	// Blow yourself up?
	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?

	// This is the only block that will be used.
	// If you hit an enemy with the flare, it does damage.
	// Enemy must be player or a monster.
	// Otherwise, nothing happens.
	if (ent->enemy->client || (ent->enemy->svflags & SVF_MONSTER))
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		mod = MOD_FLARE ;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);

		// Cleanup.
		G_FreeEdict (ent);
	}

}

static void Flare_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// Guess you can't hit yourself in the face with a flare.
	if (other == ent->owner)
		return;

	// If you toss it off the world, it's gone.
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// NH change: Flare damage.
	// Only do this if flares are intended to do damage.
	if (getFlareDamage() > 0) {

	  // If the other entity doesn't take damage, why bother?
	  if (!other->takedamage) {

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

	  // Otherwise, BOOM!
	  ent->enemy = other;
	  Flare_Explode (ent);
	}
}

void fire_flare (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	// Firing flare ends safety mode.
	clearSafetyMode(self) ;

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

	// This gives it the "glow area".
	// Wish there was a way to alter the area of effect.
	grenade->s.effects |= EF_BFG ;
	grenade->s.effects |= EF_COLOR_SHELL ;

	// Should be a white flare.
	grenade->s.renderfx |= (RF_SHELL_GREEN | RF_SHELL_RED | RF_SHELL_BLUE) ;
	grenade->s.renderfx &= ~RF_GLOW ;

	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = Flare_Touch;

	// Time is any number after timer.
	//grenade->nextthink = level.time + timer + flare_bright_time->value ;
	grenade->nextthink = level.time + 1 ;

	// Number of seconds in this "phase" of flare life.
	grenade->count = getFlareBrightTime() * 10 ; 

	// Important. The flare starts to "burn out" after n ticks.
	grenade->think = Flare_Burnout ;

	// NH changes: Flares may not do damage.
	grenade->dmg = getFlareDamage() ;
	grenade->dmg_radius = getFlareDamageRadius();
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	// grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	// New for flare.
	VectorSet(grenade->mins, -3, -3, 0) ;
	VectorSet(grenade->maxs, 3, 3, 6) ;
	grenade->mass = 2 ;

	// This seems to be expected, though it will never be called.
	grenade->die = Flare_Die ;

	// Do flares take damage?
	// NH change: Flares may not have health.
	grenade->health = getFlareHealth() ;
	//grenade->health = flare_bright_time->value * 10 ;
	grenade->takedamage = DAMAGE_YES ;
	// grenade->monsterinfo.aiflags = AI_NOSTEP ;

	// This is the "ticking", if you want to get rid of it.
	// gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);

	gi.linkentity (grenade);

	// Clear safety mode?
	clearSafetyMode(self) ;
}

// Flare command.
void Cmd_Flare_f(edict_t *ent)
{


	int			index;
	gitem_t		*it;

	// Pred cannot use flares.
	if (ent->isPredator) {
		gi.cprintf(ent, PRINT_HIGH, "Predator cannot use flares\n") ;
		return ;
	}

	it = FindItem ("flares");
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{

                // Customized out of ammo message.
	        if ((int) max_flares->value <= 0) 
		  gi.cprintf(ent, PRINT_HIGH, "Flares are not enabled.\n") ;
	        else if ((int) max_flares->value == 1) 
		  gi.cprintf(ent, PRINT_HIGH,
			     "Only one flare per respawn.\n") ;
		else
		  gi.cprintf(ent, PRINT_HIGH, "Only %d flares per respawn.\n", 
			     (int) max_flares->value) ;
		return;
	}

	gi.cprintf(ent, PRINT_HIGH, "Flare selected\n") ;
	it->use (ent, it);
}

void validateMaxFlares() {

  // maxflares must be between 0 and 100
  if ((max_flares->value < 0) ||
      (max_flares->value > 100))
    gi.cvar_set("max_flares", MAX_FLARES_DEFAULT );
}

int getMaxFlares() {

  // Check if the cvar has changed.
  if (max_flares->modified)
    validateMaxFlares() ;

  return max_flares->value ;
}

void validateFlareBrightTime() {

  // flare_bright_time must be between 0 and 60
  if ((flare_bright_time->value < 0) ||
      (flare_bright_time->value > 60))
    gi.cvar_set("flare_bright_time", FLARE_BRIGHT_DEFAULT );
}

int getFlareBrightTime() {

  // Check if the cvar has changed.
  if (flare_bright_time->modified)
    validateFlareBrightTime() ;

  return flare_bright_time->value ;
}

void validateFlareDimTime() {

  // flare_dim_time must be between 0 and 60
  if ((flare_dim_time->value < 0) ||
      (flare_dim_time->value > 60))
    gi.cvar_set("flare_dim_time", FLARE_DIM_DEFAULT );
}

int getFlareDimTime() {

  // Check if the cvar has changed.
  if (flare_dim_time->modified)
    validateFlareDimTime() ;

  return flare_dim_time->value ;
}

void validateFlareDieTime() {

  // flare_die_time must be between 0 and 60
  if ((flare_die_time->value < 0) ||
      (flare_die_time->value > 60))
    gi.cvar_set("flare_die_time", FLARE_DIE_DEFAULT );
}

int getFlareDieTime() {

  // Check if the cvar has changed.
  if (flare_die_time->modified)
    validateFlareDieTime() ;

  return flare_die_time->value ;
}

void validateFlareHealth() {

  // flare_health must be between 0 and 200
  if ((flare_health->value < 0) ||
      (flare_health->value > 200))
    gi.cvar_set("flare_health", FLARE_HEALTH_DEFAULT );
}

int getFlareHealth() {

  // Check if the cvar has changed.
  if (flare_health->modified)
    validateFlareHealth() ;

  return flare_health->value ;
}

void validateFlareDamage() {

  // flare_damage must be between 0 and 200
  if ((flare_damage->value < 0) ||
      (flare_damage->value > 200))
    gi.cvar_set("flare_damage", FLARE_DAMAGE_DEFAULT );
}

int getFlareDamage() {

  // Check if the cvar has changed.
  if (flare_damage->modified)
    validateFlareDamage() ;

  return flare_damage->value ;
}

void validateFlareDamageRadius() {

  // flare_damage_radius must be between 0 and 200
  if ((flare_damage_radius->value < 0) ||
      (flare_damage_radius->value > 200))
    gi.cvar_set("flare_damage_radius", FLARE_RADIUS_DEFAULT );
}

int getFlareDamageRadius() {

  // Check if the cvar has changed.
  if (flare_damage_radius->modified)
    validateFlareDamageRadius() ;

  return flare_damage_radius->value ;
}


