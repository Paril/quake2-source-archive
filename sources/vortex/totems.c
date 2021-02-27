#ifndef TOTEMS_C
#define TOTEMS_C

#include "g_local.h"

void RemoveTotem(edict_t *self)
{
	if (!self || !self->inuse || (self->deadflag == DEAD_DEAD))
		return;

	//Remove from the caster's totem list.
	if(self->activator)
	{
		//If totem #1 dies, shift #2 to its location and make #2 NULL.
		//This should make players with 1 totem out always have it in the slot for totem #1.
		if(self->activator->totem1 == self)
			self->activator->totem1 = self->activator->totem2;
		self->activator->totem2 = NULL;
	}

	// prep for removal
	self->think = BecomeTE;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->deadflag = DEAD_DEAD;
	self->nextthink = level.time + FRAMETIME;
	gi.unlinkentity(self);
}

void totem_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	//If a totem is destroyed, the caster should not be able to make another one for a few seconds.
	if(self->activator && self->activator->client)
	{
		//self->activator->client->ability_delay = level.time + 2.5;
		safe_cprintf(self->activator, PRINT_HIGH, "*** Your totem was destroyed ***\n");
	}

	RemoveTotem(self);
}

edict_t *NextNearestTotem(edict_t *ent, int totemType, edict_t *lastTotem, qboolean allied)
{
	edict_t *ed, *owner, *closestEnt = NULL;
	float closestDistance = -1.0;
	float lastTotemDistance = 0.0;

	if (lastTotem)		lastTotemDistance = V_EntDistance(ent, lastTotem);

	for (ed = g_edicts ; ed < &g_edicts[globals.num_edicts]; ed++)
	{
		float thisDistance;

		//skip all ents that are not totems, and skip the last "closest" totem.
		if(ed->mtype != totemType || ed == lastTotem)
			continue;
		
		// air totems can be configured to protect only their owner
		if (PM_PlayerHasMonster(ed->activator))
			owner = ed->activator->owner;
		else
			owner = ed->activator;
		if (ed->mtype == TOTEM_AIR && ed->style && ent != owner)
			continue;

		//skip any totems that are "not on my team" or are "not my enemy"
		if(allied && !OnSameTeam(ent, ed))
			continue;
		else if(!allied && OnSameTeam(ent, ed))
			continue;

		thisDistance = V_EntDistance(ent, ed);

        //Totem must be in range and must be in "line of sight".
		//Using visible1() so forcewall, etc.. can block it.
		if(thisDistance > TOTEM_MAX_RANGE || !visible1(ed, ent))
			continue;

		//The closest totem found must be farther away than the last one found.
		if(lastTotem != NULL && thisDistance < lastTotemDistance)
			continue;

		//This is a valid totem, but is it the closest one?		
		if(thisDistance < closestDistance || closestDistance == -1.0)
		{
			closestDistance = thisDistance;
			closestEnt = ed;
		}
	}

	//This should be the next closest totem to the target, and should be null if there isn't one.
	return closestEnt;
}

int GetTotemLevel(edict_t *ent, int totemType, qboolean allied)
{
	edict_t *totem = NextNearestTotem(ent, totemType, NULL, allied);
	if(totem != NULL)		return totem->monsterinfo.level;
	
	return 0;
}

//************************************************************************************************
//			Totem Think Functions
//************************************************************************************************
void fire_fireball (edict_t *self, vec3_t start, vec3_t aimdir, int damage, float damage_radius, int speed, int flames, int flame_damage);
void FireTotem_think(edict_t *self, edict_t *caster)
{
	int talentLevel;
	float chance;
	edict_t *target = NULL;

	//Totem should not work underwater (gee I wonder why).
	if(!self->waterlevel)
	{
		//Talent: Volcanic
		talentLevel = getTalentLevel(caster, TALENT_VOLCANIC);
		chance = 0.02 * talentLevel;

		//Find players in radius and attack them.
		while ((target = findclosestradius(target, self->s.origin, TOTEM_MAX_RANGE)) != NULL)
		{
			if (G_ValidTarget(self, target, true) && (self->s.origin[2]+64>target->s.origin[2]))
			{
				vec3_t forward, end;
				int damage = FIRETOTEM_DAMAGE_BASE + self->monsterinfo.level * FIRETOTEM_DAMAGE_MULT;
				int count = 10 + self->monsterinfo.level;
				int speed = 600;
				float	val, dist;
				qboolean fireball=false;

				// copy target location
				G_EntMidPoint(target, end);

				// calculate distance to target
				dist = distance(end, self->s.origin);

				// move our target point based on projectile and enemy velocity
				VectorMA(end, (float)dist/speed, target->velocity, end);

				//Talent: Volcanic - chance to shoot a fireball
				if (talentLevel > 0 && chance > random())
					fireball = true;

				// aim up
				if (fireball)
					val = ((dist/2048)*(dist/2048)*2048) + (end[2]-self->s.origin[2]);//4.4
				else
					val = ((dist/512)*(dist/512)*512) + (end[2]-self->s.origin[2]);
				if (val < 0)
					val = 0;
				end[2] += val;

				// calculate direction vector to target
				VectorSubtract(end, self->s.origin, forward);
				VectorNormalize(forward);
				
				// don't fire in a perfectly straight line
				forward[1] += 0.05*crandom(); 

				// spawn flames
				if (fireball)
					fire_fireball(self, self->s.origin, forward, 200, 125, 600, 5, 20);//4.4
				else
					ThrowFlame(self, self->s.origin, forward, distance(self->s.origin, target->s.origin), speed, damage, GetRandom(2, 4));
				
				self->lastsound = level.framenum;

				// refire delay
				self->delay = level.time + FRAMETIME;
			}
		}
	}

}

void WaterTotem_think(edict_t *self, edict_t *caster)
{
	edict_t *target = NULL;

	//Find players in radius and attack them.
	while ((target = findclosestradius(target, self->s.origin, TOTEM_MAX_RANGE)) != NULL)
	{
		// (apple)
		// Since ice talent and watertotem work concurrently now, 
		// checking for chill_duration will throttle ice talent's refire.
		if (G_ValidTarget(self, target, true))
		{
			vec3_t normal;
			int talentLevel;
			float duration = WATERTOTEM_DURATION_BASE + self->monsterinfo.level * WATERTOTEM_DURATION_MULT;

			//Get a directional vector from the totem to the target.
			VectorSubtract(self->s.origin, target->s.origin, normal);

			//Talent: Ice. Damages players.
			talentLevel = getTalentLevel(caster, TALENT_ICE);
			if(talentLevel > 0)
			{
				int damage = GetRandom(10, 20) * talentLevel;
				vec3_t normal;
				
				//Damage the target
				VectorSubtract(target->s.origin, self->s.origin, normal);				
				T_Damage(target, self, self, vec3_origin, self->s.origin, 
					normal, damage, 0, DAMAGE_NO_KNOCKBACK, MOD_WATERTOTEM);
			}
			
			//Chill the target.
			target->chill_level = self->monsterinfo.level;
			target->chill_time = level.time + duration;
			//gi.dprintf("chilled %s for %.1f seconds at level %d\n", target->classname, duration, self->monsterinfo.level);
			
		}
	}
	//Next think.
	self->delay = level.time + WATERTOTEM_REFIRE_BASE + WATERTOTEM_REFIRE_MULT * self->monsterinfo.level;
}

void NatureTotem_think(edict_t *self, edict_t *caster)
{
	edict_t *target = NULL;
	qboolean isSomeoneHealed = false;
	float cdmult = 1.0;

	//Find players in radius and attack them.
	while ((target = findradius(target, self->s.origin, TOTEM_MAX_RANGE)) != NULL)
	{
		if (G_ValidAlliedTarget(self, target, true))
		{
			int maxHP;// = MAX_HEALTH(target);
			int maxArmor;// = MAX_ARMOR(target);
			int maxcubes;// = target->client->pers.max_powercubes;
			int *armor;// = &target->client->pers.inventory[body_armor_index];
			int *cubes;// = &target->client->pers.inventory[ITEM_INDEX(Fdi_POWERCUBE)];
			int	regen_frames;//4.2 regen for non-clients
			
			if (!target->client)
			{
				regen_frames = 1000 / self->monsterinfo.level; // full-regeneration in 15 seconds at level 10
				M_Regenerate(target, regen_frames, 50, 1.0, true, true, false, &target->monsterinfo.regen_delay2);
				continue;
			}
			
			maxHP = MAX_HEALTH(target);
			maxArmor = MAX_ARMOR(target);
			maxcubes = target->client->pers.max_powercubes;
			armor = &target->client->pers.inventory[body_armor_index];
			cubes = &target->client->pers.inventory[ITEM_INDEX(Fdi_POWERCUBE)];

			//Heal their health.
			if(target->health < maxHP)
			{
				target->health += NATURETOTEM_HEALTH_BASE + self->monsterinfo.level * NATURETOTEM_HEALTH_MULT;
				if(target->health > maxHP)
					target->health = maxHP;
				isSomeoneHealed = true;
			}
			//Heal their armor.
			if(*armor < maxArmor)
			{
				*armor += NATURETOTEM_ARMOR_BASE + self->monsterinfo.level * NATURETOTEM_ARMOR_MULT;
				if(*armor > maxArmor)
					*armor = maxArmor;
				isSomeoneHealed = true;
			}

			//Talent: Peace.
			if(*cubes < maxcubes)
			{
				//Give them 5 cubes per talent point.
				*cubes += getTalentLevel(caster, TALENT_PEACE) * 10;
				if(*cubes > maxcubes)
					*cubes = maxcubes;
				isSomeoneHealed = true;
			}

			// We remove curses. -az
			CurseRemove(target, 0);
		}
	}

	//Play a sound or something.
	if(isSomeoneHealed)
	{
		gi.sound(self, CHAN_ITEM, gi.soundindex("items/n_health.wav"), 1, ATTN_NORM, 0);
	}

	cdmult = level.time + NATURETOTEM_REFIRE_BASE + NATURETOTEM_REFIRE_MULT * self->monsterinfo.level;

	if (cdmult <= level.time)
		cdmult = level.time + 0.1;

	//Next think.
	self->delay = cdmult;
}

void totem_effects (edict_t *self)
{
	// totems should have team colors in CTF and Domination modes
	if (ctf->value || domination->value)
	{
		self->s.effects = EF_COLOR_SHELL;
		if (self->activator->teamnum == 1)	
			self->s.renderfx = RF_SHELL_RED;
		else if (self->activator->teamnum == 2)	
			self->s.renderfx = RF_SHELL_BLUE;
	}
}

void totem_general_think(edict_t *self)
{
	edict_t *caster = self->activator;

	// Die if caster is not alive, or is not a valid ent
	if (!caster || !caster->client || !G_EntIsAlive(caster) || caster->flags & FL_CHATPROTECT)
	{
		RemoveTotem(self);
		return;
	}

	//Some players can have two totems out (with talent). Take cubes away from them every 5 seconds.
	if(level.framenum % 50 == 0 && caster->client && caster->totem2 == self)
	{
		int *cubes = &caster->client->pers.inventory[ITEM_INDEX(Fdi_POWERCUBE)];

		//Talent: Totemic Focus.
		int cost = 10;//30 - getTalentLevel(caster, TALENT_TOTEM) * 5;
        if(*cubes < cost)
		{
			*cubes = 0;
			RemoveTotem(self);
			return;
		}
		*cubes -= cost;
	}

	if(self->delay < level.time)
	{
		switch(self->mtype)
		{
		case TOTEM_FIRE:			FireTotem_think(self, caster);			break;
		case TOTEM_WATER:			WaterTotem_think(self, caster);			break;
		case TOTEM_NATURE:			NatureTotem_think(self, caster);		break;
		default:					break;
		}
	}
	
	// totem mastery allows regeneration
	if (level.time > self->lasthurt + 1.0 && !caster->myskills.abilities[TOTEM_MASTERY].disable 
		&& caster->myskills.abilities[TOTEM_MASTERY].current_level > 0)
		M_Regenerate(self, TOTEM_REGEN_FRAMES, TOTEM_REGEN_DELAY, 1.0, true, false, false, &self->monsterinfo.regen_delay1);

	//Rotate a little.
	self->s.angles[YAW] += 5;
	if(self->s.angles[YAW] == 360)
		self->s.angles[YAW] = 0;
//GHz 4.32
	// if position has been updated, check for ground entity
	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	// don't slide
	if (self->groundentity)
		VectorClear(self->velocity);
	
	M_CatagorizePosition (self);
	M_WorldEffects (self);
//GHz
	totem_effects(self);

	self->nextthink = level.time + FRAMETIME;
}

//Using a modified version of the item drop code to drop a totem.
edict_t *DropTotem(edict_t *ent)
{
	edict_t	*dropped;
	vec3_t	forward, right;
	vec3_t	offset;

	dropped = G_Spawn();

	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	//dropped->owner = ent;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
			dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		VectorCopy(ent->s.angles, forward);
		forward[YAW] = GetRandom(0, 360);
		AngleVectors (forward, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}
	dropped->s.angles[YAW] = GetRandom(0, 360);
	VectorScale (forward, GetRandom(50, 150), dropped->velocity);
	dropped->velocity[2] = 300;
	gi.linkentity (dropped);
	return dropped;
}

void totem_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	V_Touch(self, other, plane, surf);
}

void SpawnTotem(edict_t *ent, int abilityID)
{
	int			talentLevel, cost=TOTEM_COST;
	edict_t		*totem;
	int			totemType;
	vec3_t		start;//GHz 4.32

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5) 
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	if(!V_CanUseAbilities(ent, abilityID, cost, true))
		return;
	
	if (ctf->value && abilityID == FIRE_TOTEM 
		&& (CTF_DistanceFromBase(ent, NULL, CTF_GetEnemyTeam(ent->teamnum)) < CTF_BASE_DEFEND_RANGE))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build in enemy base!\n");
		return;
	}

	//Determine the totem type.
	switch(abilityID)
	{
	case FIRE_TOTEM:		totemType = TOTEM_FIRE;		break;
	case WATER_TOTEM:		totemType = TOTEM_WATER;	break;
	case AIR_TOTEM:			totemType = TOTEM_AIR;		break;
	case EARTH_TOTEM:		totemType = TOTEM_EARTH;	break;
	case DARK_TOTEM:		totemType = TOTEM_DARKNESS;	break;
	case NATURE_TOTEM:		totemType = TOTEM_NATURE;	break;
	default: return;
	}

	//Can't create too many totems.
	if(ent->totem1)
	{
		//Can't have more than one totem without the talent.
		/*if(getTalentLevel(ent, TALENT_TOTEM) < 1)
		{
			safe_cprintf(ent, PRINT_HIGH, "You already have a totem active.\n");
			return;
		}
		//Can't have more than two totems.
		else*/ if(ent->totem2)
		{
			safe_cprintf(ent, PRINT_HIGH, "You already have two totems active.\n");
			return;
		}
		//Can't have two totems of opposite alignment.
		else
		{
			int opposite = 0;
			switch(ent->totem1->mtype)
			{
			case TOTEM_FIRE:		opposite = TOTEM_WATER;		break;
			case TOTEM_WATER:		opposite = TOTEM_FIRE;		break;
			case TOTEM_AIR:			opposite = TOTEM_EARTH;		break;
			case TOTEM_EARTH:		opposite = TOTEM_AIR;		break;
			case TOTEM_DARKNESS:	opposite = TOTEM_NATURE;	break;
			case TOTEM_NATURE:		opposite = TOTEM_DARKNESS;	break;
			}
			if(totemType == opposite)
			{
				safe_cprintf(ent, PRINT_HIGH, "You can't create two totems of opposite elemental alignment.\n");
				return;
			}
			else if(totemType == ent->totem1->mtype)
			{
				safe_cprintf(ent, PRINT_HIGH, "You can't create totems of the same type.\n");
				return;
			}
		}
	}

	//Drop a totem.
	totem = DropTotem(ent);
	totem->mtype = totemType;
	totem->monsterinfo.level = ent->myskills.abilities[abilityID].current_level;
	totem->classname = "totem";
	/*totem->owner =*/ totem->activator = ent;
	totem->think = totem_general_think;
	totem->touch = totem_touch;
	totem->nextthink = level.time + FRAMETIME*2;
	totem->delay = level.time + 0.5;
	totem->die = totem_die;

	//TODO: update this with the new model.
	totem->s.modelindex = gi.modelindex("models/items/mega_h/tris.md2");
	//totem->s.angles[ROLL] = 270;
	VectorSet (totem->mins, -8, -8, -12);
	VectorSet (totem->maxs, 8, 8, 16);
	VectorCopy(ent->s.origin, totem->s.origin);
	
	totem->health = TOTEM_HEALTH_BASE + TOTEM_HEALTH_MULT * totem->monsterinfo.level;

	//Talent: Totemic Focus - increases totem health
	if((talentLevel = getTalentLevel(ent, TALENT_TOTEM)) > 0)
		totem->health *= 1 + 0.1666 * talentLevel;

	if (totemType == TOTEM_FIRE)
	{
		// fire totem is much tougher
		totem->health *= 2;
		// fire totem has a longer delay
		totem->delay = level.time + 2.0;
	}

	totem->max_health = totem->health*2;

	//Not sure if this stuff is needed (Archer)
	totem->svflags |= SVF_MONSTER;
	totem->takedamage = DAMAGE_AIM;
	totem->clipmask = MASK_MONSTERSOLID;

	//Back to stuff we need
	totem->mass = 200;
	totem->movetype = MOVETYPE_TOSS;//MOVETYPE_WALK;
	totem->deadflag = DEAD_NO;
	totem->svflags &= ~SVF_DEADMONSTER;	
	totem->solid = SOLID_BBOX;

	//Graphical effects
	//TODO: update this to make it look better.
	totem->s.effects |= EF_PLASMA | EF_COLOR_SHELL | EF_SPHERETRANS;
	switch(totemType)
	{
	case TOTEM_FIRE:
		totem->s.effects |= 262144;		//red radius light
		totem->s.renderfx |= RF_SHELL_RED;
		break;
	case TOTEM_WATER:
		totem->s.effects |= 524288;		//blue radius light
		totem->s.renderfx |= RF_SHELL_CYAN;
		break;
	case TOTEM_AIR:
		totem->s.effects |= 64;			//bright light radius
		totem->s.renderfx |= RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_GREEN;		
		break;
	case TOTEM_EARTH:
		totem->s.renderfx |= RF_SHELL_YELLOW;
		break;
	case TOTEM_DARKNESS:
		totem->s.effects |= 2147483648;		//strange darkness effect
		//totem->s.renderfx |= RF_SHELL_RED | RF_SHELL_BLUE;
		break;
	case TOTEM_NATURE:
		totem->s.effects |= 128;	//green radius light
		totem->s.renderfx |= RF_SHELL_GREEN;
		break;
	}
//GHz 4.32
	if (!G_GetSpawnLocation(ent, 64, totem->mins, totem->maxs, start))
	{
		G_FreeEdict(totem);
		return;
	}

	VectorCopy(start, totem->s.origin);
	gi.linkentity(totem);
//GHz
	if(!ent->totem1)	ent->totem1 = totem;
	else				ent->totem2 = totem;

	ent->client->pers.inventory[ITEM_INDEX(Fdi_POWERCUBE)] -= cost;

	// calling entity made a sound, used to alert monsters
	ent->lastsound = level.framenum;
	ent->client->ability_delay = level.time + 1.3;
}

#endif