
#include "g_local.h"


//========================================================
//========================================================
// Q1 EXPLOSION SPRITE
//========================================================
//========================================================


static void q1_lavaball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg,(ent->dmg*2), 0, MOD_Q1_RL);
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_Q1_RL_SPLASH);
	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("q1weap/rocket/r_exp3.wav"), 1.0, ATTN_NORM, 0);	

//
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
//	
	// explosion sprite 
	gi.unlinkentity(ent);
	ent->solid = SOLID_NOT;
	ent->touch = NULL;
	VectorCopy(ent->s.origin, origin);
	VectorCopy(ent->s.origin, ent->s.old_origin);
	VectorClear (ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/s_explod.sp2"); 
	ent->s.frame = 0;
	ent->s.sound = 0;
	ent->s.effects &= ~EF_ANIM_ALLFAST; 
	ent->think = q1_explode; 
	ent->nextthink = level.time + FRAMETIME;
	ent->enemy = other;
	gi.linkentity (ent);
}


	
void q1_fire_lavaball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*lavaball;

	lavaball = G_Spawn();
	VectorCopy (start, lavaball->s.origin);
	VectorCopy (dir, lavaball->movedir);
	vectoangles (dir, lavaball->s.angles);
	VectorScale (dir, speed, lavaball->velocity);
	lavaball->movetype = MOVETYPE_FLYMISSILE;
	lavaball->clipmask = MASK_SHOT;
	lavaball->solid = SOLID_BBOX;
	lavaball->s.effects |= EF_FLAG1;
	lavaball->s.renderfx |= RF_GLOW;

	VectorClear (lavaball->mins);
	VectorClear (lavaball->maxs);
	lavaball->s.modelindex = gi.modelindex ("models/monsters/q1monstr/boss/lavaball/tris.md2");
	lavaball->owner = self;
	lavaball->touch = q1_lavaball_touch;
	lavaball->nextthink = level.time + 8000/speed;
	lavaball->think = G_FreeEdict;
	lavaball->dmg = damage;
	lavaball->radius_dmg = radius_damage;
	lavaball->dmg_radius = damage_radius;
	lavaball->s.sound = gi.soundindex ("weapons/rockfly.wav");
	lavaball->classname = "lavaball";

	gi.linkentity (lavaball);
}


//========================================================
//========================================================
//========================================================

static void q1_acidbolt_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (other->takedamage)
	{
		// FIX MOD
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_Q1_RL);
	}
	
	G_FreeEdict (ent);
}

/*
void G_Spawn_Trails(int type, vec3_t start, vec3_t endpos, vec3_t origin ) {
gi.WriteByte(svc_temp_entity);
gi.WriteByte(type);
gi.WritePosition(start);
gi.WritePosition(endpos);
gi.multicast(origin, MULTICAST_PVS);
}
*/

void q1_fire_acidspit (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*acidbolt;

	acidbolt = G_Spawn();
	VectorCopy (start, acidbolt->s.origin);
	VectorCopy (dir, acidbolt->movedir);
	vectoangles (dir, acidbolt->s.angles);
	VectorScale (dir, speed, acidbolt->velocity);
	acidbolt->movetype = MOVETYPE_FLYMISSILE;
	acidbolt->clipmask = MASK_SHOT;
	acidbolt->solid = SOLID_BBOX;
	acidbolt->s.effects |= EF_HYPERBLASTER; //EF_BFG EF_GREENTRAIL
	acidbolt->s.renderfx |= RF_TRANSLUCENT;//FULLBRIGHT;

	VectorClear (acidbolt->mins);
	VectorClear (acidbolt->maxs);
	acidbolt->s.modelindex = gi.modelindex ("models/monsters/q1monstr/scrag/bolt/tris.md2");
	acidbolt->owner = self;
	acidbolt->touch = q1_acidbolt_touch;
	acidbolt->nextthink = level.time + 8000/speed;
	acidbolt->think = G_FreeEdict;
	acidbolt->dmg = damage;
	//acidbolt->s.sound = gi.soundindex ("weapons/rockfly.wav");
	acidbolt->classname = "acidbolt";
	
	gi.linkentity (acidbolt);
}


//========================================================
//========================================================
//========================================================


static void q1_zombiegib_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (other->takedamage)
	{
		// FIX MOD
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_Q1_RL); 
		gi.sound (ent, CHAN_RELIABLE|CHAN_WEAPON, gi.soundindex ("q1monstr/zombie/z_hit.wav"), 1.0, ATTN_NORM, 0);	
	}
	else
	{
		gi.sound (ent, CHAN_RELIABLE|CHAN_WEAPON, gi.soundindex ("q1monstr/zombie/z_miss.wav"), 1.0, ATTN_NORM, 0);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLOOD);
		gi.WritePosition (ent->s.origin);
		gi.WriteDir (plane->normal);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
		
// no more touches	
	ent->touch = NULL;

	ent->nextthink = level.time + 3;
	ent->think = G_FreeEdict;
	//G_FreeEdict (ent);
}

void q1_fire_gib (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
{
	edict_t	*gib;
	vec3_t	dir;
	vec3_t	up; //forward, right,

	vectoangles (aimdir, dir);
	AngleVectors (dir, NULL, NULL, up);

	gib = G_Spawn();
	VectorCopy (start, gib->s.origin);
	VectorScale (aimdir, speed, gib->velocity);
	VectorMA (gib->velocity, 200, up, gib->velocity);
	//VectorMA (gib->velocity, 20, right, gib->velocity);
	
	VectorSet (gib->avelocity, 3000, 1000, 2000);
	gib->movetype = MOVETYPE_BOUNCE;
	gib->clipmask = MASK_SHOT;
	gib->solid = SOLID_BBOX;
	gib->s.effects |= EF_GIB;
	VectorClear (gib->mins);
	VectorClear (gib->maxs);
	gib->s.modelindex = gi.modelindex ("models/monsters/q1monstr/zombie/gib/tris.md2"); 
	gib->owner = self;
	gib->touch = q1_zombiegib_touch;
	gib->nextthink = level.time + 2.5;
	gib->think = G_FreeEdict;
	gib->dmg = damage;
	gib->classname = "gib";

	gi.linkentity (gib);
}



//========================================================
//========================================================
//========================================================

static void d_texplode (edict_t *self)
{
	if (self->s.frame == 2)
	{
		G_FreeEdict (self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
}

static void d_toxicball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg,(ent->dmg*2), 0, MOD_Q1_RL);
	}

	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("dmonstr/fhit.wav"), 1.0, ATTN_NORM, 0);	

	// explosion sprite 
	gi.unlinkentity(ent);
	ent->solid = SOLID_NOT;
	ent->touch = NULL;
	VectorCopy(ent->s.origin, origin);
	VectorCopy(ent->s.origin, ent->s.old_origin);
	VectorClear (ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/dtexp.sp2"); 
	ent->s.frame = 0;
	ent->s.sound = 0;
	ent->s.effects &= ~EF_ANIM_ALLFAST; 
	ent->think = d_texplode; 
	ent->nextthink = level.time + FRAMETIME;
	ent->enemy = other;
	gi.linkentity (ent);
}


void d_fire_baron (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
{
	edict_t	*toxicball;

	toxicball = G_Spawn();
	VectorCopy (start, toxicball->s.origin);
	VectorCopy (aimdir, toxicball->movedir);
	vectoangles (aimdir, toxicball->s.angles);
	VectorScale (aimdir, speed, toxicball->velocity);
	toxicball->movetype = MOVETYPE_FLYMISSILE;
	toxicball->clipmask = MASK_SHOT;
	toxicball->solid = SOLID_BBOX;
	toxicball->s.effects |= (EF_ANIM_ALLFAST|EF_BFG);
	toxicball->s.renderfx |= RF_TRANSLUCENT;

	VectorClear (toxicball->mins);
	VectorClear (toxicball->maxs);
	toxicball->s.modelindex = gi.modelindex ("sprites/dtoxic.sp2");
	toxicball->owner = self;
	toxicball->touch = d_toxicball_touch;
	toxicball->nextthink = level.time + 8000/speed;
	toxicball->think = G_FreeEdict;
	toxicball->dmg = damage;
	toxicball->s.sound = gi.soundindex ("dmonstr/ffire.wav");
	toxicball->classname = "toxicball";

	gi.linkentity (toxicball);
}



//========================================================
//========================================================
//========================================================


static void d_fireball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg,(ent->dmg*2), 0, MOD_Q1_RL);
	}
	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("dmonstr/fhit.wav"), 1.0, ATTN_NORM, 0);	
	// explosion sprite 
	gi.unlinkentity(ent);
	ent->solid = SOLID_NOT;
	ent->touch = NULL;
	VectorCopy(ent->s.origin, origin);
	VectorCopy(ent->s.origin, ent->s.old_origin);
	VectorClear (ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/dfexp.sp2"); 
	ent->s.frame = 0;
	ent->s.sound = 0;
	ent->s.effects &= ~EF_ANIM_ALLFAST; 
	ent->think = d_texplode; 
	ent->nextthink = level.time + FRAMETIME;
	ent->enemy = other;
	gi.linkentity (ent);
}

void d_fire_imp (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
{
	edict_t	*toxicball;
	toxicball = G_Spawn();
	VectorCopy (start, toxicball->s.origin);
	VectorCopy (aimdir, toxicball->movedir);
	vectoangles (aimdir, toxicball->s.angles);
	VectorScale (aimdir, speed, toxicball->velocity);
	toxicball->movetype = MOVETYPE_FLYMISSILE;
	toxicball->clipmask = MASK_SHOT;
	toxicball->solid = SOLID_BBOX;
	toxicball->s.effects |= (EF_ANIM_ALLFAST|EF_HYPERBLASTER);
	toxicball->s.renderfx |= RF_TRANSLUCENT;
	VectorClear (toxicball->mins);
	VectorClear (toxicball->maxs);
	toxicball->s.modelindex = gi.modelindex ("sprites/dfire.sp2");
	toxicball->owner = self;
	toxicball->touch = d_toxicball_touch;
	toxicball->nextthink = level.time + 8000/speed;
	toxicball->think = G_FreeEdict;
	toxicball->dmg = damage;
	toxicball->s.sound = gi.soundindex ("dmonstr/ffire.wav");
	toxicball->classname = "fireball";
	gi.linkentity (toxicball);
}
