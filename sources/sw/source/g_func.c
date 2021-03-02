#include "g_local.h"
#include "g_files.h"

#define TRAIN_START_ON		   1
#define TRAIN_TOGGLE		   2
#define TRAIN_BLOCK_STOPS	   4
#define TRAIN_ROTATE           8
#define TRAIN_ROTATE_CONSTANT 16
#define TRAIN_ROTATE_MASK     (TRAIN_ROTATE | TRAIN_ROTATE_CONSTANT)
#define TRAIN_ANIMATE         32
#define TRAIN_ANIMATE_FAST    64
#define TRAIN_SMOOTH         128

void CoolGrav (edict_t *ent, edict_t *other, edict_t *activator);
void za_apa (edict_t *ent);
void spawn_sprite (edict_t *self);
void bouncebaby (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
/*
=========================================================

  PLATS

  movement options:

  linear
  smooth start, hard stop
  smooth start, smooth stop

  start
  end
  acceleration
  speed
  deceleration
  begin sound
  end sound
  target fired when reaching end
  wait at end

  object characteristics that use move segments
  ---------------------------------------------
  movetype_push, or movetype_stop
  action when touched
  action when blocked
  action when used
	disabled?
  auto trigger spawning


=========================================================
*/

#define PLAT_LOW_TRIGGER	1

#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3

#define DOOR_START_OPEN		1
#define DOOR_REVERSE		2
#define DOOR_CRUSHER		4
#define DOOR_NOMONSTER		8
#define DOOR_TOGGLE			32
#define DOOR_X_AXIS			64
#define DOOR_Y_AXIS			128


//
// Support routines for movement (changes in origin using velocity)
//

void Move_Done (edict_t *ent)
{
	VectorClear (ent->velocity);
	ent->moveinfo.endfunc (ent);
}

void Move_Final (edict_t *ent)
{
	if (ent->moveinfo.remaining_distance == 0)
	{
		Move_Done (ent);
		return;
	}

	VectorScale (ent->moveinfo.dir, ent->moveinfo.remaining_distance / FRAMETIME, ent->velocity);

	ent->think = Move_Done;
	ent->nextthink = level.time + FRAMETIME;
}

void Move_Begin (edict_t *ent)
{
	float	frames;

	if ((ent->moveinfo.speed * FRAMETIME) >= ent->moveinfo.remaining_distance)
	{
		Move_Final (ent);
		return;
	}
	VectorScale (ent->moveinfo.dir, ent->moveinfo.speed, ent->velocity);
	frames = floor((ent->moveinfo.remaining_distance / ent->moveinfo.speed) / FRAMETIME);
	ent->moveinfo.remaining_distance -= frames * ent->moveinfo.speed * FRAMETIME;
	ent->nextthink = level.time + (frames * FRAMETIME);
	ent->think = Move_Final;
}

void Think_AccelMove (edict_t *ent);

void Move_Calc (edict_t *ent, vec3_t dest, void(*func)(edict_t*))
{
	VectorClear (ent->velocity);
	VectorSubtract (dest, ent->s.origin, ent->moveinfo.dir);
	ent->moveinfo.remaining_distance = VectorNormalize (ent->moveinfo.dir);
	ent->moveinfo.endfunc = func;

	if (ent->moveinfo.speed == ent->moveinfo.accel && ent->moveinfo.speed == ent->moveinfo.decel)
	{
		if (level.current_entity == ((ent->flags & FL_TEAMSLAVE) ? ent->teammaster : ent))
		{
			Move_Begin (ent);
		}
		else
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = Move_Begin;
		}
	}
	else
	{
		// accelerative
		ent->moveinfo.current_speed = 0;
		ent->think = Think_AccelMove;
		ent->nextthink = level.time + FRAMETIME;
	}
}


//
// Support routines for angular movement (changes in angle using avelocity)
//

void AngleMove_Done (edict_t *ent)
{
	VectorClear (ent->avelocity);
	ent->moveinfo.endfunc (ent);
}

void AngleMove_Final (edict_t *ent)
{
	vec3_t	move;

	if (ent->moveinfo.state == STATE_UP)
		VectorSubtract (ent->moveinfo.end_angles, ent->s.angles, move);
	else
		VectorSubtract (ent->moveinfo.start_angles, ent->s.angles, move);

	if (VectorCompare (move, vec3_origin))
	{
		AngleMove_Done (ent);
		return;
	}

	VectorScale (move, 1.0/FRAMETIME, ent->avelocity);

	ent->think = AngleMove_Done;
	ent->nextthink = level.time + FRAMETIME;
}

void AngleMove_Begin (edict_t *ent)
{
	vec3_t	destdelta;
	float	len;
	float	traveltime;
	float	frames;

	// set destdelta to the vector needed to move
	if (ent->moveinfo.state == STATE_UP)
		VectorSubtract (ent->moveinfo.end_angles, ent->s.angles, destdelta);
	else
		VectorSubtract (ent->moveinfo.start_angles, ent->s.angles, destdelta);
	
	// calculate length of vector
	len = VectorLength (destdelta);
	
	// divide by speed to get time to reach dest
	traveltime = len / ent->moveinfo.speed;

	if (traveltime < FRAMETIME)
	{
		AngleMove_Final (ent);
		return;
	}

	frames = floor(traveltime / FRAMETIME);

	// scale the destdelta vector by the time spent traveling to get velocity
	VectorScale (destdelta, 1.0 / traveltime, ent->avelocity);

	// set nextthink to trigger a think when dest is reached
	ent->nextthink = level.time + frames * FRAMETIME;
	ent->think = AngleMove_Final;
}

void AngleMove_Calc (edict_t *ent, void(*func)(edict_t*))
{
	VectorClear (ent->avelocity);
	ent->moveinfo.endfunc = func;
	if (level.current_entity == ((ent->flags & FL_TEAMSLAVE) ? ent->teammaster : ent))
	{
		AngleMove_Begin (ent);
	}
	else
	{
		ent->nextthink = level.time + FRAMETIME;
		ent->think = AngleMove_Begin;
	}
}


/*
==============
Think_AccelMove

The team has completed a frame of movement, so
change the speed for the next frame
==============
*/
#define AccelerationDistance(target, rate)	(target * ((target / rate) + 1) / 2)

void plat_CalcAcceleratedMove(moveinfo_t *moveinfo)
{
	float	accel_dist;
	float	decel_dist;

	moveinfo->move_speed = moveinfo->speed;

	if (moveinfo->remaining_distance < moveinfo->accel)
	{
		moveinfo->current_speed = moveinfo->remaining_distance;
		return;
	}

	accel_dist = AccelerationDistance (moveinfo->speed, moveinfo->accel);
	decel_dist = AccelerationDistance (moveinfo->speed, moveinfo->decel);

	if ((moveinfo->remaining_distance - accel_dist - decel_dist) < 0)
	{
		float	f;

		f = (moveinfo->accel + moveinfo->decel) / (moveinfo->accel * moveinfo->decel);
		moveinfo->move_speed = (-2 + sqrt(4 - 4 * f * (-2 * moveinfo->remaining_distance))) / (2 * f);
		decel_dist = AccelerationDistance (moveinfo->move_speed, moveinfo->decel);
	}

	moveinfo->decel_distance = decel_dist;
};

void plat_Accelerate (moveinfo_t *moveinfo)
{
	// are we decelerating?
	if (moveinfo->remaining_distance <= moveinfo->decel_distance)
	{
		if (moveinfo->remaining_distance < moveinfo->decel_distance)
		{
			if (moveinfo->next_speed)
			{
				moveinfo->current_speed = moveinfo->next_speed;
				moveinfo->next_speed = 0;
				return;
			}
			if (moveinfo->current_speed > moveinfo->decel)
				moveinfo->current_speed -= moveinfo->decel;
		}
		return;
	}

	// are we at full speed and need to start decelerating during this move?
	if (moveinfo->current_speed == moveinfo->move_speed)
		if ((moveinfo->remaining_distance - moveinfo->current_speed) < moveinfo->decel_distance)
		{
			float	p1_distance;
			float	p2_distance;
			float	distance;

			p1_distance = moveinfo->remaining_distance - moveinfo->decel_distance;
			p2_distance = moveinfo->move_speed * (1.0 - (p1_distance / moveinfo->move_speed));
			distance = p1_distance + p2_distance;
			moveinfo->current_speed = moveinfo->move_speed;
			moveinfo->next_speed = moveinfo->move_speed - moveinfo->decel * (p2_distance / distance);
			return;
		}

	// are we accelerating?
	if (moveinfo->current_speed < moveinfo->speed)
	{
		float	old_speed;
		float	p1_distance;
		float	p1_speed;
		float	p2_distance;
		float	distance;

		old_speed = moveinfo->current_speed;

		// figure simple acceleration up to move_speed
		moveinfo->current_speed += moveinfo->accel;
		if (moveinfo->current_speed > moveinfo->speed)
			moveinfo->current_speed = moveinfo->speed;

		// are we accelerating throughout this entire move?
		if ((moveinfo->remaining_distance - moveinfo->current_speed) >= moveinfo->decel_distance)
			return;

		// during this move we will accelrate from current_speed to move_speed
		// and cross over the decel_distance; figure the average speed for the
		// entire move
		p1_distance = moveinfo->remaining_distance - moveinfo->decel_distance;
		p1_speed = (old_speed + moveinfo->move_speed) / 2.0;
		p2_distance = moveinfo->move_speed * (1.0 - (p1_distance / p1_speed));
		distance = p1_distance + p2_distance;
		moveinfo->current_speed = (p1_speed * (p1_distance / distance)) + (moveinfo->move_speed * (p2_distance / distance));
		moveinfo->next_speed = moveinfo->move_speed - moveinfo->decel * (p2_distance / distance);
		return;
	}

	// we are at constant velocity (move_speed)
	return;
};

void Think_AccelMove (edict_t *ent)
{
	ent->moveinfo.remaining_distance -= ent->moveinfo.current_speed;

	if (ent->moveinfo.current_speed == 0)		// starting or blocked
		plat_CalcAcceleratedMove(&ent->moveinfo);

	plat_Accelerate (&ent->moveinfo);

	// will the entire move complete on next frame?
	if (ent->moveinfo.remaining_distance <= ent->moveinfo.current_speed)
	{
		Move_Final (ent);
		return;
	}

	VectorScale (ent->moveinfo.dir, ent->moveinfo.current_speed*10, ent->velocity);
	ent->nextthink = level.time + FRAMETIME;
	ent->think = Think_AccelMove;
}


void plat_go_down (edict_t *ent);

void plat_hit_top (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ATTN_STATIC, 0);
		ent->s.sound = 0;
	}
	ent->moveinfo.state = STATE_TOP;

	ent->think = plat_go_down;
	ent->nextthink = level.time + 3;
}

void plat_hit_bottom (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ATTN_STATIC, 0);
		ent->s.sound = 0;
	}
	ent->moveinfo.state = STATE_BOTTOM;
}

void plat_go_down (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_start)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ATTN_STATIC, 0);
		ent->s.sound = ent->moveinfo.sound_middle;
	}
	ent->moveinfo.state = STATE_DOWN;
	Move_Calc (ent, ent->moveinfo.end_origin, plat_hit_bottom);
}

void plat_go_up (edict_t *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_start)
			gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ATTN_STATIC, 0);
		ent->s.sound = ent->moveinfo.sound_middle;
	}
	ent->moveinfo.state = STATE_UP;
	Move_Calc (ent, ent->moveinfo.start_origin, plat_hit_top);
}

void plat_blocked (edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client) )
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other)
			BecomeExplosion1 (other);
		return;
	}

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

	if (self->moveinfo.state == STATE_UP)
		plat_go_down (self);
	else if (self->moveinfo.state == STATE_DOWN)
		plat_go_up (self);
}


void Use_Plat (edict_t *ent, edict_t *other, edict_t *activator)
{ 
	if (ent->think)
		return;		// already down
	plat_go_down (ent);
}


void Touch_Plat_Center (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;
		
	if (other->health <= 0)
		return;

	ent = ent->enemy;	// now point at the plat, not the trigger
	if (ent->moveinfo.state == STATE_BOTTOM)
		plat_go_up (ent);
	else if (ent->moveinfo.state == STATE_TOP)
		ent->nextthink = level.time + 1;	// the player is still on the plat, so delay going down
}

void plat_spawn_inside_trigger (edict_t *ent)
{
	edict_t	*trigger;
	vec3_t	tmin, tmax;

//
// middle trigger
//	
	trigger = G_Spawn();
	trigger->touch = Touch_Plat_Center;
	trigger->movetype = MOVETYPE_NONE;
	trigger->solid = SOLID_TRIGGER;
	trigger->enemy = ent;
	
	tmin[0] = ent->mins[0] + 25;
	tmin[1] = ent->mins[1] + 25;
	tmin[2] = ent->mins[2];

	tmax[0] = ent->maxs[0] - 25;
	tmax[1] = ent->maxs[1] - 25;
	tmax[2] = ent->maxs[2] + 8;

	tmin[2] = tmax[2] - (ent->pos1[2] - ent->pos2[2] + st.lip);

	if (ent->spawnflags & PLAT_LOW_TRIGGER)
		tmax[2] = tmin[2] + 8;
	
	if (tmax[0] - tmin[0] <= 0)
	{
		tmin[0] = (ent->mins[0] + ent->maxs[0]) *0.5;
		tmax[0] = tmin[0] + 1;
	}
	if (tmax[1] - tmin[1] <= 0)
	{
		tmin[1] = (ent->mins[1] + ent->maxs[1]) *0.5;
		tmax[1] = tmin[1] + 1;
	}
	
	VectorCopy (tmin, trigger->mins);
	VectorCopy (tmax, trigger->maxs);

	gi.linkentity (trigger);
}


/*QUAKED func_plat (0 .5 .8) ? PLAT_LOW_TRIGGER
speed	default 150

Plats are always drawn in the extended position, so they will light correctly.

If the plat is the target of another trigger or button, it will start out disabled in the extended position until it is trigger, when it will lower and become a normal plat.

"speed"	overrides default 200.
"accel" overrides default 500
"lip"	overrides default 8 pixel lip

If the "height" key is set, that will determine the amount the plat moves, instead of being implicitly determoveinfoned by the model's height.

Set "sounds" to one of the following:
1) base fast
2) chain slow
*/
void SP_func_plat (edict_t *ent)
{
	VectorClear (ent->s.angles);
	ent->solid = SOLID_BSP;
	ent->movetype = MOVETYPE_PUSH;

	gi.setmodel (ent, ent->model);

	ent->blocked = plat_blocked;

	if (!ent->speed)
		ent->speed = 20;
	else
		ent->speed *= 0.1;

	if (!ent->accel)
		ent->accel = 5;
	else
		ent->accel *= 0.1;

	if (!ent->decel)
		ent->decel = 5;
	else
		ent->decel *= 0.1;

	if (!ent->dmg)
		ent->dmg = 2;

	if (!st.lip)
		st.lip = 8;

	// pos1 is the top position, pos2 is the bottom
	VectorCopy (ent->s.origin, ent->pos1);
	VectorCopy (ent->s.origin, ent->pos2);

	if (st.height)
		ent->pos2[2] -= st.height;
	else
		ent->pos2[2] -= (ent->maxs[2] - ent->mins[2]) - st.lip;

	ent->use = Use_Plat;

	plat_spawn_inside_trigger (ent);	// the "start moving" trigger	

	if (ent->targetname)
	{
		ent->moveinfo.state = STATE_UP;
	}
	else
	{
		VectorCopy (ent->pos2, ent->s.origin);
		gi.linkentity (ent);
		ent->moveinfo.state = STATE_BOTTOM;
	}

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	VectorCopy (ent->pos1, ent->moveinfo.start_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy (ent->pos2, ent->moveinfo.end_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.end_angles);

	ent->moveinfo.sound_start = gi.soundindex ("plats/pt1_strt.wav");
	ent->moveinfo.sound_middle = gi.soundindex ("plats/pt1_mid.wav");
	ent->moveinfo.sound_end = gi.soundindex ("plats/pt1_end.wav");
}

//====================================================================

/*QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS TOUCH_PAIN STOP ANIMATED ANIMATED_FAST
You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"speed" determines how fast it moves; default value is 100.
"dmg"	damage to inflict when blocked (2 default)

REVERSE will cause the it to rotate in the opposite direction.
STOP mean it will stop moving instead of pushing entities
*/

void rotating_blocked (edict_t *self, edict_t *other)
{
	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void rotating_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (self->avelocity[0] || self->avelocity[1] || self->avelocity[2])
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void rotating_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!VectorCompare (self->avelocity, vec3_origin))
	{
		self->s.sound = 0;
		VectorClear (self->avelocity);
		self->touch = NULL;
	}
	else
	{
		self->s.sound = self->moveinfo.sound_middle;
		VectorScale (self->movedir, self->speed, self->avelocity);
		if (self->spawnflags & 16)
			self->touch = rotating_touch;
	}
}

void SP_func_rotating (edict_t *ent)
{
	ent->solid = SOLID_BSP;
	if (ent->spawnflags & 32)
		ent->movetype = MOVETYPE_STOP;
	else
		ent->movetype = MOVETYPE_PUSH;

	// set the axis of rotation
	VectorClear(ent->movedir);
	if (ent->spawnflags & 4)
		ent->movedir[2] = 1.0;
	else if (ent->spawnflags & 8)
		ent->movedir[0] = 1.0;
	else // Z_AXIS
		ent->movedir[1] = 1.0;

	// check for reverse rotation
	if (ent->spawnflags & 2)
		VectorNegate (ent->movedir, ent->movedir);

	if (!ent->speed)
		ent->speed = 100;
	if (!ent->dmg)
		ent->dmg = 2;

//	ent->moveinfo.sound_middle = "doors/hydro1.wav";

	ent->use = rotating_use;
	if (ent->dmg)
		ent->blocked = rotating_blocked;

	if (ent->spawnflags & 1)
		ent->use (ent, NULL, NULL);

	if (ent->spawnflags & 64)
		ent->s.effects |= EF_ANIM_ALL;
	if (ent->spawnflags & 128)
		ent->s.effects |= EF_ANIM_ALLFAST;

	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);
}

/*
======================================================================

BUTTONS

======================================================================
*/

/*QUAKED func_button (0 .5 .8) ?
When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets, waits some time, then returns to it's original position where it can be triggered again.

"angle"		determines the opening direction
"target"	all entities with a matching targetname will be used
"speed"		override the default 40 speed
"wait"		override the default 1 second wait (-1 = never return)
"lip"		override the default 4 pixel lip remaining at end of move
"health"	if set, the button must be killed instead of touched
"sounds"
1) silent
2) steam metal
3) wooden clunk
4) metallic click
5) in-out
*/

void button_done (edict_t *self)
{
	self->moveinfo.state = STATE_BOTTOM;
	self->s.effects &= ~EF_ANIM23;
	self->s.effects |= EF_ANIM01;
}

void button_return (edict_t *self)
{
	self->moveinfo.state = STATE_DOWN;

	Move_Calc (self, self->moveinfo.start_origin, button_done);

	self->s.frame = 0;

	if (self->health)
		self->takedamage = DAMAGE_YES;
}

void button_wait (edict_t *self)
{
	self->moveinfo.state = STATE_TOP;
	self->s.effects &= ~EF_ANIM01;
	self->s.effects |= EF_ANIM23;

	G_UseTargets (self, self->activator);
	self->s.frame = 1;
	if (self->moveinfo.wait >= 0)
	{
		self->nextthink = level.time + self->moveinfo.wait;
		self->think = button_return;
	}
}

void button_fire (edict_t *self)
{
	if (self->moveinfo.state == STATE_UP || self->moveinfo.state == STATE_TOP)
		return;

	self->moveinfo.state = STATE_UP;
	if (self->moveinfo.sound_start && !(self->flags & FL_TEAMSLAVE))
		gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
	Move_Calc (self, self->moveinfo.end_origin, button_wait);
}

void button_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;
	button_fire (self);
}

void button_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;

	if (other->health <= 0)
		return;

	self->activator = other;
	button_fire (self);
}

void button_killed (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->activator = attacker;
	self->health = self->max_health;
	self->takedamage = DAMAGE_NO;
	button_fire (self);
}

void SP_func_button (edict_t *ent)
{
	vec3_t	abs_movedir;
	float	dist;

	G_SetMovedir (ent->s.angles, ent->movedir);
	ent->movetype = MOVETYPE_STOP;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);

	if (ent->sounds != 1)
		ent->moveinfo.sound_start = gi.soundindex ("switches/butn2.wav");
	
	if (!ent->speed)
		ent->speed = 40;
	if (!ent->accel)
		ent->accel = ent->speed;
	if (!ent->decel)
		ent->decel = ent->speed;

	if (!ent->wait)
		ent->wait = 3;
	if (!st.lip)
		st.lip = 4;

	VectorCopy (ent->s.origin, ent->pos1);
	abs_movedir[0] = fabs(ent->movedir[0]);
	abs_movedir[1] = fabs(ent->movedir[1]);
	abs_movedir[2] = fabs(ent->movedir[2]);
	dist = abs_movedir[0] * ent->size[0] + abs_movedir[1] * ent->size[1] + abs_movedir[2] * ent->size[2] - st.lip;
	VectorMA (ent->pos1, dist, ent->movedir, ent->pos2);

	ent->use = button_use;
	ent->s.effects |= EF_ANIM01;

	if (ent->health)
	{
		ent->max_health = ent->health;
		ent->die = button_killed;
		ent->takedamage = DAMAGE_YES;
	}
	else if (! ent->targetname)
		ent->touch = button_touch;

	ent->moveinfo.state = STATE_BOTTOM;

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	VectorCopy (ent->pos1, ent->moveinfo.start_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy (ent->pos2, ent->moveinfo.end_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.end_angles);

	gi.linkentity (ent);
}

/*
======================================================================

DOORS

  spawn a trigger surrounding the entire team unless it is
  already targeted by another

======================================================================
*/

/*QUAKED func_door (0 .5 .8) ? START_OPEN x CRUSHER NOMONSTER ANIMATED TOGGLE ANIMATED_FAST
TOGGLE		wait in both the start and end states for a trigger event.
START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER	monsters will not trigger this door

"message"	is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"	if set, door must be shot open
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"lip"		lip remaining at end of move (8 default)
"dmg"		damage to inflict when blocked (2 default)
"sounds"
1)	silent
2)	light
3)	medium
4)	heavy
*/

void door_use_areaportals (edict_t *self, qboolean open)
{
	edict_t	*t = NULL;

	if (!self->target)
		return;

	while ((t = G_Find (t, FOFS(targetname), self->target)))
	{
		if (Q_stricmp(t->classname, "func_areaportal") == 0)
		{
			gi.SetAreaPortalState (t->style, open);
		}
	}
}

void door_go_down (edict_t *self);

void door_hit_top (edict_t *self)
{
	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_end)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_STATIC, 0);
		self->s.sound = 0;
	}
	self->moveinfo.state = STATE_TOP;
	if (self->spawnflags & DOOR_TOGGLE)
		return;
	if (self->moveinfo.wait >= 0)
	{
		self->think = door_go_down;
		self->nextthink = level.time + self->moveinfo.wait;
	}
}

void door_hit_bottom (edict_t *self)
{
	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_end)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_STATIC, 0);
		self->s.sound = 0;
	}
	self->moveinfo.state = STATE_BOTTOM;
	door_use_areaportals (self, false);
}

void door_go_down (edict_t *self)
{
	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_start)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
		self->s.sound = self->moveinfo.sound_middle;
	}
	if (self->max_health)
	{
		self->takedamage = DAMAGE_YES;
		self->health = self->max_health;
	}
	
	self->moveinfo.state = STATE_DOWN;
	if (strcmp(self->classname, "func_door") == 0)
		Move_Calc (self, self->moveinfo.start_origin, door_hit_bottom);
	else if (strcmp(self->classname, "func_door_rotating") == 0)
		AngleMove_Calc (self, door_hit_bottom);
}

void door_go_up (edict_t *self, edict_t *activator)
{
	if (self->moveinfo.state == STATE_UP)
		return;		// already going up

	if (self->moveinfo.state == STATE_TOP)
	{	// reset top wait time
		if (self->moveinfo.wait >= 0)
			self->nextthink = level.time + self->moveinfo.wait;
		return;
	}
	
	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_start)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
		self->s.sound = self->moveinfo.sound_middle;
	}
	self->moveinfo.state = STATE_UP;
	if (strcmp(self->classname, "func_door") == 0)
		Move_Calc (self, self->moveinfo.end_origin, door_hit_top);
	else if (strcmp(self->classname, "func_door_rotating") == 0)
		AngleMove_Calc (self, door_hit_top);

	G_UseTargets (self, activator);
	door_use_areaportals (self, true);
}

void door_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*ent;

	if (self->flags & FL_TEAMSLAVE)
		return;

	if (self->spawnflags & DOOR_TOGGLE)
	{
		if (self->moveinfo.state == STATE_UP || self->moveinfo.state == STATE_TOP)
		{
			// trigger all paired doors
			for (ent = self ; ent ; ent = ent->teamchain)
			{
				ent->message = NULL;
				ent->touch = NULL;
				door_go_down (ent);
			}
			return;
		}
	}
	
	// trigger all paired doors
	for (ent = self ; ent ; ent = ent->teamchain)
	{
		ent->message = NULL;
		ent->touch = NULL;
		door_go_up (ent, activator);
	}
};

void Touch_DoorTrigger (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->health <= 0)
		return;

	if (!(other->svflags & SVF_MONSTER) && (!other->client))
		return;

	if ((self->owner->spawnflags & DOOR_NOMONSTER) && (other->svflags & SVF_MONSTER))
		return;

	if (level.time < self->touch_debounce_time)
		return;
	self->touch_debounce_time = level.time + 1.0;

	door_use (self->owner, other, other);
}

void Think_CalcMoveSpeed (edict_t *self)
{
	edict_t	*ent;
	float	min;
	float	time;
	float	newspeed;
	float	ratio;
	float	dist;

	if (self->flags & FL_TEAMSLAVE)
		return;		// only the team master does this

	// find the smallest distance any member of the team will be moving
	min = fabs(self->moveinfo.distance);
	for (ent = self->teamchain; ent; ent = ent->teamchain)
	{
		dist = fabs(ent->moveinfo.distance);
		if (dist < min)
			min = dist;
	}

	time = min / self->moveinfo.speed;

	// adjust speeds so they will all complete at the same time
	for (ent = self; ent; ent = ent->teamchain)
	{
		newspeed = fabs(ent->moveinfo.distance) / time;
		ratio = newspeed / ent->moveinfo.speed;
		if (ent->moveinfo.accel == ent->moveinfo.speed)
			ent->moveinfo.accel = newspeed;
		else
			ent->moveinfo.accel *= ratio;
		if (ent->moveinfo.decel == ent->moveinfo.speed)
			ent->moveinfo.decel = newspeed;
		else
			ent->moveinfo.decel *= ratio;
		ent->moveinfo.speed = newspeed;
	}
}

void Think_SpawnDoorTrigger (edict_t *ent)
{
	edict_t		*other;
	vec3_t		mins, maxs;

	if (ent->flags & FL_TEAMSLAVE)
		return;		// only the team leader spawns a trigger

	VectorCopy (ent->absmin, mins);
	VectorCopy (ent->absmax, maxs);

	for (other = ent->teamchain ; other ; other=other->teamchain)
	{
		AddPointToBounds (other->absmin, mins, maxs);
		AddPointToBounds (other->absmax, mins, maxs);
	}

	// expand 
	mins[0] -= 60;
	mins[1] -= 60;
	maxs[0] += 60;
	maxs[1] += 60;

	other = G_Spawn ();
	VectorCopy (mins, other->mins);
	VectorCopy (maxs, other->maxs);
	other->owner = ent;
	other->solid = SOLID_TRIGGER;
	other->movetype = MOVETYPE_NONE;
	other->touch = Touch_DoorTrigger;
	gi.linkentity (other);

	if (ent->spawnflags & DOOR_START_OPEN)
		door_use_areaportals (ent, true);

	Think_CalcMoveSpeed (ent);
}

void door_blocked  (edict_t *self, edict_t *other)
{
	edict_t	*ent;

	if (!(other->svflags & SVF_MONSTER) && (!other->client) )
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other)
			BecomeExplosion1 (other);
		return;
	}

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

	if (self->spawnflags & DOOR_CRUSHER)
		return;


// if a door has a negative wait, it would never come back if blocked,
// so let it just squash the object to death real fast
	if (self->moveinfo.wait >= 0)
	{
		if (self->moveinfo.state == STATE_DOWN)
		{
			for (ent = self->teammaster ; ent ; ent = ent->teamchain)
				door_go_up (ent, ent->activator);
		}
		else
		{
			for (ent = self->teammaster ; ent ; ent = ent->teamchain)
				door_go_down (ent);
		}
	}
}

void door_killed (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t	*ent;

	for (ent = self->teammaster ; ent ; ent = ent->teamchain)
	{
		ent->health = ent->max_health;
		ent->takedamage = DAMAGE_NO;
	}
	door_use (self->teammaster, attacker, attacker);
}

void door_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;

	if (level.time < self->touch_debounce_time)
		return;
	self->touch_debounce_time = level.time + 5.0;

	safe_centerprintf (other, "%s", self->message);
	gi.sound (other, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
}

void SP_func_door (edict_t *ent)
{
	vec3_t	abs_movedir;

	if (ent->sounds != 1)
	{
		ent->moveinfo.sound_start = gi.soundindex  ("doors/dr1_strt.wav");
		ent->moveinfo.sound_middle = gi.soundindex  ("doors/dr1_mid.wav");
		ent->moveinfo.sound_end = gi.soundindex  ("doors/dr1_end.wav");
	}

	G_SetMovedir (ent->s.angles, ent->movedir);
	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);

	ent->blocked = door_blocked;
	ent->use = door_use;
	
	if (!ent->speed)
		ent->speed = 100;
	if (deathmatch->value)
		ent->speed *= 2;

	if (!ent->accel)
		ent->accel = ent->speed;
	if (!ent->decel)
		ent->decel = ent->speed;

	if (!ent->wait)
		ent->wait = 3;
	if (!st.lip)
		st.lip = 8;
	if (!ent->dmg)
		ent->dmg = 2;

	// calculate second position
	VectorCopy (ent->s.origin, ent->pos1);
	abs_movedir[0] = fabs(ent->movedir[0]);
	abs_movedir[1] = fabs(ent->movedir[1]);
	abs_movedir[2] = fabs(ent->movedir[2]);
	ent->moveinfo.distance = abs_movedir[0] * ent->size[0] + abs_movedir[1] * ent->size[1] + abs_movedir[2] * ent->size[2] - st.lip;
	VectorMA (ent->pos1, ent->moveinfo.distance, ent->movedir, ent->pos2);

	// if it starts open, switch the positions
	if (ent->spawnflags & DOOR_START_OPEN)
	{
		VectorCopy (ent->pos2, ent->s.origin);
		VectorCopy (ent->pos1, ent->pos2);
		VectorCopy (ent->s.origin, ent->pos1);
	}

	ent->moveinfo.state = STATE_BOTTOM;

	if (ent->health)
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = door_killed;
		ent->max_health = ent->health;
	}
	else if (ent->targetname && ent->message)
	{
		gi.soundindex ("misc/talk.wav");
		ent->touch = door_touch;
	}
	
	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	VectorCopy (ent->pos1, ent->moveinfo.start_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.start_angles);
	VectorCopy (ent->pos2, ent->moveinfo.end_origin);
	VectorCopy (ent->s.angles, ent->moveinfo.end_angles);

	if (ent->spawnflags & 16)
		ent->s.effects |= EF_ANIM_ALL;
	if (ent->spawnflags & 64)
		ent->s.effects |= EF_ANIM_ALLFAST;

	// to simplify logic elsewhere, make non-teamed doors into a team of one
	if (!ent->team)
		ent->teammaster = ent;

	gi.linkentity (ent);

	ent->nextthink = level.time + FRAMETIME;
	if (ent->health || ent->targetname)
		ent->think = Think_CalcMoveSpeed;
	else
		ent->think = Think_SpawnDoorTrigger;
}


/*QUAKED func_door_rotating (0 .5 .8) ? START_OPEN REVERSE CRUSHER NOMONSTER ANIMATED TOGGLE X_AXIS Y_AXIS
TOGGLE causes the door to wait in both the start and end states for a trigger event.

START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER	monsters will not trigger this door

You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"distance" is how many degrees the door will be rotated.
"speed" determines how fast the door moves; default value is 100.

REVERSE will cause the door to rotate in the opposite direction.

"message"	is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"	if set, door must be shot open
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"dmg"		damage to inflict when blocked (2 default)
"sounds"
1)	silent
2)	light
3)	medium
4)	heavy
*/

void SP_func_door_rotating (edict_t *ent)
{
	VectorClear (ent->s.angles);

	// set the axis of rotation
	VectorClear(ent->movedir);
	if (ent->spawnflags & DOOR_X_AXIS)
		ent->movedir[2] = 1.0;
	else if (ent->spawnflags & DOOR_Y_AXIS)
		ent->movedir[0] = 1.0;
	else // Z_AXIS
		ent->movedir[1] = 1.0;

	// check for reverse rotation
	if (ent->spawnflags & DOOR_REVERSE)
		VectorNegate (ent->movedir, ent->movedir);

	if (!st.distance)
	{
		gi.dprintf("%s at %s with no distance set\n", ent->classname, vtos(ent->s.origin));
		st.distance = 90;
	}

	VectorCopy (ent->s.angles, ent->pos1);
	VectorMA (ent->s.angles, st.distance, ent->movedir, ent->pos2);
	ent->moveinfo.distance = st.distance;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);

	ent->blocked = door_blocked;
	ent->use = door_use;

	if (!ent->speed)
		ent->speed = 100;
	if (!ent->accel)
		ent->accel = ent->speed;
	if (!ent->decel)
		ent->decel = ent->speed;

	if (!ent->wait)
		ent->wait = 3;
	if (!ent->dmg)
		ent->dmg = 2;

	if (ent->sounds != 1)
	{
		ent->moveinfo.sound_start = gi.soundindex  ("doors/dr1_strt.wav");
		ent->moveinfo.sound_middle = gi.soundindex  ("doors/dr1_mid.wav");
		ent->moveinfo.sound_end = gi.soundindex  ("doors/dr1_end.wav");
	}

	// if it starts open, switch the positions
	if (ent->spawnflags & DOOR_START_OPEN)
	{
		VectorCopy (ent->pos2, ent->s.angles);
		VectorCopy (ent->pos1, ent->pos2);
		VectorCopy (ent->s.angles, ent->pos1);
		VectorNegate (ent->movedir, ent->movedir);
	}

	if (ent->health)
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = door_killed;
		ent->max_health = ent->health;
	}
	
	if (ent->targetname && ent->message)
	{
		gi.soundindex ("misc/talk.wav");
		ent->touch = door_touch;
	}

	ent->moveinfo.state = STATE_BOTTOM;
	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	VectorCopy (ent->s.origin, ent->moveinfo.start_origin);
	VectorCopy (ent->pos1, ent->moveinfo.start_angles);
	VectorCopy (ent->s.origin, ent->moveinfo.end_origin);
	VectorCopy (ent->pos2, ent->moveinfo.end_angles);

	if (ent->spawnflags & 16)
		ent->s.effects |= EF_ANIM_ALL;

	// to simplify logic elsewhere, make non-teamed doors into a team of one
	if (!ent->team)
		ent->teammaster = ent;

	gi.linkentity (ent);

	ent->nextthink = level.time + FRAMETIME;
	if (ent->health || ent->targetname)
		ent->think = Think_CalcMoveSpeed;
	else
		ent->think = Think_SpawnDoorTrigger;
}


/*QUAKED func_water (0 .5 .8) ? START_OPEN
func_water is a moveable water brush.  It must be targeted to operate.  Use a non-water texture at your own risk.

START_OPEN causes the water to move to its destination when spawned and operate in reverse.

"angle"		determines the opening direction (up or down only)
"speed"		movement speed (25 default)
"wait"		wait before returning (-1 default, -1 = TOGGLE)
"lip"		lip remaining at end of move (0 default)
"sounds"	(yes, these need to be changed)
0)	no sound
1)	water
2)	lava
*/

void SP_func_water (edict_t *self)
{
	vec3_t	abs_movedir;

	G_SetMovedir (self->s.angles, self->movedir);
	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_BSP;
	gi.setmodel (self, self->model);

	switch (self->sounds)
	{
		default:
			break;

		case 1: // water
			self->moveinfo.sound_start = gi.soundindex  ("world/mov_watr.wav");
			self->moveinfo.sound_end = gi.soundindex  ("world/stp_watr.wav");
			break;

		case 2: // lava
			self->moveinfo.sound_start = gi.soundindex  ("world/mov_watr.wav");
			self->moveinfo.sound_end = gi.soundindex  ("world/stp_watr.wav");
			break;
	}

	// calculate second position
	VectorCopy (self->s.origin, self->pos1);
	abs_movedir[0] = fabs(self->movedir[0]);
	abs_movedir[1] = fabs(self->movedir[1]);
	abs_movedir[2] = fabs(self->movedir[2]);
	self->moveinfo.distance = abs_movedir[0] * self->size[0] + abs_movedir[1] * self->size[1] + abs_movedir[2] * self->size[2] - st.lip;
	VectorMA (self->pos1, self->moveinfo.distance, self->movedir, self->pos2);

	// if it starts open, switch the positions
	if (self->spawnflags & DOOR_START_OPEN)
	{
		VectorCopy (self->pos2, self->s.origin);
		VectorCopy (self->pos1, self->pos2);
		VectorCopy (self->s.origin, self->pos1);
	}

	VectorCopy (self->pos1, self->moveinfo.start_origin);
	VectorCopy (self->s.angles, self->moveinfo.start_angles);
	VectorCopy (self->pos2, self->moveinfo.end_origin);
	VectorCopy (self->s.angles, self->moveinfo.end_angles);

	self->moveinfo.state = STATE_BOTTOM;

	if (!self->speed)
		self->speed = 25;
	self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed = self->speed;

	if (!self->wait)
		self->wait = -1;
	self->moveinfo.wait = self->wait;

	self->use = door_use;

	if (self->wait == -1)
		self->spawnflags |= DOOR_TOGGLE;

	self->classname = "func_door";

	gi.linkentity (self);
}

/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed	default 100
dmg		default	2
noise	looping sound to play when the train is in motion

*/

void Moving_Speaker_Think(edict_t *speaker)
{
	qboolean moved;
	vec3_t  offset;
	edict_t *owner;

	owner = speaker->owner;
	if(!owner)
	{
		G_FreeEdict(speaker);
		return;
	}
	if(!owner->inuse)
	{
		G_FreeEdict(speaker);
		return;
	}
	if(speaker->spawnflags & 15)
	{
		if((speaker->spawnflags & 8) && (
			(!speaker->owner->groundentity) ||      // not on the ground
			(!speaker->owner->activator)    ||      // not "activated" by anything
			(!speaker->owner->activator->client)  ) // not activated by client
			) {
			moved = false;
		} else
		{
			moved = false;
			VectorSubtract(speaker->s.origin,owner->s.origin,offset);
			if((speaker->spawnflags & 1) && (fabs(offset[0] - speaker->offset[0]) > 0.125) )
				moved = true;
			if((speaker->spawnflags & 2) && (fabs(offset[1] - speaker->offset[1]) > 0.125) )
				moved = true;
			if((speaker->spawnflags & 4) && (fabs(offset[2] - speaker->offset[2]) > 0.125) )
				moved = true;
		}
		if(moved)
			speaker->s.sound = speaker->owner->noise_index;
		else
			speaker->s.sound = 0;
	}
	else
		speaker->s.sound = speaker->owner->noise_index;

	VectorAdd(owner->s.origin,speaker->offset,speaker->s.origin);
	speaker->nextthink = level.time + FRAMETIME;
	gi.linkentity(speaker);
}

void train_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t	*e, *next;

	if(self->deathtarget)
	{
		self->target = self->deathtarget;
		G_UseTargets (self, attacker);
	}
/*	e = self->movewith_next;
	while(e) {
		next = e->movewith_next;
		if(e->solid == SOLID_NOT) {
			e->nextthink = 0;
			G_FreeEdict(e);
		} else
			BecomeExplosion1 (e);
		e = next;
	} */
	BecomeExplosion1 (self);
}

void train_next (edict_t *self);

void train_blocked (edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client) )
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other)
		{
			// DWH: Some of our ents don't have origin near the model
			vec3_t save;
			VectorCopy(other->s.origin,save);
			VectorMA (other->absmin, 0.5, other->size, other->s.origin);
			// end DWH
			BecomeExplosion1 (other);
		}
		return;
	}

	if (level.time < self->touch_debounce_time)
		return;

	if (!self->dmg)
		return;
	self->touch_debounce_time = level.time + 0.5;
	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void train_wait (edict_t *self)
{
	if (self->target_ent->pathtarget)
	{
		char	*savetarget;
		edict_t	*ent;

		ent = self->target_ent;
		savetarget = ent->target;
		ent->target = ent->pathtarget;
		G_UseTargets (ent, self->activator);
		ent->target = savetarget;

		// make sure we didn't get killed by a killtarget
		if (!self->inuse)
			return;
	}

	// DWH
	if(self->target_ent) {
		if(self->target_ent->speed) {
			self->speed = self->target_ent->speed;
			self->moveinfo.speed = self->speed;
			self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;
		}
		if(self->spawnflags & TRAIN_ROTATE) {
			if(self->target_ent->pitch_speed)
				self->pitch_speed = self->target_ent->pitch_speed;
			if(self->target_ent->yaw_speed)
				self->yaw_speed   = self->target_ent->yaw_speed;
			if(self->target_ent->roll_speed)
				self->roll_speed  = self->target_ent->roll_speed;
		} else if(self->spawnflags & TRAIN_ROTATE_CONSTANT) {
			if(self->target_ent->pitch_speed)
				self->pitch_speed += self->target_ent->pitch_speed;
			if(self->target_ent->yaw_speed)
				self->yaw_speed   += self->target_ent->yaw_speed;
			if(self->target_ent->roll_speed)
				self->roll_speed  += self->target_ent->roll_speed;
		}
	}
	// end DWH

	if (self->moveinfo.wait)
	{
		if (self->moveinfo.wait > 0)
		{
			// DWH
			self->s.effects &= ~(EF_ANIM_ALL | EF_ANIM_ALLFAST);
			// end DWH
			self->nextthink = level.time + self->moveinfo.wait;
			self->think = train_next;
		}
		else if (self->spawnflags & TRAIN_TOGGLE)  // && wait < 0
		{
			train_next (self);
			self->spawnflags &= ~TRAIN_START_ON;
			VectorClear (self->velocity);
			// DWH
			self->s.effects &= ~(EF_ANIM_ALL | EF_ANIM_ALLFAST);
			// end DWH
			self->nextthink = 0;
		}

		if (!(self->flags & FL_TEAMSLAVE))
		{
			if (self->moveinfo.sound_end)
				gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_STATIC, 0);
			self->s.sound = 0;
		}
	}
	else
	{
		train_next (self);
	}
	
}

// Rroff's rotating train stuff, with quite a few changes

void train_yaw (edict_t *self)
{
	float	cur_yaw, idl_yaw, cur_pitch, idl_pitch, cur_roll, idl_roll;
	float	yaw_vel, pitch_vel, roll_vel;
	float	Dist_1, Dist_2, Distance;

	if(!self->enemy || !self->enemy->inuse)
		return;

	cur_yaw   = self->enemy->s.angles[YAW];
	idl_yaw   = self->enemy->ideal_yaw;
	cur_pitch = self->enemy->s.angles[PITCH];
	idl_pitch = self->enemy->ideal_pitch;
	cur_roll  = self->enemy->s.angles[ROLL];
	idl_roll  = self->enemy->ideal_roll;

//	gi.dprintf("current angles=%g %g %g, ideal angles=%g %g %g\n",
//		cur_pitch,cur_yaw,cur_roll,idl_pitch,idl_yaw,idl_roll);

	yaw_vel   = self->enemy->yaw_speed;
	pitch_vel = self->enemy->pitch_speed;
	roll_vel  = self->enemy->roll_speed;

	if (cur_yaw == idl_yaw)
		self->enemy->avelocity[YAW] = 0;
	if (cur_pitch == idl_pitch)
		self->enemy->avelocity[PITCH] = 0;
	if (cur_roll == idl_roll)
		self->enemy->avelocity[ROLL] = 0;
	if ((cur_yaw == idl_yaw) && (cur_pitch == idl_pitch) && (cur_roll == idl_roll) ) {
		self->nextthink = level.time + FRAMETIME;
//		if(self->enemy->movewith_next) set_child_movement(self->enemy);
		return;
	} 

	if (cur_yaw != idl_yaw) {
		if (cur_yaw < idl_yaw)
		{
			Dist_1 = (idl_yaw - cur_yaw)*10;
			Dist_2 = ((360 - idl_yaw) + cur_yaw)*10;
			
			if (Dist_1 < Dist_2)
			{
				Distance = Dist_1;
				
				if (Distance < yaw_vel)
					yaw_vel = Distance;
				
				self->enemy->avelocity[YAW] = yaw_vel;
			}
			else
			{
				Distance = Dist_2;
				
				if (Distance < yaw_vel)
					yaw_vel = Distance;
				
				self->enemy->avelocity[YAW] = -yaw_vel;
			}
		}
		else
		{
			Dist_1 = (cur_yaw - idl_yaw)*10;
			Dist_2 = ((360 - cur_yaw) + idl_yaw)*10;
			
			if (Dist_1 < Dist_2)
			{
				Distance = Dist_1;
				
				if (Distance < yaw_vel)
					yaw_vel = Distance;
				
				self->enemy->avelocity[YAW] = -yaw_vel;
			}
			else
			{
				Distance = Dist_2;
				
				if (Distance < yaw_vel)
					yaw_vel = Distance;
				
				self->enemy->avelocity[YAW] = yaw_vel;
			}
		}
		
		//	gi.dprintf ("train cy: %g iy: %g ys: %g\n", cur_yaw, idl_yaw, self->enemy->avelocity[1]);
		
		if (self->enemy->s.angles[YAW] < 0)
			self->enemy->s.angles[YAW] += 360;
		
		if (self->enemy->s.angles[YAW] >= 360)
			self->enemy->s.angles[YAW] -= 360;
	}

	if (cur_pitch != idl_pitch) {

		if (cur_pitch < idl_pitch)
		{
			Dist_1 = (idl_pitch - cur_pitch)*10;
			Dist_2 = ((360 - idl_pitch) + cur_pitch)*10;
			
			if (Dist_1 < Dist_2)
			{
				Distance = Dist_1;
				
				if (Distance < pitch_vel)
					pitch_vel = Distance;
				
				self->enemy->avelocity[PITCH] = pitch_vel;
			}
			else
			{
				Distance = Dist_2;
				
				if (Distance < pitch_vel)
					pitch_vel = Distance;
				
				self->enemy->avelocity[PITCH] = -pitch_vel;
			}
		}
		else
		{
			Dist_1 = (cur_pitch - idl_pitch)*10;
			Dist_2 = ((360 - cur_pitch) + idl_pitch)*10;
			
			if (Dist_1 < Dist_2)
			{
				Distance = Dist_1;
				
				if (Distance < pitch_vel)
					pitch_vel = Distance;
				
				self->enemy->avelocity[PITCH] = -pitch_vel;
			}
			else
			{
				Distance = Dist_2;
				
				if (Distance < pitch_vel)
					pitch_vel = Distance;
				
				self->enemy->avelocity[PITCH] = pitch_vel;
			}
		}
		
		if (self->enemy->s.angles[PITCH] <  0)
			self->enemy->s.angles[PITCH] += 360;
		
		if (self->enemy->s.angles[PITCH] >= 360)
			self->enemy->s.angles[PITCH] -= 360;
	}

	if (cur_roll != idl_roll) {
		if (cur_roll < idl_roll)
		{
			Dist_1 = (idl_roll - cur_roll)*10;
			Dist_2 = ((360 - idl_roll) + cur_roll)*10;
			
			if (Dist_1 < Dist_2)
			{
				Distance = Dist_1;
				
				if (Distance < roll_vel)
					roll_vel = Distance;
				
				self->enemy->avelocity[ROLL] = roll_vel;
			}
			else
			{
				Distance = Dist_2;
				
				if (Distance < roll_vel)
					roll_vel = Distance;
				
				self->enemy->avelocity[ROLL] = -roll_vel;
			}
		}
		else
		{
			Dist_1 = (cur_roll - idl_roll)*10;
			Dist_2 = ((360 - cur_roll) + idl_roll)*10;
			
			if (Dist_1 < Dist_2)
			{
				Distance = Dist_1;
				
				if (Distance < roll_vel)
					roll_vel = Distance;
				
				self->enemy->avelocity[ROLL] = -roll_vel;
			}
			else
			{
				Distance = Dist_2;
				
				if (Distance < roll_vel)
					roll_vel = Distance;
				
				self->enemy->avelocity[ROLL] = roll_vel;
			}
		}
		
		if (self->enemy->s.angles[ROLL] < 0)
			self->enemy->s.angles[ROLL] += 360;
		
		if (self->enemy->s.angles[ROLL] >= 360)
			self->enemy->s.angles[ROLL] -= 360;
	}

//	if(self->enemy->movewith_next) set_child_movement(self->enemy);
	self->nextthink = level.time + FRAMETIME;
}

void train_next (edict_t *self)
{
	edict_t		*ent;
	vec3_t		dest;
	qboolean	first;
	vec3_t		angles,v;

	first = true;
again:
	if (!self->target)
	{
//		gi.dprintf ("train_next: no next target\n");
		self->s.sound = 0;
		return;
	}

	ent = G_PickTarget (self->target);
	if (!ent)
	{
		gi.dprintf ("train_next: bad target %s\n", self->target);
		return;
	}

	self->target = ent->target;

	// Rroff: path_corners can control train speed
	// DHW: This shouldn't go here... we're one path_corner behind.
	//      Set speed before train_next is called
	/*if(ent->speed)
		self->speed = ent->speed;
	self->moveinfo.speed = self->speed; */

	// check for a teleport path_corner
	if (ent->spawnflags & 1)
	{
		if (!first)
		{
			gi.dprintf ("connected teleport path_corners, see %s at %s\n", ent->classname, vtos(ent->s.origin));
			return;
		}
		first = false;
		VectorSubtract (ent->s.origin, self->mins, self->s.origin);
		VectorCopy (self->s.origin, self->s.old_origin);
		self->s.event = EV_OTHER_TELEPORT;
		gi.linkentity (self);
		goto again;
	}

	self->moveinfo.wait = ent->wait;
	self->target_ent = ent;

	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_start)
			gi.sound (self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
		self->s.sound = self->moveinfo.sound_middle;
	}

	VectorSubtract (ent->s.origin, self->mins, dest);
	self->moveinfo.state = STATE_TOP;
	VectorCopy (self->s.origin, self->moveinfo.start_origin);
	VectorCopy (dest, self->moveinfo.end_origin);

	// Rroff rotating
	if (self->spawnflags & TRAIN_ROTATE && !(ent->spawnflags & 2))
	{
		// DWH: No no no :-). This is measuring from the center
		//      of the func_train to the path_corner. Should
		//      be path_corner to path_corner.
		//VectorSubtract (ent->s.origin, self->s.origin, v);
		VectorAdd(self->s.origin,self->mins,v);
		VectorSubtract(ent->s.origin,v,v);
		vectoangles(v,angles);
		self->ideal_yaw = angles[YAW];
		self->ideal_pitch = angles[PITCH];
		if(self->ideal_pitch < 0) self->ideal_pitch += 360;
		self->ideal_roll = ent->roll;

		VectorClear(self->movedir);
		self->movedir[1] = 1.0;

	}
	/* DWH: We don't want to do this... this would give an
	//      instantaneous change in pitch and roll and look
	//      pretty goofy. Instead we'll set the new ideal_pitch
	//      to the path_corner's angles[PITCH], and move to that
	//      angle at pitch_speed. Roll changes? Can't do it (yet)
	if (ent->count)
		if (ent->count >= 0 && ent->count <= 360)
			self->s.angles[PITCH] = ent->count;

	if (ent->sounds)
		if (ent->sounds >= 0 && ent->sounds <= 360)
			self->s.angles[ROLL] = ent->sounds;
	*/
	// end Rroff

	// DWH
	if(self->spawnflags & TRAIN_ROTATE_CONSTANT) {
		self->avelocity[PITCH] = self->pitch_speed;
		self->avelocity[YAW]   = self->yaw_speed;
		self->avelocity[ROLL]  = self->roll_speed;
	}
	// end DWH

	Move_Calc (self, dest, train_wait);
	self->spawnflags |= TRAIN_START_ON;
}

void train_resume (edict_t *self)
{
	edict_t	*ent;
	vec3_t	dest;

	ent = self->target_ent;

	VectorSubtract (ent->s.origin, self->mins, dest);
	self->moveinfo.state = STATE_TOP;
	VectorCopy (self->s.origin, self->moveinfo.start_origin);
	VectorCopy (dest, self->moveinfo.end_origin);
	Move_Calc (self, dest, train_wait);
	self->spawnflags |= TRAIN_START_ON;
	// DWH
	if(self->spawnflags & TRAIN_ROTATE_CONSTANT) {
		self->avelocity[PITCH] = self->pitch_speed;
		self->avelocity[YAW]   = self->yaw_speed;
		self->avelocity[ROLL]  = self->roll_speed;
	}
	// end DWH
}

void func_train_find (edict_t *self)
{
	edict_t *ent;

	if (!self->target)
	{
		gi.dprintf ("train_find: no target\n");
		return;
	}
	ent = G_PickTarget (self->target);
	if (!ent)
	{
		gi.dprintf ("train_find: target %s not found\n", self->target);
		return;
	}

	if(ent->speed) {
		self->speed = ent->speed;
		self->moveinfo.speed = self->speed;
		self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;
	}
	if(ent->pitch_speed)
		self->pitch_speed = ent->pitch_speed;
	if(ent->yaw_speed)
		self->yaw_speed   = ent->yaw_speed;
	if(ent->roll_speed)
		self->roll_speed  = ent->roll_speed;

	self->target = ent->target;

	if( self->spawnflags & TRAIN_ROTATE )
	{
		ent->think = train_yaw;
		ent->nextthink = level.time + FRAMETIME;
	}
//	else
//	{
//		ent->think = train_children_think;
//		ent->nextthink = level.time + FRAMETIME;
//	}
	ent->enemy = self;

//	movewith_init(self);
	VectorSubtract (ent->s.origin, self->mins, self->s.origin);
	gi.linkentity (self);

	// if not triggered, start immediately
	if (!self->targetname)
		self->spawnflags |= TRAIN_START_ON;

	if (self->spawnflags & TRAIN_START_ON)
	{
		// DWH
		if (self->spawnflags & TRAIN_ANIMATE)
			self->s.effects |= EF_ANIM_ALL;
		else if (self->spawnflags & TRAIN_ANIMATE_FAST)
			self->s.effects |= EF_ANIM_ALLFAST;
		// end DWH
		self->nextthink = level.time + FRAMETIME;
		self->think = train_next;
		self->activator = self;
	}
}

void train_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;

	if (self->spawnflags & TRAIN_START_ON)
	{
		if (!(self->spawnflags & TRAIN_TOGGLE))
			return;
		self->spawnflags &= ~TRAIN_START_ON;
		VectorClear (self->velocity);
		VectorClear (self->avelocity);
		self->s.effects &= ~(EF_ANIM_ALL | EF_ANIM_ALLFAST);
		self->nextthink = 0;
	}
	else
	{
		if (self->spawnflags & TRAIN_ANIMATE)
			self->s.effects |= EF_ANIM_ALL;
		else if (self->spawnflags & TRAIN_ANIMATE_FAST)
			self->s.effects |= EF_ANIM_ALLFAST;

		if (self->target_ent)
			train_resume(self);
		else
			train_next(self);
	}
}

void SP_func_train (edict_t *self)
{
	if( (self->spawnflags & TRAIN_ROTATE_MASK) == TRAIN_ROTATE_MASK) {
		gi.dprintf("%s has ROTATE and ROTATE_CONSTANT flags set.\n",
			self->classname);
		G_FreeEdict(self);
		return;
	}

	self->movetype = MOVETYPE_PUSH;

	VectorClear (self->s.angles);
	self->blocked = train_blocked;
	if (self->spawnflags & TRAIN_BLOCK_STOPS)
		self->dmg = 0;
	else
	{
		if (!self->dmg)
			self->dmg = 100;
	}
	self->solid = SOLID_BSP;
	gi.setmodel (self, self->model);

	if (st.noise)
		self->moveinfo.sound_middle = gi.soundindex  (st.noise);

	if (!self->speed)
		self->speed = 100;

	// DWH - Do NOT set default values for rotational speeds - if they're 0, then they're 0.

	self->moveinfo.speed = self->speed;
	self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;

	self->use = train_use;

	// DWH: damageable
	if (self->health) {
		self->die = train_die;
		self->takedamage = DAMAGE_YES;
	} else {
		self->die = NULL;
		self->takedamage = DAMAGE_NO;
	}
	// end DWH

	gi.linkentity (self);
	if (self->target)
	{
		// start trains on the second frame, to make sure their targets have had
		// a chance to spawn
		self->nextthink = level.time + FRAMETIME;
		self->think = func_train_find;
	}
	else
	{
		gi.dprintf ("func_train without a target at %s\n", vtos(self->absmin));
	}

	// DWH - TRAIN_SMOOTH forces trains to go directly to Move_Done from
	//       Move_Final rather than slowing down (if necessary) for one
	//       frame.
	if (self->spawnflags & TRAIN_SMOOTH)
		self->smooth_movement = true;
	else
		self->smooth_movement = false;

	// DWH - make noise field work w/o origin brush
	// ver. 1.3 change - do this for ALL trains
//	if(st.noise && !VectorLength(self->s.origin) )
	if(st.noise)
	{
		edict_t *speaker;

		self->noise_index    = self->moveinfo.sound_middle;
		self->moveinfo.sound_middle = 0;
		speaker = G_Spawn();
		speaker->classname   = "moving_speaker";
		speaker->s.sound     = 0;
		speaker->volume      = 1;
		speaker->attenuation = 1;
		speaker->owner       = self;
		speaker->think       = Moving_Speaker_Think;
		speaker->nextthink   = level.time + 2*FRAMETIME;
		speaker->spawnflags  = 7;       // owner must be moving to play
		self->speaker        = speaker;
		if(VectorLength(self->s.origin))
			VectorCopy(self->s.origin,speaker->s.origin);
		else {
			VectorAdd(self->absmin,self->absmax,speaker->s.origin);
			VectorScale(speaker->s.origin,0.5,speaker->s.origin);
		}
		VectorSubtract(speaker->s.origin,self->s.origin,speaker->offset);
	}

}

/*void SP_func_train (edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;

	VectorClear (self->s.angles);
	self->blocked = train_blocked;
	if (self->spawnflags & TRAIN_BLOCK_STOPS)
		self->dmg = 0;
	else
	{
		if (!self->dmg)
			self->dmg = 100;
	}
	self->solid = SOLID_BSP;
	gi.setmodel (self, self->model);

	if (st.noise)
		self->moveinfo.sound_middle = gi.soundindex  (st.noise);

	if (!self->speed)
		self->speed = 100;

	self->moveinfo.speed = self->speed;
	self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;

	self->use = train_use;

	gi.linkentity (self);

	if (self->target)
	{
		// start trains on the second frame, to make sure their targets have had
		// a chance to spawn
		self->nextthink = level.time + FRAMETIME;
		self->think = func_train_find;
	}
	else
	{
		gi.dprintf ("func_train without a target at %s\n", vtos(self->absmin));
	}
}*/


/*QUAKED trigger_elevator (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
*/
void trigger_elevator_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *target;

	if (self->movetarget->nextthink)
	{
//		gi.dprintf("elevator busy\n");
		return;
	}

	if (!other->pathtarget)
	{
		gi.dprintf("elevator used with no pathtarget\n");
		return;
	}

	target = G_PickTarget (other->pathtarget);
	if (!target)
	{
		gi.dprintf("elevator used with bad pathtarget: %s\n", other->pathtarget);
		return;
	}

	self->movetarget->target_ent = target;
	train_resume (self->movetarget);
}

void trigger_elevator_init (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("trigger_elevator has no target\n");
		return;
	}
	self->movetarget = G_PickTarget (self->target);
	if (!self->movetarget)
	{
		gi.dprintf("trigger_elevator unable to find target %s\n", self->target);
		return;
	}
	if (strcmp(self->movetarget->classname, "func_train") != 0)
	{
		gi.dprintf("trigger_elevator target %s is not a train\n", self->target);
		return;
	}

	self->use = trigger_elevator_use;
	self->svflags = SVF_NOCLIENT;

}

void SP_trigger_elevator (edict_t *self)
{
	self->think = trigger_elevator_init;
	self->nextthink = level.time + FRAMETIME;
}


/*QUAKED func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
"wait"			base time between triggering all targets, default is 1
"random"		wait variance, default is 0

so, the basic time between firing is a random time between
(wait - random) and (wait + random)

"delay"			delay before first firing when turned on, default is 0

"pausetime"		additional delay used only the very first time
				and only if spawned with START_ON

These can used but not touched.
*/
void func_timer_think (edict_t *self)
{
	G_UseTargets (self, self->activator);
	self->nextthink = level.time + self->wait + crandom() * self->random;
}

void func_timer_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;

	// if on, turn it off
	if (self->nextthink)
	{
		self->nextthink = 0;
		return;
	}

	// turn it on
	if (self->delay)
		self->nextthink = level.time + self->delay;
	else
		func_timer_think (self);
}

void SP_func_timer (edict_t *self)
{
	if (!self->wait)
		self->wait = 1.0;

	self->use = func_timer_use;
	self->think = func_timer_think;

	if (self->random >= self->wait)
	{
		self->random = self->wait - FRAMETIME;
		gi.dprintf("func_timer at %s has random >= wait\n", vtos(self->s.origin));
	}

	if (self->spawnflags & 1)
	{
		self->nextthink = level.time + 1.0 + st.pausetime + self->delay + self->wait + crandom() * self->random;
		self->activator = self;
	}

	self->svflags = SVF_NOCLIENT;
}


/*QUAKED func_conveyor (0 .5 .8) ? START_ON TOGGLE
Conveyors are stationary brushes that move what's on them.
The brush should be have a surface with at least one current content enabled.
speed	default 100
*/

void func_conveyor_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & 1)
	{
		self->speed = 0;
		self->spawnflags &= ~1;
	}
	else
	{
		self->speed = self->count;
		self->spawnflags |= 1;
	}

	if (!(self->spawnflags & 2))
		self->count = 0;
}

void SP_func_conveyor (edict_t *self)
{
	if (!self->speed)
		self->speed = 100;

	if (!(self->spawnflags & 1))
	{
		self->count = self->speed;
		self->speed = 0;
	}

	self->use = func_conveyor_use;

	gi.setmodel (self, self->model);
	self->solid = SOLID_BSP;
	gi.linkentity (self);
}


/*QUAKED func_door_secret (0 .5 .8) ? always_shoot 1st_left 1st_down
A secret door.  Slide back and then to the side.

open_once		doors never closes
1st_left		1st move is left of arrow
1st_down		1st move is down from arrow
always_shoot	door is shootebale even if targeted

"angle"		determines the direction
"dmg"		damage to inflic when blocked (default 2)
"wait"		how long to hold in the open position (default 5, -1 means hold)
*/

#define SECRET_ALWAYS_SHOOT	1
#define SECRET_1ST_LEFT		2
#define SECRET_1ST_DOWN		4

void door_secret_move1 (edict_t *self);
void door_secret_move2 (edict_t *self);
void door_secret_move3 (edict_t *self);
void door_secret_move4 (edict_t *self);
void door_secret_move5 (edict_t *self);
void door_secret_move6 (edict_t *self);
void door_secret_done (edict_t *self);

void door_secret_use (edict_t *self, edict_t *other, edict_t *activator)
{
	// make sure we're not already moving
	if (!VectorCompare(self->s.origin, vec3_origin))
		return;

	Move_Calc (self, self->pos1, door_secret_move1);
	door_use_areaportals (self, true);
}

void door_secret_move1 (edict_t *self)
{
	self->nextthink = level.time + 1.0;
	self->think = door_secret_move2;
}

void door_secret_move2 (edict_t *self)
{
	Move_Calc (self, self->pos2, door_secret_move3);
}

void door_secret_move3 (edict_t *self)
{
	if (self->wait == -1)
		return;
	self->nextthink = level.time + self->wait;
	self->think = door_secret_move4;
}

void door_secret_move4 (edict_t *self)
{
	Move_Calc (self, self->pos1, door_secret_move5);
}

void door_secret_move5 (edict_t *self)
{
	self->nextthink = level.time + 1.0;
	self->think = door_secret_move6;
}

void door_secret_move6 (edict_t *self)
{
	Move_Calc (self, vec3_origin, door_secret_done);
}

void door_secret_done (edict_t *self)
{
	if (!(self->targetname) || (self->spawnflags & SECRET_ALWAYS_SHOOT))
	{
		self->health = 0;
		self->takedamage = DAMAGE_YES;
	}
	door_use_areaportals (self, false);
}

void door_secret_blocked  (edict_t *self, edict_t *other)
{
	if (!(other->svflags & SVF_MONSTER) && (!other->client) )
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other)
			BecomeExplosion1 (other);
		return;
	}

	if (level.time < self->touch_debounce_time)
		return;
	self->touch_debounce_time = level.time + 0.5;

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void door_secret_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	door_secret_use (self, attacker, attacker);
}

void SP_func_door_secret (edict_t *ent)
{
	vec3_t	forward, right, up;
	float	side;
	float	width;
	float	length;

	ent->moveinfo.sound_start = gi.soundindex  ("doors/dr1_strt.wav");
	ent->moveinfo.sound_middle = gi.soundindex  ("doors/dr1_mid.wav");
	ent->moveinfo.sound_end = gi.soundindex  ("doors/dr1_end.wav");

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	gi.setmodel (ent, ent->model);

	ent->blocked = door_secret_blocked;
	ent->use = door_secret_use;

	if (!(ent->targetname) || (ent->spawnflags & SECRET_ALWAYS_SHOOT))
	{
		ent->health = 0;
		ent->takedamage = DAMAGE_YES;
		ent->die = door_secret_die;
	}

	if (!ent->dmg)
		ent->dmg = 2;

	if (!ent->wait)
		ent->wait = 5;

	ent->moveinfo.accel =
	ent->moveinfo.decel =
	ent->moveinfo.speed = 50;

	// calculate positions
	AngleVectors (ent->s.angles, forward, right, up);
	VectorClear (ent->s.angles);
	side = 1.0 - (ent->spawnflags & SECRET_1ST_LEFT);
	if (ent->spawnflags & SECRET_1ST_DOWN)
		width = fabs(DotProduct(up, ent->size));
	else
		width = fabs(DotProduct(right, ent->size));
	length = fabs(DotProduct(forward, ent->size));
	if (ent->spawnflags & SECRET_1ST_DOWN)
		VectorMA (ent->s.origin, -1 * width, up, ent->pos1);
	else
		VectorMA (ent->s.origin, side * width, right, ent->pos1);
	VectorMA (ent->pos1, length, forward, ent->pos2);

	if (ent->health)
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = door_killed;
		ent->max_health = ent->health;
	}
	else if (ent->targetname && ent->message)
	{
		gi.soundindex ("misc/talk.wav");
		ent->touch = door_touch;
	}
	
	ent->classname = "func_door";

	gi.linkentity (ent);
}


/*QUAKED func_killbox (1 0 0) ?
Kills everything inside when fired, irrespective of protection.
*/
void use_killbox (edict_t *self, edict_t *other, edict_t *activator)
{
	KillBox (self);
}

void SP_func_killbox (edict_t *ent)
{
	gi.setmodel (ent, ent->model);
	ent->use = use_killbox;
	ent->svflags = SVF_NOCLIENT;
}


/*============================================
QUAKED effect_lightning (1 .5 0) (-32 -32 -16) (32 32 32)
This is an entity totally controlled by the mapper.

"health"		Number of "bends" in the bolt
"count"		Movement of the bend
"angle"		Direction of the lightning
============================================*/

void effect_lightning_strike (edict_t *self, vec3_t org, vec3_t dir)
{
	vec3_t	start, end, vec;
	vec3_t	right, up;
	float		len, blen, half;
	int		i;
	trace_t	tr;

	VectorMA(org, 8192, dir, end);

	AngleVectors(dir, NULL, right, up);

	tr = gi.trace(org, NULL, NULL, end, self, CONTENTS_SOLID);

	VectorSubtract(org, tr.endpos, vec);
	len = VectorLength(vec);
	blen = len/self->health;

	half = self->count/2;

	VectorCopy(org, start);

	for(i=0; i<self->health-1; i++)
	{
		VectorMA(start, blen, dir, end);
		if(dir[2] != 0)
		{
			end[0] += (random()*self->count)-half;
			end[1] += (random()*self->count)-half;
		}
		else
		{
			VectorMA(end, (random()*self->count)-half, right, end);
			VectorMA(end, (random()*self->count)-half, up, end);
		}
		spawn_templaser(start, end);
		if(gi.pointcontents(end) == CONTENTS_SOLID)
			break;
		VectorCopy(end, start);
	}
	spawn_templaser(end, tr.endpos);
}

void effect_lightning_think (edict_t *self)
{
	effect_lightning_strike (self, self->s.origin, self->s.angles);
	self->nextthink = level.time + 0.2;
}

void SP_effect_lightning (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->think = effect_lightning_think;
	ent->nextthink = level.time + 0.2;

	if(!ent->health)
		ent->health = 8;
	if(!ent->count)
		ent->count = 16;

	if(ent->s.angles[1] == -1)
		VectorSet(ent->s.angles, 0, 0, 1);
	else if(ent->s.angles[1] == -2)
		VectorSet(ent->s.angles, 0, 0, -1);
	else if(ent->s.angles[1] == 0)
		VectorSet(ent->s.angles, 1, 0, 0);
	else if(ent->s.angles[1] <= 45)
		VectorSet(ent->s.angles, 1, 1, 0);
	else if(ent->s.angles[1] <= 90)
		VectorSet(ent->s.angles, 0, 1, 0);
	else if(ent->s.angles[1] <= 135)
		VectorSet(ent->s.angles, -1, 1, 0);
	else if(ent->s.angles[1] <= 180)
		VectorSet(ent->s.angles, -1, 0, 0);
	else if(ent->s.angles[1] <= 225)
		VectorSet(ent->s.angles, -1, -1, 0);
	else if(ent->s.angles[1] <= 270)
		VectorSet(ent->s.angles, 0, -1, 0);
	else if(ent->s.angles[1] <= 315)
		VectorSet(ent->s.angles, 1, -1, 0);
	else
		VectorSet(ent->s.angles, 0, 0, -1);

	gi.linkentity (ent);
}

/*============================================
QUAKED func_computer
This is an entity totally controlled by the mapper.

"message"		The path to the script file to use.
============================================*/
void Use_Computer (edict_t *ent, edict_t *other, edict_t *activator)
{
	Computer_Open (ent, activator);
}

void SP_func_computer (edict_t *ent)
{
	ent->solid = SOLID_NOT;

	ent->use = Use_Computer;
}

/*============================================
QUAKED func_coolgrav
This is an entity totally controlled by the mapper.

"gravity"		Gravity'
"sound"			Pitch
"angle"			Yaw
"speed"			Roll
"health"		Turn on or off
============================================*/

void SP_func_coolgrav (edict_t *ent)
{
	ent->solid = SOLID_NOT;

	ent->use = CoolGrav;
}

void CoolGrav (edict_t *ent, edict_t *other, edict_t *activator)
{
	gi.dprintf("pickle\n");
	if(ent->health == 5)
		activator->client->state_edit = true;
	else
	{
		activator->client->state_edit = false;
		return;
	}

	if(ent->sounds || ent->angle || ent->speed)
	{
		activator->client->wanted_vec[0] = ent->sounds;
		activator->client->wanted_vec[1] = ent->angle;
		activator->client->wanted_vec[2] = ent->speed;
	}

	if(ent->gravity)
		activator->client->true_grav = ent->gravity;

	VectorCopy(activator->client->v_angle, activator->client->true_vec);
}

/*============================================
QUAKED func_fountain
This is an entity totally controlled by the mapper.

"message"		sprite
"sounds"		Yaw
"angle"			Pitch
"speed"			Speed
"health"		Movetype
"style"			EF_ number
"count"			RF_ number
"dmg"			Vertical Randomness
"accel"			rand
"target"		lifespan
"decel"			gravity
"wait"			amount
============================================*/

void SP_SpriteFountain (edict_t *ent)
{
	ent->solid = SOLID_NOT;
	ent->movetype = MOVETYPE_NONE;

	givemequotes(ent->message);

//	ent->use = za_apa;

	ent->think = za_apa;
	ent->nextthink = level.time + 0.2;
}

void za_apa (edict_t *ent)
{
	edict_t	*fountain;

	fountain = G_Spawn();

	fountain->message = ent->message;
	fountain->gravity = ent->gravity;
	fountain->sounds = ent->sounds;
	fountain->angle = ent->s.angles[1];
	fountain->speed = ent->speed;
	fountain->health = ent->health;
	fountain->wait = ent->wait;
	fountain->count = ent->count;
	fountain->dmg = ent->dmg;
	fountain->accel = ent->accel;
	fountain->decel = ent->decel;
	fountain->target = ent->target;
	fountain->style = ent->style;
	fountain->solid = SOLID_NOT;
	
	VectorCopy(ent->s.origin, fountain->s.origin);

	fountain->think = spawn_sprite;

	fountain->gib_health = atoi(ent->target);

	fountain->nextthink = level.time + 0.1;

	gi.linkentity (fountain);

//	gi.dprintf("goog\n");

	ent->nextthink = level.time + fountain->wait;
}

void spawn_sprite (edict_t *self)
{
//	edict_t	*sprite;
	vec3_t	dir;
	vec3_t	forward, right, up, temp;

//	vectoangles (aimdir, dir);
	dir[0] = self->sounds;
	dir[1] = self->angle;
	dir[2] = 0;
//	if(self->accel)
//	{
		//dir[0] = (((dir[0]+1) * (self->accel * random()))-1);
		//dir[1] = (((dir[1]+1) * (self->accel * random()))-1);
//	}
	if(self->accel)
	{
		dir[0] = dir[0] + (self->dmg * (self->accel * crandom()));
		dir[1] = dir[1] + (self->accel * crandom());
	}
	AngleVectors (dir, forward, right, up);

		VectorScale (forward, self->speed, self->velocity);
		VectorScale (up, self->speed, temp);
		_VectorAdd(self->velocity, temp, self->velocity);
		switch (self->health)
		{
			case MOVETYPE_NONE:
				self->movetype = MOVETYPE_NONE;
				break;

			case MOVETYPE_NOCLIP:
				self->movetype = MOVETYPE_NOCLIP;
				break;

			case MOVETYPE_PUSH:
				self->movetype = MOVETYPE_PUSH;		// no clip to world, push on box contact
				break;

			case MOVETYPE_STOP:
				self->movetype = MOVETYPE_STOP;		// no clip to world, stops on box contact
				break;

			case MOVETYPE_WALK:
				self->movetype = MOVETYPE_WALK;		// gravity
				break;

			case MOVETYPE_STEP:
				self->movetype = MOVETYPE_STEP;		// gravity, special edge handling
				break;

			case MOVETYPE_FLY:
				self->movetype = MOVETYPE_FLY;
				break;

			case MOVETYPE_TOSS:
				self->movetype = MOVETYPE_TOSS;		// gravity
				break;

			case MOVETYPE_FLYMISSILE:
				self->movetype = MOVETYPE_FLYMISSILE;	// extra size to monsters
				break;

			case MOVETYPE_BOUNCE:
				self->movetype = MOVETYPE_BOUNCE;
				break;

			case MOVETYPE_FLOAT:
				self->movetype = MOVETYPE_FLOAT;
				break;

			case MOVETYPE_GRAVBULLET:
				self->movetype = MOVETYPE_GRAVBULLET;
				break;

			case MOVETYPE_COOLGRAV:
				self->movetype = MOVETYPE_COOLGRAV;	//Cool Grav Hack =)
				break;

			default:
				break;
		}
		self->clipmask = MASK_ALL;
/*		switch (self->dmg)
		{
			case 0:
				self->clipmask = MASK_ALL;
				break;

			case 1:
				self->clipmask = MASK_SOLID;
				break;

			case 2:
				self->clipmask = MASK_PLAYERSOLID;
				break;

			case 3:
				self->clipmask = MASK_DEADSOLID;
				break;

			case 4:
				self->clipmask = MASK_MONSTERSOLID;
				break;

			case 5:
				self->clipmask = MASK_WATER;
				break;

			case 6:
				self->clipmask = MASK_OPAQUE;
				break;

			case 7:
				self->clipmask = MASK_SHOT;
				break;

			case 8:
				self->clipmask = MASK_FORCE;
				break;

			case 9:
				self->clipmask = MASK_CURRENT;
				break;

			default:
				break;
		}*/
		self->solid = SOLID_NOT;
		self->s.renderfx = self->count;
		VectorClear (self->mins);
		VectorClear (self->maxs);
		self->s.modelindex = gi.modelindex ("sprites/s_fog2.sp2");

		switch (self->style)
		{
			case 0:
				self->s.effects = 0;
				break;

			case 1:
				self->s.effects |= EF_SPHERETRANS;
//				self->touch = bouncebaby;
				if((random() < 0.25))
					self->s.frame = 1;
				else if((random() < 0.33))
					self->s.frame = 2;
				else if((random() < 0.50))
					self->s.frame = 3;
				else
					self->s.frame = 4;
				break;

			default:
				break;
		}

//		self->s.modelindex = gi.modelindex (self->message);
//		gi.dprintf("%s\n", self->message);
		self->owner = self;
		self->nextthink = level.time + self->gib_health;
		self->think = G_FreeEdict;
//		grenade->dmg = damage;
//		grenade->dmg_radius = damage_radius;
		self->classname = "sprite";
		self->gravity = self->decel;
		self->awf = 1;

//		self->nextthink = level.time + 3;
//		self->think = G_FreeEdict;

//		grenade->s.sound = gi.soundindex("misc/comp_up.wav");

//		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
//		gi.linkentity (sprite);

//	self->nextthink = level.time + 0.01;
}

void bouncebaby (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->classname == "sprite")
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}


	ClipVelocity (ent->velocity, plane->normal, ent->velocity, 1.2);
}