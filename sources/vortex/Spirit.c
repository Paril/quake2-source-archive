#include "g_local.h"

//************************************************************************************************
//			Spirit Shoot
//************************************************************************************************

void Spirit_Shoot(edict_t *self, edict_t *target, int damage, float next_shot)
{
	vec3_t forward, start, dist;

	//Look at the target
	VectorSubtract(target->s.origin, self->s.origin, dist);
	vectoangles(dist, self->s.angles);

	//Aim at the target
	MonsterAim(self, -1, 1000, false, 0, forward, start);

	//Fire!
	monster_fire_blaster(self, start, forward, damage, 1000, EF_BLASTER, BLASTER_PROJ_BOLT, 2.0, false, 0);

	//Set up for the next shot
	self->delay = level.time + next_shot;
}

//************************************************************************************************
//			Spirit Attack Something (find a target)
//************************************************************************************************

void Spirit_AttackSomething(edict_t *self)
{
	edict_t *target = NULL;
	int abilitylevel = self->activator->myskills.abilities[YANG].current_level;
	int damage = YANG_DAMAGE_BASE	+ (abilitylevel * YANG_DAMAGE_MULT);
	float refire = YANG_ATTACK_DELAY_BASE / (1 + (abilitylevel * YANG_ATTACK_DELAY_MULT));
	int talentLevel;

	//Randomize damage
	damage = GetRandom((int)(damage * 0.66), damage);

	//Talent: Balance Spirit
	//Reduce effectiveness if this is a balance spirit
	talentLevel = getTalentLevel(self->activator, TALENT_BALANCESPIRIT);
	if((self->mtype == M_BALANCESPIRIT) && (talentLevel > 0))
		damage *= 0.75 + 0.07 * talentLevel;	//75% + 7% per upgrade point

	//Shoot at current target
	if (self->enemy && G_EntIsAlive(self->enemy) && visible(self, self->enemy))
	{
		Spirit_Shoot(self, self->enemy, damage, refire);
		return;
	}

	//Find a new target
    while ((target = findclosestradius(target, self->s.origin, SPIRIT_SIGHT_RADIUS)) != NULL)
	{
		//if (target != self->activator && G_EntIsAlive(target) && visible(self, target) && !OnSameTeam(self, target))
		if (G_ValidTarget(self, target, true))
		{
			self->enemy = target;
			Spirit_Shoot(self, self->enemy, damage, refire);
			return;
		}
	}

	//No targets found, so check again soon
	self->delay = level.time + refire;
}

//************************************************************************************************
//			Spirit Attack Corpse	(targets a valid corpse and destroys it, giving the player health)
//************************************************************************************************

void Spirit_AttackCorpse(edict_t *self)
{
	edict_t *target = NULL;
	int abilitylevel = self->activator->myskills.abilities[YIN].current_level;
	int heal = YIN_HEAL_BASE	+ (abilitylevel * YIN_HEAL_MULT);
	float refire = YIN_ATTACK_DELAY_BASE / (1 + (abilitylevel * YIN_ATTACK_DELAY_MULT));
	edict_t *caster = self->activator;
	float ammo;

	if (refire < YANG_ATTACK_DELAY_MIN)
		refire = YANG_ATTACK_DELAY_MIN;

	//Don't crash
	if (!caster) return;

	// get ammo level for respawn weapon
	ammo = AmmoLevel(caster, G_GetAmmoIndexByWeaponIndex(G_GetRespawnWeaponIndex(caster)));
	//gi.dprintf("ammo: %.1f\n",ammo);

	//Make sure the caster needs h/a
	if ((caster->health < MAX_HEALTH(caster)) || (ammo < 1.0) //4.2 added ammo check
		|| (caster->client->pers.inventory[body_armor_index] < MAX_ARMOR(caster)))
	{
		int max_hp = MAX_HEALTH(caster);
		int max_ar = MAX_ARMOR(caster);
		int talentLevel;

		//Randomize healing amount
		heal = GetRandom((int)(heal * 0.66), heal);

		//Talent: Balance Spirit
		//Reduce effectiveness if this is a balance spirit
		talentLevel = getTalentLevel(self->activator, TALENT_BALANCESPIRIT);
		if((self->mtype == M_BALANCESPIRIT) && (talentLevel > 0))
			heal *= 0.75 + 0.07 * talentLevel;	//75% + 7% per upgrade point

		//Find a target corpse
		while ((target = findclosestradius(target, self->s.origin, SPIRIT_SIGHT_RADIUS)) != NULL)
		{
			if (!G_EntExists(target))
				continue;
			if (target->health > 0)
				continue;
			if (target->deadflag != DEAD_DEAD && target->deadflag != DEAD_DYING)
				continue;
			if (!target->gib_health)
				continue;
			if (!visible(self, target))
				continue;
			if (target == self || target == self->activator)
				continue;

			//Kill the corpse
			T_Damage(target, target, self, vec3_origin, target->s.origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, 0);
			gi.sound(self, CHAN_ITEM, gi.soundindex("spells/redemption.wav"), 1, ATTN_NORM, 0);

			//Heal the caster (health first)
			if (caster->health < max_hp)
			{
				int amount = max_hp - caster->health;
				if (amount > heal) amount = heal;
				caster->health += amount;
				heal -= amount;
			}

			//Heal armor (use whatever is left)
			if (caster->client->pers.inventory[body_armor_index] < max_ar)
			{
				int amount = max_ar - caster->client->pers.inventory[body_armor_index];
				if (amount > heal) amount = heal;
				caster->client->pers.inventory[body_armor_index] += amount;
			}

			//Give them some ammo
			if (abilitylevel > 0)
			{
				int ammotype;
				int talentLevel;
				float ammo_mult = YIN_AMMO_BASE + (YIN_AMMO_MULT * (float)abilitylevel);

				switch(caster->myskills.respawn_weapon)
				{
				case 2:		//sg
				case 3:		//ssg
				case 12:	//20mm
					ammotype = AMMO_SHELLS;
					break;
				case 4:		//mg
				case 5:		//cg
					ammotype = AMMO_BULLETS;
					break;
				case 6:		//gl
				case 11:	//hg
					ammotype = AMMO_GRENADES;
					break;
				case 7:		//rl
					ammotype = AMMO_ROCKETS;
					break;
				case 9:		//rg
					ammotype = AMMO_SLUGS;
					break;
				case 1:		//sword
				case 8:		//hb
				case 10:	//bfg10k
				default:
					ammotype = AMMO_CELLS;
					break;
				}

				//Talent: Balance Spirit
				//Reduce effectiveness if this is a balance spirit
				talentLevel = getTalentLevel(self->activator, TALENT_BALANCESPIRIT);
				if((self->mtype == M_BALANCESPIRIT) && (talentLevel > 0))
					ammo_mult *= 0.75 + 0.07 * talentLevel;	//75% + 7% per upgrade point

				V_GiveAmmoClip(caster, ammo_mult, ammotype);
			}
			//Set up for the next shot
			self->delay = level.time + refire;

			//Don't show the bogus damage with the ID skill
			caster->client->ps.stats[STAT_ID_DAMAGE] = 0;
			
			//Done!
			return;
		}
	}

	//No target, so check again soon
	self->delay = level.time + refire;
}

//************************************************************************************************
//			Spirit Think
//************************************************************************************************

void Spirit_think(edict_t *self)
{
	trace_t tr;

	// Die if caster is not alive, or is not a valid ent
	if (!self->activator || !self->activator->client || !G_EntIsAlive(self->activator) || self->activator->flags & FL_CHATPROTECT)
	{
		if (self->activator) self->activator->spirit = NULL;
		G_FreeEdict(self);
		return;
	}
	else	//Stick with the caster
	{
		edict_t *caster = self->activator;
		vec3_t forward;	//caster's aiming angle
		vec3_t result;	//resultant target for the spirit's origin
		vec3_t delta;	//how much is the spirit about to move
		qboolean idling = false; //is the spirit barely moving?

		//Get the client's aiming angle
		AngleVectors(caster->s.angles, forward, NULL, NULL);

		//Flip the aiming angle direction and multiply it by the spirit distance
		//This will put a target origin directly behind the caster's aiming angle
		VectorMA(caster->s.origin, -1.0 * SPIRIT_DISTANCE, forward, result);
        
		//The result should be above the caster, not below
		result[2] = caster->s.origin[2] + (SPIRIT_DISTANCE * 2.0);
//GHz START
		tr = gi.trace(caster->s.origin, NULL, NULL, result, caster, MASK_SHOT);
		if (tr.fraction < 1)
			VectorCopy(tr.endpos, result);
//GHz END
		//Find out if the spirit is idling or not
		VectorSubtract(self->s.origin, result, delta);
		if (VectorLength(delta) < 3)	idling = true;

		//Move the spirit
		VectorCopy(result, self->s.origin);

		//Write some graphical effect
		if (level.framenum % GetRandom(1, 5) == 0 && idling)
		{
			vec3_t dir = {0, 0, -1.0 * SPIRIT_DISTANCE};
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_SPARKS);
			gi.WritePosition(self->s.origin);
			gi.WriteDir(dir);
			gi.multicast(self->s.origin, MULTICAST_PHS);
		}
		gi.linkentity(self);

		//Attack something
		if (self->delay < level.time)
		{
			switch(self->mtype)
			{
			case M_YANGSPIRIT:	Spirit_AttackSomething(self);	break;
			case M_YINSPIRIT:	Spirit_AttackCorpse(self);		break;
			case M_BALANCESPIRIT:
				if(self->activator->myskills.abilities[YANG].current_level > 0)
					Spirit_AttackSomething(self);
				if(self->activator->myskills.abilities[YIN].current_level > 0)
					Spirit_AttackCorpse(self);
				break;
			}
		}
	}

	//Next think
	self->nextthink = level.time + FRAMETIME;
}

//************************************************************************************************
//			Spirit Spawn Command
//************************************************************************************************

void cmd_Spirit(edict_t *ent, int type)
{
    //Remove an existing spirit entity
	if (ent->spirit)
	{
		G_FreeEdict(ent->spirit);
		ent->spirit = NULL;
		safe_cprintf(ent, PRINT_HIGH, "Spirit removed.\n");
		return;
	}
	else
	{
		edict_t *spirit;
		switch(type)
		{
		case M_YINSPIRIT:
			if (!V_CanUseAbilities(ent, YIN, SPIRIT_COST, true))
				return;
			break;
		case M_YANGSPIRIT:
			if (!V_CanUseAbilities(ent, YANG, SPIRIT_COST, true))
				return;
			break;
		case M_BALANCESPIRIT:
			if(getTalentLevel(ent, TALENT_BALANCESPIRIT) < 1)
			{
				safe_cprintf(ent, PRINT_HIGH, "You do not have this talent.\n");
				return;
			}

			if((!V_CanUseAbilities(ent, YIN, SPIRIT_COST, false) 
				&& !V_CanUseAbilities(ent, YANG, SPIRIT_COST, false)))
			{
				safe_cprintf(ent, PRINT_HIGH, "You must upgrade either yin or yang before you can use this talent.\n");
				return;				
			}				
			break;
		}

        //Create a new spirit entity
		spirit = G_Spawn();
		VectorClear(spirit->velocity);
		VectorClear(spirit->mins);
		VectorClear(spirit->maxs);
		spirit->movetype = MOVETYPE_NOCLIP;
		spirit->clipmask = MASK_SHOT;
		spirit->solid = SOLID_NOT;//SOLID_BBOX;
		spirit->s.modelindex = gi.modelindex("models/objects/flball/tris.md2");

		//Set render fx
		spirit->s.effects |= EF_COLOR_SHELL | EF_PLASMA | EF_SPHERETRANS | EF_GREENGIB;

		//Set spirit type, target, and caster
		spirit->activator = ent;
		ent->spirit = spirit;
		spirit->owner = ent;

		//First think
		spirit->nextthink = level.time + FRAMETIME;
		spirit->think = Spirit_think;
		spirit->delay = level.time + 1.0;

		//Set spirit type
		spirit->mtype = type;

		spirit->monsterinfo.level = ent->myskills.abilities[YANG].current_level;
		switch (type)
		{
		case M_YINSPIRIT:
			spirit->classname = "yin spirit";
			spirit->s.renderfx |= RF_SHELL_BLUE | RF_SHELL_GREEN | RF_SHELL_RED;
			safe_cprintf(ent, PRINT_HIGH, "Yin spirit summoned.\n");
			break;
		case M_YANGSPIRIT:
			spirit->classname = "yang spirit";
            spirit->s.renderfx |= RF_SHELL_YELLOW;
			safe_cprintf(ent, PRINT_HIGH, "Yang spirit summoned.\n");
			break;
		case M_BALANCESPIRIT:
			spirit->classname = "balance spirit";
			spirit->s.renderfx |= RF_SHELL_CYAN;
			safe_cprintf(ent, PRINT_HIGH, "Balance spirit summoned.\n");
			break;
		}
	}
}