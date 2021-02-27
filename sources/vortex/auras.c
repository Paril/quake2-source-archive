#include "g_local.h"

/*
=============
CheckAuraOwner

Returns true if the owner is allowed to maintain an aura
=============
*/
qboolean CheckAuraOwner (edict_t *self, int aura_cost)
{
	return (G_EntIsAlive(self->owner) && self->owner->client 
		&& (self->owner->client->pers.inventory[power_cube_index] >= aura_cost)
		&& !self->owner->client->cloaking);
}

qboolean que_valident (que_t *que)
{
	// 3.5 aura/curse is no longer valid if the owner dies
	return (que->ent && que->ent->inuse && (que->time > level.time)
		&& G_EntIsAlive(que->ent->owner));
	/*
	return (que->ent && que->ent->inuse && que->ent->owner 
		&& que->ent->owner->inuse && (que->time > level.time));
	*/
}

void que_empty (que_t *que)
{
	int i;

	for (i=0; i<QUE_MAXSIZE; i++) {
		// reset que variables
		que[i].ent = NULL;
		que[i].time = 0;

	}
}

void que_removeent (que_t *que, edict_t *other, qboolean free)
{
	int i;

	if (other->deadflag == DEAD_DEAD)
		return;

	for (i=0; i<QUE_MAXSIZE; i++) {
		// if the entity matches, then remove it from the que
		if (que[i].ent && que[i].ent->inuse && (que[i].ent == other))
		{
			//if (free)
			//	G_FreeEdict(que[i].ent);
			que[i].ent = NULL;
			que[i].time = 0;
		}
	}

	// 3.5 sometimes the que owner is invalid
	// so make sure the entity is removed
	if (free)
	{
		// mark this entity for removal
		other->think = G_FreeEdict;
		other->nextthink = level.time + FRAMETIME;
		other->deadflag = DEAD_DEAD;
		//G_FreeEdict(other);
	}
}

que_t *que_emptyslot (que_t *que)
{
	int i;

	for (i=0; i<QUE_MAXSIZE; i++) {
		// que slot is in-use as long as the entity still exists
		// and the time has not expired
		if (que_valident(&que[i]))
			continue;
		return &que[i];
	}
	return NULL;
}

qboolean que_entexists (que_t *que, edict_t *other)
{
	int i;

	for (i=0; i<QUE_MAXSIZE; i++) {
		// return true if a valid entity is in que and hasn't timed out
		if (!que_valident(&que[i]))
			continue;
		if (que[i].ent != other)
			continue;
		return true;
	}
	return false;
}

void que_list (que_t *que)
{
	int i;

	for (i=0; i<QUE_MAXSIZE; i++)
	{
		if (que[i].ent && que[i].ent->inuse)
			gi.dprintf("slot %d: classname: %s owner: %s validtime: %s\n", i, que[i].ent->classname, que[i].ent->owner?"true":"false", que[i].time>level.time?"true":"false");
		else
			gi.dprintf("slot %d: %s\n", i, que[i].ent?"invalid":"empty");
	}
}

qboolean que_typeexists (que_t *que, int type)
{
	int i;

	for (i=0; i<QUE_MAXSIZE; i++)
	{
		if (que[i].ent && que[i].ent->inuse && que[i].ent->owner && (que[i].time > level.time))
		{
			//Searching for ANY curse?
			if (type == 0)
			{
				//3.0 blessings don't count as a curse, so skip them whenever they are found
				if (que[i].ent->atype)
				{
					switch(que[i].ent->atype)
					{
					case LIFE_DRAIN:
					case AMP_DAMAGE:
					case CURSE:
					case WEAKEN:
					case AMNESIA:
					case BLEEDING:
					case CURSE_BURN:
					case CURSE_FROZEN:
						return true;
					}
					switch(que[i].ent->mtype)
					{
					case LIFE_DRAIN:
					case AMP_DAMAGE:
					case CURSE:
					case WEAKEN:
					case AMNESIA:
					case BLEEDING:
					case CURSE_BURN:
					case CURSE_FROZEN:
						return true;
					}
				}
				//We were looking for any curse, and we found an old one
				else 
				{
					//gi.dprintf("%d %.1f %.1f\n", que[i].ent->atype, level.time, que[i].time);
					return true;
				}
			}
			// search for any curse (good or bad, we don't care)
			else if (type == -1)
				return true;
			//Check for a matching curse type
			else if (que[i].ent->mtype == type || que[i].ent->atype == type)
				return true;
		}
	}
	return false;
}

que_t *que_findent (que_t *src, que_t *dst, edict_t *other)
{
	que_t *last;

	if (!dst)
		dst = src;
	else
		dst++;

	last = &src[QUE_MAXSIZE-1];

	for ( ; dst < last; dst++)
	{
		if (!que_valident(dst))
			continue;
		if (dst->ent != other)
			continue;
		return dst;
	}
	return NULL;
}

que_t *que_findtype (que_t *src, que_t *dst, int type)
{
	que_t *last;

	if (!dst)
		dst = src;
	else
		dst++;

	last = &src[QUE_MAXSIZE-1];

	for ( ; dst < last; dst++)
	{
		if (!que_valident(dst))
			continue;
		if ((dst->ent->mtype != type) && (dst->ent->atype != type))
			continue;
		return dst;
	}
	return NULL;
}

void que_removetype (que_t *que, int type, qboolean free)
{
	int i;

	for (i=0; i<QUE_MAXSIZE; i++) {
		// remove all instances of this type from the queue
		if (que[i].ent && que[i].ent->inuse && 
			((que[i].ent->mtype == type) || (que[i].ent->atype == type)))
		{
			if (free)
				G_FreeEdict(que[i].ent);
			que[i].ent = NULL;
			que[i].time = 0;
		}
	}
}

void que_cleanup (que_t *que)
{
	int i;

	// remove invalid entries in queue
	for (i=0; i<QUE_MAXSIZE; i++) {
		if (que_valident(&que[i]))
			continue;
		que->ent = NULL;
		que->time = 0;
	}
}

qboolean que_addent (que_t *que, edict_t *other, float duration)
{
	que_t		*slot = NULL;

	// if it already exists in the que, update its time/duration
	while ((slot = que_findent(que, slot, other)) != NULL)
	{
		if ((other->mtype == AURA_HOLYFREEZE) || (other->mtype == AURA_SALVATION))
			slot->time = level.time + duration;
		else
			// add to it
			slot->time += duration;
		return true;
	}

	// otherwise try to find an available slot
	if ((slot = que_emptyslot(que)) != NULL)
	{
		slot->ent = other;
		slot->time = level.time + duration;
		return true;
	}
	return false;
}

void CurseRemove (edict_t *ent, int type)
{
	int			i;
	que_t		*slot;

	for (i=0; i<QUE_MAXSIZE; i++) 
	{
		slot = &ent->curses[i];

		// the slot is already empty
		if (!slot->ent)
			continue;

		// the type doesn't match
		if (type && slot->ent->inuse && slot->ent->mtype != type && slot->ent->atype != type)
			continue;

		// this curse is specifically targetting us, so destroy it
		if (!slot->ent->takedamage // make sure that the curse is really a curse!
			&& slot->ent->enemy && slot->ent->enemy->inuse && (slot->ent->enemy == ent))
			G_FreeEdict(slot->ent);

		// remove entry from the queue
		slot->ent = NULL;
		slot->time = 0;
		
	}
}

void AuraRemove (edict_t *ent, int type)
{
	int		i;
	que_t	*slot;

	for (i=0; i<QUE_MAXSIZE; i++) {
		slot = &ent->auras[i];
		if (slot->ent && slot->ent->inuse 
			&& (!type || slot->ent->mtype == type || slot->ent->atype == type))
		{
			// if we own this aura, then destroy it
			if (slot->ent->owner && (slot->ent->owner == ent))
				G_FreeEdict(slot->ent);
			// remove entry from the queue
			slot->ent = NULL;
			slot->time = 0;
		}
	}
}

void holyfreeze_think (edict_t *self)
{
	int		radius;
	edict_t *target=NULL, *curse=NULL;
	que_t	*slot;

	// check status of owner
	if (!CheckAuraOwner(self, DEFAULT_AURA_COST))
	{
	//gi.dprintf("aura removed itself\n");

		que_removeent(self->owner->auras, self, true);
		return;
	}
	
	// owner has an active aura
	que_addent(self->owner->auras, self, DEFAULT_AURA_DURATION);

	// use cubes
	if (!(level.framenum % DEFAULT_AURA_FRAMES))
	{
		int cube_cost = DEFAULT_AURA_COST;

		self->owner->client->pers.inventory[power_cube_index] -= cube_cost;
	}

	// move aura with owner
	VectorCopy(self->owner->s.origin,self->s.origin);
	self->nextthink = level.time + FRAMETIME;
	if (level.framenum % DEFAULT_AURA_SCAN_FRAMES)
		return;

	// scan for targets
	radius = DEFAULT_AURA_MIN_RADIUS+self->owner->myskills.abilities[HOLY_FREEZE].current_level*DEFAULT_AURA_ADDON_RADIUS;
	if (radius > DEFAULT_AURA_MAX_RADIUS)
		radius = DEFAULT_AURA_MAX_RADIUS;

	while ((target = findradius (target, self->s.origin, radius)) != NULL)
	{
		slot = NULL;
		if (target == self->owner)
			continue;
		if (!G_ValidTarget(self->owner, target, true))
			continue;
		// FIXME: make this into a loop search if we plan to allow
		// more than one curse of the same type
		slot = que_findtype(target->curses, slot, AURA_HOLYFREEZE);
		if (slot && (slot->ent->owner != self->owner))
		{
		//	gi.dprintf("already slowed by someone else\n");
			continue; // already slowed by someone else
		}
		if (!slot) // aura doesn't exist in que or timed out
		{
			if (random() > 0.5)
				gi.sound(target, CHAN_ITEM, gi.soundindex("spells/blue1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(target, CHAN_ITEM, gi.soundindex("spells/blue3.wav"), 1, ATTN_NORM, 0);
		}
		que_addent(target->curses, self, DEFAULT_AURA_DURATION);
	}
}

void aura_holyfreeze(edict_t *ent) 
{
	edict_t *holyfreeze;

	holyfreeze = G_Spawn();
	holyfreeze->movetype = MOVETYPE_NOCLIP;
	holyfreeze->svflags |= SVF_NOCLIENT;
	holyfreeze->solid = SOLID_NOT;
	VectorClear (holyfreeze->mins);
	VectorClear (holyfreeze->maxs);
	holyfreeze->owner = ent;
	holyfreeze->nextthink = level.time + FRAMETIME;
	holyfreeze->think = holyfreeze_think;
	holyfreeze->classname = "Aura";
	holyfreeze->mtype = AURA_HOLYFREEZE;
	VectorCopy(ent->s.origin, holyfreeze->s.origin);

	if (!que_addent(ent->auras, holyfreeze, DEFAULT_AURA_DURATION))
		G_FreeEdict(holyfreeze); // too many auras

	//que_list(ent->auras);
}

void Cmd_HolyFreeze(edict_t *ent)
{
	qboolean sameaura=false;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_HolyFreeze()\n", ent->client->pers.netname);

	if(ent->myskills.abilities[HOLY_FREEZE].disable)
		return;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[HOLY_FREEZE].current_level, 0))
		return;
	// if we already had an aura on, remove it
	if (que_typeexists(ent->auras, AURA_HOLYFREEZE))
	{
		safe_cprintf(ent, PRINT_HIGH, "Holy freeze removed.\n");
		AuraRemove(ent, AURA_HOLYFREEZE);
		return;
	}
	
	ent->client->ability_delay = level.time + DEFAULT_AURA_DELAY;
	// do we have enough power cubes?
	if (ent->client->pers.inventory[power_cube_index] < DEFAULT_AURA_INIT_COST)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need more %d power cubes to use this ability.\n", 
			DEFAULT_AURA_INIT_COST-ent->client->pers.inventory[power_cube_index]);
		return;
	}
	ent->client->pers.inventory[power_cube_index] -= DEFAULT_AURA_INIT_COST;
	gi.sound(ent, CHAN_ITEM, gi.soundindex("auras/holywind.wav"), 1, ATTN_NORM, 0);
	safe_cprintf(ent, PRINT_HIGH, "Now using holy freeze aura.\n");
	aura_holyfreeze(ent);
}

void salvation_think (edict_t *self)
{
	int		radius;
	edict_t *other=NULL;
	que_t	*slot=NULL;

	// check status of owner
	if (!CheckAuraOwner(self, COST_FOR_SALVATION))
	{
		que_removeent(self->owner->auras, self, true);
		return;
	}

	// use cubes
	if (!(level.framenum % DEFAULT_AURA_FRAMES))
	{
		int cube_cost = DEFAULT_AURA_COST;

		self->owner->client->pers.inventory[power_cube_index] -= cube_cost;
	}
	que_addent(self->owner->auras, self, DEFAULT_AURA_DURATION);
	// move aura with owner
	VectorCopy(self->owner->s.origin,self->s.origin);
	self->nextthink = level.time + FRAMETIME;
	if (level.framenum % DEFAULT_AURA_SCAN_FRAMES)
		return;

	radius = 256;

	// scan for targets
	while ((other = findradius (other, self->s.origin, radius)) != NULL)
	{
		slot = NULL;
		if (other == self->owner)
			continue;
		if (!G_EntExists(other))
			continue;
		if (other->health < 1)
			continue;
		if (OnSameTeam(self->owner, other) < 2)
			continue;
		if (!visible(self->owner, other))
			continue;
		slot = que_findtype(other->auras, slot, AURA_SALVATION);
		if (slot && (slot->ent->owner != self->owner))
			continue;
		que_addent(other->auras, self, DEFAULT_AURA_DURATION);
	}
}

void aura_salvation(edict_t *ent) 
{
	edict_t *salvation;

	salvation = G_Spawn();
	salvation->movetype = MOVETYPE_NOCLIP;
	salvation->svflags |= SVF_NOCLIENT;
	salvation->solid = SOLID_NOT;
	VectorClear (salvation->mins);
	VectorClear (salvation->maxs);
	salvation->owner = ent;
	salvation->nextthink = level.time + FRAMETIME;
	salvation->think = salvation_think;
	salvation->classname = "Aura";
	salvation->mtype = AURA_SALVATION;
	VectorCopy(ent->s.origin, salvation->s.origin);

	if (!que_addent(ent->auras, salvation, DEFAULT_AURA_DURATION))
		G_FreeEdict(salvation); // too many auras
}

void Cmd_Salvation(edict_t *ent)
{
	que_t		*slot=NULL;
	qboolean	sameaura=false;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_Salvation()\n", ent->client->pers.netname);

	if(ent->myskills.abilities[SALVATION].disable)
		return;
	if (!G_CanUseAbilities(ent, ent->myskills.abilities[SALVATION].current_level, 0))
		return;

	// if we already had an aura on, remove it
	if ((slot = que_findtype(ent->auras, slot, AURA_SALVATION)) != NULL)
	{
		// owner is turning off his own aura
		if (slot->ent && slot->ent->owner 
			&& slot->ent->owner->inuse && slot->ent->owner == ent)
		{
			AuraRemove(ent, AURA_SALVATION);
			safe_cprintf(ent, PRINT_HIGH, "Salvation removed.\n");
			return;
		}

		AuraRemove(ent, AURA_SALVATION);
	}

	ent->client->ability_delay = level.time + DEFAULT_AURA_DELAY;
	// do we have enough power cubes?
	if (ent->client->pers.inventory[power_cube_index] < DEFAULT_AURA_INIT_COST)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need more %d power cubes to use this ability.\n", 
			DEFAULT_AURA_INIT_COST-ent->client->pers.inventory[power_cube_index]);
		return;
	}
	ent->client->pers.inventory[power_cube_index] -= DEFAULT_AURA_INIT_COST;
	gi.sound(ent, CHAN_ITEM, gi.soundindex("auras/salvation.wav"), 1, ATTN_NORM, 0);
	safe_cprintf(ent, PRINT_HIGH, "Now using salvation aura.\n");
	aura_salvation(ent);
}
/*
#define HOLYSHOCK_DEFAULT_RADIUS	56
#define HOLYSHOCK_ADDON_RADIUS		20
#define HOLYSHOCK_MAX_RADIUS		256

int shockRechargeTime(edict_t *ent)
{
	int maxTime = 10;

	//Talent: Improved Holyshock
	if(getTalentSlot(ent, TALENT_IMP_SHOCK) != -1)
		maxTime -= getTalentLevel(ent, TALENT_IMP_SHOCK);	//-1 second per upgrade

	return GetRandom(1, maxTime);
}

void fire_lightningbolt (edict_t *ent, vec3_t start, vec3_t dir, float speed, int damage);

void holyshock_think (edict_t *self)
{
	int		radius, damage;
	edict_t *other=NULL;	
	vec3_t	zvec={0,0,0}, forward;
	qboolean hit=false;
	int talentLevel;
	
	if (!self || !self->inuse)
		return; // sanity check

	// must have an owner
	if (!CheckAuraOwner(self, COST_FOR_HOLYSHOCK))
	{
		que_removeent(self->owner->auras, self, true);
		return;
	}

	// update aura status
	que_addent(self->owner->auras, self, DEFAULT_AURA_DURATION);
	// follow the owner
	VectorCopy(self->owner->s.origin, self->s.origin);
	self->nextthink = level.time + FRAMETIME;

	// use cubes
	if (!(level.framenum % DEFAULT_AURA_FRAMES))
	{
		int cube_cost = DEFAULT_AURA_COST;		

		//Talent: Meditation
		talentLevel = getTalentLevel(self->owner, TALENT_MEDITATION);
		switch(talentLevel)
		{
		case 1:	cube_cost *= 1.0 - 0.1;	break;	//-10%
		case 2:	cube_cost *= 1.0 - 0.2;	break;	//-20%
		case 3:	cube_cost *= 1.0 - 0.5;	break;	//-50%
		default: //do nothing;
			break;
		}
		self->owner->client->pers.inventory[power_cube_index] -= cube_cost;
	}

	if ((level.time > self->PlasmaDelay) && !que_typeexists(self->owner->auras, CURSE_FROZEN))
	{
		// scan for targets
		radius = HOLYSHOCK_DEFAULT_RADIUS+self->owner->myskills.abilities[HOLY_SHOCK].current_level*HOLYSHOCK_ADDON_RADIUS;
		if (radius > HOLYSHOCK_MAX_RADIUS)
			radius = HOLYSHOCK_MAX_RADIUS;

		//Talent: Holy Power
		talentLevel = getTalentLevel(self->owner, TALENT_HOLY_POWER);
		switch(talentLevel)
		{
		case 1:	radius *= 1.0 + 0.15;	break;	//+15%
		case 2:	radius *= 1.0 + 0.3;	break;	//+30%
		case 3:	radius *= 1.0 + 0.5;	break;	//+50%
		default: //do nothing
			break;
		}

		while ((other = findradius(other, self->s.origin, radius)) != NULL)
		{
			if (!G_ValidTarget(self->owner, other, false))
				continue;
			if (!visible1(self->owner, other))
				continue;
			
			// deal the damage
			damage = GetRandom(1, (100 + 20*self->owner->myskills.abilities[HOLY_SHOCK].current_level));

			// select target
			self->enemy = other;
			// look at target
			VectorSubtract(other->s.origin, self->s.origin, forward);
			VectorNormalize(forward);
			vectoangles(forward, self->s.angles);
			// aim at target
			MonsterAim(self, 0.9, 650, false, 0, forward, self->s.origin);
			// fire lightning projectile
			fire_lightningbolt(self->owner, self->s.origin, forward, 650, damage);
			hit = true;
		}

		if (!hit)
			self->PlasmaDelay = level.time + FRAMETIME; // time when we scan again
		else
			self->PlasmaDelay = level.time + shockRechargeTime(self->owner); // recharge time
	}
}

void aura_holyshock(edict_t *ent) 
{
	edict_t *holyshock;

	holyshock = G_Spawn();
	holyshock->movetype = MOVETYPE_NOCLIP;
	holyshock->solid = SOLID_NOT;
	holyshock->svflags |= SVF_NOCLIENT;
	VectorClear (holyshock->mins);
	VectorClear (holyshock->maxs);
	holyshock->owner = ent;
	holyshock->nextthink = level.time + FRAMETIME;
	holyshock->PlasmaDelay = level.time + shockRechargeTime(ent);	//4.0 new shock recharge
	holyshock->think = holyshock_think;
	holyshock->classname = "Aura";
	holyshock->mtype = AURA_HOLYSHOCK;
	VectorCopy(ent->s.origin, holyshock->s.origin);
//	gi.linkentity(holyshock);

	//if (!AddAura(ent, ent, holyshock, AURA_HOLYSHOCK, DEFAULT_AURA_DURATION))
	if (!que_addent(ent->auras, holyshock, DEFAULT_AURA_DURATION))
		G_FreeEdict(holyshock); // too many auras
}

void Cmd_HolyShock(edict_t *ent)
{
	qboolean sameaura=false;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_HolyShock()\n", ent->client->pers.netname);

	if(ent->myskills.abilities[HOLY_SHOCK].disable)
		return;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[HOLY_SHOCK].current_level, 0))
		return;
	// if we already had an aura on, remove it
	if (que_typeexists(ent->auras, AURA_HOLYSHOCK))
	{
		safe_cprintf(ent, PRINT_HIGH, "Holy shock removed.\n");
		AuraRemove(ent, AURA_HOLYSHOCK);
		return;
	}
	ent->client->ability_delay = level.time + DEFAULT_AURA_DELAY;
	// do we have enough power cubes?
	if (ent->client->pers.inventory[power_cube_index] < DEFAULT_AURA_INIT_COST)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need more %d power cubes to use this ability.\n", 
			DEFAULT_AURA_INIT_COST-ent->client->pers.inventory[power_cube_index]);
		return;
	}
	ent->client->pers.inventory[power_cube_index] -= DEFAULT_AURA_INIT_COST;
	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);//Play the spell sound!
	safe_cprintf(ent, PRINT_HIGH, "Now using holy shock aura.\n");
	aura_holyshock(ent);
}
*/
