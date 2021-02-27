#include "g_local.h"
#include "g_genutil.h"



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
	ent->s.modelindex = gi.modelindex ("models/objects/q1flame/tris.md2");
	ent->s.frame = 0;
	ent->s.sound = gi.soundindex("q1world/fire1.wav");
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
	ent->s.modelindex = gi.modelindex ("models/objects/q1flame/tris.md2");
	ent->s.frame = 6;
	ent->s.sound = gi.soundindex("q1world/fire1.wav");
	ent->nextthink = level.time + FRAMETIME;
	ent->think = largeflame;
	ent->s.renderfx = RF_FULLBRIGHT;
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
	ent->s.modelindex = gi.modelindex ("models/objects/q1torch/tris.md2");
	ent->s.frame = 0;
	ent->s.sound = gi.soundindex("q1world/fire1.wav");
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
	gi.sound (self, CHAN_AUTO, gi.soundindex("q1weap/nails/s_end.wav"), 1.0, ATTN_NORM, 0);
}


void SP_q1_target_trap(edict_t *self)
{
	self->use = use_target_trapshooter;
	G_SetMovedir (self->s.angles, self->movedir);
	self->noise_index = gi.soundindex ("q1weap/nails/spike.wav");
	gi.soundindex("q1weap/nails/s_end.wav");
	
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
// LIGHTNING BOLT
//======================================================================

//void door_use (edict_t *self, edict_t *other, edict_t *activator);
void door_go_down (edict_t *self);

static void target_fire_lightning (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
	vec3_t end;
	trace_t  tr;

	VectorNormalize(dir);
	VectorMA(start,600,dir,end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos); 
	gi.multicast (start, MULTICAST_PVS);
		
	if ((tr.ent != self) && (tr.ent->takedamage))
		T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 0, DAMAGE_ENERGY, MOD_Q1_LG);
}

void chton_bolt(edict_t *self, int no);
void think_targetbolt(edict_t *self)
{
	int	bstate1,bstate2;
	edict_t *t;
	edict_t *u;


	if(Q_stricmp(level.mapname, "qe1m7") == 0)
	{
		t = G_Find (NULL, FOFS(targetname), "t12");
		if(!t)
			return;
		bstate1=t->moveinfo.state;
		
		u = G_Find (NULL, FOFS(targetname), "t13");
		if(!u)
			return;
		bstate2=u->moveinfo.state;
		if(bstate1 != bstate2)
			return;
	}

	if(level.time >= self->delay)
	{
		self->nextthink =0;
		self->think=0;
		self->delay=0.0;
		return;
	}

	target_fire_lightning (self, self->s.origin, self->movedir, self->dmg);
	gi.sound (self, CHAN_AUTO, gi.soundindex ("q1weap/lightning/lhit.wav"), 1.0, ATTN_NORM, 0);

	if(Q_stricmp(level.mapname, "qe1m7") == 0)
	if(bstate2 == 0 &&
	   bstate1 == 0)
	{
		door_go_down (u);
		door_go_down (t);
		t=G_Find (NULL, FOFS(classname), "q1_monster_chton");
		if(t)
		{
			if(!t->deadflag && t->enemy)
			{
				self->style++;
				//gi.dprintf("HIT NUMBER:%d",self->style);

			switch(self->style)
			{
			case 1:
				chton_bolt(t, 1);
				break;
			case 2:
				chton_bolt(t,2);
				break;
			case 3:
				chton_bolt(t,3);
				break;
			default:
				self->style =0;
				break;
			}
			}
			//else
				//gi.dprintf("CHTON does not have an enemy\n");
		}
		//else
			//gi.dprintf("Could not find Chton\n");

	}
}

void use_target_bolt (edict_t *self, edict_t *other, edict_t *activator)
{
	if(Q_stricmp(level.mapname, "qe1m7") == 0)
	{
		edict_t *t;
		int	bstate1,bstate2;
	
		t = G_Find (NULL, FOFS(targetname), "t12");
		if(!t)
			return;
		bstate1=t->moveinfo.state;
		t = G_Find (NULL, FOFS(targetname), "t13");
		if(!t)
			return;
		bstate2=t->moveinfo.state;
		
		if(bstate1 != bstate2)
			return;

		if(bstate1)	
		{
			t = G_Find (NULL, FOFS(targetname), self->target);
			target_fire_lightning (t, t->s.origin, t->movedir, t->dmg);
			gi.sound (t, CHAN_AUTO, gi.soundindex("q1weap/lightning/lstart.wav"), 1.0, ATTN_NORM, 0);
			t->nextthink = level.time + 0.4;
			t->think = think_targetbolt;
			t->delay = level.time + 5.0;
			gi.sound (t, CHAN_AUTO, t->noise_index, 1, ATTN_NORM, 0);
			return;
		}
	}
	target_fire_lightning (self, self->s.origin, self->movedir, self->dmg);
	gi.sound (self, CHAN_AUTO, gi.soundindex("q1weap/lightning/lstart.wav"), 1.0, ATTN_NORM, 0);
	self->nextthink = level.time + 0.4;
	self->think = think_targetbolt;
	self->delay = level.time + 5.0;
	gi.sound (self, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
}


void SP_q1_target_bolt(edict_t *self)
{
	self->use = use_target_bolt;
	self->style =0;
	G_SetMovedir (self->s.angles, self->movedir);
	self->noise_index = gi.soundindex ("q1weap/lightning/lhit.wav"); 
	gi.soundindex("q1weap/lightning/lstart.wav");
	
	if (!self->dmg)
		self->dmg = 50;
	self->svflags = SVF_NOCLIENT;
}

//======================================================================
// Q1 EXPLOBOX
//======================================================================

void SP_q1_misc_explobox(edict_t *self);
void q1_barrellexplode (edict_t *self)
{
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 5)
		G_FreeEdict(self);
}

static void explobox_delay (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 2 * FRAMETIME;
	self->activator = attacker;
	
	T_RadiusDamage (self, self->activator, self->dmg, NULL, self->dmg+30, MOD_BARREL);

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

	
	gi.sound (self, CHAN_AUTO, gi.soundindex ("q1weap/rocket/r_exp3.wav"), 1.0, ATTN_NORM, 0);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// explosion sprite
	gi.unlinkentity (self);
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/s_explod.sp2"); 
	self->s.frame = 0; 
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST; 
	self->think = q1_barrellexplode; 
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}

void SP_q1_misc_explobox(edict_t *self)
{
	self->solid = SOLID_BBOX;
	//self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags &= ~SVF_NOCLIENT;
	
	if(self->spawnflags & 1)
	{
		self->model = "models/objects/q1explo/small/tris.md2";
		VectorSet (self->mins, -16, -16, -16);
		VectorSet (self->maxs, 16, 16, 24);
	}
	else
	{
		self->model = "models/objects/q1explo/big/tris.md2";
		VectorSet (self->mins, -16, -16, -16);
		VectorSet (self->maxs, 16, 16, 48);
	}
		
	self->s.modelindex = gi.modelindex (self->model);

	if (!self->mass)
		self->mass = 400;
	if (!self->health)
		self->health = 20;
	if (!self->dmg)
		self->dmg = 160;

	self->die = explobox_delay;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.aiflags = AI_NOSTEP;
	self->touch = NULL;
	self->think = M_droptofloor;
	self->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (self);
}

//======================================================================
// Q1 EXPLOBOX
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
		gi.sound(self, CHAN_AUTO, gi.soundindex("ditems/resp.wav"), 1, ATTN_NORM, 0);
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
//Set respawn		
		self->flags |= FL_RESPAWN;
		self->s.origin[2] -= 20;
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->nextthink = level.time + 30;// + random()*5);
		self->think =  d_spawnbarrel;
		gi.linkentity (self);
//		G_FreeEdict(self);
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

	gi.sound (self, CHAN_AUTO, gi.soundindex ("dworld/Dsbarexp.wav"), 1.0, ATTN_NORM, 0);	

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

	self->model = "models/objects/dbarrel/tris.md2";
	VectorSet (self->mins, -16, -16, -16);
	VectorSet (self->maxs, 16, 16, 28);
	self->s.modelindex = gi.modelindex (self->model);
	self->s.frame = 0;
	self->s.renderfx = RF_FULLBRIGHT;

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

//======================================================================
// Trigger SetSkill
//======================================================================

void q1setskill (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(!other->client)
		return;

	if(!other->client->resp.player_class)
		return;
	gi.cvar_forceset("skill", self->message);
}

void SP_q1_trigger_setskill(edict_t *self)
{
	int i;
	
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->touch = q1setskill;
	if(!self->message)
	{
		gi.dprintf("No Skill Setting in Trigger\n");
		G_FreeEdict (self);
		return;
	}
	
	i = atoi(self->message);
	if(i > 4 || i < 0)
	{
		 gi.dprintf("Bad Skill Setting in Trigger\n");
		 G_FreeEdict (self);
		 return;
	}
	gi.linkentity (self);
}



//======================================================================
// Misc Fireball
//======================================================================

void fireballtouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0, 0, 0);
	G_FreeEdict (self);
}
	


void fireballfly(edict_t *self)
{
	edict_t *lavaball;

	gi.dprintf ("Launching Fireball\n");
	
	lavaball = G_Spawn();
	VectorCopy (self->s.origin, lavaball->s.origin);
	VectorCopy (self->movedir, lavaball->movedir);
	vectoangles (self->movedir, lavaball->s.angles);
	VectorSet(lavaball->velocity,((random() * 100) - 50),
								   ((random() * 100) - 50),
								   (self->speed + (random() * 200)));

	lavaball->movetype = MOVETYPE_FLYMISSILE;
	lavaball->clipmask = MASK_SHOT;
	lavaball->solid = SOLID_BBOX;
	lavaball->s.effects |= EF_FLAG1;
	lavaball->s.renderfx |= RF_GLOW;
	VectorClear (lavaball->mins);
	VectorClear (lavaball->maxs);
	lavaball->s.modelindex = gi.modelindex ("models/monsters/q1monstr/boss/lavaball/tris.md2");
	lavaball->owner = self;
	lavaball->touch = fireballtouch;
	lavaball->nextthink = level.time + 2;
	lavaball->think = G_FreeEdict;
	lavaball->dmg = 20;
	lavaball->classname = "lavaball";
	gi.linkentity (lavaball);
}

void SP_q1_misc_fireball(edict_t *self)
{
	gi.modelindex ("models/monsters/q1monstr/boss/lavaball/tris.md2");
	self->nextthink = level.time + (random() * 5.0);
	self->think = fireballfly;
	self->classname = "fireball";
	gi.linkentity(self);
}





/*--------------------------------------------------------------------------
 * just here to help old map conversions
 *--------------------------------------------------------------------------*/

static void old_teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	int			i;
	vec3_t		forward;

	if (!other->client)
		return;
	dest = G_Find (NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

//ZOID
		CTFPlayerResetGrapple(other);
//ZOID

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
//	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	other->client->ps.pmove.pm_time = 160>>3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->enemy->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i=0 ; i<3 ; i++)
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

	other->s.angles[PITCH] = 0;
	other->s.angles[YAW] = dest->s.angles[YAW];
	other->s.angles[ROLL] = 0;
	VectorCopy (dest->s.angles, other->client->ps.viewangles);
	VectorCopy (dest->s.angles, other->client->v_angle);

	// give a little forward velocity
	AngleVectors (other->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 200, other->velocity);

	// kill anything at the destination
	if (!KillBox (other))
	{
	}

	gi.linkentity (other);
}

/*QUAKED trigger_teleport (0.5 0.5 0.5) ?
Players touching this will be teleported
*/
void SP_trigger_teleport (edict_t *ent)
{
	edict_t *s;
	int i;

	if (!ent->target)
	{
		gi.dprintf ("teleporter without a target.\n");
		G_FreeEdict (ent);
		return;
	}

	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->touch = old_teleporter_touch;
	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);

	// noise maker and splash effect dude
	s = G_Spawn();
	ent->enemy = s;
	for (i = 0; i < 3; i++)
		s->s.origin[i] = ent->mins[i] + (ent->maxs[i] - ent->mins[i])/2;
	s->s.sound = gi.soundindex ("world/hum1.wav");
	gi.linkentity(s);
	
}

/*QUAKED info_teleport_destination (0.5 0.5 0.5) (-16 -16 -24) (16 16 32)
Point trigger_teleports at these.
*/
void SP_info_teleport_destination (edict_t *ent)
{
	ent->s.origin[2] += 16;
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



/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/


qboolean CheckMapEntFile(char *mapname)
{
	char filename[MAX_QPATH] = "";
	FILE *f;

	memcpy(filename,"gen\\ent\\",10);
    strcat(filename, mapname);
    strcat(filename, ".ent");

	f=fopen(filename, "r");

gi.dprintf("Searching for:%s\n",filename);
	
	if(f)
	{
gi.dprintf("Loading:%s\n",filename);		
		fclose(f);
		return true;
	}
	return false;
}


//======================================================================
// Trigger Teleport
//======================================================================

static void q1_teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	int			i;
	vec3_t		forward;

	if (!other->client)
		return;
	dest = G_Find (NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

//ZOID
		CTFPlayerResetGrapple(other);
//ZOID

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
//	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	
	other->client->ps.pmove.pm_time = 160>>3;					// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;
	self->enemy->s.event = EV_PLAYER_TELEPORT;
	GenQ1TeleportSounds2(self,dest);

	// set angles
	for (i=0 ; i<3 ; i++)
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

	other->s.angles[PITCH] = 0;
	other->s.angles[YAW] = dest->s.angles[YAW];
	other->s.angles[ROLL] = 0;
	VectorCopy (dest->s.angles, other->client->ps.viewangles);
	VectorCopy (dest->s.angles, other->client->v_angle);

	// give a little forward velocity
	AngleVectors (other->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 250, other->velocity);

	// kill anything at the destination
	if (!KillBox (other))
	{
	}
	gi.linkentity (other);
}


void SP_q1_trigger_teleport(edict_t *self)
{
	edict_t *s;
	int i;

	if (!self->target)
	{
		gi.dprintf ("teleporter without a target.\n");
		G_FreeEdict (self);
		return;
	}

	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_TRIGGER;
	self->touch = q1_teleporter_touch;
	gi.setmodel (self, self->model);
	gi.linkentity (self);

	// noise maker and splash effect dude
	s = G_Spawn();
	self->enemy = s;
	for (i = 0; i < 3; i++)
		s->s.origin[i] = self->mins[i] + (self->maxs[i] - self->mins[i])/2;
	s->s.sound = gi.soundindex ("q1world/hum1.wav");
	gi.linkentity(s);

}
