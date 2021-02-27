#include "g_local.h"
#include "boss.h"

qboolean CanDoubleJump (edict_t *ent, usercmd_t *ucmd);

qboolean IsABoss(edict_t *ent)
{
	return (ent && ent->inuse && !ent->client && ((ent->mtype == BOSS_TANK) || (ent->mtype == BOSS_MAKRON)));
}

qboolean IsBossTeam (edict_t *ent)
{
	edict_t *e = G_GetClient(ent);

	return (e && IsABoss(e->owner));
}


void boss_update (edict_t *ent, usercmd_t *ucmd)
{
	int		div=15, forwardspeed, sidespeed, maxspeed;
	int		frames=0;
	vec3_t	forward, right, angles;
	edict_t *boss;

	boss = ent->owner;
	// make sure this is a valid boss entity
	if (!G_EntIsAlive(boss))
		return;
	if (!IsABoss(boss) && (boss->mtype != P_TANK))
		return;

	VectorCopy(ent->s.origin, ent->s.old_origin);
	// update player state
	//ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	//ent->client->ability_delay = level.time + FRAMETIME;
	// copy player angles to boss
	boss->s.angles[YAW] = ent->s.angles[YAW];
	boss->s.angles[PITCH] = 0;
	boss->s.angles[ROLL] = 0;
	AngleVectors(boss->s.angles, forward, right, NULL);
	vectoangles(right, angles);

	// move player into position
	boss_position_player(ent, boss);

	// speed divider, lower is faster (ucmd is 400)
	if (boss->mtype == BOSS_TANK)
		div = 20;
	else if (boss->mtype == BOSS_MAKRON)
		div = 10;
	else
		div = 26;

	// speed limiter, dont allow client to speed cheat using higher cl_speeds
	
	maxspeed = 400;
	forwardspeed = ucmd->forwardmove;
	sidespeed = ucmd->sidemove;
	//gi.dprintf("%d %d\n", forwardspeed, sidespeed);

	if ((forwardspeed > 0) && forwardspeed > maxspeed)
		forwardspeed = maxspeed;
	else if (forwardspeed < -maxspeed)
		forwardspeed = -maxspeed;
	
	if ((sidespeed > 0) && sidespeed > maxspeed)
		sidespeed = maxspeed;
	else if (sidespeed < -maxspeed)
		sidespeed = -maxspeed;
	
	//4.2 allow superspeed
	if (ent->superspeed && (level.time > ent->lasthurt + DAMAGE_ESCAPE_DELAY))
	{
		forwardspeed *= 3;
		sidespeed *= 3;
		maxspeed = 3*BOSS_MAXVELOCITY;
	}

	else if (boss->monsterinfo.air_frames)// used for boost
		maxspeed = 9999;
	else
		maxspeed = BOSS_MAXVELOCITY;

	if (level.framenum >= boss->count)
	{
		if (!(ent->client->buttons & BUTTON_ATTACK))
		{
			if (forwardspeed > 0)
			{
				M_walkmove(boss, boss->s.angles[YAW], forwardspeed/div);
				frames = FRAMES_RUN_FORWARD;
			}
			else if (forwardspeed < 0)
			{
				M_walkmove(boss, boss->s.angles[YAW], forwardspeed/div);
				frames = FRAMES_RUN_BACKWARD;
			}

			if (sidespeed > 0)
			{
				M_walkmove(boss, angles[YAW], sidespeed/div);
				frames = FRAMES_RUN_FORWARD;
			}
			else if (sidespeed < 0)
			{
				M_walkmove(boss, angles[YAW], sidespeed/div);
				frames = FRAMES_RUN_FORWARD;
			}
		}
		else
		{
			frames = FRAMES_ATTACK;
		}

		if (boss->groundentity)
		{
			boss->monsterinfo.air_frames = 0;//we're done boosting
			VectorClear(boss->velocity);
			if (ucmd->upmove > 0 && !ent->client->jump)
			{
				//gi.sound (ent, CHAN_WEAPON, gi.soundindex ("tank/sight1.wav"), 1, ATTN_NORM, 0);
				// jump in the direction we are trying to move
				boss->velocity[0] = 0.1*((forwardspeed*forward[0])+(sidespeed*right[0]));
				boss->velocity[1] = 0.1*((forwardspeed*forward[1])+(sidespeed*right[1]));
				boss->velocity[2] = 350;
				
				ent->client->jump = true;
			}
			
			boss->v_flags &= ~SFLG_DOUBLEJUMP;
		}
		else
		{
			if (ucmd->upmove < 1)
				ent->client->jump = false;

			if (CanDoubleJump(boss, ucmd))
			{
				boss->velocity[2] += 350;
				boss->v_flags |= SFLG_DOUBLEJUMP;
			}

			// steer in the direction we are trying to move
			boss->velocity[0] += 0.1*((forwardspeed*forward[0])+(sidespeed*right[0]));
			boss->velocity[1] += 0.1*((forwardspeed*forward[1])+(sidespeed*right[1]));
			
			if (ucmd->upmove && (boss->waterlevel > 1))
			{
				if (ucmd->upmove > 0)
				{
					if (boss->waterlevel == 2)
						boss->velocity[2] = 200;
					else if (boss->velocity[2] < 50)
						boss->velocity[2] = 50;
				}
				else
				{
					if (boss->velocity[2] > -50)
						boss->velocity[2] = -50;
				}
			}

			// don't move too fast
			if (boss->velocity[0] > maxspeed)
				boss->velocity[0] = maxspeed;
			if (boss->velocity[0] < -maxspeed)
				boss->velocity[0] = -maxspeed;
			if (boss->velocity[1] > maxspeed)
				boss->velocity[1] = maxspeed;
			if (boss->velocity[1] < -maxspeed)
				boss->velocity[1] = -maxspeed;
		}
		
		boss->style = frames; // set frame set boss should use
		gi.linkentity(boss);
		boss->count = level.framenum+1;
	}
	
	// move player into position
	//boss_position_player(ent, boss);

	// if we are standing on a live entity, call its touch function
	// this prevents player-bosses from being invulnerable to obstacles
	//FIXME: it may be dangerous to call a touch function without a NULL plane or surf
	/*
	if (boss->groundentity && G_EntIsAlive(boss->groundentity) && boss->groundentity->touch)
		boss->groundentity->touch(boss->groundentity, boss, NULL, NULL);
		*/
}

qboolean validDmgPlayer (edict_t *player)
{
	return (player && player->inuse && player->client && !G_IsSpectator(player));
}

void dmgListCleanup (edict_t *self, qboolean clear_all)
{
	int i;

	for (i=0; i < MAX_CLIENTS; i++) 
	{
		// is this a valid player?
		if (!clear_all && validDmgPlayer(self->monsterinfo.dmglist[i].player))
			continue;

		// clear this entry
		self->monsterinfo.dmglist[i].player = NULL;
		self->monsterinfo.dmglist[i].damage = 0;
	}
}

dmglist_t *findDmgSlot (edict_t *self, edict_t *other)
{
	int i;

	for (i=0; i < MAX_CLIENTS; i++) {
		if (self->monsterinfo.dmglist[i].player == other)
			return &self->monsterinfo.dmglist[i];
	}
	return NULL;
}

dmglist_t *findEmptyDmgSlot (edict_t *self)
{
	int i;

	for (i=0; i < MAX_CLIENTS; i++) {
		if (!validDmgPlayer(self->monsterinfo.dmglist[i].player))
			return &self->monsterinfo.dmglist[i];
	}
	return NULL;
}

float GetTotalBossDamage (edict_t *self)
{
	int		i;
	float	dmg=0;

	for (i=0; i < MAX_CLIENTS; i++) {
		if (validDmgPlayer(self->monsterinfo.dmglist[i].player))
			dmg += self->monsterinfo.dmglist[i].damage;
	}

	if (dmg < 1)
		dmg = 1;

	return dmg;
}

float GetPlayerBossDamage (edict_t *player, edict_t *boss)
{
	int		i;

	for (i=0; i < MAX_CLIENTS; i++) {
		if (boss->monsterinfo.dmglist[i].player == player)
			return boss->monsterinfo.dmglist[i].damage;
	}
	return 0;
}

dmglist_t *findHighestDmgPlayer (edict_t *self)
{
	int			i;
	dmglist_t	*slot=NULL;

	for (i=0; i < MAX_CLIENTS; i++) 
	{
		if (!validDmgPlayer(self->monsterinfo.dmglist[i].player))
			continue;

		if (!self->monsterinfo.dmglist[i].damage)
			continue;

		if (!slot)
			slot = &self->monsterinfo.dmglist[i];
		else if (self->monsterinfo.dmglist[i].damage > slot->damage)
			slot = &self->monsterinfo.dmglist[i];
	}

	return slot;
}

void printDmgList (edict_t *self) 
{
	int			 i;
	float		percent;
	dmglist_t	*slot;

	for (i=0; i < MAX_CLIENTS; i++) 
	{
		slot = &self->monsterinfo.dmglist[i];
		if (self->monsterinfo.dmglist[i].player)
		{
			percent = 100*(slot->damage/GetTotalBossDamage(self));

			if (self->client)
				gi.dprintf("(%s) slot %d: %s, %.0f damage (%.1f%c)\n", 
					self->client->pers.netname, i, slot->player->client->pers.netname, slot->damage, percent, '%');
			else if (self->mtype)
				gi.dprintf("(%s) slot %d: %s, %.0f damage (%.1f%c)\n", 
					V_GetMonsterName(self), i, slot->player->client->pers.netname, slot->damage, percent, '%');
			else
				gi.dprintf("(%s) slot %d: %s, %.0f damage (%.1f%c)\n", 
					self->classname, i, slot->player->client->pers.netname, slot->damage, percent, '%');
		}
	}	
}

void AddDmgList (edict_t *self, edict_t *other, int damage)
{
	edict_t		*cl_ent;
	dmglist_t	*slot;

	if (damage < 1)
		return;

	// sanity checks
	if (!self || !self->inuse || self->deadflag == DEAD_DEAD)
		return;
	if (!other || !other->inuse || other->deadflag == DEAD_DEAD)
		return;
	
	// don't count self-inflicted damage
	if (other == self)
		return;

	// we're only adding non-spectator clients to this list
	cl_ent = G_GetClient(other);
	if (!validDmgPlayer(cl_ent))
		return;

	// prune list of disconnected clients
	dmgListCleanup(self, false);

	// already in the queue?
	if (slot=findDmgSlot(self, cl_ent))
	{
		slot->damage += damage;
	}
	// add attacker to the queue
	else if (slot=findEmptyDmgSlot(self))
	{
		slot->player = cl_ent;
		slot->damage += damage;
	}

	//gi.dprintf("adding %d damage\n", damage);
	//printDmgList(self);
}

void boss_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	//edict_t		*cl_ent;
	//dmglist_t	*slot;

	if (self->health < 0.3*self->max_health)
		self->s.skinnum |= 1; // use damaged skin
	AddDmgList(self, other, damage);
}

// returns true if there is a nearby, visible player boss
qboolean findNearbyBoss (edict_t *self)
{
	edict_t *other=NULL;

	while ((other = findradius(other, self->s.origin, BOSS_ALLY_BONUS_RANGE)) != NULL)
	{
		if (!IsABoss(other))
			continue;
		if (!G_EntIsAlive(other))
			continue;
		return true;
	}
	return false;
}

void boss_eyecam (edict_t *player, edict_t *boss)
{
	vec3_t forward, goal;

	AngleVectors(boss->s.angles, forward, NULL, NULL);
	VectorCopy(boss->s.origin, goal);
	goal[2] = boss->absmax[2]-24;
	VectorMA(goal, boss->maxs[1], forward, goal); // move cam forward

	// move player into position
	VectorCopy(goal, player->s.origin);
	gi.linkentity(player);
}

#define BOSS_CHASEMODE_BUFFER	10 // time in seconds before allowing mode switch

void boss_position_player (edict_t *player, edict_t *boss)
{
	float	hdist, vdist;
	vec3_t	forward, start, goal;
	vec3_t	boxmin={-4,-4,0};
	vec3_t	boxmax={4,4,0};
	trace_t	tr;

	if (boss->monsterinfo.trail_time > level.time)
	{
		boss_eyecam(player, boss);
		return;
	}

	if (boss->mtype == BOSS_TANK)
	{
		hdist = boss->mins[1]-96;
		vdist = boss->absmax[2]+8;
	}
	else if (boss->mtype == BOSS_MAKRON)
	{
		hdist = boss->mins[1]-32;
		vdist = boss->absmax[2]+64;
	}

	// get starting position
	VectorCopy(boss->s.origin, start);
	start[2] += 40;

	// get ideal viewing position
	AngleVectors(boss->s.angles, forward, NULL, NULL);
	VectorCopy(start, goal);
	goal[2] = vdist; // move cam up
	VectorMA(goal, hdist, forward, goal); // move cam backward
	tr = gi.trace(start, boxmin, boxmax, goal, boss, MASK_SOLID);
	VectorCopy(tr.endpos, goal);

	// check for ceiling
	VectorCopy(goal, start);
	start[2] += 128;
	// trace from goal to point above it
	tr = gi.trace(goal, boxmin, boxmax, start, boss, MASK_SHOT);
	// if there's a collision, we've got a ceiling over us, so stay well below it
	if ((tr.fraction < 1) && (goal[2] > tr.endpos[2]-28))
		goal[2] = tr.endpos[2]-28;

	// check for minimum height; below this makes it difficult to see past the boss
	if (boss->groundentity && (goal[2] < boss->absmax[2]-32))
	{
		boss_eyecam(player, boss);
		// delay before going back to 3rd-person view, so player isn't
		// spammed by switching chasecam modes
		boss->monsterinfo.trail_time = level.time + BOSS_CHASEMODE_BUFFER; // delay before switching mode
		return;
	}

	// move player into position
	VectorCopy(goal, player->s.origin);
	gi.linkentity(player);
}

qboolean boss_findtarget (edict_t *boss)
{
	edict_t *target = NULL;

	while ((target = findclosestradius(target, boss->s.origin, BOSS_TARGET_RADIUS)) != NULL)
	{
		if (!G_ValidTarget(boss, target, true))
			continue;
		if (!infront(boss, target))
			continue;
		boss->enemy = target;
		//if (target->client)
		//	gi.dprintf("found %s\n", target->client->pers.netname);
		//else
		//	gi.dprintf("found target %s\n", target->classname);
	//	gi.sound (boss, CHAN_WEAPON, gi.soundindex ("tank/sight1.wav"), 1, ATTN_NORM, 0);
		return true;
	}
	return false;
}

qboolean boss_checkstatus (edict_t *self)
{
	if (!self->activator || !self->activator->inuse)
	{
		M_Remove(self, false, true);
		gi.dprintf("WARNING: boss_checkstatus() removed player-less boss!\n");
		return false;
	}

	M_WorldEffects (self);
	M_CatagorizePosition (self);
	//M_SetEffects (self);

	if (level.time > self->wait)
	{
		if (self->health <= (0.1*self->max_health))
			gi.centerprintf(self->owner, "***HEALTH LEVEL CRITICAL***\n");
		else if (self->health <= (0.3*self->max_health))
			gi.centerprintf(self->owner, "Low health warning.\n");
		self->wait = level.time + BOSS_STATUS_DELAY;
	}

	// monitor boss idle frames
	if (!self->style)
		self->monsterinfo.idle_frames++;
	else
		self->monsterinfo.idle_frames = 0;

	//if (self->mtype != P_TANK && !self->activator->myskills.administrator)//4.2 player-tank exception
	//	self->activator->client->ability_delay = level.time + 1; // don't let them use abilities!
	return true;
}

void boss_regenerate (edict_t *self)
{
	int health, armor, regen_frames;

	if (self->waterlevel > 0)
		return; // boss can't regenerate in water (too cheap!)

	if (self->monsterinfo.idle_frames)
		regen_frames = BOSS_REGEN_IDLE_FRAMES;
	else
		regen_frames = BOSS_REGEN_ACTIVE_FRAMES;

	health = self->max_health/regen_frames;
	if (self->health < self->max_health)
		self->health += health;
	if (self->health > self->max_health)
		self->health = self->max_health;

	if (self->health >= 0.3*self->max_health)
		self->s.skinnum &= ~1; // use normal skin

	if (!self->monsterinfo.power_armor_type)
		return;
	armor = self->monsterinfo.max_armor/regen_frames;
	if (self->monsterinfo.power_armor_power < self->monsterinfo.max_armor)
		self->monsterinfo.power_armor_power += armor;
	if (self->monsterinfo.power_armor_power > self->monsterinfo.max_armor)
		self->monsterinfo.power_armor_power = self->monsterinfo.max_armor;
}

qboolean SkipFrame (int frame, int *skip_frames)
{
	int i;

	for (i=0; skip_frames[i]; i++) {
		if (frame == skip_frames[i])
			return true;
	}
	return false;
}

void G_RunFrames1 (edict_t *ent, int start_frame, int end_frame, int *skip_frames, qboolean reverse) 
{
	int next_frame=ent->s.frame;

	if (reverse)
		next_frame = ent->s.frame-1;
	else
		next_frame = ent->s.frame+1;

	while (ent->s.frame != next_frame)
	{
		if (reverse) // run frames backwards
		{
			if (skip_frames && SkipFrame(next_frame, skip_frames)) // skip this frame
			{
				next_frame--;
				continue;
			}
			if ((next_frame >= start_frame) && (next_frame <= end_frame))
				ent->s.frame=next_frame;
			else
				ent->s.frame = next_frame = end_frame;
		}
		else
		{
			if (skip_frames && SkipFrame(next_frame, skip_frames))
			{
				next_frame++;
				continue;
			}
			if ((next_frame <= end_frame) && (next_frame >= start_frame))
				ent->s.frame=next_frame;
			else
				ent->s.frame = next_frame = start_frame;
		}
	}
}