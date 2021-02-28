#include "g_local.h"

#define SKULL_FRAMES_ATTACK_START	10
#define SKULL_FRAMES_ATTACK_END		14
#define SKULL_FRAMES_IDLE_START		0
#define SKULL_FRAMES_IDLE_END		5
#define SKULL_ATTACK				1
#define SKULL_IDLE_UP				3
#define SKULL_IDLE_DOWN				4
#define SKULL_FLOAT_HEIGHT			3		// max float travel
#define SKULL_FLOAT_SPEED			1		// vertical float speed

qboolean skull_findtarget (edict_t *self)
{
	edict_t *e=NULL;

	while ((e = findclosestradius(e, self->s.origin, 
		SKULL_TARGET_RANGE)) != NULL)
	{
		if (e == self)
			continue;
		if (!G_ValidTarget(self, e, true))
			continue;
		// ignore enemies that are too far away from activator to prevent wandering
		// FIXME: we may want to add a hurt function so that we can retaliate if we are attacked out of range
		if (entdist(e, self->activator) > SKULL_MAX_RANGE)
			continue;
		self->enemy = e;
		self->monsterinfo.search_frames = 0;
		return true;
	}

	return false;
}

qboolean skull_checkposition (edict_t *self)
{
	if (gi.pointcontents(self->s.origin) & MASK_SOLID)
	{
		if (self->activator)
		{
			self->activator->skull = NULL;
			self->activator->client->pers.inventory[power_cube_index] += SKULL_COST;
			gi.centerprintf(self->activator, "Skull was removed.\n");
		}
		G_FreeEdict(self);
		gi.dprintf("WARNING: Skull removed from solid.\n");
		return false;
	}

	self->velocity[0] *= 0.6;
	self->velocity[1] *= 0.6;
	self->velocity[2] *= 0.6;
	return true;
}

void skull_bumparound (edict_t *self)
{
	float		yaw, min_yaw, max_yaw;
	vec3_t		forward, start;
	trace_t		tr;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, 0.5*SKULL_MOVE_HORIZONTAL_SPEED, forward, start);
	tr = gi.trace(self->s.origin, self->mins, self->maxs, start, self, MASK_SHOT);
	if (tr.fraction < 1)
	{
		yaw = vectoyaw(tr.plane.normal);
		min_yaw = yaw-90;
		max_yaw = yaw+90;
		self->ideal_yaw = GetRandom(min_yaw, max_yaw);
		return;
	}
	VectorCopy(start, self->s.origin);
}

void skull_move_forward (edict_t *self, float dist)
{
	float	yaw, min_yaw, max_yaw;
	vec3_t	forward, start;
	trace_t	tr;

	if (self->wait+1 > level.time)
		dist *= 0.5; // slow down while we turn

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, dist, forward, start);
	tr = gi.trace(self->s.origin, self->mins, self->maxs, start, self, MASK_SHOT);
	// bump away from obstructions
	if ((level.time > self->wait) && (tr.fraction < 1))
	{
		//gi.dprintf("fwd bump\n");
		yaw = vectoyaw(tr.plane.normal);
		min_yaw = yaw - 90;
		max_yaw = yaw + 90;
		self->ideal_yaw = GetRandom(min_yaw, max_yaw);
		self->wait = level.time + 0.5; // time before we can make another course correction
	}

	// double-check final move
	if (!(gi.pointcontents(tr.endpos) & MASK_SOLID))
		VectorCopy(tr.endpos, self->s.origin);
}

void skull_strafe (edict_t *self, float dist)
{
	vec3_t	right, start;
	trace_t	tr;

	// don't strafe if we don't have an enemy or we are chasing activator
	if (!self->enemy)
		return;

	// don't begin strafing until we are close
	if (entdist(self, self->enemy)-32 > 512)
		return;

	// change direction
	if (entdist(self, self->enemy)-32 > SKULL_MAX_DIST && 
		level.time > self->monsterinfo.dodge_time)
	{
		if (self->monsterinfo.lefty)
			self->monsterinfo.lefty = 0;
		else
			self->monsterinfo.lefty = 1;
		self->monsterinfo.dodge_time = level.time + 0.5;
	}

//	gi.dprintf("skull_strafe()\n");

	AngleVectors(self->s.angles, NULL, right, NULL);

	if (self->monsterinfo.lefty)
	{
		VectorMA(self->s.origin, dist, right, start);
		tr = gi.trace(self->s.origin, self->mins, self->maxs, start, self, MASK_SHOT);
		if (tr.fraction == 1)
		{
			VectorCopy(tr.endpos, self->s.origin);
			return;
		}
		self->monsterinfo.lefty = 0;
	}

	VectorMA(self->s.origin, -dist, right, start);
	tr = gi.trace(self->s.origin, self->mins, self->maxs, start, self, MASK_SHOT);
	if (tr.fraction == 1)
	{
		VectorCopy(tr.endpos, self->s.origin);
		return;
	}
	self->monsterinfo.lefty = 1;
}

void skull_move_vertical (edict_t *self, float dist)
{
	vec3_t	start;
	trace_t	tr;

	VectorCopy(self->s.origin, start);
	start[2] += dist;
	tr = gi.trace(self->s.origin, self->mins, self->maxs, start, self, MASK_SHOT);

	// double-check final move
	if (!(gi.pointcontents(tr.endpos) & MASK_SOLID))
		VectorCopy(tr.endpos, self->s.origin);
}

void skull_movetogoal (edict_t *self, edict_t *goal)
{
	float	temp, dist, speed, goalpos;
	vec3_t	v;
	que_t	*slot=NULL;

	self->style = SKULL_ATTACK;
	// lock-on to enemy if he is visible, and we're not busy
	// trying to avoid an obstruction
	if (visible(self, goal) && (level.time > self->wait))
	{
		trace_t tr;

		VectorSubtract(goal->s.origin, self->s.origin, v);
		self->ideal_yaw = vectoyaw(v);

		// vertical movement
		goalpos = goal->absmax[2] + SKULL_HEIGHT; // float above enemy

		// check for obstruction
		VectorCopy(goal->s.origin, v);
		v[2] = goalpos;
		tr = gi.trace(goal->s.origin, NULL, NULL, v, goal, MASK_SOLID);
		goalpos = tr.endpos[2];

		if (goalpos > self->s.origin[2])
			skull_move_vertical(self, SKULL_MOVE_VERTICAL_SPEED);
		if (goalpos < self->s.origin[2])
			skull_move_vertical(self, -SKULL_MOVE_VERTICAL_SPEED);

		skull_strafe(self, 0.5*SKULL_MOVE_HORIZONTAL_SPEED);
	}

	// horizontal movement
	dist = entdist(self, goal);

	if (dist > SKULL_MAX_DIST)
	{
		if (dist-SKULL_MOVE_HORIZONTAL_SPEED < SKULL_MAX_DIST)
			speed = dist-SKULL_MAX_DIST;
		else
			speed = SKULL_MOVE_HORIZONTAL_SPEED;
	}
	else
	{
		if (dist+SKULL_MOVE_HORIZONTAL_SPEED > SKULL_MAX_DIST)
			speed = -(SKULL_MAX_DIST-dist);
		else
			speed = -SKULL_MOVE_HORIZONTAL_SPEED;
	}

	// are we slowed by holy freeze?
	slot = que_findtype(self->curses, slot, AURA_HOLYFREEZE);
	if (slot)
	{
		temp = 1 / (1 + 0.1 * slot->ent->owner->myskills.abilities[HOLY_FREEZE].current_level);
		if (temp < 0.25) temp = 0.25;
		speed *= temp;
	}
	
	//Talent: Frost Nova
	if(self->chill_time > level.time)
	{
		speed *= 1 / (1 + CHILL_DEFAULT_BASE + CHILL_DEFAULT_ADDON * self->chill_level);
	}

	// 3.5 weaken slows down target
	if ((slot = que_findtype(self->curses, NULL, WEAKEN)) != NULL)
	{
		temp = 1 / (1 + WEAKEN_SLOW_BASE + WEAKEN_SLOW_BONUS 
			* slot->ent->owner->myskills.abilities[WEAKEN].current_level);
		speed *= temp;
	}

	skull_move_forward(self, speed);
	M_ChangeYaw(self);
	gi.linkentity(self);
}

void skull_attack (edict_t *self)
{
	int		damage;//, knockback;
	float	dist, chance;
	vec3_t	forward, start, end, v;
	trace_t	tr;

	skull_movetogoal(self, self->enemy);
	
	dist = entdist(self, self->enemy);

	if (!visible(self, self->enemy))
		return;
	if (!infront(self, self->enemy))
		return;
	if (dist > SKULL_ATTACK_RANGE)
		return;
	if (que_typeexists(self->curses, AURA_HOLYFREEZE) && !(level.framenum % 2))
		return;

	// chill effect reduces attack rate/refire
	if (self->chill_time > level.time)
	{
		chance = 1 / (1 + CHILL_DEFAULT_BASE + CHILL_DEFAULT_ADDON * self->chill_level);
		if (random() > chance)
			return;
	}

	self->lastsound = level.framenum;

	// get muzzle origin
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorCopy(self->s.origin, start);
	start[2] = self->absmin[2]+8;
	VectorMA(start, self->maxs[1]+1, forward, start);

	G_EntMidPoint(self->enemy, end);

	// get attack vector
	VectorSubtract(end , start, v);
	VectorNormalize(v);
	//forward[2] = v[2];
	VectorMA(start, 8192, v, end);

	// do the damage
	damage = self->dmg;
	//knockback = 2*damage;
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
	if (G_EntExists(tr.ent))
	{
		//if (tr.ent->groundentity)
		//	knockback *= 2;
		T_Damage(tr.ent, self, self, forward, tr.endpos, tr.plane.normal, 
			damage, 0, DAMAGE_ENERGY, MOD_SKULL);
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_MONSTER_HEATBEAM);
	//gi.WriteByte (TE_LIGHTNING);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (start, MULTICAST_PVS);

	if (!(level.framenum%10))
		gi.sound(self, CHAN_ITEM, gi.soundindex("spells/chargedbolt1.wav"), 1, ATTN_NORM, 0);
}

void skull_runframes (edict_t *self)
{
	if (self->style == SKULL_ATTACK)
		G_RunFrames(self, SKULL_FRAMES_ATTACK_START, SKULL_FRAMES_ATTACK_END, false);
	else
		G_RunFrames(self, SKULL_FRAMES_IDLE_START, SKULL_FRAMES_IDLE_END, false);
}

void skull_regenerate (edict_t *self)
{
	if (self->health < self->max_health)
		self->health += self->max_health/SKULL_REGEN_FRAMES;
	if (self->health > self->max_health)
		self->health = self->max_health;
}

#define SKULL_RECALL_HEIGHT		64
#define SKULL_RECALL_RANGE		512

void skull_recall (edict_t *self)
{
	vec3_t	start;

//	if (!visible(self, self->activator) 
//		&& (entdist(self->activator, self) > SKULL_RECALL_RANGE))
	if (level.time > self->monsterinfo.teleport_delay)
	{
		VectorCopy(self->activator->s.origin, start);
		start[2] = self->activator->absmax[2]+SKULL_RECALL_HEIGHT;

		if (!G_IsClearPath(self->activator, MASK_SHOT, self->activator->s.origin, start))
			return;
		if (!G_IsValidLocation(self, start, self->mins, self->maxs))
			return;
		if (gi.pointcontents(start) & MASK_SOLID)
			return;

		VectorCopy(start, self->s.origin);
		VectorCopy(start, self->move_origin); // must set this or we may float up or down forever!
		self->s.event = EV_PLAYER_TELEPORT;

		self->monsterinfo.teleport_delay = level.time + 1.0;
	}
}

void skull_idle (edict_t *self)
{
//	vec3_t start;

	if (self->lockon)
		skull_recall(self);

	if (self->enemy)
	//{
		self->enemy = NULL;
	//	VectorCopy(self->s.origin, self->move_origin); // this is the point we float up and down from
//	}
	/*
	VectorCopy(self->s.origin, start);
	if (self->style == SKULL_IDLE_UP)
	{
		start[2] += SKULL_FLOAT_SPEED;
		if (start[2] >= self->move_origin[2]+SKULL_FLOAT_HEIGHT)
			self->style = SKULL_IDLE_DOWN;
	}
	else
	{
		start[2] -= SKULL_FLOAT_SPEED;
		if (start[2] <= self->move_origin[2]-SKULL_FLOAT_HEIGHT)
			self->style = SKULL_IDLE_UP;
	}

	if (!(gi.pointcontents(start) & MASK_SOLID))
	{
		VectorCopy(start, self->s.origin);
		gi.linkentity(self);
	}
*/
//	skull_regenerate(self);

	self->style = 0; // reset to idle state

	self->monsterinfo.idle_frames++;
}

void skull_remove (edict_t *self)
{
	if (self->deadflag == DEAD_DEAD)
		return;

	// reset owner's pointer to this entity
	if (self->activator && self->activator->inuse)
		self->activator->skull = NULL;

	// prep for removal
	self->think = BecomeExplosion1;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->deadflag = DEAD_DEAD;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = level.time + FRAMETIME;
	gi.unlinkentity(self);
}

void skull_return (edict_t *self)
{
	self->enemy = NULL;

	// is our activator visible?
	if (visible(self, self->activator))
	{
		// follow within 128 units
		if (entdist(self, self->activator) > 128)
		{
			skull_movetogoal(self, self->activator);
			return;
		}
	}
	// can't see activator, so teleport instead
	else
		skull_recall(self);

	// reset to idle state
	self->style = 0;
	self->monsterinfo.idle_frames++;
}

void skull_think (edict_t *self)
{
	if (!skull_checkposition(self))
		return; // skull was removed
	
	CTF_SummonableCheck(self);

	// hellspawn will auto-remove if it is asked to
	if (self->removetime > 0)
	{
		qboolean converted=false;

		if (self->flags & FL_CONVERTED)
			converted = true;

		if (level.time > self->removetime)
		{
			if (!RestorePreviousOwner(self))
			{
				skull_remove(self);
				return;
			}
		}
		// warn the converted monster's current owner
		else if (converted && self->activator && self->activator->inuse && self->activator->client 
			&& (level.time > self->removetime-5) && !(level.framenum%10))
				safe_cprintf(self->activator, PRINT_HIGH, "%s conversion will expire in %.0f seconds\n", 
					V_GetMonsterName(self), self->removetime-level.time);
	}

	if (!M_Upkeep(self, 10, 2))
		return;

	// hellspawn can't lock-on to enemy easily while cursed
	if ((level.time > self->wait) && que_typeexists(self->curses, CURSE) 
		&& (random() <= 0.2))
		self->wait = level.time + 2*random();

	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround(self);
	}

	if (!self->enemy)
		skull_findtarget(self);
	if (G_ValidTarget(self, self->enemy, false) // is the target still valid?
		&& entdist(self->activator, self->enemy) <= SKULL_MAX_RANGE) // make sure the target isn't too far from activator (don't wander)
	{
		if (!visible(self, self->enemy))
		{
			// try to find an easier target
			if (!skull_findtarget(self))
				self->monsterinfo.search_frames++;

			// give up searching for enemy after awhile
			if (self->monsterinfo.search_frames > SKULL_SEARCH_TIMEOUT)
			{
				self->enemy = NULL;
				skull_return(self);//skull_idle(self);
				self->nextthink = level.time + FRAMETIME;
				return;
			}
		}
		else
			self->monsterinfo.search_frames = 0;

		skull_attack(self);
	}
	else
		skull_return(self);//skull_idle(self);
		
	skull_runframes(self);
	M_SetEffects(self);

	self->nextthink = level.time + FRAMETIME;
}

void skull_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.centerprintf(self->activator, "Hell spawn died.\n");
	skull_remove(self);
}

void skull_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t forward, right, start, offset;

	V_Touch(self, other, plane, surf);

	if (other && other->inuse && other->client && self->activator 
		&& (other == self->activator || IsAlly(self->activator, other)))
	{
		AngleVectors (other->client->v_angle, forward, right, NULL);
		VectorScale (forward, -3, other->client->kick_origin);
		VectorSet(offset, 0, 7,  other->viewheight-8);
		P_ProjectSource (other->client, other->s.origin, offset, forward, right, start);

		self->velocity[0] += forward[0] * 100;
		self->velocity[1] += forward[1] * 100;
		self->velocity[2] += forward[2] * 100;
	}
}

void SpawnSkull (edict_t *ent)
{
	int		cost = SKULL_COST;
	vec3_t	forward, right, start, end, offset;
	edict_t *skull;
	trace_t	tr;

	// cost is doubled if you are a flyer below skill level 5
	if (ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5)
		cost *= 2;

	skull = G_Spawn();
	skull->svflags |= SVF_MONSTER;
	skull->classname = "hellspawn";
	skull->yaw_speed = 30;
	skull->s.effects |= EF_PLASMA;
	skull->activator = ent;
	skull->takedamage = DAMAGE_YES;
	skull->monsterinfo.level = ent->myskills.abilities[HELLSPAWN].current_level; // used for monster exp
	skull->monsterinfo.control_cost = 3; // used for monster exp
	skull->health = SKULL_INITIAL_HEALTH + SKULL_ADDON_HEALTH*ent->myskills.abilities[HELLSPAWN].current_level;//ent->myskills.level;
	skull->dmg = SKULL_INITIAL_DAMAGE + SKULL_ADDON_DAMAGE*ent->myskills.abilities[HELLSPAWN].current_level;//*ent->myskills.level;

	skull->max_health = skull->health;
	skull->gib_health = -150;
	skull->mass = 200;
	skull->monsterinfo.power_armor_power = 0;
	skull->monsterinfo.power_armor_type = POWER_ARMOR_NONE;
	skull->clipmask = MASK_MONSTERSOLID;
	skull->movetype = MOVETYPE_FLY;
	skull->s.renderfx |= RF_IR_VISIBLE;
	// nope
	//skull->flags |= FL_CHASEABLE; // 3.65 indicates entity can be chase cammed
	skull->solid = SOLID_BBOX;
	skull->think = skull_think;
	skull->die = skull_die;
	skull->mtype = M_SKULL;
	skull->touch = skull_touch;
	VectorSet(skull->mins, -16, -16, -16);
	VectorSet(skull->maxs, 16, 16, 16);
	skull->s.modelindex = gi.modelindex("models/monsters/idg2/head/tris.md2"); //"models/skull/tris.md2"
	skull->nextthink = level.time + SKULL_DELAY;
	skull->monsterinfo.upkeep_delay = skull->nextthink*10 + 10; // 3.2 upkeep starts 1 second after spawn
	skull->monsterinfo.cost = cost;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -3, ent->client->kick_origin);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, 64, forward, end);

	tr = gi.trace(start, skull->mins, skull->maxs, end, ent, MASK_SHOT);
	
	if (tr.fraction < 1)
	{
		G_FreeEdict(skull);
		return;
	}

	VectorCopy(end, skull->s.origin);
	VectorCopy(end, skull->move_origin);
	gi.linkentity(skull);

	ent->skull = skull;
	ent->client->pers.inventory[power_cube_index] -= cost;
	ent->client->ability_delay = level.time + SKULL_DELAY;
	ent->holdtime = level.time + SKULL_DELAY;
}

void RemoveHellspawn (edict_t *ent)
{
	if (ent->skull && ent->skull->inuse)
		skull_remove(ent->skull);
}

void skull_attackcmd (edict_t *self)
{
	vec3_t	forward, right, start, end, offset;
	trace_t	tr;

	AngleVectors (self->activator->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7,  self->activator->viewheight-8);
	P_ProjectSource (self->activator->client, self->activator->s.origin, offset, forward, right, start);
	
	VectorMA(start, 8192, forward, end);

	tr = gi.trace(start, NULL, NULL, end, self->activator, MASK_SHOT);

	if (G_ValidTarget(self, tr.ent, false))
	{
		self->enemy = tr.ent;
		self->monsterinfo.search_frames = 0;
		safe_cprintf(self->activator, PRINT_HIGH, "Skull will attack target.\n");
	}

}

void Cmd_HellSpawn_f (edict_t *ent)
{
	int cost = SKULL_COST;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_HellSpawn_f()\n", ent->client->pers.netname);

	if (!Q_strcasecmp(gi.args(), "attack") && ent->skull && ent->skull->inuse)
	{
		skull_attackcmd(ent->skull);
		return;
	}
/*
	if (!Q_strcasecmp(gi.args(), "recall") && ent->skull && ent->skull->inuse)
	{
		// toggle
		if (!ent->skull->lockon)
		{
			safe_cprintf(ent, PRINT_HIGH, "Hellspawn recall enabled.\n");
			ent->skull->lockon = 1;
		}
		else
		{
			safe_cprintf(ent, PRINT_HIGH, "Hellspawn recall disabled.\n");
			ent->skull->lockon = 0;
		}
		return;
	}
*/

	if (G_EntExists(ent->skull))
	{
		// try to restore previous owner
		if (!RestorePreviousOwner(ent->skull))
		{
			if (ent->skull->health >= ent->skull->max_health)
				ent->client->pers.inventory[power_cube_index] += ent->skull->monsterinfo.cost;
			RemoveHellspawn(ent);
			safe_cprintf(ent, PRINT_HIGH, "Hell spawn removed.\n");
		}
		else
		{
			safe_cprintf(ent, PRINT_HIGH, "Conversion removed.\n");
		}
		return;
	}

	if(ent->myskills.abilities[HELLSPAWN].disable)
		return;

	// cost is doubled if you are a flyer below skill level 5
	if (ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5)
		cost *= 2;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[HELLSPAWN].current_level, cost))
		return;

	if (ctf->value && (CTF_DistanceFromBase(ent, NULL, CTF_GetEnemyTeam(ent->teamnum)) < CTF_BASE_DEFEND_RANGE))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build in enemy base!\n");
		return;
	}

	SpawnSkull(ent);
}
