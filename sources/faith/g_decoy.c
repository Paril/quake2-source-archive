/*==============================================================================
The Weapons Factory - 
Decoy Mod
Original code by Gregg Reno
==============================================================================*/

#include "g_local.h"
#include "m_player.h"

static int	sound_idle;
static int	sound_sight1;
static int	sound_sight2;
static int	sound_pain;
static int	sound_death;


// STAND frames
void decoy_idle (edict_t *self)
{
	if (random() > 0.8)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void decoy_stand (edict_t *self);

mframe_t decoy_frames_stand1 [] =
{
    ai_stand, 0, decoy_idle,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

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
mmove_t decoy_move_stand1 = {FRAME_stand01, FRAME_stand40, decoy_frames_stand1, decoy_stand};


void decoy_stand (edict_t *self)
	{
    self->monsterinfo.currentmove = &decoy_move_stand1;
	}


// TAUNT frames

void decoy_taunt (edict_t *self);

mframe_t decoy_frames_taunt1 [] =
{
    ai_stand, 0, decoy_idle,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
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
mmove_t decoy_move_taunt1 = {FRAME_taunt01, FRAME_taunt17, decoy_frames_taunt1, decoy_taunt};

void decoy_taunt (edict_t *self)
	{
    self->monsterinfo.currentmove = &decoy_move_taunt1;
	}


//
// RUN frames
//
void decoy_run (edict_t *self);
mframe_t decoy_frames_run [] =
{
	ai_run, 10, NULL,
	ai_run, 11, NULL,
	ai_run, 11, NULL,
	ai_run, 16, NULL,
	ai_run, 10, NULL,
	ai_run, 15, NULL
};
mmove_t decoy_move_run = {FRAME_run1, FRAME_run6, decoy_frames_run, decoy_run};
void decoy_run (edict_t *self)
	{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		{
        self->monsterinfo.currentmove = &decoy_move_stand1;
		return;
		}

    self->monsterinfo.currentmove = &decoy_move_run;
	}


//
// PAIN frames
//
mframe_t decoy_frames_pain1 [] =
{
	ai_move, -3, NULL,
	ai_move, 4,  NULL,
	ai_move, 1,  NULL,
	ai_move, 0,  NULL
};
mmove_t decoy_move_pain1 = {FRAME_pain101, FRAME_pain104, decoy_frames_pain1, decoy_run};

void decoy_pain (edict_t *self, edict_t *other, float kick, int damage)
	{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
    self->monsterinfo.currentmove = &decoy_move_pain1;
	}


//
// ATTACK frames
//
static int blaster_flash [] = {MZ2_SOLDIER_BLASTER_1, MZ2_SOLDIER_BLASTER_2, MZ2_SOLDIER_BLASTER_3, MZ2_SOLDIER_BLASTER_4, MZ2_SOLDIER_BLASTER_5, MZ2_SOLDIER_BLASTER_6, MZ2_SOLDIER_BLASTER_7, MZ2_SOLDIER_BLASTER_8};
static int shotgun_flash [] = {MZ2_SOLDIER_SHOTGUN_1, MZ2_SOLDIER_SHOTGUN_2, MZ2_SOLDIER_SHOTGUN_3, MZ2_SOLDIER_SHOTGUN_4, MZ2_SOLDIER_SHOTGUN_5, MZ2_SOLDIER_SHOTGUN_6, MZ2_SOLDIER_SHOTGUN_7, MZ2_SOLDIER_SHOTGUN_8};
static int machinegun_flash [] = {MZ2_SOLDIER_MACHINEGUN_1, MZ2_SOLDIER_MACHINEGUN_2, MZ2_SOLDIER_MACHINEGUN_3, MZ2_SOLDIER_MACHINEGUN_4, MZ2_SOLDIER_MACHINEGUN_5, MZ2_SOLDIER_MACHINEGUN_6, MZ2_SOLDIER_MACHINEGUN_7, MZ2_SOLDIER_MACHINEGUN_8};


void decoy_fire (edict_t *self, int flash_number)
	{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	float	r, u;
	int		flash_index;

	flash_index = shotgun_flash[flash_number];

	AngleVectors (self->s.angles, forward, right, NULL);
//	G_ProjectSource (self->s.origin, monster_flash_offset[flash_index], forward, right, start);
	VectorCopy (self->s.origin, start);
	if (flash_number == 5 || flash_number == 6)
	{
		VectorCopy (forward, aim);
	}
	else
		{
		VectorCopy (self->enemy->s.origin, end);
		end[2] += self->enemy->viewheight;
		VectorSubtract (end, start, aim);
		vectoangles (aim, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*100;
		u = crandom()*100;
		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		VectorSubtract (end, start, aim);
		VectorNormalize (aim);
		}
	if (Q_stricmp(self->classname,"soul") == 0)
		monster_fire_rocket (self, start, aim, 20, 650, 57);
	else
		monster_fire_shotgun (self, start, aim, 5, 1, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SHOTGUN_COUNT, flash_index);
	}

// Fire weapon
void decoy_fire1 (edict_t *self)
{
        decoy_fire (self, 0);
}

mframe_t decoy_frames_attack1 [] =
{
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
    ai_charge, 0,  decoy_fire1,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
    ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL
};
mmove_t decoy_move_attack1 = {FRAME_attack1, FRAME_attack8, decoy_frames_attack1, decoy_run};

void decoy_attack(edict_t *self)
	{
    self->monsterinfo.currentmove = &decoy_move_attack1;
	}

//
// SIGHT frames
//

void decoy_sight(edict_t *self, edict_t *other)
	{
	if (random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_sight1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_sight2, 1, ATTN_NORM, 0);
	}

//
// DEATH sequence
//

void decoy_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
	{
	//	int		n;

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);

	//do a BFG kind of explosion where the decoy was
	gi.WriteByte (svc_temp_entity);
	
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	//Clear pointer of owner
	self->creator->decoy = NULL;

	//Remove entity instead of playing death sequence
	G_FreeEdict (self);

	}


//
// SPAWN
//
void spawn_decoy (edict_t *owner, int Undead)
{
    edict_t *self;
	vec3_t forward;

	self = G_Spawn();

	// Place decoy 100 units forward of our position
    AngleVectors(owner->client->v_angle, forward, NULL, NULL);
    VectorMA(owner->s.origin, 100, forward, self->s.origin);
 
	//Link two entities together
	owner->decoy = self;	//for the owner, this is a pointer to the decoy
	self->creator = owner;	//for the decoy, this is a pointer to the owner
	if (Undead == 0){
		//Use same model and skin as the person creating decoy
		self->model = owner->model;
		self->s.skinnum = owner->s.skinnum;
		self->s.modelindex = owner->s.modelindex;
		self->s.modelindex2 = owner->s.modelindex2;
	}
	else {
		gi.setmodel (self, "models/monsters/tris.md2");	
		self->s.modelindex2 = gi.modelindex("models/monsters/w_rlauncher.md2");
	}
	if (Undead == 0)
		self->s.effects = 0;
	else
		self->s.effects = EF_FLIES;
	self->s.frame = 0;
	if (Undead == 0)
		self->classname = "decoy";
	else	
		self->classname = "soul";
	if (Undead == 0){
		self->health = 50;
		self->max_health = 50;
	}
	else {
		self->health = 80;
		self->max_health = 80;
	}
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->clipmask = MASK_PLAYERSOLID;
	self->takedamage = DAMAGE_AIM;

	self->mass = 100;
    self->pain = decoy_pain;
    self->die = decoy_die;
    self->monsterinfo.stand = decoy_stand;
    self->monsterinfo.walk = NULL;
    self->monsterinfo.run = decoy_run;
    self->monsterinfo.dodge = NULL;
    self->monsterinfo.attack = decoy_attack;
	self->monsterinfo.melee = NULL;
    self->monsterinfo.sight = decoy_sight;

	//Dont attack anything to start with
	self->monsterinfo.aiflags & AI_GOOD_GUY;

	//Set up sounds
	if (Undead == 0){
		sound_idle =    gi.soundindex ("soldier/solidle1.wav");
    		sound_sight1 =  gi.soundindex ("soldier/solsght1.wav");
    		sound_sight2 =  gi.soundindex ("soldier/solsrch1.wav");
    		sound_pain = gi.soundindex ("soldier/solpain1.wav");
    		sound_death = gi.soundindex ("misc/keyuse.wav");
	}	
	else {
		sound_idle =    gi.soundindex ("monsters/idle.wav");
		sound_sight1 =  gi.soundindex ("monsters/sight1.wav");
		sound_sight2 =  gi.soundindex ("monsters/sight2.wav");
		sound_pain = gi.soundindex ("monsters/pain.wav");
		sound_death = gi.soundindex ("monsters/death.wav");
	}

	self->gib_health = -50;

	// Face the decoy the same direction as player
	self->s.angles[PITCH] = owner->s.angles[PITCH];
	self->s.angles[YAW] = owner->s.angles[YAW];
	self->s.angles[ROLL] = owner->s.angles[ROLL];

	gi.linkentity (self);

	// First animation sequence
	self->monsterinfo.stand (self);

	//Let monster code control this decoy
	walkmonster_start (self);
}


// SP_Decoy - Handle DECOY command
void SP_Decoy(edict_t *self, int Undead) {

	//See if we should decoy turn it on or off
	char    *string;
	int  turnon;

	string=gi.args();

	if (Q_stricmp ( string, "on") == 0) 
		turnon = true;
	else if (Q_stricmp ( string, "off") == 0) 
		turnon = false;
	else {  //toggle status
		if (self->decoy) turnon = false;
		else turnon = true;
	}


	//If they want to turn it on and it's already on, return
	if ( (turnon == true) && (self->decoy) ) return;

	//If they want to turn it off and it's already off, return
	if ( (turnon == false) && !(self->decoy) ) return;

	//Remove decoy if it exists
	if ( self->decoy ) {
		G_FreeEdict(self->decoy);
		self->decoy = NULL;
		if (Undead == 0)
			gi.cprintf (self, PRINT_HIGH, "Illusion destroyed.\n");
		else
			gi.cprintf (self, PRINT_HIGH, "Soul destroyed. \n");
		
	}
	//Create decoy
	spawn_decoy(self, Undead);
	if (Undead == 0)
		gi.cprintf (self, PRINT_HIGH, "Illusion created.\n");
}
