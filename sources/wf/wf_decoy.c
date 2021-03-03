/*==============================================================================
The Weapons Factory - 
Decoy Mod
Original code by Gregg Reno
==============================================================================*/

/* NOTE - Now that eraser bots are in the code, decoys do not work as they used to.
They will no longer be active players, but rather just animated models.
They will not pursue or shoot.
*/

#include "g_local.h"
#include "m_player.h"
#include "wf_classmgr.h"

qboolean IsCyborg (edict_t *ent);

static int	sound_idle;
static int	sound_sight1;
static int	sound_sight2;
static int	sound_pain;
static int	sound_death;
static int	sound_on;

edict_t	*SV_TestEntityPosition (edict_t *ent);

/*
=============
DecoyFit - see if the decoy will fit where the player is
creating it.
 
   -taken from m_move.c, M_CheckBottom function
=============
*/
qboolean DecoyFit (edict_t *ent, edict_t *owner)
{
//	vec3_t	mins, maxs, start, stop;
	vec3_t	mins, maxs;
	trace_t	trace;
	
	VectorAdd (ent->s.origin, ent->mins, mins);
	VectorAdd (ent->s.origin, ent->maxs, maxs);

	//See if the mins,maxs or origin are in something solid
	if (gi.pointcontents (mins) == CONTENTS_SOLID) return false;	
	if (gi.pointcontents (maxs) == CONTENTS_SOLID) return false;	
        if (gi.pointcontents (ent->s.origin) == CONTENTS_SOLID) return false; 

	//Trace line from these two points to see if anything is in-between
	trace = gi.trace(mins, ent->s.origin, ent->s.origin, maxs, ent, MASK_MONSTERSOLID );
	if (trace.fraction != 1.0)	// 1.0 = nothing in between
		return false;

	//Trace line from decoy origin to owner origin to see if anything is in-between
	trace = gi.trace(ent->s.origin, ent->s.origin, ent->s.origin, owner->s.origin, ent, MASK_MONSTERSOLID );
	if (trace.fraction != 1.0)	// 1.0 = nothing in between
		return false;

	if (SV_TestEntityPosition (ent))
		return false;

	return true;
}

#define DECOY_FIRST_FRAME	0
#define DECOY_LAST_FRAME	39

void Decoy_Think(edict_t *self)
{
	++self->s.frame;

	if (self->s.frame < DECOY_FIRST_FRAME)
		self->s.frame = DECOY_FIRST_FRAME;
	else if (self->s.frame > DECOY_LAST_FRAME)
		self->s.frame = DECOY_FIRST_FRAME;

	self->nextthink = level.time + 0.1;
}



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

// Flipoff frames
void decoy_flip (edict_t *self);
mframe_t decoy_frames_flip1 [] =
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
	ai_stand, 0, NULL
};
mmove_t decoy_move_flip1 = {FRAME_flip01, FRAME_flip12, decoy_frames_flip1, decoy_flip};
void decoy_flip (edict_t *self)
	{
    self->monsterinfo.currentmove = &decoy_move_flip1;
	}

// Salute frames
void decoy_salute (edict_t *self);
mframe_t decoy_frames_salute1 [] =
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

	ai_stand, 0, NULL
};
mmove_t decoy_move_salute1 = {FRAME_salute01, FRAME_salute11, decoy_frames_salute1, decoy_salute};
void decoy_salute (edict_t *self)
	{
    self->monsterinfo.currentmove = &decoy_move_salute1;
	}


// Wave frames
void decoy_wave (edict_t *self);
mframe_t decoy_frames_wave1 [] =
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

	ai_stand, 0, NULL
};
mmove_t decoy_move_wave1 = {FRAME_wave01, FRAME_wave11, decoy_frames_wave1, decoy_wave};
void decoy_wave (edict_t *self)
	{
    self->monsterinfo.currentmove = &decoy_move_wave1;
	}

// Point frames
void decoy_point (edict_t *self);
mframe_t decoy_frames_point1 [] =
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
	ai_stand, 0, NULL
};
mmove_t decoy_move_point1 = {FRAME_point01, FRAME_point12, decoy_frames_point1, decoy_point};
void decoy_point (edict_t *self)
	{
    self->monsterinfo.currentmove = &decoy_move_point1;
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
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_index], forward, right, start);

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

		r = crandom()*1000;
		u = crandom()*500;
		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		VectorSubtract (end, start, aim);
		VectorNormalize (aim);
		}

	monster_fire_shotgun (self, start, aim, 2, 1, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SHOTGUN_COUNT, flash_index, MOD_SHOTGUN);
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
	gi.WriteByte (TE_BFG_EXPLOSION);
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
qboolean spawn_decoy (edict_t *owner)
{
    edict_t *self;
	vec3_t forward;
	char modelname[64];
	char weaponname[64];
	int classnum;

	self = G_Spawn();

	// Place decoy 100 units forward of our position
    AngleVectors(owner->client->v_angle, forward, NULL, NULL);
    VectorMA(owner->s.origin, 100, forward, self->s.origin);

	//set the team
	self->wf_team = owner->client->resp.ctf_team;

	//Use same model and skin as the person creating decoy
	self->model = owner->model;

	classnum = owner->client->pers.player_class;
	sprintf(modelname, "wfactory/models/decoys/%s/tris.md2", classinfo[classnum].model_name);
	sprintf(weaponname, "players/%s/weapon.md2",classinfo[classnum].model_name);
	self->s.modelindex = gi.modelindex (modelname);
	self->s.modelindex2 = gi.modelindex (weaponname);
	self->s.skinnum = classinfo[classnum].decoyskin;

	self->s.frame = DECOY_FIRST_FRAME;
	self->think = Decoy_Think;
	self->nextthink = level.time + 0.1;

	//Set the skin color

	//red team skin number is 1 higher than blue
	if (self->wf_team == CTF_TEAM1)	//team 1 is red
	{
		++self->s.skinnum;
	}

	//test stuff for other decoy settings
/*
	if ((int)wfflags->value & WF_TMP3) 
	{
		self->s.skinnum = 0;		//team 2 is blue
		self->s.modelindex = gi.modelindex ("wfactory/models/decoys/crakhor/tris.md2");
		self->s.modelindex2 = gi.modelindex ("players/crakhor/weapon.md2");
	}
*/
	self->s.effects = 0;
	self->s.frame = 0;
	self->classname = "decoy";
	
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
	//self->monsterinfo.aiflags & AI_GOOD_GUY;

	//Stay in one spot or pursue depending on wfflag setting
	if ((int)wfflags->value & WF_DECOY_PURSUE)
		self->monsterinfo.aiflags = 0;
	else
		self->monsterinfo.aiflags &= AI_STAND_GROUND;

    self->monsterinfo.sight = decoy_sight;
	//newdecoy for proxies etc if think ever gets fixed remove this
	self->svflags |= SVF_MONSTER;
	self->s.renderfx |= RF_FRAMELERP;
	self->max_health = self->health;

	//Set up sounds
	sound_idle =    gi.soundindex ("soldier/solidle1.wav");
    sound_sight1 =  gi.soundindex ("soldier/solsght1.wav");
    sound_sight2 =  gi.soundindex ("soldier/solsrch1.wav");
    sound_pain = gi.soundindex ("soldier/solpain1.wav");

	//Set up on/off sounds
	if (IsFemale (owner))
	{
	    sound_death = gi.soundindex ("fdecoyof.wav");
	    sound_on = gi.soundindex ("fdecoyon.wav");
	}
        else if (IsCyborg (owner))
	{
            sound_death = gi.soundindex ("cdecoyof.wav");
            sound_on = gi.soundindex ("cdecoyon.wav");
	}
	else
	{
	    sound_death = gi.soundindex ("mdecoyof.wav");
	    sound_on = gi.soundindex ("mdecoyon.wav");
	}

	//Temporary - replease sounds with ones from Q2
    //sound_death = gi.soundindex ("misc/keyuse.wav");
    //sound_on =    gi.soundindex ("soldier/solatck1.wav");

	self->health = 30;
	self->max_health = 30;
	self->gib_health = -30;

	if (wfdebug)
	{
		//self->health = 200;
		//self->max_health = 200;
	}

	// Face the decoy the same direction as player
	self->s.angles[PITCH] = owner->s.angles[PITCH];
	self->s.angles[YAW] = owner->s.angles[YAW];
	self->s.angles[ROLL] = owner->s.angles[ROLL];

	// See if the decoy will fit
	if (DecoyFit(self, owner) == false)
	{
		G_FreeEdict(self);
		safe_cprintf (owner, PRINT_HIGH, "Decoy won't fit.  Try aiming a little higher.\n");
		return false;
	}
 
	//Link two entities together
	owner->decoy = self;	//for the owner, this is a pointer to the decoy
	self->creator = owner;	//for the decoy, this is a pointer to the owner

	gi.linkentity (self);

	// First animation sequence
	self->monsterinfo.stand (self);

	//Let monster code control this decoy
	//REMOVED FOR ERASER
	//walkmonster_start (self);

	//Play the startup sound
	gi.sound (self, CHAN_VOICE, sound_on, 1, ATTN_NORM, 0);

	//Reduce cell count
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		owner->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= DECOY_CELLS;

	return true;
}


// SP_Decoy - Handle DECOY command
void SP_Decoy(edict_t *self) {

	//See if we should decoy turn it on or off
	char    *string;
	int  turnon;

	if ((int)wfflags->value & WF_NO_DECOYS)
	{
		safe_cprintf (self, PRINT_HIGH, "Sorry, Decoys are DISABLED on this server.\n");
		return;
	}

	string=gi.args();

	if (Q_stricmp ( string, "on") == 0) 
		turnon = true;
	else if (Q_stricmp ( string, "off") == 0) 
		turnon = false;
	else {  //toggle status
		if (self->decoy) turnon = false;
		else turnon = true;
	}

//gi.dprintf("Decoy on = %d, decoy ent = %d\n", turnon, self->decoy);

	//If they want to turn it on and it's already on, return
	if ( (turnon == true) && (self->decoy) ) return;

	//If they want to turn it off and it's already off, return
	if ( (turnon == false) && !(self->decoy) ) return;

	//Remove decoy if it exists
	if ( self->decoy ) {
		gi.sound (self->decoy, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
		G_FreeEdict(self->decoy);
		self->decoy = NULL;
		safe_cprintf (self, PRINT_HIGH, "Decoy destroyed.\n");
		return;
		}

	//Cant create decoy in observer/spectator mode
	if (self->solid == SOLID_NOT)
	{
		safe_cprintf(self, PRINT_HIGH, "Can't create decoy in spectator mode (nice try!).\n");
		return;
	}

	//Create decoy if you have enough cells
	if (self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < DECOY_CELLS)
	{
		safe_cprintf(self, PRINT_HIGH, "Not enough cells for decoy.\n");
		return;
	}
	if (spawn_decoy(self))
		safe_cprintf (self, PRINT_HIGH, "Decoy created.\n");
	}
