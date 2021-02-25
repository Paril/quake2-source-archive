/*
==============================================================================

DOOM PAIN ELEMENTAL

==============================================================================
*/

#include "g_local.h"
#include "m_idg2_skullspawner.h"

void ED_CallSpawn (edict_t *ent);

#define MAX_LOST_SOULS 12

static int	sound_sight;
static int	sound_pain;
static int	sound_death;


static int Abs(int i)
{
	if (i >= 0) return i; 
	else return - i;
}; 

//
// CheckSkullSpawn
//
// Checks if a Lost Soul can spawn at the given location
//

qboolean CheckSkullSpawn (vec3_t position)
{
	int i;
	int count = 0;
	edict_t	*ent;

	ent = &g_edicts[0];
	
	for (i=0 ; i<globals.num_edicts ; i++, ent++)
	{
		if (!ent->inuse)
			continue;
			
		if (ent->solid != SOLID_BBOX)
			continue;
	
		
		// Checking if another object is too close to the proposed spawn location.
		
		if ((Abs(ent->s.origin[0] - position[0]) < (ent->maxs[0] + 18)) &&
			(Abs(ent->s.origin[1] - position[1]) < (ent->maxs[1] + 18)) &&
			(Abs(ent->s.origin[2] - position[2]) < (ent->maxs[2] + 18)))
			return false;
	
		// Counting number of Lost Souls that are already in the map,
		// to make sure that we don't flood the map with Lost Souls.
	
		if (strcmp(ent->classname, "monster_idg2_skull") == 0) count++; 
	
	}
	
	if (count > MAX_LOST_SOULS) return false;
	
	return true;
}



void idg2_skullspawner_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


mframe_t idg2_skullspawner_frames_stand [] =
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
mmove_t	idg2_skullspawner_move_stand = {FRAME_stand1, FRAME_stand10, idg2_skullspawner_frames_stand, NULL};

void idg2_skullspawner_stand (edict_t *self)
{	self->monsterinfo.currentmove = &idg2_skullspawner_move_stand;
}

mframe_t idg2_skullspawner_frames_walk [] =
{
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL,
	ai_walk, 5, NULL
};
mmove_t	idg2_skullspawner_move_walk = {FRAME_walk1, FRAME_walk5, idg2_skullspawner_frames_walk, NULL};

void idg2_skullspawner_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_skullspawner_move_walk;
}


mframe_t idg2_skullspawner_frames_run [] =
{
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL
};
mmove_t idg2_skullspawner_move_run = {FRAME_walk1, FRAME_walk5, idg2_skullspawner_frames_run, NULL};

void idg2_skullspawner_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &idg2_skullspawner_move_walk;
	else
		self->monsterinfo.currentmove = &idg2_skullspawner_move_run;
}

mframe_t idg2_skullspawner_frames_pain [] =
{	
		ai_move, -2, NULL,
		ai_move, -2, NULL,
		ai_move, -1, NULL,
		ai_move, 0,  NULL
};
mmove_t idg2_skullspawner_move_pain = {FRAME_pain1, FRAME_pain4, idg2_skullspawner_frames_pain, idg2_skullspawner_run};


void idg2_skullspawner_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	if (skill->value == 3)

		return;		// no pain anims in nightmare

	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	self->monsterinfo.currentmove = &idg2_skullspawner_move_pain;
}


void idg2_skullspawner_spawn_skull (edict_t *self)
{
	edict_t *skull;
	
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t  offset;
	
	trace_t      tr;

	// Calculate a good spawn position in front of the Pain Elemental
	
	AngleVectors (self->s.angles, forward, right, NULL);
	
	VectorSet(offset,92,0,16);	
	
	G_ProjectSource (self->s.origin, offset, forward, right, start);	

	if (!CheckSkullSpawn(start))
		return;
	
	// Prepare Lost Soul
	
	skull = G_Spawn();
	
	VectorCopy(start, skull->s.origin);
	
	skull->classname = "monster_idg2_skull";

	// Check if spawn position isn't behind a wall
	
	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	
	if (tr.fraction < 1.000)
   		return;

	// Give the Lost Soul a little push
	
	VectorSubtract (start, self->s.origin, dir);
	
	VectorNormalize (dir);
	
	VectorScale(dir, 200, skull->velocity); // Give it a bit of a push start
	
	VectorCopy(dir, skull->movedir);
	
	VectorCopy (self->s.angles, skull->s.angles);	
	
	
	skull->spawnflags = 0;

	skull->monsterinfo.aiflags = 0;

	skull->target = NULL;

	skull->targetname = NULL;

	skull->combattarget = NULL;

	skull->deathtarget = NULL;

	skull->owner = self;

	ED_CallSpawn (skull);

	skull->owner = NULL;

	if (skull->think)

	{

		skull->nextthink = level.time;

		skull->think (skull);

	}
		
}


mframe_t idg2_skullspawner_frames_attack [] =
{
		ai_charge, 0, NULL,
		ai_charge, 0, NULL,
		ai_charge, 0, NULL,
		ai_charge, 0, NULL,
		ai_charge, 0, idg2_skullspawner_spawn_skull,
		
		ai_charge, 0, NULL,
		ai_charge, 0, NULL,
		ai_charge, 0, NULL,
		ai_charge, 0, NULL
};
mmove_t idg2_skullspawner_move_attack = {FRAME_attack1, FRAME_attack9, idg2_skullspawner_frames_attack, idg2_skullspawner_run};




void idg2_skullspawner_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_skullspawner_move_attack;
}


void idg2_skullspawner_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	BecomeExplosion1(self);
}
	

/*QUAKED monster_idg2_skullspawner (1 .5 0) (-64 -64 -24) (64 64 64) Ambush Trigger_Spawn Sight
DOOM II Pain Elemental
*/
void SP_monster_idg2_skullspawner (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_sight = gi.soundindex ("idg2monster/skullspawner/sight.wav");
	sound_pain = gi.soundindex ("idg2monster/skullspawner/pain.wav");
	sound_death = gi.soundindex ("idg2monster/skullspawner/death.wav");
	
	self->s.modelindex = gi.modelindex ("models/monsters/idg2/skullspawner/tris.md2");
	VectorSet (self->mins, -64, -64, -24);
	VectorSet (self->maxs, 64, 64, 64);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	self->health = 400;
	self->gib_health = -70;
	self->mass = 50;

	self->pain = idg2_skullspawner_pain;
	self->die = idg2_skullspawner_die;

	self->monsterinfo.stand = idg2_skullspawner_stand;
	self->monsterinfo.walk = idg2_skullspawner_walk;
	self->monsterinfo.run = idg2_skullspawner_run;
	self->monsterinfo.attack = idg2_skullspawner_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = idg2_skullspawner_sight;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &idg2_skullspawner_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	flymonster_start (self);
}
