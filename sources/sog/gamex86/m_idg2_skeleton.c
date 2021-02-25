//=======================================================
//
//  IDG2 SKELETON  ( DOOM II REVENANT )
//
//=======================================================

#include "g_local.h"
#include "m_idg2_skeleton.h"

static int sound_death;
static int sound_pain;
static int sound_sight;
static int sound_punch;
static int sound_shoot; 


//
// Revenant Homing Missiles
//
// by Sir Psycho
//

void revenant_fireball_think (edict_t *ent)
{
	edict_t *target = NULL;
	edict_t *blip = NULL;
	vec3_t targetdir, blipdir;
	vec_t speed;

	while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
	{
		if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			continue;
		if (blip == ent->owner)
			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health <= 0)
			continue;
		if (!visible(ent, blip))
			continue;
		if (!infront(ent, blip))
			continue;
		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		blipdir[2] += 16;
		if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
		{
			target = blip;
			VectorCopy(blipdir, targetdir);
		}
	}

	if (target != NULL)
	{
		// target acquired, nudge our direction toward it
		VectorNormalize(targetdir);
		VectorScale(targetdir, 0.5, targetdir); // Adjust the 0.5 to change how fast it turns
		VectorAdd(targetdir, ent->movedir, targetdir);
		VectorNormalize(targetdir);
		VectorCopy(targetdir, ent->movedir);
		vectoangles(targetdir, ent->s.angles);
		speed = VectorLength(ent->velocity);
		VectorScale(targetdir, speed, ent->velocity);
	}

	ent->nextthink = level.time + 0.1;
}

void revenant_fireball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t origin;
	int n;

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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
				ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void revenant_fire_fireball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t *fireball;

	fireball = G_Spawn();
	VectorCopy (start, fireball->s.origin);
	VectorCopy (dir, fireball->movedir);
	vectoangles (dir, fireball->s.angles);
	VectorScale (dir, speed, fireball->velocity);
	fireball->movetype = MOVETYPE_FLYMISSILE;
	fireball->clipmask = MASK_SHOT;
	fireball->solid = SOLID_BBOX;
	fireball->s.effects |= EF_ROCKET;
	fireball->s.renderfx |= RF_GLOW;
	VectorClear (fireball->mins);
	VectorClear (fireball->maxs);

	fireball->s.modelindex = gi.modelindex ("models/objects/idg3lavaball/tris.md2");
	fireball->owner = self;
	fireball->think = revenant_fireball_think;
	fireball->nextthink = level.time + 0.1;
	fireball->touch = revenant_fireball_touch;
	fireball->dmg = damage;
	fireball->radius_dmg = radius_damage;
	fireball->dmg_radius = damage_radius;
	fireball->classname = "fireball";

	gi.linkentity (fireball);
} 


// end Sir Psycho

void idg2_skeleton_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


//STAND

void idg2_skeleton_stand (edict_t *self);

mframe_t idg2_skeleton_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};

mmove_t	idg2_skeleton_move_stand = {FRAME_stand1, FRAME_stand10, idg2_skeleton_frames_stand, NULL};

void idg2_skeleton_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_skeleton_move_stand;
}

//WALK

mframe_t idg2_skeleton_frames_walk [] =
{
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,

	ai_walk, 5, NULL,
	ai_walk, 5, NULL
};
mmove_t idg2_skeleton_move_walk = {FRAME_walk1, FRAME_walk7, idg2_skeleton_frames_walk, NULL};

void idg2_skeleton_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_skeleton_move_walk;
}

//RUN

mframe_t idg2_skeleton_frames_run [] =
{
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL
};

mmove_t idg2_skeleton_move_run = {FRAME_walk1, FRAME_walk7, idg2_skeleton_frames_run, NULL};

void idg2_skeleton_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &idg2_skeleton_move_walk;
	else
		self->monsterinfo.currentmove = &idg2_skeleton_move_run;
}

//PAIN
mframe_t idg2_skeleton_frames_pain [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL,
	ai_move, 2,	 NULL,
	ai_move, 1,  NULL
};
mmove_t idg2_skeleton_move_pain = {FRAME_pain1, FRAME_pain4, idg2_skeleton_frames_pain, idg2_skeleton_run};

void idg2_skeleton_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	gi.sound (self, CHAN_VOICE, sound_pain, 1.0, ATTN_NORM, 0);	

	if(damage > 25)
	{
		self->monsterinfo.currentmove = &idg2_skeleton_move_pain;
		self->pain_debounce_time = level.time + 3;
	}
}

//Death

void idg2_skeleton_dead (edict_t *self)
{
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, 0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t idg2_skeleton_frames_death [] =
{
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	
	ai_move, 0, NULL,
	ai_move, 0,	NULL
};
mmove_t idg2_skeleton_move_death = {FRAME_death1, FRAME_death12, idg2_skeleton_frames_death, idg2_skeleton_dead};


void idg2_skeleton_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)

	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("idg2/gib.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;



// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &idg2_skeleton_move_death;
}



//ATTACK

// rockets

void idg2_skeleton_fire(edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	vec3_t  offset;

	if(!self->enemy)
		return;


	AngleVectors (self->s.angles, forward, right, NULL);
	VectorSet(offset,8,16,32);	
	G_ProjectSource (self->s.origin, offset, forward, right, start);

	VectorCopy (self->enemy->s.origin, vec);
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	gi.sound (self, CHAN_WEAPON, sound_shoot, 1.0, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_BOSS2_ROCKET_1);
	gi.multicast (start, MULTICAST_PVS);

	revenant_fire_fireball (self,start, dir, 50, 300, 100, 100);
}

mframe_t idg2_skeleton_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, idg2_skeleton_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t idg2_skeleton_move_attack = {FRAME_attackb1, FRAME_attackb5, idg2_skeleton_frames_attack, idg2_skeleton_run};


void idg2_skeleton_attack(edict_t *self)
{
	if (!self->enemy)
		return;
	
	if(!self->enemy->inuse) 
		return;

	if (infront(self,self->enemy) && visible(self,self->enemy))
		self->monsterinfo.currentmove = &idg2_skeleton_move_attack;
	else
	{
		if(random() < 0.5)
			gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &idg2_skeleton_move_run;
	}
}

static void idg2_skeleton_punch (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 8);
	if (fire_hit (self, aim, (5 + (rand() %5)), 100))
		gi.sound (self, CHAN_WEAPON, sound_punch, 1, ATTN_NORM, 0);
}


mframe_t idg2_skeleton_frames_melee [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, idg2_skeleton_punch
};
mmove_t idg2_skeleton_move_melee = {FRAME_attacka1, FRAME_attacka4, idg2_skeleton_frames_melee, idg2_skeleton_run};

void idg2_skeleton_melee(edict_t *self)
{
	if (!self->enemy)
		return;
	
	if(!self->enemy->inuse) 
		return;

	if(!self->enemy->health)
		return;

	if (range (self, self->enemy) == RANGE_MELEE)
		self->monsterinfo.currentmove = &idg2_skeleton_move_melee;
}



//
// SPAWN
//

/*QUAKED monster_idg2_skeleton (1 .5 0) (-32 -32 -24) (32 32 80) Ambush Trigger_Spawn Sight
Revenant
*/

void SP_monster_idg2_skeleton (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		SP_monster_idg1_priest (self);
		return;
	}

	sound_pain =gi.soundindex ("idg2monster/skeleton/pain.wav");	
	sound_death=gi.soundindex ("idg2monster/skeleton/death.wav");	
	sound_sight=gi.soundindex ("idg2monster/skeleton/sight.wav");
	sound_punch=gi.soundindex ("idg2weapons/punch.wav");
	sound_shoot=gi.soundindex ("idg2weapons/rocket.wav");	
	
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/idg2/skeleton/tris.md2");
		
	VectorSet (self->mins, -28, -28, -24);
	VectorSet (self->maxs, 28, 28, 80);

	self->health = 500;
	self->gib_health = -80;
	self->mass = 300;
	self->pain = idg2_skeleton_pain;
	self->die = idg2_skeleton_die;

	self->flags |= FL_D_MONSTER;

	self->monsterinfo.stand = idg2_skeleton_stand;
	self->monsterinfo.walk = idg2_skeleton_walk;
	self->monsterinfo.run = idg2_skeleton_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = idg2_skeleton_attack;
	self->monsterinfo.melee = idg2_skeleton_melee;
	self->monsterinfo.sight = idg2_skeleton_sight;
	self->monsterinfo.search = idg2_skeleton_stand;

	gi.linkentity (self);
	self->monsterinfo.currentmove = &idg2_skeleton_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;
	walkmonster_start (self);
}



