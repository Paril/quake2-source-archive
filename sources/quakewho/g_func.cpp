/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "g_local.h"

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

constexpr int32_t PLAT_LOW_TRIGGER		= bit(0);

constexpr int32_t DOOR_START_OPEN		= bit(0);
constexpr int32_t DOOR_REVERSE			= bit(1);
constexpr int32_t DOOR_CRUSHER			= bit(2);
constexpr int32_t DOOR_NOMONSTER		= bit(3);
constexpr int32_t DOOR_TOGGLE			= bit(5);
constexpr int32_t DOOR_X_AXIS			= bit(6);
constexpr int32_t DOOR_Y_AXIS			= bit(7);


//
// Support routines for movement (changes in origin using velocity)
//

static void Move_Done (edict_t &ent)
{
	ent.velocity.Clear();
	ent.moveinfo.endfunc (ent);
}

static void Move_Final (edict_t &ent)
{
	if (ent.moveinfo.remaining_distance == 0)
	{
		Move_Done (ent);
		return;
	}

	ent.velocity = ent.moveinfo.dir * (ent.moveinfo.remaining_distance / FRAME_S);

	ent.think = Move_Done;
	ent.nextthink = level.time + FRAME_MS;
}

static void Move_Begin (edict_t &ent)
{
	if ((ent.moveinfo.speed * FRAME_S) >= ent.moveinfo.remaining_distance)
	{
		Move_Final (ent);
		return;
	}

	ent.velocity = ent.moveinfo.dir * ent.moveinfo.speed;
	const vec_t frames = floor((ent.moveinfo.remaining_distance / ent.moveinfo.speed) / FRAME_S);
	ent.moveinfo.remaining_distance -= frames * ent.moveinfo.speed * FRAME_S;
	ent.nextthink = level.time + (frames * FRAME_MS);
	ent.think = Move_Final;
}

static void Think_AccelMove (edict_t &ent);

static void Move_Calc (edict_t &ent, const vec3_t &dest, void(*func)(edict_t&))
{
	ent.velocity.Clear();
	ent.moveinfo.dir = dest - ent.s.origin;
	ent.moveinfo.remaining_distance = ent.moveinfo.dir.Normalize();
	ent.moveinfo.endfunc = func;

	if (ent.moveinfo.speed == ent.moveinfo.accel && ent.moveinfo.speed == ent.moveinfo.decel)
	{
		if (level.current_entity == ((ent.flags & FL_TEAMSLAVE) ? static_cast<edict_t &>(ent.teammaster) : ent))
			Move_Begin (ent);
		else
		{
			ent.nextthink = level.time + FRAME_MS;
			ent.think = Move_Begin;
		}
	}
	else
	{
		// accelerative
		ent.moveinfo.current_speed = 0;
		ent.think = Think_AccelMove;
		ent.nextthink = level.time + FRAME_MS;
	}
}


//
// Support routines for angular movement (changes in angle using avelocity)
//

static void AngleMove_Done (edict_t &ent)
{
	ent.avelocity.Clear();
	ent.moveinfo.endfunc (ent);
}

static void AngleMove_Final (edict_t &ent)
{
	vec3_t move;

	if (ent.moveinfo.state == STATE_UP)
		move = ent.moveinfo.end_angles - ent.s.angles;
	else
		move = ent.moveinfo.start_angles - ent.s.angles;

	if (move == vec3_origin)
	{
		AngleMove_Done (ent);
		return;
	}

	ent.avelocity = move * (1.0f / FRAME_S);
	ent.think = AngleMove_Done;
	ent.nextthink = level.time + FRAME_MS;
}

static void AngleMove_Begin (edict_t &ent)
{
	vec3_t destdelta;

	// set destdelta to the vector needed to move
	if (ent.moveinfo.state == STATE_UP)
		destdelta = ent.moveinfo.end_angles - ent.s.angles;
	else
		destdelta = ent.moveinfo.start_angles - ent.s.angles;
	
	// calculate length of vector
	const vec_t len = destdelta.Length();
	
	// divide by speed to get time to reach dest
	const vec_t traveltime = len / ent.moveinfo.speed;

	if (traveltime < FRAME_S)
	{
		AngleMove_Final (ent);
		return;
	}

	const vec_t frames = floor(traveltime / FRAME_S);

	// scale the destdelta vector by the time spent traveling to get velocity
	ent.avelocity = destdelta * (1.0f / traveltime);

	// set nextthink to trigger a think when dest is reached
	ent.nextthink = level.time + frames * FRAME_S;
	ent.think = AngleMove_Final;
}

static void AngleMove_Calc (edict_t &ent, void(*func)(edict_t&))
{
	ent.avelocity.Clear();
	ent.moveinfo.endfunc = func;

	if (level.current_entity == ((ent.flags & FL_TEAMSLAVE) ? static_cast<edict_t &>(ent.teammaster) : ent))
		AngleMove_Begin (ent);
	else
	{
		ent.nextthink = level.time + FRAME_MS;
		ent.think = AngleMove_Begin;
	}
}


/*
==============
Think_AccelMove

The team has completed a frame of movement, so
change the speed for the next frame
==============
*/
constexpr vec_t AccelerationDistance(const vec_t &target, const vec_t &rate)
{
	return (target * ((target / rate) + 1) / 2);
}

static void plat_CalcAcceleratedMove(moveinfo_t &moveinfo)
{
	moveinfo.move_speed = moveinfo.speed;

	if (moveinfo.remaining_distance < moveinfo.accel)
	{
		moveinfo.current_speed = moveinfo.remaining_distance;
		return;
	}

	const vec_t accel_dist = AccelerationDistance (moveinfo.speed, moveinfo.accel);
	vec_t decel_dist = AccelerationDistance (moveinfo.speed, moveinfo.decel);

	if ((moveinfo.remaining_distance - accel_dist - decel_dist) < 0)
	{
		vec_t f = (moveinfo.accel + moveinfo.decel) / (moveinfo.accel * moveinfo.decel);
		moveinfo.move_speed = (-2 + sqrt(4 - 4 * f * (-2 * moveinfo.remaining_distance))) / (2 * f);
		decel_dist = AccelerationDistance (moveinfo.move_speed, moveinfo.decel);
	}

	moveinfo.decel_distance = decel_dist;
}

static void plat_Accelerate (moveinfo_t &moveinfo)
{
	// are we decelerating?
	if (moveinfo.remaining_distance <= moveinfo.decel_distance)
	{
		if (moveinfo.remaining_distance < moveinfo.decel_distance)
		{
			if (moveinfo.next_speed)
			{
				moveinfo.current_speed = moveinfo.next_speed;
				moveinfo.next_speed = 0;
				return;
			}
			if (moveinfo.current_speed > moveinfo.decel)
				moveinfo.current_speed -= moveinfo.decel;
		}
		return;
	}

	// are we at full speed and need to start decelerating during this move?
	if (moveinfo.current_speed == moveinfo.move_speed &&
		(moveinfo.remaining_distance - moveinfo.current_speed) < moveinfo.decel_distance)
	{
		const vec_t p1_distance = moveinfo.remaining_distance - moveinfo.decel_distance;
		const vec_t p2_distance = moveinfo.move_speed * (1.0f - (p1_distance / moveinfo.move_speed));
		const vec_t distance = p1_distance + p2_distance;
		moveinfo.current_speed = moveinfo.move_speed;
		moveinfo.next_speed = moveinfo.move_speed - moveinfo.decel * (p2_distance / distance);
		return;
	}

	// are we accelerating?
	if (moveinfo.current_speed >= moveinfo.speed)
		return; // we are at constant velocity (move_speed)

	const vec_t old_speed = moveinfo.current_speed;

	// figure simple acceleration up to move_speed
	moveinfo.current_speed += moveinfo.accel;
	if (moveinfo.current_speed > moveinfo.speed)
		moveinfo.current_speed = moveinfo.speed;

	// are we accelerating throughout this entire move?
	if ((moveinfo.remaining_distance - moveinfo.current_speed) >= moveinfo.decel_distance)
		return;

	// during this move we will accelrate from current_speed to move_speed
	// and cross over the decel_distance; figure the average speed for the
	// entire move
	const vec_t p1_distance = moveinfo.remaining_distance - moveinfo.decel_distance;
	const vec_t p1_speed = (old_speed + moveinfo.move_speed) / 2.0f;
	const vec_t p2_distance = moveinfo.move_speed * (1.0f - (p1_distance / p1_speed));
	const vec_t distance = p1_distance + p2_distance;

	moveinfo.current_speed = (p1_speed * (p1_distance / distance)) + (moveinfo.move_speed * (p2_distance / distance));
	moveinfo.next_speed = moveinfo.move_speed - moveinfo.decel * (p2_distance / distance);
}

static void Think_AccelMove (edict_t &ent)
{
	ent.moveinfo.remaining_distance -= ent.moveinfo.current_speed;

	if (ent.moveinfo.current_speed == 0)		// starting or blocked
		plat_CalcAcceleratedMove(ent.moveinfo);

	plat_Accelerate (ent.moveinfo);

	// will the entire move complete on next frame?
	if (ent.moveinfo.remaining_distance <= ent.moveinfo.current_speed)
	{
		Move_Final (ent);
		return;
	}

	ent.velocity = ent.moveinfo.dir * (ent.moveinfo.current_speed * 10);
	ent.nextthink = level.time + FRAME_MS;
	ent.think = Think_AccelMove;
}

static void plat_go_down (edict_t &ent);

static void plat_hit_top (edict_t &ent)
{
	if (!(ent.flags & FL_TEAMSLAVE))
	{
		if (ent.moveinfo.sound_end)
			ent.PlaySound(ent.moveinfo.sound_end, CHAN_NO_PHS_ADD | CHAN_VOICE, ATTN_STATIC);
		ent.s.sound = SOUND_NONE;
	}

	ent.moveinfo.state = STATE_TOP;

	ent.think = plat_go_down;
	ent.nextthink = level.time + 300;
}

static void plat_hit_bottom (edict_t &ent)
{
	if (!(ent.flags & FL_TEAMSLAVE))
	{
		if (ent.moveinfo.sound_end)
			ent.PlaySound(ent.moveinfo.sound_end, CHAN_NO_PHS_ADD | CHAN_VOICE, ATTN_STATIC);
		ent.s.sound = SOUND_NONE;
	}
	ent.moveinfo.state = STATE_BOTTOM;
}

static void plat_go_down (edict_t &ent)
{
	if (!(ent.flags & FL_TEAMSLAVE))
	{
		if (ent.moveinfo.sound_start)
			ent.PlaySound(ent.moveinfo.sound_start, CHAN_NO_PHS_ADD | CHAN_VOICE, ATTN_STATIC);
		ent.s.sound = ent.moveinfo.sound_middle;
	}
	ent.moveinfo.state = STATE_DOWN;
	Move_Calc (ent, ent.moveinfo.end_origin, plat_hit_bottom);
}

static void plat_go_up (edict_t &ent)
{
	if (!(ent.flags & FL_TEAMSLAVE))
	{
		if (ent.moveinfo.sound_start)
			ent.PlaySound(ent.moveinfo.sound_start, CHAN_NO_PHS_ADD | CHAN_VOICE, ATTN_STATIC);
		ent.s.sound = ent.moveinfo.sound_middle;
	}
	ent.moveinfo.state = STATE_UP;
	Move_Calc (ent, ent.moveinfo.start_origin, plat_hit_top);
}

static void plat_blocked (edict_t &self, edict_t &other)
{
	if (!(other.svflags & SVF_MONSTER) && !other.client)
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other.s.origin, vec3_origin, 100000, 1, DAMAGE_NONE);
		// if it's still there, nuke it
		if (other.inuse)
			BecomeExplosion1 (other);
		return;
	}

	T_Damage (other, self, self, vec3_origin, other.s.origin, vec3_origin, self.dmg, 1, DAMAGE_NONE);

	if (self.moveinfo.state == STATE_UP)
		plat_go_down (self);
	else if (self.moveinfo.state == STATE_DOWN)
		plat_go_up (self);
}

static void Use_Plat (edict_t &ent, edict_t &other, edict_t &activator)
{ 
	if (ent.think)
		return;		// already down
	plat_go_down (ent);
}

static void Touch_Plat_Center (edict_t &ent, edict_t &other, const cplane_t *plane, const csurface_t *surf)
{
	if (!other.client && !(other.svflags & SVF_MONSTER))
		return;
		
	if (other.health <= 0)
		return;

	edict_t &platform = ent.enemy;	// now point at the plat, not the trigger
	if (platform.moveinfo.state == STATE_BOTTOM)
		plat_go_up (platform);
	else if (platform.moveinfo.state == STATE_TOP)
		platform.nextthink = level.time + 100;	// the player is still on the plat, so delay going down
}

static void plat_spawn_inside_trigger (edict_t &ent)
{
//
// middle trigger
//	
	edict_t &trigger = G_Spawn();
	trigger.touch = Touch_Plat_Center;
	trigger.movetype = MOVETYPE_NONE;
	trigger.solid = SOLID_TRIGGER;
	trigger.enemy = ent;
	
	vec3_t tmin = {
		ent.mins[0] + 25,
		ent.mins[1] + 25,
		ent.mins[2]
	};

	vec3_t tmax = {
		ent.maxs[0] - 25,
		ent.maxs[1] - 25,
		ent.maxs[2] + 8
	};

	tmin[2] = tmax[2] - (ent.pos1[2] - ent.pos2[2] + st.lip);

	if (ent.spawnflags & PLAT_LOW_TRIGGER)
		tmax[2] = tmin[2] + 8;
	
	if (tmax[0] - tmin[0] <= 0)
	{
		tmin[0] = (ent.mins[0] + ent.maxs[0]) * 0.5f;
		tmax[0] = tmin[0] + 1;
	}
	if (tmax[1] - tmin[1] <= 0)
	{
		tmin[1] = (ent.mins[1] + ent.maxs[1]) * 0.5f;
		tmax[1] = tmin[1] + 1;
	}
	
	trigger.mins = tmin;
	trigger.maxs = tmax;

	trigger.Link();
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
void SP_func_plat (edict_t &ent)
{
	ent.s.angles.Clear();
	ent.solid = SOLID_BSP;
	ent.movetype = MOVETYPE_PUSH;

	gi.setmodel (ent, ent.model);

	ent.blocked = plat_blocked;

	if (!ent.speed)
		ent.speed = 20;
	else
		ent.speed *= 0.1;

	if (!ent.accel)
		ent.accel = 5;
	else
		ent.accel *= 0.1;

	if (!ent.decel)
		ent.decel = 5;
	else
		ent.decel *= 0.1;

	if (!ent.dmg)
		ent.dmg = 2;

	if (!st.lip)
		st.lip = 8;

	// pos1 is the top position, pos2 is the bottom
	ent.pos1 = ent.s.origin;
	ent.pos2 = ent.s.origin;
	if (st.height)
		ent.pos2[2] -= st.height;
	else
		ent.pos2[2] -= (ent.maxs[2] - ent.mins[2]) - st.lip;

	ent.use = Use_Plat;

	plat_spawn_inside_trigger (ent);	// the "start moving" trigger	

	if (ent.targetname)
		ent.moveinfo.state = STATE_UP;
	else
	{
		ent.s.origin = ent.pos2;
		ent.Link();
		ent.moveinfo.state = STATE_BOTTOM;
	}

	ent.moveinfo.speed = ent.speed;
	ent.moveinfo.accel = ent.accel;
	ent.moveinfo.decel = ent.decel;
	ent.moveinfo.wait = ent.wait;
	ent.moveinfo.start_origin = ent.pos1;
	ent.moveinfo.start_angles = ent.s.angles;
	ent.moveinfo.end_origin = ent.pos2;
	ent.moveinfo.end_angles = ent.s.angles;

	ent.moveinfo.sound_start = gi.soundindex ("plats/pt1_strt.wav");
	ent.moveinfo.sound_middle = gi.soundindex ("plats/pt1_mid.wav");
	ent.moveinfo.sound_end = gi.soundindex ("plats/pt1_end.wav");
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

static void rotating_blocked (edict_t &self, edict_t &other)
{
	T_Damage (other, self, self, vec3_origin, other.s.origin, vec3_origin, self.dmg, 1, DAMAGE_NONE);
}

static void rotating_touch (edict_t &self, edict_t &other, const cplane_t *plane, const csurface_t *surf)
{
	if (self.avelocity[0] || self.avelocity[1] || self.avelocity[2])
		T_Damage (other, self, self, vec3_origin, other.s.origin, vec3_origin, self.dmg, 1, DAMAGE_NONE);
}

static void rotating_use (edict_t &self, edict_t &other, edict_t &activator)
{
	if (self.avelocity != vec3_origin)
	{
		self.s.sound = SOUND_NONE;
		self.avelocity.Clear();
		self.touch = nullptr;
	}
	else
	{
		self.s.sound = self.moveinfo.sound_middle;
		self.avelocity = self.movedir * self.speed;
		if (self.spawnflags & 16)
			self.touch = rotating_touch;
	}
}

void SP_func_rotating (edict_t &ent)
{
	ent.solid = SOLID_BSP;
	if (ent.spawnflags & 32)
		ent.movetype = MOVETYPE_STOP;
	else
		ent.movetype = MOVETYPE_PUSH;

	// set the axis of rotation
	ent.movedir.Clear();
	if (ent.spawnflags & 4)
		ent.movedir[2] = 1.0;
	else if (ent.spawnflags & 8)
		ent.movedir[0] = 1.0;
	else // Z_AXIS
		ent.movedir[1] = 1.0;

	// check for reverse rotation
	if (ent.spawnflags & 2)
		ent.movedir = -ent.movedir;

	if (!ent.speed)
		ent.speed = 100;
	if (!ent.dmg)
		ent.dmg = 2;

//	ent->moveinfo.sound_middle = "doors/hydro1.wav";

	ent.use = rotating_use;
	if (ent.dmg)
		ent.blocked = rotating_blocked;

	if (ent.spawnflags & 1)
		ent.use (ent, game.world(), game.world());

	if (ent.spawnflags & 64)
		ent.s.effects |= EF_ANIM_ALL;
	if (ent.spawnflags & 128)
		ent.s.effects |= EF_ANIM_ALLFAST;

	gi.setmodel (ent, ent.model);
	ent.Link();
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

static void button_done (edict_t &self)
{
	self.moveinfo.state = STATE_BOTTOM;
	self.s.effects &= ~EF_ANIM23;
	self.s.effects |= EF_ANIM01;
}

static void button_return (edict_t &self)
{
	self.moveinfo.state = STATE_DOWN;

	Move_Calc (self, self.moveinfo.start_origin, button_done);

	self.s.frame = 0;

	if (self.health)
		self.takedamage = true;
}

static void button_wait (edict_t &self)
{
	self.moveinfo.state = STATE_TOP;
	self.s.effects &= ~EF_ANIM01;
	self.s.effects |= EF_ANIM23;

	G_UseTargets (self, self.activator);
	self.s.frame = 1;
	if (self.moveinfo.wait != -1ull)
	{
		self.nextthink = level.time + self.moveinfo.wait;
		self.think = button_return;
	}
}

static void button_fire (edict_t &self)
{
	if (self.moveinfo.state == STATE_UP || self.moveinfo.state == STATE_TOP)
		return;

	self.moveinfo.state = STATE_UP;
	if (self.moveinfo.sound_start && !(self.flags & FL_TEAMSLAVE))
		self.PlaySound(self.moveinfo.sound_start, CHAN_NO_PHS_ADD | CHAN_VOICE, ATTN_STATIC);
	Move_Calc (self, self.moveinfo.end_origin, button_wait);
}

static void button_use (edict_t &self, edict_t &other, edict_t &activator)
{
	self.activator = activator;
	button_fire (self);
}

static void button_touch (edict_t &self, edict_t &other, const cplane_t *plane, const csurface_t *surf)
{
	if (!other.client && !(other.svflags & SVF_MONSTER))
		return;

	if (other.health <= 0)
		return;

	self.activator = other;
	button_fire (self);
}

static void button_killed (edict_t &self, edict_t &inflictor, edict_t &attacker, const int32_t &damage, const vec3_t &point)
{
	self.activator = attacker;
	self.health = self.max_health;
	self.takedamage = false;
	button_fire (self);
}

void SP_func_button (edict_t &ent)
{
	G_SetMovedir (ent.s.angles, ent.movedir);
	ent.movetype = MOVETYPE_STOP;
	ent.solid = SOLID_BSP;
	gi.setmodel (ent, ent.model);

	if (ent.sounds != 1)
		ent.moveinfo.sound_start = gi.soundindex ("switches/butn2.wav");
	
	if (!ent.speed)
		ent.speed = 40;
	if (!ent.accel)
		ent.accel = ent.speed;
	if (!ent.decel)
		ent.decel = ent.speed;

	if (!ent.wait)
		ent.wait = 3;
	if (!st.lip)
		st.lip = 4;

	ent.pos1 = ent.s.origin;

	const vec3_t abs_movedir = ent.movedir.Apply(abs);
	const vec_t dist = abs_movedir.Dot(ent.size) - st.lip;

	ent.pos2 = ent.pos1 + (ent.movedir * dist);

	ent.use = button_use;
	ent.s.effects |= EF_ANIM01;

	if (ent.health)
	{
		ent.max_health = ent.health;
		ent.die = button_killed;
		ent.takedamage = true;
	}
	else if (!ent.targetname)
		ent.touch = button_touch;

	ent.moveinfo.state = STATE_BOTTOM;

	ent.moveinfo.speed = ent.speed;
	ent.moveinfo.accel = ent.accel;
	ent.moveinfo.decel = ent.decel;
	ent.moveinfo.wait = ent.wait;
	ent.moveinfo.start_origin = ent.pos1;
	ent.moveinfo.start_angles = ent.s.angles;
	ent.moveinfo.end_origin = ent.pos2;
	ent.moveinfo.end_angles = ent.s.angles;

	ent.Link();
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

static void door_use_areaportals (const edict_t &self, const bool &open)
{
	if (!self.target)
		return;

	edict_ref t = nullptr;

	while ((t = G_Find (t, FOFS(targetname), self.target)))
		if (iequals(t->classname, "func_areaportal"))
			gi.SetAreaPortalState (t->style, open);
}

static void door_go_down (edict_t &self);

static void door_hit_top (edict_t &self)
{
	if (!(self.flags & FL_TEAMSLAVE))
	{
		if (self.moveinfo.sound_end)
			self.PlaySound(self.moveinfo.sound_end, CHAN_NO_PHS_ADD | CHAN_VOICE, ATTN_STATIC);
		self.s.sound = SOUND_NONE;
	}

	self.moveinfo.state = STATE_TOP;
	
	if (self.spawnflags & DOOR_TOGGLE)
		return;

	if (self.moveinfo.wait != -1ull)
	{
		self.think = door_go_down;
		self.nextthink = level.time + self.moveinfo.wait;
	}
}

static void door_hit_bottom (edict_t &self)
{
	if (!(self.flags & FL_TEAMSLAVE))
	{
		if (self.moveinfo.sound_end)
			self.PlaySound(self.moveinfo.sound_end, CHAN_NO_PHS_ADD | CHAN_VOICE, ATTN_STATIC);
		self.s.sound = SOUND_NONE;
	}

	self.moveinfo.state = STATE_BOTTOM;
	door_use_areaportals (self, false);
}

static void door_go_down (edict_t &self)
{
	if (!(self.flags & FL_TEAMSLAVE))
	{
		if (self.moveinfo.sound_start)
			self.PlaySound(self.moveinfo.sound_start, CHAN_NO_PHS_ADD | CHAN_VOICE, ATTN_STATIC);
		self.s.sound = self.moveinfo.sound_middle;
	}
	if (self.max_health)
	{
		self.takedamage = true;
		self.health = self.max_health;
	}
	
	self.moveinfo.state = STATE_DOWN;
	if (strcmp(self.classname, "func_door") == 0)
		Move_Calc (self, self.moveinfo.start_origin, door_hit_bottom);
	else if (strcmp(self.classname, "func_door_rotating") == 0)
		AngleMove_Calc (self, door_hit_bottom);
}

static void door_go_up (edict_t &self, edict_t &activator)
{
	if (self.moveinfo.state == STATE_UP)
		return;		// already going up

	if (self.moveinfo.state == STATE_TOP)
	{	// reset top wait time
		if (self.moveinfo.wait != -1ull)
			self.nextthink = level.time + self.moveinfo.wait;
		return;
	}
	
	if (!(self.flags & FL_TEAMSLAVE))
	{
		if (self.moveinfo.sound_start)
			self.PlaySound(self.moveinfo.sound_start, CHAN_NO_PHS_ADD | CHAN_VOICE, ATTN_STATIC);
		self.s.sound = self.moveinfo.sound_middle;
	}
	self.moveinfo.state = STATE_UP;
	if (strcmp(self.classname, "func_door") == 0)
		Move_Calc (self, self.moveinfo.end_origin, door_hit_top);
	else if (strcmp(self.classname, "func_door_rotating") == 0)
		AngleMove_Calc (self, door_hit_top);

	G_UseTargets (self, activator);
	door_use_areaportals (self, true);
}

static void door_use (edict_t &self, edict_t &other, edict_t &activator)
{
	if (self.flags & FL_TEAMSLAVE)
		return;

	if (self.spawnflags & DOOR_TOGGLE)
	{
		if (self.moveinfo.state == STATE_UP || self.moveinfo.state == STATE_TOP)
		{
			// trigger all paired doors
			for (edict_ref ent = self; ent; ent = ent->teamchain)
			{
				ent->message = nullptr;
				ent->touch = nullptr;
				door_go_down (ent);
			}
			return;
		}
	}
	
	// trigger all paired doors
	for (edict_ref ent = self; ent; ent = ent->teamchain)
	{
		ent->message = nullptr;
		ent->touch = nullptr;
		door_go_up (ent, activator);
	}
};

static void Touch_DoorTrigger (edict_t &self, edict_t &other, const cplane_t *plane, const csurface_t *surf)
{
	if (other.health <= 0)
		return;

	if (!(other.svflags & SVF_MONSTER) && !other.client)
		return;

	if ((self.owner->spawnflags & DOOR_NOMONSTER) && (other.svflags & SVF_MONSTER))
		return;

	if (level.time < self.touch_debounce_time)
		return;

	self.touch_debounce_time = level.time + 100;

	door_use (self.owner, other, other);
}

static void Think_CalcMoveSpeed (edict_t &self)
{
	if (self.flags & FL_TEAMSLAVE)
		return;		// only the team master does this

	// find the smallest distance any member of the team will be moving
	vec_t min_dist = abs(self.moveinfo.distance);

	for (edict_ref ent = self.teamchain; ent; ent = ent->teamchain)
		min_dist = min(min_dist, abs(ent->moveinfo.distance));

	const vec_t time = min_dist / self.moveinfo.speed;

	// adjust speeds so they will all complete at the same time
	for (edict_ref ent = self; ent; ent = ent->teamchain)
	{
		const vec_t newspeed = abs(ent->moveinfo.distance) / time;
		const vec_t ratio = newspeed / ent->moveinfo.speed;
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

static void Think_SpawnDoorTrigger (edict_t &ent)
{
	if (ent.flags & FL_TEAMSLAVE)
		return;		// only the team leader spawns a trigger

	vec3_t mins = ent.absmin;
	vec3_t maxs = ent.absmax;

	for (edict_ref other = ent.teamchain; other; other = other->teamchain)
	{
		AddPointToBounds (other->absmin, mins, maxs);
		AddPointToBounds (other->absmax, mins, maxs);
	}

	// expand 
	mins[0] -= 60;
	mins[1] -= 60;
	maxs[0] += 60;
	maxs[1] += 60;

	edict_t &other = G_Spawn ();
	other.mins = mins;
	other.maxs = maxs;
	other.owner = ent;
	other.solid = SOLID_TRIGGER;
	other.movetype = MOVETYPE_NONE;
	other.touch = Touch_DoorTrigger;
	other.Link();

	if (ent.spawnflags & DOOR_START_OPEN)
		door_use_areaportals (ent, true);

	Think_CalcMoveSpeed (ent);
}

static void door_blocked (edict_t &self, edict_t &other)
{
	if (!(other.svflags & SVF_MONSTER) && !other.client)
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other.s.origin, vec3_origin, 100000, 1, DAMAGE_NONE);
		// if it's still there, nuke it
		if (other.inuse)
			BecomeExplosion1 (other);
		return;
	}

	T_Damage (other, self, self, vec3_origin, other.s.origin, vec3_origin, self.dmg, 1, DAMAGE_NONE);

	if (self.spawnflags & DOOR_CRUSHER)
		return;

// if a door has a negative wait, it would never come back if blocked,
// so let it just squash the object to death real fast
	if (self.moveinfo.wait == -1ull)
		return;

	if (self.moveinfo.state == STATE_DOWN)
	{
		for (edict_ref ent = self.teammaster; ent; ent = ent->teamchain)
		{
			ent->activator = other;
			door_go_up (ent, ent->activator);
		}
	}
	else
	{
		for (edict_ref ent = self.teammaster; ent; ent = ent->teamchain)
			door_go_down (ent);
	}
}

static void door_killed (edict_t &self, edict_t &inflictor, edict_t &attacker, const int32_t &damage, const vec3_t &point)
{
	for (edict_ref ent = self.teammaster; ent; ent = ent->teamchain)
	{
		ent->health = ent->max_health;
		ent->takedamage = false;
	}

	door_use (self.teammaster, attacker, attacker);
}

static void door_touch (edict_t &self, edict_t &other, const cplane_t *plane, const csurface_t *surf)
{
	if (!other.client && !(other.svflags & SVF_MONSTER))
		return;

	if (level.time < self.touch_debounce_time)
		return;

	self.touch_debounce_time = level.time + 500;

	if (other.client)
		other.client->CenterPrint("%s", self.message);
	other.PlaySound(gi.soundindex ("misc/talk1.wav"));
}

void SP_func_door (edict_t &ent)
{
	if (ent.sounds != 1)
	{
		ent.moveinfo.sound_start = gi.soundindex ("doors/dr1_strt.wav");
		ent.moveinfo.sound_middle = gi.soundindex ("doors/dr1_mid.wav");
		ent.moveinfo.sound_end = gi.soundindex ("doors/dr1_end.wav");
	}

	G_SetMovedir (ent.s.angles, ent.movedir);
	ent.movetype = MOVETYPE_PUSH;
	ent.solid = SOLID_BSP;
	gi.setmodel (ent, ent.model);

	ent.blocked = door_blocked;
	ent.use = door_use;
	
	if (!ent.speed)
		ent.speed = 100;

	ent.speed *= 2;

	if (!ent.accel)
		ent.accel = ent.speed;
	if (!ent.decel)
		ent.decel = ent.speed;

	if (!ent.wait)
		ent.wait = 3;
	if (!st.lip)
		st.lip = 8;
	if (!ent.dmg)
		ent.dmg = 2;

	// calculate second position
	ent.pos1 = ent.s.origin;
	const vec3_t abs_movedir = ent.movedir.Apply(abs);
	ent.moveinfo.distance = abs_movedir.Dot(ent.size) - st.lip;
	ent.pos2 = ent.pos1 + (ent.movedir * ent.moveinfo.distance);

	// if it starts open, switch the positions
	if (ent.spawnflags & DOOR_START_OPEN)
	{
		ent.s.origin = ent.pos2;
		ent.pos2 = ent.pos1;
		ent.pos1 = ent.s.origin;
	}

	ent.moveinfo.state = STATE_BOTTOM;

	if (ent.health)
	{
		ent.takedamage = true;
		ent.die = door_killed;
		ent.max_health = ent.health;
	}
	else if (ent.targetname && ent.message)
	{
		gi.soundindex ("misc/talk1.wav");
		ent.touch = door_touch;
	}
	
	ent.moveinfo.speed = ent.speed;
	ent.moveinfo.accel = ent.accel;
	ent.moveinfo.decel = ent.decel;
	ent.moveinfo.wait = ent.wait;
	ent.moveinfo.start_origin = ent.pos1;
	ent.moveinfo.start_angles = ent.s.angles;
	ent.moveinfo.end_origin = ent.pos2;
	ent.moveinfo.end_angles = ent.s.angles;

	if (ent.spawnflags & 16)
		ent.s.effects |= EF_ANIM_ALL;
	if (ent.spawnflags & 64)
		ent.s.effects |= EF_ANIM_ALLFAST;

	// to simplify logic elsewhere, make non-teamed doors into a team of one
	if (!ent.team)
		ent.teammaster = ent;

	ent.Link();

	ent.nextthink = level.time + FRAME_MS;

	if (ent.health || ent.targetname)
		ent.think = Think_CalcMoveSpeed;
	else
		ent.think = Think_SpawnDoorTrigger;
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

void SP_func_door_rotating (edict_t &ent)
{
	ent.s.angles.Clear();

	// set the axis of rotation
	ent.movedir.Clear();
	
	if (ent.spawnflags & DOOR_X_AXIS)
		ent.movedir[2] = 1.0;
	else if (ent.spawnflags & DOOR_Y_AXIS)
		ent.movedir[0] = 1.0;
	else // Z_AXIS
		ent.movedir[1] = 1.0;

	// check for reverse rotation
	if (ent.spawnflags & DOOR_REVERSE)
		ent.movedir = -ent.movedir;

	if (!st.distance)
	{
		gi.dprintf("%s at %s with no distance set\n", ent.classname, vtos(ent.s.origin));
		st.distance = 90;
	}

	ent.pos1 = ent.s.angles;
	ent.pos2 = ent.s.angles + (ent.movedir * st.distance);
	ent.moveinfo.distance = st.distance;

	ent.movetype = MOVETYPE_PUSH;
	ent.solid = SOLID_BSP;
	gi.setmodel (ent, ent.model);

	ent.blocked = door_blocked;
	ent.use = door_use;

	if (!ent.speed)
		ent.speed = 100;
	if (!ent.accel)
		ent.accel = ent.speed;
	if (!ent.decel)
		ent.decel = ent.speed;

	if (!ent.wait)
		ent.wait = 3;
	if (!ent.dmg)
		ent.dmg = 2;

	if (ent.sounds != 1)
	{
		ent.moveinfo.sound_start = gi.soundindex ("doors/dr1_strt.wav");
		ent.moveinfo.sound_middle = gi.soundindex ("doors/dr1_mid.wav");
		ent.moveinfo.sound_end = gi.soundindex ("doors/dr1_end.wav");
	}

	// if it starts open, switch the positions
	if (ent.spawnflags & DOOR_START_OPEN)
	{
		ent.s.angles = ent.pos2;
		ent.pos2 = ent.pos1;
		ent.pos1 = ent.s.angles;
		ent.movedir = -ent.movedir;
	}

	if (ent.health)
	{
		ent.takedamage = true;
		ent.die = door_killed;
		ent.max_health = ent.health;
	}
	
	if (ent.targetname && ent.message)
	{
		gi.soundindex ("misc/talk1.wav");
		ent.touch = door_touch;
	}

	ent.moveinfo.state = STATE_BOTTOM;
	ent.moveinfo.speed = ent.speed;
	ent.moveinfo.accel = ent.accel;
	ent.moveinfo.decel = ent.decel;
	ent.moveinfo.wait = ent.wait;
	ent.moveinfo.start_origin = ent.s.origin;
	ent.moveinfo.start_angles = ent.pos1;
	ent.moveinfo.end_origin = ent.s.origin;
	ent.moveinfo.end_angles = ent.pos2;

	if (ent.spawnflags & 16)
		ent.s.effects |= EF_ANIM_ALL;

	// to simplify logic elsewhere, make non-teamed doors into a team of one
	if (!ent.team)
		ent.teammaster = ent;

	ent.Link();

	ent.nextthink = level.time + FRAME_MS;
	if (ent.health || ent.targetname)
		ent.think = Think_CalcMoveSpeed;
	else
		ent.think = Think_SpawnDoorTrigger;
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

void SP_func_water (edict_t &self)
{
	G_SetMovedir (self.s.angles, self.movedir);
	self.movetype = MOVETYPE_PUSH;
	self.solid = SOLID_BSP;
	gi.setmodel (self, self.model);

	switch (self.sounds)
	{
		case 1: // water
			self.moveinfo.sound_start = gi.soundindex ("world/mov_watr.wav");
			self.moveinfo.sound_end = gi.soundindex ("world/stp_watr.wav");
			break;

		case 2: // lava
			self.moveinfo.sound_start = gi.soundindex ("world/mov_watr.wav");
			self.moveinfo.sound_end = gi.soundindex ("world/stp_watr.wav");
			break;
	}

	// calculate second position
	self.pos1 = self.s.origin;
	const vec3_t abs_movedir = self.movedir.Apply(abs);
	self.moveinfo.distance = abs_movedir.Dot(self.size) - st.lip;
	self.pos2 = self.pos1 + (self.movedir * self.moveinfo.distance);

	// if it starts open, switch the positions
	if (self.spawnflags & DOOR_START_OPEN)
	{
		self.s.origin = self.pos2;
		self.pos2 = self.pos1;
		self.pos1 = self.s.origin;
	}

	self.moveinfo.start_origin = self.pos1;
	self.moveinfo.start_angles = self.s.angles;
	self.moveinfo.end_origin = self.pos2;
	self.moveinfo.end_angles = self.s.angles;

	self.moveinfo.state = STATE_BOTTOM;

	if (!self.speed)
		self.speed = 25;
	self.moveinfo.accel = self.moveinfo.decel = self.moveinfo.speed = self.speed;

	if (!self.wait)
		self.wait = -1ull;
	self.moveinfo.wait = self.wait;

	self.use = door_use;

	if (self.wait == -1ull)
		self.spawnflags |= DOOR_TOGGLE;

	self.classname = "func_door";

	self.Link();
}

constexpr int32_t TRAIN_START_ON	= bit(0);
constexpr int32_t TRAIN_TOGGLE		= bit(1);
constexpr int32_t TRAIN_BLOCK_STOPS	= bit(2);

/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed	default 100
dmg		default	2
noise	looping sound to play when the train is in motion

*/
static void train_next (edict_t &self);

static void train_blocked (edict_t &self, edict_t &other)
{
	if (!(other.svflags & SVF_MONSTER) && !other.client)
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other.s.origin, vec3_origin, 100000, 1, DAMAGE_NONE);
		// if it's still there, nuke it
		if (other.inuse)
			BecomeExplosion1 (other);
		return;
	}

	if (level.time < self.touch_debounce_time)
		return;

	if (!self.dmg)
		return;

	self.touch_debounce_time = level.time + 500;
	T_Damage (other, self, self, vec3_origin, other.s.origin, vec3_origin, self.dmg, 1, DAMAGE_NONE);
}

static void train_wait (edict_t &self)
{
	if (self.target_ent->pathtarget)
	{
		edict_t &ent = self.target_ent;
		char *savetarget = ent.target;
		ent.target = ent.pathtarget;
		G_UseTargets (ent, self.activator);
		ent.target = savetarget;

		// make sure we didn't get killed by a killtarget
		if (!self.inuse)
			return;
	}

	if (self.moveinfo.wait)
	{
		if (self.moveinfo.wait > 0)
		{
			self.nextthink = level.time + self.moveinfo.wait;
			self.think = train_next;
		}
		else if (self.spawnflags & TRAIN_TOGGLE)  // && wait < 0
		{
			train_next (self);
			self.spawnflags &= ~TRAIN_START_ON;
			self.velocity.Clear();
			self.nextthink = 0;
		}

		if (!(self.flags & FL_TEAMSLAVE))
		{
			if (self.moveinfo.sound_end)
				self.PlaySound(self.moveinfo.sound_end, CHAN_NO_PHS_ADD | CHAN_VOICE, ATTN_STATIC);
			self.s.sound = SOUND_NONE;
		}
	}
	else
		train_next (self);
}

static void train_next (edict_t &self)
{
	bool first = true;

again:
	if (!self.target)
	{
//		gi.dprintf ("train_next: no next target\n");
		return;
	}

	const edict_ref &ent = G_PickTarget (self.target);

	if (!ent)
	{
		gi.dprintf ("train_next: bad target %s\n", self.target);
		return;
	}

	self.target = ent->target;

	// check for a teleport path_corner
	if (ent->spawnflags & 1)
	{
		if (!first)
		{
			gi.dprintf ("connected teleport path_corners, see %s at %s\n", ent->classname, vtos(ent->s.origin));
			return;
		}

		first = false;
		self.s.origin = ent->s.origin - self.mins;
		self.s.old_origin = self.s.origin;
		self.s.event = EV_OTHER_TELEPORT;
		self.Link();
		goto again;
	}

	self.moveinfo.wait = ent->wait;
	self.target_ent = ent;

	if (!(self.flags & FL_TEAMSLAVE))
	{
		if (self.moveinfo.sound_start)
			self.PlaySound(self.moveinfo.sound_start, CHAN_NO_PHS_ADD | CHAN_VOICE, ATTN_STATIC);
		self.s.sound = self.moveinfo.sound_middle;
	}

	const vec3_t dest = ent->s.origin - self.mins;
	self.moveinfo.state = STATE_TOP;
	self.moveinfo.start_origin = self.s.origin;
	self.moveinfo.end_origin = dest;
	Move_Calc (self, dest, train_wait);
	self.spawnflags |= TRAIN_START_ON;
}

static void train_resume (edict_t &self)
{
	const edict_ref &ent = self.target_ent;
	const vec3_t dest = ent->s.origin - self.mins;
	self.moveinfo.state = STATE_TOP;
	self.moveinfo.start_origin = self.s.origin;
	self.moveinfo.end_origin = dest;
	Move_Calc (self, dest, train_wait);
	self.spawnflags |= TRAIN_START_ON;
}

static void func_train_find (edict_t &self)
{
	if (!self.target)
	{
		gi.dprintf ("train_find: no target\n");
		return;
	}

	const edict_ref &ent = G_PickTarget (self.target);

	if (!ent)
	{
		gi.dprintf ("train_find: target %s not found\n", self.target);
		return;
	}

	self.target = ent->target;
	self.s.origin = ent->s.origin - self.mins;
	self.Link();

	// if not triggered, start immediately
	if (!self.targetname)
		self.spawnflags |= TRAIN_START_ON;

	if (self.spawnflags & TRAIN_START_ON)
	{
		self.nextthink = level.time + FRAME_MS;
		self.think = train_next;
		self.activator = self;
	}
}

static void train_use (edict_t &self, edict_t &other, edict_t &activator)
{
	self.activator = activator;

	if (self.spawnflags & TRAIN_START_ON)
	{
		if (!(self.spawnflags & TRAIN_TOGGLE))
			return;
		self.spawnflags &= ~TRAIN_START_ON;
		self.velocity.Clear();
		self.nextthink = 0;
	}
	else if (self.target_ent)
		train_resume(self);
	else
		train_next(self);
}

void SP_func_train (edict_t &self)
{
	self.movetype = MOVETYPE_PUSH;

	self.s.angles.Clear();
	self.blocked = train_blocked;

	if (self.spawnflags & TRAIN_BLOCK_STOPS)
		self.dmg = 0;
	else if (!self.dmg)
		self.dmg = 100;

	self.solid = SOLID_BSP;
	gi.setmodel (self, self.model);

	if (st.noise)
		self.moveinfo.sound_middle = gi.soundindex (st.noise);

	if (!self.speed)
		self.speed = 100;

	self.moveinfo.speed = self.speed;
	self.moveinfo.accel = self.moveinfo.decel = self.moveinfo.speed;

	self.use = train_use;

	self.Link();

	if (self.target)
	{
		// start trains on the second frame, to make sure their targets have had
		// a chance to spawn
		self.nextthink = level.time + FRAME_MS;
		self.think = func_train_find;
	}
	else
		gi.dprintf ("func_train without a target at %s\n", vtos(self.absmin));
}


/*QUAKED trigger_elevator (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
*/
static void trigger_elevator_use (edict_t &self, edict_t &other, edict_t &activator)
{
	if (self.movetarget->nextthink)
	{
//		gi.dprintf("elevator busy\n");
		return;
	}

	if (!other.pathtarget)
	{
		gi.dprintf("elevator used with no pathtarget\n");
		return;
	}

	const edict_ref &target = G_PickTarget (other.pathtarget);
	
	if (!target)
	{
		gi.dprintf("elevator used with bad pathtarget: %s\n", other.pathtarget);
		return;
	}

	self.movetarget->target_ent = target;
	train_resume (self.movetarget);
}

static void trigger_elevator_init (edict_t &self)
{
	if (!self.target)
	{
		gi.dprintf("trigger_elevator has no target\n");
		return;
	}

	self.movetarget = G_PickTarget (self.target);

	if (!self.movetarget)
	{
		gi.dprintf("trigger_elevator unable to find target %s\n", self.target);
		return;
	}

	if (strcmp(self.movetarget->classname, "func_train") != 0)
	{
		gi.dprintf("trigger_elevator target %s is not a train\n", self.target);
		return;
	}

	self.use = trigger_elevator_use;
	self.svflags = SVF_NOCLIENT;

}

void SP_trigger_elevator (edict_t &self)
{
	self.think = trigger_elevator_init;
	self.nextthink = level.time + FRAME_MS;
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
static void func_timer_think (edict_t &self)
{
	G_UseTargets (self, self.activator);
	self.nextthink = level.time + self.wait + crandom(self.random);
}

static void func_timer_use (edict_t &self, edict_t &other, edict_t &activator)
{
	self.activator = activator;

	// if on, turn it off
	if (self.nextthink)
	{
		self.nextthink = 0;
		return;
	}

	// turn it on
	if (self.delay)
		self.nextthink = level.time + self.delay;
	else
		func_timer_think (self);
}

void SP_func_timer (edict_t &self)
{
	if (!self.wait)
		self.wait = 1.0;

	self.use = func_timer_use;
	self.think = func_timer_think;

	if (self.random >= self.wait)
	{
		self.random = self.wait - FRAME_MS;
		gi.dprintf("func_timer at %s has random >= wait\n", vtos(self.s.origin));
	}

	if (self.spawnflags & 1)
	{
		self.nextthink = level.time + 100 + st.pausetime + self.delay + self.wait + crandom(self.random);
		self.activator = self;
	}

	self.svflags = SVF_NOCLIENT;
}


/*QUAKED func_conveyor (0 .5 .8) ? START_ON TOGGLE
Conveyors are stationary brushes that move what's on them.
The brush should be have a surface with at least one current content enabled.
speed	default 100
*/

static void func_conveyor_use (edict_t &self, edict_t &other, edict_t &activator)
{
	if (self.spawnflags & 1)
	{
		self.speed = 0;
		self.spawnflags &= ~1;
	}
	else
	{
		self.speed = self.count;
		self.spawnflags |= 1;
	}

	if (!(self.spawnflags & 2))
		self.count = 0;
}

void SP_func_conveyor (edict_t &self)
{
	if (!self.speed)
		self.speed = 100;

	if (!(self.spawnflags & 1))
	{
		self.count = self.speed;
		self.speed = 0;
	}

	self.use = func_conveyor_use;

	gi.setmodel (self, self.model);
	self.solid = SOLID_BSP;
	self.Link();
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

constexpr int32_t SECRET_ALWAYS_SHOOT	= bit(0);
constexpr int32_t SECRET_1ST_LEFT		= bit(1);
constexpr int32_t SECRET_1ST_DOWN		= bit(2);

static void door_secret_move1 (edict_t &self);
static void door_secret_move2 (edict_t &self);
static void door_secret_move3 (edict_t &self);
static void door_secret_move4 (edict_t &self);
static void door_secret_move5 (edict_t &self);
static void door_secret_move6 (edict_t &self);
static void door_secret_done (edict_t &self);

static void door_secret_use (edict_t &self, edict_t &other, edict_t &activator)
{
	// make sure we're not already moving
	if (self.s.origin != vec3_origin)
		return;

	Move_Calc (self, self.pos1, door_secret_move1);
	door_use_areaportals (self, true);
}

static void door_secret_move1 (edict_t &self)
{
	self.nextthink = level.time + 100;
	self.think = door_secret_move2;
}

static void door_secret_move2 (edict_t &self)
{
	Move_Calc (self, self.pos2, door_secret_move3);
}

static void door_secret_move3 (edict_t &self)
{
	if (self.wait == -1ull)
		return;
	self.nextthink = level.time + self.wait;
	self.think = door_secret_move4;
}

static void door_secret_move4 (edict_t &self)
{
	Move_Calc (self, self.pos1, door_secret_move5);
}

static void door_secret_move5 (edict_t &self)
{
	self.nextthink = level.time + 100;
	self.think = door_secret_move6;
}

static void door_secret_move6 (edict_t &self)
{
	Move_Calc (self, vec3_origin, door_secret_done);
}

static void door_secret_done (edict_t &self)
{
	if (!(self.targetname) || (self.spawnflags & SECRET_ALWAYS_SHOOT))
	{
		self.health = 0;
		self.takedamage = true;
	}

	door_use_areaportals (self, false);
}

static void door_secret_blocked (edict_t &self, edict_t &other)
{
	if (!(other.svflags & SVF_MONSTER) && !other.client)
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other.s.origin, vec3_origin, 100000, 1, DAMAGE_NONE);
		// if it's still there, nuke it
		if (other.inuse)
			BecomeExplosion1 (other);
		return;
	}

	if (level.time < self.touch_debounce_time)
		return;

	self.touch_debounce_time = level.time + 500;
	T_Damage (other, self, self, vec3_origin, other.s.origin, vec3_origin, self.dmg, 1, DAMAGE_NONE);
}

static void door_secret_die (edict_t &self, edict_t &inflictor, edict_t &attacker, const int32_t &damage, const vec3_t &point)
{
	self.takedamage = false;
	door_secret_use (self, attacker, attacker);
}

void SP_func_door_secret (edict_t &ent)
{
	ent.moveinfo.sound_start = gi.soundindex ("doors/dr1_strt.wav");
	ent.moveinfo.sound_middle = gi.soundindex ("doors/dr1_mid.wav");
	ent.moveinfo.sound_end = gi.soundindex ("doors/dr1_end.wav");

	ent.movetype = MOVETYPE_PUSH;
	ent.solid = SOLID_BSP;
	gi.setmodel (ent, ent.model);

	ent.blocked = door_secret_blocked;
	ent.use = door_secret_use;

	if (!ent.targetname || (ent.spawnflags & SECRET_ALWAYS_SHOOT))
	{
		ent.health = 0;
		ent.takedamage = true;
		ent.die = door_secret_die;
	}

	if (!ent.dmg)
		ent.dmg = 2;

	if (!ent.wait)
		ent.wait = 5;

	ent.moveinfo.accel =
	ent.moveinfo.decel =
	ent.moveinfo.speed = 50;

	// calculate positions
	const auto &[ forward, right, up ] = ent.s.angles.AngleVectors();
	ent.s.angles.Clear();

	const vec_t side = 1.0 - (ent.spawnflags & SECRET_1ST_LEFT);
	vec_t width;
	
	if (ent.spawnflags & SECRET_1ST_DOWN)
		width = abs(up.Dot(ent.size));
	else
		width = abs(right.Dot(ent.size));
	
	const vec_t length = abs(forward.Dot(ent.size));

	if (ent.spawnflags & SECRET_1ST_DOWN)
		ent.pos1 = ent.s.origin + (up * (-1 * width));
	else
		ent.pos1 = ent.s.origin + (right * (side * width));

	ent.pos2 = ent.pos1 + (forward * length);

	if (ent.health)
	{
		ent.takedamage = true;
		ent.die = door_killed;
		ent.max_health = ent.health;
	}
	else if (ent.targetname && ent.message)
	{
		gi.soundindex ("misc/talk1.wav");
		ent.touch = door_touch;
	}
	
	ent.classname = "func_door";
	ent.Link();
}


/*QUAKED func_killbox (1 0 0) ?
Kills everything inside when fired, irrespective of protection.
*/
static void use_killbox (edict_t &self, edict_t &other, edict_t &activator)
{
	KillBox (self);
}

void SP_func_killbox (edict_t &ent)
{
	gi.setmodel (ent, ent.model);
	ent.use = use_killbox;
	ent.svflags = SVF_NOCLIENT;
}

