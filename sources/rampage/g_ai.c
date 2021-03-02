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
void M_avoid_danger(edict_t *self)
{
	//gi.bprintf(PRINT_HIGH, "M_avoid_danger: FUNCTION START!\n");

	edict_t *ent;
	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, self->dmg_radius * 1.5)) != NULL)
	{
		if (!ent->monsterinfo.scale)
			continue;

		if (ent->health <= 0)
			continue;
		if (!ent->monsterinfo.dodge)
			continue;
		//gi.bprintf(PRINT_HIGH, "M_avoid_danger: should jump away from danger!\n");
		ent->monsterinfo.aiflags |= AI_JUMPDODGEPROJ;
		ent->monsterinfo.aiflags |= AI_JUMPDODGE;
		ent->monsterinfo.jump_ent = self;
		
		ent->monsterinfo.dodge(ent, self, 0);
	}
}


void monster_jump(edict_t *self)
{
	if(!(self->monsterinfo.aiflags & AI_JUMPDODGE))
		return;

	vec3_t left, right, jump_dir;
	float f_left, f_right;


	if (self->monsterinfo.aiflags & AI_JUMPDODGEPROJ && self->monsterinfo.jump_ent)
	{
		
			VectorSubtract(self->s.origin, self->monsterinfo.jump_ent->s.origin, jump_dir);
			//gi.bprintf(PRINT_HIGH, "monster_jump: should jump away from danger!\n");
			jump_dir[2] *= 0.25;
	}
	else
	{
		f_left = scan_dir(self, SCAN_LEFT, 128, left);

		f_right = scan_dir(self, SCAN_RIGHT, 128, right);

		if (f_left < f_right)
		{
			//gi.bprintf(PRINT_HIGH, "monster_jump: JUMP LEFT\n");
			VectorSubtract(self->s.origin, left, jump_dir);
		}
		else if (f_left > f_right)
		{
			//gi.bprintf(PRINT_HIGH, "monster_jump: JUMP RIGHT\n");
			VectorSubtract(self->s.origin, right, jump_dir);
		}
		else if (random() > 0.5)
		{
			//gi.bprintf(PRINT_HIGH, "monster_jump: JUMP LEFT(RANDOM)\n");
			VectorSubtract(self->s.origin, left, jump_dir);
		}
		else
		{
			//gi.bprintf(PRINT_HIGH, "monster_jump: JUMP RIGHT(RANDOM)\n");
			VectorSubtract(self->s.origin, right, jump_dir);
		}
	}
	VectorNormalize(jump_dir);
	VectorMA(self->velocity, 300 + (100 * random()), jump_dir, self->velocity);

	self->velocity[2] += 150 + random() * 50;
	self->monsterinfo.jump_ent = NULL;
	self->monsterinfo.aiflags &= ~AI_JUMPDODGEPROJ;
	self->monsterinfo.aiflags &= ~AI_JUMPDODGE;



	gi.sound(self, CHAN_AUTO, gi.soundindex(va("player/step%i.wav", rand() % 5)), 1, ATTN_IDLE, 0);
	gi.sound(self, CHAN_AUTO, gi.soundindex(va("player/step%i.wav", rand() % 5)), 1, ATTN_IDLE, 0.05);


}

qboolean ffire_radius_check(edict_t *self)
{
	edict_t *ent = NULL;
	if (!self->enemy)
		return false;
	while ((ent = findradius(ent, self->enemy->s.origin, 256)) != NULL)
	{
		if (!ent->monsterinfo.scale)
			continue;

		if (self->health <= 0 || self->deadflag & DEAD_DEAD)
			continue;

		return true;
	}
	return false;
}
void predict_shot(edict_t *self, float proj_speed, vec3_t end)
{
	vec3_t vel;
	vec3_t vec;
	float prediction;

	if (self->enemy->client)
	{
		VectorCopy(self->enemy->velocity, vel);
		VectorScale(vel, FRAMETIME, vel);
	}
	else
		VectorSubtract(self->enemy->s.origin, self->enemy->s.old_origin, vel);


	VectorSubtract(self->s.origin, self->enemy->s.origin, vec);

	if (VectorLength(vel))
	{
		if (!self->enemy->groundentity && self->enemy->movetype != MOVETYPE_FLY)
		{
			if (vel[2] > 0)
				vel[2] *= 0.25; //don't predict player/enemy flying away
			else
				vel[2] *= 0.5 + (crandom() * 0.5); //if enemy is falling predict with half accuracy
		}
			

		if (self->s.skinnum < 2 && strcmp(self->classname, "monster_soldier_light") == 0 || proj_speed > 0) //why checking if it's only light guard???
		{
			prediction = VectorLength(vec) / (proj_speed - VectorLength(vel));
			prediction *= (1 / FRAMETIME) ;
			VectorScale(vel, prediction, vel);
		}
		//gi.bprintf(PRINT_HIGH, "PREDICTION: vel = %s, classname = %s\n", vtos(vel), self->classname);



		//gi.bprintf(PRINT_HIGH, " vec vel frame = %s, vel frame = %f, predm = %f, pred = %s, predl = %f\n", vtos(vel_frame), VectorLength(vel_frame), prediction, vtos(vel), VectorLength(vel));

		//VectorAdd(end, vel, end);
		//debug_trail(self->s.origin, end);
		//gi.bprintf(PRINT_HIGH, "PREDICT SHOT: CHANGE, o1 = %s, o2 = %s, vel = %s\n", vtos(self->enemy->s.origin), vtos(self->enemy->s.old_origin), vtos(vel));
		VectorAdd(end, vel, end);

	}
	//else
	//	gi.bprintf(PRINT_HIGH, "PREDICT SHOT: NO CHANGE, o1 = %s, o2 = %s\n", vtos(self->enemy->s.origin), vtos(self->enemy->s.old_origin));
}
qboolean FindTarget(edict_t *self);
extern cvar_t	*maxclients;

qboolean ai_checkattack(edict_t *self, float dist);

qboolean	enemy_vis;
qboolean	enemy_infront;
int			enemy_range;
float		enemy_yaw;

//============================================================================

/*
=============
visible

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
qboolean visible_shootable(edict_t *self, edict_t *other)
{
	vec3_t	spot1;
	vec3_t	spot2;
	trace_t	trace;
	int got_enemy;
	if (self->enemy == other && self->enemy->client)
		got_enemy = 1;
	VectorCopy(self->s.origin, spot1);
	spot1[2] += self->viewheight;
	VectorCopy(other->s.origin, spot2);
	if (other->health > 0)
		spot2[2] += other->viewheight;
	trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE);
	/*if (self->health <= 0)
	{
		gi.bprintf(PRINT_HIGH, "VISIBLE: spot1 = %s, spot2 = %s\n", vtos(spot1), vtos(spot2));
	}*/
	if (trace.fraction == 1.0)
	{
		if (got_enemy)
		{
			VectorCopy(other->s.origin, self->monsterinfo.last_sighting2);
			//gi.bprintf(PRINT_HIGH, "VISIBLE SHOOTABLE: set last_sighting2 to %s\n", vtos(self->monsterinfo.last_sighting2));
		}
		if (!strcmp(self->classname, "monster_makron") || !strcmp(self->classname, "monster_parasite") || !strcmp(self->classname, "monster_flipper"))
			return true;
		vec3_t forward, right, offset;
		AngleVectors(self->s.angles, forward, right, NULL);
		offset[0] = self->maxs[0];
		G_ProjectSource(self->s.origin, offset, forward, right, spot1);
		trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE);

		if (trace.fraction == 1.0)
			return true;
		else
			return false;
	}

	return false;
}

qboolean visible(edict_t *self, edict_t *other)
{
	vec3_t	spot1;
	vec3_t	spot2;
	trace_t	trace;
	int got_enemy;
	if (self->enemy == other && self->enemy->client)
		got_enemy = 1;
	VectorCopy(self->s.origin, spot1);
	spot1[2] += self->viewheight;
	VectorCopy(other->s.origin, spot2);
	if (other->health > 0)
		spot2[2] += other->viewheight;
	trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE);
	/*if (self->health <= 0)
	{
		gi.bprintf(PRINT_HIGH, "VISIBLE: spot1 = %s, spot2 = %s\n", vtos(spot1), vtos(spot2));
	}*/
	if (trace.fraction == 1.0)
	{
		if (got_enemy)
		{
			VectorCopy(other->s.origin, self->monsterinfo.last_sighting2);
			//gi.bprintf(PRINT_HIGH, "VISIBLE: set last_sighting2 to %s\n", vtos(self->monsterinfo.last_sighting2));
		}
		return true;
	}

	return false;
}

qboolean visible_point(edict_t *self, edict_t *other, vec3_t point)
{
	vec3_t	spot1;
	vec3_t	spot2;
	trace_t	trace;

	VectorCopy(self->s.origin, spot1);
	spot1[2] += self->viewheight;
	VectorCopy(point, spot2);
	if (other->health > 0)
		spot2[2] += other->viewheight;
	trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE);
	/*if (self->health <= 0)
	{
		gi.bprintf(PRINT_HIGH, "VISIBLE: spot1 = %s, spot2 = %s\n", vtos(spot1), vtos(spot2));
	}*/
	if (trace.fraction == 1.0)
	{
		//VectorCopy(other->s.origin, self->monsterinfo.last_sighting2);
		//gi.bprintf(PRINT_HIGH, "VISIBLE POINT: set last_sighting2 to %s\n", vtos(self->monsterinfo.last_sighting2));
		return true;
	}

	return false;
}

/*
=============
infront

returns 1 if the entity is in front (in sight) of self
=============
*/
qboolean infront(edict_t *self, edict_t *other)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorSubtract(other->s.origin, self->s.origin, vec);
	VectorNormalize(vec);
	dot = DotProduct(vec, forward);

	if (dot > 0.3)
		return true;
	return false;
}

qboolean infront_point_dir(edict_t *self, vec3_t point)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorSubtract(point, self->s.origin, vec);
	VectorNormalize(vec);
	dot = DotProduct(vec, forward);

	if (dot > 0.9)
		return true;
	return false;
}

void check_move_dir(edict_t *self, vec3_t point)
{
	if (!self->enemy)
		return;
	float	ideal;
	float	current;
	float	move;
	float	speed;
	vec3_t temp;
	current = anglemod(self->s.angles[YAW]);

	VectorSubtract(point, self->s.origin, temp);
	ideal = vectoyaw(temp);
	move = ideal - current;

	if (ideal > current)
	{

		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (infront_point_dir(self, point))
	{
		self->monsterinfo.move_dir = MOVE_STRAIGHT;
	}
	else if (move < 0)
	{
		self->monsterinfo.move_dir = MOVE_LEFT;
	}
	else
	{
		self->monsterinfo.move_dir = MOVE_RIGHT;
	}

	
	//gi.bprintf(PRINT_HIGH, "check_move_dir:  %i\n", self->monsterinfo.move_dir);
	
}



qboolean infront_point(edict_t *self, vec3_t point)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorSubtract(point, self->s.origin, vec);
	VectorNormalize(vec);
	dot = DotProduct(vec, forward);

	if (dot > 0.2)
		return true;
	return false;
}
/*
=================
AI_SetSightClient

Called once each frame to set level.sight_client to the
player to be checked for in findtarget.

If all clients are either dead or in notarget, sight_client
will be null.

In coop games, sight_client will cycle between the clients.
=================
*/
void AI_SetSightClient(void)
{
	edict_t	*ent;
	int		start, check;

	if (level.sight_client == NULL)
		start = 1;
	else
		start = level.sight_client - g_edicts;

	check = start;
	while (1)
	{
		check++;
		if (check > game.maxclients)
			check = 1;
		ent = &g_edicts[check];
		if (ent->inuse
			&& ent->health > 0
			&& !(ent->flags & FL_NOTARGET))
		{
			level.sight_client = ent;
			return;		// got one
		}
		if (check == start)
		{
			level.sight_client = NULL;
			return;		// nobody to see
		}
	}
}

//============================================================================

/*
=============
ai_move

Move the specified distance at current facing.
This replaces the QC functions: ai_forward, ai_back, ai_pain, and ai_painforward
==============
*/
void ai_move(edict_t *self, float dist)
{
	M_walkmove(self, self->s.angles[YAW], dist);
}


/*
=============
ai_stand

Used for standing around and looking for players
Distance is for slight position adjustments needed by the animations
==============
*/
void ai_stand(edict_t *self, float dist)
{
	vec3_t	v;

	if (dist)
		M_walkmove(self, self->s.angles[YAW], dist);

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		if (self->enemy)
		{
			VectorSubtract(self->enemy->s.origin, self->s.origin, v);
			self->ideal_yaw = vectoyaw(v);
			//gi.bprintf(PRINT_HIGH, "STAND GROUND, UPDATE IDEAL YAW\n");
			if (self->s.angles[YAW] != self->ideal_yaw && self->monsterinfo.aiflags & AI_TEMP_STAND_GROUND)
			{
				self->monsterinfo.aiflags &= ~(AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
				self->monsterinfo.run(self);
				//gi.bprintf(PRINT_HIGH, "ai_stand: self->monsterinfo.run\n");
			}
			M_ChangeYaw(self);
			ai_checkattack(self, 0);
		}
		else
			FindTarget(self);
		return;
	}

	if (FindTarget(self))
		return;

	if (level.time > self->monsterinfo.pausetime)
	{
		self->monsterinfo.walk(self);
		return;
	}

	if (!(self->spawnflags & 1) && (self->monsterinfo.idle) && (level.time > self->monsterinfo.idle_time))
	{
		if (self->monsterinfo.idle_time)
		{
			self->monsterinfo.idle(self);
			self->monsterinfo.idle_time = level.time + 15 + random() * 15;
		}
		else
		{
			self->monsterinfo.idle_time = level.time + random() * 15;
		}
	}
}


/*
=============
ai_walk

The monster is walking it's beat
=============
*/
void ai_walk(edict_t *self, float dist)
{
	M_MoveToGoal(self, dist);

	// check for noticing a player
	if (FindTarget(self))
		return;

	if ((self->monsterinfo.search) && (level.time > self->monsterinfo.idle_time))
	{
		if (self->monsterinfo.idle_time)
		{
			self->monsterinfo.search(self);
			self->monsterinfo.idle_time = level.time + 15 + random() * 15;
		}
		else
		{
			self->monsterinfo.idle_time = level.time + random() * 15;
		}
	}
}


/*
=============
ai_charge

Turns towards target and advances
Use this call with a distnace of 0 to replace ai_face
==============
*/
void ai_charge(edict_t *self, float dist)
{
	vec3_t	v;
	vec3_t temp;
	qboolean enemy_vis = visible(self, self->enemy);
	/*int enemy_infront = infront(self, self->enemy);
	int enemy_range = range(self, self->enemy);
	if (!enemy_vis)
		VectorSubtract(self->monsterinfo.last_sighting2, self->s.origin, temp);
	else if (enemy_vis)
		VectorSubtract(self->enemy->s.origin, self->s.origin, temp);
	gi.bprintf(PRINT_HIGH, "see enemy? = %i\n", enemy_vis);
	self->ideal_yaw = vectoyaw(temp);*/

	VectorSubtract(self->enemy->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);

	//gi.bprintf(PRINT_HIGH, "AI CHARGE, UPDATE IDEAL YAW\n");

	M_ChangeYaw(self);

	if (dist)
		M_walkmove(self, self->s.angles[YAW], dist);
}


/*
=============
ai_turn

don't move, but turn towards ideal_yaw
Distance is for slight position adjustments needed by the animations
=============
*/
void ai_turn(edict_t *self, float dist)
{
	if (dist)
		M_walkmove(self, self->s.angles[YAW], dist);

	if (FindTarget(self))
		return;

	M_ChangeYaw(self);
}


/*

.enemy
Will be world if not currently angry at anyone.

.movetarget
The next path spot to walk toward.  If .enemy, ignore .movetarget.
When an enemy is killed, the monster will try to return to it's path.

.hunt_time
Set to time + something when the player is in sight, but movement straight for
him is blocked.  This causes the monster to use wall following code for
movement direction instead of sighting on the player.

.ideal_yaw
A yaw angle of the intended direction, which will be turned towards at up
to 45 deg / state.  If the enemy is in view and hunt_time is not active,
this will be the exact line towards the enemy.

.pausetime
A monster will leave it's stand state and head towards it's .movetarget when
time > .pausetime.

walkmove(angle, speed) primitive is all or nothing
*/

/*
=============
range

returns the range catagorization of an entity reletive to self
0	melee range, will become hostile even if back is turned
1	visibility and infront, or visibility and show hostile
2	infront and show hostile
3	only triggered by damage
=============
*/
int range(edict_t *self, edict_t *other)
{
	vec3_t	v;
	float	len;

	VectorSubtract(self->s.origin, other->s.origin, v);
	len = VectorLength(v);
	if (len < MELEE_DISTANCE)
		return RANGE_MELEE;
	if (len < 500)
		return RANGE_NEAR;
	if (len < 1000)
		return RANGE_MID;
	if (len < 2000)
		return RANGE_FAR;
	return RANGE_VERYFAR;
}




//============================================================================

void HuntTarget(edict_t *self)
{
	vec3_t	vec;

	self->goalentity = self->enemy;
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.stand(self);
	else
		self->monsterinfo.run(self);
	VectorSubtract(self->enemy->s.origin, self->s.origin, vec);
	self->ideal_yaw = vectoyaw(vec);
	// wait a while before first attack
	if (!(self->monsterinfo.aiflags & AI_STAND_GROUND))
		AttackFinished(self, 0.1);
}

void FoundTarget(edict_t *self)
{
	// let other monsters see this monster for a while
	if (self->enemy->client)
	{
		level.sight_entity = self;
		level.sight_entity_framenum = level.framenum;
		level.sight_entity->light_level = 128;
	}
	if (!(self->flags & FL_HEADSHOT))
	self->show_hostile = level.time + 2;		// wake up other monsters //bughunt

	VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);
	self->monsterinfo.trail_time = level.time;

	if (!self->combattarget)
	{
		HuntTarget(self);
		return;
	}

	self->goalentity = self->movetarget = G_PickTarget(self->combattarget);
	if (!self->movetarget)
	{
		self->goalentity = self->movetarget = self->enemy;
		HuntTarget(self);
		gi.dprintf("%s at %s, combattarget %s not found\n", self->classname, vtos(self->s.origin), self->combattarget);
		return;
	}

	// clear out our combattarget, these are a one shot deal
	self->combattarget = NULL;
	self->monsterinfo.aiflags |= AI_COMBAT_POINT;

	// clear the targetname, that point is ours!
	self->movetarget->targetname = NULL;
	self->monsterinfo.pausetime = 0;

	// run for it
	self->monsterinfo.run(self);
}


/*
===========
FindTarget

Self is currently not attacking anything, so try to find a target

Returns TRUE if an enemy was sighted

When a player fires a missile, the point of impact becomes a fakeplayer so
that monsters that see the impact will respond as if they had seen the
player.

To avoid spending too much time, only a single client (or fakeclient) is
checked each frame.  This means multi player games will have slightly
slower noticing monsters.
============
*/
qboolean FindTarget(edict_t *self)
{
	edict_t		*client;
	qboolean	heardit;
	int			r;

	if (self->monsterinfo.aiflags & AI_GOOD_GUY)
	{
		if (self->goalentity && self->goalentity->inuse && self->goalentity->classname)
		{
			if (strcmp(self->goalentity->classname, "target_actor") == 0)
				return false;
		}

		//FIXME look for monsters?
		return false;
	}

	// if we're going to a combat point, just proceed
	if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
		return false;

	// if the first spawnflag bit is set, the monster will only wake up on
	// really seeing the player, not another monster getting angry or hearing
	// something

	// revised behavior so they will wake up if they "see" a player make a noise
	// but not weapon impact/explosion noises

	heardit = false;
	if ((level.sight_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & 1 && rand() %2))
	{
		client = level.sight_entity;
		if (client->enemy == self->enemy)
		{
			return false;
		}
	}
	else if (level.sound_entity_framenum >= (level.framenum - 1))
	{
		client = level.sound_entity;
		heardit = true;
	}
	else if (!(self->enemy) && (level.sound2_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & 1 ||  rand() % 2))
	{
		client = level.sound2_entity;
		heardit = true;
	}
	else
	{
		client = level.sight_client;
		if (!client)
			return false;	// no clients to get mad at
	}

	// if the entity went away, forget it
	if (!client->inuse)
		return false;

	if (client == self->enemy)
		return true;	// JDC false;

	if (client->client)
	{
		if (client->flags & FL_NOTARGET)
			return false;
	}
	else if (client->svflags & SVF_MONSTER)
	{
		if (!client->enemy)
			return false;
		if (client->enemy->flags & FL_NOTARGET)
			return false;
	}
	else if (heardit)
	{
		if (client->owner->flags & FL_NOTARGET)
			return false;
	}
	else
		return false;

	if (!heardit)
	{
		r = range(self, client);

		if (r == RANGE_VERYFAR)
			return false;

		// this is where we would check invisibility

				// is client in an spot too dark to be seen?
		if (client->light_level <= 25 & VectorLength(client->velocity) < 100 && range(self, client) > RANGE_MELEE)
			return false;

		if (!visible(self, client))
		{
			return false;
		}

		if (r == RANGE_NEAR)
		{
			if (client->show_hostile < level.time && !infront(self, client))
			{
				return false;
			}
		}
		else if (r == RANGE_VERYFAR)
		{
			if (!infront(self, client))
			{
				return false;
			}
		}

		self->enemy = client;

		if (strcmp(self->enemy->classname, "player_noise") != 0)
		{
			self->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

			if (!self->enemy->client)
			{
				self->enemy = self->enemy->enemy;
				if (!self->enemy->client)
				{
					self->enemy = NULL;
					//self->velocity[2] += 500;
					return false;
				}
			}
		}
	}
	else	// heardit
	{
		vec3_t	temp;

		if (self->spawnflags & 1)
		{
			if (!visible(self, client))
				return false;
		}
		else
		{
			if (!gi.inPHS(self->s.origin, client->s.origin))
				return false;
		}

		VectorSubtract(client->s.origin, self->s.origin, temp);

		if (VectorLength(temp) > 2500)	// too far to hear
		{
			return false;
		}

		// check area portals - if they are different and not connected then we can't hear it
		//if (client->areanum != self->areanum)
		//	if (!gi.AreasConnected(self->areanum, client->areanum))
				//return false;

		self->ideal_yaw = vectoyaw(temp);
		M_ChangeYaw(self);

		// hunt the sound for a bit; hopefully find the real player
		self->monsterinfo.aiflags |= AI_SOUND_TARGET;
		self->enemy = client;
	}

	//
	// got one
	//
	FoundTarget(self);

	if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET) && (self->monsterinfo.sight))
		self->monsterinfo.sight(self, self->enemy);

	return true;
}


//=============================================================================

/*
============
FacingIdeal

============
*/
qboolean FacingIdeal(edict_t *self)
{
	float	delta;

	delta = anglemod(self->s.angles[YAW] - self->ideal_yaw);
	if (delta > 45 && delta < 315)
		return false;
	return true;
}


//=============================================================================
qboolean M_CheckClearShot(edict_t *self)
{
	vec3_t	spot1, spot2, dir, forward, right, angles, spot2backup;
	float	chance, dist_mult;
	trace_t	tr;
	/*if (!self->enemy)
		return false;
	if (self->enemy->health < 1)
	{
		self->enemy = NULL;
		return false;
	}*/

	if ((!visible(self, self->enemy) || !infront(self, self->enemy)) && self->health > 0 )
	{
		//gi.bprintf(PRINT_HIGH, "DON'T HAVE A CLEAR SHOT!!! not visible or not in front, or dead/dying! %f\n", level.time);
		return false;

	}



	// see if any entities are in the way of the shot
	VectorCopy(self->s.origin, spot1);
	spot1[2] += self->viewheight / 2;
	VectorCopy(self->enemy->s.origin, spot2);
	if (self->enemy->health > 0)
		spot2[2] += self->enemy->viewheight;
	dist_mult = 1 + (get_dist(self, self->enemy) * 0.01);

	//gi.bprintf(PRINT_HIGH, "spot2 = %s, time = %f\n", vtos(spot2), level.time);

	//gi.bprintf(PRINT_HIGH, "spot2 = %s, time = %f\n", vtos(spot2), level.time);
	//debug_trail(spot1, spot2);
	if ((strcmp(self->classname, "monster_soldier_light") == 0 || strcmp(self->classname, "monster_soldier") == 0 || strcmp(self->classname, "monster_soldier_ss") == 0) )
	{
		if(self->s.frame >= 272 && self->s.frame <= 307)
		spot1[2] -= 16.7 * 1.2;

		AngleVectors(self->s.angles, forward, right, NULL);
		G_ProjectSource(self->s.origin, monster_flash_offset[MZ2_SOLDIER_BLASTER_1], forward, right, spot1);
	}
	tr = gi.trace(spot1, NULL, NULL, spot2, self, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER | CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_WINDOW);
	if (tr.fraction < 1 && (tr.ent->svflags & SVF_MONSTER) && tr.ent != self->enemy)
	{
		//gi.bprintf(PRINT_HIGH, "DON'T HAVE A CLEAR SHOT!!! 1 %f\n", level.time);
		return false;
	}
	//debug_trail(spot1, spot2);
	// do we have a clear shot?
//	if (tr.ent != self->enemy)
//	{
		//gi.bprintf(PRINT_HIGH, "DON'T HAVE A CLEAR SHOT!!! 1 %f\n", level.time);

		/*if(tr.ent)
			gi.bprintf(PRINT_HIGH, "DON'T HAVE A CLEAR SHOT!!!11 start = %s, end = %s, origin = %s, classname = %s\n", vtos(spot1), vtos(spot2), vtos(self->enemy->s.origin), tr.ent->classname);

		else
			gi.bprintf(PRINT_HIGH, "DON'T HAVE A CLEAR SHOT!!!11 start = %s, end = %s, origin = %s\n", vtos(spot1), vtos(spot2), vtos(self->enemy->s.origin));
		debug_trail(spot1, spot2);*/

		//		return false;


		//	}

	if (range(self, self->enemy) == RANGE_MELEE)
	{
		//gi.bprintf(PRINT_HIGH, "M_CheckClearShot: CLEAR SHOT!, melee\n");
		return true;
	}


	//gi.bprintf(PRINT_HIGH, "M_CheckClearShot: SPOT2 = %s in\n", vtos(spot2));
	if (visible(self, self->enemy) && infront(self, self->enemy))
	{


		if (strcmp(self->classname, "monster_soldier_light") == 0)
			predict_shot(self, MONSTER_GUARD_BOLT_SPEED, spot2);
		else
			predict_shot(self, 0, spot2);
	}
	//gi.bprintf(PRINT_HIGH, "M_CheckClearShot: SPOT2 = %s out\n", vtos(spot2));
	VectorCopy(spot2, spot2backup);



	tr = gi.trace(spot1, NULL, NULL, spot2, self, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_WINDOW);
	if (tr.fraction < 1 && (tr.ent->svflags & SVF_MONSTER) && tr.ent != self->enemy)
	{
		//	if (tr.fraction < 1)
		///		gi.bprintf(PRINT_HIGH, "M_CheckClearShot: tr.fraction < 1\n");
		//	if((tr.ent->svflags & SVF_MONSTER || tr.ent->client))
			//	gi.bprintf(PRINT_HIGH, "M_CheckClearShot: (tr.ent->svflags & SVF_MONSTER || tr.ent->client)\n");
		//	if(tr.ent->health <= 0)
			//	gi.bprintf(PRINT_HIGH, "M_CheckClearShot: tr.ent->health <= 0, classname = %s\n", tr.ent->classname);
		//gi.bprintf(PRINT_HIGH, "DON'T HAVE A CLEAR SHOT!!! 2 %f\n", level.time);
		return false;
	}





	spot2[0] -= 32 * dist_mult;
	//debug_trail(spot1, spot2);
	tr = gi.trace(spot1, NULL, NULL, spot2, self, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_WINDOW);
	if (tr.fraction < 1 && (tr.ent->svflags & SVF_MONSTER) && tr.ent != self->enemy)
	{
		//gi.bprintf(PRINT_HIGH, "DON'T HAVE A CLEAR SHOT!!! 3 %f\n", level.time);
		return false;
	}

	spot2[0] += 64 * dist_mult;
	//debug_trail(spot1, spot2);
	tr = gi.trace(spot1, NULL, NULL, spot2, self, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_WINDOW);
	if (tr.fraction < 1 && (tr.ent->svflags & SVF_MONSTER) && tr.ent != self->enemy)
	{
		//gi.bprintf(PRINT_HIGH, "DON'T HAVE A CLEAR SHOT!!! 4 %f\n", level.time);
		return false;
	}

	spot2[1] -= 32 * dist_mult;
	//debug_trail(spot1, spot2);
	tr = gi.trace(spot1, NULL, NULL, spot2, self, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_WINDOW);
	if (tr.fraction < 1 && (tr.ent->svflags & SVF_MONSTER) && tr.ent != self->enemy)
	{
		//gi.bprintf(PRINT_HIGH, "DON'T HAVE A CLEAR SHOT!!! 5 %f\n", level.time);
		return false;
	}

	spot2[1] += 64 * dist_mult;
	//debug_trail(spot1, spot2);
	tr = gi.trace(spot1, NULL, NULL, spot2, self, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_WINDOW);
	if (tr.fraction < 1 && (tr.ent->svflags & SVF_MONSTER) && tr.ent != self->enemy)
	{
		//gi.bprintf(PRINT_HIGH, "DON'T HAVE A CLEAR SHOT!!! 6 %f\n", level.time);
		return false;
	}
	

	//gi.bprintf(PRINT_HIGH, "M_CheckClearShot: CLEAR SHOT!, did all checks\n");
	return true;
}
qboolean M_CheckAttack(edict_t *self)
{
	vec3_t	spot1, spot2;
	float	chance;
	trace_t	tr;
	//gi.bprintf(PRINT_HIGH, "M_CheckAttack:																																																																			 \n");
	if (self->enemy->health > 0)
	{
		/*// see if any entities are in the way of the shot
		VectorCopy(self->s.origin, spot1);
		spot1[2] += self->viewheight;
		VectorCopy(self->enemy->s.origin, spot2);
		spot2[2] += self->enemy->viewheight;

		tr = gi.trace(spot1, NULL, NULL, spot2, self, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_WINDOW);

		// do we have a clear shot?
		if (tr.ent != self->enemy)
			return false;*/
		if (!M_CheckClearShot(self))
			return false;
	}

	// melee attack
	if (enemy_range == RANGE_MELEE)
	{
		// don't always melee in easy mode
		if (skill->value == 0 && (rand() & 3))
			return false;
		if (self->monsterinfo.melee)
			self->monsterinfo.attack_state = AS_MELEE;
		else
			self->monsterinfo.attack_state = AS_MISSILE;
		return true;
	}

	// missile attack
	if (!self->monsterinfo.attack)
		return false;

	if (level.time < self->monsterinfo.attack_finished)
		return false;

	//if (enemy_range == RANGE_FAR)
	//	return false;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		chance = 0.5;
	}
	else if (enemy_range == RANGE_MELEE)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_NEAR)
	{
		chance = 0.3;
	}
	else if (enemy_range == RANGE_MID)
	{
		chance = 0.2;
	}
	else if (enemy_range >= RANGE_FAR)
	{
		chance = 0.1;
	}
	else
	{
		return false;
	}

	if (skill->value == 0)
		chance *= 0.5;
	else if (skill->value >= 2)
		chance *= 1.5;

	if (random() < chance)
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + 2 * random();
		return true;
	}

	if (self->flags & FL_FLY)
	{
		if (random() < 0.3)
			self->monsterinfo.attack_state = AS_SLIDING;
		else
			self->monsterinfo.attack_state = AS_STRAIGHT;
	}

	return false;
}


/*
=============
ai_run_melee

Turn and close until within an angle to launch a melee attack
=============
*/
void ai_run_melee(edict_t *self)
{
	self->ideal_yaw = enemy_yaw;
	M_ChangeYaw(self);

	if (FacingIdeal(self))
	{
		self->monsterinfo.melee(self);
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
}


/*
=============
ai_run_missile

Turn in place until within an angle to launch a missile attack
=============
*/
void ai_run_missile(edict_t *self)
{
	self->ideal_yaw = enemy_yaw;
	M_ChangeYaw(self);

	if (FacingIdeal(self))
	{
		self->monsterinfo.attack(self);
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
};


/*
=============
ai_run_slide

Strafe sideways, but stay at aproximately the same range
=============
*/
void ai_run_slide(edict_t *self, float distance)
{
	float	ofs;

	self->ideal_yaw = enemy_yaw;
	M_ChangeYaw(self);

	if (self->monsterinfo.lefty)
		ofs = 90;
	else
		ofs = -90;

	if (M_walkmove(self, self->ideal_yaw + ofs, distance))
		return;

	self->monsterinfo.lefty = 1 - self->monsterinfo.lefty;
	M_walkmove(self, self->ideal_yaw - ofs, distance);
}


/*
=============
ai_checkattack

Decides if we're going to attack or do something else
used by ai_run and ai_stand
=============
*/
qboolean ai_checkattack(edict_t *self, float dist)
{
	vec3_t		temp;
	qboolean	hesDeadJim;
	//gi.bprintf(PRINT_HIGH, "ai_checkattack:\n");
	// this causes monsters to run blindly to the combat point w/o firing
	if (self->goalentity && random() + self->monsterinfo.aggression > 1.5)
	{
		if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
			return false;

		if (self->monsterinfo.aiflags & AI_SOUND_TARGET)
		{
			if ((level.time - self->enemy->teleport_time) > 5.0)
			{
				if (self->goalentity == self->enemy)
					if (self->movetarget)
						self->goalentity = self->movetarget;
					else
						self->goalentity = NULL;
				self->monsterinfo.aiflags &= ~AI_SOUND_TARGET;
				if (self->monsterinfo.aiflags & AI_TEMP_STAND_GROUND)
					self->monsterinfo.aiflags &= ~(AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
			}
			else
			{
				if (!(self->flags & FL_HEADSHOT))
					self->show_hostile = level.time + 1;
				return false;
			}
		}
	}

	enemy_vis = false;
	// see if the enemy is dead
	hesDeadJim = false;
	if ((!self->enemy) || (!self->enemy->inuse) || self->enemy->s.effects & EF_GIB)
	{
		hesDeadJim = true;
	}
	else if (self->monsterinfo.aiflags & AI_MEDIC)
	{
		if (self->enemy->health > 0)
		{
			hesDeadJim = true;
			self->monsterinfo.aiflags &= ~AI_MEDIC;
		}
	}
	else
	{
		//gi.bprintf(PRINT_HIGH, "ai_checkattack: dead?\n");

		if (self->enemy->s.effects & EF_GIB)
			hesDeadJim = true;
		else if (self->monsterinfo.aiflags & AI_BRUTAL)
		{
			if (self->enemy->health <= -30)
				hesDeadJim = true;
		}
		else
		{
			if (self->enemy->health <= 0)
				hesDeadJim = true;
		}
	}

	if (hesDeadJim)
	{
		//gi.bprintf(PRINT_HIGH, "ai_checkattack: PLAYER IS DEAD!!!\n");
		self->enemy = NULL;
		// FIXME: look all around for other targets
		if (self->oldenemy && self->oldenemy->health > 0)
		{
			self->enemy = self->oldenemy;
			self->oldenemy = NULL;
			HuntTarget(self);
		}
		else
		{
			if (self->movetarget)
			{
				self->goalentity = self->movetarget;
				self->monsterinfo.walk(self);
			}
			else
			{
				// we need the pausetime otherwise the stand code
				// will just revert to walking with no target and
				// the monsters will wonder around aimlessly trying
				// to hunt the world entity
				self->monsterinfo.pausetime = level.time + 100000000;
				self->monsterinfo.stand(self);
			}
			return true;
		}
	}
	if (!(self->flags & FL_HEADSHOT))
	self->show_hostile = level.time + 1;		// wake up other monsters

// check knowledge of enemy
	enemy_vis = visible(self, self->enemy);
	if (enemy_vis)
	{
		self->monsterinfo.search_time = level.time + 5;
		VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);
		//VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting2);
	}

	// look for other coop players here
	//	if (coop && self->monsterinfo.search_time < level.time)
	//	{
	//		if (FindTarget (self))
	//			return true;
	//	}

	enemy_infront = infront(self, self->enemy);
	enemy_range = range(self, self->enemy);
	if (enemy_vis)
		VectorSubtract(self->enemy->s.origin, self->s.origin, temp);
	//gi.bprintf(PRINT_HIGH, "see enemy? = %i\n", enemy_vis);
	enemy_yaw = vectoyaw(temp);

	//gi.bprintf(PRINT_HIGH, "AI_CheckAttack:\n");
	// JDC self->ideal_yaw = enemy_yaw;

	if (self->monsterinfo.attack_state == AS_MISSILE)
	{
		ai_run_missile(self);
		return true;
	}
	if (self->monsterinfo.attack_state == AS_MELEE && self->monsterinfo.aggression + random() > 0.75)
	{
		ai_run_melee(self);
		return true;
	}

	// if enemy is not currently visible, we will never attack
	if (!enemy_vis)
		return false;

	return self->monsterinfo.checkattack(self);
}

/*
=============
ai_run

The monster has an enemy it is trying to kill
=============
*/
void ai_run(edict_t *self, float dist)
{
	vec3_t		v;
	edict_t		*tempgoal;
	edict_t		*save;
	qboolean	new;
	edict_t		*marker;
	float		d1, d2;
	trace_t		tr;
	vec3_t		v_forward, v_right;
	float		left, center, right;
	vec3_t		left_target, right_target;
	if (!self->enemy) //FIXME how do we get here without an enemy???
	{
		//gi.bprintf(PRINT_HIGH, "ai_run: NO ENEMY!!!\n");
		//return;

			//gi.bprintf(PRINT_HIGH, "ai_run: FIXED NO ENEMY!!!, goalentity = %s \n", self->goalentity->classname);
		if (level.sight_client && level.sight_client->health > 0)
			self->enemy = level.sight_client;
		else
		{
			self->monsterinfo.walk(self);
			return;
		}


	}

	//gi.bprintf(PRINT_HIGH, "ai_run:\n");
	// if we're going to a combat point, just proceed
	if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
	{
		//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: self->monsterinfo.aiflags & AI_COMBAT_POINT\n");
		M_MoveToGoal(self, dist);
		return;
	}

	if (self->monsterinfo.aiflags & AI_SOUND_TARGET)
	{
		VectorSubtract(self->s.origin, self->enemy->s.origin, v);
		if (VectorLength(v) < 128)
		{
			//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: self->monsterinfo.aiflags & AI_SOUND_TARGET\n");
			self->monsterinfo.aiflags |= (AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
			self->monsterinfo.stand(self);
			return;
		}

		M_MoveToGoal(self, dist);

		if (!FindTarget(self))
		{
			//gi.bprintf(PRINT_HIGH, "SOLDIER FIRE: !FindTarget(self)\n");
			return;
		}
			
	}

	if (ai_checkattack(self, dist))
		return;

	if (self->monsterinfo.attack_state == AS_SLIDING)
	{
		ai_run_slide(self, dist);
		return;
	}

	if (enemy_vis)
	{
		//		if (self.aiflags & AI_LOST_SIGHT)
		//			dprint("regained sight\n");
		M_MoveToGoal(self, dist);
		self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
		VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);
		self->monsterinfo.trail_time = level.time;
		return;
	}

	// coop will change to another enemy if visible
	if (coop->value)
	{	// FIXME: insane guys get mad with this, which causes crashes!
		if (FindTarget(self))
			return;
	}

	if ((self->monsterinfo.search_time) && (level.time > (self->monsterinfo.search_time + 20)))
	{
		M_MoveToGoal(self, dist);
		self->monsterinfo.search_time = 0;
		//		dprint("search timeout\n");
		return;
	}

	save = self->goalentity;
	tempgoal = G_Spawn();
	self->goalentity = tempgoal;

	new = false;
	//return;
	if (!(self->monsterinfo.aiflags & AI_LOST_SIGHT))
	{
		// just lost sight of the player, decide where to go first
//		dprint("lost sight of player, last seen at "); dprint(vtos(self.last_sighting)); dprint("\n");
		self->monsterinfo.aiflags |= (AI_LOST_SIGHT | AI_PURSUIT_LAST_SEEN);
		self->monsterinfo.aiflags &= ~(AI_PURSUE_NEXT | AI_PURSUE_TEMP);
		new = true;
	}

	if (self->monsterinfo.aiflags & AI_PURSUE_NEXT)
	{
		self->monsterinfo.aiflags &= ~AI_PURSUE_NEXT;
		//		dprint("reached current goal: "); dprint(vtos(self.origin)); dprint(" "); dprint(vtos(self.last_sighting)); dprint(" "); dprint(ftos(vlen(self.origin - self.last_sighting))); dprint("\n");

				// give ourself more time since we got this far
		self->monsterinfo.search_time = level.time + 5;

		if (self->monsterinfo.aiflags & AI_PURSUE_TEMP)
		{
			//			dprint("was temp goal; retrying original\n");
			self->monsterinfo.aiflags &= ~AI_PURSUE_TEMP;
			marker = NULL;
			VectorCopy(self->monsterinfo.saved_goal, self->monsterinfo.last_sighting);
			new = true;
		}
		else if (self->monsterinfo.aiflags & AI_PURSUIT_LAST_SEEN)
		{
			self->monsterinfo.aiflags &= ~AI_PURSUIT_LAST_SEEN;
			marker = PlayerTrail_PickFirst(self);
		}
		else
		{
			marker = PlayerTrail_PickNext(self);
		}

		if (marker)
		{
			VectorCopy(marker->s.origin, self->monsterinfo.last_sighting);
			self->monsterinfo.trail_time = marker->timestamp;
			self->s.angles[YAW] = self->ideal_yaw = marker->s.angles[YAW];
			//			dprint("heading is "); dprint(ftos(self.ideal_yaw)); dprint("\n");

			//			debug_drawline(self.origin, self.last_sighting, 52);
			new = true;
		}
	}

	VectorSubtract(self->s.origin, self->monsterinfo.last_sighting, v);
	d1 = VectorLength(v);
	if (d1 <= dist)
	{
		self->monsterinfo.aiflags |= AI_PURSUE_NEXT;
		dist = d1;
	}

	VectorCopy(self->monsterinfo.last_sighting, self->goalentity->s.origin);

	if (new)
	{
		//		gi.dprintf("checking for course correction\n");

		tr = gi.trace(self->s.origin, self->mins, self->maxs, self->monsterinfo.last_sighting, self, MASK_PLAYERSOLID);
		if (tr.fraction < 1 || random() > 0.25)
		{
			int rnum = 1 + rand() % 10;
			VectorSubtract(self->goalentity->s.origin, self->s.origin, v);
			d1 = VectorLength(v);
			center = tr.fraction;
			d2 = d1 * ((center + 1) / rnum);
			self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
			AngleVectors(self->s.angles, v_forward, v_right, NULL);


			VectorSet(v, d2, -8 * rnum, 0);
			G_ProjectSource(self->s.origin, v, v_forward, v_right, left_target);
			tr = gi.trace(self->s.origin, self->mins, self->maxs, left_target, self, MASK_PLAYERSOLID);
			left = tr.fraction;

			VectorSet(v, d2, 8 * rnum, 0);
			G_ProjectSource(self->s.origin, v, v_forward, v_right, right_target);
			tr = gi.trace(self->s.origin, self->mins, self->maxs, right_target, self, MASK_PLAYERSOLID);
			right = tr.fraction;

			center = (d1*center) / d2;
			if (left >= center && left > right)
			{
				if (left < 1)
				{
					VectorSet(v, d2 * left * 0.5, -8 * rnum, 0);
					G_ProjectSource(self->s.origin, v, v_forward, v_right, left_target);
					//					gi.dprintf("incomplete path, go part way and adjust again\n");
				}
				VectorCopy(self->monsterinfo.last_sighting, self->monsterinfo.saved_goal);
				self->monsterinfo.aiflags |= AI_PURSUE_TEMP;
				VectorCopy(left_target, self->goalentity->s.origin);
				VectorCopy(left_target, self->monsterinfo.last_sighting);
				VectorSubtract(self->goalentity->s.origin, self->s.origin, v);
				self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
				//				gi.dprintf("adjusted left\n");
				//				debug_drawline(self.origin, self.last_sighting, 152);
			}
			else if (right >= center && right > left || random() > 0.25)
			{
				if (right < 1)
				{
					VectorSet(v, d2 * right * 0.5, 8 * rnum, 0);
					G_ProjectSource(self->s.origin, v, v_forward, v_right, right_target);
					//					gi.dprintf("incomplete path, go part way and adjust again\n");
				}
				VectorCopy(self->monsterinfo.last_sighting, self->monsterinfo.saved_goal);
				self->monsterinfo.aiflags |= AI_PURSUE_TEMP;
				VectorCopy(right_target, self->goalentity->s.origin);
				VectorCopy(right_target, self->monsterinfo.last_sighting);
				VectorSubtract(self->goalentity->s.origin, self->s.origin, v);
				self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
				//				gi.dprintf("adjusted right\n");
				//				debug_drawline(self.origin, self.last_sighting, 152);
			}
		}
		//		else gi.dprintf("course was fine\n");
	}

	M_MoveToGoal(self, dist);

	G_FreeEdict(tempgoal);

	if (self)
		self->goalentity = save;
}
