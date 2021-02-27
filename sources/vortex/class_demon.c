#include "g_local.h"

#define CACODEMON_FRAME_IDLE_START		0
#define CACODEMON_FRAME_IDLE_END		3
#define CACODEMON_FRAME_BLINK_START		4
#define CACODEMON_FRAME_BLINK_END		7
#define CACODEMON_FRAME_ATTACK_START	8	
#define CACODEMON_FRAME_ATTACK_FIRE		11
#define CACODEMON_FRAME_ATTACK_END		14

// additional parameters in morph.h

void bskull_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int	num;
	int	damage = CACODEMON_ADDON_BURN*self->owner->myskills.abilities[CACODEMON].current_level;

	// deal direct damage
	if (G_EntExists(other))
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, 
			plane->normal, self->dmg, 1, DAMAGE_RADIUS, MOD_CACODEMON_FIREBALL);
	// deal radius damage
	T_RadiusDamage(self, self->owner, self->radius_dmg, other, 
		self->dmg_radius, MOD_CACODEMON_FIREBALL);
/*
	if (self->owner->myskills.abilities[MORPH_MASTERY].current_level > 0)
	{
		num = GetRandom(4, 8);
		damage *= 1.5;
	}
	else
*/
		num = GetRandom(3, 6);

	SpawnFlames(self->owner, self->s.origin, num, damage, 100);
	BecomeExplosion1(self);
}

void bskull_think (edict_t *self)
{
	if (!G_EntIsAlive(self->owner) || (self->delay < level.time)) 
	{
		G_FreeEdict(self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
}

void fire_skull (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t *skull;

	skull = G_Spawn();
	VectorCopy(start, skull->s.origin);
	VectorCopy(dir, skull->movedir);
	vectoangles (dir, skull->s.angles);
	VectorScale (dir, speed, skull->velocity);
	skull->movetype = MOVETYPE_TOSS;
	skull->clipmask = MASK_SHOT;
	skull->solid = SOLID_BBOX;
	VectorClear (skull->mins);
	VectorClear (skull->maxs);
	skull->s.modelindex = gi.modelindex ("models/objects/gibs/skull/tris.md2");
	skull->owner = self;
	skull->touch = bskull_touch;
	skull->dmg = damage;
	skull->s.effects = EF_GIB;
	skull->radius_dmg = damage;
	skull->dmg_radius = damage_radius;
	skull->classname = "skull";
	skull->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");
	skull->delay = level.time + 10;
	skull->think = bskull_think;
	skull->nextthink = level.time + FRAMETIME;
//	if (self->client)
//		check_dodge (self, skull->s.origin, dir, speed, damage_radius);
	gi.linkentity (skull);
}

void cacodemon_attack (edict_t *ent)
{
	int		damage, radius;
	vec3_t	forward, right, start, offset;

	// check for sufficient ammo
	if (!ent->myskills.abilities[CACODEMON].ammo)
		return;

	if (level.time > ent->monsterinfo.attack_finished)
	{
		damage = (CACODEMON_INITIAL_DAMAGE + CACODEMON_ADDON_DAMAGE*ent->myskills.abilities[CACODEMON].current_level)*2;
		radius = CACODEMON_INITIAL_RADIUS + CACODEMON_ADDON_RADIUS*ent->myskills.abilities[CACODEMON].current_level;

		ent->s.frame = CACODEMON_FRAME_ATTACK_FIRE;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorScale (forward, -3, ent->client->kick_origin);
		VectorSet(offset, 0, 7,  ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_skull(ent, start, forward, damage, CACODEMON_SKULL_SPEED, radius);
		ent->monsterinfo.attack_finished = level.time + CACODEMON_REFIRE;

		// use ammo
		ent->myskills.abilities[CACODEMON].ammo--;
	}
}

void PlagueCloudSpawn (edict_t *ent);

void RunCacodemonFrames (edict_t *ent, usercmd_t *ucmd)
{
	int		frame;

	// if we aren't a cacodemon or we are dead, we shouldn't be here!
	if ((ent->mtype != MORPH_CACODEMON) || (ent->deadflag == DEAD_DEAD))
		return;

	if (level.framenum >= ent->count)
	{
		MorphRegenerate(ent, CACODEMON_REGEN_DELAY, CACODEMON_REGEN_FRAMES);
	//	ent->client->ability_delay = level.time + CACODEMON_DELAY; // can't use abilities
		
		if (ent->client->buttons & BUTTON_ATTACK)
		{
			ent->client->idle_frames = 0;
			// run attack frames
			G_RunFrames(ent, CACODEMON_FRAME_ATTACK_START, CACODEMON_FRAME_ATTACK_END, false);
			cacodemon_attack(ent);
		}
		else
		{
			frame = ent->s.frame;
			if (((random() <= 0.5) && (frame == CACODEMON_FRAME_IDLE_END))
				|| ((frame >= CACODEMON_FRAME_BLINK_START) && (frame < CACODEMON_FRAME_BLINK_END)))
				// run blink frames
				G_RunFrames(ent, CACODEMON_FRAME_BLINK_START, CACODEMON_FRAME_BLINK_END, false);
			else
				// run idle frames
				G_RunFrames(ent, CACODEMON_FRAME_IDLE_START, CACODEMON_FRAME_IDLE_END, false);
		}
	
		// add thrust
		if (ucmd->upmove > 0)
		{
			if (ent->groundentity)
				ent->velocity[2] = 150;
			else if (ent->velocity[2] < 0)
				ent->velocity[2] += 200;
			else
				ent->velocity[2] += 100;
			
		}

		ent->count = level.framenum + 1;
	}
}

void Cmd_PlayerToCacodemon_f (edict_t *ent)
{
	vec3_t	mins, maxs;
	trace_t	tr;
	int caco_cubecost = CACODEMON_INIT_COST;
	//Talent: More Ammo
	int talentLevel = getTalentLevel(ent, TALENT_MORE_AMMO);

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_PlayerToCacodemon_f()\n", ent->client->pers.netname);

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

	if (HasFlag(ent) && !hw->value)
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't morph while carrying flag!\n");
		return;
	}

	//Talent: Morphing
	if(getTalentSlot(ent, TALENT_MORPHING) != -1)
		caco_cubecost *= 1.0 - 0.25 * getTalentLevel(ent, TALENT_MORPHING);

	//if (!G_CanUseAbilities(ent, ent->myskills.abilities[CACODEMON].current_level, caco_cubecost))
	//	return;
	if (!V_CanUseAbilities(ent, CACODEMON, caco_cubecost, true))
		return;

	// don't get stuck
	VectorSet(mins, -24, -24, -24);
	VectorSet(maxs, 24, 24, 24);
	tr = gi.trace(ent->s.origin, mins, maxs, ent->s.origin, ent, MASK_SHOT);
	if (tr.fraction < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough room to morph.\n");
		return;
	}

	V_ModifyMorphedHealth(ent, MORPH_CACODEMON, true);

	VectorCopy(mins, ent->mins);
	VectorCopy(maxs, ent->maxs);

	ent->client->pers.inventory[power_cube_index] -= caco_cubecost;
	ent->client->ability_delay = level.time + CACODEMON_DELAY;

	ent->mtype = MORPH_CACODEMON;
	ent->s.modelindex = gi.modelindex ("models/monsters/idg2/head/tris.md2");
	ent->s.modelindex2 = 0;
	ent->s.skinnum = 0;

	ent->monsterinfo.attack_finished = level.time + 0.5;// can't attack immediately

	// set maximum skull ammo
	ent->myskills.abilities[CACODEMON].max_ammo = CACODEMON_SKULL_INITIAL_AMMO+CACODEMON_SKULL_ADDON_AMMO
		*ent->myskills.abilities[CACODEMON].current_level;

	// Talent: More Ammo
	// increases ammo 10% per talent level
	if(talentLevel > 0) ent->myskills.abilities[CACODEMON].max_ammo *= 1.0 + 0.1*talentLevel;

	// give them some starting ammo
	ent->myskills.abilities[CACODEMON].ammo = CACODEMON_SKULL_START_AMMO;

	ent->client->refire_frames = 0; // reset charged weapon
	ent->client->weapon_mode = 0; // reset weapon mode
	ent->client->pers.weapon = NULL;
	ent->client->ps.gunindex = 0;
	lasersight_off(ent);

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/morph.wav") , 1, ATTN_NORM, 0);
}

#define CORPSEEATER_DELAY			0.5
#define CORPSEEATER_RANGE			64	// maximum distance from corpse
#define CORPSEEATER_INITIAL_HEALTH	0
#define CORPSEEATER_ADDON_HEALTH	10
#define CORPSEEATER_INITIAL_DAMAGE	25
#define CORPSEEATER_ADDON_DAMAGE	2.5
#define CORPSEEATER_ADDON_MAXHEALTH	0.05

qboolean curse_add(edict_t *target, edict_t *caster, int type, int curse_level, float duration);

void EatCorpses (edict_t *ent)
{
	int			value, gain;
	vec3_t		forward, right, start, end, offset;
	trace_t		tr;
	qboolean	sound=false;

	if(ent->myskills.abilities[FLESH_EATER].disable)
		return;

	if (!V_CanUseAbilities(ent, FLESH_EATER, 0, false))
		return;

	value = CORPSEEATER_RANGE;
	if (ent->mtype)
		value += ent->maxs[1] - 16;
	
	// get starting position
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// get ending position and trace
	VectorMA(start, value, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if (G_EntExists(tr.ent) && (level.time > ent->corpseeater_time))
	{
		//Get a multiplier of how much higher the player can go over their max health
		float maxhp = 1.0 + (CORPSEEATER_ADDON_MAXHEALTH * ent->myskills.abilities[FLESH_EATER].current_level);
		
		//Talent: Cannibalism
		int talentLevel = getTalentLevel(ent, TALENT_CANNIBALISM);
		if(talentLevel > 0)		maxhp += 0.1 * talentLevel;	//+0.1x per upgrade
		
		//Figure out what their max health is
		maxhp *= ent->max_health;

		if (tr.ent->health < 1)
		{
			// kill the corpse
			T_Damage(tr.ent, tr.ent, world, vec3_origin, tr.ent->s.origin, vec3_origin, 
				10000, 0, DAMAGE_NO_PROTECTION, 0);

			//heal the player
			gain = CORPSEEATER_INITIAL_HEALTH + (CORPSEEATER_ADDON_HEALTH * ent->myskills.abilities[FLESH_EATER].current_level);
			if (ent->health < maxhp)
			{
				ent->megahealth = NULL;
				ent->health += gain;
				if (ent->health > maxhp)
					ent->health = maxhp;
				
				// sync up player-monster's health
				if (PM_PlayerHasMonster(ent))
					ent->owner->health = ent->health;
			}
			sound = true;
		}
		else if (!OnSameTeam(ent, tr.ent))
		{
			float chance;

			//damage the target
			VectorSubtract(tr.ent->s.origin, ent->s.origin, forward);
			value = CORPSEEATER_INITIAL_DAMAGE + (CORPSEEATER_ADDON_DAMAGE*ent->myskills.abilities[FLESH_EATER].current_level);
			gain = 0.5 * T_Damage(tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, value, -value, 0, MOD_CORPSEEATER);
			
			//heal the player
			if (ent->health < maxhp)
			{
				ent->megahealth = NULL;
				ent->health += gain;
				if (ent->health > maxhp)
					ent->health = maxhp;

				// sync up player-monster's health
				if (PM_PlayerHasMonster(ent))
					ent->owner->health = ent->health;
			}
			sound = true;
			
			//Talent: Fatal Wound
			talentLevel = getTalentLevel(ent, TALENT_FATAL_WOUND);
			chance = 0.1 * talentLevel;
			if (talentLevel > 0 && random() <= chance)
			{
				curse_add(tr.ent, ent, BLEEDING, 30, 10.0);
				if (tr.ent->client)
					safe_cprintf(tr.ent, PRINT_HIGH, "You have been fatally wounded!\n");
			}
		}

		if (sound) gi.sound(ent, CHAN_ITEM, gi.soundindex("brain/brnpain2.wav"), 1, ATTN_NORM, 0);
		ent->corpseeater_time = level.time + CORPSEEATER_DELAY;
	}
}

#define PLAGUE_DEFAULT_RADIUS	48
#define PLAGUE_ADDON_RADIUS		8
#define PLAGUE_MAX_RADIUS		128	
#define PLAGUE_DURATION			999
#define PLAGUE_INITIAL_DAMAGE	0
#define PLAGUE_ADDON_DAMAGE		1
#define PLAGUE_DELAY			1.0

void PlagueCloud (edict_t *ent, edict_t *target);

void PlagueCloudSpawn (edict_t *ent)
{
	float	radius;
	edict_t *e=NULL;
	int levelmax;

	if (ent->myskills.abilities[PLAGUE].disable && ent->myskills.abilities[BLOOD_SUCKER].disable)
		return;

	if (!V_CanUseAbilities(ent, PLAGUE, 0, false))
	{
		if (!V_CanUseAbilities(ent, BLOOD_SUCKER, 0, false)) // parasites have passive plague
			return;
	}

	if ((ent->myskills.class_num == CLASS_POLTERGEIST) && !ent->mtype && !PM_PlayerHasMonster(ent))
		return; // can't use this in human form

	if (ent->mtype == M_MYPARASITE) // you're a parasite? pick highest, plague or parasite.
		levelmax = max(ent->myskills.abilities[PLAGUE].current_level, ent->myskills.abilities[BLOOD_SUCKER].current_level);
	else
	{
		if (!ent->myskills.abilities[PLAGUE].disable) // we have the skill, right?
			levelmax = ent->myskills.abilities[PLAGUE].current_level;
	}

	radius = PLAGUE_DEFAULT_RADIUS+PLAGUE_ADDON_RADIUS*levelmax;

	if (radius > PLAGUE_MAX_RADIUS)
		radius = PLAGUE_MAX_RADIUS;

	// find someone nearby to infect
	while ((e = findradius(e, ent->s.origin, radius)) != NULL)
	{
		if (!G_ValidTarget(ent, e, true))
			continue;
	//	if (HasActiveCurse(e, CURSE_PLAGUE))
		if (que_typeexists(e->curses, CURSE_PLAGUE))
			continue;
		// holy water grants temporary immunity to curses
		if (e->holywaterProtection > level.time)
			continue;

		PlagueCloud(ent, e);
	}
}

void plague_think (edict_t *self)
{
	int		dmg;
	float	radius;
	edict_t *e=NULL;
	
	// plague self-terminates if:
	if (!G_EntIsAlive(self->owner) || !G_EntIsAlive(self->enemy)	//someone dies
		|| (self->owner->flags & FL_WORMHOLE)						// owner enters a wormhole
		|| (self->owner->client->tball_delay > level.time)			//owner tballs away
		|| (self->owner->flags & FL_CHATPROTECT)					//3.0 owner is in chatprotect
		|| ((self->owner->myskills.class_num == CLASS_POLTERGEIST) && (!self->owner->mtype) && !PM_PlayerHasMonster(self->owner))  //3.0 poltergeist is in human form
		|| que_findtype(self->enemy->curses, NULL, HEALING) != NULL)	//3.0 player is blessed with healing
	{
		que_removeent(self->enemy->curses, self, true);
		return;
	}

	VectorCopy(self->enemy->s.origin, self->s.origin); // follow enemy

	radius = PLAGUE_DEFAULT_RADIUS+PLAGUE_ADDON_RADIUS*self->owner->myskills.abilities[PLAGUE].current_level;

	if (radius > PLAGUE_MAX_RADIUS)
		radius = PLAGUE_MAX_RADIUS;

	// find someone nearby to infect
	while ((e = findradius(e, self->s.origin, radius)) != NULL)
	{
		if (e == self->enemy)
			continue;
		if (!G_ValidTarget(self, e, true))
			continue;
		// don't allow more than one curse of the same type
		if (que_typeexists(e->curses, CURSE_PLAGUE))
			continue;
		// holy water grants temporary immunity to curses
		if (e->holywaterProtection > level.time)
			continue;
		// spawn another plague cloud on this entity
		PlagueCloud(self->owner, e);
	}

	if (level.time > self->wait)
	{
		int maxlevel;

		if (self->owner->mtype == M_MYPARASITE)
			maxlevel = max(self->owner->myskills.abilities[PLAGUE].current_level, self->owner->myskills.abilities[BLOOD_SUCKER].current_level);
		else
			maxlevel = self->owner->myskills.abilities[PLAGUE].current_level;

		dmg = (float)maxlevel/10 * ((float)self->enemy->max_health/20);
		if (!self->enemy->client && strcmp(self->enemy->classname, "player_tank") != 0)
			dmg *= 2; // non-clients take double damage (helps with pvm)
		if (dmg < 1)
			dmg = 1;
		if (dmg > 100)
			dmg = 100;
		T_Damage(self->enemy, self->enemy, self->owner, vec3_origin, self->enemy->s.origin, vec3_origin, 
			dmg, 0, DAMAGE_NO_ABILITIES, MOD_PLAGUE); // hurt 'em
		self->wait = level.time + PLAGUE_DELAY;
	}
	
	self->nextthink = level.time + FRAMETIME;

}

void PlagueCloud (edict_t *ent, edict_t *target)
{
	edict_t *plague;

	plague = G_Spawn();
	plague->movetype = MOVETYPE_NOCLIP;
	plague->svflags |= SVF_NOCLIENT;
	plague->solid = SOLID_NOT;
	plague->enemy = target;
	plague->owner = ent;
//	plague->dmg = damage;
	plague->nextthink = level.time + FRAMETIME;
	plague->think = plague_think;
	plague->classname = "curse";
	plague->mtype = CURSE_PLAGUE;
	VectorCopy(ent->s.origin, plague->s.origin);

	// abort if the target has too many curses
//	if (!AddCurse(ent, target, plague, CURSE_PLAGUE, PLAGUE_DURATION))
	if (!que_addent(target->curses, plague, PLAGUE_DURATION))
		G_FreeEdict(plague);
}





