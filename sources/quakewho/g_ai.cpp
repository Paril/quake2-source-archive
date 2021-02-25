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
// g_ai.c

#include "g_local.h"

/*
=============
ai_move

Move the specified distance at current facing.
This replaces the QC functions: ai_forward, ai_back, ai_pain, and ai_painforward
==============
*/
void ai_move (edict_t &self, const vec_t &dist)
{
	if (self.control && level.control_delay < level.time)
		M_MoveToController (self, dist, true);
	else
		M_walkmove (self, self.s.angles[YAW], dist);
}


/*
=============
ai_stand

Used for standing around and looking for players
Distance is for slight position adjustments needed by the animations
==============
*/
void ai_stand (edict_t &self, const vec_t &dist)
{
	if (self.control && level.control_delay < level.time)
	{
		M_MoveToController (self, dist, false);
		return;
	}

	if (dist)
		M_walkmove (self, self.s.angles[YAW], dist);

	if (!(self.spawnflags & 1) && self.monsterinfo.idle)
	{		
		if (level.time > self.monsterinfo.idle_time)
		{
			if (self.monsterinfo.idle_time)
			{
				self.monsterinfo.idle (self);
				self.monsterinfo.idle_time = level.time + frandom(1500, 3000);
			}
			else
				self.monsterinfo.idle_time = level.time + frandom(1500);
		}
	}

	if (self.monsterinfo.follow_ent)
	{
		if (!M_GonnaHitSpecificThing(self, self.monsterinfo.follow_ent))
		{
			self.ideal_yaw = (self.monsterinfo.follow_ent->s.origin - self.s.origin).Normalized().ToYaw();

			if (self.monsterinfo.follow_direction)
				self.ideal_yaw += 180;

			if (random() < 0.5f)
				self.monsterinfo.run(self);
			else
				self.monsterinfo.walk(self);
		}
		else
			self.monsterinfo.follow_time -= frandom(0, 3);
	}
	else if (level.time > self.monsterinfo.next_runwalk_check)
	{
		if (prandom(50))
		{
			self.ideal_yaw = frandom(360);

			if (frandom() < 0.5f)
				self.monsterinfo.run(self);
			else
				self.monsterinfo.walk(self);

			self.monsterinfo.should_stand_check = level.time + frandom(100, 2400);
		}

		self.monsterinfo.next_runwalk_check = level.time + frandom(100, 2400);
	}
}


/*
=============
ai_walk

The monster is walking it's beat
=============
*/
void ai_walk (edict_t &self, const vec_t &dist)
{
	if (self.control && level.control_delay < level.time)
	{
		M_MoveToController (self, dist, true);
		return;
	}

	M_MoveToGoal (self, dist);
}

/*
=============
visible

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
bool visible (const edict_t &self, const edict_t &other)
{
	if ((other.svflags & SVF_NOCLIENT) || !other.health || other.deadflag)
		return false;

	const vec3_t spot1 = self.s.origin + vec3_t{ 0, 0, self.viewheight };
	const vec3_t spot2 = other.s.origin + vec3_t{ 0, 0, other.viewheight };

	return gi.trace (spot1, spot2, self, MASK_OPAQUE).fraction == 1.0f;
}


/*
=============
infront

returns 1 if the entity is in front (in sight) of self
=============
*/
bool infront (const edict_t &self, const edict_t &other)
{
	const vec3_t forward = self.s.angles.Forward();
	const vec3_t vec = (other.s.origin - self.s.origin).Normalized();
	return vec.Dot(forward) > 0.3f;
}

//=============================================================================

/*
============
FacingIdeal

============
*/
bool FacingIdeal(const edict_t &self)
{
	const vec_t delta = anglemod(self.s.angles[YAW] - self.ideal_yaw);
	return !(delta > 45 && delta < 315);
}


/*
=============
ai_run

The monster has an enemy it is trying to kill
=============
*/
void ai_run (edict_t &self, const vec_t &dist)
{
	if (self.control && level.control_delay < level.time)
		M_MoveToController (self, dist, true);
	else
		M_MoveToGoal (self, dist);
}
