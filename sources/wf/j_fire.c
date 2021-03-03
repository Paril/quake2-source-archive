#include "g_local.h"
void unfreeze_player(edict_t *ent);


//Remove all the flames attached to a player
void Remove_Player_Flames (edict_t *ent)
{
	edict_t *blip = NULL;
	int i;

	if (!ent) return;
	if (!ent->Flames) return;

	for (i=1, blip=g_edicts+i ; i < globals.num_edicts ; i++,blip++)
	{
 	  if (blip->owner == ent)
	  {

		//Flames
		if (!strcmp(blip->classname, "fire") )
		{
			//set damage to zero so flame think function will clean it up
			blip->dmg = 0;
		}
	  }
	}

	ent->Flames = 0;
}

void Remove_Flame(edict_t *ent)
{
	if (ent->owner)
	{
		ent->owner->Flames--;
		if (ent->owner->Flames < 0) ent->owner->Flames = 0;
	}
	G_FreeEdict (ent);
}

void Fire_Think (edict_t *self)
{
	vec3_t dir;
	int damage;
	float	points;
	vec3_t	v;
	int mod;

	//Must have an owner
	if (!self->owner)
	{
		G_FreeEdict (self);
		return;
	}

	if (level.time > self->delay)
	{
		Remove_Flame(self);
		return;
	}

	if (self->dmg <= 0)	//Something else wants this flame removed
	{
		Remove_Flame(self);
		return;
	}

	// ++TeT add check for freed or dead owners
	if (!self->owner->inuse || (self->owner->Flames == 0))
	{
		Remove_Flame(self);
		return;
	}


	//If player or flame is in water
	if (self->owner->waterlevel || self->waterlevel)
	{
		//Play this if in water
		if ((self->owner) && (self->owner->client))
			gi.sound (self->owner, CHAN_WEAPON, gi.soundindex ("world/airhiss1.wav"), 1, ATTN_NORM, 0);
		Remove_Flame(self);
		return;
	}

	damage = self->dmg;	
	mod = self->mod;
	if (mod == 0) mod = MOD_WF_FLAME;

	VectorAdd (self->orb->mins, self->orb->maxs, v);
	VectorMA (self->orb->s.origin, 0.5, v, v);
	VectorSubtract (self->s.origin, v, v);
	points = damage - 0.5 * (VectorLength (v));

	if (points < 8) points = 8;

	VectorSubtract (self->owner->s.origin, self->s.origin, dir);

	VectorCopy(self->owner->s.origin,self->s.origin);
	if (self->PlasmaDelay < level.time)
	{
		T_Damage (self->owner, self, self->orb, dir, self->owner->s.origin,vec3_origin, damage, 0, DAMAGE_NO_KNOCKBACK, mod);
		self->PlasmaDelay = level.time + 0.8;
	}
	self->nextthink = level.time + .2;
}

void burn_person(edict_t *target, edict_t *owner, int damage, int mod)
{
	edict_t	*flame;

	if (target->frozen)
		unfreeze_player(target);	//Thaw them out!

	if (!target->takedamage) return;

	if (target->wf_team == owner->wf_team) return;

	if (target->Flames > 1)	return;

	//If they are not a client, only burn decoys and sentry guns
	if (!target->client)
	{
		if ((strcmp(target->classname, "decoy") != 0)  &&
			(strcmp(target->classname, "SentryGun") != 0))
			return;
	}

	//Don't burn them if they have flame resistance
	if ((target->client) && (target->client->player_special & SPECIAL_FLAME_RESISTANCE) )
	{
		gi.sound (target, CHAN_WEAPON, gi.soundindex ("items/protect4.wav"), 1, ATTN_NORM, 0);
		return;
	}

	//burn em baby, but dead people don't scream (TeT)
	if ((target->client) && (target->health > 0))
		gi.sound (target, CHAN_WEAPON, gi.soundindex ("scream.wav"), 1, ATTN_NORM, 0);

	target->Flames++;
	flame = G_Spawn();
	flame->movetype = MOVETYPE_NOCLIP;
	flame->clipmask = MASK_SHOT;
	flame->solid = SOLID_NOT;
	flame->s.effects |= EF_ANIM_ALLFAST|EF_BFG|EF_HYPERBLASTER;//|EF_GRENADE|EF_BLASTER;
	flame->velocity[0] = target->velocity[0];
	flame->velocity[1] = target->velocity[1];
	flame->velocity[2] = target->velocity[2];

	VectorClear (flame->mins);
	VectorClear (flame->maxs);
	flame->s.modelindex = gi.modelindex ("sprites/fire.sp2");
	flame->owner = target;
	flame->orb = owner;
	flame->delay = level.time + 10;  //G.R. Made it larger to do more damage
	flame->nextthink = level.time + .8;
	flame->PlasmaDelay = level.time + 0.8;
	flame->think = Fire_Think;
	flame->SniperDamage = damage+2;//JR increased that
	flame->classname = "fire";
	flame->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");
	flame->dmg = damage;
	flame->mod = mod;
	gi.linkentity (flame);

	VectorCopy(target->s.origin,flame->s.origin);
}

static void Napalm_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		return;
	}
	return;
}

void Napalm_Think(edict_t *self)
{
	edict_t	*ent;
	vec3_t	v;
	float	points;
	float	dist;
	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 64)) != NULL)
	{
		if (!ent->takedamage)
			continue;
		if (!CanDamage (ent, self))
			continue;
		if (!CanDamage (ent, self->owner))
			continue;
		// ++TeT
		if (ent->wf_team == self->wf_team)
			continue;
		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (self->s.origin, v, v);
		dist = VectorLength(v);
		points = self->radius_dmg * (1.0 - sqrt(dist/self->dmg_radius));
		if (ent == self->owner)
			points = points * 0.5;
		burn_person(ent, self->owner, 4, MOD_NAPALMGRENADE);

		T_Damage (ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, 3, 25, DAMAGE_ENERGY,0);
	}
	self->nextthink = level.time + 0.8;

	if (self->delay <level.time)
		self->think = G_FreeEdict;
	if (self->waterlevel)
	{
		G_FreeEdict (self);
		return;
	}
}

//Use Gregg's function for now
void fire_napalm2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{     
	edict_t *grenade; 
       vec3_t  dir;
        vec3_t  forward, right, up;  
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
        grenade->s.effects |= /*EF_GRENADE|EF_BLASTER|*/EF_ANIM_ALLFAST|EF_BFG|EF_HYPERBLASTER;;
	//	VectorClear (grenade->mins);
      //  VectorClear (grenade->maxs);
        grenade->s.modelindex = gi.modelindex ("sprites/fire.sp2");
        grenade->owner = self;   
        grenade->wf_team = self->wf_team;   
		grenade->touch = Napalm_Touch;
        grenade->think = Napalm_Think;
		grenade->nextthink = level.time + 0.1;
		grenade->delay = level.time + timer;
		grenade->dmg = damage;
        grenade->dmg_radius = damage_radius;
        grenade->classname = "napalm";  
			// CCH: a few more attributes to let the grenade 'die'
	VectorSet(grenade->mins, -32, -32, -32);
	VectorSet(grenade->maxs, 32, 32, 32);
	grenade->mass = 5;
	grenade->monsterinfo.aiflags = AI_NOSTEP;

		gi.linkentity (grenade);
}
