#include "g_local.h"

#define BRAIN_FRAME_TENTACLE_START	76	// continuous tentacle attack
#define BRAIN_FRAME_TENTACLE_END	81
#define BRAIN_FRAME_CHEST_OPEN		75
#define BRAIN_FRAME_IDLE_START		192
#define BRAIN_FRAME_IDLE_END		221
#define BRAIN_WALK_START			0
#define	BRAIN_WALK_END				10
#define BRAIN_JUMP_START			147
#define BRAIN_JUMP_HOLD				148
#define BRAIN_JUMP_END				153

qboolean BrainValidTarget (edict_t *self, edict_t *target)
{
	// 3.6 this code is mostly redundant
	if (target == self)
		return false;
	if (!G_ValidTarget(self, target, true))
		return false;
	if (!nearfov(self, target, 45, 45))
		return false;
	return true;
}

void magmine_seteffects (edict_t *self)
{
	// set team color
	if (self->creator->teamnum == 1)
	{
		self->s.effects |= EF_COLOR_SHELL;
		self->s.renderfx |= (RF_SHELL_RED);
	}
	else if (self->creator->teamnum == 2)
	{
		self->s.effects |= EF_COLOR_SHELL;
		self->s.renderfx |= (RF_SHELL_BLUE);
	}
}

void magmine_throwsparks (edict_t *self)
{
	int		i;
	vec3_t	start, up;

	AngleVectors(self->s.angles, NULL, NULL, up);
	VectorCopy(self->s.origin, start);
	start[2]+=8;

	for (i=0; i<8; i++)
	{
		start[2] += 4;

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_LASER_SPARKS);
		gi.WriteByte(1); // particle count
		gi.WritePosition(start);
		gi.WriteDir(up);
		gi.WriteByte(12); // particle color
		gi.multicast(start, MULTICAST_PVS);
	}
}

qboolean magmine_findtarget (edict_t *self)
{
	edict_t *other=NULL;

	while ((other = findclosestradius(other, 
		self->s.origin, self->dmg_radius)) != NULL)
	{
		if (!G_ValidTarget(self, other, true))
			continue;
		//if (!BrainValidTarget(self, other))
		//	continue;
		self->enemy = other;
		return true;
	}
	return false;
}

void magmine_attack (edict_t *self)
{
	int		pull;
	vec3_t	start, end, dir;

	// magmine will not pull a target that is below it
	// this prevents players from placing magmines to pull a target off their feet
	if (self->enemy->absmin[2]+1 < self->absmin[2])
		return;

	G_EntMidPoint(self->enemy, end);
	G_EntMidPoint(self, start);
	VectorSubtract(end, start, dir);
	VectorNormalize(dir);

	pull = MAGMINE_DEFAULT_PULL + MAGMINE_ADDON_PULL * self->monsterinfo.level;
	if (self->enemy->groundentity)
		pull *= 2;

	// pull them in!
	T_Damage(self->enemy, self, self, dir, end, vec3_origin, 0, pull, 0, 0);

	if (level.time > self->delay)
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/tlaser.wav"), 1, ATTN_IDLE, 0);
		self->delay = level.time + 2;
	}
}

void magmine_think (edict_t *self)
{
	// check for valid position
	if (gi.pointcontents(self->s.origin) & CONTENTS_SOLID)
	{
		gi.dprintf("WARNING: A mag mine was removed from map due to invalid position.\n");
		safe_cprintf(self->creator, PRINT_HIGH, "Your mag mine was removed.\n");
		self->creator->magmine = NULL;
		G_FreeEdict(self);
		return;
	}

	if (!self->enemy)
	{
		if (magmine_findtarget(self))
		{
			magmine_attack(self);
			magmine_throwsparks(self);
		}
	}
	else if (G_ValidTarget(self, self->enemy, true) 
		&& (entdist(self, self->enemy) <= self->dmg_radius))
	{
		magmine_attack(self);
		magmine_throwsparks(self);
	}
	else
	{
		self->enemy = NULL;
	}

	//magmine_seteffects(self);
	M_SetEffects(self);//4.5
	self->nextthink = level.time + FRAMETIME;
}

void magmine_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->deadflag == DEAD_DEAD)
		return;

	if (debuginfo->value)
		gi.dprintf("DEBUG: magmine_die()\n");

	safe_cprintf(self->creator, PRINT_HIGH, "Your mag mine was destroyed.\n");
	self->creator->magmine = NULL;
	//BecomeExplosion1(self);

	// prepare this entity for removal
	self->think = BecomeExplosion1;
	self->nextthink = level.time + FRAMETIME;
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	gi.unlinkentity(self);
}

void magmine_spawn (edict_t *ent, int cost, float skill_mult, float delay_mult)
{
	edict_t *mine;
	vec3_t	forward, right, start, end, offset;
	trace_t	tr;

	if (debuginfo->value)
		gi.dprintf("DEBUG: magmine_spawn()\n");

	mine = G_Spawn();
	mine->creator = ent;
	VectorCopy(ent->s.angles, mine->s.angles);
	mine->s.angles[PITCH] = 270;
	mine->s.angles[ROLL] = 0;
	mine->think = magmine_think;
	mine->touch = V_Touch;
	mine->nextthink = level.time + FRAMETIME;
	mine->s.modelindex = gi.modelindex ("models/objects/minelite/light1/tris.md2");
	mine->solid = SOLID_BBOX;
	mine->movetype = MOVETYPE_TOSS;
	mine->clipmask = MASK_MONSTERSOLID;
	mine->mass = 500;
	mine->classname = "magmine";
	mine->takedamage = DAMAGE_YES;
	mine->monsterinfo.level = ent->myskills.abilities[MAGMINE].current_level * skill_mult;
	mine->health = MAGMINE_DEFAULT_HEALTH+MAGMINE_ADDON_HEALTH*mine->monsterinfo.level;
	mine->max_health = mine->health;
	mine->dmg_radius = MAGMINE_RANGE;
	mine->mtype = M_MAGMINE;//4.5
	mine->die = magmine_die;
	VectorSet(mine->mins, -12, -12, -4);
	VectorSet(mine->maxs, 12, 12, 0);

	// calculate starting position
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, 128, forward, end);
	tr = gi.trace(start, mine->mins, mine->maxs, end, ent, MASK_SHOT);

	if  (tr.fraction < 1)
	{
		// failed to spawn
		G_FreeEdict(mine);
		return;
	}

	VectorCopy(tr.endpos, mine->s.origin);
	ent->magmine = mine; // link to owner
	ent->client->pers.inventory[power_cube_index] -= cost;
	ent->client->ability_delay = level.time + MAGMINE_DELAY * delay_mult;
	ent->holdtime = level.time + MAGMINE_DELAY * delay_mult;
}

void Cmd_SpawnMagmine_f (edict_t *ent)
{
	int talentLevel,cost=MAGMINE_COST;
	float skill_mult=1.0, cost_mult=1.0, delay_mult=1.0;//Talent: Rapid Assembly & Precision Tuning
	char *opt = gi.argv(1);

	if (ent->myskills.abilities[MAGMINE].disable)
		return;

	//Talent: Rapid Assembly
	//talentLevel = getTalentLevel(ent, TALENT_RAPID_ASSEMBLY);
	//if (talentLevel > 0)
		//delay_mult -= 0.1 * talentLevel;
	//Talent: Precision Tuning
	else if ((talentLevel = getTalentLevel(ent, TALENT_PRECISION_TUNING)) > 0)
	{
		cost_mult -= PRECISION_TUNING_COST_FACTOR * talentLevel;
		delay_mult -= PRECISION_TUNING_DELAY_FACTOR * talentLevel;
		skill_mult += PRECISION_TUNING_SKILL_FACTOR * talentLevel;
	}
	cost *= cost_mult;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[MAGMINE].current_level, cost))
		return;

	if (!strcmp(opt, "self"))
	{
		if (getTalentLevel(ent, TALENT_MAGMINESELF))
		{
			ent->automag = !ent->automag;
			safe_cprintf(ent, PRINT_HIGH, "Auto Magmine %s\n", ent->automag? "enabled" : "disabled");
		}else
			safe_cprintf(ent, PRINT_HIGH, "You haven't upgraded this talent.\n");

		return;
	}

	if (ent->magmine && ent->magmine->inuse)
	{
		safe_cprintf(ent, PRINT_HIGH, "Removed mag mine.\n");
		BecomeExplosion1(ent->magmine);
		ent->magmine = NULL;
		return;
	}

	magmine_spawn(ent, cost, skill_mult, delay_mult);
}

void brain_beam_sparks (vec3_t start)
{
	vec3_t forward, angle;

	angle[YAW] = 0;
	angle[PITCH] = 0;
	angle[ROLL] = GetRandom(0, 360);

	AngleVectors(angle, NULL, NULL, forward);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_WELDING_SPARKS);
	gi.WriteByte(GetRandom(5, 15));
	gi.WritePosition(start);
	gi.WriteDir(forward);
	gi.WriteByte(0xd0d1d2d3);
	gi.multicast(start, MULTICAST_PVS);
}

void brain_fire_beam (edict_t *self)
{
	int		damage;
	vec3_t	forward, start, end;
	trace_t	tr;

	if (!V_CanUseAbilities(self, BEAM, BRAIN_BEAM_COST, false) // can we still use abilities?
		|| (self->myskills.abilities[BEAM].charge < BRAIN_BEAM_COST)) // enough charge?
	{
		safe_cprintf(self, PRINT_HIGH, "Insufficient power.\n");
		self->client->firebeam = false;
		return;
	}

	self->client->charge_index = BEAM+1;
	self->myskills.abilities[BEAM].charge -= BRAIN_BEAM_COST;
	//self->client->pers.inventory[cell_index] -= BRAIN_BEAM_COST;
	self->client->idle_frames = 0;

	// calculate starting point
	AngleVectors(self->client->v_angle, forward, NULL, NULL);
	VectorCopy(self->s.origin, start);
	start[2] += self->viewheight;
	VectorMA(start, 8192, forward, end);
	
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
	brain_beam_sparks(tr.endpos);

	damage = BRAIN_BEAM_DEFAULT_DMG+BRAIN_BEAM_ADDON_DMG*self->myskills.abilities[BEAM].current_level;

	T_Damage(tr.ent, self, self, forward, tr.endpos, tr.plane.normal, damage, damage, DAMAGE_ENERGY, MOD_BEAM);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (start, MULTICAST_PHS);

	if (level.time > self->client->beamtime)
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/phrwea.wav"), 1, ATTN_NORM, 0);
		self->client->beamtime = level.time + 2;
	}

	self->client->charge_time = level.time + 3;
	self->client->charge_regentime = level.time + 1;
}

void Cmd_FireBeam_f (edict_t *ent, int toggle)
{
	if (ent->myskills.abilities[BEAM].disable)
		return;

	if (!V_CanUseAbilities(ent, BEAM, BRAIN_BEAM_COST, false))
		return;

	if (!toggle)
	{
		ent->client->firebeam = false;
		return;
	}
	
	//if (ent->client->pers.inventory[cell_index] >= BRAIN_BEAM_COST)
	if (ent->myskills.abilities[BEAM].charge >= BRAIN_BEAM_COST)
		ent->client->firebeam = true;
	else
		safe_cprintf(ent, PRINT_HIGH, "Insufficient power.\n");
}

qboolean tentacle_findtarget (edict_t *self)
{
	edict_t *other=NULL;

	while ((other = findclosestradius(other, self->s.origin, BRAIN_ATTACK_RANGE)) != NULL)
	{
		if (!BrainValidTarget(self, other))
			continue;
		self->enemy = other;
		return true;
	}
	return false;
}

void BrainLockTarget (edict_t *self)
{
	int		i;
	vec3_t	forward, start, end;

	VectorCopy(self->s.origin, start);
	start[2] += self->viewheight;
	G_EntMidPoint(self->enemy, end);
	VectorSubtract(end, start, forward);
	VectorNormalize(forward);
	vectoangles(forward, forward);
	if (forward[PITCH] < -180)
		forward[PITCH] += 360;

	// set view angles to target
	for (i = 0 ; i < 3 ; i++)
		self->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(forward[i]-self->client->resp.cmd_angles[i]);
	VectorCopy(forward, self->client->ps.viewangles);
	VectorCopy(forward, self->client->v_angle);
}

void tentacle_pull (edict_t *self)
{
	int		pull;
	vec3_t	start, v;
	trace_t	tr;
	
	if (entdist(self, self->enemy) < BRAIN_LOCKON_RANGE)
		BrainLockTarget(self);
	
	G_EntMidPoint(self->enemy, start);
	tr = gi.trace(self->s.origin, NULL, NULL, start, self, MASK_SHOT);

	// make sure nothing is in our path
	if (tr.ent && (tr.ent == self->enemy))
	{
		pull = BRAIN_DEFAULT_KNOCKBACK+BRAIN_ADDON_KNOCKBACK
			*self->myskills.abilities[BRAIN].current_level;
		VectorSubtract(start, self->s.origin, v);
		VectorNormalize(v);
		if (self->enemy->groundentity)
			pull *= 2;
		// pull them in!
		T_Damage(self->enemy, self, self, v, tr.endpos, tr.plane.normal, 0, pull, 0, 0);
	}
}

void tentacle_attack (edict_t *self)
{
	int		damage;
	vec3_t	forward, start, end;
	trace_t	tr;

	if (!self->enemy)
	{
		if (tentacle_findtarget(self))
			tentacle_pull(self);
	}
	// make sure our target is still valid
	// 3.6 ignore fov here, since sometimes our enemy
	// snaps behind us due to massive pull forces
	else if (G_ValidTarget(self, self->enemy, true) //BrainValidTarget(self, self->enemy))
		&& (entdist(self, self->enemy) < BRAIN_ATTACK_RANGE))
	{
		tentacle_pull(self);
	}
	else
	{
		// reset target
		self->enemy = NULL;
	}

	// damage zone
	AngleVectors(self->client->v_angle, forward, NULL, NULL);
	VectorCopy(self->s.origin, start);
	start[2] += self->viewheight;
	VectorMA(start, 64, forward, end);
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

	if (G_EntExists(tr.ent))
	{
		damage = BRAIN_DEFAULT_TENTACLE_DMG+BRAIN_ADDON_TENTACLE_DMG
			*self->myskills.abilities[BRAIN].current_level;
		T_Damage(tr.ent, self, self, forward, tr.endpos, tr.plane.normal, damage, 0, 0, MOD_TENTACLE);
	}
}

qboolean BrainCanAttack (edict_t *ent)
{
	if (!ent->groundentity && (ent->waterlevel < 1))
		return false; // feet must be on ground or be in water

	return ((level.time > pregame_time->value) && (level.time > ent->client->respawn_time) 
		&& (!que_typeexists(ent->curses, CURSE_FROZEN)) && (level.time > ent->holdtime));
}

void RunBrainFrames (edict_t *ent, usercmd_t *ucmd)
{
	// don't run frames if we are dead or aren't a brain
	if ((ent->mtype != MORPH_BRAIN) || (ent->deadflag == DEAD_DEAD))
		return;

	ent->s.modelindex2 = 0; // no weapon model
	ent->s.skinnum = 0;

	if (level.framenum >= ent->count)
	{
		// play jump animation if we're off the ground and are not submerged
		if (!ent->groundentity && (ent->waterlevel < 2))
		{
			ent->s.frame = BRAIN_JUMP_HOLD;
		}
		// moving forward animation
		else if ((ucmd->forwardmove > 0) || ucmd->sidemove)
		{
			G_RunFrames(ent, BRAIN_WALK_START, BRAIN_WALK_END, false);
		}
		// play animation in reverse if we are going backwards
		else if (ucmd->forwardmove < 0)
		{
			G_RunFrames(ent, BRAIN_WALK_START, BRAIN_WALK_END, true);
		}
		else if (BrainCanAttack(ent) && (ent->client->buttons & BUTTON_ATTACK))
		{
			// chest open frames
			if ((ent->s.frame < BRAIN_FRAME_CHEST_OPEN) ||
				(ent->s.frame > BRAIN_FRAME_TENTACLE_END))
			{
				gi.sound (ent, CHAN_WEAPON, gi.soundindex("brain/brnatck1.wav"), 1, ATTN_IDLE, 0);
				ent->s.frame = BRAIN_FRAME_CHEST_OPEN;
			}
			else if (level.time > ent->monsterinfo.attack_finished)
			{
				// tentacle attack
				G_RunFrames(ent, BRAIN_FRAME_TENTACLE_START, BRAIN_FRAME_TENTACLE_END, false);
				tentacle_attack(ent);
				ent->client->idle_frames = 0;
			}
		}
		else
		{
			// run idle frames
			G_RunFrames(ent, BRAIN_FRAME_IDLE_START, BRAIN_FRAME_IDLE_END, false);
		}

		ent->count = level.framenum + 1;
	}
}

void Cmd_PlayerToBrain_f (edict_t *ent)
{
	int brain_cubecost = BRAIN_INIT_COST;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_PlayerToBrain_f()\n", ent->client->pers.netname);

	// try to switch back
	if (ent->mtype || PM_PlayerHasMonster(ent))
	{
		// don't let a player-tank unmorph if they are cocooned
		if (ent->owner && ent->owner->inuse && ent->owner->movetype == MOVETYPE_NONE)
			return;

		if (que_typeexists(ent->curses, 0))
		{
			safe_cprintf(ent, PRINT_HIGH, "You can't morph while cursed!\n");
			return;
		}

		V_RestoreMorphed(ent, 0);
		return;
	}

	//Talent: Morphing
	if(getTalentSlot(ent, TALENT_MORPHING) != -1)
		brain_cubecost *= 1.0 - 0.25 * getTalentLevel(ent, TALENT_MORPHING);

	//if (!G_CanUseAbilities(ent, ent->myskills.abilities[BRAIN].current_level, brain_cubecost))
	//	return;

	if (!V_CanUseAbilities(ent, BRAIN, brain_cubecost, true))
		return;

	if (HasFlag(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't morph while carrying flag!\n");
		return;
	}

	ent->client->pers.inventory[power_cube_index] -= brain_cubecost;

	// undo crouching / ducked state
	// try asking their client to get up
	stuffcmd(ent, "-movedown\n");
	// if their client ignores the command, force them up
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
		ent->viewheight = 22;
		ent->maxs[2] += 28;
	}

	ent->monsterinfo.attack_finished = level.time + 0.5;// can't attack immediately

	ent->mtype = MORPH_BRAIN;
	ent->s.modelindex = gi.modelindex ("models/monsters/brain/tris.md2");
	ent->s.modelindex2 = 0;
	ent->s.skinnum = 0;

	ent->client->refire_frames = 0; // reset charged weapon
	ent->client->weapon_mode = 0; // reset weapon mode
	lasersight_off(ent);

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/morph.wav") , 1, ATTN_NORM, 0);
}

