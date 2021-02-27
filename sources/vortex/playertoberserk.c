#include "g_local.h"

#define BERSERK_RUN_FORWARD		1
#define BERSERK_RUN_BACKWARD	2
#define BERSERK_FRAMES_RUN_START			36
#define BERSERK_FRAMES_RUN_END				41
#define BERSERK_FRAMES_RUNATTACK1_START		122//110
#define BERSERK_FRAMES_RUNATTACK1_END		127
#define BERSERK_FRAMES_RUNATTACK2_START		128
#define BERSERK_FRAMES_RUNATTACK2_END		145
#define BERSERK_FRAMES_ATTACK1_START		42		// slash up
#define BERSERK_FRAMES_ATTACK1_END			54
#define BERSERK_FRAMES_ATTACK2_START		55
#define BERSERK_FRAMES_ATTACK2_END			69//75		// punch
#define BERSERK_FRAMES_ATTACK3_START		76		// slash up, punch, hammer
#define BERSERK_FRAMES_ATTACK3_END			109

#define BERSERK_FRAMES_DUCK_START			169
#define BERSERK_FRAMES_DUCK_END				178
#define BERSERK_FRAMES_JUMP_END				172
#define BERSERK_FRAMES_IDLE1_START			0
#define BERSERK_FRAMES_IDLE1_END			4
#define BERSERK_FRAMES_IDLE2_START			5
#define BERSERK_FRAMES_IDLE2_END			24


#define BERSERK_FRAMES_SLAM_START			150
#define BERSERK_FRAMES_SLAM_END				155//168

#define BERSERK_FRAMES_SLASH_START			78
#define BERSERK_FRAMES_SLASH_END			83

#define BERSERK_FRAMES_PUNCH_START			64
#define BERSERK_FRAMES_PUNCH_END			69

qboolean curse_add(edict_t *target, edict_t *caster, int type, int curse_level, float duration);

int p_berserk_melee (edict_t *self, vec3_t forward, vec3_t dir, int damage, int knockback, int range, int mod)
{
	vec3_t	start, end;
	trace_t	tr;

	self->lastsound = level.framenum;

	// damage zone
	VectorCopy(self->s.origin, start);
	start[2] += self->viewheight-8;
	VectorMA(start, range, forward, end);

	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

	// bfg laser effect
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	//gi.multicast (start, MULTICAST_PHS);
	gi.unicast(self, true);

	//safe_cprintf(self, PRINT_HIGH, "Attack\n");

	if (G_EntExists(tr.ent))
	{
		if (dir)
			T_Damage(tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, knockback, 0, mod);
		else
			T_Damage(tr.ent, self, self, forward, tr.endpos, tr.plane.normal, damage, knockback, 0, mod);

		// berserk slash attack has a chance to cause bleeding
		if (!OnSameTeam(self, tr.ent) && (mod == MOD_BERSERK_SLASH) && (random() > 0.5))
		{
			curse_add(tr.ent, self, BLEEDING, self->myskills.abilities[BERSERK].current_level, 10.0);
			if (tr.ent->client)
				safe_cprintf(tr.ent, PRINT_HIGH, "You are bleeding!\n");
		}

		return MELEE_HIT_ENT; // hit a damageable ent
	}
	
	if (tr.fraction < 1)
		return MELEE_HIT_WORLDSPAWN; // hit a wall
	else
		return MELEE_HIT_NOTHING; // hit nothing
}

void p_berserk_crush (edict_t *self, int damage, float range, int mod)
{
	trace_t tr;
	edict_t *other=NULL;
	vec3_t	v;

	// must be on the ground to punch
	if (!self->groundentity)
		return;

	self->lastsound = level.framenum;

	gi.sound (self, CHAN_AUTO, gi.soundindex ("tank/tnkatck5.wav"), 1, ATTN_NORM, 0);
	
	while ((other = findradius(other, self->s.origin, range)) != NULL)
	{
		if (!G_ValidTarget(self, other, true))
			continue;
		if (!nearfov(self, other, 0, 30))
			continue;

		VectorSubtract(other->s.origin, self->s.origin, v);
		VectorNormalize(v);
		tr = gi.trace(self->s.origin, NULL, NULL, other->s.origin, self, (MASK_PLAYERSOLID | MASK_MONSTERSOLID));
		T_Damage (other, self, self, v, other->s.origin, tr.plane.normal, damage, damage, 0, mod);
		other->velocity[2] += damage / 2;
	}
}

void p_berserk_jump (edict_t *ent)
{
	// run jump animation forward until last frame, then hold it
	if (ent->s.frame != BERSERK_FRAMES_JUMP_END)
		G_RunFrames(ent, BERSERK_FRAMES_DUCK_START, BERSERK_FRAMES_DUCK_END, false);
}

void p_berserk_swing (edict_t *ent)
{
	if ((ent->s.frame == 64) || (ent->s.frame == 78) || (ent->s.frame == 122) || (ent->s.frame == 150))
		gi.sound (ent, CHAN_WEAPON, gi.soundindex ("berserk/attack.wav"), 1, ATTN_STATIC, 0);
}

void p_berserk_attack (edict_t *ent, int move_state)
{
	int		punch_dmg = BERSERK_PUNCH_INITIAL_DAMAGE + BERSERK_PUNCH_ADDON_DAMAGE * ent->myskills.abilities[BERSERK].current_level;
	int		slash_dmg = BERSERK_SLASH_INITIAL_DAMAGE + BERSERK_SLASH_ADDON_DAMAGE * ent->myskills.abilities[BERSERK].current_level;
	int		crush_dmg = BERSERK_CRUSH_INITIAL_DAMAGE + BERSERK_CRUSH_ADDON_DAMAGE * ent->myskills.abilities[BERSERK].current_level;
	vec3_t	forward, right, up, angles;

	ent->client->idle_frames = 0;
	AngleVectors(ent->s.angles, NULL, right, up);
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorCopy(ent->client->v_angle, angles);
	
	if (move_state == BERSERK_RUN_FORWARD)
	{
		G_RunFrames(ent, BERSERK_FRAMES_RUNATTACK1_START, BERSERK_FRAMES_RUNATTACK1_END, false);

		// swing left-right
		if (ent->s.frame == 124)
		{
			angles[YAW] += 20;
			AngleCheck(&angles[YAW]);
			AngleVectors(angles, forward, NULL, NULL);

			p_berserk_melee(ent, forward, NULL, punch_dmg, BERSERK_PUNCH_KNOCKBACK, BERSERK_PUNCH_RANGE, MOD_BERSERK_PUNCH);
		}
		else if (ent->s.frame == 125)
		{
			p_berserk_melee(ent, forward, NULL, punch_dmg, BERSERK_PUNCH_KNOCKBACK, BERSERK_PUNCH_RANGE, MOD_BERSERK_PUNCH);
		}
		else if (ent->s.frame == 126)
		{
			angles[YAW] -= 20;
			AngleCheck(&angles[YAW]);
			AngleVectors(angles, forward, NULL, NULL);

			p_berserk_melee(ent, forward, NULL, punch_dmg, BERSERK_PUNCH_KNOCKBACK, BERSERK_PUNCH_RANGE, MOD_BERSERK_PUNCH);
		}
	}
	else if (move_state == BERSERK_RUN_BACKWARD)
	{
		G_RunFrames(ent, BERSERK_FRAMES_RUNATTACK1_START, BERSERK_FRAMES_RUNATTACK1_END, true);

		// swing left-right
		if (ent->s.frame == 124)
		{
			angles[YAW] += 20;
			AngleCheck(&angles[YAW]);
			AngleVectors(angles, forward, NULL, NULL);

			p_berserk_melee(ent, forward, NULL, punch_dmg, BERSERK_PUNCH_KNOCKBACK, BERSERK_PUNCH_RANGE, MOD_BERSERK_PUNCH);
		}
		else if (ent->s.frame == 125)
		{
			p_berserk_melee(ent, forward, NULL, punch_dmg, BERSERK_PUNCH_KNOCKBACK, BERSERK_PUNCH_RANGE, MOD_BERSERK_PUNCH);
		}
		else if (ent->s.frame == 126)
		{
			angles[YAW] -= 20;
			AngleCheck(&angles[YAW]);
			AngleVectors(angles, forward, NULL, NULL);

			p_berserk_melee(ent, forward, NULL, punch_dmg, BERSERK_PUNCH_KNOCKBACK, BERSERK_PUNCH_RANGE, MOD_BERSERK_PUNCH);
		}
	}
	else if (ent->client->weapon_mode == 1)	// slash
	{
		G_RunFrames(ent, BERSERK_FRAMES_SLASH_START, BERSERK_FRAMES_SLASH_END, false);

		if ((ent->s.frame == 79) || (ent->s.frame == 80))
			p_berserk_melee(ent, forward, up, slash_dmg, BERSERK_SLASH_KNOCKBACK, BERSERK_SLASH_RANGE, MOD_BERSERK_SLASH);
	}
	else if (ent->client->weapon_mode == 2)	// crush
	{
		G_RunFrames(ent, BERSERK_FRAMES_SLAM_START, BERSERK_FRAMES_SLAM_END, false);

		if (ent->s.frame == 154)
			p_berserk_crush(ent, crush_dmg, BERSERK_CRUSH_RANGE, MOD_BERSERK_CRUSH);
	}
	else // punch
	{
		G_RunFrames(ent, BERSERK_FRAMES_PUNCH_START, BERSERK_FRAMES_PUNCH_END, false);
		
		// swing left-right
		if (ent->s.frame == 66)
		{
			angles[YAW] += 20;//45;
			AngleCheck(&angles[YAW]);
			AngleVectors(angles, forward, NULL, NULL);

			p_berserk_melee(ent, forward, right, punch_dmg, BERSERK_PUNCH_KNOCKBACK, BERSERK_PUNCH_RANGE, MOD_BERSERK_PUNCH);
		}
		else if (ent->s.frame == 67)
		{
			p_berserk_melee(ent, forward, right, punch_dmg, BERSERK_PUNCH_KNOCKBACK, BERSERK_PUNCH_RANGE, MOD_BERSERK_PUNCH);
		}
		else if (ent->s.frame == 68)
		{
			angles[YAW] -= 20;//45;
			AngleCheck(&angles[YAW]);
			AngleVectors(angles, forward, NULL, NULL);

			p_berserk_melee(ent, forward, right, punch_dmg, BERSERK_PUNCH_KNOCKBACK, BERSERK_PUNCH_RANGE, MOD_BERSERK_PUNCH);
		}
	}

	p_berserk_swing(ent);
}

void RunBerserkFrames (edict_t *ent, usercmd_t *ucmd)
{
	if ((ent->mtype != MORPH_BERSERK) || (ent->deadflag == DEAD_DEAD))
		return;

	ent->s.modelindex2 = 0; // no weapon model
	ent->s.skinnum = 0;

	if (level.framenum >= ent->count)
	{
		int regen_frames = BERSERK_REGEN_FRAMES;

		// morph mastery reduces regen time
		if (ent->myskills.abilities[MORPH_MASTERY].current_level > 0)
			regen_frames *= 0.5;

		M_Regenerate(ent, regen_frames, BERSERK_REGEN_DELAY, 1.0, true, false, false, &ent->monsterinfo.regen_delay1);
		
		// play running animation if we are moving forward or strafing
		if ((ucmd->forwardmove > 0) || (!ucmd->forwardmove && ucmd->sidemove))
		{
			if ((ent->client->buttons & BUTTON_ATTACK) && (level.time > ent->monsterinfo.attack_finished))
				p_berserk_attack(ent, BERSERK_RUN_FORWARD);
			else
				G_RunFrames(ent, BERSERK_FRAMES_RUN_START, BERSERK_FRAMES_RUN_END, false);
		}
		// play animation in reverse if we are going backwards
		else if (ucmd->forwardmove < 0)
		{
			if ((ent->client->buttons & BUTTON_ATTACK) && (level.time > ent->monsterinfo.attack_finished))
				p_berserk_attack(ent, BERSERK_RUN_BACKWARD);
			else
				G_RunFrames(ent, BERSERK_FRAMES_RUN_START, BERSERK_FRAMES_RUN_END, true);
		}
		// standing attack
		else if ((ent->client->buttons & BUTTON_ATTACK) && (level.time > ent->monsterinfo.attack_finished))
			p_berserk_attack(ent, 0);
		// play jump animation if we are off ground and not submerged in water
		else if (!ent->groundentity && (ent->waterlevel < 2))
			p_berserk_jump(ent);
		else
			G_RunFrames(ent, BERSERK_FRAMES_IDLE1_START, BERSERK_FRAMES_IDLE1_END, false); // run idle frames

		ent->count = level.framenum + 1;
	}
}

void Cmd_PlayerToBerserk_f (edict_t *ent)
{
	int cost = BERSERK_COST;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_PlayerToBerserk_f()\n", ent->client->pers.netname);

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
		cost *= 1.0 - 0.25 * getTalentLevel(ent, TALENT_MORPHING);

	if (!V_CanUseAbilities(ent, BERSERK, cost, true))
		return;

	if (HasFlag(ent) && !hw->value)
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't morph while carrying flag!\n");
		return;
	}

	V_ModifyMorphedHealth(ent, MORPH_BERSERK, true);

	ent->monsterinfo.attack_finished = level.time + 0.5;// can't attack immediately

	ent->client->pers.inventory[power_cube_index] -= cost;
	ent->client->ability_delay = level.time + BERSERK_DELAY;

	ent->mtype = MORPH_BERSERK;
	ent->s.modelindex = gi.modelindex ("models/monsters/berserk/tris.md2");
	ent->s.modelindex2 = 0;
	ent->s.skinnum = 0;

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
	
	ent->client->refire_frames = 0; // reset charged weapon
	ent->client->weapon_mode = 0; // reset weapon mode
	ent->client->pers.weapon = NULL;
	ent->client->ps.gunindex = 0;
	lasersight_off(ent);

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/morph.wav") , 1, ATTN_NORM, 0);
}