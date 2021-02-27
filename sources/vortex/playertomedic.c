#include "g_local.h"
#include "m_medic.h"

#define MEDIC_FRAMES_IDLE_START		12
#define MEDIC_FRAMES_IDLE_END		101
#define MEDIC_FRAMES_RUN_START		102
#define MEDIC_FRAMES_RUN_END		107
#define MEDIC_FRAMES_JUMP_START		131
#define MEDIC_FRAMES_JUMP_END		137
#define MEDIC_FRAMES_HB_START		191
#define MEDIC_FRAMES_HB_END			206
#define MEDIC_FRAMES_CABLE_START	213
#define MEDIC_FRAMES_CABLE_END		227//236
#define MEDIC_FRAMES_BOLT_START		185//177
#define MEDIC_FRAMES_BOLT_END		190

static vec3_t p_medic_cable_offsets[] =
{
	45.0,  -9.2, 15.5,
	48.4,  -9.7, 15.2,
	47.8,  -9.8, 15.8,
	47.3,  -9.3, 14.3,
	45.4, -10.1, 13.1,
	41.9, -12.7, 12.0,
	37.8, -15.8, 11.2,
	34.3, -18.4, 10.7,
	32.7, -19.7, 10.4,
	32.7, -19.7, 10.4
};

qboolean p_medic_healframe (edict_t *ent)
{
	if ((ent->s.frame >= 218) && (ent->s.frame <= 227))
		return true;
	return false;
}

edict_t *CreateSpiker (edict_t *ent, int skill_level);
edict_t *CreateObstacle (edict_t *ent, int skill_level);
edict_t *CreateGasser (edict_t *ent, int skill_level);
void organ_remove (edict_t *self, qboolean refund);

void p_medic_reanimate (edict_t *ent, edict_t *target)
{
	int		skill_level;
	vec3_t	bmin, bmax;
	edict_t *e;

	skill_level = floattoint((1.0+MEDIC_RESURRECT_BONUS)*ent->myskills.abilities[MEDIC].current_level);

	if (!strcmp(target->classname, "drone") 
		&& (ent->num_monsters + target->monsterinfo.control_cost <= MAX_MONSTERS))
	{
		target->monsterinfo.level = skill_level;
		M_SetBoundingBox(target->mtype, bmin, bmax);

		if (G_IsValidLocation(target, target->s.origin, bmin, bmax) && M_Initialize(ent, target))
		{
			// restore this drone
			target->monsterinfo.slots_freed = false; // reset freed flag
			target->monsterinfo.aiflags &= ~AI_FIND_NAVI;
			target->health = 0.33*target->max_health;
			target->monsterinfo.power_armor_power = 0.33*target->monsterinfo.max_armor;
			target->monsterinfo.resurrected_time = level.time + 2.0;
			target->activator = ent; // transfer ownership!
			target->nextthink = level.time + MEDIC_RESURRECT_DELAY;
			gi.linkentity(target);
			target->monsterinfo.stand(target);

			ent->num_monsters += target->monsterinfo.control_cost;
			ent->num_monsters_real++;
			// gi.bprintf(PRINT_HIGH, "adding %p (%d)\n", target, ent->num_monsters_real);
			safe_cprintf(ent, PRINT_HIGH, "Resurrected a %s. (%d/%d)\n", 
				target->classname, ent->num_monsters, MAX_MONSTERS);
		}
	}
	else if ((!strcmp(target->classname, "bodyque") || !strcmp(target->classname, "player"))
		&& (ent->num_monsters + 1 <= MAX_MONSTERS))
	{
		int		random=GetRandom(1, 3);
		vec3_t	start;

		e = G_Spawn();

		VectorCopy(target->s.origin, start);

		// kill the corpse
		T_Damage(target, target, target, vec3_origin, target->s.origin, 
			vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, 0);

		//4.2 random soldier type with different weapons
		if (random == 1)
		{
			// blaster
			e->mtype = M_SOLDIER;
			e->s.skinnum = 0;
		}
		else if (random == 2)
		{
			// rocket
			e->mtype = M_SOLDIERLT;
			e->s.skinnum = 4;
		}
		else
		{
			// shotgun
			e->mtype = M_SOLDIERSS;
			e->s.skinnum = 2;
		}

		e->activator = ent;
		e->monsterinfo.level = skill_level;
		M_Initialize(ent, e);
		e->health = 0.2*e->max_health;
		e->monsterinfo.power_armor_power = 0.2*e->monsterinfo.max_armor;
		e->s.skinnum |= 1; // injured skin

		e->monsterinfo.stand(e);
		
		if (!G_IsValidLocation(target, start, e->mins, e->maxs))
		{
			start[2] += 24;
			if (!G_IsValidLocation(target, start, e->mins, e->maxs))
			{
				G_FreeEdict(e);
				return;
			}
		}

		VectorCopy(start, e->s.origin);
		gi.linkentity(e);
		e->nextthink = level.time + MEDIC_RESURRECT_DELAY;

		ent->num_monsters += e->monsterinfo.control_cost;
		ent->num_monsters_real++;
		// gi.bprintf(PRINT_HIGH, "adding %p (%d) \n", target, ent->num_monsters_real);

		safe_cprintf(ent, PRINT_HIGH, "Resurrected a soldier. (%d/%d)\n", 
			ent->num_monsters, (int)MAX_MONSTERS);
	}
	else if (!strcmp(target->classname, "spiker") && ent->num_spikers + 1 <= SPIKER_MAX_COUNT)
	{
		e = CreateSpiker(ent, skill_level);

		// make sure the new entity fits
		if (!G_IsValidLocation(target, target->s.origin, e->mins, e->maxs))
		{
			ent->num_spikers--;
			G_FreeEdict(e);
			return;
		}
		
		VectorCopy(target->s.angles, e->s.angles);
		e->s.angles[PITCH] = 0;
		e->monsterinfo.cost = target->monsterinfo.cost;
		e->health = 0.33 * e->max_health;
		e->s.frame = 4;
		VectorCopy(target->s.origin, e->s.origin);
		gi.linkentity(e);

		organ_remove(target, false);
	}
	else if (!strcmp(target->classname, "obstacle") && ent->num_obstacle + 1 <= OBSTACLE_MAX_COUNT)
	{
		e = CreateObstacle(ent, skill_level);

		// make sure the new entity fits
		if (!G_IsValidLocation(target, target->s.origin, e->mins, e->maxs))
		{
			ent->num_obstacle--;
			G_FreeEdict(e);
			return;
		}
		
		VectorCopy(target->s.angles, e->s.angles);
		e->s.angles[PITCH] = 0;
		e->monsterinfo.cost = target->monsterinfo.cost;
		e->health = 0.33 * e->max_health;
		e->s.frame = 6;
		VectorCopy(target->s.origin, e->s.origin);
		gi.linkentity(e);

		organ_remove(target, false);
	}
	else if (!strcmp(target->classname, "gasser") && ent->num_gasser + 1 <= GASSER_MAX_COUNT)
	{
		e = CreateGasser(ent, skill_level);

		// make sure the new entity fits
		if (!G_IsValidLocation(target, target->s.origin, e->mins, e->maxs))
		{
			ent->num_gasser--;
			G_FreeEdict(e);
			return;
		}
		
		VectorCopy(target->s.angles, e->s.angles);
		e->s.angles[PITCH] = 0;
		e->monsterinfo.cost = target->monsterinfo.cost;
		e->health = 0.33 * e->max_health;
		e->s.frame = 0;
		VectorCopy(target->s.origin, e->s.origin);
		gi.linkentity(e);

		organ_remove(target, false);
	}
		
}

void p_medic_heal (edict_t *ent)
{
	vec3_t	forward,  right, offset, start, end, org;
	trace_t	tr;

	if (!p_medic_healframe(ent))
		return;
	
	if (ent->s.frame == 218)
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("medic/medatck2.wav"), 1, ATTN_NORM, 0);
	else if (ent->s.frame == 227)
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("medic/medatck5.wav"), 1, ATTN_NORM, 0);

	// get muzzle location
	AngleVectors (ent->s.angles, forward, right, NULL);
	VectorCopy(p_medic_cable_offsets[ent->s.frame - 218], offset);
	G_ProjectSource(ent->s.origin, offset, forward, right, org);

	// get end position
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, MEDIC_CABLE_RANGE, forward, end);

	tr = gi.trace (org, NULL, NULL, end, ent, MASK_SHOT);

	// bfg laser effect
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (org);
	gi.WritePosition (tr.endpos);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	if (G_EntExists(tr.ent))
	{
		// try to heal them if they are alive
		if ((tr.ent->deadflag != DEAD_DEAD) && (tr.ent->health > 0))
		{
			int frames = floattoint(600/(float)ent->myskills.abilities[MEDIC].current_level);
			
			if (!M_NeedRegen(tr.ent))
				return;

			if (ent->s.frame == 220)
				gi.sound (ent, CHAN_WEAPON, gi.soundindex ("medic/medatck4.wav"), 1, ATTN_NORM, 0);
			
			// heal them
			M_Regenerate(tr.ent, frames, 0, 1.0, true, true, false, &tr.ent->monsterinfo.regen_delay2);

			// hold monsters in-place
			if (tr.ent->svflags & SVF_MONSTER)
				tr.ent->holdtime = level.time + 0.2;

			// remove all curses
			CurseRemove(tr.ent, 0);

			//Give them a short period of curse immunity
			tr.ent->holywaterProtection = level.time + 2.0; //2 seconds immunity
		}
		else
		{
			// try to reanimate/resurrect the corpse
			p_medic_reanimate(ent, tr.ent);
		}
	}
}

void p_medic_hb_regen (edict_t *ent, int regen_frames, int regen_delay)
{
	int ammo;
	int max = ent->myskills.abilities[MEDIC].max_ammo;
	int *current = &ent->myskills.abilities[MEDIC].ammo;
	int *delay = &ent->myskills.abilities[MEDIC].ammo_regenframe;

	if (*current > max)
		return;

	// don't regenerate ammo if player is firing
	if ((ent->client->buttons & BUTTON_ATTACK) && (ent->client->weapon_mode != 1))
		return;

	if (regen_delay > 0)
	{
		if (level.framenum < *delay)
			return;

		*delay = level.framenum + regen_delay;
	}
	else
		regen_delay = 1;

	ammo = floattoint((float)max / ((float)regen_frames / regen_delay));

	if (ammo < 1)
		ammo = 1;

	*current += ammo;
	if (*current > max)
		*current = max;
}

void p_medic_regen (edict_t *ent)
{
	int weapon_r_frames = MEDIC_HB_REGEN_FRAMES, health_r_frames = MEDIC_REGEN_FRAMES;

	// morph mastery improves ammo/health regeneration
	if (ent->myskills.abilities[MORPH_MASTERY].current_level > 0)
	{
		weapon_r_frames *= 0.5;
		health_r_frames *= 0.5;
	}

	p_medic_hb_regen(ent, weapon_r_frames, MEDIC_HB_REGEN_DELAY);
	//MorphRegenerate(ent, MEDIC_REGEN_DELAY, MEDIC_REGEN_FRAMES);
	M_Regenerate(ent, health_r_frames, MEDIC_REGEN_DELAY, 1.0, true, false, false, &ent->monsterinfo.regen_delay1);
}

void p_medic_jump (edict_t *ent)
{
	// run jump animation forward until last frame, then hold it
	if (ent->s.frame != MEDIC_FRAMES_JUMP_END)
		G_RunFrames(ent, MEDIC_FRAMES_JUMP_START, MEDIC_FRAMES_JUMP_END, false);
}

void p_medic_firehb (edict_t *ent)
{
	int		damage = MEDIC_HB_INITIAL_DMG+MEDIC_HB_ADDON_DMG*ent->myskills.abilities[MEDIC].current_level;
	int		speed = MEDIC_HB_INITIAL_SPEED+MEDIC_HB_ADDON_SPEED*ent->myskills.abilities[MEDIC].current_level;
	vec3_t	forward, right, start;

	if (!ent->myskills.abilities[MEDIC].ammo)
		return;
	ent->myskills.abilities[MEDIC].ammo--;

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	G_ProjectSource(ent->s.origin, monster_flash_offset[MZ2_MEDIC_BLASTER_1], forward, right, start);
	fire_blaster(ent, start, forward, damage, speed, EF_HYPERBLASTER, BLASTER_PROJ_BOLT, MOD_HYPERBLASTER, 2.0, false);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_MEDIC_BLASTER_1);
	gi.multicast (start, MULTICAST_PVS);
}

void p_medic_firebolt (edict_t *ent)
{
	int		min_dmg;
	int		damage = MEDIC_BOLT_INITIAL_DMG+MEDIC_BOLT_ADDON_DMG*ent->myskills.abilities[MEDIC].current_level;
	int		speed = MEDIC_BOLT_INITIAL_SPEED+MEDIC_BOLT_ADDON_SPEED*ent->myskills.abilities[MEDIC].current_level;
	vec3_t	forward, right, start;

	// is this a firing frame?
	if (ent->s.frame != 188 && ent->s.frame != 185)
		return;

	// check for adequate ammo
	if (ent->myskills.abilities[MEDIC].ammo < MEDIC_BOLT_AMMO)
		return;
	ent->myskills.abilities[MEDIC].ammo -= MEDIC_BOLT_AMMO;

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	G_ProjectSource(ent->s.origin, monster_flash_offset[MZ2_MEDIC_BLASTER_1], forward, right, start);

	min_dmg = floattoint(0.111*damage);
	damage = GetRandom(min_dmg, damage);

	fire_blaster(ent, start, forward, damage, speed, EF_BLASTER, BLASTER_PROJ_BLAST, MOD_BLASTER, 2.0, false);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_MEDIC_BLASTER_1);
	gi.multicast (start, MULTICAST_PVS);

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/photon.wav"), 1, ATTN_NORM, 0);
}

void p_medic_attack (edict_t *ent)
{
	ent->client->idle_frames = 0;

	if (ent->client->weapon_mode == 1)
	{
		// cable/healing mode
		if (ent->s.frame != MEDIC_FRAMES_CABLE_END)
			G_RunFrames(ent, MEDIC_FRAMES_CABLE_START, MEDIC_FRAMES_CABLE_END, false);
		else
			ent->s.frame = 218; // loop from this frame forward
		p_medic_heal(ent);
	}
	else if (ent->client->weapon_mode == 2)
	{
		// blaster bolt mode
		if (ent->s.frame != MEDIC_FRAMES_BOLT_END)
			G_RunFrames(ent, MEDIC_FRAMES_BOLT_START, MEDIC_FRAMES_BOLT_END, false);
		else
			ent->s.frame = 185; // loop from this frame forward
		p_medic_firebolt(ent);
	}
	else
	{
		// hyperblaster mode
		if (ent->s.frame != MEDIC_FRAMES_HB_END)
			G_RunFrames(ent, MEDIC_FRAMES_HB_START, MEDIC_FRAMES_HB_END, false);
		else
			ent->s.frame = 195; // loop from this frame forward
		p_medic_firehb(ent);
	}
}


void RunMedicFrames (edict_t *ent, usercmd_t *ucmd)
{
	if ((ent->mtype != MORPH_MEDIC) || (ent->deadflag == DEAD_DEAD))
		return;

	ent->s.modelindex2 = 0; // no weapon model

	if (!ent->myskills.administrator)
		ent->s.skinnum = 0;
	else
		ent->s.skinnum = 2;

	if (level.framenum >= ent->count)
	{
		p_medic_regen(ent);
		
		// play running animation if we are moving forward or strafing
		if ((ucmd->forwardmove > 0) || ucmd->sidemove)
			G_RunFrames(ent, MEDIC_FRAMES_RUN_START, MEDIC_FRAMES_RUN_END, false);
		// play animation in reverse if we are going backwards
		else if (ucmd->forwardmove < 0)
			G_RunFrames(ent, MEDIC_FRAMES_RUN_START, MEDIC_FRAMES_RUN_END, true);
		// attack
		else if ((ent->client->buttons & BUTTON_ATTACK) 
			&& (level.time > ent->monsterinfo.attack_finished))
			p_medic_attack(ent);
		// play jump animation if we are off ground and not submerged in water
		else if (!ent->groundentity && (ent->waterlevel < 2))
			p_medic_jump(ent);
		else
			G_RunFrames(ent, MEDIC_FRAMES_IDLE_START, MEDIC_FRAMES_IDLE_END, false); // run idle frames

		ent->count = level.framenum + 1;
	}
}

void Cmd_PlayerToMedic_f (edict_t *ent)
{
	vec3_t	boxmin, boxmax;
	trace_t	tr;
	int cost = MEDIC_INIT_COST;
	//Talent: More Ammo
	int talentLevel = getTalentLevel(ent, TALENT_MORE_AMMO);

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_PlayerToMedic_f()\n", ent->client->pers.netname);

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

	if (!V_CanUseAbilities(ent, MEDIC, cost, true))
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
	
	V_ModifyMorphedHealth(ent, MORPH_MEDIC, true);

	VectorCopy(boxmin, ent->mins);
	VectorCopy(boxmax, ent->maxs);

	ent->monsterinfo.attack_finished = level.time + 0.5;// can't attack immediately

	ent->client->pers.inventory[power_cube_index] -= cost;
	ent->client->ability_delay = level.time + MEDIC_DELAY;

	ent->mtype = MORPH_MEDIC;
	ent->s.modelindex = gi.modelindex ("models/monsters/medic/tris.md2");
	ent->s.modelindex2 = 0;

	if (!ent->myskills.administrator)
		ent->s.skinnum = 0;
	else
		ent->s.skinnum = 2; // commander

	// set maximum hyperblaster ammo
	ent->myskills.abilities[MEDIC].max_ammo = MEDIC_HB_INITIAL_AMMO+MEDIC_HB_ADDON_AMMO
		*ent->myskills.abilities[MEDIC].current_level;

	// Talent: More Ammo
	// increases ammo 10% per talent level
	if(talentLevel > 0) ent->myskills.abilities[MEDIC].max_ammo *= 1.0 + 0.1*talentLevel;

	// give them some starting ammo
	ent->myskills.abilities[MEDIC].ammo = MEDIC_HB_START_AMMO;

	ent->client->refire_frames = 0; // reset charged weapon
	ent->client->weapon_mode = 0; // reset weapon mode
	ent->client->pers.weapon = NULL;
	ent->client->ps.gunindex = 0;

	lasersight_off(ent);

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/morph.wav") , 1, ATTN_NORM, 0);
}