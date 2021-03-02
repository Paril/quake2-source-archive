#include "g_local.h"
#include "m_player.h"

void MatrixJump (edict_t *ent)
{
	trace_t	tr;
	vec3_t	forward, pt;
	int		i;
	
	for (i=0;i < 4;i++)
	{
		vec3_t angles= {0,i*90,0};
		AngleVectors (angles, forward, NULL, NULL);
		VectorMA(ent->s.origin, 32, forward, pt);
		tr = gi.trace(ent->s.origin, vec3_origin, vec3_origin, pt, ent, MASK_SOLID);
		if (tr.fraction < 1)
		{
			vec3_t	kick;


		//VectorSubtract(ent->s.origin, tr.endpos, kick);
			//VectorNormalize(kick);
			//VectorScale(kick, 300, kick);
			VectorScale(tr.plane.normal, 300, kick);
			VectorCopy(kick, ent->velocity);
			ent->stamina -= 5;
			ent->velocity[2] = 300;
			ent->groundentity = NULL;
			ent->last_jump_time = level.time + JUMP_TIME;
			ent->matrixflip = BFLIP;
		}
	}
}

void MatrixFlip (edict_t *ent)
{
	float	forwardspeed;
	vec3_t	forward, right, up;
	int		i;
	
	AngleVectors (ent->client->v_angle, forward, right, up);
	forwardspeed = DotProduct(ent->velocity, forward);
	// Projection of the velocity on the forward vector it doesnt seem to work perfectly but good enough


	if (ent->deadflag)
		return;

 	//select roll
	if ((ent->matrixflip == NOFLIP) && (ent->jumping) && !(ent->waterlevel > 0))
	{// Flip if your jumping and not just holding jump
		if (ent->strafebuttons > 0)
			ent->matrixflip = LFLIP;

		if (ent->strafebuttons < 0)
			ent->matrixflip = RFLIP;

		if (forwardspeed < -100)
			// Flip if your going backwards and not just holding back
			ent->matrixflip = BFLIP;
	}

	if (ent->matrixflip == BFLIP)
	{
		ent->s.angles[PITCH] -= (sv_gravity->value*0.0625);
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame =  FRAME_crwalk1;
	}

	if (ent->matrixflip == FALLROLL)
		ent->s.angles[PITCH] += (sv_gravity->value*0.1);

//you will always do a full loop from jump to land (as long as the land is flat)
	//no matter what the gravity.

	//left flip
	if (ent->matrixflip == RFLIP)
	{
	//	
		ent->s.angles[ROLL] += (sv_gravity->value*0.0625);
		

		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame =  FRAME_crwalk1;

		if(ent->s.angles[ROLL] == 0)
			ent->matrixflip = NOFLIP;

		if(ent->s.angles[ROLL] < -360)
			ent->matrixflip = NOFLIP;
	}
	
	//rightflip switched for software
	if (ent->matrixflip == LFLIP)
	{
		ent->s.angles[ROLL] -= (sv_gravity->value*0.0625);
		
		
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame =  FRAME_crwalk1;

		if(ent->s.angles[ROLL] == 0)
			ent->matrixflip = NOFLIP;
		
		if(ent->s.angles[ROLL] > 360)
			ent->matrixflip = NOFLIP;
	}
	if ((ent->matrixflip == RUN_LEFT_ALONG_WALL 
		|| ent->matrixflip == RUN_RIGHT_ALONG_WALL) && !ent->velocity)
		ent->matrixflip = NOFLIP;
	//run along walls
	if (ent->matrixflip == RUN_RIGHT_ALONG_WALL && ent->velocity)
		ent->s.angles[ROLL] = 270;
	//switched
	else if (ent->matrixflip == RUN_LEFT_ALONG_WALL && ent->velocity)
		ent->s.angles[ROLL] = 90;
	else if (!ent->velocity)
		ent->s.angles[ROLL] = 0;

//run up walls
	
	if (ent->matrixflip == RWALL)
	{
		if(ent->up_yours == RIGHT)
		{
			ent->s.angles[PITCH] = 0;
			ent->s.angles[ROLL] = 270;
			
			ent->velocity[2] = 50;
		//	ent->client->kick_angles[2] = -45;
		}

		if(ent->up_yours == LEFT)
		{
			ent->s.angles[PITCH] = 0;
			ent->s.angles[ROLL] = 90;
			ent->velocity[2] = 50;
		//	ent->client->kick_angles[2] = 45;
		}
		if(ent->up_yours == STRAIGHTUP)
		{
			ent->s.angles[PITCH] = 270;
			ent->s.angles[ROLL] = 0;
			ent->velocity[2] = 300;
		}
		if(ent->up_yours == CEILING)
		{
			ent->s.angles[PITCH] = 0;
			ent->s.angles[ROLL] = 180;
			ent->s.angles[YAW] = ent->client->v_angle[YAW];
			ent->velocity[2] = 300;
		}

		ent->client->kick_angles[0] = -1;
		if (ent->s.frame == FRAME_run6)
		{
			ent->client->anim_priority = ANIM_ATTACK;
			ent->s.frame =  FRAME_run1 - 1;
		}
	}

	if(ent->matrixflip == KUNGFU)
	{
		if(ent->KungFuAnim == SPINKICK)
		{
			
			
			ent->client->anim_priority = ANIM_REVERSE;
			ent->s.frame =  45;
			ent->client->anim_end = 43;
		
			
		}
		if(ent->KungFuAnim == HOVERKICK)
		{
			if (random() < 0.5)
			{
			ent->client->anim_priority = ANIM_REVERSE;
			ent->s.frame =  44;
			ent->client->anim_end = 43;
			}
			else
			{
			ent->client->anim_priority = ANIM_REVERSE;
			ent->s.frame =  41;
			ent->client->anim_end = 40;
			}
		}
		if(ent->KungFuAnim == HOOK)
		{
			ent->client->anim_priority = ANIM_ATTACK;
			ent->s.frame =  113;
			ent->client->anim_end = 120;
		}
		if(ent->KungFuAnim == JAB)
		{
			if (random() < 0.5)
			{
			ent->client->anim_priority = ANIM_ATTACK;
			ent->s.frame =  58;
			ent->client->anim_end = 60;
			}
			else
			{
			ent->client->anim_priority = ANIM_ATTACK;
			ent->s.frame =  62;
			ent->client->anim_end = 64;
			}
		}
		if(ent->KungFuAnim == UPPERCUT)
		{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame =  72;
		ent->client->anim_end = 75;
		}
	
		
		ent->KungFuAnim = NONE;
	}
	//reset stuff
	//bug you don't do a full backflip if you are leaning forward (looking down)
	//because you IMMEDIATELY hit 0 and it turns off looping

	if(ent->client->anim_end == ent->s.frame && ent->matrixflip == KUNGFU)
	{
		ent->matrixflip = NOFLIP;
	
	}

	if (ent->matrixflip == BFLIP)
	{	
		if(ent->s.angles[PITCH] == 0)
			ent->matrixflip = NOFLIP;

		if(ent->s.angles[PITCH] < -360)
			ent->matrixflip = NOFLIP;
	}
	
	if (ent->matrixflip == FALLROLL)
	{
		if(ent->s.angles[PITCH] == 0)
			ent->matrixflip = NOFLIP;

		if(ent->s.angles[PITCH] > 360)
			ent->matrixflip = NOFLIP;
	}

	//if(ent->matrixflip == RUN_LEFT_ALONG_WALL 
	//	|| ent->matrixflip == RUN_RIGHT_ALONG_WALL)

//	return;

	if (ent->groundentity && ent->matrixflip != FALLROLL 
		&& ent->matrixflip != RUN_LEFT_ALONG_WALL 
		&& ent->matrixflip != RUN_RIGHT_ALONG_WALL
		&& ent->matrixflip != KUNGFU)
	{
	//	if (ent->matrixflip == RWALL)
	//	Cmd_Chasecam_Off (ent);
	//	if (ent->matrixflip == BFLIP)
	//	Cmd_Chasecam_Off (ent);
		ent->matrixflip = NOFLIP;		
	}
	if(ent->groundentity || ent->matrixflip != 0 || ent->waterlevel)
		ent->MatrixJumping = false;

	if(ent->MatrixJumping)
	{
		//every 5th of a second
		i =(level.time * 10);
	if (i % 2 != 0)
		SpawnShadow(ent);
	}
	
}
void MatrixRunUpWalls (edict_t *ent)
{
    vec3_t  forward, start, kick, blah;
    int     check = 0, n;
	qboolean LessThanOne = false, CutSky = false;
    trace_t tr, temptr;
	
	MatrixRunRAlongWalls (ent);
	MatrixRunLAlongWalls (ent);	
    	
	if (ent->deadflag || (ent->movetype == MOVETYPE_NOCLIP))
            return;			
	if (ent->groundentity)
			return;
	if (ent->vertbuttons<=0)
	{
		ent->matrixflip = NOFLIP;
		return;
	}
	if (ent->velocity[2]< -200)
			return;
	if (ent->waterlevel>0)
			return;

	VectorCopy (ent->client->v_angle, blah);
	
	blah[PITCH] = 0;
	blah[YAW] -= 90 ;
	AngleVectors (blah, forward, NULL, NULL);
	VectorMA(ent->s.origin, 24, forward, start);
	tr = gi.trace(ent->s.origin, vec3_origin, vec3_origin, start, ent, (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW));
	if(tr.fraction < 1)
	{
		ent->up_yours = LEFT;
		LessThanOne= true;
	VectorCopy (tr.endpos, temptr.endpos);
	
	}
	if(tr.surface->flags & SURF_SKY )
		CutSky = true;
		
	VectorCopy (ent->client->v_angle, blah);
	blah[PITCH] = 0;
	blah[YAW] += 90 ;
	AngleVectors (blah, forward, NULL, NULL);
	VectorMA(ent->s.origin, 24, forward, start);
	tr = gi.trace(ent->s.origin, vec3_origin, vec3_origin, start, ent, (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW));
	if(tr.fraction < 1 && !LessThanOne)
	{
		ent->up_yours = RIGHT;
		LessThanOne= true;
		VectorCopy (tr.endpos, temptr.endpos);
		
	}
	if(tr.surface->flags & SURF_SKY )
		CutSky = true;

	VectorCopy (ent->client->v_angle, blah);
	blah[PITCH] = 270;
	AngleVectors (blah, forward, NULL, NULL);
	VectorMA(ent->s.origin, 48, forward, start);
	tr = gi.trace(ent->s.origin, vec3_origin, vec3_origin, start, ent, (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW));
	if(tr.fraction < 1 && !LessThanOne)
	{
		ent->up_yours = CEILING;
		LessThanOne= true;
		VectorCopy (tr.endpos, temptr.endpos);

	}
	if(tr.surface->flags & SURF_SKY )
		CutSky = true;

	VectorCopy (ent->client->v_angle, blah);
	blah[PITCH] = 0;
	AngleVectors (blah, forward, NULL, NULL);
	VectorMA(ent->s.origin, 24, forward, start);
	tr = gi.trace(ent->s.origin, vec3_origin, vec3_origin, start, ent, (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW));
	if(tr.fraction < 1 && !LessThanOne)
	{
		ent->up_yours = STRAIGHTUP;
		LessThanOne= true;
		VectorCopy (tr.endpos, temptr.endpos);
		
	}
	if(tr.surface->flags & SURF_SKY )
		CutSky = true;


	
	//	gi.WriteByte (svc_temp_entity);
//		gi.WriteByte (TE_BFG_LASER);
//		gi.WritePosition (ent->s.origin);
//		gi.WritePosition (tr.endpos);
//		gi.multicast (ent->s.origin, MULTICAST_PHS);
	//detect wall
	
	if (LessThanOne && !CutSky)
	{
		if (ent->matrixflip != RWALL)
		{
			ent->client->anim_priority = ANIM_ATTACK;
			ent->s.frame = FRAME_run1 - 1;
			ent->client->anim_end = FRAME_run6; //start running
			ent->matrixflip = RWALL;		
			//Cmd_Chasecam_On (ent);
		}
		ent->wallframe = 1;
	}

	if(CutSky)
	{
		ent->wallframe = 0;
		ent->matrixflip = NOFLIP;
	}

	if (!LessThanOne)
	{
		ent->wallframe = 0;
		ent->matrixflip = NOFLIP;
	}

	//speed up wall
	if (ent->wallframe == 1)
	{
		ent->velocity[2] = 300;
	}

	
	if(ent->forwardbuttons<0)
	{
		if(ent->matrixflip == RWALL && ent->up_yours != CEILING)
		{
				VectorSubtract(ent->s.origin, temptr.endpos, kick);
				VectorNormalize(kick);
				VectorScale(kick, 300, kick);
				VectorCopy(kick, ent->velocity);
			//	ent->stamina -= 5;
				ent->velocity[2] = 300;
				ent->groundentity = NULL;
				ent->last_jump_time = level.time + JUMP_TIME;
				ent->matrixflip = BFLIP;
		}
	}
}

void MatrixRunRAlongWalls (edict_t *ent)
{
	vec3_t  forward, start, kick, blah;
    trace_t tr;
	
if(!ent->velocity)
{
	ent->matrixflip = NOFLIP;
	return;
}
	

	if(ent->matrixflip != RUN_LEFT_ALONG_WALL 
	&& ent->matrixflip != RUN_RIGHT_ALONG_WALL
	&& ent->matrixflip != NOFLIP)
		return;


	VectorCopy (ent->client->v_angle, blah);
	blah[PITCH] = 0;
	blah[YAW] += 90;
    AngleVectors (blah, forward, NULL, NULL);
	VectorMA(ent->s.origin, 24, forward, start);

	tr = gi.trace(ent->s.origin, vec3_origin, vec3_origin, start, ent, MASK_SOLID);
	
	if (tr.fraction <1)
		ent->matrixflip = RUN_RIGHT_ALONG_WALL;
	else
		ent->matrixflip = NOFLIP;



	//	gi.WriteByte (svc_temp_entity);
	//	gi.WriteByte (TE_BFG_LASER);
	//	gi.WritePosition (ent->s.origin);
	//	gi.WritePosition (tr.endpos);
	//	gi.multicast (ent->s.origin, MULTICAST_PHS);
}
void MatrixRunLAlongWalls (edict_t *ent)
{

	vec3_t  forward, start, kick, blah;
    trace_t tr;

if(!ent->velocity)
{
	ent->matrixflip = NOFLIP;

	return;
}


if(ent->matrixflip != RUN_LEFT_ALONG_WALL 
	&& ent->matrixflip != NOFLIP)
		return;

	VectorCopy (ent->client->v_angle, blah);
	blah[PITCH] = 0;
	blah[YAW] -= 90;
    AngleVectors (blah, forward, NULL, NULL);
	VectorMA(ent->s.origin, 24, forward, start);
	tr = gi.trace(ent->s.origin, vec3_origin, vec3_origin, start, ent, MASK_SOLID);
	
	if (tr.fraction <1)
		ent->matrixflip = RUN_LEFT_ALONG_WALL;
	else
		ent->matrixflip = NOFLIP;
	


	//	gi.WriteByte (svc_temp_entity);
	//	gi.WriteByte (TE_BFG_LASER);
	//	gi.WritePosition (ent->s.origin);
	//	gi.WritePosition (tr.endpos);
	//	gi.multicast (ent->s.origin, MULTICAST_PHS);
}

int MatrixFallingRoll(edict_t *ent, int damage)
{
	//called from the falling damage place p_fallingdamage
	// you must be pressing crouch
    if(ent->vertbuttons<0)
	{
		ent->matrixflip = FALLROLL;
		damage = 0; // if it's 0, it gives infinite damage
		//this was surprisingly simple to do - for me n e way :p
	}
	return damage;
}

void MatrixSpeed (edict_t *ent)
{
	int		i;


	if(ent->client->speed_framenum > level.framenum)
	{
		gi.cvar_set("cl_forwardspeed","180");
		gi.cvar_set("cl_sidespeed","180");
		gi.cvar_set("cl_upspeed","50");
	
	i =(level.time * 10);
	if (i % 2 != 0)
		SpawnShadow (ent);
	}
	else
	{
	gi.cvar_set("cl_forwardspeed","200");
	gi.cvar_set("cl_sidespeed","200");
	}

}

void SuperJump (edict_t *ent)
{
	vec3_t  forward;
	float	xyspeed;
	int		impulse;

	if(ent->stamina < 50)
		return;

	if(!ent->groundentity)
		return;

	if(ent->client->speed_framenum > level.framenum)
		impulse = 400;
	else
		impulse = 600;
	//matrix jump is a little too powerful
	ent->stamina -= 50;
	xyspeed = sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);
	//pythag. so it WAS useful for something!!

	if (xyspeed > 750)
			return;
 
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);
	VectorMA (ent->velocity, impulse, forward, ent->velocity);
	ent->velocity[2] += impulse * 0.75;
	ent->MatrixJumping = true;

	gi.sound (ent, CHAN_VOICE, gi.soundindex ("matrixjump.wav"), 1, ATTN_NORM, 0);
	//got to be a better way than this - increse pitch??
	//VectorScale (forward, ent->client->pers.jump_height, ent->velocity);

//thx to mr grim, who gave me this and finally set me straight regarding vectors.
 }

/*
BACKFLIPBOOL = Matrixmove now
its a enum, i pressume this is what you want it to be
it can only take those values that you have created i think
NOFLIP = 0 and it goes up +1 each time

Painberry's note on backflip bool. no this is not a boolean. it just started that way.
 it is an int to register which s.angle adjusting thing is happening, so that you only do one at a
  tome, e.g. inyour old code you do mental impossible flips, which i'm afraid are a bit dumb.
  also i didn't want forward flips: when you run down slopes yo start somersaulting, which is dumb.

  when backflip bool = 0 any move can be done. if it is not 0, no other moves can usurp it.
 */
void SpawnShadow (edict_t *ent)
{
	edict_t *shadow;
	
	if(ent->deadflag)
		return;
	
	if(ent->client->speed_framenum > level.framenum && ent->groundentity)
	{
		SpawnWave (ent);
		return;
	}
	// spawn the bot on a spawn spot
	shadow = G_Spawn();
	
	shadow->s = ent->s;
	
	if(!ent->velocity[0] && !ent->velocity[1])
	shadow->s.frame = random()* 180;
	
	shadow->s.modelindex2 = 0;
	shadow->s.modelindex3 = 0;
	shadow->s.modelindex4 = 0;
	VectorCopy (ent->s.old_origin, shadow->s.origin);
	VectorCopy (ent->s.angles, shadow->s.angles);


	shadow->classname = "shadow";
	shadow->movetype = MOVETYPE_NONE;
	shadow->solid = SOLID_NOT;
	shadow->owner = ent;
	
	if(IsFemale(ent))
	shadow->model = "players/female/tris.md2";
	else if(IsNeutral(ent))
	shadow->model = "players/cyborg/tris.md2";
	else
	shadow->model = "players/male/tris.md2";
	
	shadow->s.effects |= EF_SPHERETRANS;
	shadow->waterlevel = 0;
	shadow->watertype = 0;
	shadow->health = 100;
	shadow->max_health = 100;
	shadow->gib_health = -40;
	shadow->s.skinnum = 0;
	//shadow->s.skinnum = ent - g_edicts - 1;
	// think functions	
	shadow->nextthink = level.time + 0.8;
	shadow->think = G_FreeEdict;
	VectorSet (shadow->mins, -16, -16, -24);
	VectorSet (shadow->maxs, 16, 16, 32);
	

	VectorClear (shadow->velocity);
	
	gi.linkentity (shadow);
}

void SpeedWaveThink (edict_t *wave)
{
	if(wave->s.frame ==3)
		G_FreeEdict (wave);
	else
	wave->s.frame++;
}
		
void SpawnWave (edict_t *ent)
{
	edict_t *wave;
	vec3_t	dir;
	if(ent->deadflag)
		return;
	if(!ent->groundentity) // only on floor
		return;

	
	wave = G_Spawn();
	wave->s = ent->s;
	
	
	wave->s.modelindex2 = 0;
	wave->s.modelindex3 = 0;
	wave->s.modelindex4 = 0;
	VectorCopy (ent->s.old_origin, wave->s.origin);
	vectoangles(ent->velocity, wave->s.angles);
	
	wave->s.angles[PITCH] = 0; //don't want it sticking up
	wave->s.angles[ROLL] = 0; //don't want it sticking up
	
	wave->s.frame = 0;

	wave->classname = "speedwave";
	wave->movetype = MOVETYPE_NONE;
	wave->solid = SOLID_NOT;
	wave->owner = ent;
	wave->s.modelindex = gi.modelindex ("models/objects/speed/tris.md2");
	wave->s.effects |= EF_SPHERETRANS;

	//wave->waterlevel = 0;
	//wave->watertype = 0;
	//wave->health = 100;
	//wave->max_health = 100;
	//wave->gib_health = -40;
	//wave->s.skinnum = ent - g_edicts - 1;
	// think functions	
	
	wave->nextthink = level.time + 0.4;
	wave->think = G_FreeEdict;
	VectorSet (wave->mins, -16, -16, -24);
	VectorSet (wave->maxs, 16, 16, 32);
	VectorClear (wave->velocity);
	
	gi.linkentity (wave);
}
// replace "MatrixSpeed(ent)" with
// "MatrixCheckSpeed(ent)"
//and put "MatrixSpeed(ent)" at the bottom of
// cmd_BuySpeed

void MatrixCheckSpeed (edict_t *ent)
{
	int	i;

	if(ent->client->speed_framenum > level.framenum)
	{
		if(ent->velocity[0] || ent->velocity[1])
		//every 5th of a second
	i =(level.time * 10);
	if (i % 2 != 0)
		SpawnWave (ent);
		return;
	}

	if(ent->pinned_nextthink>level.framenum)
	{
		i = 0;
		VectorCopy(ent->pinnedpos,ent->s.old_origin);
		VectorClear(ent->velocity);
	}
		else
	i = 180;

	if (ent->forwardbuttons > i || ent->strafebuttons > i)
	{
		char number[12];

		Com_sprintf(number, 12, "%i", i);
	
		gi.cvar_set("cl_forwardspeed", number);
		gi.cvar_set("cl_sidespeed", number);
	}	
}

void MatrixScreenTilt (edict_t *ent)
{
	if(!ent->screentilt)
		return;
	if(ent->deadflag)
		return;

	
	if(ent->matrixflip == RWALL && ent->up_yours == RIGHT)
	{
		ent->client->kick_angles[ROLL] += 90;
		return;
	}

	if(ent->matrixflip == RWALL && ent->up_yours == LEFT)
	{
		ent->client->kick_angles[ROLL] -= 90;
		return;
	}

	if(ent->strafebuttons > 0 && !ent->groundentity)
	{
		ent->client->kick_angles[ROLL] += 5;
		return;
	}
	
	if(ent->strafebuttons < 0 && !ent->groundentity)
	{
		ent->client->kick_angles[ROLL] -= 5;
		return;
	}


	//Kyle's first addition to MQ2!
	//man i suck....
	if(ent->strafebuttons > 0)
	{
		ent->client->kick_angles[ROLL] += 5;
		return;
	}
	
	if(ent->strafebuttons < 0)
	{
		ent->client->kick_angles[ROLL] -= 5;
		return;
	}
  
	
}