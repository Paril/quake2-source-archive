#include "g_local.h"
#include "g_sogutil.h"



//======================================================================
//======================================================================
// Quake1 Flame / Torches
//======================================================================
//======================================================================

void SP_q1_misc_globe(edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.modelindex = gi.modelindex ("sprites/s_light.sp2");
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.renderfx |= RF_TRANSLUCENT;
	gi.linkentity (self);
}

//======================================================================
// SMALL FLAME
//======================================================================

void smallflame(edict_t *self)
{
	if(self->s.frame >= 5)
		self->s.frame = 0;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}	


void SP_q1_misc_small_flame(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/objects/idg3flame/tris.md2");
	ent->s.frame = 0;
	ent->s.sound = gi.soundindex("idg1weapons/fthit.wav");
	ent->s.renderfx = RF_FULLBRIGHT;
	ent->nextthink = level.time + FRAMETIME;
	ent->think = smallflame;
	gi.linkentity (ent);
}

//======================================================================
// LARGE FLAME
//======================================================================

void largeflame(edict_t *self)
{
	if(self->s.frame >= 16)
		self->s.frame = 6;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}	


void SP_q1_misc_large_flame(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/objects/idg3flame/tris.md2");
	ent->s.frame = 6;
	ent->s.sound = gi.soundindex("idg1weapons/fthit.wav");
	ent->nextthink = level.time + FRAMETIME;
	ent->think = largeflame;
	ent->s.renderfx = RF_FULLBRIGHT;
	gi.linkentity (ent);
}


//======================================================================
// FLYNN BIGTORCH
//======================================================================

void idg2_bigtorch(edict_t *self)
{
	if(self->s.frame >= 4)
		self->s.frame = 0;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}	


void SP_misc_idg2_bigtorch(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg2bigtorch/tris.md2");
	VectorSet (ent->mins, -10, -10, 0);
	VectorSet (ent->maxs, 10, 10, 84);
	ent->solid = SOLID_BBOX;
	ent->s.frame = 0;
	ent->s.sound = gi.soundindex("idg1weapons/fthit.wav");
	ent->nextthink = level.time + FRAMETIME;
	ent->think = idg2_bigtorch;
	ent->s.renderfx = RF_FULLBRIGHT;
	if (ent->spawnflags & 1)
		ent->s.skinnum = 1;
	else if (ent->spawnflags & 2)
		ent->s.skinnum = 2;
	gi.linkentity (ent);
}

//======================================================================
// FLYNN BIG TREE
//======================================================================

void SP_misc_idg2_bigtree(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg2bigtree/tris.md2");
	VectorSet (ent->mins, -32, -32, 0);
	VectorSet (ent->maxs, 32, 32, 72);
	ent->solid = SOLID_BBOX;
	ent->s.frame = 0;
	ent->s.sound = 0;
	gi.linkentity (ent);
}

//======================================================================
// FLYNN CANDLEABRA
//======================================================================

void SP_misc_idg2_candleabra(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg2candleabra/tris.md2");
	VectorSet (ent->mins, -10, -10, -24);
	VectorSet (ent->maxs, 10, 10, 30);
	ent->solid = SOLID_BBOX;
	ent->s.sound = 0;
	ent->s.renderfx = RF_FULLBRIGHT;
	gi.linkentity (ent);
}

//======================================================================
// BLAZE ARMORED
//======================================================================

void SP_misc_idg1_armored(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg1armored/tris.md2");
	VectorSet (ent->mins, -20, -20, -24);
	VectorSet (ent->maxs, 20, 20, 48);
	ent->solid = SOLID_BBOX;
	ent->s.sound = 0;
	gi.linkentity (ent);
}

//======================================================================
// BLAZE CAGE WITH SKELETON
//======================================================================

void SP_misc_idg1_cageskel(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg1cageskel/tris.md2");
	VectorSet (ent->mins, -16, -16, -24);
	VectorSet (ent->maxs, 16, 16, 56);
	ent->solid = SOLID_BBOX;
	ent->s.sound = 0;
	gi.linkentity (ent);
}

//======================================================================
// BLAZE FLAG
//======================================================================

void SP_misc_idg1_flag(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg1flag/tris.md2");
	VectorSet (ent->mins, -8, -8, -24);
	VectorSet (ent->maxs, 8, 8, 64);
	ent->solid = SOLID_BBOX;
	ent->s.sound = 0;
	gi.linkentity (ent);
}

//======================================================================
// BLAZE GREEN BARREL
//======================================================================

void SP_misc_idg1_barrel(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg1barrel/tris.md2");
	VectorSet (ent->mins, -16, -16, -24);
	VectorSet (ent->maxs, 16, 16, 24);
	ent->solid = SOLID_BBOX;
	ent->s.sound = 0;
	gi.linkentity (ent);
}

//======================================================================
// BLAZE WIDE BARREL
//======================================================================

void SP_misc_idg1_barrelwide(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg1barrelwide/tris.md2");
	VectorSet (ent->mins, -16, -16, -24);
	VectorSet (ent->maxs, 16, 16, 24);
	ent->solid = SOLID_BBOX;
	ent->s.sound = 0;
	gi.linkentity (ent);
}

//======================================================================
// BLAZE BED
//======================================================================

void SP_misc_idg1_bed(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg1bed/tris.md2");
	VectorSet (ent->mins, -40, -20, -24);
	VectorSet (ent->maxs, 40, 20, 8);
	ent->solid = SOLID_BBOX;
	ent->s.sound = 0;
	gi.linkentity (ent);
}

//======================================================================
// BLAZE BROWN PLANT
//======================================================================

void SP_misc_idg1_plant(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg1plant/tris.md2");
	VectorSet (ent->mins, -8, -8, -24);
	VectorSet (ent->maxs, 8, 8, 24);
	ent->solid = SOLID_BBOX;
	ent->s.sound = 0;
	gi.linkentity (ent);
}

//======================================================================
// BLAZE HANGING SKELETON
//======================================================================

void SP_misc_idg1_skelhang(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg1skelhang/tris.md2");
	VectorSet (ent->mins, -8, -8, -24);
	VectorSet (ent->maxs, 8, 8, 72);
	ent->solid = SOLID_BBOX;
	ent->s.sound = 0;
	gi.linkentity (ent);
}

//======================================================================
// BLAZE CAGE
//======================================================================

void SP_misc_idg1_cage(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg1cage/tris.md2");
	VectorSet (ent->mins, -16, -16, -24);
	VectorSet (ent->maxs, 16, 16, 48);
	ent->solid = SOLID_BBOX;
	ent->s.sound = 0;
	gi.linkentity (ent);
}

//======================================================================
// BLAZE SKELETON
//======================================================================

void SP_misc_idg1_skeleton(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg1skel/tris.md2");
	VectorSet (ent->mins, -32, -32, -24);
	VectorSet (ent->maxs, 32, 32, -8);
	ent->solid = SOLID_BBOX;
	ent->s.sound = 0;
	gi.linkentity (ent);
}

//======================================================================
// BLAZE BLUE POT
//======================================================================

void SP_misc_idg1_pot(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->s.modelindex = gi.modelindex ("models/objects/idg1pot/tris.md2");
	VectorSet (ent->mins, -8, -8, -24);
	VectorSet (ent->maxs, 8, 8, 8);
	ent->solid = SOLID_BBOX;
	ent->s.sound = 0;
	gi.linkentity (ent);
}


//======================================================================
// TORCH
//======================================================================

void quaketorch(edict_t *self)
{
	if(self->s.frame >= 5)
		self->s.frame = 0;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}

void SP_q1_misc_torch(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/objects/idg3torch/tris.md2");
	ent->s.frame = 0;
	ent->s.sound = gi.soundindex("idg1weapons/fthit.wav");
	ent->s.renderfx = RF_FULLBRIGHT;
	ent->nextthink = level.time + FRAMETIME;
	ent->think =quaketorch;
	gi.linkentity (ent);
}

//======================================================================

/*
==============
q1_target_trap

spawnflag 1 = superspike
defualt speed 500
damage 9 for spike, 15 for superspike
==============
*/

void use_target_trapshooter (edict_t *self, edict_t *other, edict_t *activator)
{
	qboolean super;

	if (self->spawnflags & 1)
		super = true;
	else
		super = false;

	gi.sound (self, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
	q1_fire_nail (self, self->s.origin, self->movedir, self->dmg, self->speed, super);
	gi.sound (self, CHAN_AUTO, gi.soundindex("idg3weapons/nails/s_end.wav"), 1.0, ATTN_NORM, 0);
}


void SP_idg3_target_trap(edict_t *self)
{
	self->use = use_target_trapshooter;
	G_SetMovedir (self->s.angles, self->movedir);
	self->noise_index = gi.soundindex ("idg3weapons/nails/spike.wav");
	gi.soundindex("idg3weapons/nails/s_end.wav");
	
	if(self->spawnflags & 1)
	{
		if (!self->dmg)
			self->dmg = 15;
	}
	else
	{
		if (!self->dmg)
			self->dmg = 9;
	}
	
	if (!self->speed)
		self->speed = 500;
	self->svflags = SVF_NOCLIENT;
}

//======================================================================
// AXE LAVABALL
//======================================================================

static void idg3_lavaball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("idg3weapons/rocket/r_exp3.wav"), 1.0, ATTN_NORM, 0);	


	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
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


void idg3_fire_lavaball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*lavaball;

	lavaball = G_Spawn();
	VectorCopy (start, lavaball->s.origin);
	VectorCopy (dir, lavaball->movedir);
	vectoangles (dir, lavaball->s.angles);
	VectorScale (dir, speed, lavaball->velocity);
	lavaball->movetype = MOVETYPE_TOSS;
	lavaball->clipmask = MASK_SHOT;
	lavaball->solid = SOLID_BBOX;
	lavaball->s.effects |= EF_FLAG1;
	lavaball->s.renderfx |= RF_GLOW;

	VectorClear (lavaball->mins);
	VectorClear (lavaball->maxs);
	lavaball->s.modelindex = gi.modelindex ("models/objects/idg3lavaball/tris.md2");
	lavaball->owner = self;
	lavaball->touch = idg3_lavaball_touch;
	lavaball->nextthink = level.time + 8000/speed;
	lavaball->think = G_FreeEdict;
	lavaball->dmg = damage;
	lavaball->radius_dmg = radius_damage;
	lavaball->dmg_radius = damage_radius;
	lavaball->s.sound = gi.soundindex ("weapons/rockfly.wav");
	lavaball->classname = "lavaball";

	gi.linkentity (lavaball);
}

void use_target_lavaball (edict_t *self, edict_t *other, edict_t *activator)
{
	gi.sound (self, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
	idg3_fire_lavaball (self, self->s.origin, self->s.angles, 14, self->speed,16,5);
}

void SP_idg3_target_lavaball(edict_t *self)
{
	int vx = -15 + (crandom() * 30);
	int vy = -15 + (crandom() * 30);
	
	self->use = use_target_lavaball;
	self->noise_index = gi.soundindex ("idg3weapons/rocket/r_exp3.wav");
	
	VectorSet(self->s.angles,vx,vy,32);	
	
	if (!self->speed)
		self->speed = 20;
	self->svflags = SVF_NOCLIENT;
}



//======================================================================
// FLYNN EXPLODING BARREL
//======================================================================

void SP_d_misc_barrel(edict_t *self);
void d_spawnbarrel(edict_t *self)
{
	trace_t tr;

	gi.unlinkentity (self);
	tr = gi.trace (self->s.origin, self->mins, self->maxs, self->s.origin, self, CONTENTS_PLAYERCLIP|CONTENTS_MONSTER);
		//MASK_PLAYERSOLID);
	if (!tr.ent || tr.ent->solid == SOLID_BSP)
	{
//gi.dprintf("spawning barrel\n");
		SP_d_misc_barrel(self);
		gi.sound(self, CHAN_AUTO, gi.soundindex("idg2items/itemrespawn.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
//gi.dprintf("ent blocking - %s\n",tr.ent->classname);
		self->nextthink = level.time + 5;
		self->think =  d_spawnbarrel;
		gi.linkentity (self);
	}
}

void d_barrellexplode (edict_t *self)
{

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 2) 
	{
		if (deathmatch->value)
		{
			//Set respawn		
			self->flags |= FL_RESPAWN;
			self->s.origin[2] -= 20;
			self->svflags |= SVF_NOCLIENT;
			self->solid = SOLID_NOT;
			self->nextthink = level.time + 30;// + random()*5);
			self->think =  d_spawnbarrel;
			gi.linkentity (self);
		}
		else
		// No respawn if not in deathmatch
		G_FreeEdict(self);
	}
}

static void barrel_delay (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t origin;
	
	self->takedamage = DAMAGE_NO;
	self->activator = attacker;
	
	T_RadiusDamage (self, attacker, self->dmg, 0, self->dmg+30, MOD_DOOM_BARREL);

	if(!deathmatch->value)
	{
		vec3_t	org;
		float	spd;
		vec3_t	save;
	VectorCopy (self->s.origin, save);
	VectorMA (self->absmin, 0.5, self->size, self->s.origin);

	// a few big chunks
	spd = 1.5 * (float)self->dmg / 200.0;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org);

	// bottom corners
	spd = 1.75 * (float)self->dmg / 200.0;
	VectorCopy (self->absmin, org);
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[0] += self->size[0];
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[1] += self->size[1];
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[0] += self->size[0];
	org[1] += self->size[1];
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);

	// a bunch of little chunks
	spd = 2 * self->dmg / 200;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);

	VectorCopy (save, self->s.origin);
	}

	gi.sound (self, CHAN_AUTO, gi.soundindex ("idg2world/barexp.wav"), 1.0, ATTN_NORM, 0);	

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	VectorCopy(self->s.origin,origin);
	origin[2] += 20;
	
	// explosion sprite 
	gi.unlinkentity(self);		
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorCopy(origin,self->s.origin);
	VectorCopy(origin,self->s.old_origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/Drexp.sp2"); 
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST; 
	self->think = d_barrellexplode;  
	self->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (self);
}

void barrelanim(edict_t *self)
{
	if(self->s.frame >= 7)
		self->s.frame = 0;
	else
		self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}

void SP_d_misc_barrel(edict_t *self)
{
	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_NONE;
	//self->movetype = MOVETYPE_STEP;
	self->svflags &= ~SVF_NOCLIENT;

	self->model = "models/objects/idg2barrel/tris.md2";
	VectorSet (self->mins, -16, -16, -16);
	VectorSet (self->maxs, 16, 16, 28);
	self->s.modelindex = gi.modelindex (self->model);
	self->s.frame = 0;

	if (!self->mass)
		self->mass = 400;
	if (!self->health)
		self->health = 20;
	if (!self->dmg)
		self->dmg = 150;

	self->die = barrel_delay;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.aiflags = AI_NOSTEP;
	self->touch = NULL;
	self->think = barrelanim;
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/

/*QUAKED info_player_team1 (1 0 0) (-16 -16 -24) (16 16 32)
potential team1 spawning position for ctf games
*/
void SP_info_player_team1(edict_t *self)
{
}

/*QUAKED info_player_team2 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team2(edict_t *self)
{
}

/*QUAKED misc_ctf_banner (1 .5 0) (-4 -64 0) (4 64 248) TEAM2
The origin is the bottom of the banner.
The banner is 248 tall.*/

static void misc_ctf_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_ctf_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/tris.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_ctf_small_banner (1 .5 0) (-4 -32 0) (4 32 124) TEAM2
The origin is the bottom of the banner.
The banner is 124 tall.*/

void SP_misc_ctf_small_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/small.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}