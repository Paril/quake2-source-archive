/*
==============================================================================

parasite

==============================================================================
*/

#include "g_local.h"
#include "m_parasite.h"


static int	sound_pain1;
static int	sound_pain2;
static int	sound_die;
static int	sound_launch;
static int	sound_impact;
static int	sound_suck;
static int	sound_reelin;
static int	sound_sight;
static int	sound_tap;
static int	sound_scratch;
static int	sound_search;

void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);
void drone_ai_walk (edict_t *self, float dist);
void myparasite_stand (edict_t *self);
void myparasite_start_run (edict_t *self);
void myparasite_run (edict_t *self);
void parasite_walk (edict_t *self);
void myparasite_start_walk (edict_t *self);
void myparasite_end_fidget (edict_t *self);
void myparasite_do_fidget (edict_t *self);
void myparasite_refidget (edict_t *self);
void myparasite_checkattack (edict_t *self);
void myparasite_attack1 (edict_t *self);
void myparasite_continue (edict_t *self);


void myparasite_launch (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_launch, 1, ATTN_NORM, 0);
}

void myparasite_reel_in (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_reelin, 1, ATTN_NORM, 0);
}

void myparasite_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_WEAPON, sound_sight, 1, ATTN_NORM, 0);
	myparasite_attack1(self);
}

void myparasite_tap (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_tap, 1, ATTN_IDLE, 0);
}

void myparasite_scratch (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_scratch, 1, ATTN_IDLE, 0);
}

void myparasite_search (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_search, 1, ATTN_IDLE, 0);
}


mframe_t myparasite_frames_start_fidget [] =
{
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL
};
mmove_t myparasite_move_start_fidget = {FRAME_stand18, FRAME_stand21, myparasite_frames_start_fidget, myparasite_do_fidget};

mframe_t myparasite_frames_fidget [] =
{	
	drone_ai_stand, 0, myparasite_scratch,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, myparasite_scratch,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL
};
mmove_t myparasite_move_fidget = {FRAME_stand22, FRAME_stand27, myparasite_frames_fidget, myparasite_refidget};

mframe_t myparasite_frames_end_fidget [] =
{
	drone_ai_stand, 0, myparasite_scratch,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL
};
mmove_t myparasite_move_end_fidget = {FRAME_stand28, FRAME_stand35, myparasite_frames_end_fidget, myparasite_stand};

void myparasite_end_fidget (edict_t *self)
{
	self->monsterinfo.currentmove = &myparasite_move_end_fidget;
}

void myparasite_do_fidget (edict_t *self)
{
	self->monsterinfo.currentmove = &myparasite_move_fidget;
}

void myparasite_refidget (edict_t *self)
{ 
	if (random() <= 0.8)
		self->monsterinfo.currentmove = &myparasite_move_fidget;
	else
		self->monsterinfo.currentmove = &myparasite_move_end_fidget;
}

void myparasite_idle (edict_t *self)
{ 
	self->monsterinfo.currentmove = &myparasite_move_start_fidget;
}


mframe_t myparasite_frames_stand [] =
{
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, myparasite_tap,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, myparasite_tap,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, myparasite_tap,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, myparasite_tap,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, myparasite_tap,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, myparasite_tap
};
mmove_t	myparasite_move_stand = {FRAME_stand01, FRAME_stand17, myparasite_frames_stand, myparasite_stand};

void myparasite_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &myparasite_move_stand;
}

mframe_t parasite_frames_walk [] =
{
	drone_ai_walk, 30, NULL,
	drone_ai_walk, 30, NULL,
	drone_ai_walk, 22, NULL,
	drone_ai_walk, 19, NULL,
	drone_ai_walk, 24, NULL,
	drone_ai_walk, 28, NULL,
	drone_ai_walk, 25, NULL
};
mmove_t parasite_move_walk = {FRAME_run03, FRAME_run09, parasite_frames_walk, parasite_walk};

mframe_t parasite_frames_start_walk [] =
{
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 30, parasite_walk
};
mmove_t parasite_move_start_walk = {FRAME_run01, FRAME_run02, parasite_frames_start_walk, NULL};

mframe_t parasite_frames_stop_walk [] =
{	
	drone_ai_walk, 20, NULL,
	drone_ai_walk, 20,	NULL,
	drone_ai_walk, 12, NULL,
	drone_ai_walk, 10, NULL,
	drone_ai_walk, 0,  NULL,
	drone_ai_walk, 0,  NULL
};
mmove_t parasite_move_stop_walk = {FRAME_run10, FRAME_run15, parasite_frames_stop_walk, NULL};

void parasite_start_walk (edict_t *self)
{	
	if (!self->goalentity)
		self->goalentity = world;
	self->monsterinfo.currentmove = &parasite_move_start_walk;
}

void parasite_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &parasite_move_walk;
}

mframe_t myparasite_frames_run [] =
{
	drone_ai_run, 35, NULL,
	drone_ai_run, 35, NULL,
	drone_ai_run, 35, NULL,
	drone_ai_run, 35, NULL,
	drone_ai_run, 35, NULL,
	drone_ai_run, 35, NULL,
	drone_ai_run, 35, NULL
};
mmove_t myparasite_move_run = {FRAME_run03, FRAME_run09, myparasite_frames_run, NULL};

mframe_t myparasite_frames_start_run [] =
{
	drone_ai_run, 30,	NULL,
	drone_ai_run, 30, NULL,
};
mmove_t myparasite_move_start_run = {FRAME_run01, FRAME_run02, myparasite_frames_start_run, myparasite_run};

void myparasite_start_run (edict_t *self)
{	
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &myparasite_move_stand;
	else
		self->monsterinfo.currentmove = &myparasite_move_start_run;
}

void myparasite_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &myparasite_move_stand;
	else
		self->monsterinfo.currentmove = &myparasite_move_run;
}

static qboolean ParasiteCanAttack (edict_t *self, vec3_t start, vec3_t end)
{
	vec3_t f, r, offset;

	if (!self->enemy)
		return false;
	if (entdist(self, self->enemy) > 128)
		return false;
	if (!infront(self, self->enemy))
	{
		//gi.dprintf("not infront\n");
		if (!self->groundentity || self->groundentity != self->enemy)
			return false;
	}
	//if (!nearfov(self, self->enemy, 90, 45))
	//	return false;

	// get starting point
	AngleVectors (self->s.angles, f, r, NULL);
	VectorSet (offset, 24, 0, 6);
	G_ProjectSource (self->s.origin, offset, f, r, start);

	// target the midpoint of enemy
	G_EntMidPoint(self->enemy, end);

	// make sure there is a clear shot
	//if (!G_IsClearPath(self->enemy, MASK_SHOT, start, end))
	if (!G_ClearShot(self, start, self->enemy))
	{
		//gi.dprintf("no clear path\n");
		return false;
	}
	return true;
}

void myparasite_drain_attack (edict_t *self)
{
	vec3_t start, end, dir;
	int damage, pull=0;

	if (!ParasiteCanAttack(self, start, end))
		return;

	self->lastsound = level.framenum;

	if (self->s.frame == FRAME_drain03)
	{
		gi.sound (self->enemy, CHAN_AUTO, sound_impact, 1, ATTN_NORM, 0);
	}
	else
	{
		if (self->s.frame == FRAME_drain04)
			gi.sound (self, CHAN_WEAPON, sound_suck, 1, ATTN_NORM, 0);
	}

	damage = 10+self->monsterinfo.level;

	// don't pull while mid-air
	if (self->groundentity)
	{
		pull = -60;
		if (self->enemy->groundentity)
			pull *= 2;
	}

	// gain damage back as health
	if (self->health < self->max_health)
	{
		self->health += damage;
		if (self->health > self->max_health)
			self->health = self->max_health;
	}

	// parasite tongue effect
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PARASITE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (start);
	gi.WritePosition (end);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	VectorSubtract (end, start, dir);

	T_Damage (self->enemy, self, self, dir, self->enemy->s.origin, 
		end, damage, pull, DAMAGE_NO_ABILITIES, MOD_UNKNOWN);
}

mframe_t myparasite_frames_drain [] =
{
	ai_charge, 0,	myparasite_launch,
	ai_charge, 0,	myparasite_drain_attack,
	ai_charge, 0,	myparasite_drain_attack,			// Target hits
	ai_charge, 0,	myparasite_drain_attack,			// drain
	ai_charge, 0,	myparasite_drain_attack,			// drain
	ai_charge, 0,	myparasite_drain_attack,			// drain
	ai_charge, 0,	myparasite_drain_attack,			// drain
	ai_charge, 0,  myparasite_drain_attack,			// drain
	ai_charge, 0,	myparasite_drain_attack,			// drain
	ai_charge, 0,	myparasite_drain_attack,			// drain
	ai_charge, 0,	myparasite_drain_attack,			// drain
	ai_charge, 0,	myparasite_drain_attack,			// drain
	ai_charge, 0,  myparasite_drain_attack,			// drain
	ai_charge, 0,	myparasite_reel_in				// let go
	//ai_charge, 0,	NULL,
	//ai_charge, 0,	NULL,
	//ai_charge, 0,	NULL,
	//ai_charge, 0,	NULL
};
mmove_t myparasite_move_drain = {FRAME_drain01, FRAME_drain14, myparasite_frames_drain, myparasite_run};

mframe_t myparasite_frames_runandattack [] =
{
	drone_ai_run, 30, myparasite_drain_attack,
	drone_ai_run, 30, myparasite_drain_attack,
	drone_ai_run, 30, myparasite_drain_attack,
	drone_ai_run, 30, myparasite_drain_attack,
	drone_ai_run, 30, myparasite_drain_attack,
	drone_ai_run, 30, myparasite_drain_attack,
	drone_ai_run, 30, myparasite_drain_attack
};
mmove_t myparasite_move_runandattack = {FRAME_run03, FRAME_run09, myparasite_frames_runandattack, myparasite_continue};

void myparasite_continue (edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && (entdist(self, self->enemy) <= 128))
		self->monsterinfo.currentmove = &myparasite_move_runandattack;
}

mframe_t myparasite_frames_break [] =
{
	ai_charge, 0,	NULL,
	ai_charge, -3,	NULL,
	ai_charge, 1,	NULL,
	ai_charge, 2,	NULL,
	ai_charge, -3,	NULL,
	ai_charge, 1,	NULL,
	ai_charge, 1,	NULL,
	ai_charge, 3,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, -18,	NULL,
	ai_charge, 3,	NULL,
	ai_charge, 9,	NULL,
	ai_charge, 6,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, -18,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 8,	NULL,
	ai_charge, 9,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, -18,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,		// airborne
	ai_charge, 0,	NULL,		// airborne
	ai_charge, 0,	NULL,		// slides
	ai_charge, 0,	NULL,		// slides
	ai_charge, 0,	NULL,		// slides
	ai_charge, 0,	NULL,		// slides
	ai_charge, 4,	NULL,
	ai_charge, 11,	NULL,		
	ai_charge, -2,	NULL,
	ai_charge, -5,	NULL,
	ai_charge, 1,	NULL
};
mmove_t myparasite_move_break = {FRAME_break01, FRAME_break32, myparasite_frames_break, myparasite_start_run};

/*
=== 
Break Stuff Ends
===
*/
void myparasite_checkattack (edict_t *self)
{
	vec3_t start, end;

	if (!ParasiteCanAttack(self, start, end))
		return;

//	if (random() > 0.5)
		self->monsterinfo.currentmove = &myparasite_move_drain;
//	else 
//		self->monsterinfo.currentmove = &myparasite_move_runandattack;

	// don't call the attack function again for awhile!
	//self->monsterinfo.attack_finished = level.time + 1;
}

void myparasite_attack1 (edict_t *self)
{
	if (!self->enemy)
		return;
	if (!self->enemy->inuse)
		return;

	myparasite_checkattack(self);
}

/*
===
Death Stuff Starts
===
*/

void myparasite_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	//self->nextthink = 0;
	gi.linkentity (self);
	M_PrepBodyRemoval(self);
}

mframe_t myparasite_frames_death [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t myparasite_move_death = {FRAME_death101, FRAME_death107, myparasite_frames_death, myparasite_dead};

void myparasite_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	M_Notify(self);

	// reduce lag by removing the entity right away
	if (nolag->value)
	{
		M_Remove(self, false, true);
		return;
	}

	//if (self->deadflag != DEAD_DEAD)
	//	level.total_monsters--;

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		//ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		//self->deadflag = DEAD_DEAD;
		M_Remove(self, false, false);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &myparasite_move_death;

	DroneList_Remove(self);
	if (self->activator && !self->activator->client)
	{
		self->activator->num_monsters_real--;
		// gi.bprintf(PRINT_HIGH, "releasing %p (%d)\n", self, self->activator->num_monsters_real);
	}
}

/*
===
End Death Stuff
===
*/

void myparasite_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;

	if (other && other->client && self->activator && self->activator == other) {
		AngleVectors (other->client->v_angle, forward, right, NULL);
		VectorScale (forward, -3, other->client->kick_origin);
		VectorSet(offset, 0, 7,  other->viewheight-8);
		P_ProjectSource (other->client, other->s.origin, offset, forward, right, start);

		self->velocity[0] += forward[0] * 50;
		self->velocity[1] += forward[1] * 50;
		self->velocity[2] += forward[2] * 50;
//		if (self->groundentity)
//			self->velocity[2] = 250;
	}
}

void myparasite_melee (edict_t *self)
{
	// prevent circle-strafing
}

/*QUAKED monster_parasite (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void init_drone_parasite (edict_t *self)
{
//	if (deathmatch->value)
//	{
//		G_FreeEdict (self);
//		return;
//	}

	sound_pain1 = gi.soundindex ("parasite/parpain1.wav");	
	sound_pain2 = gi.soundindex ("parasite/parpain2.wav");	
	sound_die = gi.soundindex ("parasite/pardeth1.wav");	
	sound_launch = gi.soundindex("parasite/paratck1.wav");
	sound_impact = gi.soundindex("parasite/paratck2.wav");
	sound_suck = gi.soundindex("parasite/paratck3.wav");
	sound_reelin = gi.soundindex("parasite/paratck4.wav");
	sound_sight = gi.soundindex("parasite/parsght1.wav");
	sound_tap = gi.soundindex("parasite/paridle1.wav");
	sound_scratch = gi.soundindex("parasite/paridle2.wav");
	sound_search = gi.soundindex("parasite/parsrch1.wav");

	self->s.modelindex = gi.modelindex ("models/monsters/parasite/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 0);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	//if (self->activator && self->activator->client)
	self->health = 110 + 65*self->monsterinfo.level;
	//else self->health = 200 + 80*self->monsterinfo.level;

	self->max_health = self->health;
	self->gib_health = -100;
	self->mass = 100;

//	self->pain = myparasite_pain;
	self->die = myparasite_die;
//	self->touch = myparasite_touch;

	self->monsterinfo.stand = myparasite_stand;
	self->monsterinfo.walk = parasite_start_walk;
	self->monsterinfo.jumpdn = 512;
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.run = myparasite_start_run;
	self->monsterinfo.attack = myparasite_attack1;
	self->monsterinfo.sight = myparasite_sight;
	//self->monsterinfo.idle = myparasite_idle;

	//K03 Begin
	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	self->monsterinfo.power_armor_power = 0;
	self->monsterinfo.control_cost = M_PARASITE_CONTROL_COST;
	self->monsterinfo.cost = M_PARASITE_COST;
	self->monsterinfo.aiflags |= AI_NO_CIRCLE_STRAFE;
	self->monsterinfo.melee = myparasite_melee;
	//self->monsterinfo.melee = 1;
	self->mtype = M_PARASITE;
	//K03 End

	gi.linkentity (self);

	self->monsterinfo.currentmove = &myparasite_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

//	walkmonster_start (self);
	self->nextthink = level.time + FRAMETIME;

	//self->activator->num_monsters += self->monsterinfo.control_cost;
}
