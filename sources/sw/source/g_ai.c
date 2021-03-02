// g_ai.c

#include "g_local.h"

qboolean FindTarget (edict_t *self);
extern cvar_t	*maxclients;

qboolean ai_checkattack (edict_t *self, float dist);

qboolean	enemy_vis;
qboolean	enemy_infront;
int			enemy_range;
float		enemy_yaw;
qboolean	enemy_too_highorlow;

//============================================================================

/*
=============
visible_vec

returns 1 if the vec is visible to vec2
=============
*/
qboolean visible_vec (edict_t *self, vec3_t vec, vec3_t vec2)
{
	trace_t	trace;

	trace = gi.trace (vec, vec3_origin, vec3_origin, vec2, self, MASK_OPAQUE);
	
	if (trace.fraction == 1.0)
		return true;

	return false;
}

void spawn_temp (vec3_t spot1, int c)
{
   edict_t *gib;

   gib = G_Spawn();

   VectorCopy (spot1, gib->s.origin);
   gib->movetype = MOVETYPE_NONE;
   gib->solid = SOLID_NOT;
   if(c == 1)
      gib->s.effects |= EF_QUAD; //right
   else gib->s.effects |= EF_PENT;  //left
   gib->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
   gib->nextthink = level.time + 0.5;
   gib->think = G_FreeEdict;
   gi.linkentity (gib);
}

void spawn_templaser (vec3_t spot1, vec3_t spot2)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER); //TE_RAILTRAIL
	gi.WritePosition (spot1);
	gi.WritePosition (spot2);
	gi.multicast (spot1, MULTICAST_PHS);
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
void AI_SetSightClient (void)
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
		if (ent->inuse && ent->health > 0 && !(ent->flags & FL_NOTARGET) && (Force_constant_active (ent, LFORCE_INVISIBILITY) == 255) && ent->client->invisi_time < level.time)
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
void ai_move (edict_t *self, float dist)
{
	M_walkmove (self, self->s.angles[YAW], dist);
}


/*
=============
ai_hold

Hold position
=============
*/
void ai_hold (edict_t *self, float dist)
{
	vec3_t	v;

	if (self->enemy == NULL)
	{
//		gi.bprintf (PRINT_MEDIUM, "ai_hold error, enemy is NULL\n");
		return;
	}
	VectorSubtract (self->enemy->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);
	M_ChangeYaw (self);
}

/*
=============
ai_charge

Turns towards target and advances
Use this call with a distnace of 0 to replace ai_face
==============
*/
void ai_charge (edict_t *self, float dist)
{
	vec3_t	v;

	//gi.bprintf (PRINT_MEDIUM, "ai_charge\n");

	if (self->enemy == NULL)
	{
//		gi.bprintf (PRINT_MEDIUM, "ai_charge error, enemy is NULL\n");
		return;
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
=============
ai_turn

don't move, but turn towards ideal_yaw
Distance is for slight position adjustments needed by the animations
=============
*/
void ai_turn (edict_t *self, float dist)
{
	if (dist)
		M_walkmove (self, self->s.angles[YAW], dist);

	if (FindTarget (self))
		return;
	
	M_ChangeYaw (self);
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
int range (edict_t *self, edict_t *other)
{
	vec3_t	v;
	float	len;

	VectorSubtract (self->s.origin, other->s.origin, v);
	len = VectorLength (v);
	if (len < MELEE_DISTANCE)
		return RANGE_MELEE;
	if (len < 500)
		return RANGE_NEAR;
	if (len < 1000)
		return RANGE_MID;
	return RANGE_FAR;
}

/*
=============
visible

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
qboolean visible (edict_t *self, edict_t *other)
{
	vec3_t	spot1;
	vec3_t	spot2;
	trace_t	trace;

	VectorCopy (self->s.origin, spot1);
	spot1[2] += self->viewheight;
	VectorCopy (other->s.origin, spot2);
	spot2[2] += other->viewheight;
	trace = gi.trace (spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE);
	
	if (trace.fraction == 1.0)
		return true;

	return false;
}


/*
=============
infront

returns 1 if the entity is in front (in sight) of self
=============
*/
qboolean infront (edict_t *self, edict_t *other)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;
	
	AngleVectors (self->s.angles, forward, NULL, NULL);
	VectorSubtract (other->s.origin, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);
	
	if (dot > 0.3)
		return true;
	return false;
}


//============================================================================

void HuntTarget (edict_t *self)
{
	vec3_t	vec;

	self->goalentity = self->enemy;
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.stand (self);
	else
	{
		if (self->waterlevel > 2 && !(self->flags & FL_SWIM))
			self->monsterinfo.stand (self);
		else
			self->monsterinfo.run (self);
	}

	if (self->path_node == NO_NODES)
		VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
	else
		VectorSubtract (node_list[self->path_node], self->s.origin, vec);

	self->ideal_yaw = vectoyaw(vec);
	// wait a while before first attack
	if (!(self->monsterinfo.aiflags & AI_STAND_GROUND))
		AttackFinished (self, 1);
}

void FoundTarget (edict_t *self)
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
		HuntTarget (self);
		return;
	}

	self->goalentity = self->movetarget = G_PickTarget(self->combattarget);
	if (!self->movetarget)
	{
		self->goalentity = self->movetarget = self->enemy;
		HuntTarget (self);
//		gi.dprintf("%s at %s, combattarget %s not found\n", self->classname, vtos(self->s.origin), self->combattarget);
		return;
	}

	// clear out our combattarget, these are a one shot deal
	self->combattarget = NULL;
	self->monsterinfo.aiflags |= AI_COMBAT_POINT;

	// clear the targetname, that point is ours!
	self->movetarget->targetname = NULL;
	self->monsterinfo.pausetime = 0;

	// run for it
	// make sure combat target for walking monsters is not in water
	self->monsterinfo.run (self);
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

qboolean M_CheckAttack (edict_t *self)
{
	vec3_t	spot1, spot2;
	float	chance;
	trace_t	tr;

	//gi.bprintf (PRINT_MEDIUM, "M_CheckAttack\n");

	if (self->enemy->health > 0)
	{
	// see if any entities are in the way of the shot
		VectorCopy (self->s.origin, spot1);
		spot1[2] += self->viewheight;
		VectorCopy (self->enemy->s.origin, spot2);
		spot2[2] += self->enemy->viewheight;

		tr = gi.trace (spot1, NULL, NULL, spot2, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_SLIME|CONTENTS_LAVA|CONTENTS_WINDOW);

		// do we have a clear shot?
		if (tr.ent != self->enemy)
			return false;
	}
	
	// melee attack
	if (enemy_range == RANGE_MELEE)
	{
		// don't always melee in easy mode
		if (skill->value == 0 && (rand()&3) )
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
		
//	if (enemy_range == RANGE_FAR)
//		return false;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		chance = 0.8;
	}
	else if (enemy_range == RANGE_MELEE)
	{
		chance = 0.8;
	}
	else if (enemy_range == RANGE_NEAR)
	{
		chance = 0.6;
	}
	else if (enemy_range == RANGE_MID)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_FAR)
	{
		chance = 0.2;
	}
	else
	{
		return false;
	}

	if (skill->value == 0)
		chance *= 0.5;
	else if (skill->value >= 2)
		chance *= 2;

	if (enemy_too_highorlow)
	{
		if (self->monsterinfo.attack)
			chance /= 2;
		else
			chance = 0;
	}

	if (random () < chance)
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + 2*random();

		// combat no move means monster won't use moving attacks
		if (enemy_too_highorlow)
			self->monsterinfo.aiflags |= AI_COMBAT_NOMOVE;
		else
			self->monsterinfo.aiflags &= ~AI_COMBAT_NOMOVE;

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
	M_ChangeYaw (self);

	if (FacingIdeal(self))
	{
		self->monsterinfo.melee (self);
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
	//gi.bprintf (PRINT_MEDIUM, "ai_run_missile\n");
	self->ideal_yaw = enemy_yaw;
	M_ChangeYaw (self);


	//gi.bprintf (PRINT_MEDIUM, "ai_run_missile check facing ideal\n");
	if (FacingIdeal(self))
	{
		//gi.bprintf (PRINT_MEDIUM, "ai_run_missile heading towards infantry attack\n");
		self->monsterinfo.attack (self);
		self->monsterinfo.attack_state = AS_STRAIGHT;
	}
}


/*
=============
ai_checkattack

Decides if we're going to attack or do something else
used by ai_run and ai_stand
=============
*/
qboolean ai_checkattack (edict_t *self, float dist)
{
	vec3_t		temp;
	qboolean	hesDeadJim;
	float		height = 0;


	enemy_too_highorlow = false;

	//gi.bprintf (PRINT_MEDIUM, "ai_checkattack\n");

// this causes monsters to run blindly to the combat point w/o firing
	if (self->goalentity)
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
				self->show_hostile = level.time + 1;
				return false;
			}
		}
	}

	enemy_vis = false;
	//gi.bprintf (PRINT_MEDIUM, "ai_checkattack dead check\n");

// see if the enemy is dead
	hesDeadJim = false;
	if ((!self->enemy) || (!self->enemy->inuse))
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
		if (self->monsterinfo.aiflags & AI_BRUTAL)
		{
			if (self->enemy->health <= -80)
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
		self->enemy = NULL;
	// FIXME: look all around for other targets
		if (self->oldenemy && self->oldenemy->health > 0)
		{
			self->enemy = self->oldenemy;
			self->oldenemy = NULL;
			HuntTarget (self);
		}
		else
		{
			if (self->movetarget)
			{
				self->goalentity = self->movetarget;
				self->monsterinfo.walk (self);
			}
			else
			{
				// we need the pausetime otherwise the stand code
				// will just revert to walking with no target and
				// the monsters will wonder around aimlessly trying
				// to hunt the world entity
				self->monsterinfo.pausetime = level.time + 100000000;
				self->monsterinfo.stand (self);
			}
			return true;
		}
	}

	//gi.bprintf (PRINT_MEDIUM, "ai_checkattack he's not dead jim\n");

	self->show_hostile = level.time + 1;		// wake up other monsters

	//gi.bprintf (PRINT_MEDIUM, "ai_checkattack visible check\n");
// check knowledge of enemy
	enemy_vis = visible(self, self->enemy);
	if (enemy_vis)
	{
		self->monsterinfo.search_time = level.time + 5;
		VectorCopy (self->enemy->s.origin, self->monsterinfo.last_sighting);
		//gi.bprintf (PRINT_MEDIUM, "enemy is visible %d\n", level.framenum);
		height = abs(self->enemy->s.origin[2] - self->s.origin[2]);
	}

// look for other coop players here
//	if (coop && self->monsterinfo.search_time < level.time)
//	{
//		if (FindTarget (self))
//			return true;
//	}

	//gi.bprintf (PRINT_MEDIUM, "ai_checkattack infront check\n");

	enemy_infront = infront(self, self->enemy);
	//gi.bprintf (PRINT_MEDIUM, "ai_checkattack range check\n");

	enemy_range = range(self, self->enemy);
	//if (!(self->goalentity))
//		VectorSubtract (self->enemy->s.origin, self->s.origin, temp);
	//else 
	//	VectorSubtract (self->goalentity->s.origin, self->s.origin, temp);

	//gi.bprintf (PRINT_MEDIUM, "ai_checkattack enemy_yaw set\n");

//	enemy_yaw = vectoyaw(temp);

	// JDC self->ideal_yaw = enemy_yaw;

// now check for height differences
	if (height > 120)
	{
		//return false;
		counter_t	old_path_node;
		//gi.bprintf (PRINT_MEDIUM, "ai_checkattack target too high %d\n", level.framenum);

		//return false;

		old_path_node = self->path_node;
		find_path (self);

		if (self->path_node != NO_NODES)
		{
			//gi.bprintf (PRINT_MEDIUM, "target too high %d\n", level.framenum);
			enemy_too_highorlow = true;
			VectorSubtract (node_list[self->path_node], self->s.origin, temp);
			enemy_yaw = vectoyaw(temp);
			self->path_node = old_path_node;
			return self->monsterinfo.checkattack (self);
		}

		self->path_node = old_path_node;
	}
	else
	{
		if (enemy_vis)
			VectorSubtract (self->enemy->s.origin, self->s.origin, temp);
		else
			VectorSubtract (self->monsterinfo.last_sighting, self->s.origin, temp);
	}

	enemy_yaw = vectoyaw(temp);

	if (self->monsterinfo.attack_state == AS_MISSILE)
	{
		//gi.bprintf (PRINT_MEDIUM, "ai_checkattack AS_MISSILE\n");

		ai_run_missile (self);
		return true;
	}
	if (self->monsterinfo.attack_state == AS_MELEE)
	{
		//gi.bprintf (PRINT_MEDIUM, "ai_checkattack AS_MELEE\n");

		ai_run_melee (self);
		return true;
	}

	// if enemy is not currently visible, we will never attack
	if (!enemy_vis)
		return false;

	//gi.bprintf (PRINT_MEDIUM, "ai_checkattack heading to M_CheckAttack\n");

	return self->monsterinfo.checkattack (self);
}


//
// NEW STUFF FOR AI
//


/*
=================
AI_AlertStroggs

This routine will alert other NEARBY Stroggs if a Strogg first sees an enemy

=================
*/

void AI_AlertStroggs (edict_t *self)
{

	float	dist;
	edict_t *strogg=NULL;

	if (self->enemy->flags & FL_NOTARGET)
		return;
	if(self->enemy->client && (Force_constant_active (self->enemy, LFORCE_INVISIBILITY) != 255) && self->enemy->client->invisi_time < level.time)
		return;

	if (skill->value < 1)
		return;

	if (skill->value == 1)
		dist = 650;
	else if (skill->value == 2)
		dist = 1000;
	else
		dist = 1250 + (skill->value*75);

	if (dist > 2400)
		dist = 2400;

	while ((strogg = findradius(strogg, self->s.origin, dist)) != NULL)
	{
		if (!(strogg->svflags & SVF_MONSTER))
			continue;

		if (strogg->health <= 0 || !(strogg->takedamage))
			continue;
	
		if (strogg == self)
			continue;
	
		if (strogg->client)
			continue;
	
		if (strogg->enemy)
			continue;

		if (strogg->attackflags & ATTACK_NO_ALERT)
			continue;

		if(!gi.inPVS(strogg->s.origin, self->s.origin))
			continue;

		strogg->enemy = self->enemy;
		FoundTarget (strogg);
	}

}

qboolean FindTarget (edict_t *self)
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

	if(self->monsterinfo.aiflags & AI_NPC_VIOLENT && client->holstered)
		return false;

//	if (client == self->enemy)
//		return true;	// JDC false;

	if (client == self->enemy)
		return false;


	if (client->client)
	{
		if (client->flags & FL_NOTARGET || (Force_constant_active (client, LFORCE_INVISIBILITY) != 255) && client->client->invisi_time < level.time)
			return false;
	}
	else if (client->svflags & SVF_MONSTER)
	{
		if (!client->enemy)
			return false;
		if (client->enemy->flags & FL_NOTARGET || (Force_constant_active (client->enemy, LFORCE_INVISIBILITY) != 255) && client->enemy->client->invisi_time < level.time)
			return false;
	}
	else if (heardit)
	{
		if (client->owner->flags & FL_NOTARGET || (Force_constant_active (client->owner, LFORCE_INVISIBILITY) != 255) && client->owner->client->invisi_time < level.time)
			return false;
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

/*
=============
ai_run_slide

Strafe sideways, but stay at aproximately the same range
=============
*/
void ai_run_slide(edict_t *self, float distance)
{
	float	ofs;
	
	vec3_t	v;

	//gi.bprintf (PRINT_MEDIUM, "run sliding\n");
	VectorSubtract (self->enemy->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);
	M_ChangeYaw (self);

	//self->ideal_yaw = enemy_yaw;
	//M_ChangeYaw (self);

	if (self->monsterinfo.lefty)
		ofs = 90;
	else
		ofs = -90;
	
	if (M_walkmove (self, self->ideal_yaw + ofs, distance))
		return;
		
	self->monsterinfo.lefty = 1 - self->monsterinfo.lefty;
	M_walkmove (self, self->ideal_yaw - ofs, distance);
}

void ai_run_strafe(edict_t *self, float distance)
{
	float	ofs;
	
	vec3_t	v;

	VectorSubtract (self->enemy->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);
	M_ChangeYaw (self);

	//self->ideal_yaw = enemy_yaw;
	//M_ChangeYaw (self);

	if (!(self->monsterinfo.aiflags & AI_SLIDE))
	{
		self->monsterinfo.pausetime = level.time + 0.1 + (skill->value) * FRAMETIME + (2 * rand() % 8) * FRAMETIME;
		self->monsterinfo.lefty = 1 - self->monsterinfo.lefty;
	}

	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_SLIDE;
	else
		self->monsterinfo.aiflags |= AI_SLIDE;

	if (self->monsterinfo.lefty)
		ofs = 30;
	else
		ofs = -30;

	if (skill->value < 1.1)
		ofs = 0;


	if ((M_walkmove (self, self->ideal_yaw + ofs, distance)))
	{
		return;
	}

		
	self->monsterinfo.lefty = 1 - self->monsterinfo.lefty;
	M_walkmove (self, self->ideal_yaw - ofs, distance);
}


void ai_stand (edict_t *self, float dist)
{
	vec3_t	v;

	make_node (self);

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
			FindTarget (self);
		return;
	}

	if (FindTarget (self))
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

void ai_walk (edict_t *self, float dist)
{
	M_MoveToGoal (self, dist);

	// check for noticing a player
	if (FindTarget (self))
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

void ai_run_nopath (edict_t *self, float dist);

void ai_run (edict_t *self, float dist)
{

	vec3_t		v;
	edict_t		*marker, *tempgoal, *save;
	qboolean	newgoal;
	float		d1, d2;
	trace_t		tr;
	vec3_t		v_forward, v_right;
	float		left, center, right;
	vec3_t		left_target, right_target;
	qboolean	path_updated;

	//gi.bprintf (PRINT_MEDIUM, "ai_run path start\n");
	// if we're going to a combat point, just proceed
	if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
	{
		M_MoveToGoal (self, dist);
		return;
	} 

	//gi.bprintf (PRINT_MEDIUM, "ai_run check sound target\n");

	if (self->monsterinfo.aiflags & AI_SOUND_TARGET)
	{
		//gi.bprintf (PRINT_MEDIUM, "ai_run AI_SOUND_TARGET active\n");

		VectorSubtract (self->s.origin, self->enemy->s.origin, v);
		if (VectorLengthSquared(v) < 4096)
		{
			self->monsterinfo.aiflags |= (AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
			self->monsterinfo.stand (self);
			//gi.bprintf (PRINT_MEDIUM, "ai_run AI_SOUND_TARGET reached\n");
			return;
		}

		//gi.bprintf (PRINT_MEDIUM, "ai_run AI_SOUND_TARGET moving to target\n");
		//if (self->goalentity)
			M_MoveToGoal (self, dist);

		if (!FindTarget (self))
			return;
	}

	//gi.bprintf (PRINT_MEDIUM, "ai_run check attack\n");

	if (ai_checkattack (self, dist))
		return;
	
	//gi.bprintf (PRINT_MEDIUM, "ai_run check attack state\n");

	if (self->monsterinfo.attack_state == AS_SLIDING)
	{
		ai_run_slide (self, dist);
		return;
	}

	//gi.bprintf (PRINT_MEDIUM, "ai_run check visible\n");


	if (!enemy_too_highorlow)
	{
		if (enemy_vis)
		{
//		if (self.aiflags & AI_LOST_SIGHT)
//			dprint("regained sight\n");
		//if (self->monsterinfo.aiflags & AI_LOST_SIGHT)
		//	gi.bprintf(PRINT_MEDIUM, "regained sight\n");
			if (!self->monsterinfo.attack)
			{
		// a melee monster
				if (self->enemy)
					if (range (self, self->enemy) <= RANGE_NEAR)
				{
					//gi.bprintf (PRINT_MEDIUM, "Charge!!!!!! %d\n", level.framenum);
					ai_charge (self, dist);
					return;
				}
			}


			if (self->goalentity)
				M_MoveToGoal (self, dist);
			self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
			VectorCopy (self->enemy->s.origin, self->monsterinfo.last_sighting);
			self->monsterinfo.trail_time = level.time;
			self->path_node = NO_NODES;
			return;

		}
	}
	
	//gi.bprintf (PRINT_MEDIUM, "ai_run finished id checks\n");

	// coop will change to another enemy if visible
	if (coop->value)
	{	// FIXME: insane guys get mad with this, which causes crashes!
		if (FindTarget (self))
			return;
	}

	if (self->monsterinfo.search_time) 
	{
		// Strogg will take 10 seconds to head towards next node
		// if time limit is up, it will recalculate path again
		// the very next frame
		if (level.time > (self->monsterinfo.search_time + 10))
		{	
			if (self->goalentity)
				M_MoveToGoal (self, dist);
			self->monsterinfo.search_time = 0;
//		dprint("search timeout\n");
			//gi.bprintf (PRINT_MEDIUM, "ai_run search timeout\n");
			self->path_node = NO_NODES;
			
			if (!Q_stricmp (self->goalentity->classname, "path_node"))
			{
				//gi.bprintf (PRINT_MEDIUM, "ai_run removing path node\n");
				self->goalentity->think = G_FreeEdict;
				self->goalentity->nextthink = level.time + 0.1;
			}			

			return;
		}
	}



	if (self->goalentity)
	{
		if (self->path_node != NO_NODES)
		{
			VectorSubtract (self->s.origin, self->goalentity->s.origin, v);

			if (VectorLengthSquared(v) > 1024)
			{
				// has a path_node goal to get to but hasn't reached there yet
				// so keep going
				M_MoveToGoal (self, dist);
				//gi.bprintf (PRINT_MEDIUM, "goal moving to node #%d\n", self->path_node);
				return;
			}
			{
			//else
				//gi.bprintf (PRINT_MEDIUM, "reached node #%d\n", self->path_node);

				// reached node, now delete goalentity.  Only delete if goalentity
				// was a path node.
				if (!Q_stricmp (self->goalentity->classname, "path_node"))
				{
					self->goalentity->think = G_FreeEdict;
					self->goalentity->nextthink = level.time + 0.1;
				}
			}
			
		}
	}
	
	if (self->check_path_time > level.time)
	{
		//gi.bprintf (PRINT_MEDIUM, "ai_run, pathtime set, heading to ai_run_nopath\n");
			ai_run_nopath (self, dist);
			return;
	}

	if (self->path_node != NO_NODES)
		if (self->goalentity == NULL)
	{
		//gi.bprintf (PRINT_MEDIUM, "No goal entity set error\n");
		self->path_node = NO_NODES;
	}

	// check if previous goal was temporary goal, path_node is set and goal was a 
	// path_node
	if (self->monsterinfo.aiflags & AI_PURSUE_TEMP && self->path_node != NO_NODES
		&& !(Q_stricmp (self->goalentity->classname, "path_node")) )
	{

//		gi.dprintf("was temp goal; retrying original\n");
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

		//gi.bprintf (PRINT_MEDIUM, "Marker set from old temp, moving to node #%d\n", self->path_node);

		self->goalentity = marker;
		self->s.angles[YAW] = self->ideal_yaw = marker->s.angles[YAW];
		M_ChangeYaw (self);
		
		self->monsterinfo.search_time = level.time + FRAMETIME;
		newgoal = true;
	
	}
	else if (self->path_node == NO_NODES)
	{
//		gi.bprintf (PRINT_MEDIUM, "find path\n");
		find_path (self);
		path_updated = true;
		self->monsterinfo.aiflags &= ~AI_PURSUE_TEMP;

	}
	else
	{
//		gi.bprintf (PRINT_MEDIUM, "find next node\n");
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
			self->s.angles[YAW] = self->ideal_yaw = marker->s.angles[YAW];
			M_ChangeYaw (self);

			self->monsterinfo.search_time = level.time + FRAMETIME;
			
			newgoal = true;
	
		}
		else
		{
			// for the next 5 to 10 seconds, game will run using id's 
			// "breadcrumb" chase code.
			self->check_path_time = level.time + 5 + ((rand() % 5));

			self->monsterinfo.aiflags |= AI_PURSUE_NEXT;
			self->monsterinfo.aiflags |= AI_LOST_SIGHT;
			self->monsterinfo.aiflags |= AI_PURSUIT_LAST_SEEN;

		//	gi.bprintf (PRINT_MEDIUM, "ai_run NO PATH FOUND, Heading towards ai_run_nopath\n");
			ai_run_nopath (self, dist);
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

	if (self->goalentity)	
		M_MoveToGoal (self, dist);

	G_FreeEdict(tempgoal);

	if (self)
		self->goalentity = save;

}


void ai_run_nopath (edict_t *self, float dist)
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



	newgoal = false;

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
