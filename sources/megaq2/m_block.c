#include "m_block.h"
#include "g_local.h"

#define BLOCK_WAIT 			15;
#define BLOCK_ATTACK_WAIT	5;

void block_attackstop (edict_t *self);
void block_attackloop (edict_t *self);
void block_attackstart (edict_t *self);
void block_waitstart (edict_t *self);
void block_waitloop (edict_t *self);
void block_idle (edict_t *self);

mmove_t block_move_waitstart;

//frames for attackstop
//3 frames of eye closing
//no movement
mframe_t block_frames_attackstop [] =
{
	ai_stand, 0 ,NULL,
	ai_stand, 0 ,NULL,
	ai_stand, 0 ,NULL
};
//moveinfo for attackstop
//3 frames, open3 to open5
//run attackstop at end of frames
mmove_t block_move_attackstop = {FRAME_open3, FRAME_open5, block_frames_attackstop, block_attackstop};

//attackstop function
//set moveinfo to wait
//set takedamage to false
void block_attackstop (edict_t *self)
{
	self->takedamage = false; //cannot damage
	self->monsterinfo.currentmove = &block_move_waitstart; //start waitloop again
}


//frames for attackgo
//1 frame of eye open
//no movement
mframe_t block_frames_attackgo [] =
{
	ai_charge, 0, NULL
};
//moveinfo for attackgo
//1 frame, open3 to open3
//attack loop gets called at end of frames
mmove_t block_move_attackgo = {FRAME_open3, FRAME_open3, block_frames_attackgo, block_attackloop};

//attackgo function
//sets moveinfo to attackgo
void block_attackgo (edict_t *self)
{
	self->monsterinfo.currentmove = &block_move_attackgo;
}

//attack loop frames
//1 frame of eye open
//no movement
mframe_t block_frames_attackloop [] =
{
	ai_charge, 0, NULL
};
//moveinfo for attackloop
//1 frame, open3, to open3
//attackgo gets called at end of frames
mmove_t block_move_attackloop = {FRAME_open3, FRAME_open3, block_frames_attackloop, block_attackgo};

//attackloop function
//swing at anyone nearby while moving
//runs "count" counter
//if counter is 0 then stop moving and run stopattack movement
//otherwise continue attackloop, swinging melee every frame, and adding to velocity every frame
void block_attackloop (edict_t *self)
{
	self->count--; //decrement counter

	if (self->count == 0) //if counter is finished
	{
		VectorClear(self->velocity);//clear velocity to stop any movement
		self->monsterinfo.currentmove = &block_move_attackstop;//run stop animations
	}
	else //otherwise
	{
		self->monsterinfo.currentmove = &block_move_attackloop;//continue loop

		vec3_t	aim;
		vec3_t	right, up, forward;
		int		i;

		VectorSet (aim, 50, self->mins[0], 0);
		fire_hit (self, aim, 5, 300);

		VectorSet (up, 0, 0 ,1); //set up as true up

		//if spawnflags are set for x or z
		if (self->spawnflags & 32)
			VectorSet (right, 1, 0 ,0); //set right on x axis
		else if (self->spawnflags & 64)
			VectorSet (right, 0, 1 ,0); //set right on z axis
		else //otherwise use actual right
		{
			AngleVectors (self->s.angles, forward, right, NULL); //find right
			VectorNormalize(right); //normalize to account for distance
		}


		//continue most recent movement
		i = self->radius_dmg;

		if (i == 0)
			VectorScale(right, 500, self->velocity); //move right
		else if (i == 1)
			VectorScale(right, -500, self->velocity); //move left
		else if (i == 2)
			VectorScale(up, 150, self->velocity); //move up
		else
			VectorScale(up, -150, self->velocity); //move down
	}
}

//frames for start of attack loop
//3 frames to open eyes
//on last frame we will make hitable and give a random velocity
mframe_t block_frames_attackstart [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_charge, 0, block_attackstart
};
//moveinfo for start of attack loop
//3 frames, open1 to open3
//no function gets run at movement call
mmove_t block_move_attackstart = {FRAME_open1, FRAME_open3, block_frames_attackstart, NULL};


//attackstart function
//eyes are open so make damagable
//start counter for attack loop
//randomize a direction and start movement
//set movement as attackgo
void block_attackstart (edict_t *self)
{
	vec3_t	forward, right, up;

	int i;

	self->takedamage = true; //make hitable

	self->count = BLOCK_ATTACK_WAIT; //initialize attack loop counter

	VectorSet (up, 0, 0 ,1); //set up as true up

	//if spawnflags are set for x or z
	if (self->spawnflags & 32)
		VectorSet (right, 1, 0 ,0); //set up as true up
	else if (self->spawnflags & 64)
		VectorSet (right, 0, 1 ,0); //set up as true up
	else //otherwise use actual right
	{
		AngleVectors (self->s.angles, forward, right, NULL); //find right
		VectorNormalize(right); //normalize to account for distance
	}


	if (self->spawnflags & 8)//custom spawnflags for left/right (8)
	{
		//check previous move and do the opposite
		if (self->radius_dmg == 1)
			i = 0;
		else
			i = 1; //default radius_dmg is 0 so we will move left first anyway

		//store last move
		self->radius_dmg = i;
	}
	else if (self->spawnflags & 16)//custom spawnflags for updown(16)
	{
		//check previous move and do the opposite
		if (self->radius_dmg == 3)
			i = 2;
		else
			i = 3; //default radius_dmg is 0 so we will move down first anyway

		//store last move
		self->radius_dmg = i;
	}
	else
	{
		//set i to previous move to enter loop
		i = self->radius_dmg;

		//exit loop only when i is not the same as the previous move
		while (i == self->radius_dmg)
			i = (int)(random() * 4); //choose random direction

		//store lastmove
		self->radius_dmg = i;
	}


	//only store desired move. Do not actually move block until eye is open
/*	if (i == 0)
		VectorScale(right, 500, self->velocity); //move right
	else if (i == 1)
		VectorScale(right, -500, self->velocity); //move left
	else if (i == 2)
		VectorScale(up, 150, self->velocity); //move up
	else
		VectorScale(up, -150, self->velocity); //move down*/

	self->monsterinfo.currentmove = &block_move_attackgo;
}





//frames for stand, idle, sight, waitstart, and waitloop animation
//1 frame with eye closed
//no movement
mframe_t block_frames_stand [] =
{
	ai_stand, 0 ,NULL,
};

//moveinfo for start of waiting loop
//1 frame, stand1 to stand1
//run waitloop once this frame is complete
mmove_t block_move_waitstart = {FRAME_stand1, FRAME_stand1, block_frames_stand, block_waitloop};

//moveinfo for waiting loop
//1 frame, stand1 to stand1
//run waitstart once this frame is complete
mmove_t block_move_waitloop = {FRAME_stand1, FRAME_stand1, block_frames_stand, block_waitstart};


//waitstart function
//tells monster to run start of waiting loop move info
void block_waitstart (edict_t *self)
{
	self->takedamage = false;
	self->monsterinfo.currentmove = &block_move_waitstart;
}

//waitloop function
//runs "style" counter.
//If counter is 0 then attack
//if not run the waitloop frames
void block_waitloop (edict_t *self)
{
	self->style--; //decrement syle

	if (self->style == 0) //if time has run out
	{
		self->style = BLOCK_WAIT; //pre-emptively reset counter in case attack frame breaks
		self->monsterinfo.currentmove = &block_move_attackstart; //give attack start move info
	}
	else //if counter is still going
	{
		self->monsterinfo.currentmove = &block_move_waitloop; //continue waitloop
	}
}


//frames for pain animation
//3 frames of eye flinching
//no movement
mframe_t block_frames_pain [] =
{
	ai_stand, 0 ,NULL,
	ai_stand, 0 ,NULL,
	ai_stand, 0 ,NULL
};
//moveinfo for pain animation
//3 frames, pain1 to pain3
//starts wait loop at end of animation
mmove_t block_move_pain = {FRAME_pain1, FRAME_pain3, block_frames_pain, block_waitstart};

//pain function
//reset wait counter
//make non damaged
//make moveinfo to pain frames
void block_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	self->style = BLOCK_WAIT; //reset wait counter
	self->takedamage = false; //make not hitable
	self->monsterinfo.currentmove = &block_move_pain;
}





//moveinfo for stand animation
//1 frame, stand1 to stand1
//points to idle frame
mmove_t block_move_stand = {FRAME_stand1, FRAME_stand1, block_frames_stand, block_idle};

//stand function
//change moveinfo to stand frames
void block_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &block_move_stand;
}

//moveinfo for idle animation
//1 frame, stand1 to stand1
//points to stand frame
mmove_t block_move_idle = {FRAME_stand1, FRAME_stand1, block_frames_stand, block_stand};

//stand function
//change moveinfo to stand frames
void block_idle (edict_t *self)
{
	self->monsterinfo.currentmove = &block_move_idle;
}



//moveinfo for sight animation
//1 frame, stand1 to stand1
//starts wait loop at end of animation
mmove_t block_move_sight = {FRAME_stand1, FRAME_stand1, block_frames_stand, block_waitstart};

//sight function
//change moveinfo to sight frames
void block_sight (edict_t *self, edict_t *other)
{
	self->monsterinfo.currentmove = &block_move_sight;
}




//die function
//become explosion and remove
void block_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	BecomeExplosion1(self);
}

void SP_monster_block (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->s.modelindex = gi.modelindex("models/monsters/block/tris.md2");
	VectorSet (self->mins, -16, -16, -12);
	VectorSet (self->maxs, 16, 16, 12);

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	self->health = 80;
	self->mass = 200;

	//cannot be damaged with eyes closed
	self->takedamage = false;

	//start wait counter
	self->style = BLOCK_WAIT;

	//clear attack counter
	self->count = 0;

	//clear last move
	self->radius_dmg = 0;

	self->pain = block_pain;
	self->die = block_die;

	self->monsterinfo.stand = block_stand;
	self->monsterinfo.attack = block_waitstart;
	self->monsterinfo.sight = block_sight;
	self->monsterinfo.idle = block_stand;
	self->monsterinfo.walk = block_waitstart;
	self->monsterinfo.run = block_waitstart;
	self->monsterinfo.melee = block_waitstart;

	self->monsterinfo.currentmove = &block_move_waitstart;
	//self->monsterinfo.scale = MODEL_SCALE;

	gi.linkentity (self);

	flymonster_start (self);
}
