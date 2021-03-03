// g_ai.c

#include "g_local.h"

qboolean npc_FindTarget (edict_t *self);
extern cvar_t	*maxclients;

qboolean ai_checkattack (edict_t *self, float dist);

//============================================================================

/*
=============
ai_charge

Turns towards target and advances
Use this call with a distnace of 0 to replace ai_face
==============
*/
void ai_npc_charge (edict_t *self, float dist)
{
	vec3_t	v;

	//gi.bprintf (PRINT_MEDIUM, "ai_charge\n");

	if (self->enemy == NULL)
	{
		gi.bprintf (PRINT_MEDIUM, "ai_charge error, enemy is NULL\n");
	}
	VectorSubtract (self->enemy->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);
	M_ChangeYaw (self);

	if (dist)
	{
		//gi.bprintf (PRINT_MEDIUM, "ai_charge moving\n");
		M_walkmove (self, self->s.angles[YAW], dist);
	}

	//gi.bprintf (PRINT_MEDIUM, "ai_charge end function\n");
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

void npc_FoundTarget (edict_t *self)
{
	// let other monsters see this monster for a while
	if (self->enemy->client)
	{
		level.sight_entity = self;
		level.sight_entity_framenum = level.framenum;
		level.sight_entity->light_level = 128;
	}

	self->show_hostile = level.time + 1;		// wake up other monsters

	VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);
	self->monsterinfo.trail_time = level.time;

	if (!self->combattarget)
	{
//		gi.dprintf("no combat_target\n");
		return;
	}

	self->action_target = G_PickTarget(self->combattarget);
	self->path_node = NO_NODES;
	self->goalentity = self->movetarget = NULL;

	// clear out our combattarget, these are a one shot deal
	self->combattarget = NULL;
//	self->monsterinfo.aiflags |= AI_COMBAT_POINT;

	// clear the targetname, that point is ours!
//	self->movetarget->targetname = NULL;
	self->monsterinfo.pausetime = 0;

	// run for it
	// make sure combat target for walking monsters is not in water
	self->monsterinfo.run (self);
}
//=============================================================================

//
// NEW STUFF FOR AI
//

/*
=================
AI_AlertStroggs

This routine will alert other NEARBY Stroggs if a Strogg first sees an enemy

=================
*/
/*
void AI_npc_AlertStroggs (edict_t *self)
{

	float	dist;
	edict_t *strogg=NULL;
	
	if (self->enemy->flags & FL_NOTARGET)
		return;

	//CaRRaC - holster AI
	if(self->enemy->holstered == 1)
	{
		return;
	}

	if (skill->value == 1)
		dist = 650;
	else if (skill->value == 2)
		dist = 1000;
	else
		dist = 1250 + (skill->value) * 75;

	while ((strogg = findradius(strogg, self->s.origin, dist)) != NULL)
	{
		if (!(strogg->svflags & SVF_MONSTER))
			continue;

		if(!strcmp(strogg->classname, "npc_officer"))
			continue;

		if (strogg->health <= 0 || !(strogg->takedamage))
			continue;
	
		if (strogg == self)
			continue;
	
		if (strogg->client)
			continue;
	
		if (strogg->enemy)
			continue;

		if(!(visible(strogg, self)))
			continue;
		else 	if(!(visible(strogg, self->enemy)))
			continue;

		if (strogg->attackflags & ATTACK_NO_ALERT)
			continue;

		strogg->enemy = self->enemy;
		FoundTarget (strogg);
	}
}*/

void AI_npc_AlertStroggs (edict_t *self)
{

	float	dist;
	edict_t *strogg=NULL;
	
	if (self->enemy->flags & FL_NOTARGET)
		return;

	//CaRRaC - holster AI
	if(self->enemy->holstered == 1)
	{
		return;
	}

	if (skill->value == 1)
		dist = 650;
	else if (skill->value == 2)
		dist = 1000;
	else
		dist = 1250 + (skill->value) * 75;

	while ((strogg = findradius(strogg, self->s.origin, dist)) != NULL)
	{
		if (!(strogg->svflags & SVF_MONSTER))
			continue;

		if(!strcmp(strogg->classname, "npc_officer"))
			continue;

		if (strogg->health <= 0 || !(strogg->takedamage))
			continue;
	
		if (strogg == self)
			continue;
	
		if (strogg->client)
			continue;
	
		if (strogg->enemy)
			continue;

		if(!(visible(strogg, self)))
			continue;
		else 	if(!(visible(strogg, self->enemy)))
			continue;

		if (strogg->attackflags & ATTACK_NO_ALERT)
			continue;

		strogg->enemy = self->enemy;
//		FoundTarget (strogg);
	}
}

#if 0
qboolean npc_FindTarget (edict_t *self)
{
	edict_t		*client;
	qboolean	heardit;
	int			r;

	if (self->monsterinfo.aiflags & AI_GOOD_GUY || self->monsterinfo.aiflags & AI_NPC_PASSIVE)
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
	if ((level.sight_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & 1) )
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
	else if (!(self->enemy) && (level.sound2_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & 1) )
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

//	if (client == self->enemy)
//		return true;	// JDC false;

	if (client == self->enemy)
		return false;


	if (client->client)
	{
		if (client->flags & FL_NOTARGET)
			return false;
		//CaRRaC - holster AI
		if(client->holstered == 1)
		{
                        return false;
		}
	}
	else if (client->svflags & SVF_MONSTER)
	{
		if (!client->enemy)
			return false;
		if (client->enemy->flags & FL_NOTARGET)
			return false;
		//CaRRaC - holster AI
		if(client->enemy->holstered == 1)
		{
                        return false;
		}
	}
	else if (heardit)
	{
		if (client->owner->flags & FL_NOTARGET)
			return false;
		//CaRRaC - holster AI
		if(client->owner->holstered == 1)
		{
                        return false;
		}
	}
	else
		return false;

	if (!heardit)
	{
		r = range (self, client);

		if (r == RANGE_FAR)
			return false;

		// this is where we would check invisibility

		// is client in an spot too dark to be seen?
		if (client->light_level <= 5)
			return false;

		if (!visible (self, client))
		{
			return false;
		}

		if (r == RANGE_NEAR)
		{
			if (client->show_hostile < level.time && !infront (self, client))
			{
				return false;
			}
		}
		else if (r == RANGE_MID)
		{
			if (!infront (self, client))
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
			if (!visible (self, client))
				return false;
		}
		else
		{
			if (!gi.inPHS(self->s.origin, client->s.origin))
				return false;
		}

		VectorSubtract (client->s.origin, self->s.origin, temp);

		if (VectorLength(temp) > 1000)	// too far to hear
		{
			return false;
		}

		// check area portals - if they are different and not connected then we can't hear it
		if (client->areanum != self->areanum)
			if (!gi.AreasConnected(self->areanum, client->areanum))
				return false;

		self->ideal_yaw = vectoyaw(temp);
		M_ChangeYaw (self);

		// hunt the sound for a bit; hopefully find the real player
		self->monsterinfo.aiflags |= AI_SOUND_TARGET;
		self->enemy = client;
	}

//
// got one
//
	FoundTarget (self);

	// Now alert nearby Stroggs
	AI_AlertStroggs (self);
	self->path_node = NO_NODES;
	
	if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET) && (self->monsterinfo.sight))
		self->monsterinfo.sight (self, self->enemy);

	return true;
}
#endif

qboolean npc_FindTarget (edict_t *self)
{
	edict_t		*client;
	int			r;

	if (self->monsterinfo.aiflags & AI_GOOD_GUY || self->monsterinfo.aiflags & AI_NPC_PASSIVE)
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

	if ((level.sight_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & 1) )
	{
		client = level.sight_entity;
		if (client->enemy == self->enemy)
		{
			return false;
		}
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
		return false;

	if (client->client)
	{
		if (client->flags & FL_NOTARGET)
			return false;
		//CaRRaC - holster AI
		if(client->holstered == 1)
		{
                        return false;
		}
	}
	else if (client->svflags & SVF_MONSTER)
	{
		if (!client->enemy)
			return false;
		if (client->enemy->flags & FL_NOTARGET)
			return false;
		//CaRRaC - holster AI
		if(client->enemy->holstered == 1)
		{
                        return false;
		}
	}
	else
		return false;

	r = range (self, client);

	if (r == RANGE_FAR)
		return false;

// this is where we would check invisibility

	// is client in an spot too dark to be seen?
	if (client->light_level <= 5)
		return false;

	if (!visible (self, client))
	{
		return false;
	}

	if (r == RANGE_NEAR)
	{
		if (client->show_hostile < level.time && !infront (self, client))
		{
			return false;
		}
	}
	else if (r == RANGE_MID)
	{
		if (!infront (self, client))
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
				return false;
			}
		}
	}

//
// got one
//
//	FoundTarget (self);
	npc_FoundTarget(self);

	// Now alert nearby Stroggs
	AI_npc_AlertStroggs (self);
	
	if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET) && (self->monsterinfo.sight))
		self->monsterinfo.sight (self, self->enemy);

	return true;
}

void ai_npc_stand (edict_t *self, float dist)
{
	vec3_t	v;

	make_node (self);

	if(self->action_target && self->monsterinfo.pausetime < level.time)
	{
		self->monsterinfo.walk(self);
		self->monsterinfo.pausetime = 0;
		return;
	}

	if (dist)
		M_walkmove (self, self->s.angles[YAW], dist);

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		if (self->enemy)
		{
			VectorSubtract (self->enemy->s.origin, self->s.origin, v);
			self->ideal_yaw = vectoyaw(v);
			if (self->s.angles[YAW] != self->ideal_yaw && self->monsterinfo.aiflags & AI_TEMP_STAND_GROUND)
			{
				self->monsterinfo.aiflags &= ~(AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
				self->monsterinfo.run (self);
			}
			M_ChangeYaw (self);
			ai_checkattack (self, 0);
		}
		else
			npc_FindTarget (self);
		return;
	}

	if (npc_FindTarget (self))
		return;
	
	if (level.time > self->monsterinfo.pausetime)
	{
		self->monsterinfo.walk (self);
		return;
	}

	if (!(self->spawnflags & 1) && (self->monsterinfo.idle) && (level.time > self->monsterinfo.idle_time))
	{
		if (self->monsterinfo.idle_time)
		{
			self->monsterinfo.idle (self);
			self->monsterinfo.idle_time = level.time + 15 + random() * 15;
		}
		else
		{
			self->monsterinfo.idle_time = level.time + random() * 15;
		}
	}
}

void ai_npc_walk2 (edict_t *self, float dist)
{
	M_MoveToGoal (self, dist);

	// check for noticing a player
	if (npc_FindTarget (self))
		return;

	if ((self->monsterinfo.search) && (level.time > self->monsterinfo.idle_time))
	{
		if (self->monsterinfo.idle_time)
		{
			self->monsterinfo.search (self);
			self->monsterinfo.idle_time = level.time + 15 + random() * 15;
		}
		else
		{
			self->monsterinfo.idle_time = level.time + random() * 15;
		}
	}
}

void ai_npc_run_nopath (edict_t *self, float dist);

void ai_npc_walk (edict_t *self, float dist)
{

	vec3_t		v;
	edict_t		*marker, *tempgoal, *save;
	qboolean	newgoal;
	float		d1, d2;
	trace_t		tr;
	vec3_t		v_forward, v_right;
	float		left, center, right, length;
	vec3_t		left_target, right_target;
	qboolean	path_updated;

	if(!self->action_target)
	{
		return;
	}

	// if we're going to a combat point, just proceed
	if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
	{
//		spawn_temp(self->goalentity->s.origin, 1);
//		gi.dprintf("movetarget, returning\n");
		M_MoveToGoal (self, dist);
		return;
	}

	if(npc_FindTarget (self))
	{
//		gi.dprintf("found target %s\n", self->action_target->classname);
//		spawn_temp(self->action_target->s.origin, 1);
		return;
	}

	if (self->monsterinfo.search_time)
	{
		// Strogg will take 10 seconds to head towards next node
		// if time limit is up, it will recalculate path again
		// the very next frame
		if (level.time > (self->monsterinfo.search_time + 10))
		{
//			gi.dprintf("alright, I ran out of time\n");
			if(!visible(self, self->action_target))
			{
//				gi.dprintf("and its not visible\n");
				if (self->goalentity)
					M_MoveToGoal (self, dist);
				self->monsterinfo.search_time = 0;
				self->path_node = NO_NODES;

				if (!Q_stricmp (self->goalentity->classname, "path_node"))
				{
					self->goalentity->think = G_FreeEdict;
					self->goalentity->nextthink = level.time + 0.1;
				}			
				return;
			}
		}
	}

	if (self->goalentity)
	{
		if (self->path_node != NO_NODES)
		{
			VectorSubtract (self->s.origin, self->goalentity->s.origin, v);
			length = VectorLength(v);

			if (length > 33)
			{
				// has a path_node goal to get to but hasn't reached there yet
				// so keep going
				M_MoveToGoal (self, dist);
				return;
			}
			else
			{
				if (!Q_stricmp (self->goalentity->classname, "path_node"))
				{
					if(self->path_node == self->action_target->path_node)
					{
//						gi.dprintf("yep, it is\n");
						self->goalentity = self->action_target;
						self->movetarget = self->action_target;
						VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
						self->ideal_yaw = vectoyaw (v);
						self->monsterinfo.aiflags |= AI_COMBAT_POINT;
						self->path_node = NO_NODES;
						if (!(!Q_stricmp (self->goalentity->classname, "point_combat") && (self->goalentity->spawnflags & 1)))
						{
							self->action_target = G_PickTarget(self->action_target->target);
						}
//						self->monsterinfo.stand(self);
						return;
					}
					self->goalentity->think = G_FreeEdict;
					self->goalentity->nextthink = level.time + 0.1;
				}
			}
		}
	}

	// FIXME: Crash problem occurs before this! So path routine okay.
	
/*	if (self->check_path_time > level.time)
	{
		//gi.bprintf (PRINT_MEDIUM, "ai_run, pathtime set, heading to ai_run_nopath\n");
		ai_run_nopath (self, dist);
		return;
	}
*/

	if (self->path_node != NO_NODES)
	{
		if (self->goalentity == NULL)
		{
			self->path_node = NO_NODES;
		}
	}

	// check if previous goal was temporary goal, path_node is set and goal was a 
	// path_node
	if (self->monsterinfo.aiflags & AI_PURSUE_TEMP && self->path_node != NO_NODES
		&& !(Q_stricmp (self->goalentity->classname, "path_node")) )
	{
		self->monsterinfo.aiflags &= ~AI_PURSUE_TEMP;

//		marker = NULL;
		//VectorCopy (self->monsterinfo.saved_goal, self->monsterinfo.last_sighting);
		marker = G_Spawn ();
		marker->movetype = MOVETYPE_NONE;
		marker->solid = SOLID_NOT;
		VectorClear (marker->mins);
		VectorClear (marker->maxs);
		marker->think = G_FreeEdict;
		marker->nextthink = level.time + 30;
		marker->classname = "path_node";
		marker->timestamp = level.time;
		VectorCopy (node_list[self->path_node], marker->s.origin);
		VectorSubtract (marker->s.origin, self->s.origin, v);
		marker->s.angles[YAW] = vectoyaw (v);
		gi.linkentity (marker);

		self->goalentity = marker;
		self->ideal_yaw = marker->s.angles[YAW];
//		self->s.angles[YAW] = self->ideal_yaw = marker->s.angles[YAW];
		M_ChangeYaw (self);
		
		self->monsterinfo.search_time = level.time + FRAMETIME;
		newgoal = true;	
	}
	else if (self->path_node == NO_NODES)
	{
//		gi.dprintf("Finding path\n");
		find_path (self);
		path_updated = true;
		self->monsterinfo.aiflags &= ~AI_PURSUE_TEMP;
	}
	else
	{
		find_next_node (self);
		path_updated = true;
		self->monsterinfo.aiflags &= ~AI_PURSUE_TEMP;
	}

	// path_updated is a check to see if pathing routine was executed
	// if pathing routine was executed and path found, set goal otherwise
	// execute original AI_RUN
	// if routine was not executed then this means that goal is set to
	// original goal prior to issuing temporary goal.

	if (path_updated)
	{
		if (self->path_node != NO_NODES)
		{
			marker = G_Spawn ();
			marker->movetype = MOVETYPE_NONE;
			marker->solid = SOLID_NOT;
			VectorClear (marker->mins);
			VectorClear (marker->maxs);
			marker->think = G_FreeEdict;
			marker->nextthink = level.time + 30;
			marker->classname = "path_node";
			marker->timestamp = 0;
			VectorCopy (node_list[self->path_node], marker->s.origin);
			VectorSubtract (marker->s.origin, self->s.origin, v);
			marker->s.angles[YAW] = vectoyaw (v);
			gi.linkentity (marker);
	
		//	gi.bprintf (PRINT_MEDIUM, "Marker set, moving to node #%d\n", self->path_node);
	
			self->goalentity = marker;
			self->ideal_yaw = marker->s.angles[YAW];
//			self->s.angles[YAW] = self->ideal_yaw = marker->s.angles[YAW];
			M_ChangeYaw (self);

			self->monsterinfo.search_time = level.time + FRAMETIME;
			
			newgoal = true;
		}
		else
		{
//			gi.dprintf("breadcrumbing\n");
			// for the next 5 to 10 seconds, game will run using id's
			// "breadcrumb" chase code.
			self->check_path_time = level.time + 5 + ((rand() % 5));

			self->monsterinfo.aiflags |= AI_PURSUE_NEXT;
			self->monsterinfo.aiflags |= AI_LOST_SIGHT;
			self->monsterinfo.aiflags |= AI_PURSUIT_LAST_SEEN;

		//	gi.bprintf (PRINT_MEDIUM, "ai_run NO PATH FOUND, Heading towards ai_run_nopath\n");
//			ai_run_nopath (self, dist);
			return;
		}
	}

	save = self->goalentity;
	tempgoal = G_Spawn();
	self->goalentity = tempgoal;

	VectorCopy (marker->s.origin, self->monsterinfo.last_sighting);
	VectorCopy (self->monsterinfo.last_sighting, self->goalentity->s.origin);

	if (newgoal)
	{
		tr = gi.trace(self->s.origin, self->mins, self->maxs, self->monsterinfo.last_sighting, self, MASK_PLAYERSOLID);
		if (tr.fraction < 1)
		{
			VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
			d1 = VectorLength(v);
			center = tr.fraction;
			d2 = d1 * ((center+1)/2);
			self->ideal_yaw = vectoyaw(v);
//			self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
			AngleVectors(self->s.angles, v_forward, v_right, NULL);

			VectorSet(v, d2, -16, 0);
			G_ProjectSource (self->s.origin, v, v_forward, v_right, left_target);
			tr = gi.trace(self->s.origin, self->mins, self->maxs, left_target, self, MASK_PLAYERSOLID);
			left = tr.fraction;

			VectorSet(v, d2, 16, 0);
			G_ProjectSource (self->s.origin, v, v_forward, v_right, right_target);
			tr = gi.trace(self->s.origin, self->mins, self->maxs, right_target, self, MASK_PLAYERSOLID);
			right = tr.fraction;

			center = (d1*center)/d2;
			if (left >= center && left > right)
			{
				if (left < 1)
				{
					VectorSet(v, d2 * left * 0.5, -16, 0);
					G_ProjectSource (self->s.origin, v, v_forward, v_right, left_target);
				}
				VectorCopy (self->monsterinfo.last_sighting, self->monsterinfo.saved_goal);
				self->monsterinfo.aiflags |= AI_PURSUE_TEMP;
				VectorCopy (left_target, self->goalentity->s.origin);
				VectorCopy (left_target, self->monsterinfo.last_sighting);
				VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
				self->ideal_yaw = vectoyaw(v);
//				self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
//				debug_drawline(self.origin, self.last_sighting, 152);
			}
			else if (right >= center && right > left)
			{
				if (right < 1)
				{
					VectorSet(v, d2 * right * 0.5, 16, 0);
					G_ProjectSource (self->s.origin, v, v_forward, v_right, right_target);
				}
				VectorCopy (self->monsterinfo.last_sighting, self->monsterinfo.saved_goal);
				self->monsterinfo.aiflags |= AI_PURSUE_TEMP;
				VectorCopy (right_target, self->goalentity->s.origin);
				VectorCopy (right_target, self->monsterinfo.last_sighting);
				VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
				self->ideal_yaw = vectoyaw(v);
//				self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
//				debug_drawline(self.origin, self.last_sighting, 152);
			}
		}
//		else gi.dprintf("course was fine\n");
	}

	if (self->goalentity)	
	{
		M_MoveToGoal (self, dist);
	}

	G_FreeEdict(tempgoal);

	if (self)
		self->goalentity = save;
}

void ai_npc_run_nopath (edict_t *self, float dist)
{
	vec3_t		v;
	edict_t		*tempgoal;
	edict_t		*save;
	qboolean	newgoal;
	edict_t		*marker;
	float		d1, d2;
	trace_t		tr;
	vec3_t		v_forward, v_right;
	float		left, center, right;
	vec3_t		left_target, right_target;

/*	if(visible_vec(self->s.origin, self->action_target->s.origin))
	{
		gi.dprintf("its visible alright\n");
		self->goalentity = self->action_target;
		self->movetarget = self->action_target;
		VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
		self->ideal_yaw = vectoyaw (v);
		self->monsterinfo.aiflags |= AI_COMBAT_POINT;
		self->path_node = NO_NODES;
		self->action_target = G_PickTarget(self->action_target->target);
		return;
	} */

	newgoal = false;

	//RipVTide check out
//	gi.bprintf (PRINT_MEDIUM, "No path ai executed\n");

	//return;
	if (!(self->monsterinfo.aiflags & AI_LOST_SIGHT))
	{
		// just lost sight of the player, decide where to go first
//		dprint("lost sight of player, last seen at "); dprint(vtos(self.last_sighting)); dprint("\n");
		self->monsterinfo.aiflags |= (AI_LOST_SIGHT | AI_PURSUIT_LAST_SEEN);
		self->monsterinfo.aiflags &= ~(AI_PURSUE_NEXT | AI_PURSUE_TEMP);
		newgoal = true;
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
			VectorCopy (self->monsterinfo.saved_goal, self->monsterinfo.last_sighting);
			newgoal = true;
		}
		else if (self->monsterinfo.aiflags & AI_PURSUIT_LAST_SEEN)
		{
			self->monsterinfo.aiflags &= ~AI_PURSUIT_LAST_SEEN;
			marker = PlayerTrail_PickFirst (self);
		}
		else
		{
			marker = PlayerTrail_PickNext (self);
		}

		if (marker)
		{
			VectorCopy (marker->s.origin, self->monsterinfo.last_sighting);
			self->monsterinfo.trail_time = marker->timestamp;
			self->s.angles[YAW] = self->ideal_yaw = marker->s.angles[YAW];
//			dprint("heading is "); dprint(ftos(self.ideal_yaw)); dprint("\n");

//			debug_drawline(self.origin, self.last_sighting, 52);
			newgoal = true;
		}
	}

	VectorSubtract (self->s.origin, self->monsterinfo.last_sighting, v);
	d1 = VectorLength(v);
	if (d1 <= dist)
	{
		self->monsterinfo.aiflags |= AI_PURSUE_NEXT;
		dist = d1;
	}

	save = self->goalentity;
	tempgoal = G_Spawn();
	self->goalentity = tempgoal;

	VectorCopy (self->monsterinfo.last_sighting, self->goalentity->s.origin);


	if (newgoal)
	{
//		gi.dprintf("checking for course correction\n");

		tr = gi.trace(self->s.origin, self->mins, self->maxs, self->monsterinfo.last_sighting, self, MASK_PLAYERSOLID);
		if (tr.fraction < 1)
		{
			VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
			d1 = VectorLength(v);
			center = tr.fraction;
			d2 = d1 * ((center+1)/2);
			self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
			AngleVectors(self->s.angles, v_forward, v_right, NULL);

			VectorSet(v, d2, -16, 0);
			G_ProjectSource (self->s.origin, v, v_forward, v_right, left_target);
			tr = gi.trace(self->s.origin, self->mins, self->maxs, left_target, self, MASK_PLAYERSOLID);
			left = tr.fraction;

			VectorSet(v, d2, 16, 0);
			G_ProjectSource (self->s.origin, v, v_forward, v_right, right_target);
			tr = gi.trace(self->s.origin, self->mins, self->maxs, right_target, self, MASK_PLAYERSOLID);
			right = tr.fraction;

			center = (d1*center)/d2;
			if (left >= center && left > right)
			{
				if (left < 1)
				{
					VectorSet(v, d2 * left * 0.5, -16, 0);
					G_ProjectSource (self->s.origin, v, v_forward, v_right, left_target);
//					gi.dprintf("incomplete path, go part way and adjust again\n");
				}
				VectorCopy (self->monsterinfo.last_sighting, self->monsterinfo.saved_goal);
				self->monsterinfo.aiflags |= AI_PURSUE_TEMP;
				VectorCopy (left_target, self->goalentity->s.origin);
				VectorCopy (left_target, self->monsterinfo.last_sighting);
				VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
				self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
//				gi.dprintf("adjusted left\n");
//				debug_drawline(self.origin, self.last_sighting, 152);
			}
			else if (right >= center && right > left)
			{
				if (right < 1)
				{
					VectorSet(v, d2 * right * 0.5, 16, 0);
					G_ProjectSource (self->s.origin, v, v_forward, v_right, right_target);
//					gi.dprintf("incomplete path, go part way and adjust again\n");
				}
				VectorCopy (self->monsterinfo.last_sighting, self->monsterinfo.saved_goal);
				self->monsterinfo.aiflags |= AI_PURSUE_TEMP;
				VectorCopy (right_target, self->goalentity->s.origin);
				VectorCopy (right_target, self->monsterinfo.last_sighting);
				VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
				self->s.angles[YAW] = self->ideal_yaw = vectoyaw(v);
//				gi.dprintf("adjusted right\n");
//				debug_drawline(self.origin, self.last_sighting, 152);
			}
		}
//		else gi.dprintf("course was fine\n");
	}

	M_MoveToGoal (self, dist);

	G_FreeEdict(tempgoal);

	if (self)
		self->goalentity = save;
}
