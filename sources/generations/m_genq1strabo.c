/*
==============================================================================

STRABO !

==============================================================================
*/

#include "g_local.h"
#include "m_genq1strabo.h"


static int	sound_sight;
static int	sound_idle;
static int  sound_attack;
static int	sound_die;
static int	sound_pain;
static int	sound_hit;

// lava balls
void q1_fire_lavaball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);

// enemy position
qboolean visible (edict_t *self, edict_t *other);
qboolean infront (edict_t *self, edict_t *other);
void monster_start_go (edict_t *self);
qboolean monster_start (edict_t *self);
void monster_triggered_start (edict_t *self);



void strabo_attack (edict_t *self);
void strabo_check (edict_t *self);
void strabo_fly (edict_t *self);

void strabo_sight (edict_t *self, edict_t *other)
{
	gi.dprintf("Sighted\n");
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NONE, 0);
	strabo_attack (self);
}

void strabo_idle (edict_t *self)
{
	if(random() < 0.01)
		gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_NORM, 0);
}


//================
// TURN
//================

mframe_t strabo_frames_flyturn [] =
{
	ai_charge, 30, strabo_idle,
	ai_charge, 30, NULL,
	ai_charge, 30, NULL,
	ai_charge, 30, NULL,
	ai_charge, 30, NULL,
	ai_charge, 30, NULL,
	ai_charge, 30, NULL,
	ai_charge, 30, strabo_idle,
	ai_charge, 30, NULL,
	ai_charge, 30, NULL,
	ai_charge, 30, NULL, 
	ai_charge, 30, NULL,
	ai_charge, 30, NULL,
	ai_charge, 30, strabo_check
};
mmove_t	strabo_move_flyturn = {FRAME_fly1, FRAME_fly14, strabo_frames_flyturn, strabo_fly};

void strabo_flyturn (edict_t *self)
{
	self->monsterinfo.currentmove = &strabo_move_flyturn;
}

//================
// FLY-TURN
//================


mframe_t strabo_frames_turn [] =
{
	ai_charge, 0, strabo_idle,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, strabo_idle,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL, 
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, strabo_check
};
mmove_t	strabo_move_turn = {FRAME_fly1, FRAME_fly14, strabo_frames_turn, strabo_fly};

void strabo_turn (edict_t *self)
{
	self->monsterinfo.currentmove = &strabo_move_turn;
}


//================
// HOVER
//================
mframe_t strabo_frames_hover [] =
{
	ai_stand, 0, strabo_idle,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, strabo_idle,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL, 
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, strabo_check
};
mmove_t	strabo_move_hover = {FRAME_fly1, FRAME_fly14, strabo_frames_hover, NULL};


void strabo_hover (edict_t *self)
{
	self->monsterinfo.currentmove = &strabo_move_hover;
}

//================
// FLY
//================
mframe_t strabo_frames_fly [] =
{
	ai_run, 50, strabo_idle,
	ai_run, 50, NULL,
	ai_run, 50, NULL,
	ai_run, 50, NULL,
	ai_run, 50, NULL,
	ai_run, 50, NULL,
	ai_run, 50, strabo_check,
	ai_run, 50, strabo_idle,
	ai_run, 50, NULL,
	ai_run, 50, NULL,
	ai_run, 50, NULL, 
	ai_run, 50, NULL,
	ai_run, 50, NULL,
	ai_run, 50, strabo_check
};
mmove_t	strabo_move_fly = {FRAME_fly1, FRAME_fly14, strabo_frames_fly, NULL};


void strabo_fly (edict_t *self)
{
	self->monsterinfo.currentmove = &strabo_move_fly;
}


//================
// LAME AI ROUTINE
//================
void strabo_check (edict_t *self)
{
	vec3_t point;
	int cont;

	point[0] = self->velocity[0] * 2;
	point[1] = self->velocity[1] * 2;
	point[2] = self->s.origin[2];	
	cont = gi.pointcontents (point);

	if (cont & MASK_SOLID) 
	{
		gi.dprintf("Wall in front\n");
		if(infront (self, self->enemy))
		{
			gi.dprintf("Attack the mofo\n");
			strabo_attack(self);
		}
		else if (visible (self, self->enemy))
		{
			gi.dprintf("Flying turn\n");
			strabo_flyturn(self);
		}
		else
		{
			gi.dprintf("Still Turn\n");
			strabo_turn(self);
		}
	}
	else
		strabo_fly(self);
}


//================
// PAIN
//================
mframe_t strabo_frames_pain [] =
{	
		ai_move,-20,NULL,
		ai_move, 10, NULL,
		ai_move, 20,NULL
};
mmove_t strabo_move_pain = {FRAME_pain1, FRAME_pain3, strabo_frames_pain, strabo_fly};


void strabo_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 4;
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	self->monsterinfo.currentmove = &strabo_move_pain;
}


//================
// DEAD
//================
void strabo_dead(edict_t * self)
{
	VectorSet (self->mins, -144, -136, -36);
	VectorSet (self->maxs, 88, 128, 24);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t strabo_frames_die [] =
{	
		ai_move, -30, NULL,
		ai_move, -15, NULL,
		ai_move, 0, NULL
};
mmove_t strabo_move_die = {FRAME_pain1, FRAME_pain3, strabo_frames_die, strabo_dead};


void strabo_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	
	int		n;
// check for gib
// FIX - Skid
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("oldudes/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 8; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 8; n++)
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NONE, 0);
	self->monsterinfo.currentmove = &strabo_move_die;
//	scrag_dead(self);
}


//================
// ATTACK
//================

void strabo_fireball (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	//vec3_t  offset;
	int dist;

	dist = range (self, self->enemy);

	if(!infront(self, self->enemy) || dist <= RANGE_MID)
	{
		strabo_flyturn(self);
		return;
	}

	gi.sound (self, CHAN_VOICE, sound_attack, 1, ATTN_NORM, 0);
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_BOSS2_ROCKET_1], forward, right, start);
	VectorCopy (self->enemy->s.origin, vec);

	switch(dist)
	{
		/*case RANGE_MELEE:
		//{
			//vec[2] -= (self->enemy->viewheight*(5 * random()));
			//vec[0] += self->enemy->velocity[0] * 0.25;  
			//vec[1] += self->enemy->velocity[1] * 0.25;
		//	break;
		//}
		case RANGE_NEAR:
		{
			vec[2] -= (self->enemy->viewheight*(4 * random()));
			vec[0] += self->enemy->velocity[0] * 0.33;  
			vec[1] += self->enemy->velocity[1] * 0.33; 
			break;
		}*/
		case RANGE_MID:
		{
			vec[2] -= (self->enemy->viewheight*(4 * random()));
			vec[0] += self->enemy->velocity[0] * 0.5;  
			vec[1] += self->enemy->velocity[1] * 0.5; 
			break;
		}
		case RANGE_FAR:
		default:
		{
			vec[2] -= (self->enemy->viewheight*(3 * random()));
			vec[0] += self->enemy->velocity[0];  
			vec[1] += self->enemy->velocity[1];
			break;
		}
	}

	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_BOSS2_ROCKET_1);
	gi.multicast (start, MULTICAST_PVS);
	q1_fire_lavaball (self,start, dir, 80, 700, 120, 80);
}


mframe_t strabo_frames_attack1 [] =
{
		ai_charge, 45, NULL,
		ai_charge, 30, NULL,
		ai_charge, 15, NULL,					
		ai_charge,  0, NULL,				
		ai_charge,  0, NULL,			
		ai_charge,  0,strabo_fireball,				
		ai_charge,  0,strabo_fireball,					
		ai_charge, 15,strabo_check
};

mmove_t strabo_move_attack1 = {FRAME_attack1, FRAME_attack8, strabo_frames_attack1, strabo_fly};

void strabo_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &strabo_move_attack1;
}

//===========================
//===========================
// STRABO 
//===========================
//===========================

//qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink);


void strabo_check_height( edict_t *self)
{
	vec3_t point,forward,dest;
	trace_t tr;
	
	int cont;
	qboolean moved=false;
	int i;

	point[0] = self->velocity[0] *2;
	point[1] = self->velocity[1] *2;
	point[2] = self->s.origin[2] - 120.0;	
	cont = gi.pointcontents (point);


	if (cont & MASK_SOLID) 
	{
		gi.dprintf("Adding Height\n");
		self->s.origin[2] += 30.0;
	}

	if(self->enemy && (range (self, self->enemy) < RANGE_MID))
	{
		VectorClear(point);
		
		for(i=0;i<=8;i++)
		{
			gi.dprintf("FINDING NEW CO-ORDINATES ??\n");
			
			AngleVectors (self->s.angles, forward, NULL, NULL);
			forward[0] = forward[0] + i*45 ;

			VectorMA(self->s.origin, 1024, forward, point);
			tr = gi.trace(self->s.origin, self->mins, self->maxs, point, self, MASK_MONSTERSOLID);			
			
			if (tr.fraction > 0)
			{	
				dest[0] = tr.endpos[0];
				dest[1] = tr.endpos[1];
				dest[2] = tr.endpos[2];
				
				cont=gi.pointcontents(dest);
				
				if(cont & MASK_MONSTERSOLID)
				{
					self->ideal_yaw = dest[0];
					gi.dprintf("NEW YAW\n");
					moved=true;
					break;
				}
			}
		}			//moved= SV_movestep (self, point, true);

		gi.dprintf("IT MOVED AWAY ??\n");
		if(moved)
			self->monsterinfo.currentmove = &strabo_move_fly;
	}
}


//QUAKED monster_strabo (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight

void SP_q1_monster_strabo (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_sight = gi.soundindex ("q1monstr/strabo/sight.wav");
	sound_idle = gi.soundindex ("q1monstr/strabo/idlefly.wav");
	sound_pain = gi.soundindex ("q1monstr/strabo/pain.wav");
	sound_die = gi.soundindex ("q1monstr/strabo/death.wav");
	sound_attack = gi.soundindex ("q1monstr/strabo/wattack.wav");
	sound_hit = gi.soundindex ("q1monstr/strabo/hit.wav");
	
	self->s.modelindex = gi.modelindex ("models/monsters/q1monstr/strabo/tris.md2");
	
	VectorSet (self->mins, -144, -128, -36);
	VectorSet (self->maxs, 88, 128, 24);
	
	self->movetype = MOVETYPE_FLY;
	self->solid = SOLID_BBOX;

	self->health = 4000;
	self->gib_health = 100;
	self->mass = 50;

	self->pain = strabo_pain;
	self->die = strabo_die;
	self->prethink = strabo_check_height;
	self->style =69;

	self->yaw_speed = 20;
	
	self->monsterinfo.stand = strabo_hover;
	self->monsterinfo.walk = strabo_fly;
	self->monsterinfo.run = strabo_fly;
	self->monsterinfo.attack = strabo_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = strabo_sight;
	self->monsterinfo.idle = strabo_hover;

	//self->flags |=FL_FLY;
	self->s.renderfx |= RF_FULLBRIGHT;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &strabo_move_hover;	
	self->monsterinfo.scale = MODEL_SCALE;
	

	flymonster_start (self);
}
