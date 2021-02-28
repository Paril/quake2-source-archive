#include "g_local.h"
#include "m_mutant.h"

#define MUTANT_FRAMES_IDLE_START	62
#define MUTANT_FRAMES_IDLE_END		125
#define MUTANT_FRAMES_WALK_START	56
#define MUTANT_FRAMES_WALK_END		61
#define MUTANT_FRAMES_SWING_START	10
#define MUTANT_FRAMES_SWING_END		14
#define MUTANT_FRAMES_JUMP			4


qboolean curse_add(edict_t *target, edict_t *caster, int type, int curse_level, float duration);

int melee_attack (edict_t *self, int damage, int range)
{
	vec3_t	start, forward, end;
	trace_t	tr;

	self->lastsound = level.framenum;

	// damage zone
	AngleVectors(self->client->v_angle, forward, NULL, NULL);
	VectorCopy(self->s.origin, start);
	start[2] += self->viewheight;
	VectorMA(start, range, forward, end);
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

	if (G_EntExists(tr.ent))
	{
		T_Damage(tr.ent, self, self, forward, tr.endpos, tr.plane.normal, damage, 0, 0, MOD_MUTANT);

		// mutant slash attack has a chance to cause bleeding
		if (!OnSameTeam(self, tr.ent) && (random() > 0.33))
		{
			curse_add(tr.ent, self, BLEEDING, self->myskills.abilities[MUTANT].current_level, 10.0);
			if (tr.ent->client)
				safe_cprintf(tr.ent, PRINT_HIGH, "You are bleeding!\n");
		}

		return MELEE_HIT_ENT; // hit a damageable ent
	}
	
	if (tr.fraction<1)
		return MELEE_HIT_WORLDSPAWN; // hit a wall
	else
		return MELEE_HIT_NOTHING; // hit nothing
}

void mutant_swing_attack (edict_t *self)
{
	int	dmg = MUTANT_INITIAL_SWING_DMG+MUTANT_ADDON_SWING_DMG*self->myskills.abilities[MUTANT].current_level;

	if (self->s.frame == 10) // left
	{
		if (melee_attack(self, dmg, MUTANT_SWING_RANGE)==MELEE_HIT_ENT)
			gi.sound (self, CHAN_AUTO, gi.soundindex ("mutant/mutatck3.wav"), 1, ATTN_IDLE, 0);
		else if (melee_attack(self, dmg, MUTANT_SWING_RANGE)==MELEE_HIT_WORLDSPAWN)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("tank/thud.wav"), 1, ATTN_IDLE, 0);
		else
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("mutant/mutatck1.wav"), 1, ATTN_IDLE, 0);
	}
	else if (self->s.frame == 13) // right
	{
		if (melee_attack(self, dmg, MUTANT_SWING_RANGE)==MELEE_HIT_ENT)
			gi.sound (self, CHAN_AUTO, gi.soundindex ("mutant/mutatck3.wav"), 1, ATTN_IDLE, 0);
		else if (melee_attack(self, dmg, MUTANT_SWING_RANGE)==MELEE_HIT_WORLDSPAWN)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("tank/thud.wav"), 1, ATTN_IDLE, 0);
		else
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("mutant/mutatck1.wav"), 1, ATTN_IDLE, 0);
	}
}

#define MUTANT_BOOST_VALUE	500
#define BOOST_DELAY			5

qboolean mutant_boost (edict_t *ent)
{
	int *next_frame;
	vec3_t forward, right, start, offset;

	if (ent->mtype == MORPH_MUTANT)
		next_frame = &ent->myskills.abilities[MUTANT].ammo_regenframe;
	else if (ent->mtype == MORPH_BRAIN)
		next_frame = &ent->myskills.abilities[BRAIN].ammo_regenframe;
	else
		gi.error ("invalid ent->mtype");

	if (level.framenum > *next_frame)
	{
		ent->lastsound = level.framenum;

		// get movement direction
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 0, 7,  ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		// modify velocity
		ent->velocity[0] += forward[0] * MUTANT_BOOST_VALUE;
		ent->velocity[1] += forward[1] * MUTANT_BOOST_VALUE;
		ent->velocity[2] += forward[2] * MUTANT_BOOST_VALUE;

		// delay to prevent jump spam
		*next_frame = level.framenum + BOOST_DELAY;
		stuffcmd(ent, "-moveup\n");//4.4
		return true;
	}

	return false;
}

float monster_increaseDamageByTalent(edict_t *owner, float damage);
void mutant_checkattack (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// sanity check
	if (!other || !other->inuse)
		return;

	// must be an airborne mutant
	if (((other->mtype != M_MUTANT) && (other->mtype != MORPH_MUTANT)) || (other->groundentity && other->groundentity == world))
		return;

	// check for attack delay (used when respawning/teleporting)
	if (other->monsterinfo.attack_finished > level.time)
		return;

	// check primary attack delay
	if (level.framenum < other->monsterinfo.nextattack)
		return;

	// we should stop attacking if we've landed
	if (other->s.frame != MUTANT_FRAMES_JUMP)
		return;

	// is this a valid target?
	if (G_EntExists(self) && !OnSameTeam(self, other) 
		&& (infront(other, self) || (other->groundentity && (other->groundentity == self)))) // target is within fov or below us
	{
		int		dmg;
		vec3_t	v, point;

		if (other->client)
		{
			VectorCopy(other->client->oldvelocity, v);
			dmg = MUTANT_INITIAL_JUMP_DMG + MUTANT_ADDON_JUMP_DMG * other->myskills.abilities[MUTANT].current_level;
		}
		else
		{
			VectorCopy(other->velocity, v);
			dmg = MUTANT_INITIAL_JUMP_DMG + MUTANT_ADDON_JUMP_DMG * other->monsterinfo.level;
			dmg = monster_increaseDamageByTalent(other->activator, dmg);
		}

		// calculate attack endpoint
		VectorNormalize(v);
		VectorMA (other->s.origin, other->maxs[0], v, point);

		T_Damage(self, other, other, v, point, plane->normal, dmg, dmg, DAMAGE_RADIUS, MOD_MUTANT);

		other->monsterinfo.nextattack = level.framenum + MUTANT_JUMPATTACK_DELAY;
	}
}

void mutant_jumpattack (edict_t *self)
{
	int			dmg;
	vec3_t		boxmin={-1,-1,-8},boxmax={1,1,0};
	trace_t		tr;
	edict_t		*e=NULL;
	qboolean	hit=false;

	// 3.68 check for attack delay (used when respawning/teleporting)
	if (self->monsterinfo.attack_finished > level.time)
		return;

	if (level.framenum >= self->monsterinfo.search_frames)
	{
		dmg = MUTANT_INITIAL_JUMP_DMG+MUTANT_ADDON_JUMP_DMG*self->myskills.abilities[MUTANT].current_level;

		// detect collision within area surrounding bounding box
		VectorAdd(self->maxs, boxmax, boxmax);
		VectorAdd(self->mins, boxmin, boxmin);
		tr = gi.trace(self->s.origin, boxmin, boxmax, self->s.origin, self, (CONTENTS_MONSTERCLIP|CONTENTS_MONSTER|CONTENTS_DEADMONSTER|CONTENTS_PLAYERCLIP));
		
		if (tr.fraction<1) // hit something
		{
			// find nearby targets and hurt them
			// we're using this instead of T_DamageRadius() because don't want a damage curve
			while ((e = findradius(e, self->s.origin, MUTANT_JUMPATTACK_RADIUS)) != NULL)
			{
				if (!G_EntExists(e))
					continue;
				// don't hurt teammates, unless its just a corpse
				if (OnSameTeam(self, e) && (e->deadflag != DEAD_DEAD))
					continue;
				// target must be in our fov or below us
				if (!infov(self, e, 30) && (self->absmin[2]+1 < e->absmax[2]))
					continue;
				hit = true;
				T_Damage(e, self, self, self->velocity, e->s.origin, vec3_origin, dmg, dmg, DAMAGE_RADIUS, MOD_MUTANT);
			}
			// avoid multiple hits
			if (hit)
				self->monsterinfo.search_frames = level.framenum + 5;
		}
	}
}

void RunMutantFrames (edict_t *ent, usercmd_t *ucmd)
{

	// if we aren't a parasite or we are dead, we shouldn't be here!
	if ((ent->mtype != MORPH_MUTANT) || (ent->deadflag == DEAD_DEAD))
		return;

//	if (ucmd->upmove > 0)
//		ent->monsterinfo.slots_freed = true; // try to attack

	ent->s.modelindex2 = 0; // no weapon model
	ent->s.skinnum = 0;

//	if (!ent->groundentity)
//		mutant_jumpattack(ent); // this must run every client frame

	if (level.framenum >= ent->count)
	{
		
		if (!ent->groundentity)
			ent->s.frame = MUTANT_FRAMES_JUMP;
		// play running animation if we are moving forward or strafing
		else if ((ucmd->forwardmove > 0) || ucmd->sidemove)
			G_RunFrames(ent, MUTANT_FRAMES_WALK_START, MUTANT_FRAMES_WALK_END, false);
		// play animation in reverse if we are going backwards
		else if (ucmd->forwardmove < 0)
			G_RunFrames(ent, MUTANT_FRAMES_WALK_START, MUTANT_FRAMES_WALK_END, true);
		else if ((ent->client->buttons & BUTTON_ATTACK) 
			&& (level.time > ent->monsterinfo.attack_finished))
		{
			ent->client->idle_frames = 0;
			// run attack frames
			G_RunFrames(ent, MUTANT_FRAMES_SWING_START, MUTANT_FRAMES_SWING_END, false);
			mutant_swing_attack(ent);
		}
		else
		{
			G_RunFrames(ent, MUTANT_FRAMES_IDLE_START, MUTANT_FRAMES_IDLE_END, false); // run idle frames
		}

		ent->count = level.framenum + 1;
	}
}

void Cmd_PlayerToMutant_f (edict_t *ent)
{
	vec3_t	boxmin, boxmax;
	trace_t	tr;
	int mutant_cubecost = MUTANT_INIT_COST;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_PlayerToMutant_f()\n", ent->client->pers.netname);

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
		mutant_cubecost *= 1.0 - 0.25 * getTalentLevel(ent, TALENT_MORPHING);

//	if (!G_CanUseAbilities(ent, ent->myskills.abilities[MUTANT].current_level, mutant_cubecost))
//		return;
	if (!V_CanUseAbilities(ent, MUTANT, mutant_cubecost, true))
		return;

	if (HasFlag(ent) && !hw->value)
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't morph while carrying flag!\n");
		return;
	}


	// make sure don't get stuck in a wall
	VectorSet (boxmin, -24, -24, -24);
	VectorSet (boxmax, 24, 24, 32);
	tr = gi.trace(ent->s.origin, boxmin, boxmax, ent->s.origin, ent, MASK_SHOT);
	if (tr.fraction<1)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough room to morph!\n");
		return;
	}

	V_ModifyMorphedHealth(ent, MORPH_MUTANT, true);

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
	
	VectorCopy(boxmin, ent->mins);
	VectorCopy(boxmax, ent->maxs);

	ent->monsterinfo.attack_finished = level.time + 0.5;// can't attack immediately

	ent->client->pers.inventory[power_cube_index] -= mutant_cubecost;
	ent->client->ability_delay = level.time + MUTANT_DELAY;

	ent->mtype = MORPH_MUTANT;
	ent->s.modelindex = gi.modelindex ("models/monsters/mutant/tris.md2");
	ent->s.modelindex2 = 0;
	ent->s.skinnum = 0;

	ent->client->refire_frames = 0; // reset charged weapon
	ent->client->weapon_mode = 0; // reset weapon mode
	ent->client->pers.weapon = NULL;
	ent->client->ps.gunindex = 0;

	lasersight_off(ent);

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/morph.wav") , 1, ATTN_NORM, 0);
}